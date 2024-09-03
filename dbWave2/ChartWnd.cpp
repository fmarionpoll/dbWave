#include "StdAfx.h"
#include "resource.h"
#include "ChartWnd.h"
#include "DlgChartProps.h"
#include "ColorNames.h"
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COLORREF ChartWnd::color_spike_class[] =
{
	col_black,
	RGB(126, 132, 250), // 6
	RGB(246, 133, 17), // 3
	RGB(15, 181, 174), // 1
	RGB(197, 198, 198), //4
	RGB(114, 224, 106), // 7
	RGB(222, 61, 130), // 5
	RGB(20, 122, 243), // 8
	RGB(64, 70, 202), // 2
	RGB(115, 38, 211), // 9
};

COLORREF ChartWnd::color_spike_class_selected[] =
{
	col_red,
	col_red, // 6
	col_red, // 3
	col_red, // 1
	col_red, //4
	col_red, // 7
	col_red, // 5
	col_red, // 8
	col_red, // 2
	col_red // 9
};

COLORREF ChartWnd::color_spike_class_text[] =
{
	col_white,
	col_white, // 6
	col_black, // 3
	col_black, // 1
	col_black, //4
	col_black, // 7
	col_white, // 5
	col_white, // 8
	col_white, // 2
	col_white // 9
};

HCURSOR ChartWnd::cursors_[NB_CURSORS];
int ChartWnd::cursors_drag_mode_[NB_CURSORS];
int ChartWnd::cursors_count_ = 0;

int ChartWnd::find_color_index(const COLORREF color_ref)
{
	auto color_index = -1;
	for (auto i = 0; i < nb_colors; i++)
		if (color_ref == color_spike_class[i])
			color_index = i;
	return color_index;
}

IMPLEMENT_SERIAL(ChartWnd, CWnd, 1)

ChartWnd::ChartWnd()
{
	if (cursors_count_ == 0)
	{
		int j = 0;
		cursors_[j] = ::LoadCursor(nullptr, IDC_ARROW);
		cursors_drag_mode_[j] = 0;
		j++;
		cursors_[j] = AfxGetApp()->LoadCursor(IDC_CZOOM);
		cursors_drag_mode_[j] = 1;
		j++;
		cursors_[j] = AfxGetApp()->LoadCursor(IDC_CCROSS);
		cursors_drag_mode_[j] = 1;
		j++;
		cursors_[j] = AfxGetApp()->LoadCursor(IDC_CCROSS);
		cursors_drag_mode_[j] = 1;
		j++;
		cursors_[j] = AfxGetApp()->LoadCursor(IDC_SPLITHORIZONTAL);
		cursors_drag_mode_[j] = 1;
		j++;
		cursors_[j] = AfxGetApp()->LoadCursor(IDC_SPLITVERTICAL);
		cursors_drag_mode_[j] = 1;
	}
	cursors_count_++;

	ChartWnd::set_mouse_cursor_type(0);

	client_rect_ = CRect(0, 0, 10, 10); // minimal size of the button
	adjust_display_rect(&client_rect_);

	cx_mouse_jitter_ = GetSystemMetrics(SM_CXDOUBLECLK);
	cy_mouse_jitter_ = GetSystemMetrics(SM_CYDOUBLECLK);
	black_dotted_pen_.CreatePen(PS_DOT, 0, color_spike_class[BLACK_COLOR]);

	// set colored CPen objects
	for (int i = 0; i < nb_colors; i++)
		pen_table_[i].CreatePen(PS_SOLID, 0, color_spike_class[i]);

	x_ruler.m_is_horizontal = TRUE;
	y_ruler.m_is_horizontal = FALSE;
	h_font.CreateFont(12, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY,
	                   VARIABLE_PITCH | FF_ROMAN, _T("Arial"));
}

ChartWnd::~ChartWnd()
{
	CWnd::DestroyWindow();

	cursors_count_--;
	if (cursors_count_ == 0)
	{
		for (auto i = 1; i < NB_CURSORS; i++) // element 0: global object cursor
		{
			if (nullptr != cursors_[i])
				DestroyCursor(cursors_[i]);
		}
	}
	hz_tags.remove_all_tags(); 
	vt_tags.remove_all_tags(); 
	delete temp_vertical_tag_;

	// delete array of pens
	for (auto& i : pen_table_)
		i.DeleteObject();

}

// trap call to pre-subclass in order to get source window size
// assume that palette is present within the application inside CMainFrame...
void ChartWnd::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	// at this stage, assume that m_hWnd is valid
	::GetClientRect(m_hWnd, &client_rect_);
	adjust_display_rect(&client_rect_);
	x_viewport_origin_ = display_rect_.left;
	x_viewport_extent_ = display_rect_.Width();
	y_vo_ = display_rect_.Height() / 2;
	y_ve_ = -display_rect_.Height();
}

BEGIN_MESSAGE_MAP(ChartWnd, CWnd)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

