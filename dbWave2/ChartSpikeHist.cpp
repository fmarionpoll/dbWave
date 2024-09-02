
// Purpose:
// 		contains and display histograms
// 		histogram bins are stored into an array of "long"
// 		histograms are computed from various sources of data:
//			Acq_data document
//			Spike data
//

#include "StdAfx.h"
#include "ChartSpikeHist.h"

#include "ColorNames.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ChartSpikeHist, ChartSpike)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
END_MESSAGE_MAP()

ChartSpikeHist::ChartSpikeHist()
= default;

ChartSpikeHist::~ChartSpikeHist()
{
	clear_data();
}

void ChartSpikeHist::clear_data()
{
	if (histogram_array_.GetSize() > 0) 
	{
		for (auto i = histogram_array_.GetUpperBound(); i >= 0; i--)
			delete histogram_array_[i];
		histogram_array_.RemoveAll();
	}
}

void ChartSpikeHist::plot_data_to_dc(CDC* p_dc)
{
	if (display_rect_.right <= 0 && display_rect_.bottom <= 0)
	{
		CRect r;
		GetWindowRect(&r);
		OnSize(SIZE_RESTORED, r.Width(), r.Height());
	}
	if (b_erase_background_)
		erase_background(p_dc);

	// load resources
	CRect rect1;
	GetWindowRect(rect1);
	y_vo_ = rect1.Height();

	get_extents();
	if (l_max_ == 0)
	{
		p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
		display_text_bottom_left(p_dc, cs_empty_, col_dark_gray);
		return;
	}
	const int n_saved_dc = p_dc->SaveDC();
	const auto old_background_color = p_dc->GetBkColor();
	p_dc->SetMapMode(MM_ANISOTROPIC);
	prepare_dc(p_dc);

	//loop over histograms (but not the selected one)
	for (auto i_histogram = 0; i_histogram < histogram_array_.GetSize(); i_histogram++)
	{
		const auto p_dw = histogram_array_.GetAt(i_histogram);
		if (0 == p_dw->GetSize())
			continue;

		int color_index = BLACK_COLOR;
		if (i_histogram > 0)
		{
			const auto spike_class = static_cast<int>(p_dw->GetAt(0));
			if (PLOT_ONE_CLASS_ONLY == plot_mode_ && spike_class != selected_class_)
				continue;
			if (PLOT_CLASS_COLORS == plot_mode_)
				color_index = spike_class % nb_colors;
			else if (plot_mode_ == PLOT_ONE_CLASS && spike_class == selected_class_)
			
				continue;
		}

		plot_histogram(p_dc, p_dw, color_index);
	}

	// plot selected class (one histogram)
	if (plot_mode_ == PLOT_ONE_CLASS)
	{
		CDWordArray* p_dw = nullptr;
		get_class_array(selected_class_, p_dw);
		if (p_dw != nullptr)
			plot_histogram(p_dc, p_dw, col_black);
	}

	p_dc->SetBkColor(old_background_color);
	p_dc->RestoreDC(n_saved_dc);
	
	// display cursors
	if (hz_tags.get_tag_list_size() > 0)
		display_hz_tags(p_dc);

	if (vt_tags.get_tag_list_size() > 0)
		display_vt_tags_int_values(p_dc);
}

void ChartSpikeHist::plot_histogram(CDC* p_dc, const CDWordArray* p_dw, const int color) 
{
	CRect rect_histogram;
	double interval = abscissa_min_mv_;
	const int n_bins = p_dw->GetSize();
	abscissa_bin_mv_ = (abscissa_max_mv_ - abscissa_min_mv_) / n_bins;
	rect_histogram.bottom = 0;
	rect_histogram.right = convert_mv_to_abscissa(interval);

	for (auto i = 1; i < n_bins; i++)
	{
		rect_histogram.left = rect_histogram.right;
		interval += abscissa_bin_mv_;
		rect_histogram.right = convert_mv_to_abscissa(interval);

		rect_histogram.top = static_cast<int>(p_dw->GetAt(i));
		if (rect_histogram.top > 0)
		{
			p_dc->MoveTo(rect_histogram.left, rect_histogram.bottom);
			p_dc->FillSolidRect(rect_histogram, color_spike_class[color]);
		}
	}
}

