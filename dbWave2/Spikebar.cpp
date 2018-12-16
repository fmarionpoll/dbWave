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

//////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSpikeBarWnd, CScopeScreen)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////////////////
// DISPLAY BARS
//
////////////////////////////////////////////////////////////////////////////////////


// ------------------------------------------------------
// PlotDatatoDC(CDC* pDC)
// call either display bars or displayshape according
// to flag
// ------------------------------------------------------
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
				int textlen = m_csEmpty.GetLength();
				pDC->DrawText(m_csEmpty, textlen, rect, DT_LEFT); //|DT_WORDBREAK);
				return;
			}
			else
				continue;
		}

		// plot comment at the bottom
		if (m_bBottomComment)
		{
			int textlen = m_csBottomComment.GetLength();
			pDC->DrawText(m_csBottomComment, textlen, rect, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
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
		int nSavedDC = pDC->SaveDC();
		pDC->IntersectClipRect(&m_clientRect);
		DisplayBars(pDC, &m_displayRect);
		pDC->RestoreDC(nSavedDC);

		// display stimulus
		if (p_spike_doc_->m_stimIntervals.nitems > 0)
			DisplayStim(pDC, &m_displayRect);

		// display vertical cursors
		if (GetNVTtags() > 0)
		{
			// select pen and display mode 
			CPen* oldp = pDC->SelectObject(&m_blackDottedPen);
			int noldROP = pDC->SetROP2(R2_NOTXORPEN);

			// iterate through VT cursor list	
			int y0 = 0;
			int y1 = m_displayRect.bottom;
			for (int j = GetNVTtags() - 1; j >= 0; j--)
			{
				long lk = GetVTtagLval(j);	// get val
				if (lk <m_lFirst || lk > m_lLast)
					continue;
				float llk = (lk - m_lFirst)*(float)(m_displayRect.Width()) / (m_lLast - m_lFirst + 1); //muldiv
				int k = (int)llk;
				pDC->MoveTo(k, y0);			// set initial pt
				pDC->LineTo(k, y1);			// VT line		
			}
			pDC->SetROP2(noldROP);			// restore old display mode		
			pDC->SelectObject(oldp);
		}

		// temp tag
		if (m_hwndReflect != nullptr && m_tempVTtag != nullptr)
		{
			CPen* oldp = pDC->SelectObject(&m_blackDottedPen);
			int noldROP = pDC->SetROP2(R2_NOTXORPEN);
			pDC->MoveTo(m_tempVTtag->m_pixel, m_displayRect.top + 2);
			pDC->LineTo(m_tempVTtag->m_pixel, m_displayRect.bottom - 2);
			pDC->SetROP2(noldROP);
			pDC->SelectObject(oldp);
		}
	} 

	if (m_ballFiles)
	{
		p_dbwave_doc_->DBSetCurrentRecordPosition(ncurrentfile);
		p_dbwave_doc_->OpenCurrentSpikeFile();
		p_spike_list_ = (CSpikeList*)p_dbwave_doc_->m_pSpk->GetSpkListCurrent();
	}
}

