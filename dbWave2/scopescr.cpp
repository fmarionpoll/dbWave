#include "stdafx.h"
#include "resource.h"
#include "scopescr.h"

#include "editctrl.h"
#include "ScopeScreenPropsDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COLORREF CScopeScreen::m_colorTable[] =
{
	RGB(  0,   0,   0),	// 0 black
	RGB(128,   0, 128),	// 1 purple
	RGB(201, 192, 187),	// 2 silver
	RGB(128,   0,   0),	// 3 maroon
	RGB(196,   2,  51),	// 4 red 
	RGB(128, 128, 128),	// 5 grey
	RGB(249, 132, 229),	// 6 fuschia
	RGB(  0, 128,   0),	// 7 green
	RGB( 91, 255,   0),	// 8 lime
	RGB(107, 142,  35),	// 9 olive
	RGB(255, 205,   0),	// 10 yellow
	RGB(  0,   0, 128),	// 11 navy	
	RGB(  0,   0, 255),	// 12 blue
	RGB(  0, 128, 128),	// 13 teal
	RGB(  0, 255, 255),	// 14 aqua
	RGB(255, 255, 255),	// 15 white
	RGB(1, 1, 1)		// 16 dummy
};

HCURSOR CScopeScreen::m_cursor[NB_CURSORS];
int		CScopeScreen::m_cursordragmode[NB_CURSORS];
int		CScopeScreen::m_countcurs=0;

static TCHAR csUnit[] = {_T("GM  mµpf  ")};		// units & corresp powers
static int  dUnitsPower[] = { 9,6, 0, 0, -3, -6, -9, -12, 0}; 
static int	dmaxIndex		= 8;				// nb of elmts
static int	dniceIntervals[] = {1, 5, 10,  20,  25,  30,  40, 50, 75, 100, 200, 250, 300, 400, 500, /*750, */
								/*600,*/ /*700,*//* 800, *//*900,*/
								0};

int CScopeScreen::FindColor(COLORREF ccolor)
{
	int icolor = -1;
	for (int i = 0; i < NB_COLORS; i++)
		if (ccolor == m_colorTable[i])
			icolor = i;
	return icolor;
}

int CScopeScreen::NiceUnit(float xVal)
{   
	int i=0;
	int ival = (int) xVal;	
	do	
	{
		if (ival <= dniceIntervals[i])
		{
			i++;
			break;
		}
		i++;
	} while (dniceIntervals[i] > 0);
	return dniceIntervals[i-1];
}

// --------------------------------------------------------------------------
// ChangeUnit: adapt xUnit (string) & scalefactor to xval
// input:
//		xval 
// return:
//		scaled value
//		xScalefactor
//		xUnit
// --------------------------------------------------------------------------

float CScopeScreen::ChangeUnit(float xVal, CString* xUnit, float* xScalefactor)
{   
	// avoid division by zero error
	if (xVal == 0)
	{
		*xScalefactor = 1.0f;
		xUnit->SetAt(0, ' ');
		return 0.0f;
	}

	// take absolute value of xVal and save sign
	short	i;  
	short isign = 1;
	if ( xVal <0)
	{
		isign = -1;
		xVal = -xVal;
	}
	// get power of 10 of the value
	short iprec= (short) log10(xVal);	// log10 of value (upper limit)
	if (( iprec <=0) && (xVal < 1.))	// perform extra checking if iprec <= 0
		i = 4-iprec/3;					// change equation if Units values change
	else			      
		i = 3-iprec/3;					// change equation if Units values change
	if (i > dmaxIndex)					// clip to max index
		i = dmaxIndex;
	else if (i<0)						// or clip to min index
		i = 0;
	// return data
	*xScalefactor = (float) pow(10.0f, dUnitsPower[i]);		// convert & store
	xUnit->SetAt(0, csUnit[i]);								// replace character corresp to unit
	return xVal*isign / *xScalefactor;						// return value/scale_factor
}

IMPLEMENT_SERIAL (CScopeScreen, CWnd, 1)

