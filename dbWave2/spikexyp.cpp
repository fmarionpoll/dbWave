// spikexyp.cpp Implementation File

#include "StdAfx.h"
#include "scopescr.h"
#include "Spikedoc.h"
#include "spikexyp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSpikeXYpWnd, CScopeScreen)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////////////


CSpikeXYpWnd::CSpikeXYpWnd()
{   
	m_pparm = nullptr;
	m_piitime = nullptr;
	m_pclass = nullptr;
	m_lFirst = 0;
	m_lLast = 0;
	m_currentclass=-999;
	m_btrackCurve = FALSE;
	m_hitspk=-1;
	m_selectedspike=-1;
	m_rwidth = 6;
	m_rangemode = RANGE_TIMEINTERVALS;
	SetbUseDIB(FALSE);
	m_csEmpty = "no \nspikes";
	m_pSL = nullptr;
}  


CSpikeXYpWnd::~CSpikeXYpWnd()
= default;

////////////////////////////////////////////////////////////////////////////////////
// DISPLAY BARS
//
// PlotDatatoDC(CDC* pDC)
// DisplaySpike(int nospike)
////////////////////////////////////////////////////////////////////////////////////

void CSpikeXYpWnd::PlotDatatoDC(CDC* pDC)
{
	if (m_erasebkgnd)			// erase background
		EraseBkgnd(pDC);

	pDC->SelectObject (GetStockObject (DEFAULT_GUI_FONT));
	auto rect = m_displayRect;
	rect.DeflateRect(1,1);
		
	// test presence of data		
	if (m_pparm == nullptr || m_pparm->GetSize() == 0 )
	{
		const auto textlen = m_csEmpty.GetLength();
		pDC->DrawText(m_csEmpty, textlen, rect, DT_LEFT); //|DT_WORDBREAK);
		return;
	}	
	
	// save context
	const auto n_saved_dc = pDC->SaveDC();	// save device context
	const auto bkcolor = pDC->GetBkColor();

	// display data: trap error conditions
	const auto windowduration =  m_lLast - m_lFirst + 1;	// abcissa size
	GetExtents();								// get origin and extents for xy axes
	ASSERT(m_xWE != 1);
	pDC->SetMapMode(MM_TEXT);
	
	// prepare brush & rectangles (small for all spikes, larger for spikes belonging to the selected class)
	auto selbrush = BLACK_COLOR;
	auto width = m_rwidth/2;
	rect = CRect(-width, -width, width, width);
	width = m_rwidth*2/3;
	const CRect rect1(-width, -width, width, width);
	CRect recti;
	auto ilast=m_pparm->GetUpperBound();
	auto ifirst= 0;
	if (m_rangemode == RANGE_INDEX)
	{
		if (m_spklast > ilast) 
			m_spklast = ilast;
		if (m_spkfirst < 0) 
			m_spkfirst = 0;		
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}

	// loop over all spikes
	for (auto ispk=ilast; ispk>=ifirst; ispk--)
	{
		// check that spike fits within time limits of the display
		const auto l_spike_time = m_piitime->GetAt(ispk);
		if (m_rangemode == RANGE_TIMEINTERVALS
			&& (l_spike_time < m_lFirst || l_spike_time > m_lLast))
			continue;

		// select correct brush
		const auto wspkcla = m_pclass->GetAt(ispk);
		switch (m_plotmode)
		{
		case PLOT_ONECLASSONLY:
			if (wspkcla != m_selclass)
				continue;
			break;
		case PLOT_CLASSCOLORS:
			selbrush = wspkcla % NB_COLORS;
			break;
		case PLOT_ONECLASS:
			if (wspkcla != m_selclass)
				selbrush= SILVER_COLOR;
			else
				selbrush = m_colorselected;
		default:
			break;
		}
		// adjust rectangle size
		if (wspkcla == m_selclass)
			recti = rect1;
		else
			recti = rect;	
			
		// draw point
		const auto x1 = MulDiv(l_spike_time - m_lFirst, m_xVE, windowduration) + m_xVO;
		const auto y1 = MulDiv(m_pparm->GetAt(ispk) -m_yWO, m_yVE, m_yWE) + m_yVO;
		recti.OffsetRect(x1, y1);
		pDC->MoveTo(x1, y1);
		pDC->FillSolidRect(&recti, m_colorTable[selbrush]); 
	}
	pDC->SetBkColor(bkcolor);		// restore background color

	// display spike selected
	if (m_selectedspike >= 0)
		HighlightOnePoint(m_selectedspike, pDC);

	if (m_pSL->GetSpikeFlagArrayCount() > 0)
	{
		// loop over the array of flagged spikes
		for (auto i= m_pSL->GetSpikeFlagArrayCount()-1; i>=0; i--)
		{
			const auto nospike = m_pSL->GetSpikeFlagArrayAt(i);
			HighlightOnePoint(nospike, pDC);
		}
	}

	//display HZ cursors
	if (GetNHZtags() > 0)
	{
		//DisplayHZtags(pDC);
		// select pen and display mode 
		const auto pold = pDC->SelectObject(&m_blackDottedPen);
		const auto nold_rop = pDC->SetROP2(R2_NOTXORPEN);

		// iterate through HZ cursor list	
		const auto x1 = m_displayRect.left;
		const auto x2 = m_displayRect.right;
		for (auto i = GetNHZtags()-1; i>= 0; i--)
		{
			const auto k = GetHZtagVal(i);		// get val
			const auto y1 = MulDiv(k -m_yWO, m_yVE, m_yWE) + m_yVO;
			pDC->MoveTo(x1, y1);		// set initial pt
			pDC->LineTo(x2, y1);		// HZ line

		}
		pDC->SelectObject(pold);
		pDC->SetROP2(nold_rop);			// restore old display mode	
	}

	// display VT cursors
	if (GetNVTtags() > 0)		// display vertical tags
	{
		// select pen and display mode 
		const auto nold_rop = pDC->SetROP2(R2_NOTXORPEN);
		const auto oldp =pDC->SelectObject(&m_blackDottedPen);
		m_xWO= m_lFirst;
		m_xWE = m_lLast - m_lFirst + 1;

		// iterate through VT cursor list
		const auto y0 = m_displayRect.top;
		const auto y1 = m_displayRect.bottom;
		for (auto j=GetNVTtags()-1; j>=0; j--)
		{
			const auto val = GetVTtagVal(j);			// get val
			const auto pix_x = MulDiv(val-m_xWO, m_xVE, m_xWE)+m_xVO;
			pDC->MoveTo(pix_x, y0);			// set initial pt
			pDC->LineTo(pix_x, y1);			// VT line
		}
		pDC->SelectObject(oldp);
		pDC->SetROP2(nold_rop);
	}

	// restore resources
	pDC->RestoreDC(n_saved_dc);
}