BOOL ChartWnd::Create(const LPCTSTR lpsz_window_name, const DWORD dw_style, const RECT& rect, CWnd* p_parent_wnd, const UINT n_id,
                      CCreateContext* p_context)
{
	const auto flag = CWnd::Create(nullptr, lpsz_window_name, dw_style, rect, p_parent_wnd, n_id, p_context);
	if (flag != 0)
		PreSubclassWindow();
	return flag;
}

void ChartWnd::OnSize(const UINT n_type, const int cx, const int cy)
{
	CWnd::OnSize(n_type, cx, cy);
	set_display_area_size(cx, cy);
	if (plot_dc_.GetSafeHdc())
		plot_dc_.DeleteDC();
	Invalidate();
}

void ChartWnd::set_display_area_size(const int cx, const int cy)
{
	// update coordinates
	client_rect_.bottom = cy;
	client_rect_.right = cx;
	adjust_display_rect(&client_rect_);
	x_viewport_origin_ = display_rect_.left;
	x_viewport_extent_ = display_rect_.Width();
	y_vo_ = display_rect_.Height() / 2;
	y_ve_ = -display_rect_.Height();
}

BOOL ChartWnd::OnEraseBkgnd(CDC* p_dc)
{
	return TRUE; // say we handled it
}

void ChartWnd::plot_to_bitmap(CDC* p_dc)
{
	CBitmap bitmap_plot;
	bitmap_plot.CreateBitmap(client_rect_.right, client_rect_.bottom, p_dc->GetDeviceCaps(PLANES),
	                         p_dc->GetDeviceCaps(BITSPIXEL), nullptr);
	plot_dc_.CreateCompatibleDC(p_dc);
	const auto p_old_plot_bitmap = plot_dc_.SelectObject(&bitmap_plot);
	plot_data_to_dc(&plot_dc_);
	p_dc->BitBlt(0, 0, display_rect_.right, display_rect_.bottom, &plot_dc_, 0, 0, SRCCOPY);
	plot_dc_.SelectObject(p_old_plot_bitmap);
}

void ChartWnd::OnPaint()
{
	CPaintDC dc(this);
	dc.IntersectClipRect(&client_rect_);

	if (!b_use_dib_)
		plot_data_to_dc(&dc);
	else
		plot_to_bitmap(&dc);
}

void ChartWnd::plot_data_to_dc(CDC* p_dc)
{
	CRect rect = display_rect_;
	rect.DeflateRect(2, 2);
	const CString text = _T("nothing to display (stub)");
	p_dc->DrawText(text, text.GetLength(), rect, DT_LEFT | DT_SINGLELINE | DT_CENTER);
}

void ChartWnd::display_text_bottom_left(CDC* p_dc, const CString& text, const COLORREF& color) const
{
	CRect rect = display_rect_;
	rect.DeflateRect(2, 2);
	const int previous_map_mode = p_dc->SetMapMode(MM_TEXT);
	if (previous_map_mode == MM_ANISOTROPIC)
		rect.OffsetRect(0, -y_vo_);

	const COLORREF old_color = p_dc->SetTextColor(color);
	p_dc->DrawText(text, text.GetLength(), rect, DT_LEFT | DT_SINGLELINE | DT_BOTTOM); // DT_TOP

	p_dc->SetMapMode(previous_map_mode);
	p_dc->SetTextColor(old_color);
}

void ChartWnd::erase_background(CDC* p_dc)
{
	// erase background around m_displayRect
	if (b_nice_grid)
	{
		auto rect = client_rect_;
		rect.right = display_rect_.left;
		p_dc->FillSolidRect(rect, GetSysColor(COLOR_BTNFACE));

		rect.top = display_rect_.bottom;
		rect.right = client_rect_.right;
		rect.left = display_rect_.left;
		p_dc->FillSolidRect(rect, GetSysColor(COLOR_BTNFACE));
	}
	// erase display area
	CBrush brush;
	brush.CreateSolidBrush(scope_structure_.cr_scope_fill);
	const auto p_old_brush = p_dc->SelectObject(&brush);

	const auto p_old_pen = static_cast<CPen*>(p_dc->SelectStockObject(BLACK_PEN));
	p_dc->Rectangle(&display_rect_);

	p_dc->SelectObject(p_old_pen);
	p_dc->SelectObject(p_old_brush);

	// display grid
	draw_grid(p_dc);
}

