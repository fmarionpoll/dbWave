// SpikeShapeWnd.cpp Implementation File

#include "stdafx.h"
#include "scopescr.h"
#include "spikedoc.h"
#include "dbWaveDoc.h"
#include "spikeshape.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL (CSpikeShapeWnd, CScopeScreen, 1)

//////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSpikeShapeWnd, CScopeScreen)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////////////

CSpikeShapeWnd::CSpikeShapeWnd()
{                                        
	m_pSL=NULL;
	m_lFirst = 0;
	m_lLast = 0;
	m_currentclass=-999;
	m_btrackCurve = FALSE;
	m_hitspk=-1;
	m_selectedspike=-1;
	m_rangemode = RANGE_TIMEINTERVALS;
	m_colorselectedspike = RED_COLOR;
	m_bText = FALSE;
	SetbUseDIB(FALSE); 
	m_csEmpty = "no \nspikes";	
	m_ballFiles = FALSE;
	m_pDoc = NULL;
}

// ---------------------------------------------------------------------------------

CSpikeShapeWnd::~CSpikeShapeWnd()
{
}

////////////////////////////////////////////////////////////////////////////////////
// DISPLAY SPIKES
//
// Display(CDC*)
// DisplaySpike(no)
// XORtrackLine()
// SelectSpike(n)
////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------