//---------------------------------------------------------------------------

BOOL CSpikeXYpWnd::IsSpikeWithinRange(int spikeno)
{
	if (spikeno > m_piitime->GetSize()-1)
		return FALSE;

	if (m_spklast > m_pparm->GetUpperBound()) 
		m_spklast = m_pparm->GetUpperBound();

	if (m_spkfirst < 0) 
		m_spkfirst = 0;
		
	if (m_rangemode == RANGE_TIMEINTERVALS
		&& (m_piitime->GetAt(spikeno) < m_lFirst 
		 || m_piitime->GetAt(spikeno) > m_lLast))
		return FALSE;

	if (m_rangemode == RANGE_INDEX
		&& (spikeno>m_spklast || spikeno < m_spkfirst))
		return FALSE;

	if (m_plotmode == PLOT_ONECLASSONLY 
		&& m_pclass->GetAt(spikeno) != m_selclass)
		return FALSE;

	return TRUE;
}

//---------------------------------------------------------------------------
// display individual spike

void CSpikeXYpWnd::DisplaySpike(int nospike, BOOL bselect)
{
	if (!IsSpikeWithinRange(nospike))
		return;

	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);
	int color;
	if (!bselect)
	{		
		switch (m_plotmode)
		{
		case PLOT_ONECLASSONLY:
		case PLOT_ONECLASS:
			color = BLACK_COLOR;			// Black
			if (m_pclass->GetAt(nospike) != m_selclass)
				color = SILVER_COLOR;		// LTgrey
			break;
		case PLOT_CLASSCOLORS:
			if (nospike == m_selectedspike)
				HighlightOnePoint(nospike, &dc);
			color = m_pclass->GetAt(nospike) % 8;
			break;
		case PLOT_BLACK:
		default:
			color = BLACK_COLOR;	// Black
			break;
		}
	}
	else
	{
		switch (m_plotmode)
		{
		case PLOT_CLASSCOLORS:
			HighlightOnePoint(nospike, &dc);
			color = m_pclass->GetAt(nospike) % 8;
			break;
		case PLOT_BLACK:
		case PLOT_ONECLASSONLY:
		case PLOT_ONECLASS:
		default:
			color = RED_COLOR;		// Red
			break;
		}
	}

	// display spike	
	DrawSelectedSpike(nospike, color, &dc);	
}

