// spikebar.cpp Implementation File

#include "StdAfx.h"
#include "scopescr.h"
#include "Spikedoc.h"
#include "dbWaveDoc.h"
#include "Spikebar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL (CSpikeBarWnd, CScopeScreen, 1)

BEGIN_MESSAGE_MAP(CSpikeBarWnd, CScopeScreen)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

CSpikeBarWnd::CSpikeBarWnd()
{
	p_spike_list_= nullptr;
	p_envelope_ = nullptr;
	m_lFirst = 0;
	m_lLast = 0;
	m_currentclass=-999;
	m_btrackCurve = FALSE;
	m_hitspk=-1;	
	m_selectedspike=-1;
	m_rangemode = RANGE_TIMEINTERVALS;
	m_barheight = 10;
	SetbUseDIB(FALSE);
	m_csEmpty = "no \nspikes";
	m_ballFiles = FALSE;
	p_dbwave_doc_ = nullptr;
	m_spklast = 0;
}  

CSpikeBarWnd::~CSpikeBarWnd()
{
	if (p_envelope_ != nullptr)
	{
		p_envelope_->RemoveAll();
		delete p_envelope_;
	}
}


void CSpikeBarWnd::PlotDatatoDC(CDC* pDC)
{
	// prepare display
	if (m_erasebkgnd)			// erase background
		EraseBkgnd(pDC);

	// print text
	pDC->SelectObject (GetStockObject (DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1,1);

	long nfiles = 1;
	long ncurrentfile = 0;
	if (m_ballFiles)
	{
		nfiles = p_dbwave_doc_->DBGetNRecords();
		ncurrentfile = p_dbwave_doc_->DBGetCurrentRecordPosition();
	}

	for (long ifile=0; ifile < nfiles; ifile++)
	{
		if (m_ballFiles)
		{
			p_dbwave_doc_->DBSetCurrentRecordPosition(ifile);
			p_dbwave_doc_->OpenCurrentSpikeFile();
			p_spike_list_ = p_dbwave_doc_->m_pSpk->GetSpkListCurrent();
		}

		// test presence of data	
		if (p_spike_list_ == nullptr || p_spike_list_->GetTotalSpikes() == 0)
		{
			if (!m_ballFiles)
			{
				pDC->DrawText(m_csEmpty, m_csEmpty.GetLength(), rect, DT_LEFT); //|DT_WORDBREAK);
				return;
			}
			continue;
		}

		// plot comment at the bottom
		if (m_bBottomComment)
		{
			pDC->DrawText(m_csBottomComment, m_csBottomComment.GetLength(), rect, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
		}

		// display data: trap error conditions
		if (m_yWE == 1)
		{
			int maxval, minval;
			p_spike_list_->GetTotalMaxMin(TRUE, &maxval, &minval);
			m_yWE = maxval - minval;
			m_yWO = (maxval + minval) / 2;
		}
		if (m_xWE == 1) // this is generally the case: && m_xWO == 0)
		{
			m_xWE = m_displayRect.right;
			m_xWO = m_displayRect.left;
		}

		// display data
		const auto n_saved_dc = pDC->SaveDC();
		pDC->IntersectClipRect(&m_clientRect);
		DisplayBars(pDC, &m_displayRect);
		pDC->RestoreDC(n_saved_dc);

		// display stimulus
		if (p_spike_doc_->m_stimIntervals.nitems > 0)
			DisplayStim(pDC, &m_displayRect);

		// display vertical cursors
		if (GetNVTtags() > 0)
		{
			// select pen and display mode 
			const auto oldp = pDC->SelectObject(&m_blackDottedPen);
			const auto nold_rop = pDC->SetROP2(R2_NOTXORPEN);

			// iterate through VT cursor list	
			const auto y0 = 0;
			const int y1 = m_displayRect.bottom;
			for (auto j = GetNVTtags() - 1; j >= 0; j--)
			{
				const auto lk = GetVTtagLval(j);	// get val
				if (lk <m_lFirst || lk > m_lLast)
					continue;
				const auto k = static_cast<int>((lk - m_lFirst)*static_cast<float>(m_displayRect.Width()) / (m_lLast - m_lFirst + 1));
				pDC->MoveTo(k, y0);			// set initial pt
				pDC->LineTo(k, y1);			// VT line		
			}
			pDC->SetROP2(nold_rop);			// restore old display mode		
			pDC->SelectObject(oldp);
		}

		// temp tag
		if (m_hwndReflect != nullptr && m_tempVTtag != nullptr)
		{
			const auto oldp = pDC->SelectObject(&m_blackDottedPen);
			const auto nold_rop = pDC->SetROP2(R2_NOTXORPEN);
			pDC->MoveTo(m_tempVTtag->m_pixel, m_displayRect.top + 2);
			pDC->LineTo(m_tempVTtag->m_pixel, m_displayRect.bottom - 2);
			pDC->SetROP2(nold_rop);
			pDC->SelectObject(oldp);
		}
	} 

	if (m_ballFiles)
	{
		p_dbwave_doc_->DBSetCurrentRecordPosition(ncurrentfile);
		p_dbwave_doc_->OpenCurrentSpikeFile();
		p_spike_list_ = p_dbwave_doc_->m_pSpk->GetSpkListCurrent();
	}
}

void CSpikeBarWnd::DisplayStim(CDC* p_dc, CRect* rect) const
{
	CPen bluepen;
	bluepen.CreatePen(PS_SOLID, 0, RGB(0,0,255));
	const auto pold_p = (CPen*) p_dc->SelectObject(&bluepen);
	const int top = rect->bottom - m_barheight +2;; // m_clientRect.bottom - m_barheight +2;
	const int bottom = rect->bottom-3; //m_clientRect.bottom - 3;
	const auto displen = rect->Width(); //m_clientRect.Width();
		
	// search first stimulus transition within interval
	const auto iistart = m_lFirst;
	const auto iiend = m_lLast;
	const auto iilen = iiend - iistart;
	auto i0 = 0;
	while (i0 < p_spike_doc_->m_stimIntervals.intervalsArray.GetSize() 
			&& p_spike_doc_->m_stimIntervals.intervalsArray.GetAt(i0) < iistart)
		i0++;							// loop until found

	auto istate =bottom;				// use this variable to keep track of pulse broken by display limits
	const auto jj = (i0/2)*2;					// keep index of the ON transition
	if (jj != i0)
		istate = top;
	p_dc->MoveTo(rect->left, istate); 

	const auto nsti = ((p_spike_doc_->m_stimIntervals.intervalsArray.GetSize())/2)*2;
	for (auto ii=jj; ii< nsti; ii+=2)
	{
		// stim starts here
		int iix0 = p_spike_doc_->m_stimIntervals.intervalsArray.GetAt(ii) - iistart;
		if (iix0 >= iilen)				// first transition ON after last graph pt?
			break;						// yes = exit loop
		if (iix0 < 0)					// first transition off graph?
			iix0 = 0;					// yes = clip

		iix0 = MulDiv(displen, iix0, iilen) + rect->left;
		p_dc->LineTo(iix0, istate);		// draw line up to the first point of the pulse
		p_dc->LineTo(iix0, top);			// draw vertical line to top of pulse

		// stim ends here
		istate = bottom;				// after pulse, descend to bottom level
		int iix1 = p_spike_doc_->m_stimIntervals.intervalsArray.GetAt(ii+1) - iistart;
		if (iix1 > iilen)				// last transition off graph?
		{
			iix1 = iilen;				// yes = clip
			istate = top;				// do not descend..
		}
		iix1 = MulDiv(displen, iix1, iilen) + rect->left+1;
		p_dc->LineTo(iix1, top);			// draw top of pulse
		p_dc->LineTo(iix1, istate);		// draw descent to bottom line
	}
	p_dc->LineTo(rect->left + displen, istate);		// end of loop - draw the rest
	p_dc->SelectObject(pold_p);
}


void CSpikeBarWnd::DisplayBars(CDC* p_dc, CRect* rect)
{
	// prepare loop to display spikes
	auto* pold_pen = (CPen*) p_dc->SelectStockObject(BLACK_PEN);
	const long xextent = rect->Width();
	if (m_yWE == 1)
	{		
		int maxval, minval;
		p_spike_list_->GetTotalMaxMin(TRUE, &maxval, &minval);
		m_yWE = maxval - minval;  
		m_yWO = (maxval + minval)/2;
	}
	const auto yextent = m_yWE;
	const auto yzero = m_yWO;
	const auto y_vo = static_cast<int>(rect->Height() / 2) + rect->top;
	const auto y_ve = -rect->Height(); 

	// draw horizontal line
	const int baseline = MulDiv(p_spike_list_->GetAcqBinzero()-yzero, y_ve, yextent) + y_vo;
	p_dc->MoveTo(rect->left, baseline);
	p_dc->LineTo(rect->right, baseline);

	// loop through all spikes of the list
	auto ifirst = 0;
	auto ilast = p_spike_list_->GetTotalSpikes() -1;
	if (m_rangemode == RANGE_INDEX)
	{
		if (m_spklast > ilast) 
			m_spklast = ilast;
		if (m_spkfirst < 0) 
			m_spkfirst = 0;
		ilast = m_spklast;		// reduces the nb of spikes examined
		ifirst = m_spkfirst;	// assuming an ordered list...
	}
	const auto len =  (m_lLast - m_lFirst + 1);
	int max, min;

	auto selpen = BLACK_COLOR;
	for (auto ispk=ilast; ispk >= ifirst; ispk--)
	{
		// skip spike if outside time range && option
		const auto l_spike_time = p_spike_list_->GetSpikeTime(ispk);
		if (m_rangemode == RANGE_TIMEINTERVALS
			&& (l_spike_time < m_lFirst || l_spike_time > m_lLast))
			continue;

		// select correct pen
		const auto wspkcla = p_spike_list_->GetSpikeClass(ispk);
		switch (m_plotmode)
		{
		case PLOT_ONECLASSONLY:
			if (wspkcla != m_selclass)
				continue;
			break;
		case PLOT_CLASSCOLORS:
			selpen = wspkcla % NB_COLORS;
			break;
		case PLOT_ONECLASS:
			if (wspkcla != m_selclass)
				selpen= SILVER_COLOR;
			else
				selpen = m_colorselected;
		default:
			break;
		}
		p_dc->SelectObject(&m_penTable[selpen]); 
		
		// and draw spike: compute abcissa & draw from max to min     
		const auto llk = (l_spike_time - m_lFirst)*static_cast<float>(xextent) / len; //muldiv
		const int abcissa = static_cast<int>(llk) + rect->left;
		p_spike_list_->GetSpikeExtrema(ispk, &max, &min);
		max = MulDiv(max-yzero, y_ve, yextent) + y_vo;
		min = MulDiv(min-yzero, y_ve, yextent) + y_vo;
		p_dc->MoveTo(abcissa, max);
		p_dc->LineTo(abcissa, min);

		if (wspkcla >= 1)
		{
			TEXTMETRIC txtmetric;
			if (p_dc->GetTextMetrics(&txtmetric)) {
				auto cletter = _T('0');
				cletter += wspkcla;
				p_dc->TextOut(abcissa, min + (txtmetric.tmHeight / 2), &cletter, 1);
			}
		}
	}

	// display selected spike
	if (m_selectedspike >= 0)
		DisplaySpike(m_selectedspike, TRUE);
	if (p_spike_list_->GetSpikeFlagArrayCount() > 0)
		DisplayFlaggedSpikes(TRUE);

	p_dc->SelectObject(pold_pen);
}


void CSpikeBarWnd::DisplayFlaggedSpikes(const BOOL b_high_light)
{
	CClientDC dc(this);
	if (m_xWE == 1 || m_yWE == 1)
		return;
	m_xWE = m_displayRect.Width();
	m_xWO = m_displayRect.left;

	dc.IntersectClipRect(&m_clientRect);
	PrepareDC(&dc);	
	const auto pensize = 0;

	// loop over the array of flagged spikes
	for (auto i= p_spike_list_->GetSpikeFlagArrayCount()-1; i>=0; i--)
	{
		const auto nospike = p_spike_list_->GetSpikeFlagArrayAt(i);
		const auto nospikeclass = p_spike_list_->GetSpikeClass(nospike);
		if (PLOT_ONECLASSONLY == m_plotmode && nospikeclass != m_selclass)
			continue;

		auto color = RED_COLOR;
		if (!b_high_light)
		{		
			switch (m_plotmode)
			{
			case PLOT_ONECLASSONLY:
			case PLOT_ONECLASS:
				color = BLACK_COLOR;
				if (nospikeclass != m_selclass)
					color = SILVER_COLOR;
				break;
			case PLOT_CLASSCOLORS:
				if (nospike == m_selectedspike)
					HighlightOneBar(nospike, &dc);
				color = nospikeclass % NB_COLORS;
				break;
			case PLOT_BLACK:
			default:
				color = BLACK_COLOR;
				break;
			}
		}
	
		CPen new_pen;
		new_pen.CreatePen(PS_SOLID, pensize, m_colorTable[color]);
		const auto oldpen = (CPen*) dc.SelectObject(&new_pen);

		// display data
		const auto l_spike_time = p_spike_list_->GetSpikeTime(nospike);
		const auto len =  (m_lLast - m_lFirst + 1);
		const auto llk = (l_spike_time - m_lFirst)*static_cast<float>(m_xWE) / len; 		
		const auto abcissa = static_cast<int>(llk)  + m_xWO;
		int max, min;
		p_spike_list_->GetSpikeExtrema(nospike, &max, &min);

		dc.MoveTo(abcissa, max);
		dc.LineTo(abcissa, min);
		dc.SelectObject(oldpen);
	}
}

void CSpikeBarWnd::DisplaySpike(const int nospike, const BOOL bselect)
{
	if (!IsSpikeWithinRange(nospike))
		return;

	CClientDC dc(this);	
	if (m_xWE == 1 || m_yWE == 1)
		return;
	m_xWE = m_displayRect.Width();
	m_xWO = m_displayRect.left;
	dc.IntersectClipRect(&m_clientRect);
	PrepareDC(&dc);

	int  color;
	// spike is not selected
	if (!bselect)
	{		
		switch (m_plotmode)
		{
		case PLOT_ONECLASSONLY:
		case PLOT_ONECLASS:
			color = BLACK_COLOR;
			if (p_spike_list_->GetSpikeClass(nospike) != m_selclass)
				color = SILVER_COLOR;	
			break;
		case PLOT_CLASSCOLORS:
			if (nospike == m_selectedspike)
				HighlightOneBar(nospike, &dc);
			color = p_spike_list_->GetSpikeClass(nospike) % NB_COLORS;
			break;
		case PLOT_BLACK:
		default:
			color = BLACK_COLOR;	
			break;
		}
	}
	// spike is selected
	else
	{
		switch (m_plotmode)
		{
		case PLOT_CLASSCOLORS:
			HighlightOneBar(nospike, &dc);
			color = p_spike_list_->GetSpikeClass(nospike) % NB_COLORS;
			break;
		case PLOT_BLACK:
		case PLOT_ONECLASSONLY:
		case PLOT_ONECLASS:
		default:
			color = RED_COLOR;	
			break;
		}
	}
	
	CPen new_pen;
	const auto pensize = 0;
	new_pen.CreatePen(PS_SOLID, pensize, m_colorTable[color]);
	const auto oldpen = (CPen*)dc.SelectObject(&new_pen);

	// display data
	const auto l_spike_time = p_spike_list_->GetSpikeTime(nospike);
	const auto len =  (m_lLast - m_lFirst + 1);
	const auto llk = (l_spike_time - m_lFirst)*static_cast<float>(m_xWE) / len;
	const auto abcissa = static_cast<int>(llk) + m_xWO;
	int max, min;
	p_spike_list_->GetSpikeExtrema(nospike, &max, &min);


	dc.MoveTo(abcissa, max);
	dc.LineTo(abcissa, min);
	dc.SelectObject(oldpen);
}

BOOL CSpikeBarWnd::IsSpikeWithinRange(const int spikeno)
{
	if (m_spklast > p_spike_list_->GetTotalSpikes()-1) 
		m_spklast = p_spike_list_->GetTotalSpikes()-1;
	if (m_spkfirst < 0) m_spkfirst = 0;
	if (spikeno < 0 || spikeno > p_spike_list_->GetTotalSpikes()-1)
		return FALSE;
	
	if (m_rangemode == RANGE_TIMEINTERVALS
		&& (p_spike_list_->GetSpikeTime(spikeno) < m_lFirst || p_spike_list_->GetSpikeTime(spikeno) > m_lLast))
		return FALSE;
	else if (m_rangemode == RANGE_INDEX
		&& (spikeno>m_spklast || spikeno < m_spkfirst))
		return FALSE;
	if (m_plotmode == PLOT_ONECLASSONLY && p_spike_list_->GetSpikeClass(spikeno) != m_selclass)
		return FALSE;
	return TRUE;
}


void CSpikeBarWnd::HighlightOneBar(const int nospike, CDC* p_dc) const
{
	const auto old_rop = p_dc->GetROP2();
	p_dc->SetROP2(R2_NOTXORPEN);

	const auto l_spike_time = p_spike_list_->GetSpikeTime(nospike);
	const auto len =  (m_lLast - m_lFirst + 1);
	const auto  llk = (l_spike_time - m_lFirst)*static_cast<float>(m_xWE) / len; 
	const auto abcissa = static_cast<int>(llk) + m_xWO;

	const auto max = MulDiv(1-m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto min = MulDiv(m_displayRect.Height()-2-m_yVO, m_yWE, m_yVE) + m_yWO;

	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, 1, RGB(196,   2,  51));
	const auto oldpen = (CPen*) p_dc->SelectObject(&new_pen);

	p_dc->MoveTo(abcissa-1, max);
	p_dc->LineTo(abcissa+1, max);
	p_dc->LineTo(abcissa+1, min);
	p_dc->LineTo(abcissa-1, min);
	p_dc->LineTo(abcissa-1, max);

	// restore resources	
	p_dc->SelectObject(oldpen);
	p_dc->SetROP2(old_rop);
}

//---------------------------------------------------------------------------

int	CSpikeBarWnd::SelectSpike(const int spikeno)
{
	// erase old selected spike
	const auto oldselected = m_selectedspike; 
	if (m_selectedspike >= 0) 
		DisplaySpike(m_selectedspike, FALSE);

	DisplaySpike(spikeno, TRUE);
	m_selectedspike = spikeno;

	return oldselected;
}

//-----------------------------------------------------------------------------
// flag all spikes within a rectangle in screen coordinates

void CSpikeBarWnd::SelectSpikesWithinRect(CRect* p_rect, const UINT n_flags) const
{
	// make sure that the rectangle is ok
	int i;
	if (p_rect->top > p_rect->bottom)
	{
		i = p_rect->top;
		p_rect->top = p_rect->bottom;
		p_rect->bottom= i;
	}
	if (p_rect->left > p_rect->right)
	{
		i = p_rect->left;
		p_rect->left = p_rect->right;
		p_rect->right = i;
	}
	
	const auto len =  (m_lLast - m_lFirst + 1);
	const auto l_first= MulDiv(p_rect->left, len, m_displayRect.Width()) + m_lFirst;
	const auto l_last= MulDiv(p_rect->right, len, m_displayRect.Width()) + m_lFirst;
	const auto vmin = MulDiv(p_rect->bottom -m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto vmax = MulDiv(p_rect->top -m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto b_flag = (n_flags & MK_SHIFT) || (n_flags & MK_CONTROL);
	p_spike_list_->SelectSpikeswithinRect(vmin, vmax, l_first, l_last, b_flag);
}


void CSpikeBarWnd::OnLButtonUp(const UINT n_flags, const CPoint point) 
{		
	if (!m_bLmouseDown)
	{
		PostMyMessage(HINT_DROPPED, NULL);
		return;
	}
	CScopeScreen::OnLButtonUp(n_flags, point);

	CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
	const auto jitter = 5;
	if ((abs(rect_out.Height())< jitter) && (abs(rect_out.Width())< jitter))
	{
		if (m_cursorType != CURSOR_ZOOM)
			PostMyMessage(HINT_HITAREA, NULL);
		else
			ZoomIn();
		return;	 // exit: mouse movement was too small
	}

	// perform action according to cursor type
	auto rect_in = m_displayRect;
	switch (m_cursorType)
	{
		case 0:
			if (m_hitspk < 0)
			{
				auto rect = GetDefinedRect();
				SelectSpikesWithinRect(&rect, n_flags);
				PostMyMessage(HINT_SELECTSPIKES, NULL); 
			}
			break;

		case CURSOR_ZOOM: 	// zoom operation
			ZoomData(&rect_in, &rect_out);
			m_ZoomFrom = rect_in;
			m_ZoomTo   = rect_out;					
			m_iUndoZoom = 1;
			PostMyMessage(HINT_SETMOUSECURSOR, m_oldcursorType);
			break;

		default:
			break;
	}	
}

void CSpikeBarWnd::OnLButtonDown(const UINT nFlags, CPoint point)
{
	m_bLmouseDown = TRUE;

	// detect bar hit: test if curve hit -- specific to SpikeBarButton
	if (m_currCursorMode==0)
	{
		m_btrackCurve = FALSE;
		m_hitspk = DoesCursorHitCurve(point);
		// tell parent spike selected
		if (m_hitspk >= 0)
		{
			if (nFlags & MK_SHIFT)
				PostMyMessage(HINT_HITSPIKE_SHIFT, m_hitspk);	
			else if (nFlags & MK_CONTROL)
				PostMyMessage(HINT_HITSPIKE_CTRL, m_hitspk);
			else
				PostMyMessage(HINT_HITSPIKE, m_hitspk);	
			return;
		}
	}
	CScopeScreen::OnLButtonDown(nFlags, point);	
}


//---------------------------------------------------------------------------
// ZoomData()
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo= window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=yextent, ve=rect.height/2, vo = -rect.Height()/2
//---------------------------------------------------------------------------

void CSpikeBarWnd::ZoomData(CRect* rFrom, CRect* rDest)
{
	rFrom->NormalizeRect();	
	rDest->NormalizeRect();
	
	// change y gain & y offset		
	const auto y_we = m_yWE;
	m_yWE = MulDiv (m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
			-MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
			+MulDiv(rDest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed	
	auto l_size = m_lLast-m_lFirst+1;
	m_lFirst = m_lFirst + l_size*(rDest->left-rFrom->left)/rFrom->Width();
	if (m_lFirst <0) 
		m_lFirst = 0;
	l_size = l_size *rDest->Width() / rFrom->Width();
	m_lLast = m_lFirst +l_size -1;
		
	PostMyMessage(HINT_CHANGEHZLIMITS, NULL);

}

void CSpikeBarWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_selectedspike < 0 && p_spike_list_->GetSpikeFlagArrayCount () < 1)|| m_hitspk < 0)
		CScopeScreen::OnLButtonDblClk(nFlags, point);
	else
	{
		if (m_selectedspike >= 0)
		{
			PostMyMessage(HINT_DBLCLKSEL, m_selectedspike);
		}
		else
		{
			const auto iselectedspike = DoesCursorHitCurve(point);
			if (iselectedspike > 0)
				PostMyMessage(HINT_DBLCLKSEL, iselectedspike);
		}
	}
}

//---------------------------------------------------------------------------

int CSpikeBarWnd::DoesCursorHitCurve(const CPoint point)
{
	auto hitspk = -1;
	// for y coordinates, conversion is straightforward:
	const auto mouse_y = MulDiv(point.y-m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto deltay = MulDiv(3, m_yWE, m_yVE);

	// for x coordinates, the formula is in 2 steps:
	// 1) time -> relative time: iitime = (spiketime-m_lFirst) [-offset]
	// 2) rel time -> logical coordinate(LC): LC = iitime* m_xWE / len + m_xWO
	// formula used to display: time (long) --> abcissa (int)
	// 		long len =  (m_lLast - m_lFirst + 1);
	// 		int abcissa = (int) (((lSpikeTime - m_lFirst) * (long) m_xWE) / len) + m_xWO;	
	// reverse formula: abcissa -> time
	//		long lSpikeTime  = (abcissa - m_xWO)*len/m_xWE + m_lFirst;
	// convert device coordinates into logical coordinates
	const auto mouse_x = MulDiv(point.x-m_xVO, m_xWE, m_xVE) + m_xWO;
	const auto deltax = MulDiv(3, m_xWE, m_xVE);
	const auto len_data_displayed =  (m_lLast - m_lFirst + 1);	

	// find a spike which time of occurence fits between lXmax and lXmin
	const auto l_xmax = m_lFirst + len_data_displayed*(mouse_x + deltax)/static_cast<long>(m_xWE);
	const auto l_xmin = m_lFirst + len_data_displayed*(mouse_x - deltax)/static_cast<long>(m_xWE);
	
	// loop through all spikes
	auto ifirst = 0;
	auto ilast = p_spike_list_->GetTotalSpikes()-1;
	if (m_rangemode == RANGE_INDEX)	
	{
		if (m_spklast > ilast) m_spklast = ilast;
		if (m_spkfirst < 0) m_spkfirst = 0;
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}

	for (auto ispk=ilast; ispk>=ifirst; ispk--)
	{
		const auto l_spike_time = p_spike_list_->GetSpikeTime(ispk);
		if (l_spike_time < l_xmin || l_spike_time > l_xmax)
			continue;
		if (m_plotmode == PLOT_ONECLASSONLY
			&& p_spike_list_->GetSpikeClass(ispk) != m_selclass)
			continue;

		int max;
		int min;
		p_spike_list_->GetSpikeExtrema(ispk, &max, &min);
		if (mouse_y+deltay < max && mouse_y-deltay > min)
		{			
			hitspk = ispk;
			break;
		}
	}

	return hitspk;
}


void CSpikeBarWnd::CenterCurve()
{
	if (p_spike_list_ == nullptr || p_spike_list_->GetTotalSpikes() <= 0)
		return;
	int max, min;		
	p_spike_list_->GetTotalMaxMin(TRUE, &max, &min);
	m_yWO = max/2 + min/2;
}


void CSpikeBarWnd::MaxGain()
{
	if (p_spike_list_ == nullptr || p_spike_list_->GetTotalSpikes() <= 0)
		return;
	int max, min;		
	p_spike_list_->GetTotalMaxMin(TRUE, &max, &min);
	m_yWE = MulDiv(max-min+1, 10, 8);
}

void CSpikeBarWnd::MaxCenter()
{
	if (p_spike_list_ == nullptr || p_spike_list_->GetTotalSpikes() <= 0)
		return;
	int max, min;		
	p_spike_list_->GetTotalMaxMin(TRUE, &max, &min);
	m_yWE = MulDiv(max-min+1, 10, 8);
	m_yWO = max/2 + min/2;
}


void CSpikeBarWnd::Print(CDC* pDC, CRect* rect)
{
	// check if there are valid data to display
	if (p_spike_list_ == nullptr || p_spike_list_->GetTotalSpikes()== 0)
		return;		

	// set mapping mode and viewport
	const auto n_saved_dc = pDC->SaveDC();				// save display context	
	DisplayBars(pDC, rect);
	if (p_spike_doc_->m_stimIntervals.nitems > 0)	
		DisplayStim(pDC, rect);

	pDC->RestoreDC(n_saved_dc);	
}


void CSpikeBarWnd::Serialize( CArchive& ar )
{
	CScopeScreen::Serialize(ar);

	auto bdummy=TRUE;
	if (ar.IsStoring())
	{
		ar << m_rangemode;		// display range (time OR storage index)
		ar << m_lFirst;			// time index of first pt displayed
		ar << m_lLast;			// time index of last pt displayed
		ar << m_spkfirst;		// index first spike
		ar << m_spklast;		// index last spike
		ar << m_currentclass;	// current class in case of displaying classes
		ar << m_selectedspike;	// selected spike (disply differently)
		ar << m_hitspk;			// no of spike selected	
		ar << m_selclass;		// index class selected	
		ar << m_btrackCurve;	// track curve ?
		ar << bdummy;
		ar << m_selpen;	
	}
	else
	{		
		ar >> m_rangemode;		// display range (time OR storage index)
		ar >> m_lFirst;			// time index of first pt displayed
		ar >> m_lLast;			// time index of last pt displayed
		ar >> m_spkfirst;		// index first spike
		ar >> m_spklast;		// index last spike
		ar >> m_currentclass;	// current class in case of displaying classes
		ar >> m_selectedspike;	// selected spike (disply differently)
		ar >> m_hitspk;			// no of spike selected	
		ar >> m_selclass;		// index class selected	
		ar >> m_btrackCurve;	// track curve ?
		ar >> bdummy;
		ar >> m_selpen;	
	}
}