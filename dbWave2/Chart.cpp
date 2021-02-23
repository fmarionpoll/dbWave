#include "StdAfx.h"
#include "resource.h"
#include "chart.h"

//#include "Editctrl.h"
#include "DlgChartProps.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COLORREF CChartWnd::m_colorTable[] =
{
	RGB(0,   0,   0),	// 0 black
	RGB(128,   0, 128),	// 1 purple
	RGB(201, 192, 187),	// 2 silver
	RGB(128,   0,   0),	// 3 maroon
	RGB(196,   2,  51),	// 4 red
	RGB(128, 128, 128),	// 5 grey
	RGB(249, 132, 229),	// 6 fuschia
	RGB(0, 128,   0),	// 7 green
	RGB(91, 255,   0),	// 8 lime
	RGB(107, 142,  35),	// 9 olive
	RGB(255, 205,   0),	// 10 yellow
	RGB(0,   0, 128),	// 11 navy
	RGB(0,   0, 255),	// 12 blue
	RGB(0, 128, 128),	// 13 teal
	RGB(0, 255, 255),	// 14 aqua
	RGB(255, 255, 255),	// 15 white
	RGB(1, 1, 1)		// 16 dummy
};

HCURSOR CChartWnd::m_cursor[NB_CURSORS];
int	CChartWnd::m_cursordragmode[NB_CURSORS];
int CChartWnd::m_countcurs = 0;

TCHAR CChartWnd::csUnit[] = { _T("GM  mµpf  ") };
int  CChartWnd::dUnitsPower[] = { 9,6, 0, 0, -3, -6, -9, -12, 0 };
int	CChartWnd::dmaxIndex = 8;
int	CChartWnd::dniceIntervals[] = { 1, 5, 10,  20,  25,  30,  40, 50, 75, 100, 200, 250, 300, 400, 500, 0 };

int CChartWnd::FindColor(COLORREF ccolor)
{
	auto icolor = -1;
	for (auto i = 0; i < NB_COLORS; i++)
		if (ccolor == m_colorTable[i])
			icolor = i;
	return icolor;
}

int CChartWnd::NiceUnit(float x_val)
{
	auto i = 0;
	const auto ival = int(x_val);
	do
	{
		if (ival <= dniceIntervals[i])
		{
			i++;
			break;
		}
		i++;
	} while (dniceIntervals[i] > 0);
	return dniceIntervals[i - 1];
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

float CChartWnd::ChangeUnit(float xVal, CString* xUnit, float* xScalefactor)
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
	if (xVal < 0)
	{
		isign = -1;
		xVal = -xVal;
	}
	// get power of 10 of the value
	const auto iprec = short(log10(xVal));	// log10 of value (upper limit)
	if ((iprec <= 0) && (xVal < 1.))	// perform extra checking if iprec <= 0
		i = 4 - iprec / 3;					// change equation if Units values change
	else
		i = 3 - iprec / 3;					// change equation if Units values change
	if (i > dmaxIndex)					// clip to max index
		i = dmaxIndex;
	else if (i < 0)						// or clip to min index
		i = 0;
	// return data
	*xScalefactor = float(pow(10.0f, dUnitsPower[i]));		// convert & store
	xUnit->SetAt(0, csUnit[i]);								// replace character corresp to unit
	return xVal * isign / *xScalefactor;						// return value/scale_factor
}

IMPLEMENT_SERIAL(CChartWnd, CWnd, 1)

CChartWnd::CChartWnd()
{
	// load cursors from resources		// #define NB_CURSORS 3
	if (m_countcurs == 0)
	{
		short j = 0;
		m_cursor[j] = ::LoadCursor(nullptr, IDC_ARROW);
		m_cursordragmode[j] = 0;
		j++;
		m_cursor[j] = AfxGetApp()->LoadCursor(IDC_CZOOM);
		m_cursordragmode[j] = 1;
		j++;
		m_cursor[j] = AfxGetApp()->LoadCursor(IDC_CCROSS);
		m_cursordragmode[j] = 1;
		j++;
		m_cursor[j] = AfxGetApp()->LoadCursor(IDC_CCROSS);
		m_cursordragmode[j] = 1;

		j++;
		m_cursor[j] = AfxGetApp()->LoadCursor(IDC_SPLITHORIZONTAL);
		m_cursordragmode[j] = 1;
		j++;
		m_cursor[j] = AfxGetApp()->LoadCursor(IDC_SPLITVERTICAL);
		m_cursordragmode[j] = 1;
	}
	m_countcurs++;

	SetMouseCursor(0);

	m_clientRect = CRect(0, 0, 10, 10);	// minimal size of the button
	AdjustDisplayRect(&m_clientRect);

	m_cxjitter = GetSystemMetrics(SM_CXDOUBLECLK);
	m_cyjitter = GetSystemMetrics(SM_CYDOUBLECLK);
	m_blackDottedPen.CreatePen(PS_DOT, 0, m_colorTable[BLACK_COLOR]);

	// set colored CPen objects
	for (int i = 0; i < NB_COLORS; i++)
		m_penTable[i].CreatePen(PS_SOLID, 0, m_colorTable[i]);

	m_xRuler.m_bHorizontal = TRUE;
	m_yRuler.m_bHorizontal = FALSE;
	m_hFont.CreateFont(12, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_ROMAN, _T("Arial"));
}