void ChartWnd::draw_grid_evenly_spaced(CDC* p_dc) const
{
	auto rect = display_rect_;
	rect.DeflateRect(1, 1);

	// Standard grid is 8 high by 10 wide
	CPen pen_scale(PS_SOLID, 1, scope_structure_.cr_scope_grid);
	const auto pen_old = p_dc->SelectObject(&pen_scale);
	const auto i_x_ticks = scope_structure_.i_x_cells * scope_structure_.i_x_ticks;
	const auto i_y_ticks = scope_structure_.i_y_cells * scope_structure_.i_y_ticks;

	// do the grid lines
	for (auto i = 1; i < scope_structure_.i_x_cells; i++)
	{
		p_dc->MoveTo(i * rect.right / scope_structure_.i_x_cells, 0);
		p_dc->LineTo(i * rect.right / scope_structure_.i_x_cells, rect.bottom);
	}
	for (auto i = 1; i < scope_structure_.i_y_cells; i++)
	{
		p_dc->MoveTo(0, i * rect.bottom / scope_structure_.i_y_cells);
		p_dc->LineTo(rect.right, i * rect.bottom / scope_structure_.i_y_cells);
	}

	// Put tick marks on the axis lines
	for (auto i = 1; i < i_x_ticks; i++)
	{
		constexpr auto i_tick_width = 2;
		const int y = rect.bottom - rect.bottom * scope_structure_.i_x_tick_line / scope_structure_.i_y_cells;
		p_dc->MoveTo(i * rect.right / i_x_ticks, y - i_tick_width);
		p_dc->LineTo(i * rect.right / i_x_ticks, y + i_tick_width);
	}

	for (auto i = 1; i < i_y_ticks; i++)
	{
		constexpr auto i_tick_height = 2;
		const int x = rect.right * scope_structure_.i_y_tick_line / scope_structure_.i_x_cells;
		p_dc->MoveTo(x - i_tick_height, i * rect.bottom / i_y_ticks);
		p_dc->LineTo(x + i_tick_height, i * rect.bottom / i_y_ticks);
	}
	p_dc->SelectObject(pen_old);

	// if grids, draw scale text (dummy)
	if (scope_structure_.i_x_cells > 1 && scope_structure_.i_y_cells > 1)
	{
		// convert value into text
		CString cs;
		cs.Format(_T("%.3f mV; %.3f ms"), static_cast<double>(scope_structure_.y_scale_unit_value), static_cast<double>(scope_structure_.x_scale_unit_value));
		const auto text_length = cs.GetLength();
		// plot text
		p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
		rect.DeflateRect(1, 1);
		p_dc->SetTextColor(scope_structure_.cr_scope_grid);
		p_dc->DrawText(cs, text_length, rect, DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
	}
}

void ChartWnd::draw_grid_from_ruler(CDC* p_dc, const Ruler* p_ruler) const
{
	auto rc_client = display_rect_;
	rc_client.DeflateRect(1, 1);

	// exit if length is not properly defined
	if (p_ruler->m_highest_value == p_ruler->m_lowest_value)
		return;

	CPen a_pen2;
	a_pen2.CreatePen(PS_SOLID, 1, scope_structure_.cr_scope_grid);
	const auto p_old_pen = p_dc->SelectObject(&a_pen2);

	// draw ticks and legends
	//auto tick_small_height = 4;
	int tick_max;
	if (p_ruler->m_is_horizontal) // horizontal
		tick_max = rc_client.Width();
	else // vertical
		tick_max = rc_client.Height();

	// draw scale
	//double small_scale_inc = pRuler->m_d_scale_inc / 5.;
	auto d_pos = floor(p_ruler->m_first_major_scale);
	const auto d_len = p_ruler->m_highest_value - p_ruler->m_lowest_value;
	p_dc->SetBkMode(TRANSPARENT);

	while (d_pos <= p_ruler->m_highest_value)
	{
		int tick_pos;
		if (p_ruler->m_is_horizontal) // horizontal
		{
			tick_pos = static_cast<int>(rc_client.Width() * (d_pos - p_ruler->m_lowest_value) / d_len) + rc_client.left;
			if (tick_pos >= 0 && tick_pos <= tick_max)
			{
				p_dc->MoveTo(tick_pos, rc_client.bottom - 1); // line
				p_dc->LineTo(tick_pos, rc_client.top + 1);
			}
		}
		else // vertical
		{
			tick_pos = static_cast<int>(rc_client.Height() * (p_ruler->m_highest_value - d_pos) / d_len) + rc_client.top;
			if (tick_pos >= 0 && tick_pos <= tick_max)
			{
				p_dc->MoveTo(rc_client.left + 1, tick_pos); // line
				p_dc->LineTo(rc_client.right - 1, tick_pos);
			}
		}
		if (d_pos != 0. && fabs(d_pos) < 1E-10)
			d_pos = 0;
		d_pos += p_ruler->m_length_major_scale;
	}
	// restore objects used in this routine
	p_dc->SelectObject(p_old_pen);
}

void ChartWnd::draw_scale_from_ruler(CDC* p_dc, const Ruler* p_ruler)
{
	auto rc_client = display_rect_;
	rc_client.DeflateRect(1, 1);

	// exit if length is not properly defined
	if (p_ruler->m_highest_value == p_ruler->m_lowest_value)
		return;

	CPen a_pen1;
	CPen a_pen2;
	a_pen1.CreatePen(PS_SOLID, 1, scope_structure_.cr_scope_grid);
	const auto p_old_pen = p_dc->SelectObject(&a_pen1);
	a_pen2.CreatePen(PS_SOLID, 1, scope_structure_.cr_scope_grid);
	/*auto p_old_font = */
	p_dc->SelectObject(&h_font);
	CString str;

	// draw ticks and legends
	int tick_max;
	if (p_ruler->m_is_horizontal) // horizontal
		tick_max = rc_client.Width();
	else // vertical
		tick_max = rc_client.Height();

	// draw scale
	const auto small_scale_inc = p_ruler->m_length_major_scale / 5.;
	auto d_pos = floor(p_ruler->m_first_major_scale);
	const auto d_len = p_ruler->m_highest_value - p_ruler->m_lowest_value;
	p_dc->SetBkMode(TRANSPARENT);

	while (d_pos <= p_ruler->m_highest_value)
	{
		// display small tick marks
		p_dc->SelectObject(&a_pen1);
		auto d_small_pos = d_pos;
		int tick_pos;
		for (auto i = 0; i < 4; i++)
		{
			constexpr auto tick_small_height = 4;
			d_small_pos += small_scale_inc;
			if (p_ruler->m_is_horizontal) // ---------------------------- horizontal
			{
				tick_pos = static_cast<int>(rc_client.Width() * (d_small_pos - p_ruler->m_lowest_value) / d_len) + rc_client.left;
				if (tick_pos >= rc_client.left && tick_pos <= tick_max)
				{
					p_dc->MoveTo(tick_pos, rc_client.bottom - 1);
					p_dc->LineTo(tick_pos, rc_client.bottom - tick_small_height);
				}
			}
			else // --------------------------------------------------- vertical
			{
				tick_pos = static_cast<int>(rc_client.Height() * (p_ruler->m_highest_value - d_small_pos) / d_len) + rc_client.top;
				if (tick_pos >= rc_client.top && tick_pos <= tick_max)
				{
					p_dc->MoveTo(rc_client.left + 1, tick_pos);
					p_dc->LineTo(rc_client.left + tick_small_height, tick_pos);
				}
			}
		}

		// display large ticks and text
		p_dc->SelectObject(&a_pen2);
		if (p_ruler->m_is_horizontal) // horizontal
			tick_pos = static_cast<int>(rc_client.Width() * (d_pos - p_ruler->m_lowest_value) / d_len) + rc_client.left;
		else // vertical
			tick_pos = static_cast<int>(rc_client.Height() * (p_ruler->m_highest_value - d_pos) / d_len) + rc_client.top;
		if (tick_pos >= 0 && tick_pos <= tick_max)
		{
			str.Format(_T("%g"), d_pos);
			const auto size = p_dc->GetTextExtent(str);
			int x, y;
			if (p_ruler->m_is_horizontal) // ----------- horizontal
			{
				p_dc->MoveTo(tick_pos, rc_client.bottom - 1); // line
				p_dc->LineTo(tick_pos, rc_client.top + 1);
				x = tick_pos - (size.cx / 2); // text position (upper left)
				if (x < 0)
					x = 0;
				if (x + size.cx > rc_client.right)
					x = rc_client.right - size.cx;
				y = rc_client.bottom + 1; // - 1; //- size.cy
			}
			else // ---------------------------------- vertical
			{
				p_dc->MoveTo(rc_client.left + 1, tick_pos); // line
				p_dc->LineTo(rc_client.right - 1, tick_pos);
				x = rc_client.left - size.cx - 2; // text position (upper left)
				y = tick_pos - (size.cy / 2);
			}
			p_dc->TextOut(x, y, str);
		}
		if (d_pos != 0. && fabs(d_pos) < 1E-10)
			d_pos = 0;
		d_pos += p_ruler->m_length_major_scale;
	}

	// restore objects used in this routine
	p_dc->SelectObject(p_old_pen);
}

void ChartWnd::draw_grid_nicely_spaced(CDC* p_dc)
{
	if (x_ruler_bar == nullptr)
	{
		draw_scale_from_ruler(p_dc, &x_ruler);
	}
	else
	{
		x_ruler_bar->draw_scale_from_ruler(&x_ruler);
		x_ruler_bar->Invalidate();
		draw_grid_from_ruler(p_dc, &x_ruler);
	}

	if (y_ruler_bar == nullptr)
		draw_scale_from_ruler(p_dc, &y_ruler);
	else
	{
		y_ruler_bar->draw_scale_from_ruler(&y_ruler);
		y_ruler_bar->Invalidate();
		draw_grid_from_ruler(p_dc, &y_ruler);
	}
}

void ChartWnd::adjust_display_rect(const CRect* p_rect)
{
	display_rect_ = *p_rect;
	if (b_nice_grid)
	{
		if (y_ruler_bar == nullptr)
			display_rect_.left += ordinates_width;
		if (x_ruler_bar == nullptr)
			display_rect_.bottom -= abscissa_height;
	}
}

void ChartWnd::draw_grid(CDC* p_dc)
{
	// TODO: get major intervals from ruler bar is not nullptr
	if (b_nice_grid)
		draw_grid_nicely_spaced(p_dc);
	else
		draw_grid_evenly_spaced(p_dc);
}

void ChartWnd::set_n_x_scale_cells(const int i_cells, const int i_ticks, const int i_tick_line)
{
	scope_structure_.i_x_cells = i_cells;
	scope_structure_.i_x_ticks = i_ticks;
	scope_structure_.i_x_tick_line = i_tick_line;
}

void ChartWnd::set_ny_scale_cells(const int i_cells, const int i_ticks, const int i_tick_line)
{
	scope_structure_.i_y_cells = i_cells;
	scope_structure_.i_y_ticks = i_ticks;
	scope_structure_.i_y_tick_line = i_tick_line;
}

void ChartWnd::send_my_message(const int code, const int code_parameter) const
{
	GetParent()->SendMessage(WM_MYMESSAGE, code, MAKELONG(code_parameter, GetDlgCtrlID()));
}

void ChartWnd::prepare_dc(CDC* p_dc, const CPrintInfo* p_info)
{
	if (p_info == nullptr)
	{
		p_dc->SetViewportOrg(x_viewport_origin_, y_vo_);
		p_dc->SetViewportExt(x_viewport_extent_, y_ve_);
		if (y_we_ == 0)
			y_we_ = 1024;
		if (x_we_ == 0)
			x_we_ = 1024;
		p_dc->SetWindowExt(x_we_, y_we_);
		p_dc->SetWindowOrg(x_wo_, y_wo_);
	}
}

int ChartWnd::set_mouse_cursor_type(const int cursor_type)
{
	old_cursor_type_ = cursor_type_;
	ASSERT(NB_CURSORS > cursor_type);
	ASSERT(0 <= cursor_type);
	cursor_type_ = cursor_type;
	handle_current_cursor_ = cursors_[cursor_type_];
	current_cursor_mode_ = cursors_drag_mode_[cursor_type_];
	return cursor_type;
}

void ChartWnd::set_mouse_cursor(const int cursor_type)
{
	set_mouse_cursor_type(cursor_type);
	SetCursor(handle_current_cursor_);
}

void ChartWnd::set_yw_ext_org(const int extent, const int zero)
{
	y_we_ = extent;
	y_wo_ = zero;
}

void ChartWnd::capture_cursor()
{
	SetCapture();
	auto rect_limit = display_rect_;
	ClientToScreen(rect_limit);
	ClipCursor(rect_limit);
}

void ChartWnd::release_cursor()
{
	track_mode_ = TRACK_OFF;
	ReleaseCapture();
	ClipCursor(nullptr);
}

BOOL ChartWnd::OnSetCursor(CWnd* p_wnd, UINT n_hit_test, UINT message) 
{
	SetCursor(handle_current_cursor_);
	return TRUE;
}

void ChartWnd::OnLButtonDblClk(UINT n_flags, CPoint point)
{
	int new_cursor = cursor_type_ + 1;
	if (new_cursor >= cursor_index_max_)
		new_cursor = 0;
	send_my_message(HINT_SET_MOUSE_CURSOR, new_cursor);
}

void ChartWnd::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	if (h_wnd_reflect_ != nullptr)
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(h_wnd_reflect_, &rect0);
		::SendMessage(h_wnd_reflect_, WM_LBUTTONDOWN, n_flags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left), point.y + (rect1.top - rect0.top)));
	}
	else
	{
		pt_first_ = point;
		pt_last_ = point;
		pt_curr_ = point;

		// take action according to cursor mode
		switch (cursor_type_)
		{
		// track horizontal & VT cursors if mouse HIT
		case 0: 
		case CURSOR_CROSS: 
			if (n_flags & MK_CONTROL)
				send_my_message(HINT_L_MOUSE_BUTTON_DOWN_CTRL, MAKELONG(point.x, point.y));

			track_mode_ = TRACK_RECT; 

		// test HZ tags - if OK, then start tracking & init variables & flags
			hc_trapped_ = hit_horizontal_tag(point.y);
			if (hc_trapped_ >= 0)
			{
				track_mode_ = TRACK_HZ_TAG;
				pt_last_.x = 0; 
				pt_last_.y = hz_tags.get_tag_pixel(hc_trapped_);
				pt_first_ = pt_last_;
				send_my_message(HINT_HIT_HZ_TAG, hc_trapped_);
				break;
			}

		// test VT tags - if OK, then track
			if (!b_vertical_tags_as_long_)
				hc_trapped_ = hit_vertical_tag_pixel(static_cast<int>(point.x));
			else
			{
				file_position_equivalent_to_mouse_jitter_ = static_cast<long>(cx_mouse_jitter_) * (file_position_last_right_pixel_ - file_position_first_left_pixel_ + 1) / static_cast<long>(
					display_rect_.Width());
				const auto lx = static_cast<long>(point.x) * (file_position_last_right_pixel_ - file_position_first_left_pixel_ + 1) / static_cast<long>(
					display_rect_.Width()) + file_position_first_left_pixel_;
				hc_trapped_ = hit_vertical_tag_long(lx);
			}

		// mouse cursor did hit a tag, either horizontal or vertical
			if (hc_trapped_ >= 0)
			{
				track_mode_ = TRACK_VT_TAG;
				if (b_vertical_tags_as_long_)
					pt_last_.x = static_cast<int>((vt_tags.get_tag_value_long(hc_trapped_) - file_position_first_left_pixel_) 
						* static_cast<long>(display_rect_.Width()) 
						/ (file_position_last_right_pixel_ - file_position_first_left_pixel_ + 1));
				else
					pt_last_.x = vt_tags.get_tag_pixel(hc_trapped_);
				pt_last_.y = 0;
				send_my_message(HINT_HIT_VERT_TAG, hc_trapped_);
				break;
			}
			break;

		// track rectangle and invert content of the rectangle
		case CURSOR_ZOOM: // zoom (1)
			track_mode_ = TRACK_RECT;
			invert_tracker(point); // invert rectangle
			break;

		case CURSOR_VERTICAL:
			GetParent()->SendMessage(WM_MYMESSAGE, WM_LBUTTONDOWN, MAKELONG(point.x, point.y));
			return;

		default:
			break;
		}

		// limit the tracking to the client area of the view
		capture_cursor();
	}
}

