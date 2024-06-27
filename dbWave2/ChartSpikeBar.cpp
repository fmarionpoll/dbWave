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
{
	set_b_use_dib(FALSE);
	cs_empty_ = _T("no spikes (spikebar)");
}

ChartSpikeBar::~ChartSpikeBar()
{
	if (p_envelope_ != nullptr)
	{
		p_envelope_->RemoveAll();
		delete p_envelope_;
	}
}

void ChartSpikeBar::plot_data_to_dc(CDC* p_dc)
{
	if (b_erase_background_)
		erase_background(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_display_rect_;
	rect.DeflateRect(1, 1);

	// save context
	const auto n_saved_dc = p_dc->SaveDC();
	p_dc->IntersectClipRect(&m_client_rect_);

	long n_files = 1;
	long current_file = 0;
	if (display_all_files_)
	{
		n_files = dbwave_doc_->db_get_n_records();
		current_file = dbwave_doc_->db_get_current_record_position();
	}

	for (long i_file = 0; i_file < n_files; i_file++)
	{
		if (display_all_files_)
		{
			if (dbwave_doc_->db_set_current_record_position(i_file))
				p_spike_doc_ = dbwave_doc_->open_current_spike_file();
		}
		p_spike_list_ = p_spike_doc_->get_spike_list_current();

		// test presence of data
		if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() <= 0)
		{
			if (!display_all_files_)
				p_dc->DrawText(cs_empty_, cs_empty_.GetLength(), rect, DT_LEFT);
			continue;
		}

		// plot comment at the bottom
		if (b_bottom_comment)
			p_dc->DrawText(cs_bottom_comment, cs_bottom_comment.GetLength(), rect,DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

		// display data: trap error conditions
		if (m_y_we_ == 1)
		{
			short value_max, value_min;
			p_spike_list_->get_total_max_min(TRUE, &value_max, &value_min);
			m_y_we_ = value_max - value_min;
			m_y_wo_ = (value_max + value_min) / 2;
		}

		if (m_x_we_ == 1) // this is generally the case: && m_xWO == 0)
		{
			m_x_we_ = m_display_rect_.right;
			m_x_wo_ = m_display_rect_.left;
		}

		display_bars(p_dc, &m_display_rect_);

		const CIntervals* p_intervals = &(p_spike_doc_->m_stimulus_intervals);

		if (p_intervals->n_items > 0)
			display_stimulus(p_dc, &m_display_rect_);

		// display vertical cursors
		if (vertical_tags.get_tag_list_size() > 0)
		{
			// select pen and display mode
			const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
			const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);

			// iterate through VT cursor list
			const int y1 = m_display_rect_.bottom;
			for (auto j = vertical_tags.get_tag_list_size() - 1; j >= 0; j--)
			{
				constexpr auto y0 = 0;
				const auto lk = vertical_tags.get_tag_l_val(j); // get value
				if (lk < l_first_ || lk > l_last_)
					continue;
				const auto k = MulDiv(lk - l_first_, m_display_rect_.Width() , l_last_ - l_first_ + 1);
				p_dc->MoveTo(k, y0); // set initial pt
				p_dc->LineTo(k, y1); // VT line
			}
			p_dc->SetROP2(old_rop2); // restore old display mode
			p_dc->SelectObject(old_pen);
		}

		// temporary tag
		if (m_hwnd_reflect_ != nullptr && m_temp_vertical_tag_ != nullptr)
		{
			const auto old_object = p_dc->SelectObject(&black_dotted_pen_);
			const auto old_ROP2 = p_dc->SetROP2(R2_NOTXORPEN);
			p_dc->MoveTo(m_temp_vertical_tag_->m_pixel, m_display_rect_.top + 2);
			p_dc->LineTo(m_temp_vertical_tag_->m_pixel, m_display_rect_.bottom - 2);
			p_dc->SetROP2(old_ROP2);
			p_dc->SelectObject(old_object);
		}
	}

	p_dc->RestoreDC(n_saved_dc);

	if (display_all_files_)
	{
		if (dbwave_doc_->db_set_current_record_position(current_file))
			dbwave_doc_->open_current_spike_file();
		p_spike_list_ = dbwave_doc_->m_p_spk->get_spike_list_current();
	}
}

