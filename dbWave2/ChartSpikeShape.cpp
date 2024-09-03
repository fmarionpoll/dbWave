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
= default;

ChartSpikeShape::~ChartSpikeShape()
= default;

void ChartSpikeShape::plot_data_to_dc_prepare_dc(CDC* p_dc)
{
	if (b_erase_background_)
		erase_background(p_dc);
	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	saved_dc_ = p_dc->SaveDC();
	saved_background_color_ = p_dc->GetBkColor();

	get_extents();
	p_dc->SetMapMode(MM_ANISOTROPIC);
	prepare_dc(p_dc);
}


void ChartSpikeShape::plot_data_to_dc(CDC * p_dc)
{
	plot_data_to_dc_prepare_dc(p_dc);

	auto n_files = 1;
	long index_current_file = 0;
	if (b_display_all_files_)
	{
		n_files = dbwave_doc_->db_get_records_count();
		index_current_file = dbwave_doc_->db_get_current_record_position();
	}

	for (auto index_file = 0; index_file < n_files; index_file++)
	{
		if (!get_spike_list_from_file(index_file) && !b_display_all_files_)
		{
			display_text_bottom_left(p_dc, cs_empty_, col_dark_gray);
			continue;
		}

		const auto spike_length = p_spike_list_->get_spike_length();
		ASSERT(spike_length > 0);
		if (polyline_points_.GetSize() < p_spike_list_->get_spike_length())
			polyline_points_.SetSize(spike_length, 2);
		init_polypoint_x_axis();

		// loop through all spikes of the list
		auto i_first_spike = 0;
		auto i_last_spike = p_spike_list_->get_spikes_count() - 1;
		if (range_mode_ == RANGE_INDEX)
		{
			i_first_spike = index_first_spike_;
			i_last_spike = index_last_spike_;
		}
		auto selected_pen_color = BLACK_COLOR;
		if (plot_mode_ == PLOT_ONE_CLASS || plot_mode_ == PLOT_ONE_COLOR)
			selected_pen_color = SILVER_COLOR;
		const auto old_pen = p_dc->SelectObject(&pen_table_[selected_pen_color]);

		for (auto i_spike = i_first_spike; i_spike <= i_last_spike; i_spike++)
		{
			const Spike* spike = p_spike_list_->get_spike(i_spike);
			if (range_mode_ == RANGE_TIME_INTERVALS)
			{
				if (spike->get_time() < l_first_)
					continue;
				if (spike->get_time() > l_last_)
					break;
			}

			// select pen according to class
			const auto spike_class = spike->get_class_id();
			switch (plot_mode_)
			{
			case PLOT_ONE_CLASS_ONLY:
				if (spike_class != selected_class_)
					continue;
				break;
			case PLOT_CLASS_COLORS:
				selected_pen_color = spike_class % nb_colors;
				p_dc->SelectObject(&pen_table_[selected_pen_color]);
				break;
			case PLOT_ONE_CLASS:
				if (spike_class == selected_class_)
					continue;
				break;
			default:
				break;
			}
			display_spike_data(p_dc, spike);
		}
		p_dc->SelectObject(old_pen);

		if (plot_mode_ == PLOT_ONE_CLASS || plot_mode_ == PLOT_ONE_COLOR)
		{
			selected_pen_color = index_color_selected_;
			if (plot_mode_ == PLOT_ONE_COLOR)
				selected_pen_color = selected_class_ % nb_colors;
			p_dc->SelectObject(&pen_table_[selected_pen_color]);
			for (auto i_spike = i_first_spike; i_spike <= i_last_spike; i_spike++)
			{
				const Spike* spike = p_spike_list_->get_spike(i_spike);
				if (range_mode_ == RANGE_TIME_INTERVALS
					&& (spike->get_time() < l_first_ || spike->get_time() > l_last_))
					continue;
				if (spike->get_class_id() != selected_class_)
					continue;

				display_spike_data(p_dc, spike);
			}
		}

		p_dc->SelectObject(old_pen);
	}

	if (p_spike_list_->get_spike_flag_array_count() > 0)
		draw_flagged_spikes(p_dc);

	// restore resources
	p_dc->SetBkColor(saved_background_color_);
	p_dc->RestoreDC(saved_dc_);

	// display tags
	if (hz_tags.get_tag_list_size() > 0)
		display_hz_tags(p_dc);

	if (vt_tags.get_tag_list_size() > 0) 
		display_vt_tags_int_values(p_dc);

	// display text
	if (b_text_ && plot_mode_ == PLOT_ONE_CLASS_ONLY)
	{
		TCHAR num[10];
		wsprintf(num, _T("%i"), get_selected_class());
		p_dc->TextOut(1, 1, num);
	}

	// display selected spike
	if (spike_selected_.spike_index >= 0 && is_spike_within_range(spike_selected_))
	{
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		draw_spike_on_dc(spike, p_dc);
	}

	// restore selection to initial file
	if (b_display_all_files_)
	{
		if (dbwave_doc_->db_set_current_record_position(index_current_file))
			dbwave_doc_->open_current_spike_file();
		if (dbwave_doc_->m_p_spk_doc != nullptr)
			p_spike_list_ = dbwave_doc_->m_p_spk_doc->get_spike_list_current();
	}
}