void ChartWnd::OnMouseMove(const UINT n_flags, const CPoint point)
{
	// track rectangle : update rectangle size
	switch (track_mode_)
	{
	case TRACK_RECT:
		invert_tracker(point);
		break;

	// track horizontal tag : move tag, get value and send message
	case TRACK_HZ_TAG:
		if (point.y != pt_curr_.y)
		{
			Tag* p_tag = hz_tags.get_tag(hc_trapped_);
			xor_hz_tag(point.y, p_tag->swap_pixel(point.y));

			pt_curr_ = point;
			const auto val = MulDiv(point.y - y_vo_, y_we_, y_ve_) + y_wo_;
			p_tag->value_int = val;
			send_my_message(HINT_MOVE_HZ_TAG, hc_trapped_);
		}
		break;

	// track vertical tag : move tag & update value
	case TRACK_VT_TAG:
		if (point.x != pt_curr_.x)
		{
			Tag* p_tag = vt_tags.get_tag(hc_trapped_);
			xor_vt_tag(point.x, p_tag->swap_pixel(point.x));

			pt_curr_ = point;
			if (!b_vertical_tags_as_long_)
			{
				const auto val = MulDiv(point.x - x_viewport_origin_, x_we_, x_viewport_extent_) + x_wo_;
				p_tag->value_int = val;
			}
			else
			{
				const auto lvalue = static_cast<long>(point.x) * (file_position_last_right_pixel_ - file_position_first_left_pixel_ + 1) / static_cast<long>(
					display_rect_.Width()) + file_position_first_left_pixel_;
				p_tag->value_long = lvalue;
			}
			send_my_message(HINT_MOVE_VERT_TAG, hc_trapped_);
		}
		break;

	default:
		if (h_wnd_reflect_ != nullptr)
		{
			// convert coordinates
			CRect rect0, rect1;
			GetWindowRect(&rect1);
			::GetWindowRect(h_wnd_reflect_, &rect0);

			// reflect mouse move message
			::SendMessage(h_wnd_reflect_, WM_MOUSEMOVE, n_flags,
			              MAKELPARAM(point.x + (rect1.left - rect0.left),
			                         point.y + (rect1.top - rect0.top)));
		}
		break;
	}
}