void CSpikeShapeWnd::PlotDatatoDC(CDC* pDC)
{
	if (m_erasebkgnd)			// erase background
		EraseBkgnd(pDC);


	// display data: trap error conditions
	int nSavedDC = pDC->SaveDC();
	GetExtents();
	pDC->SetViewportOrg (m_displayRect.left, m_displayRect.Height()/2);
	pDC->SetViewportExt (m_displayRect.right, -m_displayRect.Height());

	PrepareDC(pDC);
	long nfiles = 1;
	long ncurrentfile = 0;
	if (m_ballFiles)
	{
		nfiles = m_pDoc->DBGetNRecords();
		ncurrentfile = m_pDoc->DBGetCurrentRecordPosition();
	}

	for (long ifile = 0; ifile < nfiles; ifile++)
	{
		if (m_ballFiles)
		{
			m_pDoc->DBSetCurrentRecordPosition(ifile);
			m_pDoc->OpenCurrentSpikeFile();
			m_pSL = (CSpikeList*)m_pDoc->m_pSpk->GetSpkListCurrent();
		}

		//test if data are there - if none, write it and exit
		if (m_pSL == NULL || m_pSL->GetTotalSpikes() == 0)
		{
			if (!m_ballFiles)
			{
				pDC->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
				CRect rect = m_displayRect;
				rect.DeflateRect(1, 1);
				int textlen = m_csEmpty.GetLength();
				pDC->DrawText(m_csEmpty, textlen, rect, DT_LEFT); //|DT_WORDBREAK);
				return;
			}
			else
				continue;
		}
		// load resources and prepare context	
		int taillespk = m_pSL->GetSpikeLength();
		ASSERT(taillespk >0);

		if (m_polypts.GetSize() != taillespk * 2)
		{
			m_polypts.SetSize(taillespk * 2, 2);
			InitPolypointAbcissa();
		}

		// loop through all spikes of the list
		short* lpspk;							// pointer to spk
		long* lpDest = (long*)&m_polypts[0];	// pointer to array displayed
		// prepare pen    
		int oldcla = 0;							// old class displayed

		// loop to display spikes
		int ilast = m_pSL->GetTotalSpikes() - 1;
		int ifirst = 0;
		if (m_rangemode == RANGE_INDEX)
		{
			ilast = m_spklast;
			ifirst = m_spkfirst;
		}
		int selpen = BLACK_COLOR;
		if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECOLOR)
			selpen = SILVER_COLOR;
		CPen* poldPen = pDC->SelectObject(&m_penTable[selpen]);

		for (int ispk = ilast; ispk >= ifirst; ispk--)
		{
			// skip spike ?
			if (m_rangemode == RANGE_TIMEINTERVALS
				&& (m_pSL->GetSpikeTime(ispk) < m_lFirst
					|| m_pSL->GetSpikeTime(ispk) > m_lLast))
				continue;

			// select pen according to class
			int wspkcla = m_pSL->GetSpikeClass(ispk);
			switch (m_plotmode)
			{
			case PLOT_ONECLASSONLY:
				if (wspkcla != m_selclass)
					continue;
				break;
			case PLOT_CLASSCOLORS:
				selpen = wspkcla % NB_COLORS;
				pDC->SelectObject(&m_penTable[selpen]);
				break;
			case PLOT_ONECLASS:
				if (wspkcla == m_selclass)
					continue;
			default:
				break;
			}

			// display data
			lpspk = m_pSL->GetpSpikeData(ispk);
			FillPolypointOrdinates(lpspk);
			pDC->Polyline((LPPOINT)lpDest, taillespk);
		}

		if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECOLOR)
		{
			selpen = m_colorselected;
			if (m_plotmode == PLOT_ONECOLOR)
				selpen = m_selclass % NB_COLORS;
			pDC->SelectObject(&m_penTable[selpen]);
			for (int ispk = ilast; ispk >= ifirst; ispk--)
			{
				// skip spike ?
				if (m_rangemode == RANGE_TIMEINTERVALS
					&& (m_pSL->GetSpikeTime(ispk) < m_lFirst
						|| m_pSL->GetSpikeTime(ispk) > m_lLast))
					continue;

				// skip spikes with the wrong class
				if (m_pSL->GetSpikeClass(ispk) != m_selclass)
					continue;
				// display data
				lpspk = m_pSL->GetpSpikeData(ispk);
				FillPolypointOrdinates(lpspk);
				pDC->Polyline((LPPOINT)lpDest, taillespk);
			}
		}

		// display selected spike
		int iselect = -1;
		if (m_selectedspike >= 0 && (IsSpikeWithinRange(m_selectedspike)))
			iselect = m_selectedspike;
		DrawSelectedSpike(iselect, pDC);

		if (m_pSL->GetSpikeFlagArrayCount() > 0)
			DrawFlaggedSpikes(pDC);

		// display tags
		if (GetNHZtags() > 0)
			DisplayHZtags(pDC);

		if (GetNVTtags() > 0)
			DisplayVTtags(pDC);

		// restore resources		
		pDC->SelectObject(poldPen);
		pDC->RestoreDC(nSavedDC);

		// display text
		if (m_bText && m_plotmode == PLOT_ONECLASSONLY)
		{
			TCHAR num[10];
			wsprintf(num, _T("%i"), GetSelClass());
			pDC->TextOut(1, 1, num);
		}
	}
	
	if (m_ballFiles)
	{
		m_pDoc->DBSetCurrentRecordPosition(ncurrentfile);
		m_pDoc->OpenCurrentSpikeFile();
		m_pSL = (CSpikeList*)m_pDoc->m_pSpk->GetSpkListCurrent();
	}
}
// ---------------------------------------------------------------------------------

void CSpikeShapeWnd::DrawSelectedSpike(int nospike, CDC* pDC)
{
	CBitmap* poldbitmap = NULL;
	int nSavedDC = pDC->SaveDC();
	CRect rect = m_displayRect;
	pDC->DPtoLP(rect);
	pDC->IntersectClipRect(&rect);
	
	if (nospike >= 0)
	{
		// change coordinate settings
		GetExtents();
		PrepareDC(pDC);
		
		pDC->SetViewportOrg (m_displayRect.left, m_displayRect.Height()/2 + m_displayRect.top);
		pDC->SetViewportExt (m_displayRect.Width(), -m_displayRect.Height());
		
		// prepare pen and select pen
		int pensize = 2;
		CPen newPen(PS_SOLID, pensize, m_colorTable[m_colorselectedspike]);
		CPen* poldpen = (CPen*) pDC->SelectObject(&newPen);

		// display data
		short* lpspk = m_pSL->GetpSpikeData(nospike);
		FillPolypointOrdinates(lpspk);
		long* lpDest = (long*) &m_polypts[0];
		pDC->Polyline((LPPOINT) lpDest, m_pSL->GetSpikeLength());

		// restore resources
		pDC->SelectObject(poldpen);
	}
	// restore ressources
	pDC->RestoreDC(nSavedDC);	
}