void ChartSpikeHist::move_hz_tag_to_val(const int tag_index, const int value)
{
	Tag* p_tag = hz_tags.get_tag(tag_index);
	p_tag->value_int = value;
	p_tag->pixel = MulDiv(value - y_wo_, y_ve_, y_we_) + y_vo_;
	xor_hz_tag(p_tag->pixel, p_tag->swap_pixel(p_tag->pixel));
}

void ChartSpikeHist::move_vt_tag_to_value_mv(const int tag_index, const double value_mv)
{
	Tag* p_tag = vt_tags.get_tag(tag_index);
	p_tag->value_int = convert_mv_to_abscissa(value_mv);
	const auto pixel = MulDiv(p_tag->value_int - x_wo_,
							x_viewport_extent_, 
							x_we_) + x_viewport_origin_;
	xor_vt_tag(pixel, p_tag->swap_pixel(pixel));
}

void ChartSpikeHist::set_vt_tag_value_mv(const int tag_index, const double value_mv)
{
	Tag* p_tag = vt_tags.get_tag(tag_index);
	p_tag->value_int = convert_mv_to_abscissa(value_mv);
	p_tag->pixel = -1;
}

double ChartSpikeHist::get_vt_tag_value_mv(const int tag_index)
{
	const Tag* p_tag = vt_tags.get_tag(tag_index);
	return convert_abscissa_to_mv(p_tag->value_int); 
}

void ChartSpikeHist::get_class_array(const int i_class, CDWordArray*& p_dw)
{
	// test if pDW at 0 position is the right one
	if ((nullptr != p_dw) && (static_cast<int>(p_dw->GetAt(0)) == i_class))
		return;

	// not found, scan the array
	p_dw = nullptr;
	for (auto i = 1; i < histogram_array_.GetSize(); i++)
	{
		if (static_cast<int>((histogram_array_[i])->GetAt(0)) == i_class)
		{
			p_dw = histogram_array_[i];
			break;
		}
	}
}

LPTSTR ChartSpikeHist::export_ascii(LPTSTR lp)
{
	// print all ordinates line-by-line, different classes on same line
	lp += wsprintf(lp, _T("Histogram\nn_bins=%i\nnclasses=%i"), abscissa_n_bins_, histogram_array_.GetSize());
	lp += wsprintf(lp, _T("\nmax_mV=%f\tmin_mV=%f\nbin_mV=%f"), abscissa_max_mv_, abscissa_min_mv_, abscissa_bin_mv_);
	// export classes & points
	lp += wsprintf(lp, _T("classes;\n"));
	int i;
	for (i = 0; i < histogram_array_.GetSize(); i++)
		lp += wsprintf(lp, _T("%i\t"), static_cast<int>((histogram_array_[i])->GetAt(0)));
	lp--; // erase \t and replace with \n

	// loop through all points
	lp += wsprintf(lp, _T("\nvalues;\n"));
	for (auto j = 1; j <= abscissa_n_bins_; j++)
	{
		for (i = 0; i < histogram_array_.GetSize(); i++)
			lp += wsprintf(lp, _T("%i\t"), static_cast<int>((histogram_array_[i])->GetAt(j)));
		lp--; // erase \t and replace with \n
		lp += wsprintf(lp, _T("\n"));
	}
	*lp = _T('\0');
	return lp;
}