void ChartWnd::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	if (track_mode_ != TRACK_OFF)
	{
		release_cursor();
		if (track_mode_ == TRACK_RECT && current_cursor_mode_)
			invert_tracker(point);
		track_mode_ = TRACK_OFF;
	}
	else if (h_wnd_reflect_ != nullptr)
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(h_wnd_reflect_, &rect0);

		// reflect mouse move message
		::SendMessage(h_wnd_reflect_, WM_LBUTTONUP, n_flags,
		              MAKELPARAM(point.x + (rect1.left - rect0.left),
		                         point.y + (rect1.top - rect0.top)));
	}
	b_left_mouse_button_down_ = FALSE;
}

void ChartWnd::left_button_up_horizontal_tag(const UINT n_flags, const CPoint point)
{
	Tag* p_tag = hz_tags.get_tag(hc_trapped_);
	p_tag->value_int = MulDiv(point.y - y_vo_, 
						y_we_, 
						y_ve_) + y_wo_;
	xor_hz_tag(point.y, p_tag->swap_pixel(point.y));

	ChartWnd::OnLButtonUp(n_flags, point);
	send_my_message(HINT_CHANGE_HZ_TAG, hc_trapped_);
}

void ChartWnd::left_button_up_vertical_tag(const UINT n_flags, const CPoint point)
{
	Tag* p_tag = vt_tags.get_tag(hc_trapped_);
	p_tag->value_int = MulDiv(point.x - x_viewport_origin_,
		x_we_,
		x_viewport_extent_) + x_wo_;
	xor_vt_tag(point.x, p_tag->swap_pixel(point.x));

	ChartWnd::OnLButtonUp(n_flags, point);
	send_my_message(HINT_CHANGE_VERT_TAG, hc_trapped_);
}