void CSpikeBarWnd::DisplayStim(CDC* pDC, CRect* rect)
{
	CPen bluepen;
	bluepen.CreatePen(PS_SOLID, 0, RGB(0,0,255));
	CPen* poldP = (CPen*) pDC->SelectObject(&bluepen);

	int top = rect->bottom - m_barheight +2;; // m_clientRect.bottom - m_barheight +2;
	int bottom = rect->bottom-3; //m_clientRect.bottom - 3;
	int displen = rect->Width(); //m_clientRect.Width();
		
	// search first stimulus transition within interval
	long iistart = m_lFirst;
	long iiend = m_lLast; 
	long iilen = iiend - iistart;
	int i0 = 0;
	while (i0 < p_spike_doc_->m_stimIntervals.intervalsArray.GetSize() 
			&& p_spike_doc_->m_stimIntervals.intervalsArray.GetAt(i0) < iistart)
		i0++;				// loop until found
	int iix00 = 0;			// start looping from the first interval that meet the criteria
	int istate =bottom;		// use this variable to keep track of pulse broken by display limits
	int ii = (i0/2)*2;		// keep index of the ON transition
	if (ii != i0)
		istate = top;
	pDC->MoveTo(rect->left, istate); // iix00, istate);

	int nsti = ((p_spike_doc_->m_stimIntervals.intervalsArray.GetSize())/2)*2;
	for (ii; ii< nsti; ii++, ii++)
	{
		// stim starts here
		int iix0 = p_spike_doc_->m_stimIntervals.intervalsArray.GetAt(ii) - iistart;
		if (iix0 >= iilen)				// first transition ON after last graph pt?
			break;						// yes = exit loop

		if (iix0 < 0)					// first transition off graph?
			iix0 = 0;					// yes = clip

		iix0 = MulDiv(displen, iix0, iilen) + rect->left;
		pDC->LineTo(iix0, istate);		// draw line up to the first point of the pulse
		pDC->LineTo(iix0, top);			// draw vertical line to top of pulse

		// stim ends here
		istate = bottom;				// after pulse, descend to bottom level
		int iix1 = p_spike_doc_->m_stimIntervals.intervalsArray.GetAt(ii+1) - iistart;
		if (iix1 > iilen)				// last transition off graph?
		{
			iix1 = iilen;				// yes = clip
			istate = top;				// do not descend..
		}

		iix1 = MulDiv(displen, iix1, iilen) + rect->left+1;

		pDC->LineTo(iix1, top);			// draw top of pulse
		pDC->LineTo(iix1, istate);		// draw descent to bottom line
	}
	pDC->LineTo(rect->left + displen, istate);		// end of loop - draw the rest
	pDC->SelectObject(poldP);
}


//---------------------------------------------------------------------------

void CSpikeBarWnd::DisplayBars(CDC* pDC, CRect* rect)
{
	// prepare loop to display spikes
	// load pen & font
	CPen* poldPen = (CPen*) pDC->SelectStockObject(BLACK_PEN);
	TEXTMETRIC txtmetric;
	pDC->GetTextMetrics(&txtmetric);

	// set mapping mode and viewport
	long xextent = rect->Width();
	if (m_yWE == 1)
	{		
		int maxval, minval;
		p_spike_list_->GetTotalMaxMin(TRUE, &maxval, &minval);
		m_yWE = maxval - minval;  
		m_yWO = (maxval + minval)/2;
	}
	int yextent = m_yWE;
	int yzero = m_yWO;
	int yVO = (int) (rect->Height()/2) + rect->top;
	int yVE = -rect->Height(); 

	// draw horizontal line
	int BASELINE = MulDiv(p_spike_list_->GetAcqBinzero()-yzero, yVE, yextent) + yVO;
	pDC->MoveTo(rect->left, BASELINE);
	pDC->LineTo(rect->right, BASELINE);

	// loop through all spikes of the list
	int ifirst = 0;	
	int ilast = p_spike_list_->GetTotalSpikes() -1;
	if (m_rangemode == RANGE_INDEX)
	{
		if (m_spklast > ilast) 
			m_spklast = ilast;
		if (m_spkfirst < 0) 
			m_spkfirst = 0;
		ilast = m_spklast;		// reduces the nb of spikes examined
		ifirst = m_spkfirst;	// assuming an ordered list...
	}
	long len =  (m_lLast - m_lFirst + 1);
	int max, min;
	
	int selpen = BLACK_COLOR;
	for (int ispk=ilast; ispk >= ifirst; ispk--)
	{
		// skip spike if outside time range && option
		long lSpikeTime = p_spike_list_->GetSpikeTime(ispk);
		if (m_rangemode == RANGE_TIMEINTERVALS
			&& (lSpikeTime < m_lFirst || lSpikeTime > m_lLast))
			continue;

		// select correct pen
		int wspkcla = p_spike_list_->GetSpikeClass(ispk);
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
		pDC->SelectObject(&m_penTable[selpen]); 
		
		// and draw spike: compute abcissa & draw from max to min     
		float llk = (lSpikeTime - m_lFirst)*(float)(xextent) / len; //muldiv
		int abcissa = (int) llk + rect->left;
		p_spike_list_->GetSpikeExtrema(ispk, &max, &min);
		max = MulDiv(max-yzero, yVE, yextent) + yVO;
		min = MulDiv(min-yzero, yVE, yextent) + yVO;
		pDC->MoveTo(abcissa, max);
		pDC->LineTo(abcissa, min);

		if (wspkcla >= 1)
		{
			TCHAR cletter = _T('0');
			cletter += wspkcla;
			pDC->TextOut(abcissa, min + (txtmetric.tmHeight/2 ), &cletter, 1);
		}
	}

	// display selected spike
	if (m_selectedspike >= 0)
		DisplaySpike(m_selectedspike, TRUE);
	if (p_spike_list_->GetSpikeFlagArrayCount() > 0)
		DisplayFlaggedSpikes(TRUE);

	// restore old pen
	pDC->SelectObject(poldPen);
}

