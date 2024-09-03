#include "StdAfx.h"
#include "ChartSpikeBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_b_all_files is true: spike hit

IMPLEMENT_SERIAL(ChartSpikeBar, ChartSpike, 1)

BEGIN_MESSAGE_MAP(ChartSpikeBar, ChartSpike)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

ChartSpikeBar::ChartSpikeBar()
= default;

ChartSpikeBar::~ChartSpikeBar()
{
	if (p_envelope_ != nullptr)
	{
		p_envelope_->RemoveAll();
		delete p_envelope_;
	}
}

void ChartSpikeBar::plot_data_to_dc_prepare_dc(CDC* p_dc)
{
	if (b_erase_background_)
		erase_background(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	saved_dc_ = p_dc->SaveDC();
	saved_background_color_ = p_dc->GetBkColor();

	display_rect_ = client_rect_;
	saved_dc_ = p_dc->SaveDC();

	if (x_we_ == 1)
	{
		x_we_ = display_rect_.Width();
		x_wo_ = display_rect_.left;
	}
	p_dc->IntersectClipRect(&client_rect_);
}

void ChartSpikeBar::plot_data_to_dc(CDC* p_dc)
{
	plot_data_to_dc_prepare_dc(p_dc);

	auto n_files = 1;
	long index_current_file = 0;
	if (b_display_all_files_)
	{
		n_files = dbwave_doc_->db_get_records_count();
		index_current_file = dbwave_doc_->db_get_current_record_position();
	}

	for (long i_file = 0; i_file < n_files; i_file++)
	{
		if (!get_spike_list_from_file(i_file) && !b_display_all_files_)
		{
			display_text_bottom_left(p_dc, cs_empty_, col_dark_gray);
			continue;
		}

		if (b_bottom_comment)
			display_text_bottom_left(p_dc, cs_bottom_comment, col_blue);

		display_bars(p_dc, &display_rect_);

		if (p_spike_doc_ != nullptr && p_spike_doc_->m_stimulus_intervals.n_items > 0)
			display_stimulus(p_dc, &display_rect_);

		if (vt_tags.get_tag_list_size() > 0)
			display_vt_tags_long_values(p_dc);

		if (h_wnd_reflect_ != nullptr && temp_vertical_tag_ != nullptr)
			display_temporary_tag(p_dc);
	}

	// restore resources
	p_dc->SetBkColor(saved_background_color_);
	p_dc->RestoreDC(saved_dc_);

	// restore selection to initial file
	if (b_display_all_files_)
	{
		if (dbwave_doc_->db_set_current_record_position(index_current_file))
			dbwave_doc_->open_current_spike_file();
		if (dbwave_doc_->m_p_spk_doc != nullptr)
			p_spike_list_ = dbwave_doc_->m_p_spk_doc->get_spike_list_current();
	}
}

void ChartSpikeBar::display_temporary_tag(CDC* p_dc)
{
	const auto old_object = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	p_dc->MoveTo(temp_vertical_tag_->pixel, display_rect_.top + 2);
	p_dc->LineTo(temp_vertical_tag_->pixel, display_rect_.bottom - 2);
	p_dc->SetROP2(old_rop2);
	p_dc->SelectObject(old_object);
}

void ChartSpikeBar::display_vt_tags_long_values(CDC* p_dc)
{
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);

	const int y0 = display_rect_.top;
	const int y1 = display_rect_.bottom;
	for (auto tag_index = vt_tags.get_tag_list_size() - 1; tag_index >= 0; tag_index--)
	{
		Tag* p_tag = vt_tags.get_tag(tag_index);
		const auto lk = p_tag->value_long;
		if (lk < l_first_ || lk > l_last_)
			continue;
		p_tag->pixel = MulDiv(lk - l_first_, display_rect_.Width(), l_last_ - l_first_ + 1);
		p_dc->MoveTo(p_tag->pixel, y0);
		p_dc->LineTo(p_tag->pixel, y1);
	}
	p_dc->SetROP2(old_rop2);
	p_dc->SelectObject(old_pen);
}