void ChartWnd::OnRButtonDown(const UINT n_flags, const CPoint point)
{
	switch (cursor_type_)
	{
	case CURSOR_ZOOM:
	case CURSOR_CROSS: // tracking type
		pt_first_ = point;
		pt_last_ = point;
		track_mode_ = TRACK_RECT; // flag track_rect
		invert_tracker(point); // invert rectangle
		capture_cursor();
		break;
	case CURSOR_VERTICAL:
	default:
		CWnd::OnRButtonDown(n_flags, point);
		break;
	}
}

void ChartWnd::OnRButtonUp(const UINT n_flags, const CPoint point)
{
	switch (track_mode_)
	{
	case TRACK_RECT:
		{
			release_cursor();
			// skip too small a rectangle (5 pixels?)
			CRect rect_out(pt_first_.x, pt_first_.y, pt_last_.x, pt_last_.y);
			constexpr int jitter = 3;
			if (rect_out.Height() < jitter && rect_out.Width() < jitter)
			{
				zoom_out();
			}
			else
			{
				auto rect_in = display_rect_;
				zoom_data(&rect_out, &rect_in);
				rect_zoom_from_ = rect_out;
				rect_zoom_to_ = rect_in;
				i_undo_zoom_ = -1;
			}
		}
		if (cursor_type_ == CURSOR_CROSS)
			send_my_message(HINT_R_MOUSE_BUTTON_UP, NULL); // post?
		else
			send_my_message(HINT_SET_MOUSE_CURSOR, old_cursor_type_); // post?
		break;

	case TRACK_OFF:
		CWnd::OnRButtonUp(n_flags, point);
		if (b_allow_props_)
		{
			const auto params_old = new options_scope_struct();
			*params_old = scope_structure_;
			DlgChartProps dlg;
			dlg.m_p_scope = this;
			b_allow_props_ = FALSE; // inhibit properties

			// if Cancel or Escape or anything else: restore previous values
			if (IDOK != dlg.DoModal())
			{
				scope_structure_ = *params_old;
				Invalidate();
			}
			else
				send_my_message(HINT_WINDOW_PROPS_CHANGED, NULL); // post?
			b_allow_props_ = TRUE;
			delete params_old;
		}
		break;

	default:
		release_cursor();
		zoom_out();
		break;
	}
	track_mode_ = TRACK_OFF;
}