void CSpikeXYpWnd::HighlightOnePoint(int nospike, CDC* pDC)
{
	const auto nold_rop = pDC->SetROP2(R2_NOTXORPEN);

	const auto l_spike_time = m_piitime->GetAt(nospike);
	const auto windowduration =  m_lLast - m_lFirst + 1;
	const auto x1 = MulDiv (l_spike_time - m_lFirst, m_xVE, windowduration) + m_xVO;
	const auto y1 = MulDiv(m_pparm->GetAt(nospike) -m_yWO, m_yVE, m_yWE) + m_yVO;
	
	CPen new_pen;
	new_pen.CreatePen(PS_SOLID, 1, RGB(196,   2,  51)); //RGB(255, 255, 255));
	auto* oldpen = (CPen*) pDC->SelectObject(&new_pen);

	// ------------------------------------------------
	const auto width = m_rwidth*2/3+2;
	CRect rect1(-width, -width, width, width);
	rect1.OffsetRect(x1-1, y1-1);
	pDC->MoveTo(rect1.left, rect1.top);
	pDC->LineTo(rect1.right, rect1.top);
	pDC->LineTo(rect1.right, rect1.bottom);
	pDC->LineTo(rect1.left, rect1.bottom);
	pDC->LineTo(rect1.left, rect1.top);

	// restore resources	
	pDC->SelectObject(oldpen);
	pDC->SetROP2(nold_rop);
}

//---------------------------------------------------------------------------

void CSpikeXYpWnd::DrawSelectedSpike(int nospike, int color, CDC* pDC)
{
	// ------------------------------------------------
	const auto l_spike_time = m_piitime->GetAt(nospike);
	const auto windowduration =  m_lLast - m_lFirst + 1;
	const auto x1 = MulDiv(l_spike_time - m_lFirst,  m_xVE, windowduration) + m_xVO;
	const auto y1 = MulDiv(m_pparm->GetAt(nospike) -m_yWO, m_yVE, m_yWE) + m_yVO;
	CRect rect(0, 0, m_rwidth, m_rwidth);
	rect.OffsetRect(x1-m_rwidth/2, y1-m_rwidth/2);

	const auto bkcolor = pDC->GetBkColor();
	pDC->MoveTo(x1, y1);
	pDC->FillSolidRect(&rect, m_colorTable[color]);
	pDC->SetBkColor(bkcolor);
}

//---------------------------------------------------------------------------
// change position of HZ/VT tag
//---------------------------------------------------------------------------
void CSpikeXYpWnd::MoveHZtagtoVal(int i, int val)
{	
	m_ptLast.y = MulDiv(GetHZtagVal(i)- m_yWO, m_yVE, m_yWE) + m_yVO;
	const auto ypix = MulDiv(val- m_yWO, m_yVE, m_yWE) + m_yVO;
	XorHZtag(ypix);
	SetHZtagVal(i, val);
}

void CSpikeXYpWnd::MoveVTtagtoVal(int i, int val)
{	
	m_ptLast.x = MulDiv(GetVTtagVal(i)- m_xWO, m_xVE, m_xWE) + m_xVO;
	const auto xpix = MulDiv(val- m_xWO, m_xVE, m_xWE) + m_xVO;
	XorVTtag(xpix);	
	SetVTtagVal(i, val);
}