void ChartSpikeBar::display_stimulus(CDC* p_dc, const CRect* rect) const
{
	CPen blue_pen;
	blue_pen.CreatePen(PS_SOLID, 0, RGB(0, 0, 255));
	const auto old_pen = p_dc->SelectObject(&blue_pen);

	const int top = rect->bottom - bar_height_ + 2;
	const int bottom = rect->bottom - 3;
	const auto display_width = rect->Width();

	// search first stimulus transition within interval
	const auto ii_first = l_first_;
	const auto ii_last = l_last_;
	const auto ii_length = ii_last - ii_first;
	auto i0 = 0;
	CIntervals* p_intervals = &p_spike_doc_->m_stimulus_intervals;

	while (i0 < p_intervals->get_size()
		&& p_intervals->get_at(i0) < ii_first)
		i0++; 

	auto state = bottom;	 
	const auto on_transition_index = (i0 / 2) * 2; 
	if (on_transition_index != i0)
		state = top;
	p_dc->MoveTo(rect->left, state);

	const auto n_stimuli = ((p_intervals->get_size()) / 2) * 2;
	for (auto ii = on_transition_index; ii < n_stimuli; ii += 2)
	{
		// stimulus starts here
		int iix0 = p_intervals->get_at(ii) - ii_first;
		if (iix0 >= ii_length) // first transition ON after last graph pt?
			break; 
		if (iix0 < 0) // first transition off graph?
			iix0 = 0; 

		iix0 = MulDiv(display_width, iix0, ii_length) + rect->left;
		p_dc->LineTo(iix0, state);	
		p_dc->LineTo(iix0, top);	

		// stimulus ends here
		state = bottom; // after pulse on, descend to bottom level
		int iix1 = p_intervals->get_at(ii + 1) - ii_first;
		if (iix1 > ii_length) // last transition off graph?
		{
			iix1 = ii_length; // yes = clip
			state = top; // do not descend
		}
		iix1 = MulDiv(display_width, iix1, ii_length) + rect->left + 1;
		p_dc->LineTo(iix1, top); 
		p_dc->LineTo(iix1, state); 
	}
	p_dc->LineTo(rect->left + display_width, state); 
	p_dc->SelectObject(old_pen);
}

void ChartSpikeBar::display_bars(CDC* p_dc, const CRect* rect)
{
	// prepare loop to display spikes
	auto* old_pen = static_cast<CPen*>(p_dc->SelectStockObject(BLACK_PEN));
	const long rect_width = rect->Width();
	if (y_we_ == 1)
	{
		int value_max, value_min;
		p_spike_list_->get_total_max_min(TRUE, &value_max, &value_min);
		y_we_ = value_max - value_min;
		y_wo_ = (value_max + value_min) / 2;
	}

	// draw horizontal line
	const int baseline = MulDiv(p_spike_list_->get_acq_bin_zero() - y_wo_, y_ve_, y_we_) + y_vo_;
	p_dc->MoveTo(rect->left, baseline);
	p_dc->LineTo(rect->right, baseline);

	// prepare text
	const auto h_font = CreateFont(10, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, _T("SYSTEM_FIXED_FONT"));
	const auto h_tmp = static_cast<HFONT>(p_dc->SelectObject(h_font));
	p_dc->SetBkMode(TRANSPARENT);

	// loop through all spikes of the list
	auto i_first = 0;
	auto i_last = p_spike_list_->get_spikes_count() - 1;
	if (range_mode_ == RANGE_INDEX)
	{
		if (index_last_spike_ > i_last)
			index_last_spike_ = i_last;
		if (index_first_spike_ < 0)
			index_first_spike_ = 0;
		i_last = index_last_spike_; 
		i_first = index_first_spike_;
	}
	const auto len = (l_last_ - l_first_ + 1);
	auto pen_color = BLACK_COLOR;

	for (auto i_spike = i_last; i_spike >= i_first; i_spike--)
	{
		const Spike* spike = p_spike_list_->get_spike(i_spike);
		const auto l_spike_time = spike->get_time();
		if (range_mode_ == RANGE_TIME_INTERVALS
			&& (l_spike_time < l_first_ || l_spike_time > l_last_))
			continue;

		// select correct pen
		const auto spike_class = spike->get_class_id();
		switch (plot_mode_)
		{
		case PLOT_ONE_CLASS_ONLY:
			if (spike_class != selected_class_)
				continue;
			break;
		case PLOT_CLASS_COLORS:
			pen_color = spike_class % nb_colors;
			break;
		case PLOT_ONE_CLASS:
			if (spike_class != selected_class_)
				pen_color = SILVER_COLOR;
			else
				pen_color = index_color_selected_;
		default:
			break;
		}
		p_dc->SelectObject(&pen_table_[pen_color]);

		// and draw spike: compute abscissa & draw from max to min
		const auto llk = MulDiv((l_spike_time - l_first_), rect_width, len);
		const int abscissa = static_cast<int>(llk) + rect->left;

		int max, min;
		p_spike_list_->get_spike(i_spike)->get_max_min(&max, &min);
		max = MulDiv(max - y_wo_, y_ve_, y_we_) + y_vo_;
		min = MulDiv(min - y_wo_, y_ve_, y_we_) + y_vo_;
		p_dc->MoveTo(abscissa, min);
		p_dc->LineTo(abscissa, max);

		if (spike_class >= 1)
		{
			p_dc->SetTextColor(color_spike_class[spike_class % nb_colors]);
			CString str;
			str.Format(_T("%i"), spike_class);
			p_dc->TextOut(abscissa, min, str, str.GetLength());
		}
	}

	// display selected spike
	if (spike_selected_.spike_index >= 0)
	{
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		display_spike(spike);
		highlight_spike(spike);
	}
	if (p_spike_list_->get_spike_flag_array_count() > 0)
		display_flagged_spikes(TRUE);

	p_dc->SelectObject(old_pen);
	DeleteObject(p_dc->SelectObject(h_tmp));


}