//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------
// display spikes stored into SpikeFlagArray of a spike list 
// bselect: 0=normal; 1=select
// ------------------------------------------------------------------------------------

void CSpikeBarWnd::DisplayFlaggedSpikes(BOOL bHighLight)
{
	CClientDC dc(this);					// get DC
	if (m_xWE == 1 || m_yWE == 1)		// exit if bad coordinates
		return;
	m_xWE = m_displayRect.Width();			// adjust m_xWE
	m_xWO = m_displayRect.left;

	dc.IntersectClipRect(&m_clientRect);// clip rectangle
	PrepareDC(&dc);						// adjust scales

	// set correct color and draw mode
	int pensize = 0;
	CPen* oldpen = nullptr;
	int color;

	// loop over the array of flagged spikes
	for (int i= p_spike_list_->GetSpikeFlagArrayCount()-1; i>=0; i--)
	{
		int nospike = p_spike_list_->GetSpikeFlagArrayAt(i);
		int nospikeclass = p_spike_list_->GetSpikeClass(nospike);
		// skip spike if not valid in this display
		if (PLOT_ONECLASSONLY == m_plotmode && nospikeclass != m_selclass)
			continue;

		// spike is not selected
		if (!bHighLight)
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
		// spike is selected
		else
		{
			color = RED_COLOR;	
			/*switch (m_plotmode)
			{
			case PLOT_CLASSCOLORS:
				color = nospikeclass % 8;
				break;
			case PLOT_BLACK:
			case PLOT_ONECLASSONLY:
			case PLOT_ONECLASS:
			default:
				color = RED_COLOR;	
				break;
			}*/
		}
	
		// create pen corresponding to selection and set correct drawing mode
		CPen newPen;
		newPen.CreatePen(PS_SOLID, pensize, m_colorTable[color]);
		oldpen = (CPen*) dc.SelectObject(&newPen);

		// display data
		long lSpikeTime = p_spike_list_->GetSpikeTime(nospike);
		long len =  (m_lLast - m_lFirst + 1);
		float llk = (lSpikeTime - m_lFirst)*(float)(m_xWE) / len; //muldiv		
		int abcissa = (int)llk  + m_xWO;
		int max, min;
		p_spike_list_->GetSpikeExtrema(nospike, &max, &min);

		// ------------------------------------------------
		dc.MoveTo(abcissa, max);
		dc.LineTo(abcissa, min);

		// restitute objects
		dc.SelectObject(oldpen);
	}
}

