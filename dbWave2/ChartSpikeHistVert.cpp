// Purpose:
// 		contains and display histograms
// 		histogram bins are stored into an array of "long"
// 		histograms are computed from various sources of data:
//			Acq data document
//			Spike data


#include "StdAfx.h"
#include "ChartSpikeHistVert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ChartSpikeHistVert, ChartSpike)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
END_MESSAGE_MAP()

ChartSpikeHistVert::ChartSpikeHistVert()
= default;

ChartSpikeHistVert::~ChartSpikeHistVert()
{
	remove_hist_data();
}

void ChartSpikeHistVert::remove_hist_data()
{
	if (histogram_ptr_array.GetSize() > 0) // delete objects pointed by elements
	{
		// of m_pHist array
		for (auto i = histogram_ptr_array.GetUpperBound(); i >= 0; i--)
			delete histogram_ptr_array[i];
		histogram_ptr_array.RemoveAll();
	}
}

void ChartSpikeHistVert::plot_data_to_dc(CDC* p_dc)
{
	if (display_rect_.right <= 0 && display_rect_.bottom <= 0)
	{
		CRect r;
		GetWindowRect(&r);
		OnSize(SIZE_RESTORED, r.Width(), r.Height());
	}
	if (b_erase_background_) // erase background
		erase_background(p_dc);

	// load resources
	get_extents();
	if (m_lmax_ == 0)
	{
		p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
		auto rect = display_rect_;
		rect.DeflateRect(1, 1);
		const auto text_len = cs_empty_.GetLength();
		p_dc->DrawText(cs_empty_, text_len, rect, DT_LEFT); 
		return;
	}
	const int n_saved_dc = p_dc->SaveDC();
	p_dc->SetMapMode(MM_ANISOTROPIC);
	prepare_dc(p_dc);

	int color;
	// save background color which is changed by later calls to FillSolidRect
	// when doing so, pens created with PS_DOT pattern and with XOR_PEN do
	// not work properly. Restoring the background color solves the pb.
	const auto background_color = p_dc->GetBkColor();
	switch (plot_mode_)
	{
	case PLOT_BLACK:
	case PLOT_ONE_CLASS_ONLY:
		color = BLACK_COLOR;
		break;
	default:
		color = SILVER_COLOR;
		break;
	}

	for (auto i_hist = 0; i_hist < histogram_ptr_array.GetSize(); i_hist++)
	{
		const auto p_dw = histogram_ptr_array.GetAt(i_hist);
		if (0 == p_dw->GetSize())
			continue;

		// select correct color
		if (i_hist > 0)
		{
			const auto spk_cla = static_cast<int>(p_dw->GetAt(0));
			color = BLACK_COLOR;
			if (PLOT_ONE_CLASS_ONLY == plot_mode_ && spk_cla != selected_class_)
				continue;
			if (PLOT_CLASS_COLORS == plot_mode_)
				color = spk_cla % nb_colors;
			else if (plot_mode_ == PLOT_ONE_CLASS && spk_cla == selected_class_)
			{
				color = BLACK_COLOR;
				continue;
			}
		}
		plot_histogram(p_dc, p_dw, color);
	}

	// plot selected class (one histogram)
	if (plot_mode_ == PLOT_ONE_CLASS)
	{
		color = BLACK_COLOR;
		CDWordArray* p_dw = nullptr;
		get_class_array(selected_class_, p_dw);
		if (p_dw != nullptr)
		{
			plot_histogram(p_dc, p_dw, color);
		}
	}

	// display cursors
	p_dc->SetBkColor(background_color); // restore background color
	if (hz_tags.get_tag_list_size() > 0) // display horizontal tags
		display_hz_tags(p_dc);
	if (vt_tags.get_tag_list_size() > 0) // display vertical tags
	{
		display_vt_tags_int_values(p_dc);
	}
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeHistVert::plot_histogram(CDC* p_dc, const CDWordArray* p_dw, const int color) const
{
	CRect rect_histogram;
	rect_histogram.bottom = abscissa_min_val_ - bin_size_;
	rect_histogram.top = abscissa_min_val_;
	rect_histogram.left = 0;
	for (auto i = 1; i < p_dw->GetSize(); i++)
	{
		rect_histogram.bottom += bin_size_;
		rect_histogram.top += bin_size_;

		rect_histogram.right = static_cast<int>(p_dw->GetAt(i));
		if (rect_histogram.top > 0)
		{
			p_dc->MoveTo(rect_histogram.bottom, rect_histogram.left);
			p_dc->FillSolidRect(rect_histogram, color_spike_class[color]);
		}
	}
}

void ChartSpikeHistVert::move_hz_tag_to_val(const int tag_index, const int value)
{
	Tag* p_tag = hz_tags.get_tag(tag_index);
	p_tag->value_int = value;
	p_tag->pixel = MulDiv(value - y_wo_, y_ve_, y_we_) + y_vo_;
	xor_hz_tag(p_tag->pixel, p_tag->swap_pixel(p_tag->pixel));
}

void ChartSpikeHistVert::move_vt_tag_to_val(const int i_tag, const int val)
{
	Tag* p_tag = vt_tags.get_tag(i_tag);
	p_tag->value_int = val;
	const auto pixels = MulDiv(val - x_wo_, x_viewport_extent_, x_we_) + x_viewport_origin_;
	xor_vt_tag(pixels, p_tag->swap_pixel(pixels));
}

void ChartSpikeHistVert::get_class_array(const int i_class, CDWordArray*& p_dw)
{
	// test if pDW at 0 position is the right one
	if ((nullptr != p_dw) && (static_cast<int>(p_dw->GetAt(0)) == i_class))
		return;

	// not found, scan the array
	p_dw = nullptr;
	for (auto i = 1; i < histogram_ptr_array.GetSize(); i++)
	{
		if (static_cast<int>((histogram_ptr_array[i])->GetAt(0)) == i_class)
		{
			p_dw = histogram_ptr_array[i];
			break;
		}
	}
}

LPTSTR ChartSpikeHistVert::export_ascii(LPTSTR lp)
{
	// print all ordinates line-by-line, different classes on same line
	lp += wsprintf(lp, _T("Histogram\nnbins=%i\nnclasses=%i"), n_bins_, histogram_ptr_array.GetSize());
	lp += wsprintf(lp, _T("\nmax=%i\nmin=%i"), abscissa_max_val_, abscissa_min_val_);
	// export classes & points
	lp += wsprintf(lp, _T("classes;\n"));
	int i;
	for (i = 0; i < histogram_ptr_array.GetSize(); i++)
		lp += wsprintf(lp, _T("%i\t"), static_cast<int>((histogram_ptr_array[i])->GetAt(0)));
	lp--; // erase \t and replace with \n

	// loop through all points
	lp += wsprintf(lp, _T("\nvalues;\n"));
	for (auto j = 1; j <= n_bins_; j++)
	{
		for (i = 0; i < histogram_ptr_array.GetSize(); i++)
			lp += wsprintf(lp, _T("%i\t"), static_cast<int>((histogram_ptr_array[i])->GetAt(j)));
		lp--; // erase \t and replace with \n
		lp += wsprintf(lp, _T("\n"));
	}
	*lp = _T('\0');
	return lp;
}

void ChartSpikeHistVert::OnLButtonUp(const UINT n_flags, const CPoint point)
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

void ChartSpikeHistVert::OnLButtonDown(UINT n_flags, CPoint point)
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
	// if hit, then tell parent to select corresponding histogram (spike)
	dbwave_doc_->set_spike_hit(spike_hit_ = hit_curve_in_doc(point));
	if (spike_hit_.spike_index >= 0)
	{
		release_cursor();
		send_my_message(HINT_HIT_SPIKE, NULL); // post?
	}
}