void ChartSpikeBar::display_flagged_spikes(const BOOL b_high_light)
{
	if (p_spike_list_->get_spike_flag_array_count() < 1)
		return;

	CClientDC dc(this);
	if (x_we_ == 1 || y_we_ == 1)
		return;
	x_we_ = display_rect_.Width();
	x_wo_ = display_rect_.left;

	dc.IntersectClipRect(&client_rect_);
	dc.SetMapMode(MM_ANISOTROPIC);
	prepare_dc(&dc);

	// loop over the array of flagged spikes
	const boolean is_selected_spike_in_this_list = 
		(dbwave_doc_->get_current_spike_file()->get_index_current_spike_list()
			== spike_selected_.spike_list_index);

	for (auto i = p_spike_list_->get_spike_flag_array_count() - 1; i >= 0; i--)
	{
		constexpr auto pen_size = 0;
		const auto spike_index_flagged = p_spike_list_->get_spike_index_of_flag(i);

		const Spike* spike = p_spike_list_->get_spike(spike_index_flagged);
		const auto no_spike_class = spike->get_class_id();
		if (PLOT_ONE_CLASS_ONLY == plot_mode_ && no_spike_class != selected_class_)
			continue;

		auto color_index = RED_COLOR;
		if (!b_high_light)
		{
			switch (plot_mode_)
			{
			case PLOT_ONE_CLASS_ONLY:
			case PLOT_ONE_CLASS:
				color_index = BLACK_COLOR;
				if (no_spike_class != selected_class_)
					color_index = SILVER_COLOR;
				break;
			case PLOT_CLASS_COLORS:
				if (is_selected_spike_in_this_list && spike_index_flagged == spike_selected_.spike_index)
					highlight_spike(spike);
				color_index = no_spike_class % nb_colors;
				break;
			case PLOT_BLACK:
			default:
				color_index = BLACK_COLOR;
				break;
			}
		}

		draw_spike(&dc, spike, color_spike_class_selected[color_index]);
	}
}