void CSpikeBarWnd::DisplaySpike(int nospike, BOOL bselect)
{
	if (!IsSpikeWithinRange(nospike))
		return;

	CClientDC dc(this);					// get DC
	if (m_xWE == 1 || m_yWE == 1)		// exit if bad coordinates
		return;
	m_xWE = m_displayRect.Width();			// adjust m_xWE
	m_xWO = m_displayRect.left;

	dc.IntersectClipRect(&m_clientRect);// clip rectangle
	PrepareDC(&dc);						// adjust scales

	// set correct color and draw mode
	int pensize = 0;
	CPen* oldpen = nullptr;

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
	
	// create pen corresponding to selection and set correct drawing mode
	CPen newPen;
	newPen.CreatePen(PS_SOLID, pensize, m_colorTable[color]);
	oldpen = (CPen*) dc.SelectObject(&newPen);

	// display data
	long lSpikeTime = p_spike_list_->GetSpikeTime(nospike);
	long len =  (m_lLast - m_lFirst + 1);
	float llk = (lSpikeTime - m_lFirst)*(float)(m_xWE) / len; //muldiv
	int abcissa = (int) llk + m_xWO;
	int max, min;
	p_spike_list_->GetSpikeExtrema(nospike, &max, &min);

	// ------------------------------------------------
	dc.MoveTo(abcissa, max);
	dc.LineTo(abcissa, min);

	// restitute object
	dc.SelectObject(oldpen);
}

