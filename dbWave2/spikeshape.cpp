// SpikeShapeWnd.cpp Implementation File

#include "StdAfx.h"
#include "scopescr.h"
#include "Spikedoc.h"
#include "dbWaveDoc.h"
#include "spikeshape.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO loop through files when m_ballfiles is true: spike hit

IMPLEMENT_SERIAL (CSpikeShapeWnd, CScopeScreen, 1)

BEGIN_MESSAGE_MAP(CSpikeShapeWnd, CScopeScreen)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

CSpikeShapeWnd::CSpikeShapeWnd()
{                                        
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
	p_doc_ = nullptr;
	p_spikelist_ = nullptr;
}

CSpikeShapeWnd::~CSpikeShapeWnd()
{
}

void CSpikeShapeWnd::PlotDatatoDC(CDC* p_dc)
{
	if (m_erasebkgnd)
		EraseBkgnd(p_dc);
	
	// display data: trap error conditions
	const auto n_saved_dc = p_dc->SaveDC();
	GetExtents();
	PrepareDC(p_dc);
	auto ncurrentfile = 0;
	auto file_first = ncurrentfile;
	auto file_last = ncurrentfile;
	if (m_ballFiles)
	{
		file_first = 0;
		file_last = p_doc_->DBGetNRecords() -1;
		ncurrentfile = p_doc_->DBGetCurrentRecordPosition();
	}

	for (auto ifile = file_first; ifile <= file_last; ifile++)
	{
		if (m_ballFiles)
		{
			p_doc_->DBSetCurrentRecordPosition(ifile);
			p_doc_->OpenCurrentSpikeFile();
			p_spikelist_ = p_doc_->m_pSpk->GetSpkListCurrent();
		}

		//test if data are there - if none, display message and exit
		if (p_spikelist_ == nullptr || p_spikelist_->GetTotalSpikes() == 0)
		{
			if (!m_ballFiles)
			{
				p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
				auto rect = m_displayRect;
				rect.DeflateRect(1, 1);
				const auto textlen = m_csEmpty.GetLength();
				p_dc->DrawText(m_csEmpty, textlen, rect, DT_LEFT); //|DT_WORDBREAK);
				return;
			}
			continue;
		}
		
		// load resources and prepare context	
		const auto taillespk = p_spikelist_->GetSpikeLength();
		ASSERT(taillespk >0);
		if (polypoints_.GetSize() != taillespk)
		{
			polypoints_.SetSize(taillespk, 2);
			InitPolypointAbcissa();
		}

		// loop through all spikes of the list
		short* lpspk;
		auto ilast = p_spikelist_->GetTotalSpikes() - 1;
		auto ifirst = 0;
		if (m_rangemode == RANGE_INDEX)
		{
			ilast = m_spklast;
			ifirst = m_spkfirst;
		}
		auto selpen = BLACK_COLOR;
		if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECOLOR)
			selpen = SILVER_COLOR;
		const auto pold_pen = p_dc->SelectObject(&m_penTable[selpen]);

		for (auto ispk = ilast; ispk >= ifirst; ispk--)
		{
			// skip spike ?
			if (m_rangemode == RANGE_TIMEINTERVALS
				&& (p_spikelist_->GetSpikeTime(ispk) < m_lFirst
					|| p_spikelist_->GetSpikeTime(ispk) > m_lLast))
				continue;

			// select pen according to class
			const auto wspkcla = p_spikelist_->GetSpikeClass(ispk);
			switch (m_plotmode)
			{
			case PLOT_ONECLASSONLY:
				if (wspkcla != m_selclass)
					continue;
				break;
			case PLOT_CLASSCOLORS:
				selpen = wspkcla % NB_COLORS;
				p_dc->SelectObject(&m_penTable[selpen]);
				break;
			case PLOT_ONECLASS:
				if (wspkcla == m_selclass)
					continue;
			default:
				break;
			}

			// display data
			lpspk = p_spikelist_->GetpSpikeData(ispk);
			FillPolypointOrdinates(lpspk);
			p_dc->Polyline(&polypoints_[0], taillespk);
		}

		if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECOLOR)
		{
			selpen = m_colorselected;
			if (m_plotmode == PLOT_ONECOLOR)
				selpen = m_selclass % NB_COLORS;
			p_dc->SelectObject(&m_penTable[selpen]);
			for (auto ispk = ilast; ispk >= ifirst; ispk--)
			{
				// skip spike ?
				if (m_rangemode == RANGE_TIMEINTERVALS
					&& (p_spikelist_->GetSpikeTime(ispk) < m_lFirst
					|| p_spikelist_->GetSpikeTime(ispk) > m_lLast))
					continue;

				// skip spikes with the wrong class
				if (p_spikelist_->GetSpikeClass(ispk) != m_selclass)
					continue;
				// display data
				lpspk = p_spikelist_->GetpSpikeData(ispk);
				FillPolypointOrdinates(lpspk);
				p_dc->Polyline(&polypoints_[0], taillespk);
			}
		}

		// display selected spike
		auto iselect = -1;
		if (m_selectedspike >= 0 && (IsSpikeWithinRange(m_selectedspike)))
			iselect = m_selectedspike;
		DrawSelectedSpike(iselect, p_dc);

		if (p_spikelist_->GetSpikeFlagArrayCount() > 0)
			DrawFlaggedSpikes(p_dc);

		// display tags
		if (GetNHZtags() > 0)
			DisplayHZtags(p_dc);

		if (GetNVTtags() > 0)
			DisplayVTtags(p_dc);

		// display text
		if (m_bText && m_plotmode == PLOT_ONECLASSONLY)
		{
			TCHAR num[10];
			wsprintf(num, _T("%i"), GetSelClass());
			p_dc->TextOut(1, 1, num);
		}

		// restore resource
		p_dc->SelectObject(pold_pen);
	}

	// restore resources		
	p_dc->RestoreDC(n_saved_dc);

	if (m_ballFiles)
	{
		p_doc_->DBSetCurrentRecordPosition(ncurrentfile);
		p_doc_->OpenCurrentSpikeFile();
		p_spikelist_ = p_doc_->m_pSpk->GetSpkListCurrent();
	}
}