CScopeScreen::CScopeScreen()
{
	// load cursors from resources		// #define NB_CURSORS 3
	if (m_countcurs == 0)
	{		
		short j=0;
		m_cursor[j] = ::LoadCursor(nullptr,IDC_ARROW);
		m_cursordragmode[j] = 0;			// draw frame when changed
		j++;								// catch object
		m_cursor[j] = AfxGetApp()->LoadCursor(IDC_CZOOM);
		m_cursordragmode[j] = 1; 
		j++;								// draw inverted rectangle
		m_cursor[j] = AfxGetApp()->LoadCursor(IDC_CCROSS);
		m_cursordragmode[j] = 1; 
		j++;								// add a spike
		m_cursor[j] = AfxGetApp()->LoadCursor(IDC_CCROSS);
		m_cursordragmode[j] = 1; 
		j++;								// add a spike
	}
	m_countcurs++;

	m_currCursor = m_cursor[0];			// standard cursor: arrow
	m_currCursorMode = m_cursordragmode[0];
	m_cursorType = 0;					// standard cursor until de-selected
	m_oldcursorType = 0;
	
	m_iUndoZoom = 0;					// no undo stored yet
	m_clientRect = CRect(0,0, 10, 10);	// minimal size of the button
	AdjustDisplayRect(&m_clientRect);
	m_trackMode = TRACK_OFF;
	m_bVTtagsLONG = FALSE;				// VT tags are defined as int
	m_yWE = 1;							// initial parameters for
	m_yWO = 0;							// y and  x axis extent and
	m_xWE = 1;							// origin when data displayed
	m_xWO = 0;							// under 
	m_xVE = 1;
	m_xVO = 0;
	m_yVE = 1;
	m_yVO = 0;

	m_plotmode=0;
	m_cxjitter = GetSystemMetrics(SM_CXDOUBLECLK);
	m_cyjitter = GetSystemMetrics(SM_CYDOUBLECLK);
	m_erasebkgnd=TRUE;	
	
	m_bUseDIB=FALSE;
	m_bAllowProps=TRUE;
	m_bLmouseDown=FALSE;
	m_hwndReflect = nullptr;
	m_tempVTtag = nullptr;

	m_bBottomComment = FALSE;
	m_blackDottedPen.CreatePen(PS_DOT, 0, m_colorTable[BLACK_COLOR]);
	m_colorbackgr=SILVER_COLOR;
	m_colorselected=BLACK_COLOR;

	// set colored CPen objects
	for (int i = 0; i < NB_COLORS; i++) 
		m_penTable[i].CreatePen(PS_SOLID, 0, m_colorTable[i]);

	m_bNiceGrid				= FALSE;
	m_xRuler.m_bHorizontal	= TRUE;
	m_yRuler.m_bHorizontal	= FALSE;
	m_hFont.CreateFont(12, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH|FF_ROMAN, _T("Arial"));
	m_abcissaheight  = 10;
	m_ordinateswidth = 25;

	m_pXRulerBar = nullptr;
	m_pYRulerBar = nullptr;

	m_liFirst = 0;
	m_liLast = 0;
}

CScopeScreen::~CScopeScreen()
{
	// remove static objects
	m_countcurs--;
	if (m_countcurs == 0)
	{
		for (int i = 1; i<NB_CURSORS; i++)	// elmt 0: global object cursor
		{
			if (nullptr != m_cursor[i]) 
				::DestroyCursor(m_cursor[i]);
		}
	}
	m_HZtags.RemoveAllTags();			// remove horizontal tags
	m_VTtags.RemoveAllTags();			// remove vertical tags	
	if (m_tempVTtag != nullptr) delete m_tempVTtag;

	// delete array of pens
	for (int i = 0; i < NB_COLORS; i++) 
		m_penTable[i].DeleteObject();

}

// trap call to presubclass in order to get source window size..
// assume that palette is present within the application inside CMainFrame...
void CScopeScreen::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	// at this stage, assume that m_hWnd is valid
	::GetClientRect(m_hWnd, &m_clientRect);
	AdjustDisplayRect(&m_clientRect);
	m_xVO=	m_displayRect.left;
	m_xVE=	m_displayRect.Width();
	m_yVO=	m_displayRect.Height()/2;
	m_yVE=	-m_displayRect.Height();	
}

BEGIN_MESSAGE_MAP(CScopeScreen, CWnd)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScopeScreen message handlers

BOOL CScopeScreen::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	BOOL flag = CWnd::Create(nullptr, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	if (flag != 0)
		PreSubclassWindow();
	return flag;
}

void CScopeScreen::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	SetDisplayAreaSize(cx, cy);
	// force DC to redraw
	if (m_PlotDC.GetSafeHdc())
		m_PlotDC.DeleteDC();
	// refresh control
	Invalidate();
}

void CScopeScreen::SetDisplayAreaSize(int cx, int cy)
{
	// update coordinates
	m_clientRect.bottom = cy;
	m_clientRect.right = cx;
	AdjustDisplayRect(&m_clientRect);
	m_xVO = m_displayRect.left;
	m_xVE = m_displayRect.Width();
	m_yVO = m_displayRect.Height()/2;
	m_yVE =-m_displayRect.Height();
}

BOOL CScopeScreen::OnEraseBkgnd(CDC* pDC)
{
	return TRUE; // say we handled it
}

//-----------------------------------------
// PlotToBitmap()
// two bitmaps are used to display data
//  CBitmap*	m_pbitmapPlot;		// main data
//	CBitmap*	m_pbitmapSelect;	// data selected (displayed over bitmapplot)
	
void CScopeScreen::PlotToBitmap(CDC *pDC)
{
	CBitmap* poldPlotBitmap = nullptr;
	CBitmap bitmapPlot;
	bitmapPlot.CreateBitmap(m_clientRect.right, m_clientRect.bottom, pDC->GetDeviceCaps(PLANES), pDC->GetDeviceCaps(BITSPIXEL), nullptr);
	m_PlotDC.CreateCompatibleDC(pDC);
	poldPlotBitmap = m_PlotDC.SelectObject(&bitmapPlot);
	PlotDatatoDC(&m_PlotDC);
	pDC->BitBlt(0, 0, m_displayRect.right, m_displayRect.bottom, &m_PlotDC,0,0, SRCCOPY);
	m_PlotDC.SelectObject(poldPlotBitmap);
}

void CScopeScreen::OnPaint()
{
	CPaintDC dc(this);
	dc.IntersectClipRect(&m_clientRect);

	// plot directly on the screen
	if (!m_bUseDIB)
		PlotDatatoDC(&dc);
	// display the bitmap
	else
		PlotToBitmap(&dc);
}

