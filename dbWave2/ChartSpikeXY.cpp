
#include "StdAfx.h"
#include "ChartSpikeXY.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_ballfiles is true: display and spike hit

BEGIN_MESSAGE_MAP(ChartSpikeXY, ChartSpike)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

ChartSpikeXY::ChartSpikeXY()
{
	SetbUseDIB(FALSE);
	m_csEmpty = _T("no spikes (spikexp)");
}

ChartSpikeXY::~ChartSpikeXY()
= default;

void ChartSpikeXY::PlotDataToDC(CDC* p_dc)
{
	if (m_erasebkgnd)
		EraseBkgnd(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1, 1);

	// save context
	const auto saved_dc = p_dc->SaveDC();
	const auto saved_background_color = p_dc->GetBkColor();

	// display data: trap error conditions
	const auto window_duration = m_lLast - m_lFirst + 1; 
	get_extents(); 
	ASSERT(m_xWE != 1);
	p_dc->SetMapMode(MM_TEXT);

	// prepare brush & rectangles (small for all spikes, larger for spikes belonging to the selected class)
	auto width = m_dot_width / 2;
	const CRect rect0(-width, -width, width, width);
	width = m_dot_width * 2 / 3;
	const CRect rect1(-width, -width, width, width);

	long n_files = 1;
	long index_current_file = 0;
	if (m_display_all_files)
	{
		n_files = p_dbwave_doc->db_get_n_records();
		index_current_file = p_dbwave_doc->db_get_current_record_position();
	}

	for (long index_file = 0; index_file < n_files; index_file++)
	{
		if (m_display_all_files)
		{
			if (p_dbwave_doc->db_set_current_record_position(index_file))
				p_dbwave_doc->open_current_spike_file();
		}
		p_spike_list = p_dbwave_doc->m_pSpk->get_spike_list_current();

		// test presence of data
		if (p_spike_list == nullptr || p_spike_list->get_spikes_count() == 0)
		{
			if (!m_display_all_files)
				p_dc->DrawText(m_csEmpty, m_csEmpty.GetLength(), rect, DT_LEFT);
			continue;
		}

		auto last_spike_index = p_spike_list->get_spikes_count() - 1;
		auto first_spike_index = 0;
		if (m_range_mode == RANGE_INDEX)
		{
			if (m_index_last_spike > last_spike_index)
				m_index_last_spike = last_spike_index;
			if (m_index_first_spike < 0)
				m_index_first_spike = 0;
			last_spike_index = m_index_last_spike;
			first_spike_index = m_index_first_spike;
		}

		// loop over all spikes
		for (auto spike_index = last_spike_index; spike_index >= first_spike_index; spike_index--)
		{
			display_spike(p_spike_list->get_spike(spike_index), p_dc, rect0, rect1, window_duration);
		}

		// display spike selected
		if (spike_selected_.database_position == index_file && spike_selected_.spike_index >= 0)
		{
			const Spike* spike = p_dbwave_doc->get_spike(spike_selected_);
			highlight_spike(spike);
		}

		if (p_spike_list->get_spike_flag_array_count() > 0)
		{
			// loop over the array of flagged spikes
			auto spike_sel = Spike_selected (index_file, p_dbwave_doc->m_pSpk->get_spike_list_current_index(), 0);
			for (auto i = p_spike_list->get_spike_flag_array_count() - 1; i >= 0; i--)
			{
				spike_sel.spike_index = p_spike_list->get_spike_flag_array_at(i);
				const Spike* spike = p_dbwave_doc->get_spike(spike_sel);
				highlight_spike(spike);
			}
		}

		//display cursors
		if (m_HZtags.GetNTags() > 0)
			display_hz_tags(p_dc);
		if (m_VTtags.GetNTags() > 0)
			display_vt_tags(p_dc);
	}

	// restore resources
	p_dc->SetBkColor(saved_background_color);
	p_dc->RestoreDC(saved_dc);
	// restore selection to initial file
	if (m_display_all_files)
	{
		if(p_dbwave_doc->db_set_current_record_position(index_current_file))
			p_dbwave_doc->open_current_spike_file();
		p_spike_list = p_dbwave_doc->m_pSpk->get_spike_list_current();
	}
}