void ChartWnd::zoom_data(CRect* prev_rect, CRect* new_rect)
{
}

void ChartWnd::zoom_pop()
{
	zoom_data(&rect_zoom_to_, &rect_zoom_from_);
	i_undo_zoom_ = 0;
}

void ChartWnd::zoom_out()
{
	if (i_undo_zoom_ > 0) 
		zoom_pop();
	else
	{
		CClientDC dc(this);
		rect_zoom_to_ = display_rect_;
		rect_zoom_from_ = rect_zoom_to_;
		const auto y_shrink = rect_zoom_to_.Height() / 4;
		const auto x_shrink = rect_zoom_to_.Width() / 4;
		rect_zoom_to_.InflateRect(x_shrink, y_shrink);
		zoom_data(&rect_zoom_from_, &rect_zoom_to_);
		i_undo_zoom_ = -1;
	}
}

void ChartWnd::zoom_in()
{
	if (i_undo_zoom_ < 0) 
		zoom_pop();
	else
	{
		CClientDC dc(this);
		rect_zoom_to_ = display_rect_;
		rect_zoom_from_ = rect_zoom_to_;
		const auto y_shrink = -rect_zoom_to_.Height() / 4;
		const auto x_shrink = -rect_zoom_to_.Width() / 4;
		rect_zoom_to_.InflateRect(x_shrink, y_shrink);
		zoom_data(&rect_zoom_from_, &rect_zoom_to_);
		i_undo_zoom_ = 1;
	}
}

int ChartWnd::hit_horizontal_tag(const int y)
{
	auto hit_cursor = -1; 
	const auto j = hz_tags.get_tag_list_size();
	for (auto i = 0; i < j; i++) 
	{
		constexpr auto jitter = 3;
		const auto val = hz_tags.get_tag_pixel(i); 
		if (val <= y + jitter && val >= y - jitter)
		{
			hit_cursor = i;
			break;
		}
	}
	return hit_cursor;
}

void ChartWnd::invert_tracker(const CPoint point)
{
	CClientDC dc(this); 
	const auto old_brush = static_cast<CBrush*>(dc.SelectStockObject(NULL_BRUSH));
	const auto old_rop = dc.SetROP2(R2_NOTXORPEN);
	const auto old_pen = dc.SelectObject(&black_dotted_pen_);
	dc.Rectangle(pt_first_.x, pt_first_.y, pt_last_.x, pt_last_.y);
	dc.Rectangle(pt_first_.x, pt_first_.y, point.x, point.y);

	dc.SelectObject(old_pen);
	dc.SelectObject(old_brush);
	dc.SetROP2(old_rop);
	pt_last_ = point;
}