void CScopeScreen::PlotDatatoDC(CDC* pDC)
{
}

void CScopeScreen::EraseBkgnd(CDC* pDC)
{
	// erase background around m_displayRect (assume only left and bottom areas)
	if (m_bNiceGrid)
	{
		CRect rect = m_clientRect;
		rect.right = m_displayRect.left;
		pDC->FillSolidRect(rect, GetSysColor(COLOR_BTNFACE));
		rect.top = m_displayRect.bottom;
		rect.right = m_clientRect.right;
		rect.left = m_displayRect.left;
		pDC->FillSolidRect(rect, GetSysColor(COLOR_BTNFACE));
	}
	// erase display area
	CBrush brush;
	brush.CreateSolidBrush(m_parms.crScopeFill);
	CBrush* pOldBrush = pDC->SelectObject(&brush);
	CPen* pOldPen = (CPen*) pDC->SelectStockObject(BLACK_PEN);
	pDC->Rectangle(&m_displayRect);
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	// display grid
	DrawGrid(pDC);
}

void CScopeScreen::DrawGridEvenlySpaced(CDC *pDC)
{
	CRect rect = m_displayRect;
	rect.DeflateRect(1, 1);

	// Standard grid is 8 high by 10 wide
	CPen penScale(PS_SOLID, 1, m_parms.crScopeGrid);
	CPen* ppenOld = pDC->SelectObject(&penScale);
	int iXTicks = m_parms.iXCells * m_parms.iXTicks;
	int iYTicks = m_parms.iYCells * m_parms.iYTicks;
	int iTickWidth = 2;
	int iTickHeight = 2; 

	// do the grid lines
	for (int i = 1; i < m_parms.iXCells; i++) 
	{
		pDC->MoveTo(i * rect.right / m_parms.iXCells, 0);
		pDC->LineTo(i * rect.right / m_parms.iXCells, rect.bottom);
	}	
	for (int i = 1; i < m_parms.iYCells; i++) 
	{
		pDC->MoveTo(0, i * rect.bottom / m_parms.iYCells);
		pDC->LineTo(rect.right, i * rect.bottom / m_parms.iYCells);
	}

	// Put tick marks on the axis lines
	for (int i = 1; i < iXTicks; i++) 
	{
		int y = rect.bottom - rect.bottom * m_parms.iXTickLine / m_parms.iYCells;
		pDC->MoveTo(i * rect.right / iXTicks, y - iTickWidth);
		pDC->LineTo(i * rect.right / iXTicks, y + iTickWidth);
	}
	for (int i = 1; i < iYTicks; i++) 
	{
		int x = rect.right * m_parms.iYTickLine / m_parms.iXCells;
		pDC->MoveTo(x - iTickHeight, i * rect.bottom / iYTicks);
		pDC->LineTo(x + iTickHeight, i * rect.bottom / iYTicks);
	}
	pDC->SelectObject(ppenOld);

	// if grids, draw scale text (dummy)
	if (m_parms.iXCells  > 1 && m_parms.iYCells > 1)
	{
		// convert value into text
		CString cs;
		cs.Format(_T("%.3f mV; %.3f ms"), m_parms.yScaleUnitValue, m_parms.xScaleUnitValue) ;
		int textlen = cs.GetLength();
		// plot text
		pDC->SelectObject (GetStockObject (DEFAULT_GUI_FONT));
		rect.DeflateRect(1,1);
		pDC->SetTextColor(m_parms.crScopeGrid);
		pDC->DrawText(cs, textlen, rect, DT_LEFT | DT_BOTTOM | DT_SINGLELINE); 
	}
}

void CScopeScreen::DrawGridFromRuler(CDC *pDC, CRuler* pRuler)
{
	CRect rcClient = m_displayRect;
	rcClient.DeflateRect(1, 1);

	// exit if length is not properly defined
	if (pRuler->m_dlast == pRuler->m_dfirst)
		return;

	CPen aPen2;
	aPen2.CreatePen(PS_SOLID, 1, m_parms.crScopeGrid);
	CPen* pOldPen = pDC->SelectObject(&aPen2);

	// draw ticks and legends
	int tickSmallHeight = 4;
	int tickmax;
	if (pRuler->m_bHorizontal)		// horizontal
		tickmax = rcClient.Width();
	else							// vertical
		tickmax = rcClient.Height();

	// draw scale
	//double smallscaleinc = pRuler->m_dscaleinc / 5.;
	double dpos = floor(pRuler->m_dscalefirst);
	double dlen = pRuler->m_dlast - pRuler->m_dfirst;
	pDC->SetBkMode(TRANSPARENT);

	while (dpos <= pRuler->m_dlast)
	{
		int tickPos;
		if (pRuler->m_bHorizontal)	// horizontal
		{
			tickPos = (int)(rcClient.Width() * (dpos - pRuler->m_dfirst) / dlen) + rcClient.left;
			if (tickPos >= 0 && tickPos <= tickmax) {
				pDC->MoveTo(tickPos, rcClient.bottom - 1);	// line
				pDC->LineTo(tickPos, rcClient.top    + 1);
			}
		}
		else						// vertical
		{
			tickPos = (int)(rcClient.Height() * (pRuler->m_dlast - dpos) / dlen) + rcClient.top;
			if (tickPos >= 0 && tickPos <= tickmax) {
				pDC->MoveTo(rcClient.left + 1, tickPos);	// line
				pDC->LineTo(rcClient.right - 1, tickPos);
			}
		}
		if( dpos != 0. && fabs(dpos) < 1E-10 )
		   dpos = 0 ;
		dpos += pRuler->m_dscaleinc;
	}
	// restore objects used in this routine
	pDC->SelectObject(pOldPen);
}