void CSpikeShapeWnd::DrawSelectedSpike(int nospike, CDC* p_dc)
{
	const auto n_saved_dc = p_dc->SaveDC();
	auto rect = m_displayRect;
	p_dc->DPtoLP(rect);
	p_dc->IntersectClipRect(&rect);
	
	if (nospike >= 0)
	{
		// change coordinate settings
		GetExtents();
		PrepareDC(p_dc);
		
		p_dc->SetViewportOrg (m_displayRect.left, m_displayRect.Height()/2 + m_displayRect.top);
		p_dc->SetViewportExt (m_displayRect.Width(), -m_displayRect.Height());
		
		// prepare pen and select pen
		const auto pensize = 2;
		CPen new_pen(PS_SOLID, pensize, m_colorTable[m_colorselectedspike]);
		auto* poldpen = (CPen*) p_dc->SelectObject(&new_pen);

		// display data
		auto* lpspk = p_spikelist_->GetpSpikeData(nospike);
		FillPolypointOrdinates(lpspk);
		p_dc->Polyline(&polypoints_[0], p_spikelist_->GetSpikeLength());

		// restore resources
		p_dc->SelectObject(poldpen);
	}
	// restore ressources
	p_dc->RestoreDC(n_saved_dc);	
}

void CSpikeShapeWnd::DrawFlaggedSpikes(CDC* pDC0)
{
	ASSERT (pDC0 != NULL);
	auto p_dc = pDC0;
	const auto n_saved_dc = p_dc->SaveDC();	

	// change coordinate settings 
	GetExtents();
	PrepareDC(p_dc);
	p_dc->SetViewportOrg (m_displayRect.left, m_displayRect.Height()/2);
	p_dc->SetViewportExt (m_displayRect.right, -m_displayRect.Height());

	// prepare pen and select pen
	const auto pensize = 1;
	CPen new_pen(PS_SOLID, pensize, m_colorTable[m_colorselectedspike]);
	const auto oldpen = (CPen*) p_dc->SelectObject(&new_pen);

	// loop through all flagged spikes
	for (auto i= p_spikelist_->GetSpikeFlagArrayCount()-1; i>=0; i--)
	{
		const auto nospike = p_spikelist_->GetSpikeFlagArrayAt(i);
		// skip spike if not valid in this display
		if (!IsSpikeWithinRange(nospike))
			continue;
		//if (PLOT_ONECLASSONLY == m_plotmode && nospikeclass != m_selclass)
		//	continue;
		FillPolypointOrdinates(p_spikelist_->GetpSpikeData(nospike));
		p_dc->Polyline(&polypoints_[0], p_spikelist_->GetSpikeLength());
	}

	// restore resources
	p_dc->SelectObject(oldpen);
	pDC0->RestoreDC(n_saved_dc);
}