void ChartSpikeShape::display_spike_data(CDC* p_dc, const Spike* spike)
{
	int* p_spike_data = spike->get_p_data();
	if (p_spike_data != nullptr)
	{
		fill_polypoint_y_axis(p_spike_data);
		p_dc->Polyline(&polyline_points_[0], spike->get_spike_length());
	}
}

void ChartSpikeShape::draw_flagged_spikes(CDC * p_dc)
{
	const auto n_saved_dc = p_dc->SaveDC();
	constexpr auto pen_size = 1;
	CPen new_pen(PS_SOLID, pen_size, color_spike_class_selected[0]);
	const auto old_pen = p_dc->SelectObject(&new_pen);

	get_extents();
	p_dc->SetMapMode(MM_ANISOTROPIC);
	prepare_dc(p_dc);
	p_dc->SetViewportOrg(display_rect_.left, display_rect_.Height() / 2);
	p_dc->SetViewportExt(display_rect_.right, -display_rect_.Height());
	auto spike_sel = db_spike(dbwave_doc_->db_get_current_record_id(), //db_get_current_record_position(),
		dbwave_doc_->m_p_spk_doc->get_index_current_spike_list(),
		0);

	for (auto i = 0; i < p_spike_list_->get_spike_flag_array_count(); i ++)
	{
		spike_sel.spike_index = p_spike_list_->get_spike_index_of_flag(i);
		if (!is_spike_within_range(spike_sel))
			continue;
		display_spike_data(p_dc, dbwave_doc_->get_spike(spike_sel));
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

int ChartSpikeShape::display_ex_data(int* p_data, const int color)
{
	const auto spike_length = p_spike_list_->get_spike_length();
	if (polyline_points_.GetSize() < spike_length)
	{
		polyline_points_.SetSize(spike_length, 2);
		init_polypoint_x_axis();
	}

	CClientDC dc(this);
	dc.IntersectClipRect(&client_rect_);
	dc.SetMapMode(MM_ANISOTROPIC);
	prepare_dc(&dc);
	CPen new_pen(PS_SOLID, 0, color_spike_class[color]);
	const auto old_pen = dc.SelectObject(&new_pen);
	fill_polypoint_y_axis(p_data);
	dc.Polyline(&polyline_points_[0], p_spike_list_->get_spike_length());

	dc.SelectObject(old_pen);
	return color;
}

void ChartSpikeShape::select_spike(db_spike& spike_sel)
{
	spike_selected_ = spike_sel;
	if (spike_sel.spike_index >= 0) 
	{
		const Spike* spike = dbwave_doc_->get_spike(spike_sel);
		if (spike != nullptr)
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
	auto rect = display_rect_;
	p_dc->DPtoLP(rect);
	p_dc->IntersectClipRect(&rect);

	get_extents();
	p_dc->SetMapMode(MM_ANISOTROPIC);
	prepare_dc(p_dc);

	p_dc->SetViewportOrg(display_rect_.left, display_rect_.Height() / 2 + display_rect_.top);
	p_dc->SetViewportExt(display_rect_.Width(), -display_rect_.Height());

	constexpr auto pen_size = 2;
	CPen new_pen(PS_SOLID, pen_size, color_spike_class_selected[0]);
	auto* old_pen = p_dc->SelectObject(&new_pen);

	display_spike_data(p_dc, spike);

	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeShape::move_vt_track(const int i_track, const int value)
{
	Tag* p_tag = vt_tags.get_tag(i_track);
	p_tag->value_int = value;
	const auto pixel = MulDiv(vt_tags.get_value_int(i_track) - x_wo_, x_viewport_extent_, x_we_) + x_viewport_origin_;
	xor_vt_tag(pixel, p_tag->swap_pixel(pixel));
}

void ChartSpikeShape::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	if (!b_left_mouse_button_down_)
	{
		send_my_message(HINT_DROPPED, NULL); // post?
		return;
	}

	switch (track_mode_)
	{
	case TRACK_BASELINE:
		{
			if (point.y != pt_last_.y || point.x != pt_last_.x)
				OnMouseMove(n_flags, point);
			release_cursor();
			ChartSpike::OnLButtonUp(n_flags, point);
		}
		break;

	case TRACK_VT_TAG:
		left_button_up_vertical_tag(n_flags, point);
		break;

	default:
	{
		// none of those: zoom data or  offset display
		ChartSpike::OnLButtonUp(n_flags, point);
		CRect rect_out(pt_first_.x, pt_first_.y, pt_last_.x, pt_last_.y);
		constexpr auto jitter = 3;
		if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
		{
			if (cursor_type_ != CURSOR_ZOOM)
				send_my_message(HINT_HIT_AREA, NULL); // post?
			else
				zoom_in();
			return;
		}

		// perform action according to cursor type
		auto rect_in = display_rect_;
		switch (cursor_type_)
		{
		case 0:
			rect_out = rect_in;
			rect_out.OffsetRect(pt_first_.x - pt_last_.x, pt_first_.y - pt_last_.y);
			send_my_message(HINT_DEFINED_RECT, NULL); // post?
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
	break;
	}
}

void ChartSpikeShape::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	b_left_mouse_button_down_ = TRUE;
	// call base class to test for horizontal cursor or XORing rectangle
	if (vt_tags.get_tag_list_size() > 0)
	{
		for (auto i_tag = vt_tags.get_tag_list_size() - 1; i_tag >= 0; i_tag--)
			vt_tags.set_pixel(i_tag, MulDiv(vt_tags.get_value_int(i_tag) - x_wo_, x_viewport_extent_, x_we_) + x_viewport_origin_);
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
			send_my_message(HINT_HIT_SPIKE_SHIFT, NULL); // post?

		else
			send_my_message(HINT_HIT_SPIKE, NULL); // post?
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
	const auto y_we = y_we_; // save previous window extent
	y_we_ = MulDiv(y_we_, r_dest->Height(), r_from->Height());
	y_wo_ = y_wo_
		- MulDiv(r_from->top - y_vo_, y_we_, y_ve_)
		+ MulDiv(r_dest->top - y_vo_, y_we, y_ve_);

	// change index of first and last pt displayed
	const auto x_we = x_we_; // save previous window extent
	x_we_ = MulDiv(x_we_, r_dest->Width(), r_from->Width());
	x_wo_ = x_wo_
		- MulDiv(r_from->left - x_viewport_origin_, x_we_, x_viewport_extent_)
		+ MulDiv(r_dest->left - x_viewport_origin_, x_we, x_viewport_extent_);

	// display
	Invalidate();
	send_my_message(HINT_CHANGE_ZOOM, 0); // post?
}

void ChartSpikeShape::OnLButtonDblClk(UINT n_flags, CPoint point)
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
			const auto selected_spike = hit_curve(point);
			if (selected_spike > 0)
				send_my_message(HINT_DBL_CLK_SEL, selected_spike); // post?
		}
	}
}

int ChartSpikeShape::hit_curve(const CPoint point)
{
	auto index_spike_hit = -1;
	// convert device coordinates into logical coordinates
	const auto mouse_x = MulDiv(point.x - x_viewport_origin_, x_we_, x_viewport_extent_) + x_wo_;
	if (mouse_x < 0 || mouse_x > p_spike_list_->get_spike_length())
		return index_spike_hit;
	const auto mouse_y = MulDiv(point.y - y_vo_, y_we_, y_ve_) + y_wo_;
	const auto delta_y = MulDiv(3, y_we_, y_ve_);

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
	if (b_display_all_files_)
	{
		file_first = 0;
		file_last = dbwave_doc_->db_get_records_count() - 1;
	}

	if (y_we_ == 1 || y_we_ == 0)
	{
		for (auto file_index = file_first; file_index <= file_last; file_index++)
		{
			if (file_index != current_file_index)
			{
				if (dbwave_doc_->db_set_current_record_position(file_index))
					dbwave_doc_->open_current_spike_file();
				p_spike_list_ = dbwave_doc_->m_p_spk_doc->get_spike_list_current();
			}
			if (p_spike_list_ != nullptr)
			{
				get_extents_current_spk_list();
				if (y_we_ != 0)
					break;
			}
		}
	}
}

void ChartSpikeShape::get_extents_current_spk_list()
{
	if (y_we_ == 1 || y_we_ == 0)
	{
		int value_max, value_min;
		p_spike_list_->get_total_max_min(TRUE, &value_max, &value_min);
		y_we_ = MulDiv((value_max - value_min), 10, 9) + 1;
		y_wo_ = value_max / 2 + value_min / 2;
	}

	if (x_we_ <= 1)
	{
		x_we_ = p_spike_list_->get_spike_length();
		x_wo_ = 0;
	}
}

void ChartSpikeShape::init_polypoint_x_axis()
{
	const auto n_elements = polyline_points_.GetSize();
	x_we_ = n_elements + 1;
	ASSERT(n_elements > 0);
	for (auto i = 0; i < n_elements; i++)
		polyline_points_[i].x = i + 1;
}

void ChartSpikeShape::fill_polypoint_y_axis(int* lp_source)
{
 
	if (p_spike_list_->get_spike_length() > polyline_points_.GetSize())
	{
		polyline_points_.SetSize(p_spike_list_->get_spike_length(), 2);
		init_polypoint_x_axis();
	}

	for (auto i = 0; i < p_spike_list_->get_spike_length(); i++, lp_source++)
		polyline_points_[i].y = static_cast<long>(static_cast<unsigned long>(*lp_source));
}

void ChartSpikeShape::print(CDC * p_dc, const CRect * rect)
{
	// check if there are valid data to display
	if (p_spike_list_ == nullptr || p_spike_list_->get_spikes_count() == 0)
		return;

	const auto old_y_vo = y_vo_;
	const auto old_y_ve = y_ve_;
	const auto old_x_extent = x_we_;
	const auto old_x_origin = x_wo_;

	// size of the window
	y_vo_ = rect->Height() / 2 + rect->top;
	y_ve_ = -rect->Height();

	// check initial conditions
	if (y_we_ == 1)
	{
		int value_max, value_min;
		p_spike_list_->get_total_max_min(TRUE, &value_max, &value_min);
		y_we_ = value_max - value_min + 1;
		y_wo_ = (value_max + value_min) / 2;
	}

	x_wo_ = rect->left;
	x_we_ = rect->Width() - 2;

	const auto spike_length = p_spike_list_->get_spike_length();
	if (polyline_points_.GetSize() < spike_length)
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
		const Spike* spike = p_spike_list_->get_spike(spike_index);
		if (range_mode_ == RANGE_INDEX 
			&& (spike_index > index_last_spike_ || spike_index < index_first_spike_))
			continue;
		if (range_mode_ == RANGE_TIME_INTERVALS
			&& (spike->get_time() < l_first_ || spike->get_time() > l_last_))
				continue;

		const auto spike_class = p_spike_list_->get_spike(spike_index)->get_class_id();
		if (plot_mode_ == PLOT_ONE_CLASS_ONLY && spike_class != selected_class_)
			continue;
		if (plot_mode_ == PLOT_ONE_CLASS && spike_class == selected_class_)
			continue;

		print_array_to_dc(p_dc, spike->get_p_data());
	}

	// display selected class if requested by option
	if (plot_mode_ == PLOT_ONE_CLASS)
	{
		p_dc->SelectObject(&pen_table_[index_color_selected_]);
		for (auto spike_index = spike_index_last; spike_index >= spike_index_first; spike_index--)
		{
			const Spike* spike = p_spike_list_->get_spike(spike_index);
			if (range_mode_ == RANGE_TIME_INTERVALS)
			{
				const auto spike_time = spike->get_time();
				if (spike_time < l_first_ || spike_time > l_last_)
					continue;
			}
			if (spike->get_class_id() != selected_class_)
				continue;
			print_array_to_dc(p_dc, spike->get_p_data());
		}
	}

	// display selected spike
	if (spike_selected_.spike_index >= 0 && is_spike_within_range(spike_selected_))
	{
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		CPen new_pen(PS_SOLID, 0, color_spike_class_selected[spike->get_class_id() % nb_colors]);
		p_dc->SelectObject(&new_pen);
		print_array_to_dc(p_dc, spike->get_p_data());
	}

	// restore resources
	p_dc->SelectObject(old_pen);
	p_dc->RestoreDC(n_saved_dc);

	x_we_ = old_x_extent;
	x_wo_ = old_x_origin;
	y_vo_ = old_y_vo;
	y_ve_ = old_y_ve;
}

void ChartSpikeShape::print_array_to_dc(CDC * p_dc, int* p_array)
{
	const auto n_elements = polyline_points_.GetSize();
	for (auto i = 0; i < n_elements; i++, p_array++)
	{
		auto y = *p_array;
		y = MulDiv(y - y_wo_, y_ve_, y_we_) + y_vo_;
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

void ChartSpikeShape::Serialize(CArchive & ar)
{
	int dummy_int = 1;
	WORD w_version = 1;
	ChartSpike::Serialize(ar);
	if (ar.IsStoring())
	{
		ar << w_version;
		ar << range_mode_; 
		ar << l_first_; 
		ar << l_last_; 
		ar << index_first_spike_; 
		ar << index_last_spike_; 
		ar << current_class_;
		ar << dummy_int;
		ar << dummy_int; 
		ar << dummy_int; 
		ar << selected_class_; 
		ar << b_text_; 
		ar << selected_class_; 
	}
	else
	{
		ar >> w_version;
		ar >> range_mode_; 
		ar >> l_first_;
		ar >> l_last_; 
		ar >> index_first_spike_; 
		ar >> index_last_spike_; 
		ar >> current_class_; 
		ar >> dummy_int;
		ar >> dummy_int; 
		ar >> dummy_int; 
		ar >> selected_class_; 
		ar >> b_text_; 
		ar >> selected_class_; 
	}
}

float ChartSpikeShape::get_display_max_mv()
{
	get_extents();
	return (p_spike_list_->convert_acquisition_point_to_mv(y_we_ - y_wo_ ));
}

float ChartSpikeShape::get_display_min_mv()
{
	if (p_spike_list_ == nullptr)
		return 1.f;
	get_extents();
	return p_spike_list_->convert_acquisition_point_to_mv(y_wo_ - y_we_ );
}

float ChartSpikeShape::get_extent_mv()
{
	if (p_spike_list_ == nullptr)
		return 1.f;
	get_extents();
	return (p_spike_list_->get_acq_volts_per_bin() * static_cast<float>(y_we_) * 1000.f);
}

float ChartSpikeShape::get_extent_ms()
{
	if (p_spike_list_ == nullptr)
		return 1.f;
	get_extents();
	return (static_cast<float>(1000.0 * x_we_) / p_spike_list_->get_acq_sampling_rate());
}