void ChartSpikeHist::OnLButtonUp(const UINT n_flags, CPoint point)
{
	// test if horizontal tag was tracked
	switch (track_mode_)
	{
	case TRACK_HZ_TAG:
		left_button_up_horizontal_tag(n_flags, point);
		break;

	case TRACK_VT_TAG:
		left_button_up_vertical_tag(n_flags, point);
		break;

	case TRACK_RECT:
		{
			ChartSpike::OnLButtonUp(n_flags, point); // else release mouse

			// none: zoom data or offset display
			ChartSpike::OnLButtonUp(n_flags, point);
			CRect rect_out(pt_first_.x, pt_first_.y, pt_last_.x, pt_last_.y);
			constexpr int jitter = 3;
			if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
			{
				if (cursor_type_ != CURSOR_ZOOM)
					send_my_message(HINT_HIT_AREA, NULL); // post?
				else
					zoom_in();
				break; // exit: mouse movement was too small
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
	default:
		break;
	}
}

void ChartSpikeHist::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	// compute pixel position of horizontal tags
	if (hz_tags.get_tag_list_size() > 0)
	{
		for (auto i_cur = hz_tags.get_tag_list_size() - 1; i_cur >= 0; i_cur--)
			hz_tags.set_pixel(i_cur, MulDiv(hz_tags.get_value_int(i_cur) - y_wo_, y_ve_, y_we_) + y_vo_);
	}
	// compute pixel position of vertical tags
	if (vt_tags.get_tag_list_size() > 0)
	{
		for (auto i_cur = vt_tags.get_tag_list_size() - 1; i_cur >= 0; i_cur--) // loop through all tags
			vt_tags.set_pixel(i_cur, MulDiv(vt_tags.get_value_int(i_cur) - x_wo_, x_viewport_extent_, x_we_) + x_viewport_origin_);
	}

	ChartSpike::OnLButtonDown(n_flags, point);
	if (current_cursor_mode_ != 0 || hc_trapped_ >= 0) // do nothing else if mode != 0
		return; // or any tag hit (VT, HZ) detected

	// test if mouse hit one histogram
	// if hit, then tell parent to select histogram (spike)
	spike_hit_ = hit_curve_in_doc(point);
	dbwave_doc_->set_spike_hit(spike_hit_);
	if (spike_hit_.spike_index >= 0)
	{
		release_cursor(); 
		send_my_message(HINT_HIT_SPIKE, NULL); // post?
	}
	else
		release_cursor();
}

// zoom_data
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// parameters:
// wo = window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=y-extent, ve=rect.height/2, vo = -rect.GetRectHeight()/2

void ChartSpikeHist::zoom_data(CRect* r_from, CRect* r_dest)
{
	r_from->NormalizeRect();
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

void ChartSpikeHist::OnLButtonDblClk(const UINT n_flags, const CPoint point)
{
	if (spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(n_flags, point);
	else
	{
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),
		                         reinterpret_cast<LPARAM>(m_hWnd));
	}
}

int ChartSpikeHist::hit_curve(const CPoint point)
{
	auto hit_spk = -1;
	// convert device coordinates into logical coordinates
	const auto delta_x = MulDiv(3, x_we_, x_viewport_extent_);
	const auto mouse_x = MulDiv(point.x - x_viewport_origin_, x_we_, x_viewport_extent_) + x_wo_;
	auto mouse_x1 = mouse_x - delta_x;
	auto mouse_x2 = mouse_x - delta_x;
	if (mouse_x1 < 1)
		mouse_x1 = 1;
	if (mouse_x1 > abscissa_n_bins_)
		mouse_x1 = abscissa_n_bins_;
	if (mouse_x2 < 1)
		mouse_x2 = 1;
	if (mouse_x2 > abscissa_n_bins_)
		mouse_x2 = abscissa_n_bins_;

	const auto delta_y = MulDiv(3, y_we_, y_ve_);
	const auto mouse_y = static_cast<DWORD>(MulDiv(point.y - y_vo_, y_we_, y_ve_)) + y_wo_ + delta_y;

	// test selected histogram first (foreground)
	const CDWordArray* p_dw = nullptr;
	if (plot_mode_ == PLOT_ONE_CLASS || plot_mode_ == PLOT_ONE_CLASS_ONLY)
	{
		auto i_hist = 1;
		// get array corresponding to selected_class_ as well as histogram index
		for (auto i = 1; i < histogram_array_.GetSize(); i++)
		{
			if (static_cast<int>((histogram_array_[i])->GetAt(0)) == selected_class_)
			{
				p_dw = histogram_array_[i];
				i_hist = i;
				break;
			}
		}
		//
		if (p_dw != nullptr)
		{
			for (auto i = mouse_x1; i < mouse_x2; i++)
			{
				const auto iww = p_dw->GetAt(i - 1);
				if (mouse_y <= iww)
				{
					hit_spk = i_hist;
					break;
				}
			}
		}
	}

	// test other histograms
	if (plot_mode_ != PLOT_ONE_CLASS_ONLY && hit_spk < 0)
	{
		for (auto j_hist = 1; j_hist < histogram_array_.GetSize() && hit_spk < 0; j_hist++)
		{
			p_dw = histogram_array_.GetAt(j_hist);
			if (plot_mode_ == PLOT_ONE_CLASS && static_cast<int>(p_dw->GetAt(0)) == selected_class_)
				continue;
			for (auto i = mouse_x1; i <= mouse_x2; i++)
			{
				const auto iww = p_dw->GetAt(i - 1);
				if (mouse_y <= iww)
				{
					hit_spk = j_hist;
					break;
				}
			}
		}
	}
	return hit_spk;
}

void ChartSpikeHist::get_extents()
{
	if (y_we_ == 1) // && m_yWO == 0)
	{
		if (l_max_ == 0)
			get_histogram_limits(0);
		y_we_ = static_cast<int>(l_max_);
		y_wo_ = 0;
	}

	//if (m_x_we_ == 1) // && m_xWO == 0)
	//{
		x_we_ = static_cast<int>(floor((abscissa_max_mv_ - abscissa_min_mv_ + 1)/ abscissa_bin_mv_));
		x_wo_ = 0; 
	//}
}

void ChartSpikeHist::get_histogram_limits(const int i_hist)
{
	// for some unknown reason, m_p_Hist_array is set at zero when arriving here
	if (histogram_array_.GetSize() <= 0)
	{
		const auto p_dw = new (CDWordArray); // init array
		ASSERT(p_dw != NULL);
		histogram_array_.Add(p_dw); // save pointer to this new array
		ASSERT(histogram_array_.GetSize() > 0);
	}
	const auto p_dw = histogram_array_[i_hist];
	if (p_dw->GetSize() <= 1)
		return;

	// get index of minimal value
	i_first_ = 1; // search first interval with data
	while (i_first_ <= abscissa_n_bins_ && p_dw->GetAt(i_first_) == 0)
		i_first_++;

	i_last_ = abscissa_n_bins_; // search last interval with data
	while (p_dw->GetAt(i_last_) == 0 && i_last_ > i_first_)
		i_last_--;

	// get index of maximum value
	i_max_ = i_first_; // index first pt
	l_max_ = 0; // max
	for (auto i = i_first_; i <= i_last_; i++)
	{
		const auto dw_item = p_dw->GetAt(i);
		if (dw_item > l_max_)
		{
			i_max_ = i;
			l_max_ = p_dw->GetAt(i);
		}
	}
}

void ChartSpikeHist::resize_histograms(const double min_mv, const double max_mv, const double bin_mv)
{
	const int n_bins = static_cast<int>((max_mv - min_mv) / bin_mv) + 1;
	abscissa_n_bins_ = n_bins;
	abscissa_bin_mv_ = bin_mv;
	abscissa_min_mv_ = min_mv;
	abscissa_max_mv_ = min_mv + n_bins * abscissa_bin_mv_;
	if (abscissa_max_mv_ < max_mv)
		abscissa_max_mv_ += abscissa_bin_mv_;
	ASSERT(abscissa_max_mv_ >= max_mv);

	if (histogram_array_.GetSize() <= 0)
	{
		const auto p_dword_array = new (CDWordArray);
		ASSERT(p_dword_array != NULL);
		histogram_array_.Add(p_dword_array);
		ASSERT(histogram_array_.GetSize() > 0);
	}

	for (auto j = histogram_array_.GetUpperBound(); j >= 0; j--)
	{
		const auto p_dw = histogram_array_[j];
		p_dw->SetSize(n_bins + 1);
		for (auto i = 1; i <= n_bins; i++)
			p_dw->SetAt(i, 0);
	}
}

void ChartSpikeHist::OnSize(const UINT n_type, const int cx, const int cy)
{
	ChartSpike::OnSize(n_type, cx, cy);
	y_vo_ = cy;
}

CDWordArray* ChartSpikeHist::init_class_array(const int n_bins, const int spike_class)
{
	const auto p_dw = new (CDWordArray);
	ASSERT(p_dw != NULL);
	histogram_array_.Add(p_dw);

	p_dw->SetSize(n_bins + 1);
	for (auto j = 1; j <= n_bins; j++)
		p_dw->SetAt(j, 0);
	p_dw->SetAt(0, spike_class);
	return p_dw;
}

void ChartSpikeHist::build_hist_from_spike_list(SpikeList* p_spk_list, 
			const long l_first, const long l_last, 
			const double min_mv, const double max_mv, const double bin_mv)
{
	auto* p_dword_array = histogram_array_[0];

	CDWordArray* p_dw = nullptr;
	const auto n_spikes = p_spk_list->get_spikes_count();
	for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		const auto spike_element = p_spk_list->get_spike(i_spike);
		const auto ii_time = spike_element->get_time();
		if (ii_time < l_first || ii_time > l_last)
			continue;

		const auto y1_mv = p_spk_list->convert_difference_to_mv(spike_element->get_y1());
		if (y1_mv > abscissa_max_mv_ || y1_mv < abscissa_min_mv_)
			continue;

		// increment corresponding histogram interval in the first histogram (general, displayed in grey)
		auto index = static_cast<int>(floor((y1_mv - abscissa_min_mv_) / abscissa_bin_mv_) + 1);
		if (index >= p_dword_array->GetSize())
			index = p_dword_array->GetSize() - 1;
		auto dw_data = p_dword_array->GetAt(index) + 1;
		p_dword_array->SetAt(index, dw_data);

		// dispatch into corresponding class histogram (create one if necessary)
		const auto spike_class = spike_element->get_class_id();
		get_class_array(spike_class, p_dw);
		if (p_dw == nullptr)
			p_dw = init_class_array(abscissa_n_bins_, spike_class);

		if (p_dw != nullptr)
		{
			dw_data = p_dw->GetAt(index) + 1;
			p_dw->SetAt(index, dw_data);
		}
	}
	get_histogram_limits(0);
}

