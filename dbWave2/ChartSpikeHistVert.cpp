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
	SetbUseDIB(FALSE);
	m_csEmpty = _T("no spikes (WndVerticalHistogram)");
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

void ChartSpikeHistVert::PlotDataToDC(CDC* p_dc)
{
	if (m_displayRect.right <= 0 && m_displayRect.bottom <= 0)
	{
		CRect r;
		GetWindowRect(&r);
		OnSize(SIZE_RESTORED, r.Width(), r.Height());
	}
	if (m_erasebkgnd) // erase background
		EraseBkgnd(p_dc);

	// load resources
	getExtents();
	if (m_lmax == 0)
	{
		p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
		auto rect = m_displayRect;
		rect.DeflateRect(1, 1);
		const auto textlen = m_csEmpty.GetLength();
		p_dc->DrawText(m_csEmpty, textlen, rect, DT_LEFT); //|DT_WORDBREAK);
		return;
	}
	const int n_saved_dc = p_dc->SaveDC();
	prepareDC(p_dc);
	int color;
	// save background color which is changed by later calls to FillSolidRect
	// when doing so, pens created with PS_DOT pattern and with XOR_PEN do
	// not work properly. Restoring the background color solves the pb.
	const auto bkcolor = p_dc->GetBkColor();
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
			if (PLOT_ONECLASSONLY == m_plotmode && spkcla != m_selclass)
				continue;
			if (PLOT_CLASSCOLORS == m_plotmode)
				color = spkcla % NB_COLORS;
			else if (m_plotmode == PLOT_ONECLASS && spkcla == m_selclass)
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
	p_dc->SetBkColor(bkcolor); // restore background color
	if (m_HZtags.GetNTags() > 0) // display horizontal tags
		DisplayHZtags(p_dc);
	if (m_VTtags.GetNTags() > 0) // display vertical tags
		DisplayVTtags_Value(p_dc);
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
			p_dc->FillSolidRect(rect_histog, m_colorTable[color]);
		}
	}
}

void ChartSpikeHistVert::MoveHZtagtoVal(int i, int val)
{
	m_ptLast.y = MulDiv(m_HZtags.GetValue(i) - m_yWO, m_yVE, m_yWE) + m_yVO;
	const auto j = MulDiv(val - m_yWO, m_yVE, m_yWE) + m_yVO;
	XorHZtag(j);
	m_HZtags.SetTagVal(i, val);
}

void ChartSpikeHistVert::MoveVTtagtoVal(int itag, int ival)
{
	m_ptLast.x = MulDiv(m_VTtags.GetValue(itag) - m_xWO, m_xVE, m_xWE) + m_xVO;
	const auto j = MulDiv(ival - m_xWO, m_xVE, m_xWE) + m_xVO;
	XorVTtag(j);
	m_VTtags.SetTagVal(itag, ival);
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
			ChartSpike::OnLButtonUp(nFlags, point);
			postMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
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

void ChartSpikeHistVert::OnLButtonDown(UINT nFlags, CPoint point)
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
	ChartSpike::OnLButtonDown(nFlags, point);
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

void ChartSpikeHistVert::ZoomData(CRect* rFrom, CRect* rDest)
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

void ChartSpikeHistVert::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_hitspk < 0)
		ChartSpike::OnLButtonDblClk(nFlags, point);
	else
	{
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),
		                         reinterpret_cast<LPARAM>(m_hWnd));
	}
}

int ChartSpikeHistVert::hitCurve(CPoint point)
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

void ChartSpikeHistVert::getExtents()
{
	if (m_xWE == 1) // && m_xWO == 0)
	{
		if (m_lmax == 0)
			getHistogLimits(0);
		m_xWE = static_cast<int>(m_lmax);
		m_xWO = 0;
	}

	if (m_yWE == 1) // && m_yWO == 0)
	{
		m_yWE = m_abcissamaxval - m_abcissaminval + 1;
		m_yWO = m_abcissaminval;
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
	// � z�ro. Au dela de max toute les cases du tableau sont � z�ro.

	m_ifirst = 1; // search first interval with data
	while (m_ifirst <= m_nbins && p_dw->GetAt(m_ifirst) == 0)
		m_ifirst++;

	m_ilast = m_nbins; // search last interval with data
	while (p_dw->GetAt(m_ilast) == 0 && m_ilast > m_ifirst)
		m_ilast--;

	// R�cuperation de l'indice du maximum
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
	m_yVO = cy;
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
	auto nspikes = p_spk_list->GetTotalSpikes();
	for (auto ispk = 0; ispk < nspikes; ispk++)
	{
		const auto spike_element = p_spk_list->GetSpike(ispk);

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
		const auto spike_class = spike_element->get_class();
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
		file_last = p_doc->GetDB_NRecords() - 1;
		currentfile = p_doc->GetDB_CurrentRecordPosition();
	}

	for (auto ifile = file_first; ifile <= file_last; ifile++)
	{
		if (ballFiles)
		{
			p_doc->SetDB_CurrentRecordPosition(ifile);
			p_doc->OpenCurrentSpikeFile();
		}
		SpikeList* p_spikelist = p_doc->m_pSpk->GetSpkList_Current();
		if (p_spikelist != nullptr && p_spikelist->GetTotalSpikes() > 0)
			buildHistFromSpikeList(p_spikelist, l_first, l_last, max, min, nbins, bNew);
	}

	if (ballFiles)
	{
		p_doc->SetDB_CurrentRecordPosition(currentfile);
		p_doc->OpenCurrentSpikeFile();
	}
}