CChartWnd::~CChartWnd()
{
	// remove static objects
	m_countcurs--;
	if (m_countcurs == 0)
	{
		for (auto i = 1; i < NB_CURSORS; i++)	// elmt 0: global object cursor
		{
			if (nullptr != m_cursor[i])
				::DestroyCursor(m_cursor[i]);
		}
	}
	m_HZtags.RemoveAllTags();			// remove horizontal tags
	m_VTtags.RemoveAllTags();			// remove vertical tags
	delete m_tempVTtag;

	// delete array of pens
	for (auto& i : m_penTable)
		i.DeleteObject();
}

// trap call to presubclass in order to get source window size..
// assume that palette is present within the application inside CMainFrame...
void CChartWnd::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	// at this stage, assume that m_hWnd is valid
	::GetClientRect(m_hWnd, &m_clientRect);
	AdjustDisplayRect(&m_clientRect);
	m_xVO = m_displayRect.left;
	m_xVE = m_displayRect.Width();
	m_yVO = m_displayRect.Height() / 2;
	m_yVE = -m_displayRect.Height();
}

BEGIN_MESSAGE_MAP(CChartWnd, CWnd)
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

BOOL CChartWnd::Create(LPCTSTR lpszWindowName, DWORD dw_style, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	const auto flag = CWnd::Create(nullptr, lpszWindowName, dw_style, rect, pParentWnd, nID, pContext);
	if (flag != 0)
		PreSubclassWindow();
	return flag;
}

void CChartWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	SetDisplayAreaSize(cx, cy);
	// force DC to redraw
	if (m_PlotDC.GetSafeHdc())
		m_PlotDC.DeleteDC();
	// refresh control
	Invalidate();
}

void CChartWnd::SetDisplayAreaSize(int cx, int cy)
{
	// update coordinates
	m_clientRect.bottom = cy;
	m_clientRect.right = cx;
	AdjustDisplayRect(&m_clientRect);
	m_xVO = m_displayRect.left;
	m_xVE = m_displayRect.Width();
	m_yVO = m_displayRect.Height() / 2;
	m_yVE = -m_displayRect.Height();
}

BOOL CChartWnd::OnEraseBkgnd(CDC* p_dc)
{
	return TRUE; // say we handled it
}

void CChartWnd::PlotToBitmap(CDC* p_dc)
{
	CBitmap bitmap_plot;
	bitmap_plot.CreateBitmap(m_clientRect.right, m_clientRect.bottom, p_dc->GetDeviceCaps(PLANES), p_dc->GetDeviceCaps(BITSPIXEL), nullptr);
	m_PlotDC.CreateCompatibleDC(p_dc);
	const auto pold_plot_bitmap = m_PlotDC.SelectObject(&bitmap_plot);
	PlotDatatoDC(&m_PlotDC);
	p_dc->BitBlt(0, 0, m_displayRect.right, m_displayRect.bottom, &m_PlotDC, 0, 0, SRCCOPY);
	m_PlotDC.SelectObject(pold_plot_bitmap);
}

void CChartWnd::OnPaint()
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

void CChartWnd::PlotDatatoDC(CDC* p_dc)
{
}

void CChartWnd::EraseBkgnd(CDC* p_dc)
{
	// erase background around m_displayRect (assume only left and bottom areas)
	if (m_bNiceGrid)
	{
		auto rect = m_clientRect;
		rect.right = m_displayRect.left;
		p_dc->FillSolidRect(rect, GetSysColor(COLOR_BTNFACE));
		rect.top = m_displayRect.bottom;
		rect.right = m_clientRect.right;
		rect.left = m_displayRect.left;
		p_dc->FillSolidRect(rect, GetSysColor(COLOR_BTNFACE));
	}
	// erase display area
	CBrush brush;
	brush.CreateSolidBrush(m_scopestruct.crScopeFill);
	const auto p_old_brush = p_dc->SelectObject(&brush);
	const auto p_old_pen = (CPen*)p_dc->SelectStockObject(BLACK_PEN);
	p_dc->Rectangle(&m_displayRect);
	p_dc->SelectObject(p_old_pen);
	p_dc->SelectObject(p_old_brush);

	// display grid
	DrawGrid(p_dc);
}

