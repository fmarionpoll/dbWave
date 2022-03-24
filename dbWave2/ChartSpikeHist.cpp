
// Purpose:
// 		contains and display histograms
// 		histogram bins are stored into an array of "long"
// 		histograms are computed from various sources of data:
//			Acqdata document
//			Spike data
// structure:
// 		CWnd derived : MFC control.
// 		CChartWnd derived : FMP enriched control (mouse cursors, HZTags)
// 		evolved from CLineviewButton
//

#include "StdAfx.h"
#include "ChartWnd.h"
#include "Spikedoc.h"
#include "ChartSpikeHist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ChartSpikeHist, ChartWnd)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
END_MESSAGE_MAP()

ChartSpikeHist::ChartSpikeHist()
{
	SetbUseDIB(FALSE);
	m_csEmpty = _T("no \n_spikes (spikehist)");
}

ChartSpikeHist::~ChartSpikeHist()
{
	RemoveHistData();
}

void ChartSpikeHist::RemoveHistData()
{
	if (histogram_ptr_array.GetSize() > 0) 
	{
		for (auto i = histogram_ptr_array.GetUpperBound(); i >= 0; i--)
			delete histogram_ptr_array[i];
		histogram_ptr_array.RemoveAll();
	}
}

void ChartSpikeHist::PlotDatatoDC(CDC* p_dc)
{
	if (m_displayRect.right <= 0 && m_displayRect.bottom <= 0)
	{
		CRect r;
		GetWindowRect(&r);
		OnSize(SIZE_RESTORED, r.Width(), r.Height());
	}
	if (m_erasebkgnd)
		EraseBkgnd(p_dc);

	// load resources
	CRect rect1;
	GetWindowRect(rect1);
	m_yVO = rect1.Height();

	getExtents();
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
	prepareDC(p_dc);
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
			if (PLOT_ONECLASSONLY == m_plotmode && spike_class != m_selclass)
				continue;
			if (PLOT_CLASSCOLORS == m_plotmode)
				color = spike_class % NB_COLORS;
			else if (m_plotmode == PLOT_ONECLASS && spike_class == m_selclass)
			{
				color = BLACK_COLOR;
				continue;
			}
		}

		plotHistogram(p_dc, p_dw, color);
	}

	// plot selected class (one histogram)
	if (m_plotmode == PLOT_ONECLASS)
	{
		color = BLACK_COLOR;
		CDWordArray* p_dw = nullptr;
		getClassArray(m_selclass, p_dw);
		if (p_dw != nullptr)
		{
			plotHistogram(p_dc, p_dw, color);
		}
	}

	// display cursors
	p_dc->SetBkColor(background_color); // restore background color
	if (m_HZtags.GetNTags() > 0) // display horizontal tags
		DisplayHZtags(p_dc);
	if (m_VTtags.GetNTags() > 0) // display vertical tags
		DisplayVTtags_Value(p_dc);
	p_dc->RestoreDC(n_saved_dc);
}

void ChartSpikeHist::plotHistogram(CDC* p_dc, CDWordArray* p_dw, int color)
{
	CRect rect_histog;
	rect_histog.left = m_abcissaminval - m_binsize;
	rect_histog.right = m_abcissaminval;
	rect_histog.bottom = 0;
	for (auto i = 1; i < p_dw->GetSize(); i++)
	{
		rect_histog.left += m_binsize;
		rect_histog.right += m_binsize;
		rect_histog.top = static_cast<int>(p_dw->GetAt(i));
		if (rect_histog.top > 0)
		{
			p_dc->MoveTo(rect_histog.left, rect_histog.bottom);
			p_dc->FillSolidRect(rect_histog, m_colorTable[color]);
		}
	}
}

void ChartSpikeHist::MoveHZtagtoVal(int i, int val)
{
	m_ptLast.y = MulDiv(m_HZtags.GetValue(i) - m_yWO, m_yVE, m_yWE) + m_yVO;
	const auto j = MulDiv(val - m_yWO, m_yVE, m_yWE) + m_yVO;
	XorHZtag(j);
	m_HZtags.SetTagVal(i, val);
}

void ChartSpikeHist::MoveVTtagtoVal(int itag, int ival)
{
	m_ptLast.x = MulDiv(m_VTtags.GetValue(itag) - m_xWO, m_xVE, m_xWE) + m_xVO;
	const auto j = MulDiv(ival - m_xWO, m_xVE, m_xWE) + m_xVO;
	XorVTtag(j);
	m_VTtags.SetTagVal(itag, ival);
}