void ChartSpikeXY::display_spike(const Spike* spike, CDC* p_dc, const CRect& rect, const CRect& rect1, const long window_duration) const
{
	const auto l_spike_time = spike->get_time();
	if (m_range_mode == RANGE_TIMEINTERVALS
		&& (l_spike_time < m_lFirst || l_spike_time > m_lLast))
		return;

	// select correct brush
	const auto spike_class_id = spike->get_class_id();
	auto selected_brush = BLACK_COLOR;
	switch (m_plotmode)
	{
	case PLOT_ONECLASSONLY:
		if (spike_class_id != m_selected_class)
			return;
		break;
	case PLOT_CLASSCOLORS:
		selected_brush = spike_class_id % NB_COLORS;
		break;
	case PLOT_ONECLASS:
		if (spike_class_id != m_selected_class)
			selected_brush = SILVER_COLOR;
		else
			selected_brush = m_colorselected;
	default:
		break;
	}
	// adjust rectangle size
	CRect rect_i;
	if (spike_class_id == m_selected_class)
		rect_i = rect1;
	else
		rect_i = rect;

	// draw point
	const auto x1 = MulDiv(l_spike_time - m_lFirst, m_xVE, window_duration) + m_xVO;
	const auto y1 = MulDiv(spike->get_y1() - m_yWO, m_yVE, m_yWE) + m_yVO;
	rect_i.OffsetRect(x1, y1);
	p_dc->MoveTo(x1, y1);
	p_dc->FillSolidRect(&rect_i, m_colorTable[selected_brush]);
}

void ChartSpikeXY::display_vt_tags(CDC* p_dc)
{
	// select pen and display mode
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	const auto old_pen = p_dc->SelectObject(&m_blackDottedPen);
	m_xWO = m_lFirst;
	m_xWE = m_lLast - m_lFirst + 1;

	// iterate through VT cursor list
	const auto y0 = m_displayRect.top;
	const auto y1 = m_displayRect.bottom;
	for (auto j = m_VTtags.GetNTags() - 1; j >= 0; j--)
	{
		const auto val = m_VTtags.GetValue(j); // get value
		const auto pix_x = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
		p_dc->MoveTo(pix_x, y0); // set initial pt
		p_dc->LineTo(pix_x, y1); // VT line
	}
	p_dc->SelectObject(old_pen);
	p_dc->SetROP2(old_rop2);
}

void ChartSpikeXY::display_hz_tags(CDC* p_dc)
{
	const auto old_pen = p_dc->SelectObject(&m_blackDottedPen);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	const auto x1 = m_displayRect.left;
	const auto x2 = m_displayRect.right;
	for (auto i = m_HZtags.GetNTags() - 1; i >= 0; i--)
	{
		const auto k = m_HZtags.GetValue(i);
		const auto y1 = MulDiv(k - m_yWO, m_yVE, m_yWE) + m_yVO;
		p_dc->MoveTo(x1, y1);
		p_dc->LineTo(x2, y1);
	}
	p_dc->SelectObject(old_pen);
	p_dc->SetROP2(old_rop2);
}

void ChartSpikeXY::display_spike(const Spike* spike) 
{
	const auto spike_class = spike->get_class_id();
	int color_index = BLACK_COLOR;
	switch (m_plotmode)
	{
	case PLOT_ONECLASSONLY:
	case PLOT_ONECLASS:
		color_index = BLACK_COLOR; 
		if (spike_class != m_selected_class)
			color_index = SILVER_COLOR; 
		break;
	case PLOT_CLASSCOLORS:
		color_index = spike_class % 8;
		break;
	case PLOT_BLACK:
	default:
		break;
	}
	draw_spike(spike, color_index);
}