void CChartWnd::DrawGridEvenlySpaced(CDC* p_dc)
{
	auto rect = m_displayRect;
	rect.DeflateRect(1, 1);

	// Standard grid is 8 high by 10 wide
	CPen pen_scale(PS_SOLID, 1, m_scopestruct.crScopeGrid);
	const auto ppen_old = p_dc->SelectObject(&pen_scale);
	const auto i_x_ticks = m_scopestruct.iXCells * m_scopestruct.iXTicks;
	const auto i_y_ticks = m_scopestruct.iYCells * m_scopestruct.iYTicks;
	const auto i_tick_width = 2;
	const auto i_tick_height = 2;

	// do the grid lines
	for (auto i = 1; i < m_scopestruct.iXCells; i++)
	{
		p_dc->MoveTo(i * rect.right / m_scopestruct.iXCells, 0);
		p_dc->LineTo(i * rect.right / m_scopestruct.iXCells, rect.bottom);
	}
	for (auto i = 1; i < m_scopestruct.iYCells; i++)
	{
		p_dc->MoveTo(0, i * rect.bottom / m_scopestruct.iYCells);
		p_dc->LineTo(rect.right, i * rect.bottom / m_scopestruct.iYCells);
	}

	// Put tick marks on the axis lines
	for (auto i = 1; i < i_x_ticks; i++)
	{
		const int y = rect.bottom - rect.bottom * m_scopestruct.iXTickLine / m_scopestruct.iYCells;
		p_dc->MoveTo(i * rect.right / i_x_ticks, y - i_tick_width);
		p_dc->LineTo(i * rect.right / i_x_ticks, y + i_tick_width);
	}
	for (auto i = 1; i < i_y_ticks; i++)
	{
		const int x = rect.right * m_scopestruct.iYTickLine / m_scopestruct.iXCells;
		p_dc->MoveTo(x - i_tick_height, i * rect.bottom / i_y_ticks);
		p_dc->LineTo(x + i_tick_height, i * rect.bottom / i_y_ticks);
	}
	p_dc->SelectObject(ppen_old);

	// if grids, draw scale text (dummy)
	if (m_scopestruct.iXCells > 1 && m_scopestruct.iYCells > 1)
	{
		// convert value into text
		CString cs;
		cs.Format(_T("%.3f mV; %.3f ms"), m_scopestruct.yScaleUnitValue, m_scopestruct.xScaleUnitValue);
		const auto textlen = cs.GetLength();
		// plot text
		p_dc->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
		rect.DeflateRect(1, 1);
		p_dc->SetTextColor(m_scopestruct.crScopeGrid);
		p_dc->DrawText(cs, textlen, rect, DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
	}
}

void CChartWnd::DrawGridFromRuler(CDC* p_dc, CRuler* pRuler)
{
	auto rc_client = m_displayRect;
	rc_client.DeflateRect(1, 1);

	// exit if length is not properly defined
	if (pRuler->m_dlast == pRuler->m_dfirst)
		return;

	CPen a_pen2;
	a_pen2.CreatePen(PS_SOLID, 1, m_scopestruct.crScopeGrid);
	const auto p_old_pen = p_dc->SelectObject(&a_pen2);

	// draw ticks and legends
	//auto tick_small_height = 4;
	int tickmax;
	if (pRuler->m_bHorizontal)		// horizontal
		tickmax = rc_client.Width();
	else							// vertical
		tickmax = rc_client.Height();

	// draw scale
	//double smallscaleinc = pRuler->m_dscaleinc / 5.;
	auto dpos = floor(pRuler->m_dscalefirst);
	const auto dlen = pRuler->m_dlast - pRuler->m_dfirst;
	p_dc->SetBkMode(TRANSPARENT);

	while (dpos <= pRuler->m_dlast)
	{
		int tick_pos;
		if (pRuler->m_bHorizontal)	// horizontal
		{
			tick_pos = int(rc_client.Width() * (dpos - pRuler->m_dfirst) / dlen) + rc_client.left;
			if (tick_pos >= 0 && tick_pos <= tickmax) {
				p_dc->MoveTo(tick_pos, rc_client.bottom - 1);	// line
				p_dc->LineTo(tick_pos, rc_client.top + 1);
			}
		}
		else						// vertical
		{
			tick_pos = int(rc_client.Height() * (pRuler->m_dlast - dpos) / dlen) + rc_client.top;
			if (tick_pos >= 0 && tick_pos <= tickmax) {
				p_dc->MoveTo(rc_client.left + 1, tick_pos);	// line
				p_dc->LineTo(rc_client.right - 1, tick_pos);
			}
		}
		if (dpos != 0. && fabs(dpos) < 1E-10)
			dpos = 0;
		dpos += pRuler->m_dscaleinc;
	}
	// restore objects used in this routine
	p_dc->SelectObject(p_old_pen);
}

void CChartWnd::DrawScalefromRuler(CDC* p_dc, CRuler* pRuler)
{
	auto rc_client = m_displayRect;
	rc_client.DeflateRect(1, 1);

	// exit if length is not properly defined
	if (pRuler->m_dlast == pRuler->m_dfirst)
		return;

	CPen a_pen1;
	CPen a_pen2;
	a_pen1.CreatePen(PS_SOLID, 1, m_scopestruct.crScopeGrid);
	const auto p_old_pen = p_dc->SelectObject(&a_pen1);
	a_pen2.CreatePen(PS_SOLID, 1, m_scopestruct.crScopeGrid);
	/*auto p_old_font = */p_dc->SelectObject(&m_hFont);
	CString str;

	// draw ticks and legends
	const auto tick_small_height = 4;
	int tickmax;
	if (pRuler->m_bHorizontal)		// horizontal
		tickmax = rc_client.Width();
	else							// vertical
		tickmax = rc_client.Height();

	// draw scale
	const auto smallscaleinc = pRuler->m_dscaleinc / 5.;
	auto dpos = floor(pRuler->m_dscalefirst);
	const auto dlen = pRuler->m_dlast - pRuler->m_dfirst;
	p_dc->SetBkMode(TRANSPARENT);

	while (dpos <= pRuler->m_dlast)
	{
		// display small tick marks
		p_dc->SelectObject(&a_pen1);
		auto dsmallpos = dpos;
		int tick_pos;
		for (auto i = 0; i < 4; i++)
		{
			dsmallpos += smallscaleinc;
			if (pRuler->m_bHorizontal) // ---------------------------- horizontal
			{
				tick_pos = int(rc_client.Width() * (dsmallpos - pRuler->m_dfirst) / dlen) + rc_client.left;
				if (tick_pos >= rc_client.left && tick_pos <= tickmax)
				{
					p_dc->MoveTo(tick_pos, rc_client.bottom - 1);
					p_dc->LineTo(tick_pos, rc_client.bottom - tick_small_height);
				}
			}
			else // --------------------------------------------------- vertical
			{
				tick_pos = int(rc_client.Height() * (pRuler->m_dlast - dsmallpos) / dlen) + rc_client.top;
				if (tick_pos >= rc_client.top && tick_pos <= tickmax)
				{
					p_dc->MoveTo(rc_client.left + 1, tick_pos);
					p_dc->LineTo(rc_client.left + tick_small_height, tick_pos);
				}
			}
		}

		// display large ticks and text
		p_dc->SelectObject(&a_pen2);
		if (pRuler->m_bHorizontal)	// horizontal
			tick_pos = int(rc_client.Width() * (dpos - pRuler->m_dfirst) / dlen) + rc_client.left;
		else						// vertical
			tick_pos = int(rc_client.Height() * (pRuler->m_dlast - dpos) / dlen) + rc_client.top;
		if (tick_pos >= 0 && tick_pos <= tickmax)
		{
			str.Format(_T("%g"), dpos);
			const auto size = p_dc->GetTextExtent(str);
			int x, y;
			if (pRuler->m_bHorizontal)	// ----------- horizontal
			{
				p_dc->MoveTo(tick_pos, rc_client.bottom - 1);	// line
				p_dc->LineTo(tick_pos, rc_client.top + 1);
				x = tick_pos - (size.cx / 2);					// text position (upper left)
				if (x < 0)
					x = 0;
				if (x + size.cx > rc_client.right)
					x = rc_client.right - size.cx;
				y = rc_client.bottom + 1; // - 1; //- size.cy
			}
			else // ---------------------------------- vertical
			{
				p_dc->MoveTo(rc_client.left + 1, tick_pos);	// line
				p_dc->LineTo(rc_client.right - 1, tick_pos);
				x = rc_client.left - size.cx - 2;			// text position (upper left)
				y = tick_pos - (size.cy / 2);
			}
			p_dc->TextOut(x, y, str);
		}
		if (dpos != 0. && fabs(dpos) < 1E-10)
			dpos = 0;
		dpos += pRuler->m_dscaleinc;
	}

	// restore objects used in this routine
	p_dc->SelectObject(p_old_pen);
}

void CChartWnd::DrawGridNicelySpaced(CDC* p_dc)
{
	if (m_pXRulerBar == nullptr)
		DrawScalefromRuler(p_dc, &m_xRuler);
	else
	{
		m_pXRulerBar->DrawScalefromRuler(&m_xRuler);
		m_pXRulerBar->Invalidate();
		DrawGridFromRuler(p_dc, &m_xRuler);
	}

	if (m_pYRulerBar == nullptr)
		DrawScalefromRuler(p_dc, &m_yRuler);
	else
	{
		m_pYRulerBar->DrawScalefromRuler(&m_yRuler);
		m_pYRulerBar->Invalidate();
		DrawGridFromRuler(p_dc, &m_yRuler);
	}
}

void CChartWnd::AdjustDisplayRect(CRect* pRect)
{
	m_displayRect = *pRect;
	if (m_bNiceGrid)
	{
		if (m_pYRulerBar == nullptr)
			m_displayRect.left += m_ordinateswidth;
		if (m_pXRulerBar == nullptr)
			m_displayRect.bottom -= m_abcissaheight;
	}
}

void CChartWnd::DrawGrid(CDC* p_dc)
{
	if (m_bNiceGrid)
		DrawGridNicelySpaced(p_dc);
	else
		DrawGridEvenlySpaced(p_dc);
}

void CChartWnd::SetNxScaleCells(int iCells, int iTicks, int iTickLine)
{
	m_scopestruct.iXCells = iCells;
	m_scopestruct.iXTicks = iTicks;
	m_scopestruct.iXTickLine = iTickLine;
}

void CChartWnd::SetNyScaleCells(int iCells, int iTicks, int iTickLine)
{
	m_scopestruct.iYCells = iCells;
	m_scopestruct.iYTicks = iTicks;
	m_scopestruct.iYTickLine = iTickLine;
}

void CChartWnd::SendMyMessage(int code, int codeparm)
{
	GetParent()->SendMessage(WM_MYMESSAGE, code, MAKELONG(codeparm, GetDlgCtrlID()));
}

void CChartWnd::PostMyMessage(int code, int codeparm)
{
	GetParent()->PostMessage(WM_MYMESSAGE, code, MAKELONG(codeparm, GetDlgCtrlID()));
}

void CChartWnd::PrepareDC(CDC* p_dc, CPrintInfo* pInfo)
{
	p_dc->SetMapMode(MM_ANISOTROPIC);
	if (pInfo == nullptr)
	{
		p_dc->SetViewportOrg(m_xVO, m_yVO);
		p_dc->SetViewportExt(m_xVE, m_yVE);
		if (m_yWE == 0)
			m_yWE = 1024;
		p_dc->SetWindowExt(m_xWE, m_yWE);
		p_dc->SetWindowOrg(m_xWO, m_yWO);
	}
}

int CChartWnd::SetMouseCursorType(int cursorm)
{
	m_oldcursorType = m_cursorType;
	ASSERT(NB_CURSORS > cursorm);
	ASSERT(0 <= cursorm);
	m_cursorType = cursorm;
	m_currCursor = m_cursor[m_cursorType];
	m_currCursorMode = m_cursordragmode[m_cursorType];
	return cursorm;
}

void CChartWnd::SetMouseCursor(int cursorm) {
	SetMouseCursorType(cursorm);
	SetCursor(m_currCursor);
}

void CChartWnd::CaptureCursor()
{
	SetCapture();				// capture mouse
	auto rect_limit = m_displayRect;
	ClientToScreen(rect_limit);	// convert coordinates
	ClipCursor(rect_limit);		// tell mouse cursor what are the limits
}

void CChartWnd::ReleaseCursor()
{
	// mouse was captured
	ReleaseCapture();
	ClipCursor(nullptr);
}

BOOL CChartWnd::OnSetCursor(CWnd* p_wnd, UINT nHitTest, UINT message)
{
	::SetCursor(m_currCursor);
	return TRUE;
}

void CChartWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	PostMyMessage(HINT_SETMOUSECURSOR, m_cursorType + 1);
}

void CChartWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_hwndReflect != nullptr)
	{
		// convert coordinates
		CRect rect0, rect1;
		GetWindowRect(&rect1);
		::GetWindowRect(m_hwndReflect, &rect0);

		// reflect mouse move message
		::SendMessage(m_hwndReflect, WM_LBUTTONDOWN, nFlags,
			MAKELPARAM(point.x + (rect1.left - rect0.left),
				point.y + (rect1.top - rect0.top)));
		return;
	}

	m_ptFirst = point;
	m_ptLast = point;
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
		if (m_HCtrapped >= 0)
		{
			m_trackMode = TRACK_HZTAG;
			m_ptLast.x = 0;				// set initial coordinates
			m_ptLast.y = m_HZtags.GetTagPix(m_HCtrapped);
			m_ptFirst = m_ptLast;
			// tell parent that HZtag was selected
			SendMyMessage(HINT_HITHZTAG, m_HCtrapped);
			break;
		}

		// test VT tags - if OK, then track
		if (!m_bVTtagsLONG)
			m_HCtrapped = HitTestVTtagPix(int(point.x));
		else
		{
			m_liJitter = long(m_cxjitter) * (m_liLast - m_liFirst + 1) / long(m_displayRect.Width());
			const auto lx = long(point.x) * (m_liLast - m_liFirst + 1) / long(m_displayRect.Width()) + m_liFirst;
			m_HCtrapped = HitTestVTtagLong(lx);
		}

		// mouse cursor did hit a tag, either horizontal or vertical
		if (m_HCtrapped >= 0)
		{
			m_trackMode = TRACK_VTTAG;
			if (m_bVTtagsLONG)
				m_ptLast.x = int((m_VTtags.GetTagLVal(m_HCtrapped) - m_liFirst) * long(m_displayRect.Width()) / (m_liLast - m_liFirst + 1));
			else
				m_ptLast.x = m_VTtags.GetTagPix(m_HCtrapped);
			m_ptLast.y = 0;
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

	default:
		break;
	}

	// limit the tracking to the client area of the view
	CaptureCursor();
	return;
}