void CScopeScreen::DrawScalefromRuler(CDC *pDC, CRuler* pRuler)
{
	CRect rcClient = m_displayRect;
	rcClient.DeflateRect(1, 1);

	// exit if length is not properly defined
	if (pRuler->m_dlast == pRuler->m_dfirst)
		return;

	CPen aPen1, aPen2;
	aPen1.CreatePen(PS_SOLID, 1, m_parms.crScopeGrid);
	CPen* pOldPen = pDC->SelectObject(&aPen1);
	aPen2.CreatePen(PS_SOLID, 1, m_parms.crScopeGrid);
	CFont* pOldFont = pDC->SelectObject(&m_hFont);
	CString str;

	// draw ticks and legends
	int tickSmallHeight = 4;
	int tickmax;
	if (pRuler->m_bHorizontal)		// horizontal
		tickmax = rcClient.Width();
	else							// vertical
		tickmax = rcClient.Height();

	// draw scale
	double smallscaleinc = pRuler->m_dscaleinc / 5.;
	double dpos = floor(pRuler->m_dscalefirst);
	double dlen = pRuler->m_dlast - pRuler->m_dfirst;
	pDC->SetBkMode(TRANSPARENT);

	while (dpos <= pRuler->m_dlast)
	{
		// display small tick marks
		pDC->SelectObject(&aPen1);
		double dsmallpos = dpos;
		int tickPos;
		for (int i = 0; i<4; i++)
		{
			dsmallpos += smallscaleinc;
			double ratio = (pRuler->m_dlast - dsmallpos) / dlen;
			if (pRuler->m_bHorizontal) // ----------------------------- horizontal
			{
				tickPos = (int)(rcClient.Width() * (dsmallpos - pRuler->m_dfirst) / dlen) + rcClient.left;
				if (tickPos >= rcClient.left && tickPos <= tickmax)
				{
					pDC->MoveTo(tickPos, rcClient.bottom - 1);
					pDC->LineTo(tickPos, rcClient.bottom - tickSmallHeight);
				}
			}
			else // --------------------------------------------------- vertical
			{
				tickPos = (int)(rcClient.Height() * (pRuler->m_dlast - dsmallpos) / dlen) + rcClient.top;
				if (tickPos >= rcClient.top && tickPos <= tickmax)
				{
					pDC->MoveTo(rcClient.left + 1, tickPos);
					pDC->LineTo(rcClient.left + tickSmallHeight, tickPos);
				}
			}
		}

		// display large ticks and text
		pDC->SelectObject(&aPen2);
		if (pRuler->m_bHorizontal)	// horizontal
			tickPos = (int)(rcClient.Width() * (dpos - pRuler->m_dfirst) / dlen) + rcClient.left;
		else						// vertical
			tickPos = (int)(rcClient.Height() * (pRuler->m_dlast - dpos) / dlen) + rcClient.top;
		if (tickPos >= 0 && tickPos <= tickmax)
		{
			str.Format(_T("%g"), dpos);
			CSize size = pDC->GetTextExtent(str);
			int x, y;
			if (pRuler->m_bHorizontal)	// ----------- horizontal
			{
				pDC->MoveTo(tickPos, rcClient.bottom - 1);	// line
				pDC->LineTo(tickPos, rcClient.top + 1);
				x = tickPos - (size.cx / 2);					// text position (upper left)
				if (x < 0)
					x = 0;
				if (x + size.cx > rcClient.right)
					x = rcClient.right - size.cx;
				y = rcClient.bottom + 1; // - 1; //- size.cy
			}
			else // ---------------------------------- vertical
			{
				pDC->MoveTo(rcClient.left + 1, tickPos);	// line
				pDC->LineTo(rcClient.right - 1, tickPos);
				x = rcClient.left - size.cx - 2;			// text position (upper left)
				y = tickPos - (size.cy / 2);
			}
			pDC->TextOut(x, y, str);
		}
		if (dpos != 0. && fabs(dpos) < 1E-10)
			dpos = 0;
		dpos += pRuler->m_dscaleinc;
	}

	// restore objects used in this routine
	pDC->SelectObject(pOldPen);
}

void CScopeScreen::DrawGridNicelySpaced(CDC *pDC)
{
	if (m_pXRulerBar == nullptr)
		DrawScalefromRuler (pDC, &m_xRuler);
	else
	{ 
		m_pXRulerBar->DrawScalefromRuler(&m_xRuler);
		m_pXRulerBar->Invalidate();
		DrawGridFromRuler(pDC, &m_xRuler);
	}
		
	if (m_pYRulerBar == nullptr) 
		DrawScalefromRuler (pDC, &m_yRuler);
	else
	{
		m_pYRulerBar->DrawScalefromRuler(&m_yRuler);
		m_pYRulerBar->Invalidate(); 
		DrawGridFromRuler(pDC, &m_yRuler);
	}
}