void ChartSpikeHistVert::zoom_data(CRect* prev_rect, CRect* new_rect)
{
	prev_rect->NormalizeRect();
	new_rect->NormalizeRect();

	// change y gain & y offset
	const auto y_we = y_we_; // save previous window extent
	y_we_ = MulDiv(y_we_, new_rect->Height(), prev_rect->Height());
	y_wo_ = y_wo_
		- MulDiv(prev_rect->top - y_vo_, y_we_, y_ve_)
		+ MulDiv(new_rect->top - y_vo_, y_we, y_ve_);

	// change index of first and last pt displayed
	const auto x_we = x_we_; // save previous window extent
	x_we_ = MulDiv(x_we_, new_rect->Width(), prev_rect->Width());
	x_wo_ = x_wo_
		- MulDiv(prev_rect->left - x_viewport_origin_, x_we_, x_viewport_extent_)
		+ MulDiv(new_rect->left - x_viewport_origin_, x_we, x_viewport_extent_);

	// display
	Invalidate();
	send_my_message(HINT_CHANGE_ZOOM, 0); // post?
}

void ChartSpikeHistVert::OnLButtonDblClk(const UINT n_flags, const CPoint point)
{
	if (spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(n_flags, point);
	else
	{
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),
		                         reinterpret_cast<LPARAM>(m_hWnd));
	}
}