void ChartSpikeXY::draw_spike(const Spike* spike, const int color_index)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);

	const auto l_spike_time = spike->get_time();
	const auto window_duration = m_lLast - m_lFirst + 1;
	const auto x1 = MulDiv(l_spike_time - m_lFirst, m_xVE, window_duration) + m_xVO;
	const auto y1 = MulDiv(spike->get_y1() - m_yWO, m_yVE, m_yWE) + m_yVO;
	CRect rect(0, 0, m_dot_width, m_dot_width);
	rect.OffsetRect(x1 - m_dot_width / 2, y1 - m_dot_width / 2);

	const auto background_color = dc.GetBkColor();
	dc.MoveTo(x1, y1);
	dc.FillSolidRect(&rect, m_colorTable[color_index]);
	dc.SetBkColor(background_color);
}

void ChartSpikeXY::highlight_spike(const Spike* spike)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);

	const auto old_rop2 = dc.SetROP2(R2_NOTXORPEN);
	const auto l_spike_time = spike->get_time();
	const auto window_duration = m_lLast - m_lFirst + 1;
	const auto x1 = MulDiv(l_spike_time - m_lFirst, m_xVE, window_duration) + m_xVO;
	const auto y1 = MulDiv(spike->get_y1() - m_yWO, m_yVE, m_yWE) + m_yVO;

	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, 1, RGB(196, 2, 51));
	auto* old_pen = dc.SelectObject(&new_pen);
	auto* old_brush = dc.SelectStockObject(NULL_BRUSH);

	const auto width = m_dot_width * 2 / 3 + 2;
	CRect rect1(-width, -width, width, width);
	constexpr int delta = 0;
	rect1.OffsetRect(x1 - delta, y1 - delta);
	dc.Rectangle(&rect1);

	// restore resources
	dc.SelectObject(old_pen);
	dc.SelectObject(old_brush);
	dc.SetROP2(old_rop2);
}

void ChartSpikeXY::move_hz_tag(const int index, const int new_value)
{
	m_ptLast.y = MulDiv(m_HZtags.GetValue(index) - m_yWO, m_yVE, m_yWE) + m_yVO;
	const auto y_pixel = MulDiv(new_value - m_yWO, m_yVE, m_yWE) + m_yVO;
	XorHZtag(y_pixel);
	m_HZtags.SetTagVal(index, new_value);
}

void ChartSpikeXY::move_vt_tag(const int index, const int new_value)
{
	m_ptLast.x = MulDiv(m_VTtags.GetValue(index) - m_xWO, m_xVE, m_xWE) + m_xVO;
	const auto x_pixel = MulDiv(new_value - m_xWO, m_xVE, m_xWE) + m_xVO;
	XorVTtag(x_pixel);
	m_VTtags.SetTagVal(index, new_value);
}

void ChartSpikeXY::select_spike(const Spike_selected& new_spike_selected)
{
	if (spike_selected_.spike_index >= 0) {
		const Spike* spike = p_dbwave_doc->get_spike(spike_selected_);
		highlight_spike(spike);
		display_spike(spike);

	}

	spike_selected_ = new_spike_selected;
	if (spike_selected_.spike_index >= 0) {
		const Spike* spike = p_dbwave_doc->get_spike(spike_selected_);
		display_spike(spike);
		highlight_spike(spike);
	}
}