// ---------------------------------------------------------------------------------

void CSpikeShapeWnd::DrawFlaggedSpikes(CDC* pDC0)
{
	CBitmap* poldbitmap = NULL;
	ASSERT (pDC0 != NULL);
	CDC* pDC = pDC0;
	int nSavedDC = pDC->SaveDC();	

	// change coordinate settings 
	GetExtents();
	PrepareDC(pDC);
	pDC->SetViewportOrg (m_displayRect.left, m_displayRect.Height()/2);
	pDC->SetViewportExt (m_displayRect.right, -m_displayRect.Height());

	// prepare pen and select pen
	int pensize = 1;
	CPen newPen(PS_SOLID, pensize, m_colorTable[m_colorselectedspike]);
	CPen* oldpen = (CPen*) pDC->SelectObject(&newPen);

	// loop through all flagged spikes
	for (int i= m_pSL->GetSpikeFlagArrayCount()-1; i>=0; i--)
	{
		int nospike = m_pSL->GetSpikeFlagArrayAt(i);
		int nospikeclass = m_pSL->GetSpikeClass(nospike);
		// skip spike if not valid in this display
		if (!IsSpikeWithinRange(nospike))
			continue;
		//if (PLOT_ONECLASSONLY == m_plotmode && nospikeclass != m_selclass)
		//	continue;
		short* lpspk = m_pSL->GetpSpikeData(nospike);
		FillPolypointOrdinates(lpspk);
		long* lpDest = (long*) &m_polypts[0];
		pDC->Polyline((LPPOINT) lpDest, m_pSL->GetSpikeLength());
	}

	// restore resources
	pDC->SelectObject(oldpen);
	pDC0->RestoreDC(nSavedDC);
}

void CSpikeShapeWnd::DisplayFlaggedSpikes (BOOL bHighLight)
{
	if (bHighLight)
		DrawFlaggedSpikes(&m_PlotDC); 
	Invalidate();
}

// ---------------------------------------------------------------------------------

int	CSpikeShapeWnd::DisplayExData(short* pData, int color)
{
	// prepare array
	int nelements = m_pSL->GetSpikeLength();
	if (m_polypts.GetSize() != nelements *2)
	{
		m_polypts.SetSize(nelements *2, 2);
		InitPolypointAbcissa();	
	}
	
	// prepare DC
	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);
	PrepareDC(&dc);

	// display data
	CPen newPen(PS_SOLID, 0, m_colorTable[color]);
	CPen* oldpen = (CPen*) dc.SelectObject(&newPen);

	// display data
	FillPolypointOrdinates(pData);

	// display data		
	long* lpDest = (long*) &m_polypts[0];
	dc.Polyline((LPPOINT) lpDest, m_pSL->GetSpikeLength());

	// restore resources
	dc.SelectObject(oldpen);

	return color;
}

// ---------------------------------------------------------------------------------

BOOL CSpikeShapeWnd::IsSpikeWithinRange(int spikeno)
{	
	if (spikeno > m_pSL->GetTotalSpikes()-1)
		return FALSE;
	if (m_rangemode == RANGE_TIMEINTERVALS
		&& (m_pSL->GetSpikeTime(spikeno) < m_lFirst || m_pSL->GetSpikeTime(spikeno) > m_lLast))
		return FALSE;
	else if (m_rangemode == RANGE_INDEX
		&& (spikeno>m_spklast || spikeno < m_spkfirst))
		return FALSE;
	if (m_plotmode == PLOT_ONECLASSONLY 
		&& (m_pSL->GetSpikeClass(spikeno) != m_selclass))
		return FALSE;
	return TRUE;
}