void CChartWnd::OnMouseMove(UINT nFlags, CPoint point)
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
			const auto val = MulDiv(point.y - m_yVO, m_yWE, m_yVE) + m_yWO;
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
				const auto val = MulDiv(point.x - m_xVO, m_xWE, m_xVE) + m_xWO;
				m_VTtags.SetTagVal(m_HCtrapped, val);
			}
			else
			{
				const auto lval = long(point.x) * (m_liLast - m_liFirst + 1) / long(m_displayRect.Width()) + m_liFirst;
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
			::GetWindowRect(m_hwndReflect, &rect0);

			// reflect mouse move message
			::SendMessage(m_hwndReflect,
				WM_MOUSEMOVE,
				nFlags,
				MAKELPARAM(point.x + (rect1.left - rect0.left),
					point.y + (rect1.top - rect0.top)));
		}
		//else
		//{
		//	CRect rect;
		//	GetWindowRect(rect);
		//	if ((point.x > (rect.Width() - (SPLITSIZE + TRACKSIZE))) || (point.x < (SPLITSIZE + TRACKSIZE)))
		//		SetMouseCursor(CURSOR_RESIZE_HZ);
		//	else if ((point.y > (rect.Height() - (SPLITSIZE + TRACKSIZE +12))) || (point.y < (SPLITSIZE + TRACKSIZE +12)))
		//		SetMouseCursor(CURSOR_RESIZE_VERT);
		//	else
		//		SetMouseCursor(CURSOR_ARROW);
		//}
		break;
	}
}