void CSpikeShapeWnd::DisplayFlaggedSpikes (BOOL bHighLight)
{
	if (bHighLight)
		DrawFlaggedSpikes(&m_PlotDC); 
	Invalidate();
}

int	CSpikeShapeWnd::DisplayExData(short* p_data, int color)
{
	// prepare array
	const auto nelements = p_spikelist_->GetSpikeLength();
	if (polypoints_.GetSize() != nelements)
	{
		polypoints_.SetSize(nelements, 2);
		InitPolypointAbcissa();	
	}

	CClientDC dc(this);
	dc.IntersectClipRect(&m_clientRect);
	PrepareDC(&dc);
	CPen new_pen(PS_SOLID, 0, m_colorTable[color]);
	const auto oldpen = (CPen*) dc.SelectObject(&new_pen);
	FillPolypointOrdinates(p_data);
	dc.Polyline(&polypoints_[0], p_spikelist_->GetSpikeLength());

	dc.SelectObject(oldpen);
	return color;
}

BOOL CSpikeShapeWnd::IsSpikeWithinRange(int spikeno) const
{	
	if (spikeno > p_spikelist_->GetTotalSpikes()-1)
		return FALSE;
	if (m_rangemode == RANGE_TIMEINTERVALS
		&& (p_spikelist_->GetSpikeTime(spikeno) < m_lFirst || p_spikelist_->GetSpikeTime(spikeno) > m_lLast))
		return FALSE;
	else if (m_rangemode == RANGE_INDEX
		&& (spikeno>m_spklast || spikeno < m_spkfirst))
		return FALSE;
	if (m_plotmode == PLOT_ONECLASSONLY 
		&& (p_spikelist_->GetSpikeClass(spikeno) != m_selclass))
		return FALSE;
	return TRUE;
}

int	CSpikeShapeWnd::SelectSpikeShape(int spikeno)
{
	// erase plane
	const auto oldselected = m_selectedspike;
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
		const auto val = MulDiv(point.x-m_xVO, m_xWE, m_xVE)+m_xWO;
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
			auto rect_in= m_displayRect;
		switch (m_cursorType)
		{
		case 0:
			rect_out = rect_in;
			rect_out.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
			PostMyMessage(HINT_DEFINEDRECT, NULL);	// tell parent that val changed
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
		break;
	}
}

void CSpikeShapeWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLmouseDown = TRUE;
	// call base class to test for horiz cursor or XORing rectangle	
	if (GetNVTtags() > 0)
	{
		for (auto icur = GetNVTtags()-1; icur>=0; icur--)	// loop through all tags
			SetVTtagPix(icur, MulDiv(GetVTtagVal(icur)-m_xWO, m_xVE, m_xWE)+m_xVO);
	}

	// track rectangle or VTtag?
	CScopeScreen::OnLButtonDown(nFlags, point);		// capture cursor eventually
	if (m_currCursorMode!=0 || m_HCtrapped >= 0)// do nothing else if mode != 0
		return;	 								// or any tag hit (VT, HZ) detected
	
	// test if mouse hit one spike
	// if hit, then tell parent to select corresp spike
	m_hitspk = DoesCursorHitCurveInDoc(point);
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
	const auto y_we = m_yWE;				// save previous window extent
	m_yWE = MulDiv (m_yWE, rDest->Height(), rFrom->Height());
	m_yWO = m_yWO
			-MulDiv(rFrom->top - m_yVO, m_yWE, m_yVE)
			+MulDiv(rDest->top - m_yVO, y_we, m_yVE);

	// change index of first and last pt displayed	
	const auto x_we = m_xWE;				// save previous window extent
	m_xWE = MulDiv (m_xWE, rDest->Width(), rFrom->Width());
	m_xWO = m_xWO
			-MulDiv(rFrom->left - m_xVO, m_xWE, m_xVE)
			+MulDiv(rDest->left - m_xVO, x_we, m_xVE);

	// display
	Invalidate();
	PostMyMessage(HINT_CHANGEZOOM, 0);
}

void CSpikeShapeWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_selectedspike < 0 && p_spikelist_->GetSpikeFlagArrayCount () < 1)|| m_hitspk < 0)
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

int CSpikeShapeWnd::DoesCursorHitCurveInDoc(CPoint point)
{
	long nfiles = 1;
	long ncurrentfile = 0;
	if (m_ballFiles)
	{
		nfiles = p_doc_->DBGetNRecords();
		ncurrentfile = p_doc_->DBGetCurrentRecordPosition();
	}

	int result = -1;
	for (long ifile = 0; ifile < nfiles; ifile++)
	{
		if (m_ballFiles)
		{
			p_doc_->DBSetCurrentRecordPosition(ifile);
			p_doc_->OpenCurrentSpikeFile();
			p_spikelist_ = p_doc_->m_pSpk->GetSpkListCurrent();
		}

		if (p_spikelist_ == nullptr || p_spikelist_->GetTotalSpikes() == 0)
		{
			continue;
		}
		result = DoesCursorHitCurve(point);
		if (result >= 0)
			break;
	}

	if (m_ballFiles && result < 0)
	{
		p_doc_->DBSetCurrentRecordPosition(ncurrentfile);
		p_doc_->OpenCurrentSpikeFile();
		p_spikelist_ = p_doc_->m_pSpk->GetSpkListCurrent();
	}

	return result;
}

