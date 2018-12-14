///////////////////////////////////////////////////////////////////////////
// spikehistp.cpp : implementation file
//
// Purpose:
// 		contains and display histograms
// 		histogram bins are stored into an array of "long"
// 		histograms are computed from various sources of data:
//			Acqdata document
//			Spike data
// structure:
// 		CWnd derived : MFC control.
// 		CScopeScreen derived : FMP enriched control (mouse cursors, HZTags)
// 		evolved from CLineviewButton
//
///////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "scopescr.h"
#include "Spikedoc.h"
#include "spikehistp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSpikeHistWnd, CScopeScreen)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
END_MESSAGE_MAP()


//**********************************************************************************
// CSpikeHistWnd() init variables on creation
//**********************************************************************************

CSpikeHistWnd::CSpikeHistWnd() 
{
	m_pSL = nullptr;				// source spk list
	m_lFirst = 0;				// source data intervals
	m_lLast = 0;				// last interval
	m_selclass = 0;				// selected class
	m_hitspk=-1;				// hit spike?
	SetbUseDIB(FALSE);

	// set bin size initial conditions
	m_nbins = 0;				// with 0 bins
	m_abcissaminval=0;			// first abcissa
	m_abcissamaxval=0;			// last abbcissa
	m_lmax=0;					// histogram max
	m_csEmpty = "no \nspikes";
}

//**********************************************************************************
// ~CSpikeHistWnd()
//**********************************************************************************

CSpikeHistWnd::~CSpikeHistWnd()
{
	RemoveHistData();
}

void CSpikeHistWnd::RemoveHistData()
{
	if (histogram_ptr_array.GetSize() >0)	// delete objects pointed by elements
	{							// of m_pHistarray
		for (int i=	histogram_ptr_array.GetUpperBound(); i>= 0; i--)
			delete histogram_ptr_array[i];
		histogram_ptr_array.RemoveAll();		
	}
}

////////////////////////////////////////////////////////////////////////////////////
// DISPLAY HISTOGRAMS
//
// PlotDatatoDC(CDC* pDC)
// DisplaySpike(int nospike)
////////////////////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------------------------------

void CSpikeHistWnd::PlotDatatoDC(CDC* pDC)
{
	if (m_displayRect.right <= 0 && m_displayRect.bottom <= 0)
	{
		CRect r;
		GetWindowRect(&r);
		OnSize(SIZE_RESTORED, r.Width(), r.Height());
	}
	if (m_erasebkgnd)		// erase background
		EraseBkgnd(pDC);

	// load resources
	GetExtents();
	if (m_lmax == 0)
	{
		pDC->SelectObject (GetStockObject (DEFAULT_GUI_FONT));
		CRect rect = m_displayRect;
		rect.DeflateRect(1,1);
		int textlen = m_csEmpty.GetLength();
		pDC->DrawText(m_csEmpty, textlen, rect, DT_LEFT); //|DT_WORDBREAK);
		return;
	}
	int nSavedDC = pDC->SaveDC();	
	PrepareDC(pDC);
	int color;
	// save background color which is changed by later calls to FillSolidRect
	// when doing so, pens created with PS_DOT pattern and with XOR_PEN do
	// not work properly. Restoring the background color solves the pb.
	COLORREF bkcolor = pDC->GetBkColor();
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
	CRect RectHistog;
	
	int i;
	for (int ihist=0; ihist<histogram_ptr_array.GetSize(); ihist++)
	{
		CDWordArray* pDW = histogram_ptr_array.GetAt(ihist);
		if (0 == pDW->GetSize())
			continue;

		// select correct color
		if (ihist > 0)
		{
			int spkcla = (int) pDW->GetAt(0);
			color = BLACK_COLOR; 
			if (PLOT_ONECLASSONLY == m_plotmode && spkcla != m_selclass)
				continue;
			else if (PLOT_CLASSCOLORS == m_plotmode)
				color = spkcla % NB_COLORS;
			else if (m_plotmode == PLOT_ONECLASS && spkcla == m_selclass)
			{
				color = BLACK_COLOR ;
				continue;
			}
		}
		RectHistog.left = m_abcissaminval-m_binsize;
		RectHistog.right = m_abcissaminval;
		RectHistog.bottom = 0;		
		for (i=1; i<pDW->GetSize(); i++)
		{
			RectHistog.left+= m_binsize;
			RectHistog.right+= m_binsize;
			RectHistog.top = (int) (pDW->GetAt(i));
			if (RectHistog.top > 0)
			{
				pDC->MoveTo(RectHistog.left, RectHistog.bottom);
				pDC->FillSolidRect(RectHistog, m_colorTable[color]);
			}
		}		
	}

	// plot selected class (one histogram)
	if (m_plotmode == PLOT_ONECLASS)
	{
		color = BLACK_COLOR; 
		CDWordArray* pDW = nullptr;
		GetClassArray(m_selclass, pDW);
		if (pDW  != nullptr)
		{
			RectHistog.left = m_abcissaminval- m_binsize;
			RectHistog.right = m_abcissaminval;
			RectHistog.bottom = 0;
			for (i=1; i<pDW->GetSize(); i++)
			{
				RectHistog.left+= m_binsize;
				RectHistog.right+= m_binsize;
				RectHistog.top = (int) (pDW->GetAt(i) /* /scale */);
				if (RectHistog.top > 0)
				{
					pDC->MoveTo(RectHistog.left, RectHistog.bottom);
					pDC->FillSolidRect(RectHistog, m_colorTable[color]);
				}
			}
		}
	}

	// display cursors
	pDC->SetBkColor(bkcolor);	// restore background color
	if (GetNHZtags() > 0)		// display horizontal tags
		DisplayHZtags(pDC);
	if (GetNVTtags() > 0)		// display vertical tags
		DisplayVTtags(pDC);	
	pDC->RestoreDC(nSavedDC);
}