// ------------------------------------------------------------------------------------

int	CSpikeShapeWnd::SelectSpikeShape(int spikeno)
{
	// erase plane
	int oldselected = m_selectedspike;
	m_selectedspike = spikeno;
	
	if (!m_bUseDIB)
	{
		CClientDC dc(this) ;
		DrawSelectedSpike(m_selectedspike, &dc); 
	}
	else
	{
		if (m_PlotDC.GetSafeHdc())
			DrawSelectedSpike(m_selectedspike, &m_PlotDC);
	}
	Invalidate();
	return oldselected;	
}


////////////////////////////////////////////////////////////////////////////////////
// MOUSE OPERATIONS
// OnLButtonUp()
// OnLButtonDown(UINT nFlags, CPoint point)
// OnMouseMove(UINT nFlags, CPoint point)
// ZoomData(CRect* r1, CRect* r2)
////////////////////////////////////////////////////////////////////////////////////

void CSpikeShapeWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bLmouseDown)
		{
			PostMyMessage(HINT_DROPPED, NULL);
			return;
		}

	switch (m_trackMode)
	{
	case TRACK_BASELINE:
		// curve was tracked
		{
		if (point.y != m_ptLast.y || point.x != m_ptLast.x)
			OnMouseMove(nFlags, point);
		m_trackMode = TRACK_OFF;
		ReleaseCursor();
		CScopeScreen::OnLButtonUp(nFlags, point);		
		}
		break;

	case TRACK_VTTAG:
	// vertical tag was tracked
		{
		// convert pix into data value and back again
		int val = MulDiv(point.x-m_xVO, m_xWE, m_xVE)+m_xWO;
		SetVTtagVal(m_HCtrapped, val);
		point.x=MulDiv(val-m_xWO, m_xVE, m_xWE)+m_xVO;
		XorVTtag(point.x);
		CScopeScreen::OnLButtonUp(nFlags, point);
		PostMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);
		}
		break;

	default:
		{

		// none of those: zoom data or  offset display
		CScopeScreen::OnLButtonUp(nFlags, point);
		CRect rectOut(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
		const int jitter = 3;
		if ((abs(rectOut.Height())< jitter) && (abs(rectOut.Width())< jitter))
		{		
			if (m_cursorType != CURSOR_ZOOM)
				PostMyMessage(HINT_HITAREA, NULL);
			else
				ZoomIn();
			return;					// exit: mouse movement was too small
		}

		// perform action according to cursor type
		CRect rectIn= m_displayRect;
		switch (m_cursorType)
		{
		case 0:
			rectOut = rectIn;
			rectOut.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
			PostMyMessage(HINT_DEFINEDRECT, NULL);	// tell parent that val changed
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
		break;
	}
}


// ------------------------------------------------------------------------------------

void CSpikeShapeWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLmouseDown = TRUE;

	// call base class to test for horiz cursor or XORing rectangle	
	if (GetNVTtags() > 0)
	{
		for (int icur = GetNVTtags()-1; icur>=0; icur--)	// loop through all tags
			SetVTtagPix(icur, MulDiv(GetVTtagVal(icur)-m_xWO, m_xVE, m_xWE)+m_xVO);
	}

	// track rectangle or VTtag?
	CScopeScreen::OnLButtonDown(nFlags, point);		// capture cursor eventually
	if (m_currCursorMode!=0 || m_HCtrapped >= 0)// do nothing else if mode != 0
		return;	 								// or any tag hit (VT, HZ) detected
	
	// test if mouse hit one spike
	// if hit, then tell parent to select corresp spike
	m_hitspk = DoesCursorHitCurve(point);
	if (m_hitspk >= 0)
	{
		// cancel track rect mode
		m_trackMode = TRACK_OFF;		// flag trackrect
		ReleaseCursor();				// release cursor capture
		if (nFlags & MK_SHIFT)
			PostMyMessage(HINT_HITSPIKE_SHIFT, m_hitspk);	// tell parent spike selected

		else
			PostMyMessage(HINT_HITSPIKE, m_hitspk);		
		return;
	}
}

