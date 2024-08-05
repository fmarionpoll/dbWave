
#include "StdAfx.h"
#include "ChartSpikeXY.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_b_all_files is true: display and spike hit

BEGIN_MESSAGE_MAP(ChartSpikeXY, ChartSpike)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

ChartSpikeXY::ChartSpikeXY()
{
	set_b_use_dib(FALSE);
	cs_empty_ = _T("no spikes (spikexp)");
}

ChartSpikeXY::~ChartSpikeXY()
= default;

void ChartSpikeXY::plot_data_to_dc(CDC* p_dc)
{
	if (b_erase_background_)
		erase_background(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_display_rect_;
	rect.DeflateRect(1, 1);

	// save context
	const auto saved_dc = p_dc->SaveDC();
	const auto saved_background_color = p_dc->GetBkColor();

	// display data: trap error conditions
	const auto window_duration = l_last_ - l_first_ + 1; 
	get_extents(); 
	ASSERT(m_x_we_ != 1);
	p_dc->SetMapMode(MM_TEXT);

	// prepare brush & rectangles (small for all spikes, larger for spikes belonging to the selected class)
	auto width = dot_width_ / 2;
	const CRect rect0(-width, -width, width, width);
	width = dot_width_ * 2 / 3;
	const CRect rect1(-width, -width, width, width);

	long n_files = 1;
	long index_current_file = 0;
	if (display_all_files_)
	{
		n_files = dbwave_doc_->db_get_n_records();
		index_current_file = dbwave_doc_->db_get_current_record_position();
	}

	for (long index_file = 0; index_file < n_files; index_file++)
	{
		if (display_all_files_)
		{
			if (dbwave_doc_->db_set_current_record_position(index_file))
				dbwave_doc_->open_current_spike_file();
		}
		if (dbwave_doc_->m_p_spk == nullptr)
			continue;

		p_spike_list_ = dbwave_doc_->m_p_spk->get_spike_list_current();

		// test presence of data
		if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() == 0)
		{
			if (!display_all_files_)
				p_dc->DrawText(cs_empty_, cs_empty_.GetLength(), rect, DT_LEFT);
			continue;
		}

		auto last_spike_index = p_spike_list_->get_spikes_count() - 1;
		auto first_spike_index = 0;
		if (range_mode_ == RANGE_INDEX)
		{
			if (index_last_spike_ > last_spike_index)
				index_last_spike_ = last_spike_index;
			if (index_first_spike_ < 0)
				index_first_spike_ = 0;
			last_spike_index = index_last_spike_;
			first_spike_index = index_first_spike_;
		}

		// loop over all spikes
		for (auto spike_index = last_spike_index; spike_index >= first_spike_index; spike_index--)
		{
			display_spike(p_spike_list_->get_spike(spike_index), p_dc, rect0, rect1, window_duration);
		}

		// display spike selected
		if (spike_selected_.database_position == index_file && spike_selected_.spike_index >= 0)
		{
			const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
			highlight_spike(spike);
		}

		if (p_spike_list_->get_spike_flag_array_count() > 0)
		{
			// loop over the array of flagged spikes
			auto spike_sel = db_spike (index_file, dbwave_doc_->m_p_spk->get_spike_list_current_index(), 0);
			for (auto i = p_spike_list_->get_spike_flag_array_count() - 1; i >= 0; i--)
			{
				spike_sel.spike_index = p_spike_list_->get_spike_flag_array_at(i);
				const Spike* spike = dbwave_doc_->get_spike(spike_sel);
				highlight_spike(spike);
			}
		}

		////display cursors
		//if (horizontal_tags.get_tag_list_size() > 0)
		//	display_hz_tags(p_dc);
		//if (vertical_tags.get_tag_list_size() > 0)
		//	display_vt_tags(p_dc);
	}

	// restore resources
	p_dc->SetBkColor(saved_background_color);
	p_dc->RestoreDC(saved_dc);

	//display cursors
	if (horizontal_tags.get_tag_list_size() > 0)
		display_hz_tags(p_dc);
	if (vertical_tags.get_tag_list_size() > 0)
		display_vt_tags(p_dc);

	// restore selection to initial file
	if (display_all_files_)
	{
		if(dbwave_doc_->db_set_current_record_position(index_current_file))
			dbwave_doc_->open_current_spike_file();
		if (dbwave_doc_->m_p_spk != nullptr)
			p_spike_list_ = dbwave_doc_->m_p_spk->get_spike_list_current();
	}
}