int  CSpikeShapeWnd::DoesCursorHitCurve(const CPoint point) const
{
	auto hitspk = -1;
	// convert device coordinates into logical coordinates
	const auto mouse_x = MulDiv(point.x-m_xVO, m_xWE, m_xVE) + m_xWO;
	if (mouse_x <0 || mouse_x > p_spikelist_->GetSpikeLength())
		return hitspk;
	const auto mouse_y = MulDiv(point.y-m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto deltay = MulDiv(3, m_yWE, m_yVE);

	// loop through all spikes
	auto ilast = p_spikelist_->GetTotalSpikes()-1;
	auto ifirst = 0;
	if (m_rangemode == RANGE_INDEX)	
	{
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}
	for (auto ispk=ilast; ispk>=ifirst; ispk--)
	{
		if (m_rangemode == RANGE_TIMEINTERVALS
			&& (p_spikelist_->GetSpikeTime(ispk) < m_lFirst 
			   || p_spikelist_->GetSpikeTime(ispk) > m_lLast))
			continue;
		if (m_plotmode == PLOT_ONECLASSONLY
		 && p_spikelist_->GetSpikeClass(ispk) != m_selclass)
			continue;
		
		const auto val = p_spikelist_->GetSpikeValAt(ispk, mouse_x);
		if (mouse_y+deltay < val && mouse_y-deltay > val)
		{			
			hitspk = ispk;
			break;
		}
	}	
	return hitspk;
}

void CSpikeShapeWnd::GetExtents()
{
	if (!m_ballFiles)
	{
		GetExtentsCurrentSpkList();
		return;
	}

	const auto ncurrentfile = p_doc_->DBGetCurrentRecordPosition();
	const auto file_first = 0;
	const auto file_last = p_doc_->DBGetNRecords();

	if (m_yWE == 1 || m_yWE == 0) // && m_yWO == 0)
	 {
		for (auto ifile = file_first; ifile <= file_last; ifile++)
		{
			p_doc_->DBSetCurrentRecordPosition(ifile);
			p_doc_->OpenCurrentSpikeFile();
			p_spikelist_ = p_doc_->m_pSpk->GetSpkListCurrent();

			GetExtentsCurrentSpkList();
			if (m_yWE != 0)
				break;
		}
	}

	// exit
	p_doc_->DBSetCurrentRecordPosition(ncurrentfile);
	p_doc_->OpenCurrentSpikeFile();
	p_spikelist_ = p_doc_->m_pSpk->GetSpkListCurrent();
}

void CSpikeShapeWnd::GetExtentsCurrentSpkList()
{
	if (m_yWE == 1 || m_yWE == 0)
	{
		int maxval, minval;
		p_spikelist_->GetTotalMaxMin(TRUE, &maxval, &minval);
		m_yWE = MulDiv((maxval - minval), 10, 9) + 1;
		m_yWO = maxval / 2 + minval / 2;
	}

	if (m_xWE == 1)
	{
		m_xWE = p_spikelist_->GetSpikeLength();
		m_xWO = 0;
	}
}

void CSpikeShapeWnd::InitPolypointAbcissa()
{
	const auto nelements = polypoints_.GetSize();
	m_xWE = nelements +1;
	ASSERT(nelements > 0);

	for (auto i = 0; i<nelements; i++)
		polypoints_[i].x = i+1;
}

void CSpikeShapeWnd::FillPolypointOrdinates(short* lpSource)
{
	auto nelements = polypoints_.GetSize();
	ASSERT(nelements > 0);
	if (nelements == 0)
	{
		nelements = p_spikelist_->GetSpikeLength();
		ASSERT(nelements > 0);
		polypoints_.SetSize(nelements, 2);
		InitPolypointAbcissa();	
	}

	for (auto i = 0; i<nelements; i++, lpSource++)
		polypoints_[i].y  = *lpSource;
}

void CSpikeShapeWnd::Print(CDC* p_dc, CRect* rect)
{
	// check if there are valid data to display
	if (p_spikelist_ == nullptr || p_spikelist_->GetTotalSpikes()== 0)
		return;

	const auto old_y_vo = m_yVO;
	const auto old_y_ve = m_yVE;
	const auto old_xextent = m_xWE;
	const auto old_xorg = m_xWO;
	
	// size of the window
	m_yVO = rect->Height()/2 + rect->top;
	m_yVE = -rect->Height();

	// check initial conditions
	if (m_yWE == 1) // && m_yWO == 0)
	{
		int maxval, minval;
		p_spikelist_->GetTotalMaxMin(TRUE, &maxval, &minval);
		m_yWE = maxval - minval +1; 
		m_yWO = (maxval + minval)/2;	
	}

	m_xWO = rect->left;
	m_xWE = rect->Width()-2;

	const auto taillespk = p_spikelist_->GetSpikeLength();
	if (polypoints_.GetSize() != taillespk)
		polypoints_.SetSize(taillespk, 2);

	// set mapping mode and viewport
	const auto n_saved_dc = p_dc->SaveDC();
	for (auto i=0; i<taillespk; i++)
		polypoints_[i].x = rect->left + MulDiv(i, rect->Width(), taillespk);

 	int selpen;	
	switch (m_plotmode)
	{
	//case PLOT_BLACK:			selpen = BLACK_COLOR; break;
	//case PLOT_ONECLASSONLY:	selpen = BLACK_COLOR; break;
	case PLOT_ONECLASS:		
		selpen = m_colorbackgr;
		break;
	case PLOT_ALLGREY:		
		selpen = m_colorbackgr;	
		break;
	default:				
		selpen = BLACK_COLOR; 
		break;
	}

	const auto old_pen = p_dc->SelectObject(&m_penTable[selpen]);
	auto ilast=p_spikelist_->GetTotalSpikes()-1;
	auto ifirst= 0;
	if (m_rangemode == RANGE_INDEX)
	{
		ilast = m_spklast;
		ifirst = m_spkfirst;
	}

	for (auto ispk=ilast; ispk >= ifirst; ispk--)
	{
		if (m_rangemode == RANGE_INDEX && (ispk > m_spklast || ispk < m_spkfirst))
			continue;
		if (m_rangemode == RANGE_TIMEINTERVALS)
		{
			if (p_spikelist_->GetSpikeTime(ispk) < m_lFirst)
				continue;
			if (p_spikelist_->GetSpikeTime(ispk) > m_lLast)
				continue;
		}

		const auto spkcla = p_spikelist_->GetSpikeClass(ispk);
		if (m_plotmode == PLOT_ONECLASSONLY && spkcla != m_selclass)
			continue;
		if (m_plotmode == PLOT_ONECLASS && spkcla == m_selclass)
			continue;

		PlotArraytoDC(p_dc, p_spikelist_->GetpSpikeData(ispk));
	}

	// display selected class if requested by option
	if (m_plotmode == PLOT_ONECLASS)
	{
		p_dc->SelectObject(&m_penTable[m_colorselected]);
		for (auto ispk=ilast; ispk >= ifirst; ispk--)
		{
			if (m_rangemode == RANGE_TIMEINTERVALS)
			{
				const auto ltime = p_spikelist_->GetSpikeTime(ispk);
				if (ltime < m_lFirst || ltime > m_lLast)
					continue;
			}
			if ( p_spikelist_->GetSpikeClass(ispk) != m_selclass)
				continue;
			PlotArraytoDC(p_dc, p_spikelist_->GetpSpikeData(ispk));
		}
	}	

	// display selected spike	
	if (m_selectedspike >= 0 && IsSpikeWithinRange(m_selectedspike))
	{
		CPen new_pen(PS_SOLID, 0, m_colorTable[m_colorselectedspike]);
		p_dc->SelectObject(&new_pen);
		PlotArraytoDC(p_dc, p_spikelist_->GetpSpikeData(m_selectedspike));
	}

	// restore resources
	p_dc->SelectObject(old_pen);	// restore pen
	p_dc->RestoreDC(n_saved_dc);	// restore DC

	m_xWE = old_xextent;		// restore old X extent
	m_xWO = old_xorg;			// restore old X origin
	m_yVO = old_y_vo;			// same with Y
	m_yVE = old_y_ve;
}

void CSpikeShapeWnd::PlotArraytoDC(CDC* p_dc, short* pspk)
{
	const auto nelements = polypoints_.GetSize();
	for (auto i = 0; i<nelements; i++, pspk++)
	{
		auto y = *pspk;	
		y = MulDiv(y-m_yWO, m_yVE, m_yWE) + m_yVO;
		polypoints_[i].y = y;
	}

	if ( p_dc->m_hAttribDC == nullptr 
		|| (p_dc->GetDeviceCaps(LINECAPS) & LC_POLYLINE))
		p_dc->Polyline(&polypoints_[0], nelements);
	else									
	{
		p_dc->MoveTo(polypoints_[0]);		
		for (auto i=0; i<nelements; i++)
			p_dc->LineTo(polypoints_[i]);
	}
}

float CSpikeShapeWnd::GetDisplayMaxMv() 
{
	GetExtents();
	return (p_spikelist_->GetAcqVoltsperBin()*1000.f*(m_yWE-m_yWO-p_spikelist_->GetAcqBinzero()));
}

float CSpikeShapeWnd::GetDisplayMinMv()
{
	if (p_spikelist_ == nullptr)
		return 1.f;
	GetExtents();
	return (p_spikelist_->GetAcqVoltsperBin()*1000.f*(m_yWO-m_yWE-p_spikelist_->GetAcqBinzero()));
}

float CSpikeShapeWnd::GetExtent_mV()
{
	if (p_spikelist_ == nullptr)
		return 1.f;
	GetExtents();
	return (p_spikelist_->GetAcqVoltsperBin()*m_yWE*1000.f);
}

float CSpikeShapeWnd::GetExtent_ms()
{
	if (p_spikelist_ == nullptr)
		return 1.f;
	GetExtents();
	return (static_cast<float>(1000.0 * m_xWE)/p_spikelist_->GetAcqSampRate());
}

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
	if (ar.IsStoring())
	{
		CScopeScreen::Serialize(ar);
		polypoints_.Serialize(ar);

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
		CScopeScreen::Serialize(ar);
		polypoints_.Serialize(ar);

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