void ChartSpikeBar::display_spike(const Spike* spike)
{
	int color_index = BLACK_COLOR;
	const auto spike_class = spike->get_class_id();
	switch (plot_mode_)
	{
	case PLOT_ONE_CLASS_ONLY:
	case PLOT_ONE_CLASS:
		if (spike_class != selected_class_)
			color_index = SILVER_COLOR;
		break;
	case PLOT_CLASS_COLORS:
		color_index = spike_class % nb_colors;
		break;
	case PLOT_BLACK:
	default:
		color_index = BLACK_COLOR;
		break;
	}

	draw_spike(spike, color_spike_class[color_index]);
}

void ChartSpikeBar::draw_spike(const Spike* spike, const COLORREF& color)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&client_rect_);

	draw_spike(&dc, spike, color);
}

void ChartSpikeBar::draw_spike(CDC* p_dc,  const Spike* spike, const COLORREF& color) const
{
	CPen new_pen;
	constexpr auto pen_size = 0;
	new_pen.CreatePen(PS_SOLID, pen_size, color);
	const auto old_pen = p_dc->SelectObject(&new_pen);

	// display data
	const auto l_spike_time = spike->get_time();
	const auto len = l_last_ - l_first_ + 1;
	const auto llk = MulDiv(l_spike_time - l_first_, x_we_, len);
	const auto abscissa = static_cast<int>(llk) + x_wo_;
	int max, min;
	spike->get_max_min(&max, &min);
	p_dc->MoveTo(abscissa, max);
	p_dc->LineTo(abscissa, min);

	p_dc->SelectObject(old_pen);
}

void ChartSpikeBar::highlight_spike(const Spike* spike)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&client_rect_);

	const auto old_rop2 = dc.GetROP2();
	dc.SetROP2(R2_NOTXORPEN);
	draw_spike(&dc, spike, col_red);

	dc.SetROP2(old_rop2);
}

void ChartSpikeBar::select_spike(const db_spike& new_spike_selected)
{
	if (spike_selected_.spike_index >= 0)
	{
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		highlight_spike(spike);
	}

	spike_selected_ = new_spike_selected;
	if (spike_selected_.spike_index >= 0)
	{
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		highlight_spike(spike);
	}
}

// flag all spikes within a rectangle in screen coordinates
void ChartSpikeBar::select_spikes_within_rect(CRect* p_rect, const UINT n_flags) const
{
	// make sure that the rectangle is ok
	int i;
	if (p_rect->top > p_rect->bottom)
	{
		i = p_rect->top;
		p_rect->top = p_rect->bottom;
		p_rect->bottom = i;
	}
	if (p_rect->left > p_rect->right)
	{
		i = p_rect->left;
		p_rect->left = p_rect->right;
		p_rect->right = i;
	}

	const auto len = (l_last_ - l_first_ + 1);
	const auto l_first = MulDiv(p_rect->left, len, display_rect_.Width()) + l_first_;
	const auto l_last = MulDiv(p_rect->right, len, display_rect_.Width()) + l_first_;
	const auto v_min = MulDiv(p_rect->bottom - y_vo_, y_we_, y_ve_) + y_wo_;
	const auto v_max = MulDiv(p_rect->top - y_vo_, y_we_, y_ve_) + y_wo_;
	const auto b_flag = (n_flags & MK_SHIFT) || (n_flags & MK_CONTROL);
	p_spike_list_->flag_spikes_within_bounds(v_min, v_max, l_first, l_last, b_flag);
}

void ChartSpikeBar::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	if (!b_left_mouse_button_down_)
	{
		send_my_message(HINT_DROPPED, NULL); // post?
		return;
	}
	ChartSpike::OnLButtonUp(n_flags, point);

	CRect rect_out(pt_first_.x, pt_first_.y, pt_last_.x, pt_last_.y);
	constexpr auto jitter = 5;
	if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
	{
		if (cursor_type_ != CURSOR_ZOOM)
			send_my_message(HINT_HIT_AREA, NULL); // post?
		else
			zoom_in();
		return; // exit: mouse movement was too small
	}

	// perform action according to cursor type
	auto rect_in = display_rect_;
	switch (cursor_type_)
	{
	case 0:
		if (spike_hit_.spike_index < 0)
		{
			auto rect = get_defined_rect();
			select_spikes_within_rect(&rect, n_flags);
			send_my_message(HINT_SELECT_SPIKES, NULL); // post?
		}
		break;

	case CURSOR_ZOOM:
		zoom_data(&rect_in, &rect_out);
		rect_zoom_from_ = rect_in;
		rect_zoom_to_ = rect_out;
		i_undo_zoom_ = 1;
		send_my_message(HINT_SET_MOUSE_CURSOR, old_cursor_type_); // post?
		break;

	default:
		break;
	}
}