// ------------------------------------------------------------------------------------
void CSpikeHistWnd::MoveHZtagtoVal(int i, int val)
{
	m_ptLast.y = MulDiv(GetHZtagVal(i) - m_yWO, m_yVE, m_yWE) + m_yVO;
	int j = MulDiv(val - m_yWO, m_yVE, m_yWE) + m_yVO;
	XorHZtag(j);
	SetHZtagVal(i, val);
}

void CSpikeHistWnd::MoveVTtagtoVal(int itag, int ival)
{
	m_ptLast.x = MulDiv(GetVTtagVal(itag) - m_xWO, m_xVE, m_xWE) + m_xVO;
	int j = MulDiv(ival - m_xWO, m_xVE, m_xWE) + m_xVO;
	XorVTtag(j);
	SetVTtagVal(itag, ival);
}

// check if any histogram array has been created for class "iclass"
// (actually, this parameter is stored in the first item of the array)
void CSpikeHistWnd::GetClassArray(int iclass, CDWordArray*& pDW)
{
	// test if pDW at 0 position is the right one
	if ((nullptr != pDW) && ((int) pDW->GetAt(0) == iclass))
		return;

	// not found, scan the array
	pDW = nullptr;
	for (int i=1; i<histogram_ptr_array.GetSize(); i++)
	{
		if( (int) (histogram_ptr_array[i])->GetAt(0) == iclass)
		{
			pDW = histogram_ptr_array[i];
			break;
		}
	}
}

//**********************************************************************************
// export content of the histogram to the clipboard
//**********************************************************************************

LPTSTR CSpikeHistWnd::ExportAscii(LPTSTR lp)
{
	// print all ordinates line-by-line, differnt classes on same line
	lp += wsprintf(lp, _T("Histogram\nnbins=%i\nnclasses=%i"), m_nbins, histogram_ptr_array.GetSize());
	lp += wsprintf(lp, _T("\nmax=%i\nmin=%i"), m_abcissamaxval, m_abcissaminval);
	// export classes & points
	lp += wsprintf(lp, _T("classes;\n"));
	int i, j;
	for (i=0; i<histogram_ptr_array.GetSize(); i++)
		lp += wsprintf(lp, _T("%i\t"), (int) (histogram_ptr_array[i])->GetAt(0));
	lp--;	// erase \t and replace with \n	
		
	// loop through all points	
	lp += wsprintf(lp, _T("\nvalues;\n"));
	for (j=1; j<=m_nbins; j++)
	{
		for (i=0; i<histogram_ptr_array.GetSize(); i++)
			lp += wsprintf(lp, _T("%i\t"), (int) (histogram_ptr_array[i])->GetAt(j));
		lp--;	// erase \t and replace with \n
		lp += wsprintf(lp, _T("\n"));
	}
	*lp = _T('\0');
	return lp;
}

////////////////////////////////////////////////////////////////////////////////////
// MOUSE related events

void CSpikeHistWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// test if horizontal tag was tracked
	switch (m_trackMode)
	{
	case TRACK_HZTAG:
		{
		// convert pix into data value
		int val = MulDiv(m_ptLast.y-m_yVO, m_yWE, m_yVE)+m_yWO;
		SetHZtagVal(m_HCtrapped,val);				// change cursor value		
		point.y = MulDiv(val-m_yWO, m_yVE, m_yWE)+m_yVO;
		XorHZtag(point.y);
		CScopeScreen::OnLButtonUp(nFlags, point);
		PostMyMessage(HINT_CHANGEHZTAG, m_HCtrapped);		
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

	case TRACK_RECT:
		{
		CScopeScreen::OnLButtonUp(nFlags, point);  // else release mouse

		// none: zoom data or offset display
		CScopeScreen::OnLButtonUp(nFlags, point);
		CRect rectOut(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
		const int jitter = 3;
		if ((abs(rectOut.Height())< jitter) && (abs(rectOut.Width())< jitter))
		{
			if (m_cursorType != CURSOR_ZOOM)
				PostMyMessage(HINT_HITAREA, NULL);
			else
				ZoomIn();
			break;					// exit: mouse movement was too small
		}
	
		// perform action according to cursor type
		CRect rectIn = m_displayRect;
		switch (m_cursorType)
		{			
		case 0:
			rectOut = rectIn;
			rectOut.OffsetRect(m_ptFirst.x - m_ptLast.x, m_ptFirst.y - m_ptLast.y);
			ZoomData(&rectIn, &rectOut);
			break;
		case CURSOR_ZOOM: 	// zoom operation
			ZoomData(&rectIn, &rectOut);
			m_ZoomFrom = rectIn;
			m_ZoomTo   = rectOut;					
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


//---------------------------------------------------------------------------

void CSpikeHistWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// compute pixel position of horizontal tags
	if (GetNHZtags() > 0)								// exit if none
	{
		for (int icur = GetNHZtags()-1; icur>=0; icur--)
			SetHZtagPix(icur, MulDiv(GetHZtagVal(icur)-m_yWO, m_yVE, m_yWE)+m_yVO);			
	}
	// compute pixel position of vertical tags
	if (GetNVTtags() > 0)
	{
		for (int icur = GetNVTtags()-1; icur>=0; icur--)	// loop through all tags
			SetVTtagPix(icur, MulDiv(GetVTtagVal(icur)-m_xWO, m_xVE, m_xWE)+m_xVO);
	}	
	CScopeScreen::OnLButtonDown(nFlags, point);	
	if (m_currCursorMode!=0 || m_HCtrapped >= 0)// do nothing else if mode != 0
		return;	 								// or any tag hit (VT, HZ) detected	
	
	// test if mouse hit one histogram
	// if hit, then tell parent to select corresp histogram (spike)
	m_hitspk = DoesCursorHitCurve(point);
	if (m_hitspk >= 0)
	{
		// cancel track rect mode
		m_trackMode = TRACK_OFF;		// flag trackrect
		ReleaseCursor();				// release cursor capture
		PostMyMessage(HINT_HITSPIKE, m_hitspk);		
		return;
	}
}

//---------------------------------------------------------------------------
// ZoomData
// convert pixels to logical pts and reverse to adjust curve to the
// rectangle selected
// lp to dp: d = (l -wo)*ve/we + vo
// dp to lp: l = (d -vo)*we/ve + wo
// wo= window origin; we= window extent; vo=viewport origin, ve=viewport extent
// with ordinates: wo=zero, we=yextent, ve=rect.height/2, vo = -rect.Height()/2
//---------------------------------------------------------------------------

void CSpikeHistWnd::ZoomData(CRect* rFrom, CRect* rDest)
{
	rFrom->NormalizeRect();
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

void CSpikeHistWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if ( m_hitspk < 0)
		CScopeScreen::OnLButtonDblClk(nFlags, point);
	else
	{
		GetParent()->PostMessage(WM_COMMAND,MAKELONG(GetDlgCtrlID(), BN_DOUBLECLICKED),(LPARAM) m_hWnd);
	}
}


// -------------------------------------------------------------

int CSpikeHistWnd::DoesCursorHitCurve(CPoint point)
{
	int hitspk = -1;
	// convert device coordinates into logical coordinates
	int deltax = MulDiv(3, m_xWE, m_xVE);
	int mouseX = MulDiv(point.x-m_xVO, m_xWE, m_xVE) + m_xWO;
	int mouseX1 = mouseX - deltax;
	int mouseX2 = mouseX-deltax;
	if (mouseX1 < 1)
		mouseX1 = 1;
	if (mouseX1 > m_nbins)
		mouseX1 = m_nbins;
	if (mouseX2 < 1)
		mouseX2 = 1;
	if (mouseX2 > m_nbins)
		mouseX2 = m_nbins;
		
	int deltay = MulDiv(3, m_yWE, m_yVE);
	DWORD mouseY = (DWORD) MulDiv(point.y-m_yVO, m_yWE, m_yVE) + m_yWO + deltay;	
	
	// test selected histogram first (foreground)
	int ihist =1;
	CDWordArray* pDW = nullptr;
	if (m_plotmode == PLOT_ONECLASS || m_plotmode == PLOT_ONECLASSONLY)
	{
		// get array corresp to m_selclass as well as histogram index
		for (int i=1; i<histogram_ptr_array.GetSize(); i++)
		{
			if ((int) (histogram_ptr_array[i])->GetAt(0) == m_selclass)
			{
				pDW = histogram_ptr_array[i];
				ihist = i;
				break;
			}					
		}
		//
		if (pDW != nullptr)
		{ 
			for (int i = mouseX1; i<mouseX2; i++)
			{
				DWORD iww = pDW->GetAt(i - 1);
				if (mouseY <= iww)
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
		for (int ihist=1; ihist<histogram_ptr_array.GetSize() && hitspk<0; ihist++)
		{
			pDW = histogram_ptr_array.GetAt(ihist);
			if (m_plotmode == PLOT_ONECLASS && ((int) pDW->GetAt(0)) == m_selclass)
				continue;			
			for (int i=mouseX1; i<=mouseX2; i++)
			{
				DWORD iww = pDW->GetAt(i-1);
				if (mouseY <= iww)
				{			
					hitspk = ihist;
					break;
				}
			}			
		}
	}
	return hitspk;
}

// ------------------------------------------------------------------------------------

void CSpikeHistWnd::GetExtents()
{
	if (m_yWE == 1) // && m_yWO == 0)
	{
		if (m_lmax==0)
			GetHistogLimits(0);		
		m_yWE = (int) m_lmax;
		m_yWO = 0;
	}

	if (m_xWE == 1) // && m_xWO == 0)
	{
		m_xWE = m_abcissamaxval-m_abcissaminval+1;
		m_xWO = m_abcissaminval;
	}
}

////////////////////////////////////////////////////////////////////////////////////
// Histogram data
																			   

//*************************************************************************************
// GetHistogLimits()
// compute max, index max and index first and last interval with data
//*************************************************************************************
void CSpikeHistWnd::GetHistogLimits(int ihist)
{
	// for some unknown reason, m_pHistarray is set at zero when arriving here
	if (histogram_ptr_array.GetSize() <= 0)
	{
		CDWordArray* pDW = new (CDWordArray);	// init array
		ASSERT(pDW != NULL);
		histogram_ptr_array.Add(pDW);					// save pointer to this new array
		ASSERT(histogram_ptr_array.GetSize() > 0);
	}
	CDWordArray* pDW = histogram_ptr_array[ihist];
	if (pDW->GetSize() <= 1)
		return;
	// Recherche de l'indice min et max de l'histograme
	// En dessous de min toutes les cases du tableau sont
	// à zéro. Au dela de max toute les cases du tableau sont à zéro.

	m_ifirst = 1;		// search first interval with data
	while( m_ifirst <=m_nbins && pDW->GetAt(m_ifirst) == 0)
		m_ifirst++;		

	m_ilast = m_nbins;	// search last interval with data
	while( pDW->GetAt(m_ilast) == 0 && m_ilast > m_ifirst)
		m_ilast--;		

	// Récuperation de l'indice du maximum
	m_imax = m_ifirst;          // index first pt
	m_lmax=0;					// max	
	DWORD dwitem;
	for (int i=m_ifirst; i<=m_ilast; i++)
	{
		dwitem = pDW->GetAt(i);
		if ( dwitem > m_lmax)
		{
			m_imax = i;
			m_lmax = pDW->GetAt(i);
		}
	}
}
	
//**********************************************************************************
// 	BuildHistogFromDoc()
// parameters
//		CWordArray* pVal	- word array source data
//		CDWordArray	pTime	- dword array with time intervals assoc with pVal
//		long lFirst			= index first pt from file
//		long lLast 			= index last pt from file
//		int max				= maximum
//		int min				= minimum
//		int nbins			= number of bins -> bin size
//		BOOL bNew=TRUE		= erase old data (TRUE) or add to old value (FALSE)
//    
//**********************************************************************************

void CSpikeHistWnd::ReSize_And_Clear_Histograms(int nbins, int max, int min)
{
	m_binsize = (max-min+1)/nbins+1;		// set bin size
	m_abcissaminval = min;					// set min
	m_abcissamaxval = min+nbins*m_binsize;	// set max
	
	m_nbins = nbins;
	for (int j=histogram_ptr_array.GetUpperBound(); j>= 0; j--)
	{
		CDWordArray* pDW = histogram_ptr_array[j];
		pDW->SetSize(nbins+1);
		// erase all data from histogram
		for (int i=1; i<= nbins; i++)
			pDW->SetAt(i, 0);
	}
}


void CSpikeHistWnd::OnSize(UINT nType, int cx, int cy) 
{
	CScopeScreen::OnSize(nType, cx, cy);
	m_yVO=cy;
}

// BuildHistFromWordArray
// parameters:
// 1) data arrays
//		pVal	- array of words storing the parameters measured on the spikes; size: nspikes.
//		piiTime	- array of spike occurence time; size: nspikes.
//		pClass	- array of class values; ; size: nspikes.
// 2) boundaries:
//		lFirst	- first time interval that is ok
//		lLast	- last time interval that is ok
//		max		- maximum value of "pVal" taken into account
//		min		- minimum value of "pVal"
//		nbins	- number of bins in the histogram
// 3) afresh?
//		bNew	- yes: erase old data, no: add new data to the current ones

void CSpikeHistWnd::BuildHistFromWordArray(
			CWordArray* pVal, CDWordArray* piiTime, CWordArray* pClass,
			long lFirst, long lLast, int max, int min, int nbins,
			BOOL bNew)
{
	// erase data and arrays if bnew:
	if (bNew)
		RemoveHistData();

	// for some unknown reason, m_pHistarray is set at zero when arriving here
	if (histogram_ptr_array.GetSize() <= 0)
	{
		CDWordArray* pDW = new (CDWordArray);	// init array
		ASSERT(pDW != NULL);
		histogram_ptr_array.Add(pDW);					// save pointer to this new array
		ASSERT(histogram_ptr_array.GetSize() > 0);
	}
	CDWordArray* pDW0 = histogram_ptr_array[0];
	if (nbins != m_nbins || pDW0->GetSize() != (nbins+1))
		ReSize_And_Clear_Histograms (nbins, max, min);

	// get parameters from document		
	// loop through source data array
	DWORD dwData;
	int spike_class;
	CDWordArray* pDW = nullptr;

	for (int i=pVal->GetUpperBound(); i>=0; i--)	
	{
		// check that the corresp spike fits within the time limits requested
		long iitime = piiTime->GetAt(i);		// get spike time
		if (iitime < lFirst || iitime > lLast)	// check if within requested interval
			continue;							// no: skip this spike
		int index = pVal->GetAt(i);				// get spike parameter value
		if (index > m_abcissamaxval || index < m_abcissaminval)
			continue;

		// increment corresponding histogram interval into the first histogram (general, displayed in grey)
		index = (index-m_abcissaminval)/m_binsize +1;
		dwData = pDW0->GetAt(index) + 1;
		pDW0->SetAt(index, dwData);

		// dispatch into corresp class histogram (create one if necessary)
		if (pClass->GetSize() > 0 )
		{
			spike_class = pClass->GetAt(i);
			GetClassArray(spike_class, pDW);
			if (pDW == nullptr)
			{
				pDW = new (CDWordArray);	// init array
				ASSERT(pDW != NULL);
				histogram_ptr_array.Add(pDW);		// save pointer to this new array
				pDW->SetSize(nbins+1);
				for (int i=1; i<= nbins; i++)
					pDW->SetAt(i, 0);
				pDW->SetAt(0, spike_class);
			}
		}
		if (pDW != nullptr)
		{ 
			dwData = pDW->GetAt(index)+1;
			pDW->SetAt(index, dwData);
		}
	}
	GetHistogLimits(0);    
}