int ChartSpikeHistVert::hit_curve(CPoint point)
{
	auto hit_spike = -1;
	// convert device coordinates into logical coordinates
	const auto delta_x = MulDiv(3, x_we_, x_viewport_extent_);
	const auto mouse_x = MulDiv(point.x - x_viewport_origin_, x_we_, x_viewport_extent_) + x_wo_;
	auto mouse_x1 = mouse_x - delta_x;
	auto mouse_x2 = mouse_x - delta_x;
	if (mouse_x1 < 1)
		mouse_x1 = 1;
	if (mouse_x1 > n_bins_)
		mouse_x1 = n_bins_;
	if (mouse_x2 < 1)
		mouse_x2 = 1;
	if (mouse_x2 > n_bins_)
		mouse_x2 = n_bins_;

	const auto delta_y = MulDiv(3, y_we_, y_ve_);
	const auto mouse_y = static_cast<DWORD>(MulDiv(point.y - y_vo_, y_we_, y_ve_)) + y_wo_ + delta_y;

	// test selected histogram first (foreground)
	const CDWordArray* p_dw = nullptr;
	if (plot_mode_ == PLOT_ONE_CLASS || plot_mode_ == PLOT_ONE_CLASS_ONLY)
	{
		auto i_hist = 1;
		// get array corresponding to selected_class_ as well as histogram index
		for (auto i = 1; i < histogram_ptr_array.GetSize(); i++)
		{
			if (static_cast<int>((histogram_ptr_array[i])->GetAt(0)) == selected_class_)
			{
				p_dw = histogram_ptr_array[i];
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
					hit_spike = i_hist;
					break;
				}
			}
		}
	}

	// test other histograms
	if (plot_mode_ != PLOT_ONE_CLASS_ONLY && hit_spike < 0)
	{
		for (auto j_hist = 1; j_hist < histogram_ptr_array.GetSize() && hit_spike < 0; j_hist++)
		{
			p_dw = histogram_ptr_array.GetAt(j_hist);
			if (plot_mode_ == PLOT_ONE_CLASS && static_cast<int>(p_dw->GetAt(0)) == selected_class_)
				continue;
			for (auto i = mouse_x1; i <= mouse_x2; i++)
			{
				const auto iww = p_dw->GetAt(i - 1);
				if (mouse_y <= iww)
				{
					hit_spike = j_hist;
					break;
				}
			}
		}
	}
	return hit_spike;
}

void ChartSpikeHistVert::get_extents()
{
	if (x_we_ == 1) // && m_xWO == 0)
	{
		if (m_lmax_ == 0)
			get_histogram_limits(0);
		x_we_ = static_cast<int>(m_lmax_);
		x_wo_ = 0;
	}

	if (y_we_ == 1) // && m_yWO == 0)
	{
		y_we_ = abscissa_max_val_ - abscissa_min_val_ + 1;
		y_wo_ = abscissa_min_val_;
	}
}

void ChartSpikeHistVert::get_histogram_limits(const int histogram_index)
{
	// for some unknown reason, m_pHist array is set at zero when arriving here
	if (histogram_ptr_array.GetSize() <= 0)
	{
		const auto p_dw = new (CDWordArray); // init array
		ASSERT(p_dw != NULL);
		histogram_ptr_array.Add(p_dw); // save pointer to this new array
		ASSERT(histogram_ptr_array.GetSize() > 0);
	}
	const auto p_dw = histogram_ptr_array[histogram_index];
	if (p_dw->GetSize() <= 1)
		return;
	// search position of min and max ; set cells outside as zero

	m_i_first_ = 1; // search first interval with data
	while (m_i_first_ <= n_bins_ && p_dw->GetAt(m_i_first_) == 0)
		m_i_first_++;

	m_i_last_ = n_bins_; // search last interval with data
	while (p_dw->GetAt(m_i_last_) == 0 && m_i_last_ > m_i_first_)
		m_i_last_--;

	// get index of max
	m_imax_ = m_i_first_; // index first pt
	m_lmax_ = 0; // max
	for (auto i = m_i_first_; i <= m_i_last_; i++)
	{
		const auto dw_item = p_dw->GetAt(i);
		if (dw_item > m_lmax_)
		{
			m_imax_ = i;
			m_lmax_ = p_dw->GetAt(i);
		}
	}
}

// 	size_and_clear_histograms()
// parameters
//		CWordArray* pVal	- word array source data
//		CDWordArray	pTime	- dword array with time intervals assoc with pVal
//		long l_first			= index first pt from file
//		long l_last 			= index last pt from file
//		int max				= maximum
//		int min				= minimum
//		int n_bins			= number of bins -> bin size
//		BOOL bNew=TRUE		= erase old data (TRUE) or add to old value (FALSE)