void ChartSpikeBar::OnLButtonDown(const UINT n_flags, CPoint point)
{
	b_left_mouse_button_down_ = TRUE;

	// detect bar hit: test if curve hit -- specific to SpikeBarButton
	if (current_cursor_mode_ == 0)
	{
		track_curve_ = FALSE;
		dbwave_doc_->set_spike_hit(spike_hit_ = hit_curve_in_doc(point));
		if (spike_hit_.spike_index >= 0)
		{
			if (n_flags & MK_SHIFT)
				send_my_message(HINT_HIT_SPIKE_SHIFT, NULL); // post?
			else if (n_flags & MK_CONTROL)
				send_my_message(HINT_HIT_SPIKE_CTRL, NULL); // post?
			else
				send_my_message(HINT_HIT_SPIKE, NULL); // post?
			return;
		}
	}
	ChartSpike::OnLButtonDown(n_flags, point);
}

//---------------------------------------------------------------------------
// zoom_data()
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo = window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=y_extent, ve=rect.height/2, vo = -rect.GetRectHeight()/2
//---------------------------------------------------------------------------

void ChartSpikeBar::zoom_data(CRect* prev_rect, CRect* new_rect)
{
	prev_rect->NormalizeRect();
	new_rect->NormalizeRect();

	// change y gain & y offset
	const auto y_we = y_we_;
	y_we_ = MulDiv(y_we_, new_rect->Height(), prev_rect->Height());
	y_wo_ = y_wo_
		- MulDiv(prev_rect->top - y_vo_, y_we_, y_ve_)
		+ MulDiv(new_rect->top - y_vo_, y_we, y_ve_);

	// change index of first and last pt displayed
	auto l_size = l_last_ - l_first_ + 1;
	l_first_ = l_first_ + l_size * (new_rect->left - prev_rect->left) / prev_rect->Width();
	if (l_first_ < 0)
		l_first_ = 0;
	l_size = l_size * new_rect->Width() / prev_rect->Width();
	l_last_ = l_first_ + l_size - 1;

	send_my_message(HINT_CHANGE_HZ_LIMITS, NULL); // post?
}