void ChartSpikeXY::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch (m_trackMode)
	{
	case TRACK_HZTAG:
		lbuttonUp_HzTag(nFlags, point);
		break;

	case TRACK_VTTAG:
		{
			// convert pix into data value
			const auto val = MulDiv(m_ptLast.x - m_xVO, m_xWE, m_xVE) + m_xWO;
			m_VTtags.SetTagVal(m_HCtrapped, val);
			point.x = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
			XorVTtag(point.x);
			ChartSpike::OnLButtonUp(nFlags, point);
			postMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
		}
		break;

	default:
		{
			ChartSpike::OnLButtonUp(nFlags, point);
			CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
			constexpr auto jitter = 3;
			if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
			{
				if (m_cursorType != CURSOR_ZOOM)
					postMyMessage(HINT_HITAREA, NULL);
				else
					zoomIn();
				return; // exit: mouse movement was too small
			}

			// perform action according to cursor type
			auto rect_in = m_displayRect;
			switch (m_cursorType)
			{
			case 0:
				rect_out = rect_in;
				rect_out.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
				ZoomData(&rect_in, &rect_out);
				break;
			case CURSOR_ZOOM: // zoom operation
				ZoomData(&rect_in, &rect_out);
				m_ZoomFrom = rect_in;
				m_ZoomTo = rect_out;
				m_iUndoZoom = 1;
				break;
			default:
				break;
			}
		}
		break;
	}
}

void ChartSpikeXY::OnLButtonDown(const UINT nFlags, const CPoint point)
{
	// compute pixel position of tags
	if (m_HZtags.GetNTags() > 0)
	{
		for (auto tag_index = m_HZtags.GetNTags() - 1; tag_index >= 0; tag_index--)
			m_HZtags.SetTagPix(tag_index, MulDiv(m_HZtags.GetValue(tag_index) - m_yWO, m_yVE, m_yWE) + m_yVO);
	}
	if (m_VTtags.GetNTags() > 0)
	{
		for (auto tag_index = m_VTtags.GetNTags() - 1; tag_index >= 0; tag_index--)
		{
			const auto val = m_VTtags.GetValue(tag_index);
			const auto pix = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
			m_VTtags.SetTagPix(tag_index, pix);
		}
	}

	// track rectangle or HZ tag?
	ChartSpike::OnLButtonDown(nFlags, point);
	if (m_currCursorMode != 0 || m_HCtrapped >= 0) // do nothing else if mode != 0
	{
		if (m_trackMode == TRACK_HZTAG || m_trackMode == TRACK_VTTAG)
			return; // or any tag hit (VT, HZ) detected
	}

	// test if mouse hit a spike
	m_hit_spike = hit_curve_in_doc(point);
	if (m_hit_spike >= 0)
	{
		// cancel track rect mode
		m_trackMode = TRACK_OFF; 
		releaseCursor(); // release cursor capture
		if (nFlags & MK_SHIFT)
			postMyMessage(HINT_HITSPIKE_SHIFT, m_hit_spike);

		else
			postMyMessage(HINT_HITSPIKE, m_hit_spike);
	}
}

//---------------------------------------------------------------------------
// ZoomData()
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo= window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we= y extent, ve=rect.height/2, vo = -rect.GetRectHeight()/2
//---------------------------------------------------------------------------

void ChartSpikeXY::ZoomData(CRect* rect_from, CRect* rect_dest)
{
	rect_from->NormalizeRect(); // make sure that rect is not inverted
	rect_dest->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_yWE; // save previous window extent
	m_yWE = MulDiv(m_yWE, rect_dest->Height(), rect_from->Height());
	m_yWO = m_yWO
		- MulDiv(rect_from->top - m_yVO, m_yWE, m_yVE)
		+ MulDiv(rect_dest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed
	auto l_size = m_lLast - m_lFirst + 1;
	m_lFirst = m_lFirst + l_size * (rect_dest->left - rect_from->left) / rect_from->Width();
	l_size = l_size * rect_dest->Width() / rect_from->Width();
	m_lLast = m_lFirst + l_size - 1;
	// display
	Invalidate();
	postMyMessage(HINT_CHANGEHZLIMITS, NULL);
}

void ChartSpikeXY::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (spike_selected_.spike_index < 0 || m_hit_spike < 0)
		ChartSpike::OnLButtonDblClk(nFlags, point);
	else
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),
		                         reinterpret_cast<LPARAM>(m_hWnd));
}

