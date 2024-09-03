
#include "StdAfx.h"
#include "ChartSpikeXY.h"

#include "ChartSpikeShape.h"

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
= default;

ChartSpikeXY::~ChartSpikeXY()
= default;

void ChartSpikeXY::plot_data_to_dc_prepare_dc(CDC * p_dc)
{
	if (b_erase_background_)
		erase_background(p_dc);

	p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	auto rect = display_rect_;
	rect.DeflateRect(1, 1);

	// save context
	saved_dc_ = p_dc->SaveDC();
	saved_background_color_ = p_dc->GetBkColor();

	// display data: trap error conditions
	window_duration_ = l_last_ - l_first_ + 1;
	get_extents();
	ASSERT(x_we_ != 1);
	p_dc->SetMapMode(MM_TEXT);
}

void ChartSpikeXY::plot_data_to_dc(CDC* p_dc)
{
	plot_data_to_dc_prepare_dc(p_dc);

	// prepare brush & rectangles (small for all spikes, larger for spikes belonging to the selected class)
	auto width = dot_width_ / 2;
	const CRect rect0(-width, -width, width, width);
	width = dot_width_ * 2 / 3;
	const CRect rect1(-width, -width, width, width);

	long n_files = 1;
	const long index_current_file = dbwave_doc_->db_get_current_record_position();
	if (b_display_all_files_)
		n_files = dbwave_doc_->db_get_records_count();

	for (long index_file = 0; index_file < n_files; index_file++)
	{
		p_spike_list_ = dbwave_doc_->m_p_spk_doc->get_spike_list_current();
		if (!get_spike_list_from_file(index_file) && !b_display_all_files_)
		{
			display_text_bottom_left(p_dc, cs_empty_, col_dark_gray);
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
		for (int spike_index = last_spike_index; spike_index >= first_spike_index; spike_index--)
		{
			const Spike* spike = p_spike_list_->get_spike(spike_index);
			display_spike_measure(spike, p_dc, rect0, rect1, window_duration_);
		}

		// display spike selected
		if (spike_selected_.record_id == dbwave_doc_->db_get_current_record_id() && spike_selected_.spike_index >= 0)
		{
			const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
			highlight_spike_measure(spike);
		}

		if (p_spike_list_->get_spike_flag_array_count() > 0)
		{
			// loop over the array of flagged spikes
			auto spike_sel = db_spike (index_file, dbwave_doc_->m_p_spk_doc->get_index_current_spike_list(), 0);
			for (auto i = p_spike_list_->get_spike_flag_array_count() - 1; i >= 0; i--)
			{
				spike_sel.spike_index = p_spike_list_->get_spike_index_of_flag(i);
				const Spike* spike = dbwave_doc_->get_spike(spike_sel);
				highlight_spike_measure(spike);
			}
		}
	}

	// restore resources
	p_dc->SetBkColor(saved_background_color_);
	p_dc->RestoreDC(saved_dc_);

	//display cursors
	if (hz_tags.get_tag_list_size() > 0)
		display_hz_tags(p_dc);

	if (vt_tags.get_tag_list_size() > 0) {
		x_wo_ = l_first_;
		x_we_ = l_last_ - l_first_ + 1;
		display_vt_tags_int_values(p_dc);
	}

	// restore selection to initial file
	if (b_display_all_files_)
	{
		if(dbwave_doc_->db_set_current_record_position(index_current_file))
			dbwave_doc_->open_current_spike_file();
		if (dbwave_doc_->m_p_spk_doc != nullptr)
			p_spike_list_ = dbwave_doc_->m_p_spk_doc->get_spike_list_current();
	}
}

void ChartSpikeXY::display_spike_measure(const Spike* spike, CDC* p_dc, const CRect& rect, const CRect& rect1, const long window_duration) const
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
		selected_brush = spike_class_id % nb_colors;
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
	const auto x1 = MulDiv(l_spike_time - l_first_, x_viewport_extent_, window_duration) + x_viewport_origin_;
	const auto y1 = MulDiv(spike->get_y1() - y_wo_, y_ve_, y_we_) + y_vo_;
	rect_i.OffsetRect(x1, y1);
	p_dc->MoveTo(x1, y1);
	p_dc->FillSolidRect(&rect_i, color_spike_class[selected_brush]);
}

void ChartSpikeXY::display_spike_measure(const Spike* spike) 
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
	draw_spike_measure(spike, color_index);
}

void ChartSpikeXY::draw_spike_measure(const Spike* spike, const int color_index)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&client_rect_);

	const auto l_spike_time = spike->get_time();
	const auto window_duration = l_last_ - l_first_ + 1;
	const auto x1 = MulDiv(l_spike_time - l_first_, x_viewport_extent_, window_duration) + x_viewport_origin_;
	const auto y1 = MulDiv(spike->get_y1() - y_wo_, y_ve_, y_we_) + y_vo_;
	CRect rect(0, 0, dot_width_, dot_width_);
	rect.OffsetRect(x1 - dot_width_ / 2, y1 - dot_width_ / 2);

	const auto background_color = dc.GetBkColor();
	dc.MoveTo(x1, y1);
	dc.FillSolidRect(&rect, color_spike_class[color_index]);
	dc.SetBkColor(background_color);
}