void ChartSpikeBar::plot_single_spk_data_to_dc(CDC* p_dc)
{
	if (b_erase_background_)
		erase_background(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_display_rect_;
	rect.DeflateRect(1, 1);

	// save context
	const auto n_saved_dc = p_dc->SaveDC();
	p_dc->IntersectClipRect(&m_client_rect_);

	// test presence of data
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() == 0)
	{
		p_dc->DrawText(cs_empty_, cs_empty_.GetLength(), rect, DT_LEFT);
		p_dc->RestoreDC(n_saved_dc);
		return;
	}

	// plot comment at the bottom
	if (b_bottom_comment)
	{
		p_dc->DrawText(cs_bottom_comment, cs_bottom_comment.GetLength(), rect, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
	}

	// display data: trap error conditions
	if (m_y_we_ == 1)
	{
		short value_max, value_min;
		p_spike_list_->get_total_max_min(TRUE, &value_max, &value_min);
		m_y_we_ = value_max - value_min;
		m_y_wo_ = (value_max + value_min) / 2;
	}

	if (m_x_we_ == 1) // this is generally the case: && m_xWO == 0)
	{
		m_x_we_ = m_display_rect_.right;
		m_x_wo_ = m_display_rect_.left;
	}

	display_bars(p_dc, &m_display_rect_);

	if (p_spike_doc_ == nullptr) {
		p_spike_doc_ = dbwave_doc_->m_p_spk;
		if (p_spike_doc_ == nullptr)
			return;
	}
	const CIntervals* p_intervals = &(p_spike_doc_->m_stimulus_intervals);

	if (p_intervals->n_items > 0)
		display_stimulus(p_dc, &m_display_rect_);

	// display vertical cursors
	if (vertical_tags.get_tag_list_size() > 0)
	{
		// select pen and display mode
		const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
		const auto old_ROP2 = p_dc->SetROP2(R2_NOTXORPEN);

		// iterate through VT cursor list
		const int y1 = m_display_rect_.bottom;
		for (auto j = vertical_tags.get_tag_list_size() - 1; j >= 0; j--)
		{
			constexpr auto y0 = 0;
			const auto lk = vertical_tags.get_tag_l_val(j);
			if (lk < l_first_ || lk > l_last_)
				continue;
			const auto k = MulDiv(lk - l_first_, m_display_rect_.Width(), l_last_ -l_first_ + 1);
			p_dc->MoveTo(k, y0);
			p_dc->LineTo(k, y1);
		}
		p_dc->SetROP2(old_ROP2);
		p_dc->SelectObject(old_pen);
	}

	// temp tag
	if (m_hwnd_reflect_ != nullptr && m_temp_vertical_tag_ != nullptr)
	{
		const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
		const auto old_ROP2 = p_dc->SetROP2(R2_NOTXORPEN);
		p_dc->MoveTo(m_temp_vertical_tag_->m_pixel, m_display_rect_.top + 2);
		p_dc->LineTo(m_temp_vertical_tag_->m_pixel, m_display_rect_.bottom - 2);
		p_dc->SetROP2(old_ROP2);
		p_dc->SelectObject(old_pen);
	}

	p_dc->RestoreDC(n_saved_dc);
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
	CIntervals* p_intervals = &(p_spike_doc_->m_stimulus_intervals);

	while (i0 < p_intervals->GetSize()
		&& p_intervals->GetAt(i0) < ii_first)
		i0++; 

	auto state = bottom;				// use this variable to keep track of pulse broken by display limits
	const auto jj = (i0 / 2) * 2;	// keep index of the ON transition
	if (jj != i0)
		state = top;
	p_dc->MoveTo(rect->left, state);

	const auto n_stimuli = ((p_intervals->GetSize()) / 2) * 2;
	for (auto ii = jj; ii < n_stimuli; ii += 2)
	{
		// stimulus starts here
		int iix0 = p_intervals->GetAt(ii) - ii_first;
		if (iix0 >= ii_length) // first transition ON after last graph pt?
			break; 
		if (iix0 < 0) // first transition off graph?
			iix0 = 0; 

		iix0 = MulDiv(display_width, iix0, ii_length) + rect->left;
		p_dc->LineTo(iix0, state);	// draw line up to the first point of the pulse
		p_dc->LineTo(iix0, top);	// draw vertical line to top of pulse

		// stimulus ends here
		state = bottom; // after pulse, descend to bottom level
		int iix1 = p_intervals->GetAt(ii + 1) - ii_first;
		if (iix1 > ii_length) // last transition off graph?
		{
			iix1 = ii_length; // yes = clip
			state = top; // do not descend..
		}
		iix1 = MulDiv(display_width, iix1, ii_length) + rect->left + 1;
		p_dc->LineTo(iix1, top); // draw top of pulse
		p_dc->LineTo(iix1, state); // draw descent to bottom line
	}
	p_dc->LineTo(rect->left + display_width, state); // end of loop - draw the rest
	p_dc->SelectObject(old_pen);
}

void ChartSpikeBar::display_bars(CDC* p_dc, const CRect* rect)
{
	// prepare loop to display spikes
	auto* old_pen = static_cast<CPen*>(p_dc->SelectStockObject(BLACK_PEN));
	const long rect_width = rect->Width();
	if (m_y_we_ == 1)
	{
		short value_max, value_min;
		p_spike_list_->get_total_max_min(TRUE, &value_max, &value_min);
		m_y_we_ = value_max - value_min;
		m_y_wo_ = (value_max + value_min) / 2;
	}
	const auto y_we = m_y_we_;
	const auto y_wo = m_y_wo_;
	const auto y_vo = rect->Height() / 2 + rect->top;
	const auto y_ve = -rect->Height();

	// draw horizontal line
	const int baseline = MulDiv(p_spike_list_->get_acq_bin_zero() - y_wo, y_ve, y_we) + y_vo;
	p_dc->MoveTo(rect->left, baseline);
	p_dc->LineTo(rect->right, baseline);

	// loop through all spikes of the list
	auto i_first = 0;
	auto i_last = p_spike_list_->get_spikes_count() - 1;
	if (range_mode_ == RANGE_INDEX)
	{
		if (index_last_spike_ > i_last)
			index_last_spike_ = i_last;
		if (index_first_spike_ < 0)
			index_first_spike_ = 0;
		i_last = index_last_spike_; // reduces the nb of spikes examined
		i_first = index_first_spike_; // assuming an ordered list...
	}
	const auto len = (l_last_ - l_first_ + 1);
	
	auto pen_color = BLACK_COLOR;
	for (auto i_spike = i_last; i_spike >= i_first; i_spike--)
	{
		const Spike* spike = p_spike_list_->get_spike(i_spike);

		// skip spike if outside time range && option
		const auto l_spike_time = spike->get_time();
		if (range_mode_ == RANGE_TIMEINTERVALS
			&& (l_spike_time < l_first_ || l_spike_time > l_last_))
			continue;

		// select correct pen
		const auto spike_class = spike->get_class_id();
		switch (plot_mode_)
		{
		case PLOT_ONECLASSONLY:
			if (spike_class != selected_class_)
				continue;
			break;
		case PLOT_CLASSCOLORS:
			pen_color = spike_class % NB_COLORS;
			break;
		case PLOT_ONECLASS:
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
		short max, min;
		p_spike_list_->get_spike(i_spike)->get_max_min(&max, &min);
		max = static_cast<short>(MulDiv(max - y_wo, y_ve, y_we) + y_vo);
		min = static_cast<short>(MulDiv(min - y_wo, y_ve, y_we) + y_vo);
		p_dc->MoveTo(abscissa, max);
		p_dc->LineTo(abscissa, min);

		if (spike_class >= 1)
		{
			TEXTMETRIC text_metric;
			if (p_dc->GetTextMetrics(&text_metric))
			{
				auto letter = _T('0');
				letter += spike_class;
				p_dc->TextOut(abscissa, min + (text_metric.tmHeight / 2), &letter, 1);
			}
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
}

void ChartSpikeBar::display_flagged_spikes(const BOOL b_high_light)
{
	if (p_spike_list_->get_spike_flag_array_count() < 1)
		return;

	CClientDC dc(this);
	if (m_x_we_ == 1 || m_y_we_ == 1)
		return;
	m_x_we_ = m_display_rect_.Width();
	m_x_wo_ = m_display_rect_.left;

	dc.IntersectClipRect(&m_client_rect_);
	prepare_dc(&dc);

	// loop over the array of flagged spikes
	const boolean is_selected_spike_in_this_list = 
		(dbwave_doc_->get_current_spike_file()->get_spike_list_current_index()
			== spike_selected_.spike_list_index);
	for (auto i = p_spike_list_->get_spike_flag_array_count() - 1; i >= 0; i--)
	{
		constexpr auto pen_size = 0;
		const auto no_spike = p_spike_list_->get_spike_flag_array_at(i);

		const Spike* spike = p_spike_list_->get_spike(no_spike);
		const auto no_spike_class = spike->get_class_id();
		if (PLOT_ONECLASSONLY == plot_mode_ && no_spike_class != selected_class_)
			continue;

		auto color_index = RED_COLOR;
		if (!b_high_light)
		{
			switch (plot_mode_)
			{
			case PLOT_ONECLASSONLY:
			case PLOT_ONECLASS:
				color_index = BLACK_COLOR;
				if (no_spike_class != selected_class_)
					color_index = SILVER_COLOR;
				break;
			case PLOT_CLASSCOLORS:
				if (is_selected_spike_in_this_list && no_spike == spike_selected_.spike_index)
					highlight_spike(spike);
				color_index = no_spike_class % NB_COLORS;
				break;
			case PLOT_BLACK:
			default:
				color_index = BLACK_COLOR;
				break;
			}
		}

		CPen new_pen;
		new_pen.CreatePen(PS_SOLID, pen_size, color_table_[color_index]);
		const auto old_pen = dc.SelectObject(&new_pen);

		// display data
		const auto l_spike_time = spike->get_time();
		const auto len = l_last_ - l_first_ + 1;
		const auto llk = MulDiv(l_spike_time - l_first_, m_x_we_, len);
		const auto abscissa = static_cast<int>(llk) + m_x_wo_;
		short max, min;
		spike->get_max_min(&max, &min);

		dc.MoveTo(abscissa, max);
		dc.LineTo(abscissa, min);
		dc.SelectObject(old_pen);
	}
}

void ChartSpikeBar::display_spike(const Spike* spike)
{
	int color_index = BLACK_COLOR;
	const auto spike_class = spike->get_class_id();
	switch (plot_mode_)
	{
	case PLOT_ONECLASSONLY:
	case PLOT_ONECLASS:
		if (spike_class != selected_class_)
			color_index = SILVER_COLOR;
		break;
	case PLOT_CLASSCOLORS:
		color_index = spike_class % NB_COLORS;
		break;
	case PLOT_BLACK:
	default:
		color_index = BLACK_COLOR;
		break;
	}

	draw_spike(spike, color_index);
}

void ChartSpikeBar::draw_spike(const Spike* spike, const int color_index)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&m_client_rect_);
	/*
	CClientDC dc(this);
	if (m_xWE == 1 || m_yWE == 1)
		return;
	m_xWE = m_displayRect.Width();
	m_xWO = m_displayRect.left;
	dc.IntersectClipRect(&m_clientRect);
	prepare_dc(&dc);
	*/
	CPen new_pen;
	constexpr auto pen_size = 0;
	new_pen.CreatePen(PS_SOLID, pen_size, color_table_[color_index]);
	const auto old_pen = dc.SelectObject(&new_pen);

	// display data
	const auto l_spike_time = spike->get_time();
	const auto len = l_last_ - l_first_ + 1;
	const auto llk = MulDiv(l_spike_time - l_first_, m_x_we_, len);
	const auto abscissa = static_cast<int>(llk) + m_x_wo_;
	short max, min;
	spike->get_max_min(&max, &min);

	dc.MoveTo(abscissa, max);
	dc.LineTo(abscissa, min);
	dc.SelectObject(old_pen);
}

void ChartSpikeBar::highlight_spike(const Spike* spike) 
{
	CClientDC dc(this);
	dc.IntersectClipRect(&m_client_rect_);

	const auto old_rop2 = dc.GetROP2();
	dc.SetROP2(R2_NOTXORPEN);

	const auto l_spike_time = spike->get_time();
	const auto len = l_last_ - l_first_ + 1;
	const auto llk = MulDiv(l_spike_time - l_first_, m_x_we_, len);
	const auto abscissa = static_cast<int>(llk) + m_x_wo_;

	const auto max = MulDiv(1 - m_y_vo_, m_y_we_, m_y_ve_) + m_y_wo_;
	const auto min = MulDiv(m_display_rect_.Height() - 2 - m_y_vo_, m_y_we_, m_y_ve_) + m_y_wo_;

	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, 1, RGB(196, 2, 51));
	const auto old_pen = dc.SelectObject(&new_pen);
	auto* old_brush = dc.SelectStockObject(NULL_BRUSH);
	const CRect rect1(abscissa - 2, max, abscissa + 2, min);
	dc.Rectangle(&rect1);

	// restore resources
	dc.SelectObject(old_pen);
	dc.SelectObject(old_brush);
	dc.SetROP2(old_rop2);
}

void ChartSpikeBar::select_spike(const db_spike& new_spike_selected)
{
	if (spike_selected_.spike_index >= 0) {
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		highlight_spike(spike);
		display_spike(spike);
	}

	spike_selected_ = new_spike_selected;
	if (spike_selected_.spike_index >= 0) {
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		display_spike(spike);
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
	const auto l_first = MulDiv(p_rect->left, len, m_display_rect_.Width()) + l_first_;
	const auto l_last = MulDiv(p_rect->right, len, m_display_rect_.Width()) + l_first_;
	const auto v_min = MulDiv(p_rect->bottom - m_y_vo_, m_y_we_, m_y_ve_) + m_y_wo_;
	const auto v_max = MulDiv(p_rect->top - m_y_vo_, m_y_we_, m_y_ve_) + m_y_wo_;
	const auto b_flag = (n_flags & MK_SHIFT) || (n_flags & MK_CONTROL);
	p_spike_list_->select_spikes_within_bounds(v_min, v_max, l_first, l_last, b_flag);
}

void ChartSpikeBar::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	if (!b_left_mouse_button_down_)
	{
		post_my_message(HINT_DROPPED, NULL);
		return;
	}
	ChartSpike::OnLButtonUp(n_flags, point);

	CRect rect_out(m_pt_first_.x, m_pt_first_.y, m_pt_last_.x, m_pt_last_.y);
	constexpr auto jitter = 5;
	if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
	{
		if (cursor_type_ != CURSOR_ZOOM)
			post_my_message(HINT_HITAREA, NULL);
		else
			zoom_in();
		return; // exit: mouse movement was too small
	}

	// perform action according to cursor type
	auto rect_in = m_display_rect_;
	switch (cursor_type_)
	{
	case 0:
		if (spike_hit_.spike_index < 0)
		{
			auto rect = get_defined_rect();
			select_spikes_within_rect(&rect, n_flags);
			post_my_message(HINT_SELECTSPIKES, NULL);
		}
		break;

	case CURSOR_ZOOM:
		zoom_data(&rect_in, &rect_out);
		rect_zoom_from_ = rect_in;
		rect_zoom_to_ = rect_out;
		i_undo_zoom_ = 1;
		post_my_message(HINT_SETMOUSECURSOR, old_cursor_type_);
		break;

	default:
		break;
	}
}

void ChartSpikeBar::OnLButtonDown(const UINT nFlags, CPoint point)
{
	b_left_mouse_button_down_ = TRUE;

	// detect bar hit: test if curve hit -- specific to SpikeBarButton
	if (current_cursor_mode_ == 0)
	{
		track_curve_ = FALSE;
		dbwave_doc_->set_spike_hit(spike_hit_ = hit_curve_in_doc(point));
		if (spike_hit_.spike_index >= 0)
		{
			if (nFlags & MK_SHIFT)
				post_my_message(HINT_HITSPIKE_SHIFT, NULL);
			else if (nFlags & MK_CONTROL)
				post_my_message(HINT_HITSPIKE_CTRL, NULL);
			else
				post_my_message(HINT_HITSPIKE, NULL);
			return;
		}
	}
	ChartSpike::OnLButtonDown(nFlags, point);
}

//---------------------------------------------------------------------------
// zoom_data()
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo= window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=y_extent, ve=rect.height/2, vo = -rect.GetRectHeight()/2
//---------------------------------------------------------------------------

void ChartSpikeBar::zoom_data(CRect* prev_rect, CRect* new_rect)
{
	prev_rect->NormalizeRect();
	new_rect->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_y_we_;
	m_y_we_ = MulDiv(m_y_we_, new_rect->Height(), prev_rect->Height());
	m_y_wo_ = m_y_wo_
		- MulDiv(prev_rect->top - m_y_vo_, m_y_we_, m_y_ve_)
		+ MulDiv(new_rect->top - m_y_vo_, y_we, m_y_ve_);

	// change index of first and last pt displayed
	auto l_size = l_last_ - l_first_ + 1;
	l_first_ = l_first_ + l_size * (new_rect->left - prev_rect->left) / prev_rect->Width();
	if (l_first_ < 0)
		l_first_ = 0;
	l_size = l_size * new_rect->Width() / prev_rect->Width();
	l_last_ = l_first_ + l_size - 1;

	post_my_message(HINT_CHANGEHZLIMITS, NULL);
}

void ChartSpikeBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((spike_selected_.spike_index < 0 && p_spike_list_->get_spike_flag_array_count() < 1) || spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(nFlags, point);
	else
	{
		if (spike_selected_.spike_index >= 0)
		{
			post_my_message(HINT_DBLCLKSEL, spike_selected_.spike_index);
		}
		else
		{
			const auto i_selected_spike = hit_curve(point);
			if (i_selected_spike >= 0)
				post_my_message(HINT_DBLCLKSEL, i_selected_spike);
		}
	}
}

int ChartSpikeBar::hit_curve(const CPoint point)
{
	auto hit_spike = -1;
	// for y coordinates, conversion is straightforward:
	const auto mouse_y = MulDiv(point.y - m_y_vo_, m_y_we_, m_y_ve_) + m_y_wo_;
	const auto delta_y = MulDiv(3, m_y_we_, m_y_ve_);
	/*
	// for x coordinates, the formula is in 2 steps:
	// 1) time -> relative time: ii_time = (spike_time-m_lFirst) [-offset]
	// 2) rel time -> logical coordinate(LC): LC = ii_time* m_xWE / len + m_xWO
	// formula used to display: time (long) --> abscissa (int)
	// 		long len =  (m_lLast - m_lFirst + 1);
	// 		int abscissa = (int) (((lSpikeTime - m_lFirst) * (long) m_xWE) / len) + m_xWO;
	// reverse formula: abscissa -> time
	//		long lSpikeTime  = (abscissa - m_xWO)*len/m_xWE + m_lFirst;
	// convert device coordinates into logical coordinates
	*/
	const auto mouse_x = MulDiv(point.x - m_x_vo_, m_x_we_, m_x_ve_) + m_x_wo_;
	const auto delta_x = MulDiv(3, m_x_we_, m_x_ve_);
	const auto len_data_displayed = (l_last_ - l_first_ + 1);

	// find a spike which time of occurrence fits between l_X_max and l_X_min
	const auto x_max = l_first_ + MulDiv(len_data_displayed, mouse_x + delta_x, m_x_we_);
	const auto x_min = l_first_ + MulDiv(len_data_displayed, mouse_x - delta_x, m_x_we_);

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
		if (plot_mode_ == PLOT_ONECLASSONLY
			&& spike->get_class_id() != selected_class_)
			continue;

		short max, min;
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
	short max, min;
	p_spike_list_->get_total_max_min(TRUE, &max, &min);
	m_y_wo_ = max / 2 + min / 2;
}

void ChartSpikeBar::max_gain()
{
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() <= 0)
		return;
	short max, min;
	p_spike_list_->get_total_max_min(TRUE, &max, &min);
	m_y_we_ = MulDiv(max - min + 1, 10, 8);
}

