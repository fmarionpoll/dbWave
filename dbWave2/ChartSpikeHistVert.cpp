// Purpose:
// 		contains and display histograms
// 		histogram bins are stored into an array of "long"
// 		histograms are computed from various sources of data:
//			Acqdata document
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
{
	set_b_use_dib(FALSE);
	cs_empty_ = _T("no spikes (WndVerticalHistogram)");
}

ChartSpikeHistVert::~ChartSpikeHistVert()
{
	RemoveHistData();
}

void ChartSpikeHistVert::RemoveHistData()
{
	if (histogram_ptr_array.GetSize() > 0) // delete objects pointed by elements
	{
		// of m_pHistarray
		for (auto i = histogram_ptr_array.GetUpperBound(); i >= 0; i--)
			delete histogram_ptr_array[i];
		histogram_ptr_array.RemoveAll();
	}
}

void ChartSpikeHistVert::plot_data_to_dc(CDC* p_dc)
{
	if (m_display_rect_.right <= 0 && m_display_rect_.bottom <= 0)
	{
		CRect r;
		GetWindowRect(&r);
		OnSize(SIZE_RESTORED, r.Width(), r.Height());
	}
	if (b_erase_background_) // erase background
		erase_background(p_dc);

	// load resources
	getExtents();
	if (m_lmax == 0)
	{
		p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
		auto rect = m_display_rect_;
		rect.DeflateRect(1, 1);
		const auto textlen = cs_empty_.GetLength();
		p_dc->DrawText(cs_empty_, textlen, rect, DT_LEFT); //|DT_WORDBREAK);
		return;
	}
	const int n_saved_dc = p_dc->SaveDC();
	prepare_dc(p_dc);
	int color;
	// save background color which is changed by later calls to FillSolidRect
	// when doing so, pens created with PS_DOT pattern and with XOR_PEN do
	// not work properly. Restoring the background color solves the pb.
	const auto bkcolor = p_dc->GetBkColor();
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

	//loop to display all histograms (but not the selected one)
	CRect rect_histog;

	for (auto ihist = 0; ihist < histogram_ptr_array.GetSize(); ihist++)
	{
		const auto p_dw = histogram_ptr_array.GetAt(ihist);
		if (0 == p_dw->GetSize())
			continue;

		// select correct color
		if (ihist > 0)
		{
			const auto spkcla = static_cast<int>(p_dw->GetAt(0));
			color = BLACK_COLOR;
			if (PLOT_ONE_CLASS_ONLY == plot_mode_ && spkcla != selected_class_)
				continue;
			if (PLOT_CLASS_COLORS == plot_mode_)
				color = spkcla % NB_COLORS;
			else if (plot_mode_ == PLOT_ONE_CLASS && spkcla == selected_class_)
			{
				color = BLACK_COLOR;
				continue;
			}
		}
		plotHistogram(p_dc, p_dw, color);
	}

	// plot selected class (one histogram)
	if (plot_mode_ == PLOT_ONE_CLASS)
	{
		color = BLACK_COLOR;
		CDWordArray* p_dw = nullptr;
		getClassArray(selected_class_, p_dw);
		if (p_dw != nullptr)
		{
			plotHistogram(p_dc, p_dw, color);
		}
	}

	// display cursors
	p_dc->SetBkColor(bkcolor); // restore background color
	if (horizontal_tags.get_tag_list_size() > 0) // display horizontal tags
		display_horizontal_tags(p_dc);
	if (vertical_tags.get_tag_list_size() > 0) // display vertical tags
		display_vertical_tags(p_dc);
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeHistVert::plotHistogram(CDC* p_dc, CDWordArray* p_dw, int color)
{
	CRect rect_histog;
	rect_histog.bottom = m_abcissaminval - m_binsize;
	rect_histog.top = m_abcissaminval;
	rect_histog.left = 0;
	for (auto i = 1; i < p_dw->GetSize(); i++)
	{
		rect_histog.bottom += m_binsize;
		rect_histog.top += m_binsize;

		rect_histog.right = static_cast<int>(p_dw->GetAt(i));
		if (rect_histog.top > 0)
		{
			p_dc->MoveTo(rect_histog.bottom, rect_histog.left);
			p_dc->FillSolidRect(rect_histog, color_table_[color]);
		}
	}
}

void ChartSpikeHistVert::MoveHZtagtoVal(int i, int val)
{
	m_pt_last_.y = MulDiv(horizontal_tags.get_value_int(i) - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_;
	const auto j = MulDiv(val - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_;
	xor_horizontal_tag(j);
	horizontal_tags.set_value_int(i, val);
}

void ChartSpikeHistVert::MoveVTtagtoVal(int itag, int ival)
{
	m_pt_last_.x = MulDiv(vertical_tags.get_value_int(itag) - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
	const auto j = MulDiv(ival - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
	xor_vertical_tag(j);
	vertical_tags.set_value_int(itag, ival);
}

void ChartSpikeHistVert::getClassArray(int iclass, CDWordArray*& pDW)
{
	// test if pDW at 0 position is the right one
	if ((nullptr != pDW) && (static_cast<int>(pDW->GetAt(0)) == iclass))
		return;

	// not found, scan the array
	pDW = nullptr;
	for (auto i = 1; i < histogram_ptr_array.GetSize(); i++)
	{
		if (static_cast<int>((histogram_ptr_array[i])->GetAt(0)) == iclass)
		{
			pDW = histogram_ptr_array[i];
			break;
		}
	}
}

LPTSTR ChartSpikeHistVert::ExportAscii(LPTSTR lp)
{
	// print all ordinates line-by-line, differnt classes on same line
	lp += wsprintf(lp, _T("Histogram\nnbins=%i\nnclasses=%i"), m_nbins, histogram_ptr_array.GetSize());
	lp += wsprintf(lp, _T("\nmax=%i\nmin=%i"), m_abcissamaxval, m_abcissaminval);
	// export classes & points
	lp += wsprintf(lp, _T("classes;\n"));
	int i;
	for (i = 0; i < histogram_ptr_array.GetSize(); i++)
		lp += wsprintf(lp, _T("%i\t"), static_cast<int>((histogram_ptr_array[i])->GetAt(0)));
	lp--; // erase \t and replace with \n

	// loop through all points
	lp += wsprintf(lp, _T("\nvalues;\n"));
	for (auto j = 1; j <= m_nbins; j++)
	{
		for (i = 0; i < histogram_ptr_array.GetSize(); i++)
			lp += wsprintf(lp, _T("%i\t"), static_cast<int>((histogram_ptr_array[i])->GetAt(j)));
		lp--; // erase \t and replace with \n
		lp += wsprintf(lp, _T("\n"));
	}
	*lp = _T('\0');
	return lp;
}

void ChartSpikeHistVert::OnLButtonUp(UINT nFlags, CPoint point)
{
	// test if horizontal tag was tracked
	switch (track_mode_)
	{
	case TRACK_HZ_TAG:
		left_button_up_horizontal_tag(nFlags, point);
		break;

	case TRACK_VT_TAG:
		// vertical tag was tracked
		{
			// convert pix into data value and back again
			const auto val = MulDiv(point.x - m_x_viewport_origin_, m_x_we_, m_x_viewport_extent_) + m_x_wo_;
			vertical_tags.set_value_int(hc_trapped_, val);
			point.x = MulDiv(val - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_;
			xor_vertical_tag(point.x);
			ChartSpike::OnLButtonUp(nFlags, point);
			post_my_message(HINT_CHANGE_VERT_TAG, hc_trapped_);
		}
		break;

	case TRACK_RECT:
		{
			ChartSpike::OnLButtonUp(nFlags, point); // else release mouse

			// none: zoom data or offset display
			ChartSpike::OnLButtonUp(nFlags, point);
			CRect rect_out(m_pt_first_.x, m_pt_first_.y, m_pt_last_.x, m_pt_last_.y);
			const int jitter = 3;
			if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
			{
				if (cursor_type_ != CURSOR_ZOOM)
					post_my_message(HINT_HIT_AREA, NULL);
				else
					zoom_in();
				break; // exit: mouse movement was too small
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
	default:
		break;
	}
}

void ChartSpikeHistVert::OnLButtonDown(UINT nFlags, CPoint point)
{
	// compute pixel position of horizontal tags
	if (horizontal_tags.get_tag_list_size() > 0)
	{
		for (auto icur = horizontal_tags.get_tag_list_size() - 1; icur >= 0; icur--)
			horizontal_tags.set_pixel(icur, MulDiv(horizontal_tags.get_value_int(icur) - m_y_wo_, m_y_viewport_extent_, m_y_we_) + m_y_viewport_origin_);
	}
	// compute pixel position of vertical tags
	if (vertical_tags.get_tag_list_size() > 0)
	{
		for (auto icur = vertical_tags.get_tag_list_size() - 1; icur >= 0; icur--) // loop through all tags
			vertical_tags.set_pixel(icur, MulDiv(vertical_tags.get_value_int(icur) - m_x_wo_, m_x_viewport_extent_, m_x_we_) + m_x_viewport_origin_);
	}
	ChartSpike::OnLButtonDown(nFlags, point);
	if (current_cursor_mode_ != 0 || hc_trapped_ >= 0) // do nothing else if mode != 0
		return; // or any tag hit (VT, HZ) detected

	// test if mouse hit one histogram
	// if hit, then tell parent to select corresp histogram (spike)
	dbwave_doc_->set_spike_hit(spike_hit_ = hit_curve_in_doc(point));
	if (spike_hit_.spike_index >= 0)
	{
		release_cursor();
		post_my_message(HINT_HIT_SPIKE, NULL);
	}
}

void ChartSpikeHistVert::zoom_data(CRect* rFrom, CRect* rDest)
{
	rFrom->NormalizeRect();
	rDest->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_y_we_; // save previous window extent
	m_y_we_ = MulDiv(m_y_we_, rDest->Height(), rFrom->Height());
	m_y_wo_ = m_y_wo_
		- MulDiv(rFrom->top - m_y_viewport_origin_, m_y_we_, m_y_viewport_extent_)
		+ MulDiv(rDest->top - m_y_viewport_origin_, y_we, m_y_viewport_extent_);

	// change index of first and last pt displayed
	const auto x_we = m_x_we_; // save previous window extent
	m_x_we_ = MulDiv(m_x_we_, rDest->Width(), rFrom->Width());
	m_x_wo_ = m_x_wo_
		- MulDiv(rFrom->left - m_x_viewport_origin_, m_x_we_, m_x_viewport_extent_)
		+ MulDiv(rDest->left - m_x_viewport_origin_, x_we, m_x_viewport_extent_);

	// display
	Invalidate();
	post_my_message(HINT_CHANGE_ZOOM, 0);
}

void ChartSpikeHistVert::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (spike_hit_.spike_index < 0)
		ChartSpike::OnLButtonDblClk(nFlags, point);
	else
	{
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),
		                         reinterpret_cast<LPARAM>(m_hWnd));
	}
}

int ChartSpikeHistVert::hit_curve(CPoint point)
{
	auto hitspk = -1;
	// convert device coordinates into logical coordinates
	const auto deltax = MulDiv(3, m_x_we_, m_x_viewport_extent_);
	const auto mouse_x = MulDiv(point.x - m_x_viewport_origin_, m_x_we_, m_x_viewport_extent_) + m_x_wo_;
	auto mouse_x1 = mouse_x - deltax;
	auto mouse_x2 = mouse_x - deltax;
	if (mouse_x1 < 1)
		mouse_x1 = 1;
	if (mouse_x1 > m_nbins)
		mouse_x1 = m_nbins;
	if (mouse_x2 < 1)
		mouse_x2 = 1;
	if (mouse_x2 > m_nbins)
		mouse_x2 = m_nbins;

	const auto deltay = MulDiv(3, m_y_we_, m_y_viewport_extent_);
	const auto mouse_y = static_cast<DWORD>(MulDiv(point.y - m_y_viewport_origin_, m_y_we_, m_y_viewport_extent_)) + m_y_wo_ + deltay;

	// test selected histogram first (foreground)
	auto ihist = 1;
	CDWordArray* pDW = nullptr;
	if (plot_mode_ == PLOT_ONE_CLASS || plot_mode_ == PLOT_ONE_CLASS_ONLY)
	{
		// get array corresp to selected_class_ as well as histogram index
		for (auto i = 1; i < histogram_ptr_array.GetSize(); i++)
		{
			if (static_cast<int>((histogram_ptr_array[i])->GetAt(0)) == selected_class_)
			{
				pDW = histogram_ptr_array[i];
				ihist = i;
				break;
			}
		}
		//
		if (pDW != nullptr)
		{
			for (auto i = mouse_x1; i < mouse_x2; i++)
			{
				const auto iww = pDW->GetAt(i - 1);
				if (mouse_y <= iww)
				{
					hitspk = ihist;
					break;
				}
			}
		}
	}

	// test other histograms
	if (plot_mode_ != PLOT_ONE_CLASS_ONLY && hitspk < 0)
	{
		for (auto jhist = 1; jhist < histogram_ptr_array.GetSize() && hitspk < 0; jhist++)
		{
			pDW = histogram_ptr_array.GetAt(jhist);
			if (plot_mode_ == PLOT_ONE_CLASS && static_cast<int>(pDW->GetAt(0)) == selected_class_)
				continue;
			for (auto i = mouse_x1; i <= mouse_x2; i++)
			{
				const auto iww = pDW->GetAt(i - 1);
				if (mouse_y <= iww)
				{
					hitspk = jhist;
					break;
				}
			}
		}
	}
	return hitspk;
}

void ChartSpikeHistVert::getExtents()
{
	if (m_x_we_ == 1) // && m_xWO == 0)
	{
		if (m_lmax == 0)
			getHistogLimits(0);
		m_x_we_ = static_cast<int>(m_lmax);
		m_x_wo_ = 0;
	}

	if (m_y_we_ == 1) // && m_yWO == 0)
	{
		m_y_we_ = m_abcissamaxval - m_abcissaminval + 1;
		m_y_wo_ = m_abcissaminval;
	}
}

void ChartSpikeHistVert::getHistogLimits(int ihist)
{
	// for some unknown reason, m_pHistarray is set at zero when arriving here
	if (histogram_ptr_array.GetSize() <= 0)
	{
		const auto p_dw = new (CDWordArray); // init array
		ASSERT(p_dw != NULL);
		histogram_ptr_array.Add(p_dw); // save pointer to this new array
		ASSERT(histogram_ptr_array.GetSize() > 0);
	}
	const auto p_dw = histogram_ptr_array[ihist];
	if (p_dw->GetSize() <= 1)
		return;
	// Recherche de l'indice min et max de l'histograme
	// En dessous de min toutes les cases du tableau sont
	// à zéro. Au dela de max toute les cases du tableau sont à zéro.

	m_ifirst = 1; // search first interval with data
	while (m_ifirst <= m_nbins && p_dw->GetAt(m_ifirst) == 0)
		m_ifirst++;

	m_ilast = m_nbins; // search last interval with data
	while (p_dw->GetAt(m_ilast) == 0 && m_ilast > m_ifirst)
		m_ilast--;

	// Récuperation de l'indice du maximum
	m_imax = m_ifirst; // index first pt
	m_lmax = 0; // max
	for (auto i = m_ifirst; i <= m_ilast; i++)
	{
		const auto dwitem = p_dw->GetAt(i);
		if (dwitem > m_lmax)
		{
			m_imax = i;
			m_lmax = p_dw->GetAt(i);
		}
	}
}

// 	BuildHistogFromDoc()
// parameters
//		CWordArray* pVal	- word array source data
//		CDWordArray	pTime	- dword array with time intervals assoc with pVal
//		long l_first			= index first pt from file
//		long l_last 			= index last pt from file
//		int max				= maximum
//		int min				= minimum
//		int nbins			= number of bins -> bin size
//		BOOL bNew=TRUE		= erase old data (TRUE) or add to old value (FALSE)

void ChartSpikeHistVert::reSize_And_Clear_Histograms(int nbins, int max, int min)
{
	m_binsize = (max - min + 1) / nbins + 1; // set bin size
	m_abcissaminval = min; // set min
	m_abcissamaxval = min + nbins * m_binsize; // set max

	m_nbins = nbins;
	for (auto j = histogram_ptr_array.GetUpperBound(); j >= 0; j--)
	{
		auto p_dw = histogram_ptr_array[j];
		p_dw->SetSize(nbins + 1);
		// erase all data from histogram
		for (auto i = 1; i <= nbins; i++)
			p_dw->SetAt(i, 0);
	}
}

void ChartSpikeHistVert::OnSize(UINT nType, int cx, int cy)
{
	ChartSpike::OnSize(nType, cx, cy);
	m_y_viewport_origin_ = cy;
}

CDWordArray* ChartSpikeHistVert::initClassArray(int nbins, int spike_class)
{
	auto p_dw = new (CDWordArray); // init array
	ASSERT(p_dw != NULL);
	histogram_ptr_array.Add(p_dw); // save pointer to this new array
	p_dw->SetSize(nbins + 1);
	for (auto j = 1; j <= nbins; j++)
		p_dw->SetAt(j, 0);
	p_dw->SetAt(0, spike_class);
	return p_dw;
}

void ChartSpikeHistVert::buildHistFromSpikeList(SpikeList* p_spk_list, long l_first, long l_last, int max, int min,
                                                 int nbins, BOOL bNew)
{
	// erase data and arrays if bnew:
	if (bNew)
		RemoveHistData();

	if (histogram_ptr_array.GetSize() <= 0)
	{
		const auto p_dword_array = new (CDWordArray);
		ASSERT(p_dword_array != NULL);
		histogram_ptr_array.Add(p_dword_array);
		ASSERT(histogram_ptr_array.GetSize() > 0);
	}
	auto* p_dword_array = histogram_ptr_array[0];
	if (nbins == 0)
	{
		return;
	}

	if (nbins != m_nbins || p_dword_array->GetSize() != (nbins + 1))
		reSize_And_Clear_Histograms(nbins, max, min);

	CDWordArray* p_dw = nullptr;
	auto nspikes = p_spk_list->get_spikes_count();
	for (auto ispk = 0; ispk < nspikes; ispk++)
	{
		const auto spike_element = p_spk_list->get_spike(ispk);

		// check that the corresp spike fits within the time limits requested
		const auto ii_time = spike_element->get_time();
		if (ii_time < l_first || ii_time > l_last)
			continue;
		auto y1 = spike_element->get_y1();
		if (y1 > m_abcissamaxval || y1 < m_abcissaminval)
			continue;

		// increment corresponding histogram interval into the first histogram (general, displayed in grey)
		const auto index = (y1 - m_abcissaminval) / m_binsize + 1;
		auto dw_data = p_dword_array->GetAt(index) + 1;
		p_dword_array->SetAt(index, dw_data);

		// dispatch into corresp class histogram (create one if necessary)
		const auto spike_class = spike_element->get_class_id();
		getClassArray(spike_class, p_dw);
		if (p_dw == nullptr)
			p_dw = initClassArray(nbins, spike_class);

		if (p_dw != nullptr)
		{
			dw_data = p_dw->GetAt(index) + 1;
			p_dw->SetAt(index, dw_data);
		}
	}
	getHistogLimits(0);
}

void ChartSpikeHistVert::BuildHistFromDocument(CdbWaveDoc* p_doc, BOOL ballFiles, long l_first, long l_last, int max,
                                                int min, int nbins, BOOL bNew)
{
	// erase data and arrays if bnew:
	if (bNew)
	{
		RemoveHistData();
		bNew = false;
	}

	auto currentfile = 0;
	auto file_first = currentfile;
	auto file_last = currentfile;
	if (ballFiles)
	{
		file_first = 0;
		file_last = p_doc->db_get_n_records() - 1;
		currentfile = p_doc->db_get_current_record_position();
	}

	for (auto ifile = file_first; ifile <= file_last; ifile++)
	{
		if (ballFiles)
		{
			p_doc->db_set_current_record_position(ifile);
			p_doc->open_current_spike_file();
		}
		SpikeList* p_spikelist = p_doc->m_p_spk->get_spike_list_current();
		if (p_spikelist != nullptr && p_spikelist->get_spikes_count() > 0)
			buildHistFromSpikeList(p_spikelist, l_first, l_last, max, min, nbins, bNew);
	}

	if (ballFiles)
	{
		p_doc->db_set_current_record_position(currentfile);
		p_doc->open_current_spike_file();
	}
}