// 	BuildHistFromDocument()
// parameters
//		CdbWaveDoc* p_doc	- dbWave document
//		BOOL ballFiles		- if false, compute only from current spike_list, otherwise compute over the whole document
//		long l_first		= index first pt from file
//		long l_last 		= index last pt from file
//		double max_mv		= maximum value in mv
//		double min_mv		= minimum value in mv
//		double bin_mv		= bin size in mv
//		BOOL bNew=TRUE		= erase old data (TRUE) or add to old value (FALSE)

void ChartSpikeHist::build_hist_from_document(CdbWaveDoc* p_document, const BOOL b_all_files,
				const long l_first, const long l_last,
				const double min_mv, const double max_mv,  const double bin_mv)
{
	clear_data();

	dbwave_doc_ = p_document;
	constexpr auto file_first = 0;
	const auto file_last = b_all_files ? p_document->db_get_records_count() : 1;
	resize_histograms(min_mv, max_mv, bin_mv);

	for (auto i_file = file_first; i_file < file_last; i_file++)
	{
		if (b_all_files)
		{
			if (p_document->db_set_current_record_position(i_file))
				p_document->open_current_spike_file(); 
		}
		if (p_document->m_p_spk_doc != nullptr)
		{
			SpikeList* p_spike_list = p_document->m_p_spk_doc->get_spike_list_current();
			if (p_spike_list == nullptr || p_spike_list->get_spikes_count() <= 0)
				continue;

			build_hist_from_spike_list(p_spike_list, l_first, l_last, min_mv, max_mv, bin_mv);
		}
	}
}