void CChartWnd::OnLButtonUp(UINT nFlags, CPoint point)
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
		::GetWindowRect(m_hwndReflect, &rect0);

		// reflect mouse move message
		::SendMessage(m_hwndReflect, WM_LBUTTONUP, nFlags,
			MAKELPARAM(point.x + (rect1.left - rect0.left),
				point.y + (rect1.top - rect0.top)));
	}
	m_bLmouseDown = FALSE;
}

void CChartWnd::OnRButtonDown(UINT nFlags, CPoint point)
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

void CChartWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	switch (m_trackMode)
	{
	case TRACK_RECT:
	{
		ReleaseCursor();
		// skip too small a rectangle (5 pixels?)
		CRect rect_out(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
		const short jitter = 3;
		if (rect_out.Height() < jitter && rect_out.Width() < jitter)
		{
			ZoomOut();
		}
		else
		{
			auto rect_in = m_displayRect;
			ZoomData(&rect_out, &rect_in);
			m_ZoomFrom = rect_out;
			m_ZoomTo = rect_in;
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
			const auto parms_old = new SCOPESTRUCT();
			*parms_old = m_scopestruct;
			CDlgChartProps dlg;
			dlg.m_pscope = this;
			m_bAllowProps = FALSE;		// inhibit properties

			// if Cancel or Escape or anything else: restore previous values
			if (IDOK != dlg.DoModal())
			{
				m_scopestruct = *parms_old;
				Invalidate();
			}
			else
				PostMyMessage(HINT_WINDOWPROPSCHANGED, NULL);
			m_bAllowProps = TRUE;
		}
		break;

	default:
		ReleaseCursor();
		ZoomOut();
		break;
	}
	m_trackMode = TRACK_OFF;
}

void CChartWnd::ZoomData(CRect* prevRect, CRect* newRect)
{
}

void CChartWnd::ZoomPop()
{
	ZoomData(&m_ZoomTo, &m_ZoomFrom);
	m_iUndoZoom = 0;
}

void CChartWnd::ZoomOut()
{
	if (m_iUndoZoom > 0) // memory?
		ZoomPop();
	else
	{
		CClientDC dc(this);
		m_ZoomTo = m_displayRect;
		m_ZoomFrom = m_ZoomTo;
		const auto yshrink = m_ZoomTo.Height() / 4;
		const auto xshrink = m_ZoomTo.Width() / 4;
		m_ZoomTo.InflateRect(xshrink, yshrink);
		ZoomData(&m_ZoomFrom, &m_ZoomTo);
		m_iUndoZoom = -1;
	}
}

void CChartWnd::ZoomIn()
{
	if (m_iUndoZoom < 0) // memory?
		ZoomPop();
	else
	{
		CClientDC dc(this);
		m_ZoomTo = m_displayRect;
		m_ZoomFrom = m_ZoomTo;
		const auto yshrink = -m_ZoomTo.Height() / 4;
		const auto xshrink = -m_ZoomTo.Width() / 4;
		m_ZoomTo.InflateRect(xshrink, yshrink);
		ZoomData(&m_ZoomFrom, &m_ZoomTo);
		m_iUndoZoom = 1;
	}
}

int CChartWnd::HitTestHZtag(int y)
{
	auto chit = -1;				// horizontal cursor hit
	const auto jitter = 3;		// jitter allowed: 5 pixels total
	const auto j = m_HZtags.GetNTags();
	for (auto i = 0; i < j; i++)	// loop through all cursors
	{
		const auto val = m_HZtags.GetTagPix(i);	// get pixel value
		if (val <= y + jitter && val >= y - jitter)
		{
			chit = i;
			break;
		}
	}
	return chit;
}

int CChartWnd::HitTestVTtagLong(long lx)
{
	auto chit = -1;				// horizontal cursor hit
	const auto j = m_VTtags.GetNTags();
	for (auto i = 0; i < j; i++)	// loop through all cursors
	{
		const auto lval = m_VTtags.GetTagLVal(i);
		if (lval <= lx + m_liJitter && lval >= lx - m_liJitter)
		{
			chit = i;
			break;
		}
	}
	return chit;
}

int CChartWnd::HitTestVTtagPix(int x)
{
	auto chit = -1;				// horizontal cursor hit
	const auto jitter = 3;		// jitter allowed: 5 pixels total
	const auto j = m_VTtags.GetNTags();
	for (auto i = 0; i < j; i++)	// loop through all cursors
	{
		const auto val = m_VTtags.GetTagPix(i);
		if (val <= x + jitter && val >= x - jitter)
		{
			chit = i;
			break;
		}
	}
	return chit;
}

void CChartWnd::InvertTracker(CPoint point)
{
	CClientDC dc(this);						// get dc to fbutton window
	const auto old_brush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
	const auto nold_rop = dc.SetROP2(R2_NOTXORPEN);
	const auto old_pen = dc.SelectObject(&m_blackDottedPen);
	dc.Rectangle(m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y);
	dc.Rectangle(m_ptFirst.x, m_ptFirst.y, point.x, point.y);
	dc.SelectObject(old_pen);				// select old pen

	dc.SelectObject(old_brush);				// select old brush
	dc.SetROP2(nold_rop);					// select previous draw mode
	m_ptLast = point;						// update m_ptLast
}

void CChartWnd::DisplayVTtags_Value(CDC* p_dc)
{
	const auto oldp = p_dc->SelectObject(&m_blackDottedPen);
	const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);
	const auto y0 = MulDiv(0 - m_yVO, m_yWE, m_yVE) + m_yWO;
	const auto y1 = MulDiv(m_displayRect.bottom - m_yVO, m_yWE, m_yVE) + m_yWO;

	for (auto j = GetNVTtags() - 1; j >= 0; j--)
	{
		const auto k = GetVTtagVal(j);	
		p_dc->MoveTo(k, y0);
		p_dc->LineTo(k, y1);
	}

	p_dc->SelectObject(oldp);
	p_dc->SetROP2(nold_rop);
}