void ChartSpikeXY::highlight_spike_measure(const Spike* spike)
{
	CClientDC dc(this);
	dc.IntersectClipRect(&client_rect_);

	const auto old_rop2 = dc.SetROP2(R2_NOTXORPEN);
	const auto l_spike_time = spike->get_time();
	const auto window_duration = l_last_ - l_first_ + 1;
	const auto x1 = MulDiv(l_spike_time - l_first_, x_viewport_extent_, window_duration) + x_viewport_origin_;
	const auto y1 = MulDiv(spike->get_y1() - y_wo_, y_ve_, y_we_) + y_vo_;

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

void ChartSpikeXY::move_hz_tag(const int tag_index, const int value)
{
	Tag* p_tag = hz_tags.get_tag(tag_index);
	p_tag->value_int = value;
	p_tag->pixel = MulDiv(value - y_wo_, 
							y_ve_, 
							y_we_) + y_vo_;
	xor_hz_tag(p_tag->pixel, p_tag->swap_pixel(p_tag->pixel));
}

void ChartSpikeXY::move_vt_tag(const int tag_index, const int new_value)
{
	Tag* p_tag = vt_tags.get_tag(tag_index);
	p_tag->value_int = new_value;
	const auto pixels = MulDiv(new_value - x_wo_, 
							x_viewport_extent_, 
							x_we_) + x_viewport_origin_;
	xor_vt_tag(pixels, p_tag->swap_pixel(pixels));
}

void ChartSpikeXY::select_spike_measure(const db_spike& new_spike_selected)
{
	if (spike_selected_.spike_index >= 0) {
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		highlight_spike_measure(spike);
		display_spike_measure(spike);

	}

	spike_selected_ = new_spike_selected;
	if (spike_selected_.spike_index >= 0) {
		const Spike* spike = dbwave_doc_->get_spike(spike_selected_);
		display_spike_measure(spike);
		highlight_spike_measure(spike);
	}
}

void ChartSpikeXY::OnLButtonUp(UINT n_flags, CPoint point)
{
	switch (track_mode_)
	{
	case TRACK_HZ_TAG:
		left_button_up_horizontal_tag(n_flags, point);
		break;

	case TRACK_VT_TAG:
		left_button_up_vertical_tag(n_flags, point);
		break;

	default:
		{
			ChartSpike::OnLButtonUp(n_flags, point);
			CRect rect_out(pt_first_.x, pt_first_.y, pt_last_.x, pt_last_.y);
			constexpr auto jitter = 3;
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
				rect_out = rect_in;
				rect_out.OffsetRect(pt_first_.x - pt_last_.x, pt_first_.y - pt_last_.y);
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

void ChartSpikeXY::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	// compute pixel position of tags
	if (hz_tags.get_tag_list_size() > 0)
	{
		for (auto tag_index = hz_tags.get_tag_list_size() - 1; tag_index >= 0; tag_index--)
			hz_tags.set_pixel(tag_index, MulDiv(hz_tags.get_value_int(tag_index) - y_wo_, y_ve_, y_we_) + y_vo_);
	}
	if (vt_tags.get_tag_list_size() > 0)
	{
		for (auto tag_index = vt_tags.get_tag_list_size() - 1; tag_index >= 0; tag_index--)
		{
			const auto val = vt_tags.get_value_int(tag_index);
			const auto pix = MulDiv(val - x_wo_, x_viewport_extent_, x_we_) + x_viewport_origin_;
			vt_tags.set_pixel(tag_index, pix);
		}
	}

	// track rectangle or HZ tag?
	ChartSpike::OnLButtonDown(n_flags, point);
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
		if (n_flags & MK_SHIFT)
			send_my_message(HINT_HIT_SPIKE_SHIFT, NULL); // post?

		else
			send_my_message(HINT_HIT_SPIKE, NULL); // post?
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
	const auto y_we = y_we_; // save previous window extent
	y_we_ = MulDiv(y_we_, rect_dest->Height(), rect_from->Height());
	y_wo_ = y_wo_
		- MulDiv(rect_from->top - y_vo_, y_we_, y_ve_)
		+ MulDiv(rect_dest->top - y_vo_, y_we, y_ve_);

	// change index of first and last pt displayed
	auto l_size = l_last_ - l_first_ + 1;
	l_first_ = l_first_ + l_size * (rect_dest->left - rect_from->left) / rect_from->Width();
	l_size = l_size * rect_dest->Width() / rect_from->Width();
	l_last_ = l_first_ + l_size - 1;
	// display
	Invalidate();
	send_my_message(HINT_CHANGE_HZ_LIMITS, NULL); // post?
}

void ChartSpikeXY::OnLButtonDblClk(UINT n_flags, CPoint point)
{
	/*if (spike_selected_.spike_index < 0 || spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(n_flags, point);
	else
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),
		                         reinterpret_cast<LPARAM>(m_hWnd));*/
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

int ChartSpikeXY::hit_curve(const CPoint point)
{
	// abscissa
	const auto time_frame = (l_last_ - l_first_ + 1);
	x_max_ = l_first_ + time_frame * (point.x + dot_width_) / static_cast<long>(x_viewport_extent_);
	x_min_ = l_first_ + time_frame * (point.x - dot_width_) / static_cast<long>(x_viewport_extent_);
	// ordinates
	y_max_ = MulDiv(point.y - dot_width_ - y_vo_, y_we_, y_ve_) + y_wo_;
	y_min_ = MulDiv(point.y + dot_width_ - y_vo_, y_we_, y_ve_) + y_wo_;

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
	if (y_we_ == 1) // && m_yWO == 0)
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
		y_we_ = max_value - min_value + 2;
		y_wo_ = (max_value + min_value) / 2;
	}

	if (x_viewport_extent_ == 1 && x_viewport_origin_ == 0)
	{
		x_viewport_extent_ = display_rect_.Width();
		x_viewport_origin_ = display_rect_.left;
	}
	if (x_we_ == 1) // && m_xWO == 0)
	{
		x_we_ = display_rect_.Width();
		x_wo_ = display_rect_.left;
	}
}