int ChartSpikeXY::hit_curve_in_doc(CPoint point)
{
	long n_files = 1;
	long current_file_index = 0;
	if (m_display_all_files)
	{
		n_files = p_dbwave_doc->db_get_n_records();
		current_file_index = p_dbwave_doc->db_get_current_record_position();
	}

	int result = -1;
	for (long file_index = 0; file_index < n_files; file_index++)
	{
		if (m_display_all_files)
		{
			if (p_dbwave_doc->db_set_current_record_position(file_index))
				p_dbwave_doc->open_current_spike_file();
			p_spike_list = p_dbwave_doc->m_pSpk->get_spike_list_current();
		}

		if (p_spike_list == nullptr || p_spike_list->get_spikes_count() == 0)
		{
			continue;
		}
		result = hitCurve(point);
		if (result >= 0) {
			current_file_index = file_index;
			break;
		}
	}

	if (m_display_all_files)
	{
		if (p_dbwave_doc->db_set_current_record_position(current_file_index))
			p_dbwave_doc->open_current_spike_file();
		p_spike_list = p_dbwave_doc->m_pSpk->get_spike_list_current();
	}

	return result;
}

int ChartSpikeXY::hitCurve(const CPoint point)
{
	// abscissa
	const auto time_frame = (m_lLast - m_lFirst + 1);
	x_max_ = m_lFirst + time_frame * (point.x + m_dot_width) / static_cast<long>(m_xVE);
	x_min_ = m_lFirst + time_frame * (point.x - m_dot_width) / static_cast<long>(m_xVE);
	// ordinates
	y_max_ = MulDiv(point.y - m_dot_width - m_yVO, m_yWE, m_yVE) + m_yWO;
	y_min_ = MulDiv(point.y + m_dot_width - m_yVO, m_yWE, m_yVE) + m_yWO;

	// first look at black spikes (foreground)
	const auto upperbound = p_spike_list->get_spikes_count() - 1;
	if (m_plotmode == PLOT_ONECLASS)
	{
		for (int spike_index = upperbound; spike_index >= 0; spike_index--)
		{
			const auto spike = p_spike_list->get_spike(spike_index);
			if (spike->get_class_id() == m_selected_class &&  is_spike_within_limits(spike))
				return spike_index;
		}
	}

	// then look through all other spikes
	for (int spike_index = upperbound; spike_index >= 0; spike_index--)
	{
		const auto spike = p_spike_list->get_spike(spike_index);
		if (is_spike_within_limits(spike))
			return spike_index;
	}

	// none found
	return -1;
}

boolean ChartSpikeXY::is_spike_within_limits (const Spike* spike) const
{
	const auto ii_time = spike->get_time();
	if (ii_time < x_min_ || ii_time > x_max_)
		return false;

	const auto val = spike->get_y1();
	if (val < y_min_ || val > y_max_)
		return false;
	return true;
}

void ChartSpikeXY::get_extents()
{
	if (m_yWE == 1) // && m_yWO == 0)
	{
		auto max_value = 4096;
		auto min_value = 0;
		if (p_spike_list != nullptr)
		{
			const auto upperbound = p_spike_list->get_spikes_count() - 1;
			if (upperbound >= 0)
			{
				max_value = p_spike_list->get_spike(upperbound)->get_y1();
				min_value = max_value;
				for (auto i = upperbound; i >= 0; i--)
				{
					const auto val = p_spike_list->get_spike(i)->get_y1();
					if (val > max_value) max_value = val;
					if (val < min_value) min_value = val;
				}
			}
		}
		m_yWE = max_value - min_value + 2;
		m_yWO = (max_value + min_value) / 2;
	}

	if (m_xVE == 1 && m_xVO == 0)
	{
		m_xVE = m_displayRect.Width();
		m_xVO = m_displayRect.left;
	}
	if (m_xWE == 1) // && m_xWO == 0)
	{
		m_xWE = m_displayRect.Width();
		m_xWO = m_displayRect.left;
	}
}