void ChartSpikeXY::display_spike(const Spike* spike, CDC* p_dc, const CRect& rect, const CRect& rect1, const long window_duration) const
{
	const auto l_spike_time = spike->get_time();
	if (range_mode_ == RANGE_TIME_INTERVALS
		&& (l_spike_time < l_first_ || l_spike_time > l_last_))
		return;

	// select correct brush
	const auto spike_class_id = spike->get_class_id();
	auto selected_brush = BLACK_COLOR;
	switch (plot_mode_)
	{
	case PLOT_ONE_CLASS_ONLY:
		if (spike_class_id != selected_class_)
			return;
		break;
	case PLOT_CLASS_COLORS:
		selected_brush = spike_class_id % NB_COLORS;
		break;
	case PLOT_ONE_CLASS:
		if (spike_class_id != selected_class_)
			selected_brush = SILVER_COLOR;
		else
			selected_brush = index_color_selected_;
	default:
		break;
	}
	// adjust rectangle size
	CRect rect_i;
	if (spike_class_id == selected_class_)
		rect_i = rect1;
	else
		rect_i = rect;

	// draw point
	const auto x1 = MulDiv(l_spike_time - l_first_, m_x_viewport_extent_, window_duration) + m_x_viewport_origin_;
	const auto y1 = MulDiv(spike->get_y1() - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_;
	rect_i.OffsetRect(x1, y1);
	p_dc->MoveTo(x1, y1);
	p_dc->FillSolidRect(&rect_i, color_table_[selected_brush]);
}

void ChartSpikeXY::display_vt_tags(CDC* p_dc)
{
	// select pen and display mode
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	m_x_wo_ = l_first_;
	m_x_we_ = l_last_ - l_first_ + 1;

	// iterate through VT cursor list
	const auto y0 = m_display_rect_.top;
	const auto y1 = m_display_rect_.bottom;
	for (auto j = vertical_tags.get_tag_list_size() - 1; j >= 0; j--)
	{
		const auto val = vertical_tags.get_value_int(j); 
		const auto pix_x = MulDiv(val - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
		p_dc->MoveTo(pix_x, y0); // set initial pt
		p_dc->LineTo(pix_x, y1); // VT line
	}
	p_dc->SelectObject(old_pen);
	p_dc->SetROP2(old_rop2);
}

void ChartSpikeXY::display_hz_tags(CDC* p_dc)
{
	const auto old_pen = p_dc->SelectObject(&black_dotted_pen_);
	const auto old_rop2 = p_dc->SetROP2(R2_NOTXORPEN);

	const auto x_left = m_x_wo_; // m_display_rect_.left;
	const auto x_right = m_x_we_; // m_display_rect_.right;
	for (auto i = horizontal_tags.get_tag_list_size() - 1; i >= 0; i--)
	{
		const auto k = horizontal_tags.get_value_int(i);
		const auto y = MulDiv(k - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_;
		p_dc->MoveTo(x_left, y);
		p_dc->LineTo(x_right, y);
	}

	p_dc->SelectObject(old_pen);
	p_dc->SetROP2(old_rop2);
}

void ChartSpikeXY::display_spike(const Spike* spike) 
{
	const auto spike_class = spike->get_class_id();
	int color_index = BLACK_COLOR;
	switch (plot_mode_)
	{
	case PLOT_ONE_CLASS_ONLY:
	case PLOT_ONE_CLASS:
		color_index = BLACK_COLOR; 
		if (spike_class != selected_class_)
			color_index = SILVER_COLOR; 
		break;
	case PLOT_CLASS_COLORS:
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
	dc.IntersectClipRect(&m_client_rect_);

	const auto l_spike_time = spike->get_time();
	const auto window_duration = l_last_ - l_first_ + 1;
	const auto x1 = MulDiv(l_spike_time - l_first_, m_x_viewport_extent_, window_duration) + m_x_viewport_origin_;
	const auto y1 = MulDiv(spike->get_y1() - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_;
	CRect rect(0, 0, dot_width_, dot_width_);
	rect.OffsetRect(x1 - dot_width_ / 2, y1 - dot_width_ / 2);

	const auto background_color = dc.GetBkColor();
	dc.MoveTo(x1, y1);
	dc.FillSolidRect(&rect, color_table_[color_index]);
	dc.SetBkColor(background_color);
}

void ChartSpikeXY::highlight_spike(const Spike* spike)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&m_client_rect_);

	const auto old_rop2 = dc.SetROP2(R2_NOTXORPEN);
	const auto l_spike_time = spike->get_time();
	const auto window_duration = l_last_ - l_first_ + 1;
	const auto x1 = MulDiv(l_spike_time - l_first_, m_x_viewport_extent_, window_duration) + m_x_viewport_origin_;
	const auto y1 = MulDiv(spike->get_y1() - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_;

	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, 1, RGB(196, 2, 51));
	auto* old_pen = dc.SelectObject(&new_pen);
	auto* old_brush = dc.SelectStockObject(NULL_BRUSH);

	const auto width = dot_width_ * 2 / 3 + 2;
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
	m_pt_last_.y = MulDiv(horizontal_tags.get_value_int(index) - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_;
	const auto y_pixel = MulDiv(new_value - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_;
	xor_horizontal_tag(y_pixel);
	horizontal_tags.set_value_int(index, new_value);
}

void ChartSpikeXY::move_vt_tag(const int index, const int new_value)
{
	m_pt_last_.x = MulDiv(vertical_tags.get_value_int(index) - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
	const auto x_pixel = MulDiv(new_value - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
	xor_vertical_tag(x_pixel);
	vertical_tags.set_value_int(index, new_value);
}

void ChartSpikeXY::select_spike(const db_spike& new_spike_selected)
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

void ChartSpikeXY::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch (track_mode_)
	{
	case TRACK_HZ_TAG:
		left_button_up_horizontal_tag(nFlags, point);
		break;

	case TRACK_VT_TAG:
		{
			// convert pix into data value
			const auto val = MulDiv(m_pt_last_.x - m_x_viewport_origin_, m_x_we_, m_x_viewport_extent_) + m_x_wo_;
			vertical_tags.set_value_int(hc_trapped_, val);
			point.x = MulDiv(val - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
			xor_vertical_tag(point.x);
			ChartSpike::OnLButtonUp(nFlags, point);
			post_my_message(HINT_CHANGE_VERT_TAG, hc_trapped_);
		}
		break;

	default:
		{
			ChartSpike::OnLButtonUp(nFlags, point);
			CRect rect_out(m_pt_first_.x, m_pt_first_.y, m_pt_last_.x, m_pt_last_.y);
			constexpr auto jitter = 3;
			if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
			{
				if (cursor_type_ != CURSOR_ZOOM)
					post_my_message(HINT_HIT_AREA, NULL);
				else
					zoom_in();
				return; // exit: mouse movement was too small
			}

			// perform action according to cursor type
			auto rect_in = m_display_rect_;
			switch (cursor_type_)
			{
			case 0:
				rect_out = rect_in;
				rect_out.OffsetRect(m_pt_first_.x - m_pt_last_.x, m_pt_first_.y - m_pt_last_.y);
				zoom_data(&rect_in, &rect_out);
				break;
			case CURSOR_ZOOM: // zoom operation
				zoom_data(&rect_in, &rect_out);
				rect_zoom_from_ = rect_in;
				rect_zoom_to_ = rect_out;
				i_undo_zoom_ = 1;
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
	if (horizontal_tags.get_tag_list_size() > 0)
	{
		for (auto tag_index = horizontal_tags.get_tag_list_size() - 1; tag_index >= 0; tag_index--)
			horizontal_tags.set_pixel(tag_index, MulDiv(horizontal_tags.get_value_int(tag_index) - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_);
	}
	if (vertical_tags.get_tag_list_size() > 0)
	{
		for (auto tag_index = vertical_tags.get_tag_list_size() - 1; tag_index >= 0; tag_index--)
		{
			const auto val = vertical_tags.get_value_int(tag_index);
			const auto pix = MulDiv(val - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
			vertical_tags.set_pixel(tag_index, pix);
		}
	}

	// track rectangle or HZ tag?
	ChartSpike::OnLButtonDown(nFlags, point);
	if (current_cursor_mode_ != 0 || hc_trapped_ >= 0) // do nothing else if mode != 0
	{
		if (track_mode_ == TRACK_HZ_TAG || track_mode_ == TRACK_VT_TAG)
			return; 
	}

	// test if mouse hit a spike
	dbwave_doc_->set_spike_hit(spike_hit_ = hit_curve_in_doc(point));
	if (spike_hit_.spike_index >= 0)
	{
		release_cursor(); 
		if (nFlags & MK_SHIFT)
			post_my_message(HINT_HIT_SPIKE_SHIFT, NULL);

		else
			post_my_message(HINT_HIT_SPIKE, NULL);
	}
}

//---------------------------------------------------------------------------
// zoom_data()
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// with:
// wo = window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we= y extent, ve=rect.height/2, vo = -rect.GetRectHeight()/2
//---------------------------------------------------------------------------

void ChartSpikeXY::zoom_data(CRect* rect_from, CRect* rect_dest)
{
	rect_from->NormalizeRect(); // make sure that rect is not inverted
	rect_dest->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_y_we_; // save previous window extent
	m_y_we_ = MulDiv(m_y_we_, rect_dest->Height(), rect_from->Height());
	m_y_wo_ = m_y_wo_
		- MulDiv(rect_from->top - m_y_viewport_origin_, m_y_we_, m_y_viewport_extent_)
		+ MulDiv(rect_dest->top - m_y_viewport_origin_, y_we, m_y_viewport_extent_);

	// change index of first and last pt displayed
	auto l_size = l_last_ - l_first_ + 1;
	l_first_ = l_first_ + l_size * (rect_dest->left - rect_from->left) / rect_from->Width();
	l_size = l_size * rect_dest->Width() / rect_from->Width();
	l_last_ = l_first_ + l_size - 1;
	// display
	Invalidate();
	post_my_message(HINT_CHANGE_HZ_LIMITS, NULL);
}

void ChartSpikeXY::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	/*if (spike_selected_.spike_index < 0 || spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(nFlags, point);
	else
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),
		                         reinterpret_cast<LPARAM>(m_hWnd));*/
	if ((spike_selected_.spike_index < 0 && p_spike_list_->get_spike_flag_array_count() < 1) || spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(nFlags, point);
	else
	{
		if (spike_selected_.spike_index >= 0)
		{
			post_my_message(HINT_DBL_CLK_SEL, spike_selected_.spike_index);
		}
		else
		{
			const auto selected_spike = hit_curve(point);
			if (selected_spike > 0)
				post_my_message(HINT_DBL_CLK_SEL, selected_spike);
		}
	}
}

int ChartSpikeXY::hit_curve(const CPoint point)
{
	// abscissa
	const auto time_frame = (l_last_ - l_first_ + 1);
	x_max_ = l_first_ + time_frame * (point.x + dot_width_) / static_cast<long>(m_x_viewport_extent_);
	x_min_ = l_first_ + time_frame * (point.x - dot_width_) / static_cast<long>(m_x_viewport_extent_);
	// ordinates
	y_max_ = MulDiv(point.y - dot_width_ - m_y_viewport_origin_, m_y_we_, m_y_viewport_extent_) + m_y_wo_;
	y_min_ = MulDiv(point.y + dot_width_ - m_y_viewport_origin_, m_y_we_, m_y_viewport_extent_) + m_y_wo_;

	// first look at black spikes (foreground)
	const auto upperbound = p_spike_list_->get_spikes_count() - 1;
	if (plot_mode_ == PLOT_ONE_CLASS)
	{
		for (int spike_index = upperbound; spike_index >= 0; spike_index--)
		{
			const auto spike = p_spike_list_->get_spike(spike_index);
			if (spike->get_class_id() == selected_class_ &&  is_spike_within_limits(spike))
				return spike_index;
		}
	}

	// then look through all other spikes
	for (int spike_index = upperbound; spike_index >= 0; spike_index--)
	{
		const auto spike = p_spike_list_->get_spike(spike_index);
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
	if (m_y_we_ == 1) // && m_yWO == 0)
	{
		auto max_value = 4096;
		auto min_value = 0;
		if (p_spike_list_ != nullptr)
		{
			const auto upperbound = p_spike_list_->get_spikes_count() - 1;
			if (upperbound >= 0)
			{
				max_value = p_spike_list_->get_spike(upperbound)->get_y1();
				min_value = max_value;
				for (auto i = upperbound; i >= 0; i--)
				{
					const auto val = p_spike_list_->get_spike(i)->get_y1();
					if (val > max_value) max_value = val;
					if (val < min_value) min_value = val;
				}
			}
		}
		m_y_we_ = max_value - min_value + 2;
		m_y_wo_ = (max_value + min_value) / 2;
	}

	if (m_x_viewport_extent_ == 1 && m_x_viewport_origin_ == 0)
	{
		m_x_viewport_extent_ = m_display_rect_.Width();
		m_x_viewport_origin_ = m_display_rect_.left;
	}
	if (m_x_we_ == 1) // && m_xWO == 0)
	{
		m_x_we_ = m_display_rect_.Width();
		m_x_wo_ = m_display_rect_.left;
	}
}