void CChartWnd::DisplayHZtags(CDC* p_dc)
{
	// select pen and display mode
	const auto pold = p_dc->SelectObject(&m_blackDottedPen);
	const auto nold_rop = p_dc->SetROP2(R2_NOTXORPEN);

	// iterate through HZ cursor list
	auto oldval = GetHZtagVal(GetNHZtags() - 1) - 1;
	for (auto i = GetNHZtags() - 1; i >= 0; i--)
	{
		const auto k = GetHZtagVal(i);		// get val
		if (k == oldval)					// skip if already displayed
			continue;
		p_dc->MoveTo(m_xWO, k);	
		p_dc->LineTo(m_xWE, k);
		oldval = k;
	}
	p_dc->SelectObject(pold);
	p_dc->SetROP2(nold_rop);			// restore old display mode
}

void CChartWnd::XorHZtag(int ypoint)
{
	if (m_ptLast.y == ypoint)
		return;
	CClientDC dc(this);

	const auto p_old_pen = dc.SelectObject(&m_blackDottedPen);
	const auto nold_rop = dc.SetROP2(R2_NOTXORPEN);
	dc.IntersectClipRect(&m_displayRect);	// clip drawing inside rectangle

	dc.MoveTo(m_displayRect.left, m_ptLast.y);
	dc.LineTo(m_displayRect.right, m_ptLast.y);
	dc.MoveTo(m_displayRect.left, ypoint);
	dc.LineTo(m_displayRect.right, ypoint);

	dc.SetROP2(nold_rop);
	dc.SelectObject(p_old_pen);
	m_ptLast.y = ypoint;
}