void ChartSpikeHist::getClassArray(int iclass, CDWordArray*& pDW)
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

LPTSTR ChartSpikeHist::ExportAscii(LPTSTR lp)
{
	// print all ordinates line-by-line, differnt classes on same line
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
		lbuttonUp_HzTag(nFlags, point);
		break;

	case TRACK_VTTAG:
		// vertical tag was tracked
		{
			// convert pix into data value and back again
			const auto val = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
			m_VTtags.SetTagVal(m_HCtrapped, val);
			point.x = MulDiv(val - m_xWO, m_xVE, m_xWE) + m_xVO;
			XorVTtag(point.x);
			ChartWnd::OnLButtonUp(nFlags, point);
			postMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
		}
		break;

	case TRACK_RECT:
		{
			ChartWnd::OnLButtonUp(nFlags, point); // else release mouse

			// none: zoom data or offset display
			ChartWnd::OnLButtonUp(nFlags, point);
			CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
			const int jitter = 3;
			if ((abs(rect_out.Height()) < jitter) && (abs(rect_out.Width()) < jitter))
			{
				if (m_cursorType != CURSOR_ZOOM)
					postMyMessage(HINT_HITAREA, NULL);
				else
					zoomIn();
				break; // exit: mouse movement was too small
			}

			// perform action according to cursor type
			auto rect_in = m_displayRect;
			switch (m_cursorType)
			{
			case 0:
				rect_out = rect_in;
				rect_out.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
				ZoomData(&rect_in, &rect_out);
				break;
			case CURSOR_ZOOM: // zoom operation
				ZoomData(&rect_in, &rect_out);
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
	if (m_HZtags.GetNTags() > 0)
	{
		for (auto icur = m_HZtags.GetNTags() - 1; icur >= 0; icur--)
			m_HZtags.SetTagPix(icur, MulDiv(m_HZtags.GetValue(icur) - m_yWO, m_yVE, m_yWE) + m_yVO);
	}
	// compute pixel position of vertical tags
	if (m_VTtags.GetNTags() > 0)
	{
		for (auto icur = m_VTtags.GetNTags() - 1; icur >= 0; icur--) // loop through all tags
			m_VTtags.SetTagPix(icur, MulDiv(m_VTtags.GetValue(icur) - m_xWO, m_xVE, m_xWE) + m_xVO);
	}
	ChartWnd::OnLButtonDown(nFlags, point);
	if (m_currCursorMode != 0 || m_HCtrapped >= 0) // do nothing else if mode != 0
		return; // or any tag hit (VT, HZ) detected

	// test if mouse hit one histogram
	// if hit, then tell parent to select corresp histogram (spike)
	m_hitspk = hitCurve(point);
	if (m_hitspk >= 0)
	{
		// cancel track rect mode
		m_trackMode = TRACK_OFF; // flag trackrect
		releaseCursor(); // release cursor capture
		postMyMessage(HINT_HITSPIKE, m_hitspk);
	}
}

// ZoomData
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo= window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=yextent, ve=rect.height/2, vo = -rect.GetRectHeight()/2

void ChartSpikeHist::ZoomData(CRect* rFrom, CRect* rDest)
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
	postMyMessage(HINT_CHANGEZOOM, 0);
}

void ChartSpikeHist::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_hitspk < 0)
		ChartWnd::OnLButtonDblClk(nFlags, point);
	else
	{
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),
		                         reinterpret_cast<LPARAM>(m_hWnd));
	}
}