//---------------------------------------------------------------------------
// display individual spike
//---------------------------------------------------------------------------
BOOL CSpikeBarWnd::IsSpikeWithinRange(int spikeno)
{
	BOOL bYes = TRUE;
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

// ------------------------------------------------------------------------------------
// display individual spike as bar
// ------------------------------------------------------------------------------------

void CSpikeBarWnd::HighlightOneBar(int nospike, CDC* pDC)
{
	int oldROP = pDC->GetROP2();
	pDC->SetROP2(R2_NOTXORPEN);

	long lSpikeTime = p_spike_list_->GetSpikeTime(nospike);
	long len =  (m_lLast - m_lFirst + 1);
	float llk = (lSpikeTime - m_lFirst)*(float)(m_xWE) / len; //muldiv
	int abcissa = (int) llk + m_xWO;

	int max = MulDiv(1-m_yVO, m_yWE, m_yVE) + m_yWO;
	int min = MulDiv(m_displayRect.Height()-2-m_yVO, m_yWE, m_yVE) + m_yWO;

	CPen newPen;
	newPen.CreatePen(PS_SOLID, 1, RGB(196,   2,  51)); //RGB(255, 255, 255));
	CPen* oldpen = (CPen*) pDC->SelectObject(&newPen);

	// ------------------------------------------------

	pDC->MoveTo(abcissa-1, max);
	pDC->LineTo(abcissa+1, max);
	pDC->LineTo(abcissa+1, min);
	pDC->LineTo(abcissa-1, min);
	pDC->LineTo(abcissa-1, max);

	// restore resources	
	pDC->SelectObject(oldpen);
	pDC->SetROP2(oldROP);
}

//---------------------------------------------------------------------------

int	CSpikeBarWnd::SelectSpike(int spikeno)
{
	// erase old selected spike
	int oldselected = m_selectedspike; 
	if (m_selectedspike >= 0) // && m_selectedspike != spikeno)
		DisplaySpike(m_selectedspike, FALSE);

	// select new spike
	DisplaySpike(spikeno, TRUE);
	m_selectedspike = spikeno;

	return oldselected;
}

//-----------------------------------------------------------------------------
// flag all spikes within a rectangle in screen coordinates
void CSpikeBarWnd::SelectSpikesWithinRect(CRect* pRect, UINT nFlags)
{
	// make sure that the rectangle is ok
	int i;
	if (pRect->top > pRect->bottom)
	{
		i = pRect->top;
		pRect->top = pRect->bottom;
		pRect->bottom= i;
	}
	if (pRect->left > pRect->right)
	{
		i = pRect->left;
		pRect->left = pRect->right;
		pRect->right = i;
	}
	
	// convert pixel coordinates into file index and into data values
	long len =  (m_lLast - m_lFirst + 1);
	long l_first= MulDiv(pRect->left, len, m_displayRect.Width()) + m_lFirst;
	long l_last= MulDiv(pRect->right, len, m_displayRect.Width()) + m_lFirst;
	int vmin = MulDiv(pRect->bottom -m_yVO, m_yWE, m_yVE) + m_yWO;
	int vmax = MulDiv(pRect->top -m_yVO, m_yWE, m_yVE) + m_yWO;
	BOOL bFlag = FALSE;
	bFlag = (nFlags & MK_SHIFT) || (nFlags & MK_CONTROL);
	p_spike_list_->SelectSpikeswithinRect(vmin, vmax, l_first, l_last, bFlag);
}

////////////////////////////////////////////////////////////////////////////////////
// MOUSE related events

void CSpikeBarWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{		
	if (!m_bLmouseDown)
	{
		PostMyMessage(HINT_DROPPED, NULL);
		return;
	}

	CScopeScreen::OnLButtonUp(nFlags, point);
	CRect rectOut(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
	const int jitter = 5;
	if ((abs(rectOut.Height())< jitter) && (abs(rectOut.Width())< jitter))
	{
		if (m_cursorType != CURSOR_ZOOM)
			PostMyMessage(HINT_HITAREA, NULL);
		else
			ZoomIn();
		return;					// exit: mouse movement was too small
	}

	// perform action according to cursor type
	CRect rectIn = m_displayRect;
	switch (m_cursorType)
	{
		case 0:
			if (m_hitspk < 0)
			{
				CRect rect = GetDefinedRect();
				SelectSpikesWithinRect(&rect, nFlags);
				PostMyMessage(HINT_SELECTSPIKES, NULL);  // tell parent that new spikes have been selected...
			}
			break;

		case CURSOR_ZOOM: 	// zoom operation
			ZoomData(&rectIn, &rectOut);
			m_ZoomFrom = rectIn;
			m_ZoomTo   = rectOut;					
			m_iUndoZoom = 1;
			PostMyMessage(HINT_SETMOUSECURSOR, m_oldcursorType);
			break;

		default:
			break;
	}	
}


//---------------------------------------------------------------------------

void CSpikeBarWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLmouseDown = TRUE;

	// detect bar hit: test if curve hit -- specific to SpikeBarButton
	if (m_currCursorMode==0)				// only if cursor mode = 0 (arrow)
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
	// call base class to test for horiz cursor or XORing rectangle	
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
	rFrom->NormalizeRect();	// make sure that rect is not inverted
	rDest->NormalizeRect();
	
	// change y gain & y offset		
	int y_we = m_yWE;				// save previous window extent
	m_yWE = MulDiv (m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
			-MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
			+MulDiv(rDest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed	
	long lSize = m_lLast-m_lFirst+1;
	m_lFirst = m_lFirst + lSize*(rDest->left-rFrom->left)/rFrom->Width();
	if (m_lFirst <0) 
		m_lFirst = 0;
	lSize = lSize *rDest->Width() / rFrom->Width();
	m_lLast = m_lFirst +lSize -1;
		
	// call for new data only if indexes are different
	PostMyMessage(HINT_CHANGEHZLIMITS, NULL);	// code = 3: horizontal limits have changed lowp = unused

}

//---------------------------------------------------------------------------
void CSpikeBarWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_selectedspike < 0 && p_spike_list_->GetSpikeFlagArrayCount () < 1)|| m_hitspk < 0)
		CScopeScreen::OnLButtonDblClk(nFlags, point);
	else
	{
		//single spike selected?
		if (m_selectedspike >= 0)
		{
			PostMyMessage(HINT_DBLCLKSEL, m_selectedspike);
		}
		// multiple spikes selected - find the right one...
		else
		{
			int iselectedspike = DoesCursorHitCurve(point);
			if (iselectedspike > 0)
				PostMyMessage(HINT_DBLCLKSEL, iselectedspike);
		}
	}
}