void CScopeScreen::AdjustDisplayRect(CRect* pRect)
{
	m_displayRect = *pRect;
	if(m_bNiceGrid)
	{
		if (m_pYRulerBar == nullptr) 
			m_displayRect.left	+= m_ordinateswidth;
		if (m_pXRulerBar == nullptr)
			m_displayRect.bottom -= m_abcissaheight;
	}
}

void CScopeScreen::DrawGrid(CDC* pDC)
{
	if (m_bNiceGrid)
		DrawGridNicelySpaced(pDC);
	else
		DrawGridEvenlySpaced(pDC);
}

void CScopeScreen::SetNxScaleCells(int iCells, int iTicks, int iTickLine)
{ 
	m_parms.iXCells		= iCells; 
	m_parms.iXTicks		= iTicks;  
	m_parms.iXTickLine	= iTickLine;
}

void CScopeScreen::SetNyScaleCells(int iCells, int iTicks, int iTickLine)
{ 
	m_parms.iYCells = iCells; 
	m_parms.iYTicks = iTicks; 
	m_parms.iYTickLine = iTickLine;
}

void CScopeScreen::SendMyMessage(int code, int codeparm)
{
	// see "awave32.h" for available messages
	int iID = GetDlgCtrlID();
	GetParent()->SendMessage(WM_MYMESSAGE, code, MAKELONG(codeparm, iID));
}

void CScopeScreen::PostMyMessage(int code, int codeparm)
{
	// see "awave32.h" for available messages
	int iID = GetDlgCtrlID();
	GetParent()->PostMessage(WM_MYMESSAGE, code, MAKELONG(codeparm, iID));
}

void CScopeScreen::PrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	// mapping mode is MM_ANISOTROPIC
	pDC->SetMapMode(MM_ANISOTROPIC);

	// not printing.............................
	if (pInfo == nullptr)
	{
		pDC->SetViewportOrg (m_xVO, m_yVO);
		pDC->SetViewportExt (m_xVE, m_yVE);
		if (m_yWE == 0)
			m_yWE = 1024;
		pDC->SetWindowExt (m_xWE, m_yWE);
		pDC->SetWindowOrg (m_xWO, m_yWO);
	}
}

int CScopeScreen::SetMouseCursorType(int cursorm)
{
	m_oldcursorType = m_cursorType;
	if (cursorm <0)
		cursorm = 0;
	if (cursorm > NB_CURSORS-1)
		cursorm = NB_CURSORS-1;
	m_cursorType = cursorm;
	m_currCursor = m_cursor[m_cursorType];	
	m_currCursorMode = m_cursordragmode[m_cursorType];
	return cursorm;
}

void CScopeScreen::CaptureCursor()
{	
	SetCapture();				// capture mouse
	CRect rectLimit= m_displayRect;
	ClientToScreen(rectLimit);	// convert coordinates
	ClipCursor(rectLimit);		// tell mouse cursor what are the limits
}

void CScopeScreen::ReleaseCursor()
{
	// mouse was captured	
	ReleaseCapture();
	ClipCursor(nullptr);
}

BOOL CScopeScreen::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	::SetCursor(m_currCursor);
	return TRUE;	
}

void CScopeScreen::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int cursor = m_cursorType+ 1;
	PostMyMessage(HINT_SETMOUSECURSOR, cursor);
}

//---------------------------------------------------------------------------
// MOUSE left button down
// check if: HZ or VT tag hit, if yes, start tracking
//---------------------------------------------------------------------------

void CScopeScreen::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_hwndReflect != nullptr)
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(m_hwndReflect, & rect0);

		// reflect mouse move message
		::SendMessage(m_hwndReflect, WM_LBUTTONDOWN, nFlags, 
			MAKELPARAM(point.x + (rect1.left-rect0.left), 
			point.y + (rect1.top-rect0.top)));
		return;
	}

	m_ptFirst = point;					// store mouse
	m_ptLast = point;					// initial coordinates	
	m_ptCurr = point;

	// take action according to cursor mode
	switch (m_cursorType)				// tracking type
	{
	// track horizontal & VT cursors if mouse HIT
	case 0:								// arrow (default)
	case CURSOR_MEASURE:				// cross (measure mode) (2)
		if (nFlags & MK_CONTROL)
		{
			PostMyMessage(HINT_LMOUSEBUTTONDOW_CTRL, MAKELONG(point.x, point.y));
		}
		m_trackMode = TRACK_RECT;		// flag trackrect

		// test HZ tags - if OK, then start tracking & init variables & flags
		m_HCtrapped = HitTestHZtag(point.y);
		if (m_HCtrapped>=0)
		{
			m_trackMode = TRACK_HZTAG;
			m_ptLast.x = 0;				// set initial coordinates
			m_ptLast.y = m_HZtags.GetTagPix(m_HCtrapped);
			m_ptFirst  = m_ptLast;
			// tell parent that HZtag was selected
			SendMyMessage(HINT_HITHZTAG, m_HCtrapped);
			break;
		}

		// test VT tags - if OK, then track
		if (!m_bVTtagsLONG)
			m_HCtrapped = HitTestVTtagPix((int)point.x);
		else
		{
			m_liJitter = ((long) m_cxjitter) * (m_liLast-m_liFirst+1)/((long)m_displayRect.Width());
			long lx = ((long)point.x)*(m_liLast-m_liFirst+1)/((long)m_displayRect.Width()) + m_liFirst;
			m_HCtrapped = HitTestVTtagLong(lx);
		}

		// mouse cursor did hit a tag, either horizontal or vertical
		if (m_HCtrapped>=0)
		{
			m_trackMode = TRACK_VTTAG;
			if (m_bVTtagsLONG)
				m_ptLast.x= (int) ((m_VTtags.GetTagLVal(m_HCtrapped)-m_liFirst)*((long)m_displayRect.Width())/(m_liLast-m_liFirst+1));					
			else
				m_ptLast.x=m_VTtags.GetTagPix(m_HCtrapped);
			m_ptLast.y=0;
			// tell parent that VTtag was selected
			SendMyMessage(HINT_HITVERTTAG, m_HCtrapped);
			break;
		}
		break;

	// track rectangle and invert content of the rectangle
	case CURSOR_ZOOM:					// zoom (1)
		m_trackMode = TRACK_RECT;
		InvertTracker(point);			// invert rectangle
		break;

	case CURSOR_VERTICAL:
		GetParent()->SendMessage(WM_MYMESSAGE, WM_LBUTTONDOWN, MAKELONG(point.x, point.y));
		return;
		break;

	default:
		break;
	}

	// limit the tracking to the client area of the view
	CaptureCursor();
	return;
}