//---------------------------------------------------------------------------

int	CSpikeXYpWnd::SelectSpike(int spikeno)
{
	// erase old selected spike (eventually)
	const auto oldselected = m_selectedspike;
	if (m_selectedspike >= 0) // && m_selectedspike != spikeno)
		DisplaySpike(m_selectedspike, FALSE);

	if (spikeno >= 0)
		DisplaySpike(spikeno, TRUE);
	m_selectedspike = spikeno;

	return oldselected;
}

////////////////////////////////////////////////////////////////////////////////////
// MOUSE related events

void CSpikeXYpWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	switch (m_trackMode)
	{
	case TRACK_HZTAG:	
		// horizontal tag was tracked	
		{
		// convert pix into data value
		const auto val = MulDiv(m_ptLast.y-m_yVO, m_yWE, m_yVE)+m_yWO;
		SetHZtagVal(m_HCtrapped,val);
		point.y = MulDiv(val-m_yWO, m_yVE, m_yWE)+m_yVO;
		XorHZtag(point.y);
		CScopeScreen::OnLButtonUp(nFlags, point);
		PostMyMessage(HINT_CHANGEHZTAG, m_HCtrapped);		
		}
		break;

	case TRACK_VTTAG:	
		// vertical tag was tracked	
		{
		// convert pix into data value
		const auto val = MulDiv(m_ptLast.x-m_xVO, m_xWE, m_xVE)+m_xWO;
		SetVTtagVal(m_HCtrapped, val);
		point.x = MulDiv(val-m_xWO, m_xVE, m_xWE)+m_xVO;
		XorVTtag (point.x);
		CScopeScreen::OnLButtonUp(nFlags, point);
		PostMyMessage(HINT_CHANGEVERTTAG, m_HCtrapped);		
		}
		break;

	default:
		{
		// none: zoom data or offset display
		CScopeScreen::OnLButtonUp(nFlags, point);
		CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
		const auto jitter = 3;
		if ((abs(rect_out.Height())< jitter) && (abs(rect_out.Width())< jitter))
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
				rect_out = rectIn;
				rect_out.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
				ZoomData(&rectIn, &rect_out);
				break;
			case CURSOR_ZOOM: 	// zoom operation
				ZoomData(&rectIn, &rect_out);
				m_ZoomFrom = rectIn;
				m_ZoomTo   = rect_out;					
				m_iUndoZoom = 1;
				break;				
			default:
				break;
		}
		}
		break;
	}
}


//---------------------------------------------------------------------------

void CSpikeXYpWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// compute pixel position of tags
	if (GetNHZtags() > 0)
	{
		for (auto icur = GetNHZtags()-1; icur>=0; icur--)
			SetHZtagPix(icur, MulDiv(GetHZtagVal(icur)-m_yWO, m_yVE, m_yWE)+m_yVO);
	}
	if (GetNVTtags() > 0)
	{
		for (auto icur = GetNVTtags()-1; icur>=0; icur--)
		{
			const auto val = GetVTtagVal(icur);
			const auto pix = MulDiv(val-m_xWO, m_xVE, m_xWE)+m_xVO;
			SetVTtagPix (icur, pix);
		}
	}

	// track rectangle or HZtag?
	CScopeScreen::OnLButtonDown(nFlags, point);
	if (m_currCursorMode!=0 || m_HCtrapped >= 0)// do nothing else if mode != 0
	{
		if (m_trackMode == TRACK_HZTAG || m_trackMode == TRACK_VTTAG)
			return;	 								// or any tag hit (VT, HZ) detected	
	}
	
	// test if mouse hit a spike parameter
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
	}	
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