void ChartWnd::display_vt_tags_int_values(CDC* p_dc)
{
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);

	const auto y0 = display_rect_.top;
	const auto y1 = display_rect_.bottom;

	for (auto tag_index = vt_tags.get_tag_list_size() - 1; tag_index >= 0; tag_index--)
	{
		Tag* p_tag = vt_tags.get_tag(tag_index);
		p_tag->pixel = MulDiv(p_tag->value_int  - x_wo_, x_viewport_extent_, x_we_)
					+ x_viewport_origin_;
		p_dc->MoveTo(p_tag->pixel, y0);
		p_dc->LineTo(p_tag->pixel, y1);
	}

	p_dc->SelectObject(old_pen);
	p_dc->SetROP2(old_rop2);
}

void ChartWnd::display_hz_tags(CDC* p_dc)
{
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);

	const auto x0 = display_rect_.left; 
	const auto x1 = display_rect_.right; 
	for (auto tag_index = hz_tags.get_tag_list_size() - 1; tag_index >= 0; tag_index--)
	{
		Tag* p_tag = hz_tags.get_tag(tag_index);
		p_tag->pixel = MulDiv(p_tag->value_int - y_wo_, y_ve_, y_we_) + y_vo_;
		p_dc->MoveTo(x0, p_tag->pixel);
		p_dc->LineTo(x1, p_tag->pixel);
	}

	p_dc->SelectObject(old_pen);
	p_dc->SetROP2(old_rop2);
}

void ChartWnd::xor_hz_tag(const int y_new, const int y_old)
{
	CClientDC dc(this);
	const auto old_pen = dc.SelectObject(&black_dotted_pen_);
	const auto old_rop2 = dc.SetROP2(R2_NOTXORPEN);
	dc.IntersectClipRect(&display_rect_);

	// erase old
	if (y_old >= 0)
	{
		dc.MoveTo(display_rect_.left, y_old);
		dc.LineTo(display_rect_.right, y_old);
	}

	// display new
	if (y_new >= 0)
	{
		dc.MoveTo(display_rect_.left, y_new);
		dc.LineTo(display_rect_.right, y_new);
	}

	dc.SetROP2(old_rop2);
	dc.SelectObject(old_pen);
}

void ChartWnd::xor_vt_tag(const int x_new, const int x_old)
{
	CClientDC dc(this);
	const auto old_pen = dc.SelectObject(&black_dotted_pen_);
	const auto old_rop2 = dc.SetROP2(R2_NOTXORPEN);
	dc.IntersectClipRect(&display_rect_);

	if (x_old >= 0)
	{
		dc.MoveTo(x_old, display_rect_.top);
		dc.LineTo(x_old, display_rect_.bottom);
	}

	if (x_new >= 0)
	{
		dc.MoveTo(x_new, display_rect_.top);
		dc.LineTo(x_new, display_rect_.bottom);
	}

	dc.SetROP2(old_rop2);
	dc.SelectObject(old_pen);
}

void ChartWnd::xor_temp_vt_tag(const int x_point)
{
	if (temp_vertical_tag_ == nullptr)
		temp_vertical_tag_ = new Tag;

	xor_vt_tag(x_point, temp_vertical_tag_->swap_pixel(x_point));
}

options_scope_struct* ChartWnd::get_scope_parameters()
{
	return &scope_structure_;
}

void ChartWnd::set_scope_parameters(options_scope_struct* p_struct)
{
	scope_structure_ = *p_struct;
}

void ChartWnd::set_b_use_dib(const BOOL b_set_plot)
{
	b_use_dib_ = b_set_plot; //TRUE = use DIB array to draw curves FALSE: do not use it
}

void ChartWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << plot_mode_;
		ar << index_color_background_;
		ar << index_color_selected_;
		ar << x_wo_; 
		ar << x_we_;
		ar << y_wo_; 
		ar << y_we_;

		ar << x_viewport_origin_;
		ar << x_viewport_extent_;
		ar << y_vo_;
		ar << y_ve_;
	}
	else
	{
		ar >> plot_mode_;
		ar >> index_color_background_;
		ar >> index_color_selected_;
		ar >> x_wo_; 
		ar >> x_we_;
		ar >> y_wo_; 
		ar >> y_we_;

		ar >> x_viewport_origin_;
		ar >> x_viewport_extent_;
		ar >> y_vo_;
		ar >> y_ve_;
	}
	scope_structure_.Serialize(ar);
}

int ChartWnd::hit_curve(CPoint point)
{
	return 0;
}

void ChartWnd::set_x_scale_unit_value(float x)
{
	scope_structure_.x_scale_unit_value = x;
}

void ChartWnd::set_y_scale_unit_value(float y)
{
	scope_structure_.y_scale_unit_value = y;
}