//---------------------------------------------------------------------------
// OnMouseMove() update XORed-rectangle if mouse is captured
//---------------------------------------------------------------------------

void CScopeScreen::OnMouseMove(UINT nFlags, CPoint point)
{	
	// track rectangle : update rectangle size
	switch (m_trackMode)
	{
	case TRACK_RECT:
		InvertTracker(point);
		break;

	// track horizontal tag : move tag, get val and send message
	case TRACK_HZTAG:
		if (point.y != m_ptCurr.y)
		{
			m_ptCurr = point;
			int val = MulDiv(point.y-m_yVO, m_yWE, m_yVE) + m_yWO;
			XorHZtag(point.y);			// move tag to new pixel
			m_HZtags.SetTagVal(m_HCtrapped, val);
			PostMyMessage(HINT_MOVEHZTAG, m_HCtrapped);	
		}
		break;

	// track vertical tag : move tag & update val
	case TRACK_VTTAG:
		if (point.x != m_ptCurr.x)
		{
			XorVTtag(point.x);			// move cursor to new pixel		
			m_ptCurr = point;
			m_VTtags.SetTagPix(m_HCtrapped, point.x);
			if (!m_bVTtagsLONG)
			{
				int val = MulDiv(point.x-m_xVO, m_xWE, m_xVE) + m_xWO;
				m_VTtags.SetTagVal(m_HCtrapped, val);
			}
			else
			{
				long lval = ((long)point.x)*(m_liLast-m_liFirst+1)/((long)m_displayRect.Width()) + m_liFirst;
				m_VTtags.SetTagLVal(m_HCtrapped, lval);
			}
			PostMyMessage(HINT_MOVEVERTTAG, m_HCtrapped);
		}
		break;

	default:
		if (m_hwndReflect != nullptr)
		{
			// convert coordinates
			CRect rect0, rect1;
			GetWindowRect(&rect1);
			::GetWindowRect(m_hwndReflect, & rect0);

			// reflect mouse move message
			::SendMessage(m_hwndReflect, WM_MOUSEMOVE, nFlags, 
				MAKELPARAM(point.x + (rect1.left-rect0.left), 
				point.y + (rect1.top-rect0.top)));
		}
		break;
	}
}

//---------------------------------------------------------------------------
// OnLButtonUp
//---------------------------------------------------------------------------
void CScopeScreen::OnLButtonUp(UINT nFlags, CPoint point) 
{	
	if (m_trackMode != TRACK_OFF)
	{
		ReleaseCursor();
		if (m_trackMode == TRACK_RECT && m_currCursorMode)
			InvertTracker(point);
		m_trackMode = TRACK_OFF;
	}
	else if (m_hwndReflect != nullptr)
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(m_hwndReflect, & rect0);

		// reflect mouse move message
		::SendMessage(m_hwndReflect, WM_LBUTTONUP, nFlags, 
			MAKELPARAM(point.x + (rect1.left-rect0.left), 
			point.y + (rect1.top-rect0.top)));
	}
	m_bLmouseDown=FALSE;
}

//---------------------------------------------------------------------------
// MOUSE right button down
// either capture mouse (cursor mode = ZOOM) or nothing
//---------------------------------------------------------------------------
void CScopeScreen::OnRButtonDown(UINT nFlags, CPoint point) 
{	
	switch (m_cursorType)
	{
	case CURSOR_ZOOM:
	case CURSOR_MEASURE:	// tracking type	
		m_ptFirst = point;
		m_ptLast = point;
		m_trackMode = TRACK_RECT;		// flag trackrect
		InvertTracker(point);			// invert rectangle
		CaptureCursor();
		break;
	case CURSOR_VERTICAL:
	default:
		CWnd::OnRButtonDown(nFlags, point);
		break;
	}
}