// ------------------------------------------------------------------------------------

void CSpikeShapeWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	CScopeScreen::OnMouseMove(nFlags, point);
}

// ------------------------------------------------------------------------------------
// ZoomData(CRect* rFrom, CRect* rDest)
//
// max and min of rFrom should fit in rDest (same logical coordinates)
// then one can write the 4 equations: 
// assume initial conditions WE1, WO1; destination: WE2, WO2
// LPmin = (rFrom.(top/left)      - VO) * WE1 / VE + WO1	(1)
// LPMax = (rFrom.(bottom, right) - VO) * WE1 / VE + WO1	(2)
// LPmin = (rDest.(top/left)      - VO) * WE2 / VE + WO2	(3)
// LPMax = (rDest.(bottom, right) - VO) * WE2 / VE + WO2	(4)
// from (1)-(2) = (3)-(4) one get WE2
// from (1)=(3)               get WO2
// ------------------------------------------------------------------------------------

void CSpikeShapeWnd::ZoomData(CRect* rFrom, CRect* rDest)
{
	rFrom->NormalizeRect();	// make sure that rect is not inverted
	rDest->NormalizeRect();

	// change y gain & y offset		
	int yWE = m_yWE;				// save previous window extent
	m_yWE = MulDiv (m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
			-MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
			+MulDiv(rDest->top - m_yVO, yWE, m_yVE);

	// change index of first and last pt displayed	
	int xWE = m_xWE;				// save previous window extent
	m_xWE = MulDiv (m_xWE, rDest->Width(), rFrom->Width());
	m_xWO = m_xWO
			-MulDiv(rFrom->left - m_xVO, m_xWE, m_xVE)
			+MulDiv(rDest->left - m_xVO, xWE, m_xVE);

	// display
	Invalidate();
	PostMyMessage(HINT_CHANGEZOOM, 0);
}

// ------------------------------------------------------------------------------------

void CSpikeShapeWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_selectedspike < 0 && m_pSL->GetSpikeFlagArrayCount () < 1)|| m_hitspk < 0)
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


// ------------------------------------------------------------------------------------

int  CSpikeShapeWnd::DoesCursorHitCurve(CPoint point)
{
	int hitspk = -1;
	// convert device coordinates into logical coordinates
	int mouseX = MulDiv(point.x-m_xVO, m_xWE, m_xVE) + m_xWO;
	if (mouseX <0 || mouseX > m_pSL->GetSpikeLength())
		return hitspk;
	int mouseY = MulDiv(point.y-m_yVO, m_yWE, m_yVE) + m_yWO;
	int deltax = MulDiv(3, m_xWE, m_xVE);
	int deltay = MulDiv(3, m_yWE, m_yVE);

	// loop through all spikes
	int ilast = m_pSL->GetTotalSpikes()-1;
	int ifirst = 0;
	if (m_rangemode == RANGE_INDEX)	
	{
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}
	for (int ispk=ilast; ispk>=ifirst; ispk--)
	{
		// skip spikes out of range or with not correct class
		if (m_rangemode == RANGE_TIMEINTERVALS
			&& (m_pSL->GetSpikeTime(ispk) < m_lFirst 
			   || m_pSL->GetSpikeTime(ispk) > m_lLast))
			continue;
		if (m_plotmode == PLOT_ONECLASSONLY
		 && m_pSL->GetSpikeClass(ispk) != m_selclass)
			continue;
		// if spike accepted, test value
		int val = m_pSL->GetSpikeValAt(ispk, mouseX);
		if (mouseY+deltay < val && mouseY-deltay > val)
		{			
			hitspk = ispk;
			break;
		}
	}	
	return hitspk;
}

// ------------------------------------------------------------------------------------