void ChartSpikeHistVert::size_and_clear_histograms(const int n_bins, const int max, const int min)
{
	bin_size_ = (max - min + 1) / n_bins + 1; // set bin size
	abscissa_min_val_ = min; // set min
	abscissa_max_val_ = min + n_bins * bin_size_; // set max

	n_bins_ = n_bins;
	for (auto j = histogram_ptr_array.GetUpperBound(); j >= 0; j--)
	{
		const auto p_dw = histogram_ptr_array[j];
		p_dw->SetSize(n_bins + 1);
		// erase all data from histogram
		for (auto i = 1; i <= n_bins; i++)
			p_dw->SetAt(i, 0);
	}
}

void ChartSpikeHistVert::OnSize(const UINT n_type, const int cx, const int cy)
{
	ChartSpike::OnSize(n_type, cx, cy);
	y_vo_ = cy;
}

CDWordArray* ChartSpikeHistVert::init_class_array(const int n_bins, const int spike_class)
{
	auto p_dw = new (CDWordArray); // init array
	ASSERT(p_dw != NULL);
	histogram_ptr_array.Add(p_dw); // save pointer to this new array
	p_dw->SetSize(n_bins + 1);
	for (auto j = 1; j <= n_bins; j++)
		p_dw->SetAt(j, 0);
	p_dw->SetAt(0, spike_class);
	return p_dw;
}

void ChartSpikeHistVert::build_hist_from_spike_list(SpikeList* p_spk_list, const long l_first, const long l_last, const int max, const int min, const int n_bins, const BOOL b_new)
{
	// erase data and arrays if new:
	if (b_new)
		remove_hist_data();

	if (histogram_ptr_array.GetSize() <= 0)
	{
		const auto p_dword_array = new (CDWordArray);
		ASSERT(p_dword_array != NULL);
		histogram_ptr_array.Add(p_dword_array);
		ASSERT(histogram_ptr_array.GetSize() > 0);
	}
	auto* p_dword_array = histogram_ptr_array[0];
	if (n_bins == 0)
	{
		return;
	}

	if (n_bins != n_bins_ || p_dword_array->GetSize() != (n_bins + 1))
		size_and_clear_histograms(n_bins, max, min);

	CDWordArray* p_dw = nullptr;
	const auto n_spikes = p_spk_list->get_spikes_count();
	for (auto i_spk = 0; i_spk < n_spikes; i_spk++)
	{
		const auto spike_element = p_spk_list->get_spike(i_spk);

		// check that the corresponding spike fits within the time limits requested
		const auto ii_time = spike_element->get_time();
		if (ii_time < l_first || ii_time > l_last)
			continue;
		const auto y1 = spike_element->get_y1();
		if (y1 > abscissa_max_val_ || y1 < abscissa_min_val_)
			continue;

		// increment corresponding histogram interval into the first histogram (general, displayed in grey)
		const auto index = (y1 - abscissa_min_val_) / bin_size_ + 1;
		auto dw_data = p_dword_array->GetAt(index) + 1;
		p_dword_array->SetAt(index, dw_data);

		// dispatch into corresponding class histogram (create one if necessary)
		const auto spike_class = spike_element->get_class_id();
		get_class_array(spike_class, p_dw);
		if (p_dw == nullptr)
			p_dw = init_class_array(n_bins, spike_class);

		if (p_dw != nullptr)
		{
			dw_data = p_dw->GetAt(index) + 1;
			p_dw->SetAt(index, dw_data);
		}
	}
	get_histogram_limits(0);
}

void ChartSpikeHistVert::build_hist_from_document(CdbWaveDoc* p_doc, const BOOL b_all_files, const long l_first, const long l_last, const int max, const int min, const int n_bins, BOOL b_new)
{
	// erase data and arrays if new:
	if (b_new)
	{
		remove_hist_data();
		b_new = false;
	}

	auto current_file = 0;
	auto file_first = current_file;
	auto file_last = current_file;
	if (b_all_files)
	{
		file_first = 0;
		file_last = p_doc->db_get_records_count() - 1;
		current_file = p_doc->db_get_current_record_position();
	}

	for (auto i_file = file_first; i_file <= file_last; i_file++)
	{
		if (b_all_files)
		{
			if (p_doc->db_set_current_record_position(i_file))
				p_doc->open_current_spike_file();
		}
		SpikeList* p_spike_list = p_doc->m_p_spk_doc->get_spike_list_current();
		if (p_spike_list != nullptr && p_spike_list->get_spikes_count() > 0)
			build_hist_from_spike_list(p_spike_list, l_first, l_last, max, min, n_bins, b_new);
	}

	if (b_all_files)
	{
		if (p_doc->db_set_current_record_position(current_file))
			p_doc->open_current_spike_file();
	}
}