void CSpikeXYpWnd::ZoomData(CRect* rFrom, CRect* rDest)
{
	rFrom->NormalizeRect();	// make sure that rect is not inverted
	rDest->NormalizeRect();
	
	// change y gain & y offset		
	const auto y_we = m_yWE;				// save previous window extent
	m_yWE = MulDiv (m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
			-MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
			+MulDiv(rDest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed	
	auto l_size = m_lLast-m_lFirst+1;
	m_lFirst = m_lFirst + l_size*(rDest->left-rFrom->left)/rFrom->Width();
	l_size = l_size *rDest->Width() / rFrom->Width();
	m_lLast = m_lFirst +l_size -1;
	// display
	Invalidate();
	PostMyMessage(HINT_CHANGEHZLIMITS, NULL);
}


//---------------------------------------------------------------------------
void CSpikeXYpWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_selectedspike < 0 || m_hitspk < 0)
		CScopeScreen::OnLButtonDblClk(nFlags, point);
	else
		GetParent()->PostMessage(WM_COMMAND,MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),(LPARAM) m_hWnd);
}

// -------------------------------------------------------------

int CSpikeXYpWnd::DoesCursorHitCurve(CPoint point)
{
	const auto hitspk = -1;	// no spike hitted
	// convert device coordinates into logical coordinates
	
	// abcissa
	const auto taille =  (m_lLast - m_lFirst + 1);
	const auto l_xmax = m_lFirst + taille*(point.x + m_rwidth)/static_cast<long>(m_xVE);
	const auto l_xmin = m_lFirst + taille*(point.x - m_rwidth)/static_cast<long>(m_xVE);
	
	// ordinates
	const auto mouse_ymx = MulDiv(point.y +m_rwidth -m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto mouse_ymi = MulDiv(point.y -m_rwidth -m_yVO, m_yWE, m_yVE) + m_yWO;

	// loop through all spikes in two passes
	// first look black spikes (foreground), then grey (background)
	// plot selected class
	int ispk;
	if (m_plotmode == PLOT_ONECLASS)
	{
		for (ispk=m_piitime->GetUpperBound(); ispk>=0; ispk--)
		{
			// skip non selected class
			if (m_pclass->GetAt(ispk) != m_selclass)
				continue;
			// skip spike ?
			const auto l_spike_time = m_piitime->GetAt(ispk);
			if (l_spike_time < l_xmin || l_spike_time > l_xmax)
				continue;
			// test spike hit
			const auto val = m_pparm->GetAt(ispk);
			if (mouse_ymx < val && mouse_ymi > val)
				return ispk;
		}
	}

	// then look through all other spikes
	for (ispk=m_piitime->GetUpperBound(); ispk>=0; ispk--)
	{
		const auto l_spike_time = m_piitime->GetAt(ispk);
		if (l_spike_time < l_xmin || l_spike_time > l_xmax)
			continue;
		
		const auto val = m_pparm->GetAt(ispk);
		if (mouse_ymx < val && mouse_ymi > val)
			return ispk;
	}	
	return hitspk;
}
// ------------------------------------
void CSpikeXYpWnd::GetExtents()
{
	if (m_yWE == 1) // && m_yWO == 0)	
	{
		auto maxval=4096;
		auto minval=0;
		if (m_pparm->GetSize() > 0)
		{
			maxval = m_pparm->GetAt(0);
			minval = maxval;
			for (int i = m_pparm->GetUpperBound()-1; i>= 0; i--)
			{
				const auto val = m_pparm->GetAt(i);
				if (val > maxval) maxval = val;
				if (val < minval) minval = val;
			}
		}		
		m_yWE = maxval - minval +2;  
		m_yWO = (maxval + minval)/2;
	}

	if (m_xVE == 1 && m_xVO == 0)
	{
		m_xVE = m_displayRect.Width();
		m_xVO = m_displayRect.left;		
	}
	if (m_xWE == 1) // && m_xWO == 0)
	{
		m_xWE = m_displayRect.Width();
		m_xWO = m_displayRect.left;
	}
}


void CSpikeXYpWnd::SetSourceData(CArray<int, int>* pparms, CArray<long, long>* piitime, CArray<int, int>* pclass, CSpikeList* p_spk_list)
{
	m_pparm = pparms; 
	m_piitime = piitime; 
	m_pclass = pclass;
	m_selectedspike=-1;
	m_pSL = p_spk_list;
}
	
