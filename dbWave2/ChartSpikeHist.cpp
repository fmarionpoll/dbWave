
// Purpose:
// 		contains and display histograms
// 		histogram bins are stored into an array of "long"
// 		histograms are computed from various sources of data:
//			Acqdata document
//			Spike data
//

#include "StdAfx.h"
#include "ChartSpikeHist.h"

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
{
	set_b_use_dib(FALSE);
	m_csEmpty = _T("no \n_spikes (spikehist)");
}

ChartSpikeHist::~ChartSpikeHist()
{
	delete_histogram_data();
}

void ChartSpikeHist::delete_histogram_data()
{
	if (histogram_ptr_array.GetSize() > 0) 
	{
		for (auto i = histogram_ptr_array.GetUpperBound(); i >= 0; i--)
			delete histogram_ptr_array[i];
		histogram_ptr_array.RemoveAll();
	}
}

void ChartSpikeHist::plot_data_to_dc(CDC* p_dc)
{
	if (m_displayRect.right <= 0 && m_displayRect.bottom <= 0)
	{
		CRect r;
		GetWindowRect(&r);
		OnSize(SIZE_RESTORED, r.Width(), r.Height());
	}
	if (m_erasebkgnd)
		erase_background(p_dc);

	// load resources
	CRect rect1;
	GetWindowRect(rect1);
	m_yVO = rect1.Height();

	get_extents();
	if (m_lmax == 0)
	{
		p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
		auto rect2 = m_displayRect;
		rect2.DeflateRect(1, 1);
		const auto text_length = m_csEmpty.GetLength();
		p_dc->DrawText(m_csEmpty, text_length, rect2, DT_LEFT);
		return;
	}
	const int n_saved_dc = p_dc->SaveDC();
	prepare_dc(p_dc);
	int color;
	// save background color which is changed by later calls to FillSolidRect
	// when doing so, pens created with PS_DOT pattern and with XOR_PEN do
	// not work properly. Restoring the background color solves the pb.
	const auto background_color = p_dc->GetBkColor();
	switch (m_plotmode)
	{
	case PLOT_BLACK:
	case PLOT_ONECLASSONLY:
		color = BLACK_COLOR;
		break;
	default:
		color = SILVER_COLOR;
		break;
	}

	//loop to display all histograms (but not the selected one)
	for (auto i_histogram = 0; i_histogram < histogram_ptr_array.GetSize(); i_histogram++)
	{
		const auto p_dw = histogram_ptr_array.GetAt(i_histogram);
		if (0 == p_dw->GetSize())
			continue;

		// select correct color
		if (i_histogram > 0)
		{
			const auto spike_class = static_cast<int>(p_dw->GetAt(0));
			color = BLACK_COLOR;
			if (PLOT_ONECLASSONLY == m_plotmode && spike_class != m_selected_class)
				continue;
			if (PLOT_CLASSCOLORS == m_plotmode)
				color = spike_class % NB_COLORS;
			else if (m_plotmode == PLOT_ONECLASS && spike_class == m_selected_class)
			{
				color = BLACK_COLOR;
				continue;
			}
		}

		plot_histogram(p_dc, p_dw, color);
	}

	// plot selected class (one histogram)
	if (m_plotmode == PLOT_ONECLASS)
	{
		color = BLACK_COLOR;
		CDWordArray* p_dw = nullptr;
		getClassArray(m_selected_class, p_dw);
		if (p_dw != nullptr)
		{
			plot_histogram(p_dc, p_dw, color);
		}
	}

	// display cursors
	p_dc->SetBkColor(background_color); // restore background color
	if (m_HZtags.get_tag_list_size() > 0) // display horizontal tags
		DisplayHZtags(p_dc);
	if (m_VTtags.get_tag_list_size() > 0) // display vertical tags
		DisplayVTtags_Value(p_dc);
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeHist::plot_histogram(CDC* p_dc, const CDWordArray* p_dw, const int color) const
{
	CRect rect_histogram;
	rect_histogram.left = m_abcissaminval - m_binsize;
	rect_histogram.right = m_abcissaminval;
	rect_histogram.bottom = 0;
	for (auto i = 1; i < p_dw->GetSize(); i++)
	{
		rect_histogram.left += m_binsize;
		rect_histogram.right += m_binsize;
		rect_histogram.top = static_cast<int>(p_dw->GetAt(i));
		if (rect_histogram.top > 0)
		{
			p_dc->MoveTo(rect_histogram.left, rect_histogram.bottom);
			p_dc->FillSolidRect(rect_histogram, m_colorTable[color]);
		}
	}
}

void ChartSpikeHist::MoveHZtagtoVal(const int tag_index, const int value)
{
	m_ptLast.y = MulDiv(m_HZtags.get_value(tag_index) - m_yWO, m_yVE, m_yWE) + m_yVO;
	const auto j = MulDiv(value - m_yWO, m_yVE, m_yWE) + m_yVO;
	xor_horizontal_tag(j);
	m_HZtags.set_tag_val(tag_index, value);
}

void ChartSpikeHist::MoveVTtagtoVal(int itag, int ival)
{
	m_ptLast.x = MulDiv(m_VTtags.get_value(itag) - m_xWO, m_xVE, m_xWE) + m_xVO;
	const auto j = MulDiv(ival - m_xWO, m_xVE, m_xWE) + m_xVO;
	xor_vertical_tag(j);
	m_VTtags.set_tag_val(itag, ival);
}

void ChartSpikeHist::getClassArray(const int i_class, CDWordArray*& p_dw)
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

LPTSTR ChartSpikeHist::export_ascii(LPTSTR lp)
{
	// print all ordinates line-by-line, different classes on same line
	lp += wsprintf(lp, _T("Histogram\nn_bins=%i\nnclasses=%i"), m_nbins, histogram_ptr_array.GetSize());
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

void ChartSpikeHist::OnLButtonUp(UINT nFlags, CPoint point)
{
	// test if horizontal tag was tracked
	switch (m_trackMode)
	{
	case TRACK_HZTAG:
		left_button_up_horizontal_tag(nFlags, point);
		break;

	case TRACK_VTTAG:
		// vertical tag was tracked
		{
			// convert pix into data value and back again
			const auto val = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
			m_VTtags.set_tag_val(m_HCtrapped, val);
			point.x = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
			xor_vertical_tag(point.x);
			ChartSpike::OnLButtonUp(nFlags, point);
			post_my_message(HINT_CHANGEVERTTAG, m_HCtrapped);
		}
		break;

	case TRACK_RECT:
		{
			ChartSpike::OnLButtonUp(nFlags, point); // else release mouse

			// none: zoom data or offset display
			ChartSpike::OnLButtonUp(nFlags, point);
			CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
			const int jitter = 3;
			if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
			{
				if (m_cursorType != CURSOR_ZOOM)
					post_my_message(HINT_HITAREA, NULL);
				else
					zoom_in();
				break; // exit: mouse movement was too small
			}

			// perform action according to cursor type
			auto rect_in = m_displayRect;
			switch (m_cursorType)
			{
			case 0:
				rect_out = rect_in;
				rect_out.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
				zoom_data(&rect_in, &rect_out);
				break;
			case CURSOR_ZOOM: // zoom operation
				zoom_data(&rect_in, &rect_out);
				m_ZoomFrom = rect_in;
				m_ZoomTo = rect_out;
				m_iUndoZoom = 1;
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

void ChartSpikeHist::OnLButtonDown(UINT nFlags, CPoint point)
{
	// compute pixel position of horizontal tags
	if (m_HZtags.get_tag_list_size() > 0)
	{
		for (auto icur = m_HZtags.get_tag_list_size() - 1; icur >= 0; icur--)
			m_HZtags.set_tag_pixel(icur, MulDiv(m_HZtags.get_value(icur) - m_yWO, m_yVE, m_yWE) + m_yVO);
	}
	// compute pixel position of vertical tags
	if (m_VTtags.get_tag_list_size() > 0)
	{
		for (auto icur = m_VTtags.get_tag_list_size() - 1; icur >= 0; icur--) // loop through all tags
			m_VTtags.set_tag_pixel(icur, MulDiv(m_VTtags.get_value(icur) - m_xWO, m_xVE, m_xWE) + m_xVO);
	}
	ChartSpike::OnLButtonDown(nFlags, point);
	if (m_currCursorMode != 0 || m_HCtrapped >= 0) // do nothing else if mode != 0
		return; // or any tag hit (VT, HZ) detected

	// test if mouse hit one histogram
	// if hit, then tell parent to select histogram (spike)
	m_hit_spike = hit_curve(point);
	if (m_hit_spike >= 0)
	{
		// cancel track rect mode
		m_trackMode = TRACK_OFF; // flag track_rect
		release_cursor(); // release cursor capture
		post_my_message(HINT_HITSPIKE, m_hit_spike);
	}
}

// zoom_data
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo= window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=yextent, ve=rect.height/2, vo = -rect.GetRectHeight()/2

void ChartSpikeHist::zoom_data(CRect* rFrom, CRect* rDest)
{
	rFrom->NormalizeRect();
	rDest->NormalizeRect();

	// change y gain & y offset
	const auto y_we = m_yWE; // save previous window extent
	m_yWE = MulDiv(m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
		- MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
		+ MulDiv(rDest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed
	const auto x_we = m_xWE; // save previous window extent
	m_xWE = MulDiv(m_xWE, rDest->Width(), rFrom->Width());
	m_xWO = m_xWO
		- MulDiv(rFrom->left - m_xVO, m_xWE, m_xVE)
		+ MulDiv(rDest->left - m_xVO, x_we, m_xVE);

	// display
	Invalidate();
	post_my_message(HINT_CHANGEZOOM, 0);
}

void ChartSpikeHist::OnLButtonDblClk(const UINT n_flags, const CPoint point)
{
	if (m_hit_spike < 0)
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
	const auto delta_x = MulDiv(3, m_xWE, m_xVE);
	const auto mouse_x = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
	auto mouse_x1 = mouse_x - delta_x;
	auto mouse_x2 = mouse_x - delta_x;
	if (mouse_x1 < 1)
		mouse_x1 = 1;
	if (mouse_x1 > m_nbins)
		mouse_x1 = m_nbins;
	if (mouse_x2 < 1)
		mouse_x2 = 1;
	if (mouse_x2 > m_nbins)
		mouse_x2 = m_nbins;

	const auto delta_y = MulDiv(3, m_yWE, m_yVE);
	const auto mouse_y = static_cast<DWORD>(MulDiv(point.y - m_yVO, m_yWE, m_yVE)) + m_yWO + delta_y;

	// test selected histogram first (foreground)
	const CDWordArray* p_dw = nullptr;
	if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECLASSONLY)
	{
		auto i_hist = 1;
		// get array corresponding to m_selected_class as well as histogram index
		for (auto i = 1; i < histogram_ptr_array.GetSize(); i++)
		{
			if (static_cast<int>((histogram_ptr_array[i])->GetAt(0)) == m_selected_class)
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
					hit_spk = i_hist;
					break;
				}
			}
		}
	}

	// test other histograms
	if (m_plotmode != PLOT_ONECLASSONLY && hit_spk < 0)
	{
		for (auto j_hist = 1; j_hist < histogram_ptr_array.GetSize() && hit_spk < 0; j_hist++)
		{
			p_dw = histogram_ptr_array.GetAt(j_hist);
			if (m_plotmode == PLOT_ONECLASS && static_cast<int>(p_dw->GetAt(0)) == m_selected_class)
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
	if (m_yWE == 1) // && m_yWO == 0)
	{
		if (m_lmax == 0)
			get_histogram_limits(0);
		m_yWE = static_cast<int>(m_lmax);
		m_yWO = 0;
	}

	if (m_xWE == 1) // && m_xWO == 0)
	{
		m_xWE = m_abcissamaxval - m_abcissaminval + 1;
		m_xWO = m_abcissaminval;
	}
}

void ChartSpikeHist::get_histogram_limits(const int i_hist)
{
	// for some unknown reason, m_p_Hist_array is set at zero when arriving here
	if (histogram_ptr_array.GetSize() <= 0)
	{
		const auto p_dw = new (CDWordArray); // init array
		ASSERT(p_dw != NULL);
		histogram_ptr_array.Add(p_dw); // save pointer to this new array
		ASSERT(histogram_ptr_array.GetSize() > 0);
	}
	const auto p_dw = histogram_ptr_array[i_hist];
	if (p_dw->GetSize() <= 1)
		return;

	// get index of minimal value
	m_ifirst = 1; // search first interval with data
	while (m_ifirst <= m_nbins && p_dw->GetAt(m_ifirst) == 0)
		m_ifirst++;

	m_ilast = m_nbins; // search last interval with data
	while (p_dw->GetAt(m_ilast) == 0 && m_ilast > m_ifirst)
		m_ilast--;

	// get index of maximum value
	m_imax = m_ifirst; // index first pt
	m_lmax = 0; // max
	for (auto i = m_ifirst; i <= m_ilast; i++)
	{
		const auto dw_item = p_dw->GetAt(i);
		if (dw_item > m_lmax)
		{
			m_imax = i;
			m_lmax = p_dw->GetAt(i);
		}
	}
}

void ChartSpikeHist::resize_and_clear_histograms(const int n_bins, const int max, const int min)
{
	m_binsize = (max - min + 1) / n_bins + 1; 
	m_abcissaminval = min;
	m_abcissamaxval = min + n_bins * m_binsize; 

	m_nbins = n_bins;
	for (auto j = histogram_ptr_array.GetUpperBound(); j >= 0; j--)
	{
		const auto p_dw = histogram_ptr_array[j];
		p_dw->SetSize(n_bins + 1);
		for (auto i = 1; i <= n_bins; i++)
			p_dw->SetAt(i, 0);
	}
}

void ChartSpikeHist::OnSize(UINT nType, int cx, int cy)
{
	ChartSpike::OnSize(nType, cx, cy);
	m_yVO = cy;
}

CDWordArray* ChartSpikeHist::init_class_array(const int n_bins, const int spike_class)
{
	auto p_dw = new (CDWordArray);
	ASSERT(p_dw != NULL);
	histogram_ptr_array.Add(p_dw); // save pointer to this new array
	p_dw->SetSize(n_bins + 1);
	for (auto j = 1; j <= n_bins; j++)
		p_dw->SetAt(j, 0);
	p_dw->SetAt(0, spike_class);
	return p_dw;
}

void ChartSpikeHist::build_hist_from_spike_list(SpikeList* p_spk_list, const long l_first, const long l_last, const int max, const int min,
                                                const int n_bins, const BOOL b_new)
{
	// erase data and arrays if bnew:
	if (b_new)
		delete_histogram_data();

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

	if (n_bins != m_nbins || p_dword_array->GetSize() != (n_bins + 1))
		resize_and_clear_histograms(n_bins, max, min);

	CDWordArray* p_dw = nullptr;
	auto n_spikes = p_spk_list->get_spikes_count();
	for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		const auto spike_element = p_spk_list->get_spike(i_spike);
		const auto ii_time = spike_element->get_time();
		if (ii_time < l_first || ii_time > l_last)
			continue;
		auto y1 = spike_element->get_y1();
		if (y1 > m_abcissamaxval || y1 < m_abcissaminval)
			continue;

		// increment corresponding histogram interval in the first histogram (general, displayed in grey)
		auto index = (y1 - m_abcissaminval) / m_binsize + 1;
		if (index >= p_dword_array->GetSize())
			index = p_dword_array->GetSize() - 1;
		auto dw_data = p_dword_array->GetAt(index) + 1;
		p_dword_array->SetAt(index, dw_data);

		// dispatch into corresponding class histogram (create one if necessary)
		const auto spike_class = spike_element->get_class_id();
		getClassArray(spike_class, p_dw);
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

// 	BuildHistFromDocument()
// parameters
//		CdbWaveDoc* p_doc	- dbWave document
//		BOOL ballFiles		- if false, compute only from current spikelist, otherwise compute over the whole document
//		long l_first		= index first pt from file
//		long l_last 		= index last pt from file
//		int max				= maximum
//		int min				= minimum
//		int n_bins			= number of bins -> bin size
//		BOOL bNew=TRUE		= erase old data (TRUE) or add to old value (FALSE)

void ChartSpikeHist::build_hist_from_document(CdbWaveDoc* p_doc, const BOOL b_all_files, const long l_first, const long l_last,
                                              const int max, const int min, const int n_bins, BOOL b_new)
{
	// erase data and arrays if b_new:
	if (b_new)
	{
		delete_histogram_data();
		b_new = false;
	}

	auto current_file = 0;
	auto file_first = current_file;
	auto file_last = current_file;
	if (b_all_files)
	{
		file_first = 0;
		file_last = p_doc->db_get_n_records() - 1;
		current_file = p_doc->db_get_current_record_position();
	}

	for (auto i_file = file_first; i_file <= file_last; i_file++)
	{
		if (b_all_files)
		{
			if (p_doc->db_set_current_record_position(i_file))
				p_doc->open_current_spike_file();
		}
		SpikeList* p_spike_list = p_doc->m_p_spk->get_spike_list_current();
		if (p_spike_list != nullptr && p_spike_list->get_spikes_count() > 0)
			build_hist_from_spike_list(p_spike_list, l_first, l_last, max, min, n_bins, b_new);
	}

	if (b_all_files)
	{
		if (p_doc->db_set_current_record_position(current_file))
			p_doc->open_current_spike_file();
	}
}