//---------------------------------------------------------------------------
// MOUSE right button up
// either - unzoom area (special case: only if cursor=ZOOM)
// or     - start properties dialog box
//---------------------------------------------------------------------------
void CScopeScreen::OnRButtonUp(UINT nFlags, CPoint point) 
{
	switch (m_trackMode)
	{
	case TRACK_RECT:
		{
		ReleaseCursor();
		// skip too small a rectangle (5 pixels?)
		CRect rectOut(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
		const short jitter = 3;
		if (rectOut.Height()< jitter && rectOut.Width()< jitter)
		{
			ZoomOut();
		}
		else
		{
			CRect rectIn = m_displayRect;
			ZoomData(&rectOut, &rectIn);
			m_ZoomFrom = rectOut;
			m_ZoomTo   = rectIn;
			m_iUndoZoom = -1;
		}
		}
		if (m_cursorType == CURSOR_MEASURE)
			PostMyMessage(HINT_RMOUSEBUTTONUP, NULL);
		else
			PostMyMessage(HINT_SETMOUSECURSOR, m_oldcursorType);
		break;

	case TRACK_OFF:
		CWnd::OnRButtonUp(nFlags, point);
		if (m_bAllowProps)
		{
			CScopeScreenPropsDlg dlg;
			dlg.m_pscope = this;
			// save parameters to restore it later if necessary
			SCOPESTRUCT parms_old;
			parms_old = m_parms;
			m_bAllowProps=FALSE;		// inhibit properties

			// if Cancel or Escape or anything else: restore previous values
			if (IDOK != dlg.DoModal())
			{
				m_parms = parms_old;
				Invalidate();				
			}
			else
				PostMyMessage(HINT_WINDOWPROPSCHANGED, NULL);
			m_bAllowProps=TRUE;
		}
		break;

	default:
		ReleaseCursor();
		ZoomOut();
		break;
	}
	m_trackMode = TRACK_OFF;
}

void CScopeScreen::ZoomData(CRect* prevRect, CRect* newRect)
{
}

void CScopeScreen::ZoomPop()
{
	ZoomData(&m_ZoomTo, &m_ZoomFrom);
	m_iUndoZoom = 0;
}

void CScopeScreen::ZoomOut()
{
	if (m_iUndoZoom>0) // memory?
		ZoomPop();		
	else
	{
		CClientDC dc(this);
		m_ZoomTo		= m_displayRect;
		m_ZoomFrom		= m_ZoomTo;
		short yshrink	= m_ZoomTo.Height()/4;
		short xshrink	= m_ZoomTo.Width()/4;
		m_ZoomTo.InflateRect(xshrink, yshrink);
		ZoomData(&m_ZoomFrom, &m_ZoomTo);
		m_iUndoZoom		= -1;
	}	
}

void CScopeScreen::ZoomIn()
{
	if (m_iUndoZoom<0) // memory?
		ZoomPop();
	else
	{
		CClientDC dc(this);
		m_ZoomTo		= m_displayRect;
		m_ZoomFrom		= m_ZoomTo;
		short yshrink	= -m_ZoomTo.Height()/4;
		short xshrink	= -m_ZoomTo.Width()/4;
		m_ZoomTo.InflateRect(xshrink, yshrink);
		ZoomData(&m_ZoomFrom, &m_ZoomTo);
		m_iUndoZoom = 1;
	}
}

//---------------------------------------------------------------------------
// HitTest.. -HZtag; -VTtagLong; -VTtagPix
//---------------------------------------------------------------------------
int CScopeScreen::HitTestHZtag(int y)
{
	int chit = -1;				// horizontal cursor hit
	const int jitter = 3;		// jitter allowed: 5 pixels total		
	int j = m_HZtags.GetNTags();
	for (int i = 0; i<j; i++)	// loop through all cursors
	{
		int val = m_HZtags.GetTagPix(i);	// get pixel value
		if (val <= y+ jitter && val >= y-jitter)
		{
			chit = i;
			break;
		}
	}
	return chit;
}

int CScopeScreen::HitTestVTtagLong(long lx)
{
	int chit = -1;				// horizontal cursor hit
	int j = m_VTtags.GetNTags();
	for (int i = 0; i<j; i++)	// loop through all cursors
	{
		long lval = m_VTtags.GetTagLVal(i);
		//long lval = (long) m_vt_tags.GetTagVal(i);
		if (lval <= lx+ m_liJitter && lval >= lx-m_liJitter)
		{
			chit = i;
			break;
		}
	}
	return chit;
}

int CScopeScreen::HitTestVTtagPix(int x)
{
	int chit = -1;				// horizontal cursor hit
	const int jitter = 3;		// jitter allowed: 5 pixels total
	int j = m_VTtags.GetNTags();
	for (int i = 0; i<j; i++)	// loop through all cursors
	{
		int val = m_VTtags.GetTagPix(i);
		if (val <= x+ jitter && val >= x-jitter)
		{
			chit = i;
			break;
		}
	}
	return chit;
}

void CScopeScreen::InvertTracker(CPoint point)
{	
	CClientDC dc(this);						// get dc to fbutton window	
	CBrush* oldBrush = (CBrush*) dc.SelectStockObject(NULL_BRUSH);
	int noldROP = dc.SetROP2(R2_NOTXORPEN);
	CPen* oldPen = dc.SelectObject(&m_blackDottedPen);
	dc.Rectangle(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
	dc.Rectangle(m_ptFirst.x, m_ptFirst.y, point.x, point.y);
	dc.SelectObject(oldPen);				// select old pen

	dc.SelectObject(oldBrush);				// select old brush	
	dc.SetROP2(noldROP);					// select previous draw mode
	m_ptLast = point;						// update m_ptLast
}

void CScopeScreen::DisplayVTtags(CDC* pDC)
{
	// select pen and display mode 
	int noldROP = pDC->SetROP2(R2_NOTXORPEN);
	CPen* oldp =pDC->SelectObject(&m_blackDottedPen);

	// iterate through VT cursor list
	int y0 = MulDiv(0-m_yVO, m_yWE, m_yVE) +m_yWO;
	int y1 = MulDiv(m_displayRect.bottom-m_yVO, m_yWE, m_yVE) +m_yWO;
	for (int j=GetNVTtags()-1; j>=0; j--)
	{
		int k = GetVTtagVal(j);		// get val
		pDC->MoveTo(k,y0);			// set initial pt
		pDC->LineTo(k,y1);			// VT line
	}

	pDC->SelectObject(oldp);
	pDC->SetROP2(noldROP);			// restore old display mode
}

void CScopeScreen::DisplayHZtags(CDC* pDC)
{
	// select pen and display mode 
	CPen* pold = pDC->SelectObject(&m_blackDottedPen);
	int noldROP = pDC->SetROP2(R2_NOTXORPEN);

	// iterate through HZ cursor list		
	int oldval = GetHZtagVal(GetNHZtags()-1)-1;
	for (int i = GetNHZtags()-1; i>= 0; i--)
	{
		int k = GetHZtagVal(i);		// get val
		if (k == oldval)			// skip if already displayed
			continue;
		pDC->MoveTo(m_xWO, k);		// set initial pt
		pDC->LineTo(m_xWE, k);		// HZ line
		oldval = k;
	}
	pDC->SelectObject(pold);
	pDC->SetROP2(noldROP);			// restore old display mode		
}

void CScopeScreen::XorHZtag(int ypoint)
{
	if (m_ptLast.y == ypoint)
		return;
	CClientDC dc(this);

	CPen* pOldPen = dc.SelectObject(&m_blackDottedPen);
	int noldROP = dc.SetROP2(R2_NOTXORPEN);
	dc.IntersectClipRect(&m_displayRect);	// clip drawing inside rectangle
				
	dc.MoveTo(m_displayRect.left,  m_ptLast.y);
	dc.LineTo(m_displayRect.right, m_ptLast.y);
	dc.MoveTo(m_displayRect.left,  ypoint);
	dc.LineTo(m_displayRect.right, ypoint);

	dc.SetROP2(noldROP);
	dc.SelectObject(pOldPen);
	m_ptLast.y = ypoint;
}

void CScopeScreen::XorVTtag(int xpoint)
{
	CClientDC dc(this);
	
	CPen* pOldPen = dc.SelectObject(&m_blackDottedPen);
	int noldROP = dc.SetROP2(R2_NOTXORPEN);    
	dc.IntersectClipRect(&m_clientRect);		// clip drawing inside rect

	dc.MoveTo(m_ptLast.x, m_displayRect.top);
	dc.LineTo(m_ptLast.x, m_displayRect.bottom);

	dc.MoveTo(xpoint, m_displayRect.top);
	dc.LineTo(xpoint, m_displayRect.bottom);

	dc.SetROP2(noldROP);
	dc.SelectObject(pOldPen);
	m_ptLast.x = xpoint;
}

void CScopeScreen::XorTempVTtag(int xpoint)
{
	if (m_tempVTtag == nullptr)
	{
		m_tempVTtag = new CTag;
		m_ptLast.x = -1;
	}
	XorVTtag(xpoint);
	m_tempVTtag->m_pixel = xpoint;
}

CTagList* CScopeScreen::GetHZtagList() 
{
	return (CTagList*) &m_HZtags;
}

CTagList* CScopeScreen::GetVTtagList()
{
	return (CTagList*) &m_VTtags;
}

// bsetPlot:   TRUE=use DIB array to draw curves FALSE: do not use it
// bsetSelect: TRUE=use a separate bitmap to draw selected curve
void CScopeScreen::SetbUseDIB(BOOL bsetPlot)
{
	m_bUseDIB=bsetPlot;
}

int CScopeScreen::GetNHZtags() {return m_HZtags.GetNTags();}

int CScopeScreen::GetNVTtags() {return m_VTtags.GetNTags();}

void CScopeScreen::Serialize( CArchive& ar )
{
	if (ar.IsStoring())
	{
		ar << m_plotmode;
		ar << m_colorbackgr;
		ar << m_colorselected;
		ar << m_xWO;				// x origin, extent / window & view
		ar << m_xWE;
		ar << m_yWO;				// y origin, extent / window & view
		ar << m_yWE;

		ar << m_xVO;
		ar << m_xVE;
		ar << m_yVO;
		ar << m_yVE;

	} 
	else
	{		
		ar >> m_plotmode;
		ar >> m_colorbackgr;
		ar >> m_colorselected;
		ar >> m_xWO;				// x origin, extent / window & view
		ar >> m_xWE;
		ar >> m_yWO;				// y origin, extent / window & view
		ar >> m_yWE;

		ar >> m_xVO;
		ar >> m_xVE;
		ar >> m_yVO;
		ar >> m_yVE;
	}
	m_parms.Serialize(ar);
}

void CScopeScreen::PlotToBitmap(CBitmap* pBitmap)
{
	CClientDC dc(this);
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	memDC.SelectObject(pBitmap);
	PlotDatatoDC(&memDC);
}