//---------------------------------------------------------------------------

int CSpikeBarWnd::DoesCursorHitCurve(CPoint point)
{
	int hitspk = -1;
	// for y coordinates, conversion is straightforward:
	int mouseY = MulDiv(point.y-m_yVO, m_yWE, m_yVE) + m_yWO;
	int deltay = MulDiv(3, m_yWE, m_yVE);

	// for x coordinates, the formula is in 2 steps:
	// 1) time -> relative time: iitime = (spiketime-m_lFirst) [-offset]
	// 2) rel time -> logical coordinate(LC): LC = iitime* m_xWE / len + m_xWO
	// formula used to display: time (long) --> abcissa (int)
	// 		long len =  (m_lLast - m_lFirst + 1);
	// 		int abcissa = (int) (((lSpikeTime - m_lFirst) * (long) m_xWE) / len) + m_xWO;	
	// reverse formula: abcissa -> time
	//		long lSpikeTime  = (abcissa - m_xWO)*len/m_xWE + m_lFirst;
	// convert device coordinates into logical coordinates
	int mouseX = MulDiv(point.x-m_xVO, m_xWE, m_xVE) + m_xWO;
	int deltax = MulDiv(3, m_xWE, m_xVE);
	long len =  (m_lLast - m_lFirst + 1);	// size of data displayed	

	// find a spike which time of occurence fits between lXmax and lXmin
	long lXmax = m_lFirst + len*(mouseX + deltax)/(long)m_xWE;
	long lXmin = m_lFirst + len*(mouseX - deltax)/(long)m_xWE;
	
	// loop through all spikes
	int ifirst = 0;
	int ilast = p_spike_list_->GetTotalSpikes()-1;
	// loop over all spikes except if range mode is selected
	if (m_rangemode == RANGE_INDEX)	
	{
		if (m_spklast > ilast) m_spklast = ilast;
		if (m_spkfirst < 0) m_spkfirst = 0;
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}

	// spikes displayed as a bar ----------------------------------
	for (int ispk=ilast; ispk>=ifirst; ispk--)
	{
		long lSpikeTime = p_spike_list_->GetSpikeTime(ispk);
		if (lSpikeTime < lXmin || lSpikeTime > lXmax)
			continue;
		if (m_plotmode == PLOT_ONECLASSONLY
			&& p_spike_list_->GetSpikeClass(ispk) != m_selclass)
			continue;

		int max;
		int min;
		p_spike_list_->GetSpikeExtrema(ispk, &max, &min);
		if (mouseY+deltay < max && mouseY-deltay > min)
		{			
			hitspk = ispk;
			break;
		}
	}
	// exit
	return hitspk;
}


//---------------------------------------------------------------------------
void CSpikeBarWnd::CenterCurve()
{
	if (p_spike_list_ == nullptr || p_spike_list_->GetTotalSpikes() <= 0)
		return;
	int max, min;		
	p_spike_list_->GetTotalMaxMin(TRUE, &max, &min);
	m_yWO = max/2 + min/2;
}

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
// Print()
//---------------------------------------------------------------------------

void CSpikeBarWnd::Print(CDC* pDC, CRect* rect)
{
	// check if there are valid data to display
	if (p_spike_list_ == nullptr || p_spike_list_->GetTotalSpikes()== 0)
		return;		

	// set mapping mode and viewport
	int nSavedDC = pDC->SaveDC();				// save display context	
	DisplayBars(pDC, rect);
	if (p_spike_doc_->m_stimIntervals.nitems > 0)	
		DisplayStim(pDC, rect);

	pDC->RestoreDC(nSavedDC);	
}


void CSpikeBarWnd::Serialize( CArchive& ar )
{
	CScopeScreen::Serialize(ar);

	BOOL bdummy=TRUE;
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