void ChartSpikeBar::OnLButtonDblClk(const UINT n_flags, const CPoint point)
{
	if ((spike_selected_.spike_index < 0 && p_spike_list_->get_spike_flag_array_count() < 1) || spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(n_flags, point);
	else
	{
		if (spike_selected_.spike_index >= 0)
		{
			send_my_message(HINT_DBL_CLK_SEL, spike_selected_.spike_index); // post?
		}
		else
		{
			const auto i_selected_spike = hit_curve(point);
			if (i_selected_spike >= 0)
				send_my_message(HINT_DBL_CLK_SEL, i_selected_spike); // post?
		}
	}
}

int ChartSpikeBar::hit_curve(const CPoint point)
{
	auto hit_spike = -1;
	// for y coordinates, conversion is straightforward:
	const auto mouse_y = MulDiv(point.y - y_vo_, y_we_, y_ve_) + y_wo_;
	const auto delta_y = MulDiv(3, y_we_, y_ve_);
	/*
	// for x coordinates, the formula is in 2 steps:
	// 1) time -> relative time: ii_time = (spike_time-m_lFirst) [-offset]
	// 2) relative time -> logical coordinate(LC): LC = ii_time* m_xWE / len + m_xWO
	// formula used to display: time (long) --> abscissa (int)
	// 		long len =  (m_lLast - m_lFirst + 1);
	// 		int abscissa = (int) (((lSpikeTime - m_lFirst) * (long) m_xWE) / len) + m_xWO;
	// reverse formula: abscissa -> time
	//		long lSpikeTime  = (abscissa - m_xWO)*len/m_xWE + m_lFirst;
	// convert device coordinates into logical coordinates
	*/
	const auto mouse_x = MulDiv(point.x - x_viewport_origin_, x_we_, x_viewport_extent_) + x_wo_;
	const auto delta_x = MulDiv(3, x_we_, x_viewport_extent_);
	const auto len_data_displayed = (l_last_ - l_first_ + 1);

	// find a spike which time of occurrence fits between l_X_max and l_X_min
	const auto x_max = l_first_ + MulDiv(len_data_displayed, mouse_x + delta_x, x_we_);
	const auto x_min = l_first_ + MulDiv(len_data_displayed, mouse_x - delta_x, x_we_);

	// loop through all spikes
	auto i_spike_first = 0;
	auto i_spike_last = p_spike_list_->get_spikes_count() - 1;
	if (range_mode_ == RANGE_INDEX)
	{
		if (index_last_spike_ > i_spike_last) index_last_spike_ = i_spike_last;
		if (index_first_spike_ < 0) index_first_spike_ = 0;
		i_spike_last = index_last_spike_;
		i_spike_first = index_first_spike_;
	}

	for (auto i_spike = i_spike_last; i_spike >= i_spike_first; i_spike--)
	{
		const Spike* spike = p_spike_list_->get_spike(i_spike);
		const auto l_spike_time = spike->get_time();
		if (l_spike_time < x_min || l_spike_time > x_max)
			continue;
		if (plot_mode_ == PLOT_ONE_CLASS_ONLY
			&& spike->get_class_id() != selected_class_)
			continue;

		int max, min;
		spike->get_max_min(&max, &min);
		if (mouse_y + delta_y < max && mouse_y - delta_y > min)
		{
			hit_spike = i_spike;
			break;
		}
	}

	return hit_spike;
}

void ChartSpikeBar::center_curve()
{
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() <= 0)
		return;
	int max, min;
	p_spike_list_->get_total_max_min(TRUE, &max, &min);
	y_wo_ = (max + min) / 2;
}

void ChartSpikeBar::max_gain()
{
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() <= 0)
		return;
	int max, min;
	p_spike_list_->get_total_max_min(TRUE, &max, &min);
	y_we_ = MulDiv(max - min + 1, 10, 8);
}

void ChartSpikeBar::max_center()
{
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() <= 0)
		return;
	int max, min;
	p_spike_list_->get_total_max_min(TRUE, &max, &min);
	y_we_ = MulDiv(max - min + 1, 10, 9);
	y_wo_ = (max  + min) / 2;
}

void ChartSpikeBar::print(CDC* p_dc, const CRect* rect)
{
	// check if there are valid data to display
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() == 0)
		return;

	// set mapping mode and viewport
	const auto n_saved_dc = p_dc->SaveDC(); // save display context
	display_bars(p_dc, rect);

	if (dbwave_doc_->m_p_spk_doc->m_stimulus_intervals.n_items > 0)
		display_stimulus(p_dc, rect);

	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeBar::Serialize(CArchive& archive)
{
	ChartSpike::Serialize(archive);

	auto dummy = TRUE;
	int dummy_int = 1;
	if (archive.IsStoring())
	{
		archive << range_mode_; 
		archive << l_first_;
		archive << l_last_; 
		archive << index_first_spike_; 
		archive << index_last_spike_; 
		archive << current_class_; 
		archive << dummy_int;
		archive << dummy_int; 
		archive << selected_class_;
		archive << track_curve_; 
		archive << dummy;
		archive << selected_pen_;
	}
	else
	{
		archive >> range_mode_; 
		archive >> l_first_;
		archive >> l_last_; 
		archive >> index_first_spike_;
		archive >> index_last_spike_; 
		archive >> current_class_;
		archive >> dummy_int; 
		archive >> dummy_int;
		archive >> selected_class_; 
		archive >> track_curve_;
		archive >> dummy;
		archive >> selected_pen_;
	}
}