void CSpikeShapeWnd::GetExtents()
{
	long nfiles = 1;
	long ncurrentfile = 0;
	if (m_ballFiles)
	{
		nfiles = m_pDoc->DBGetNRecords();
		ncurrentfile = m_pDoc->DBGetCurrentRecordPosition();
	}

	for (long ifile = 0; ifile < nfiles; ifile++)
	{
		if (m_ballFiles)
		{
			m_pDoc->DBSetCurrentRecordPosition(ifile);
			m_pDoc->OpenCurrentSpikeFile();
			m_pSL = (CSpikeList*)m_pDoc->m_pSpk->GetSpkListCurrent();
		}

		if (m_yWE == 1 || m_yWE == 0) // && m_yWO == 0)
		{
			int maxval, minval;
			m_pSL->GetTotalMaxMin(TRUE, &maxval, &minval);
			m_yWE = MulDiv((maxval - minval), 10, 9) +1;	// avoid null ext
			m_yWO = maxval / 2 + minval / 2;
		}

		if (m_xWE == 1) // && m_xWO == 0)
		{
			m_xWE = m_pSL->GetSpikeLength(); //+1;
			m_xWO = 0;
		}
	}

	// exit
	if (m_ballFiles)
	{
		m_pDoc->DBSetCurrentRecordPosition(ncurrentfile);
		m_pDoc->OpenCurrentSpikeFile();
		m_pSL = (CSpikeList*)m_pDoc->m_pSpk->GetSpkListCurrent();
	}
}

// ------------------------------------------------------------------------------------

void CSpikeShapeWnd::InitPolypointAbcissa()
{
	int nelements = (m_polypts.GetSize())/2;
	m_xWE = nelements +1;
	ASSERT(nelements > 0);
	long* lpDest = (long*) &m_polypts[0];	
	for (int i = 1; i<=nelements; i++)
	{
		*lpDest = i;	// copy data
		lpDest++;		// point to the next element, 2 positions after
		lpDest++;		// (keep space for either abcissa or ordinates)		
	}
}

// ------------------------------------------------------------------------------------

void CSpikeShapeWnd::FillPolypointOrdinates(short* lpSource)
{
	int nelements = (m_polypts.GetSize())/2;
	if (nelements == 0)
	{
		nelements = m_pSL->GetSpikeLength();
		ASSERT(nelements > 0);
		m_polypts.SetSize(nelements *2, 2);
		InitPolypointAbcissa();	
	}

	long* lpDest = (long*) &m_polypts[1];	// source data: an Envelope	
	for (int i = 0; i<nelements; i++)
	{
		*lpDest = *lpSource;	// copy data
		lpDest++;				// point to the next element, 2 positions after
		lpDest++;				// (keep space for either abcissa or ordinates)
		lpSource++;				// point to the next source element
	}
}

//---------------------------------------------------------------------------
// Print()
//---------------------------------------------------------------------------