int ChartSpikeHist::hitCurve(CPoint point)
{
	auto hitspk = -1;
	// convert device coordinates into logical coordinates
	const auto deltax = MulDiv(3, m_xWE, m_xVE);
	const auto mouse_x = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
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

	const auto deltay = MulDiv(3, m_yWE, m_yVE);
	const auto mouse_y = static_cast<DWORD>(MulDiv(point.y - m_yVO, m_yWE, m_yVE)) + m_yWO + deltay;

	// test selected histogram first (foreground)
	auto ihist = 1;
	CDWordArray* pDW = nullptr;
	if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECLASSONLY)
	{
		// get array corresp to m_selclass as well as histogram index
		for (auto i = 1; i < histogram_ptr_array.GetSize(); i++)
		{
			if (static_cast<int>((histogram_ptr_array[i])->GetAt(0)) == m_selclass)
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
	if (m_plotmode != PLOT_ONECLASSONLY && hitspk < 0)
	{
		for (auto jhist = 1; jhist < histogram_ptr_array.GetSize() && hitspk < 0; jhist++)
		{
			pDW = histogram_ptr_array.GetAt(jhist);
			if (m_plotmode == PLOT_ONECLASS && static_cast<int>(pDW->GetAt(0)) == m_selclass)
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

void ChartSpikeHist::getExtents()
{
	if (m_yWE == 1) // && m_yWO == 0)
	{
		if (m_lmax == 0)
			getHistogLimits(0);
		m_yWE = static_cast<int>(m_lmax);
		m_yWO = 0;
	}

	if (m_xWE == 1) // && m_xWO == 0)
	{
		m_xWE = m_abcissamaxval - m_abcissaminval + 1;
		m_xWO = m_abcissaminval;
	}
}

void ChartSpikeHist::getHistogLimits(int ihist)
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

void ChartSpikeHist::reSize_And_Clear_Histograms(int n_bins, int max, int min)
{
	m_binsize = (max - min + 1) / n_bins + 1; // set bin size
	m_abcissaminval = min; // set min
	m_abcissamaxval = min + n_bins * m_binsize; // set max

	m_nbins = n_bins;
	for (auto j = histogram_ptr_array.GetUpperBound(); j >= 0; j--)
	{
		auto p_dw = histogram_ptr_array[j];
		p_dw->SetSize(n_bins + 1);
		// erase all data from histogram
		for (auto i = 1; i <= n_bins; i++)
			p_dw->SetAt(i, 0);
	}
}

void ChartSpikeHist::OnSize(UINT nType, int cx, int cy)
{
	ChartWnd::OnSize(nType, cx, cy);
	m_yVO = cy;
}

CDWordArray* ChartSpikeHist::initClassArray(int n_bins, int spike_class)
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

void ChartSpikeHist::buildHistFromSpikeList(SpikeList* p_spk_list, long l_first, long l_last, int max, int min,
                                             int n_bins, BOOL bNew)
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
	if (n_bins == 0)
	{
		return;
	}

	if (n_bins != m_nbins || p_dword_array->GetSize() != (n_bins + 1))
		reSize_And_Clear_Histograms(n_bins, max, min);

	CDWordArray* p_dw = nullptr;
	auto n_spikes = p_spk_list->GetTotalSpikes();
	for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		const auto spike_element = p_spk_list->GetSpikeElemt(i_spike);
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
		const auto spike_class = spike_element->get_class();
		getClassArray(spike_class, p_dw);
		if (p_dw == nullptr)
			p_dw = initClassArray(n_bins, spike_class);

		if (p_dw != nullptr)
		{
			dw_data = p_dw->GetAt(index) + 1;
			p_dw->SetAt(index, dw_data);
		}
	}
	getHistogLimits(0);
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

void ChartSpikeHist::BuildHistFromDocument(CdbWaveDoc* p_doc, const BOOL b_all_files, const long l_first, const long l_last, 
											int max, int min, int n_bins, BOOL bNew)
{
	// erase data and arrays if bnew:
	if (bNew)
	{
		RemoveHistData();
		bNew = false;
	}

	auto current_file = 0;
	auto file_first = current_file;
	auto file_last = current_file;
	if (b_all_files)
	{
		file_first = 0;
		file_last = p_doc->GetDB_NRecords() - 1;
		current_file = p_doc->GetDB_CurrentRecordPosition();
	}

	for (auto i_file = file_first; i_file <= file_last; i_file++)
	{
		if (b_all_files)
		{
			p_doc->SetDB_CurrentRecordPosition(i_file);
			p_doc->OpenCurrentSpikeFile();
		}
		SpikeList* p_spikelist = p_doc->m_pSpk->GetSpkList_Current();
		if (p_spikelist != nullptr && p_spikelist->GetTotalSpikes() > 0)
			buildHistFromSpikeList(p_spikelist, l_first, l_last, max, min, n_bins, bNew);
	}

	if (b_all_files)
	{
		p_doc->SetDB_CurrentRecordPosition(current_file);
		p_doc->OpenCurrentSpikeFile();
	}
}
