#include "StdAfx.h"
#include "ChartSpikeShape.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_b_all_files is true: spike hit

IMPLEMENT_SERIAL(ChartSpikeShape, ChartSpike, 1)

BEGIN_MESSAGE_MAP(ChartSpikeShape, ChartSpike)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

ChartSpikeShape::ChartSpikeShape()
{
	set_b_use_dib(FALSE);
	cs_empty_ = _T("no spikes (spikeshape)");
}

ChartSpikeShape::~ChartSpikeShape()
= default;

void ChartSpikeShape::plot_data_to_dc(CDC * p_dc)
{
	if (b_erase_background_)
		erase_background(p_dc);

	const auto n_saved_dc = p_dc->SaveDC();
	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = m_display_rect_;
	rect.DeflateRect(1, 1);
	get_extents();
	prepare_dc(p_dc);

	auto n_files = 1;
	if (display_all_files_)
		n_files = dbwave_doc_->db_get_n_records();

	for (auto i_file = 0; i_file < n_files; i_file++)
	{
		if (display_all_files_)
		{
			if (dbwave_doc_->db_set_current_record_position(i_file))
				dbwave_doc_->open_current_spike_file();
			else
				continue;
		}
		if (dbwave_doc_->m_p_spk == nullptr)
			continue;

		p_spike_list_ = dbwave_doc_->m_p_spk->get_spike_list_current();

		//test if data are there - if none, display message and exit
		if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() == 0)
		{
			if (!display_all_files_)
			{
				p_dc->DrawText(cs_empty_, cs_empty_.GetLength(), rect, DT_LEFT);
				return;
			}
			continue;
		}

		// load resources and prepare context
		const auto spike_length = p_spike_list_->get_spike_length();
		ASSERT(spike_length > 0);
		if (polyline_points_.GetSize() != spike_length)
		{
			polyline_points_.SetSize(spike_length, 2);
			init_polypoint_x_axis();
		}

		// loop through all spikes of the list
		short* p_spike_data;
		auto i_last_spike = p_spike_list_->get_spikes_count() - 1;
		auto i_first_spike = 0;
		if (range_mode_ == RANGE_INDEX)
		{
			i_last_spike = index_last_spike_;
			i_first_spike = index_first_spike_;
		}
		auto selected_pen_color = BLACK_COLOR;
		if (plot_mode_ == PLOT_ONE_CLASS || plot_mode_ == PLOT_ONE_COLOR)
			selected_pen_color = SILVER_COLOR;
		const auto old_pen = p_dc->SelectObject(&pen_table_[selected_pen_color]);

		for (auto i_spike = i_last_spike; i_spike >= i_first_spike; i_spike--)
		{
			const Spike* spike = p_spike_list_->get_spike(i_spike);

			// skip spike ?
			if (range_mode_ == RANGE_TIME_INTERVALS
				&& (spike->get_time() < l_first_ || spike->get_time() > l_last_))
				continue;

			// select pen according to class
			const auto spike_class = spike->get_class_id();
			switch (plot_mode_)
			{
			case PLOT_ONE_CLASS_ONLY:
				if (spike_class != selected_class_)
					continue;
				break;
			case PLOT_CLASS_COLORS:
				selected_pen_color = spike_class % NB_COLORS;
				p_dc->SelectObject(&pen_table_[selected_pen_color]);
				break;
			case PLOT_ONE_CLASS:
				if (spike_class == selected_class_)
					continue;
			default:
				break;
			}

			// display data
			p_spike_data = spike->get_p_data();
			fill_polypoint_y_axis(p_spike_data);
			p_dc->Polyline(&polyline_points_[0], spike_length);
		}

		if (plot_mode_ == PLOT_ONE_CLASS || plot_mode_ == PLOT_ONE_COLOR)
		{
			selected_pen_color = index_color_selected_;
			if (plot_mode_ == PLOT_ONE_COLOR)
				selected_pen_color = selected_class_ % NB_COLORS;
			p_dc->SelectObject(&pen_table_[selected_pen_color]);
			for (auto i_spike = i_last_spike; i_spike >= i_first_spike; i_spike--)
			{
				const Spike* spike = p_spike_list_->get_spike(i_spike);
				// skip spike ?
				if (range_mode_ == RANGE_TIME_INTERVALS
					&& (spike->get_time() < l_first_ || spike->get_time() > l_last_))
					continue;

				// skip spikes with the wrong class
				if (spike->get_class_id() != selected_class_)
					continue;

				// display data
				p_spike_data = spike->get_p_data();
				fill_polypoint_y_axis(p_spike_data);
				p_dc->Polyline(&polyline_points_[0], spike_length);
			}
		}

		if (p_spike_list_->get_spike_flag_array_count() > 0)
			draw_flagged_spikes(p_dc);

		// display tags
		if (horizontal_tags.get_tag_list_size() > 0)
			display_horizontal_tags(p_dc);

		if (vertical_tags.get_tag_list_size() > 0)
			display_vertical_tags(p_dc);

		// display text
		if (b_text_ && plot_mode_ == PLOT_ONE_CLASS_ONLY)
		{
			TCHAR num[10];
			wsprintf(num, _T("%i"), get_selected_class());
			p_dc->TextOut(1, 1, num);
		}

		// restore resource
		p_dc->SelectObject(old_pen);
	}

	// display selected spike
	if ((spike_selected_.spike_index >= 0) && is_spike_within_range(spike_selected_))
	{
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		draw_spike_on_dc(spike, p_dc);
	}

	// restore resources
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeShape::draw_flagged_spikes(CDC * p_dc)
{
	const auto n_saved_dc = p_dc->SaveDC();

	// change coordinate settings
	get_extents();
	prepare_dc(p_dc);
	p_dc->SetViewportOrg(m_display_rect_.left, m_display_rect_.Height() / 2);
	p_dc->SetViewportExt(m_display_rect_.right, -m_display_rect_.Height());

	constexpr auto pen_size = 1;
	CPen new_pen(PS_SOLID, pen_size, color_table_[color_selected_spike_]);
	const auto old_pen = p_dc->SelectObject(&new_pen);

	// loop through all flagged spikes
	auto spike_sel = db_spike(dbwave_doc_->db_get_current_record_position(),
		dbwave_doc_->m_p_spk->get_spike_list_current_index(),
		0);
	for (auto i = p_spike_list_->get_spike_flag_array_count() - 1; i >= 0; i--)
	{
		spike_sel.spike_index = p_spike_list_->get_spike_flag_array_at(i);
		if (!is_spike_within_range(spike_sel))
			continue;
		const Spike* spike = dbwave_doc_->get_spike(spike_sel);
		fill_polypoint_y_axis(spike->get_p_data());
		p_dc->Polyline(&polyline_points_[0], p_spike_list_->get_spike_length());
	}

	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeShape::display_flagged_spikes(const BOOL b_highlight)
{
	if (b_highlight)
		draw_flagged_spikes(&plot_dc_);
	Invalidate();
}

int ChartSpikeShape::display_ex_data(short* p_data, const int color)
{
	// prepare array
	const auto spike_length = p_spike_list_->get_spike_length();
	if (polyline_points_.GetSize() != spike_length)
	{
		polyline_points_.SetSize(spike_length, 2);
		init_polypoint_x_axis();
	}

	CClientDC dc(this);
	dc.IntersectClipRect(&m_client_rect_);
	prepare_dc(&dc);
	CPen new_pen(PS_SOLID, 0, color_table_[color]);
	const auto old_pen = dc.SelectObject(&new_pen);
	fill_polypoint_y_axis(p_data);
	dc.Polyline(&polyline_points_[0], p_spike_list_->get_spike_length());

	dc.SelectObject(old_pen);
	return color;
}

void ChartSpikeShape::select_spike(const db_spike& spike_sel)
{
	spike_selected_ = spike_sel;
	if (spike_sel.spike_index >= 0) 
	{
		const Spike* spike = dbwave_doc_->get_spike(spike_sel);
		draw_spike(spike);
	}
}

void ChartSpikeShape::draw_spike(const Spike * spike)
{
	if (!b_use_dib_) {
		CClientDC dc(this);
		draw_spike_on_dc(spike, &dc);
	}
	else if (plot_dc_.GetSafeHdc()) {
		draw_spike_on_dc(spike, &plot_dc_);
	}
	Invalidate();
}

void ChartSpikeShape::draw_spike_on_dc(const Spike* spike, CDC * p_dc)
{
	const auto n_saved_dc = p_dc->SaveDC();
	auto rect = m_display_rect_;
	p_dc->DPtoLP(rect);
	p_dc->IntersectClipRect(&rect);

	get_extents();
	prepare_dc(p_dc);

	p_dc->SetViewportOrg(m_display_rect_.left, m_display_rect_.Height() / 2 + m_display_rect_.top);
	p_dc->SetViewportExt(m_display_rect_.Width(), -m_display_rect_.Height());

	constexpr auto pen_size = 2;
	CPen new_pen(PS_SOLID, pen_size, color_table_[color_selected_spike_]);
	auto* old_pen = p_dc->SelectObject(&new_pen);

	auto* p_data = spike->get_p_data();
	fill_polypoint_y_axis(p_data);
	p_dc->Polyline(&polyline_points_[0], p_spike_list_->get_spike_length());

	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeShape::OnLButtonUp(const UINT n_flags, CPoint point)
{
	if (!b_left_mouse_button_down_)
	{
		post_my_message(HINT_DROPPED, NULL);
		return;
	}

	switch (track_mode_)
	{
	case TRACK_BASELINE:
	{
		if (point.y != m_pt_last_.y || point.x != m_pt_last_.x)
			OnMouseMove(n_flags, point);
		release_cursor();
		ChartSpike::OnLButtonUp(n_flags, point);
	}
	break;

	case TRACK_VT_TAG:
	{
		// convert pix into data value and back again
		const auto val = MulDiv(point.x - m_x_viewport_origin_, m_x_we_, m_x_viewport_extent_) + m_x_wo_;
		vertical_tags.set_value_int(hc_trapped_, val);
		point.x = MulDiv(val - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
		xor_vertical_tag(point.x);
		ChartSpike::OnLButtonUp(n_flags, point);
		post_my_message(HINT_CHANGE_VERT_TAG, hc_trapped_);
	}
	break;

	default:
	{
		// none of those: zoom data or  offset display
		ChartSpike::OnLButtonUp(n_flags, point);
		CRect rect_out(m_pt_first_.x, m_pt_first_.y, m_pt_last_.x, m_pt_last_.y);
		constexpr auto jitter = 3;
		if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
		{
			if (cursor_type_ != CURSOR_ZOOM)
				post_my_message(HINT_HIT_AREA, NULL);
			else
				zoom_in();
			return;
		}

		// perform action according to cursor type
		auto rect_in = m_display_rect_;
		switch (cursor_type_)
		{
		case 0:
			rect_out = rect_in;
			rect_out.OffsetRect(m_pt_first_.x - m_pt_last_.x, m_pt_first_.y - m_pt_last_.y);
			post_my_message(HINT_DEFINED_RECT, NULL);
			break;
		case CURSOR_ZOOM:
			zoom_data(&rect_in, &rect_out);
			rect_zoom_from_ = rect_in;
			rect_zoom_to_ = rect_out;
			i_undo_zoom_ = 1;
			post_my_message(HINT_SET_MOUSE_CURSOR, old_cursor_type_);
			break;
		default:
			break;
		}
	}
	break;
	}
}

void ChartSpikeShape::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	b_left_mouse_button_down_ = TRUE;
	// call base class to test for horizontal cursor or XORing rectangle
	if (vertical_tags.get_tag_list_size() > 0)
	{
		for (auto i_tag = vertical_tags.get_tag_list_size() - 1; i_tag >= 0; i_tag--)
			vertical_tags.set_pixel(i_tag, MulDiv(vertical_tags.get_value_int(i_tag) - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_);
	}

	// track rectangle or VT_tag?
	ChartSpike::OnLButtonDown(n_flags, point);
	if (current_cursor_mode_ != 0 || hc_trapped_ >= 0)
		return;

	// test if mouse hit one spike
	// if hit, then tell parent to select corresponding spike
	dbwave_doc_->set_spike_hit(spike_hit_ = hit_curve_in_doc(point));
	if (spike_hit_.spike_index >= 0)
	{
		release_cursor();
		if (n_flags & MK_SHIFT)
			post_my_message(HINT_HIT_SPIKE_SHIFT, NULL);

		else
			post_my_message(HINT_HIT_SPIKE, NULL);
	}
}

// zoom_data(CRect* rFrom, CRect* rDest)
//
// max and min of rFrom should fit in rDest (same logical coordinates)
// then one can write the 4 equations:
// assume initial conditions WE1, WO1; destination: WE2, WO2
// LP_min = (rFrom.(top/left)      - VO) * WE1 / VE + WO1	(1)
// LP_Max = (rFrom.(bottom, right) - VO) * WE1 / VE + WO1	(2)
// LP_min = (rDest.(top/left)      - VO) * WE2 / VE + WO2	(3)
// LP_Max = (rDest.(bottom, right) - VO) * WE2 / VE + WO2	(4)
// from (1)-(2) = (3)-(4) one get WE2
// from (1)=(3)               get WO2

void ChartSpikeShape::zoom_data(CRect * r_from, CRect * r_dest)
{
	r_from->NormalizeRect(); // make sure that rect is not inverted
	r_dest->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_y_we_; // save previous window extent
	m_y_we_ = MulDiv(m_y_we_, r_dest->Height(), r_from->Height());
	m_y_wo_ = m_y_wo_
		- MulDiv(r_from->top - m_y_viewport_origin_, m_y_we_, m_y_viewport_extent_)
		+ MulDiv(r_dest->top - m_y_viewport_origin_, y_we, m_y_viewport_extent_);

	// change index of first and last pt displayed
	const auto x_we = m_x_we_; // save previous window extent
	m_x_we_ = MulDiv(m_x_we_, r_dest->Width(), r_from->Width());
	m_x_wo_ = m_x_wo_
		- MulDiv(r_from->left - m_x_viewport_origin_, m_x_we_, m_x_viewport_extent_)
		+ MulDiv(r_dest->left - m_x_viewport_origin_, x_we, m_x_viewport_extent_);

	// display
	Invalidate();
	post_my_message(HINT_CHANGE_ZOOM, 0);
}

void ChartSpikeShape::OnLButtonDblClk(UINT n_flags, CPoint point)
{
	if ((spike_selected_.spike_index < 0 && p_spike_list_->get_spike_flag_array_count() < 1) || spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(n_flags, point);
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

int ChartSpikeShape::hit_curve(const CPoint point)
{
	auto index_spike_hit = -1;
	// convert device coordinates into logical coordinates
	const auto mouse_x = MulDiv(point.x - m_x_viewport_origin_, m_x_we_, m_x_viewport_extent_) + m_x_wo_;
	if (mouse_x < 0 || mouse_x > p_spike_list_->get_spike_length())
		return index_spike_hit;
	const auto mouse_y = MulDiv(point.y - m_y_viewport_origin_, m_y_we_, m_y_viewport_extent_) + m_y_wo_;
	const auto delta_y = MulDiv(3, m_y_we_, m_y_viewport_extent_);

	// loop through all spikes
	auto index_last_spike = p_spike_list_->get_spikes_count() - 1;
	auto index_first_spike = 0;
	if (range_mode_ == RANGE_INDEX)
	{
		index_last_spike = index_last_spike_;
		index_first_spike = index_first_spike_;
	}
	for (auto spike_index = index_last_spike; spike_index >= index_first_spike; spike_index--)
	{
		if (range_mode_ == RANGE_TIME_INTERVALS
			&& (p_spike_list_->get_spike(spike_index)->get_time() < l_first_
				|| p_spike_list_->get_spike(spike_index)->get_time() > l_last_))
			continue;
		if (plot_mode_ == PLOT_ONE_CLASS_ONLY
			&& p_spike_list_->get_spike(spike_index)->get_class_id() != selected_class_)
			continue;

		const auto val = p_spike_list_->get_spike(spike_index)->get_value_at_offset(mouse_x);
		if (mouse_y + delta_y < val && mouse_y - delta_y > val)
		{
			index_spike_hit = spike_index;
			break;
		}
	}
	return index_spike_hit;
}

void ChartSpikeShape::get_extents()
{
	const auto current_file_index = dbwave_doc_->db_get_current_record_position();
	auto file_first = current_file_index;
	auto file_last = current_file_index;
	if (display_all_files_)
	{
		file_first = 0;
		file_last = dbwave_doc_->db_get_n_records() - 1;
	}

	if (m_y_we_ == 1 || m_y_we_ == 0)
	{
		for (auto file_index = file_first; file_index <= file_last; file_index++)
		{
			if (file_index != current_file_index)
			{
				if (dbwave_doc_->db_set_current_record_position(file_index))
					dbwave_doc_->open_current_spike_file();
				p_spike_list_ = dbwave_doc_->m_p_spk->get_spike_list_current();
			}
			if (p_spike_list_ != nullptr)
			{
				get_extents_current_spk_list();
				if (m_y_we_ != 0)
					break;
			}
		}
	}
}

void ChartSpikeShape::get_extents_current_spk_list()
{
	if (m_y_we_ == 1 || m_y_we_ == 0)
	{
		short value_max, value_min;
		p_spike_list_->get_total_max_min(TRUE, &value_max, &value_min);
		m_y_we_ = MulDiv((value_max - value_min), 10, 9) + 1;
		m_y_wo_ = value_max / 2 + value_min / 2;
	}

	if (m_x_we_ <= 1)
	{
		m_x_we_ = p_spike_list_->get_spike_length();
		m_x_wo_ = 0;
	}
}

void ChartSpikeShape::init_polypoint_x_axis()
{
	const auto n_elements = polyline_points_.GetSize();
	m_x_we_ = n_elements + 1;
	ASSERT(n_elements > 0);

	for (auto i = 0; i < n_elements; i++)
		polyline_points_[i].x = i + 1;
}

void ChartSpikeShape::fill_polypoint_y_axis(short* lp_source)
{
	auto n_elements = polyline_points_.GetSize();
	if (n_elements == 0)
	{
		n_elements = p_spike_list_->get_spike_length();
		ASSERT(n_elements > 0);
		polyline_points_.SetSize(n_elements, 2);
		init_polypoint_x_axis();
	}

	for (auto i = 0; i < n_elements; i++, lp_source++)
		polyline_points_[i].y = *lp_source;
}

void ChartSpikeShape::print(CDC * p_dc, const CRect * rect)
{
	// check if there are valid data to display
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() == 0)
		return;

	const auto old_y_vo = m_y_viewport_origin_;
	const auto old_y_ve = m_y_viewport_extent_;
	const auto old_x_extent = m_x_we_;
	const auto old_x_origin = m_x_wo_;

	// size of the window
	m_y_viewport_origin_ = rect->Height() / 2 + rect->top;
	m_y_viewport_extent_ = -rect->Height();

	// check initial conditions
	if (m_y_we_ == 1)
	{
		short value_max, value_min;
		p_spike_list_->get_total_max_min(TRUE, &value_max, &value_min);
		m_y_we_ = value_max - value_min + 1;
		m_y_wo_ = (value_max + value_min) / 2;
	}

	m_x_wo_ = rect->left;
	m_x_we_ = rect->Width() - 2;

	const auto spike_length = p_spike_list_->get_spike_length();
	if (polyline_points_.GetSize() != spike_length)
		polyline_points_.SetSize(spike_length, 2);

	// set mapping mode and viewport
	const auto n_saved_dc = p_dc->SaveDC();
	for (auto i = 0; i < spike_length; i++)
		polyline_points_[i].x = rect->left + MulDiv(i, rect->Width(), spike_length);

	int selected_color = BLACK_COLOR;
	switch (plot_mode_)
	{
		//case PLOT_BLACK:			sel_pen = BLACK_COLOR; break;
		//case PLOT_ONE_CLASS_ONLY:	sel_pen = BLACK_COLOR; break;
	case PLOT_ONE_CLASS:
	case PLOT_ALLGREY:
		selected_color = index_color_background_;
		break;
	default:
		break;
	}

	const auto old_pen = p_dc->SelectObject(&pen_table_[selected_color]);
	auto spike_index_last = p_spike_list_->get_spikes_count() - 1;
	auto spike_index_first = 0;
	if (range_mode_ == RANGE_INDEX)
	{
		spike_index_last = index_last_spike_;
		spike_index_first = index_first_spike_;
	}

	for (auto spike_index = spike_index_last; spike_index >= spike_index_first; spike_index--)
	{
		Spike* spike = p_spike_list_->get_spike(spike_index);
		if (range_mode_ == RANGE_INDEX && (spike_index > index_last_spike_ || spike_index < index_first_spike_))
			continue;
		if (range_mode_ == RANGE_TIME_INTERVALS)
		{
			if (spike->get_time() < l_first_)
				continue;
			if (spike->get_time() > l_last_)
				continue;
		}

		const auto spike_class = p_spike_list_->get_spike(spike_index)->get_class_id();
		if (plot_mode_ == PLOT_ONE_CLASS_ONLY && spike_class != selected_class_)
			continue;
		if (plot_mode_ == PLOT_ONE_CLASS && spike_class == selected_class_)
			continue;

		plot_array_to_dc(p_dc, spike->get_p_data());
	}

	// display selected class if requested by option
	if (plot_mode_ == PLOT_ONE_CLASS)
	{
		p_dc->SelectObject(&pen_table_[index_color_selected_]);
		for (auto spike_index = spike_index_last; spike_index >= spike_index_first; spike_index--)
		{
			Spike* spike = p_spike_list_->get_spike(spike_index);
			if (range_mode_ == RANGE_TIME_INTERVALS)
			{
				const auto spike_time = spike->get_time();
				if (spike_time < l_first_ || spike_time > l_last_)
					continue;
			}
			if (spike->get_class_id() != selected_class_)
				continue;
			plot_array_to_dc(p_dc, spike->get_p_data());
		}
	}

	// display selected spike
	if (spike_selected_.spike_index >= 0 && is_spike_within_range(spike_selected_))
	{
		CPen new_pen(PS_SOLID, 0, color_table_[color_selected_spike_]);
		p_dc->SelectObject(&new_pen);
		Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		plot_array_to_dc(p_dc, spike->get_p_data());
	}

	// restore resources
	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);

	m_x_we_ = old_x_extent;
	m_x_wo_ = old_x_origin;
	m_y_viewport_origin_ = old_y_vo;
	m_y_viewport_extent_ = old_y_ve;
}

void ChartSpikeShape::plot_array_to_dc(CDC * p_dc, short* p_array)
{
	const auto n_elements = polyline_points_.GetSize();
	for (auto i = 0; i < n_elements; i++, p_array++)
	{
		auto y = *p_array;
		y = static_cast<short>(MulDiv(y - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_);
		polyline_points_[i].y = y;
	}

	if (p_dc->m_hAttribDC == nullptr
		|| (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE))
		p_dc->Polyline(&polyline_points_[0], n_elements);
	else
	{
		p_dc->MoveTo(polyline_points_[0]);
		for (auto i = 0; i < n_elements; i++)
			p_dc->LineTo(polyline_points_[i]);
	}
}

void ChartSpikeShape::move_vt_track(int i_track, int new_value)
{
	CPoint point;
	m_pt_last_.x = MulDiv(vertical_tags.get_value_int(i_track) - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
	vertical_tags.set_value_int(i_track, new_value);
	point.x = MulDiv(new_value - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
	xor_vertical_tag(point.x);
}

void ChartSpikeShape::Serialize(CArchive & ar)
{
	int dummy_int = 1;
	if (ar.IsStoring())
	{
		ChartSpike::Serialize(ar);
		polyline_points_.Serialize(ar);

		ar << range_mode_; // display range (time OR storage index)
		ar << l_first_; // time first (real time = index/sampling rate)
		ar << l_last_; // time last
		ar << index_first_spike_; // index first spike
		ar << index_last_spike_; // index last spike
		ar << current_class_; // selected class (different color) (-1 = display all)
		ar << dummy_int;
		ar << color_selected_spike_; // color selected spike (index / color table)
		ar << dummy_int; 
		ar << selected_class_; // index class selected
		ar << b_text_; // allow text default false
		ar << selected_class_; // dummy
	}
	else
	{
		ChartSpike::Serialize(ar);
		polyline_points_.Serialize(ar);

		ar >> range_mode_; // display range (time OR storage index)
		ar >> l_first_; // time first (real time = index/sampling rate)
		ar >> l_last_; // time last
		ar >> index_first_spike_; // index first spike
		ar >> index_last_spike_; // index last spike
		ar >> current_class_; // selected class (different color) (-1 = display all)
		ar >> dummy_int;
		ar >> color_selected_spike_; // color selected spike (index / color table)
		ar >> dummy_int; 
		ar >> selected_class_; // index class selected
		ar >> b_text_; // allow text default false
		ar >> selected_class_; // dummy
	}
}

float ChartSpikeShape::get_display_max_mv()
{
	get_extents();
	return (p_spike_list_->convert_acquisition_point_to_mv(m_y_we_ - m_y_wo_ ));
}

float ChartSpikeShape::get_display_min_mv()
{
	if (p_spike_list_ == nullptr)
		return 1.f;
	get_extents();
	return p_spike_list_->convert_acquisition_point_to_mv(m_y_wo_ - m_y_we_ );
}

float ChartSpikeShape::get_extent_mv()
{
	if (p_spike_list_ == nullptr)
		return 1.f;
	get_extents();
	return (p_spike_list_->get_acq_volts_per_bin() * static_cast<float>(m_y_we_) * 1000.f);
}

float ChartSpikeShape::get_extent_ms()
{
	if (p_spike_list_ == nullptr)
		return 1.f;
	get_extents();
	return (static_cast<float>(1000.0 * m_x_we_) / p_spike_list_->get_acq_sampling_rate());
}