void ChartSpikeBar::max_center()
{
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() <= 0)
		return;
	short max, min;
	p_spike_list_->get_total_max_min(TRUE, &max, &min);
	
	m_y_we_ = MulDiv(max - min + 1, 10, 8);
	m_y_wo_ = max / 2 + min / 2;
}

void ChartSpikeBar::print(CDC* p_dc, const CRect* rect)
{
	// check if there are valid data to display
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() == 0)
		return;

	// set mapping mode and viewport
	const auto n_saved_dc = p_dc->SaveDC(); // save display context
	display_bars(p_dc, rect);

	if (dbwave_doc_->m_p_spk->m_stimulus_intervals.n_items > 0)
		display_stimulus(p_dc, rect);

	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeBar::Serialize(CArchive& ar)
{
	ChartSpike::Serialize(ar);

	auto dummy = TRUE;
	int dummy_int = 1;
	if (ar.IsStoring())
	{
		ar << range_mode_; // display range (time OR storage index)
		ar << l_first_; // time index of first pt displayed
		ar << l_last_; // time index of last pt displayed
		ar << index_first_spike_; // index first spike
		ar << index_last_spike_; // index last spike
		ar << current_class_; // current class in case of displaying classes
		ar << dummy_int;
		ar << dummy_int; // no of spike selected
		ar << selected_class_; // index class selected
		ar << track_curve_; // track curve ?
		ar << dummy;
		ar << selected_pen_;
	}
	else
	{
		ar >> range_mode_; // display range (time OR storage index)
		ar >> l_first_; // time index of first pt displayed
		ar >> l_last_; // time index of last pt displayed
		ar >> index_first_spike_; // index first spike
		ar >> index_last_spike_; // index last spike
		ar >> current_class_; // current class in case of displaying classes
		ar >> dummy_int; 
		ar >> dummy_int; // no of spike selected
		ar >> selected_class_; // index class selected
		ar >> track_curve_; // track curve ?
		ar >> dummy;
		ar >> selected_pen_;
	}
}