void CSpikeShapeWnd::Print(CDC* pDC, CRect* rect)
{
	// check if there are valid data to display
	if (m_pSL == NULL || m_pSL->GetTotalSpikes()== 0)
		return;

	int old_yVO = m_yVO;
	int old_yVE = m_yVE;
	int oldXextent = m_xWE;
	int oldXorg = m_xWO;
	
	// size of the window
	m_yVO = (int) (rect->Height()/2) + rect->top;
	m_yVE = -rect->Height();

	// check initial conditions
	if (m_yWE == 1) // && m_yWO == 0)				// test window parms
	{
		int maxval, minval;						// if not set, search max, min
		m_pSL->GetTotalMaxMin(TRUE, &maxval, &minval);
		m_yWE = maxval - minval +1;  				// center / ordinates origin
		m_yWO = (maxval + minval)/2;			// extent and origin		
	}

	m_xWO = rect->left;
	m_xWE = rect->Width()-2;

	int taillespk = m_pSL->GetSpikeLength();	// adjust horiz size
	if (m_polypts.GetSize() != taillespk*2)
		m_polypts.SetSize(taillespk*2, 2);

	// set mapping mode and viewport
	int nSavedDC = pDC->SaveDC();				// save display context	

	// init polypoint abcissa
	for (int i=0; i<taillespk; i++)
		m_polypts[i*2] = rect->left + MulDiv(i, rect->Width(), taillespk);

	// loop through all spikes of the list
		// prepare pen    
	int selpen;								// index selected pen
	int oldcla = 0;							// old class displayed
	switch (m_plotmode)
	{
		//case PLOT_BLACK:		selpen = BLACK_COLOR; break;
		//case PLOT_ONECLASSONLY:	selpen = BLACK_COLOR; break;
		case PLOT_ONECLASS:		selpen = m_colorbackgr; 
								oldcla = m_selclass; break;
		case PLOT_ALLGREY:		selpen = m_colorbackgr;	break;
		default:				selpen = BLACK_COLOR; break;
	}

	CPen* oldPen = pDC->SelectObject(&m_penTable[selpen]);

	int ilast=m_pSL->GetTotalSpikes()-1;
	int ifirst= 0;
	if (m_rangemode == RANGE_INDEX)
	{
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}

	for (int ispk=ilast; ispk >= ifirst; ispk--)
	{
		// skip spike ?
		if (m_rangemode == RANGE_INDEX && (ispk > m_spklast || ispk < m_spkfirst))
			continue;
		else if (m_rangemode == RANGE_TIMEINTERVALS)
		{
			if (m_pSL->GetSpikeTime(ispk) < m_lFirst)
				continue;
			if (m_pSL->GetSpikeTime(ispk) > m_lLast)
				continue;
		}
		
		// select pen according to class
		int spkcla = m_pSL->GetSpikeClass(ispk);
		if (m_plotmode == PLOT_ONECLASSONLY && spkcla != m_selclass)
			continue;
		if (m_plotmode == PLOT_ONECLASS && spkcla == m_selclass)
			continue;

		// display data 
		PlotArraytoDC(pDC, m_pSL->GetpSpikeData(ispk));
	}

	// display selected class if requested by option
	if (m_plotmode == PLOT_ONECLASS)
	{
		pDC->SelectObject(&m_penTable[m_colorselected]);

		// loop to display spikes			
		for (int ispk=ilast; ispk >= ifirst; ispk--)
		{
			// skip spike ?
			if (m_rangemode == RANGE_TIMEINTERVALS)
			{
				long ltime = m_pSL->GetSpikeTime(ispk);
				if (ltime < m_lFirst || ltime > m_lLast)
					continue;
			}
			if ( m_pSL->GetSpikeClass(ispk) != m_selclass)
				continue;

			// display data 
			PlotArraytoDC(pDC, m_pSL->GetpSpikeData(ispk));
		}
	}	

	// display selected spike	
	if (m_selectedspike >= 0 && IsSpikeWithinRange(m_selectedspike))
	{
		CPen newPen(PS_SOLID, 0, m_colorTable[m_colorselectedspike]);
		pDC->SelectObject(&newPen);
		PlotArraytoDC(pDC, m_pSL->GetpSpikeData(m_selectedspike));
	}

	// restore resources
	pDC->SelectObject(oldPen);	// restore pen
	pDC->RestoreDC(nSavedDC);	// restore DC

	m_xWE = oldXextent;			// restore old X extent
	m_xWO = oldXorg;			// restore old X origin
	m_yVO = old_yVO;			// same with Y
	m_yVE = old_yVE;
}

// ---------------------------------------------------------------------------------