void CChartWnd::XorVTtag(int xpoint)
{
	CClientDC dc(this);

	const auto p_old_pen = dc.SelectObject(&m_blackDottedPen);
	const auto nold_rop = dc.SetROP2(R2_NOTXORPEN);
	dc.IntersectClipRect(&m_clientRect);		// clip drawing inside rect

	dc.MoveTo(m_ptLast.x, m_displayRect.top);
	dc.LineTo(m_ptLast.x, m_displayRect.bottom);

	dc.MoveTo(xpoint, m_displayRect.top);
	dc.LineTo(xpoint, m_displayRect.bottom);

	dc.SetROP2(nold_rop);
	dc.SelectObject(p_old_pen);
	m_ptLast.x = xpoint;
}

void CChartWnd::XorTempVTtag(int xpoint)
{
	if (m_tempVTtag == nullptr)
	{
		m_tempVTtag = new CTag;
		m_ptLast.x = -1;
	}
	XorVTtag(xpoint);
	m_tempVTtag->m_pixel = xpoint;
}

CTagList* CChartWnd::GetHZtagList()
{
	return &m_HZtags;
}

CTagList* CChartWnd::GetVTtagList()
{
	return &m_VTtags;
}

SCOPESTRUCT* CChartWnd::GetScopeParameters()
{
	return &m_scopestruct;
}

void CChartWnd::SetScopeParameters(SCOPESTRUCT* pStruct)
{
	m_scopestruct = *pStruct;
}

// bsetPlot:   TRUE=use DIB array to draw curves FALSE: do not use it
// bsetSelect: TRUE=use a separate bitmap to draw selected curve
void CChartWnd::SetbUseDIB(BOOL bsetPlot)
{
	m_bUseDIB = bsetPlot;
}

int CChartWnd::GetNHZtags() { return m_HZtags.GetNTags(); }

int CChartWnd::GetNVTtags() { return m_VTtags.GetNTags(); }

void CChartWnd::Serialize(CArchive& ar)
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
	m_scopestruct.Serialize(ar);
}

void CChartWnd::PlotToBitmap(CBitmap* pBitmap)
{
	CClientDC dc(this);
	CDC mem_dc;
	mem_dc.CreateCompatibleDC(&dc);
	mem_dc.SelectObject(pBitmap);
	PlotDatatoDC(&mem_dc);
}