void CSpikeShapeWnd::PlotArraytoDC(CDC* pDC, short* pspk)
{
	// (1) transfer to polypoint & adjust amplitude
	long* lpDest0 = (long*) &m_polypts[0];
	long* lpDest = lpDest0 + 1;
	int nelements = (m_polypts.GetSize())/2;
	
	// (2) scale all points and copy them into polypoint
	for (int i = 0; i<nelements; i++, lpDest++, lpDest++, pspk++)
	{
		short y = *pspk;							// read data
		y = MulDiv(y-m_yWO, m_yVE, m_yWE) + m_yVO;	// scale it
		*lpDest = y;								// copy it into polypoint
	}

	// (3) display polypoint
	CPoint* pP = (CPoint*) lpDest0;	
	BOOL bPolyLine = (pDC->m_hAttribDC == NULL) 
		|| (pDC->GetDeviceCaps(LINECAPS) && LC_POLYLINE);
	if (bPolyLine)							// polypoint enabled
		pDC->Polyline(pP, nelements);		// draw curve : polypoint
	else									// no polypoint capabilities
	{
		pDC->MoveTo(*pP);					// move current position
		for (int i=0; i<nelements; i++, pP++)	// loop to draw all points
			pDC->LineTo(*pP);
	}
}

// ---------------------------------------------------------------------------------

float CSpikeShapeWnd::GetDisplayMaxMv() 
{
	GetExtents();
	return (m_pSL->GetAcqVoltsperBin()*1000.f*(m_yWE-m_yWO-m_pSL->GetAcqBinzero()));
}

// ---------------------------------------------------------------------------------

float CSpikeShapeWnd::GetDisplayMinMv()
{
	if (m_pSL == NULL)
		return 1.f;

	GetExtents();
	return (m_pSL->GetAcqVoltsperBin()*1000.f*(m_yWO-m_yWE-m_pSL->GetAcqBinzero()));
}

// ---------------------------------------------------------------------------------

float CSpikeShapeWnd::GetExtent_mV()
{
	if (m_pSL == NULL)
		return 1.f;

	GetExtents();
	return (m_pSL->GetAcqVoltsperBin()*m_yWE*1000.f);
}

// ---------------------------------------------------------------------------------

float CSpikeShapeWnd::GetExtent_ms()
{
	if (m_pSL == NULL)
		return 1.f;

	GetExtents();
	return ((float)(1000.0*m_xWE)/m_pSL->GetAcqSampRate());
}

// ---------------------------------------------------------------------------------

void CSpikeShapeWnd::MoveVTtrack(int itrack, int newval)
{
	CPoint point;
	m_ptLast.x=MulDiv(GetVTtagVal(itrack)-m_xWO, m_xVE, m_xWE)+m_xVO;
	SetVTtagVal(itrack, newval);			// set new value	
	point.x=MulDiv(newval-m_xWO, m_xVE, m_xWE)+m_xVO;	// convert val into pixel	
	XorVTtag(point.x);						// xor line
}


void CSpikeShapeWnd::Serialize( CArchive& ar )
{
	CScopeScreen::Serialize(ar);
	m_polypts.Serialize(ar);		// points displayed with polyline

	if (ar.IsStoring())
	{
		ar << m_rangemode;			// display range (time OR storage index)
		ar << m_lFirst;				// time first (real time = index/sampling rate)
		ar << m_lLast;				// time last
		ar << m_spkfirst;			// index first spike
		ar << m_spklast;			// index last spike
		ar << m_currentclass;		// selected class (different color) (-1 = display all)
		ar << m_selectedspike;		// selected spike (display differently)
		ar << m_colorselectedspike; // color selected spike (index / color table)
		ar << m_hitspk;				// index spike
		ar << m_selclass;			// index class selected
		ar << m_bText;				// allow text default false
		ar << m_selclass;			// dummy
	}
	else
	{		
		ar >> m_rangemode;			// display range (time OR storage index)
		ar >> m_lFirst;				// time first (real time = index/sampling rate)
		ar >> m_lLast;				// time last
		ar >> m_spkfirst;			// index first spike
		ar >> m_spklast;			// index last spike
		ar >> m_currentclass;		// selected class (different color) (-1 = display all)
		ar >> m_selectedspike;		// selected spike (display differently)
		ar >> m_colorselectedspike; // color selected spike (index / color table)
		ar >> m_hitspk;				// index spike
		ar >> m_selclass;			// index class selected
		ar >> m_bText;				// allow text default false
		ar >> m_selclass;			// dummy
	}
}