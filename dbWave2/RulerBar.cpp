#include "StdAfx.h"
#include "ChartData.h"
#include "Ruler.h"
#include "RulerBar.h"
#include <math.h>
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CRulerBar, CWnd)

CRulerBar::CRulerBar()
{
	m_hFont.CreateFont(12, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY,
	                   VARIABLE_PITCH | FF_ROMAN, _T("Arial"));
}

CRulerBar::~CRulerBar()
{
}

BEGIN_MESSAGE_MAP(CRulerBar, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CRulerBar message handlers

void CRulerBar::OnPaint()
{
	DrawScalefromRuler(nullptr);
}

void CRulerBar::DrawScalefromRuler(CRuler* pRuler)
{
	CPaintDC dc(this);

	// exit if the length is not properly defined
	if (pRuler == nullptr)
	{
		if (m_pRuler != nullptr)
			pRuler = m_pRuler;
		else if (m_pChartDataWnd != nullptr)
		{
			if (m_rcClient.Height() > m_rcClient.Width())
				m_pRuler = &m_pChartDataWnd->m_yRuler;
			else
				m_pRuler = &m_pChartDataWnd->m_xRuler;
			pRuler = m_pRuler;
		}
		else
			return;
	}
	if (pRuler->m_dlast == pRuler->m_dfirst)
	{
		return;
	}

	CPen a_pen;
	a_pen.CreatePen(PS_SOLID, 0, m_penColor);
	const auto p_old_pen = dc.SelectObject(&a_pen);
	const auto p_old_font = dc.SelectObject(&m_hFont);
	CString str;

	// draw ticks and legends
	int tick_big_height;
	if (!m_bHorizontal)
		tick_big_height = m_rcClient.Width() / 2 - 2;
	else
		tick_big_height = m_rcClient.Height() / 2 - 2;
	const int tick_small_height = tick_big_height / 2;

	// draw solid background
	dc.IntersectClipRect(m_rcClient);
	dc.FillSolidRect(m_rcClient, GetSysColor(COLOR_3DFACE));

	// draw baseline on the right side
	if (!m_bHorizontal)
	{
		dc.MoveTo(m_rcClient.right - 1, m_rcClient.top);
		dc.LineTo(m_rcClient.right - 1, m_rcClient.bottom);
	}
	else
	{
		dc.MoveTo(m_rcClient.left, m_rcClient.top);
		dc.LineTo(m_rcClient.right, m_rcClient.top);
	}

	// draw scale
	auto dpos = floor(pRuler->m_dscalefirst);
	const auto dlen = pRuler->m_dlast - pRuler->m_dfirst;
	const auto smallscaleinc = pRuler->m_dscaleinc / 5.;
	dc.SetBkMode(TRANSPARENT);

	while (dpos <= pRuler->m_dlast)
	{
		// display small ticks
		auto dsmallpos = dpos;
		int tick_pos;
		for (auto i = 0; i < 4; i++)
		{
			dsmallpos += smallscaleinc;
			auto ratio = (pRuler->m_dlast - dsmallpos) / dlen;
			if (!m_bHorizontal)
			{
				tick_pos = static_cast<int>(m_rcClient.Height() * (pRuler->m_dlast - dsmallpos) / dlen);
				dc.MoveTo(m_rcClient.right, tick_pos);
				dc.LineTo(m_rcClient.right - tick_small_height, tick_pos);
			}
			else
			{
				tick_pos = static_cast<int>(m_rcClient.Width() * (dsmallpos - pRuler->m_dfirst) / dlen);
				dc.MoveTo(tick_pos, m_rcClient.top);
				dc.LineTo(tick_pos, m_rcClient.top + tick_small_height);
			}
		}

		// display large ticks and text
		if (!m_bHorizontal)
			tick_pos = static_cast<int>(m_rcClient.Height() * (pRuler->m_dlast - dpos) / dlen);
		else
			tick_pos = static_cast<int>(m_rcClient.Width() * (dpos - pRuler->m_dfirst) / dlen);

		if (tick_pos >= 0)
		{
			str.Format(_T("%g"), dpos);
			auto size = dc.GetTextExtent(str);
			int x, y;
			if (!m_bHorizontal)
			{
				// tick
				dc.MoveTo(m_rcClient.right, tick_pos);
				dc.LineTo(m_rcClient.right - tick_big_height, tick_pos);
				// text
				if (dpos != 0. && fabs(dpos) < 1E-10) // prevents "bad" zero 
				{
					dpos = 0;
					str.Format(_T("%g"), dpos);
					size = dc.GetTextExtent(str);
				}
				x = m_rcClient.right - tick_big_height - size.cx - 2;
				y = tick_pos - (size.cy / 2);
			}
			else
			{
				dc.MoveTo(tick_pos, m_rcClient.top);
				dc.LineTo(tick_pos, m_rcClient.top + tick_big_height);
				x = tick_pos - (size.cx / 2);
				if (x < 0)
					x = 0;
				if (x + size.cx > m_rcClient.right)
					x = m_rcClient.right - size.cx;
				y = m_rcClient.top + tick_big_height + 1;
			}
			dc.TextOut(x, y, str);
		}
		dpos += pRuler->m_dscaleinc;
	}

	// restore objects used in this routine
	dc.SelectObject(p_old_pen);
	dc.SelectObject(p_old_font);
}

BOOL CRulerBar::OnEraseBkgnd(CDC* p_dc)
{
	return TRUE;
	//return CWnd::OnEraseBkgnd(p_dc);
}

void CRulerBar::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	GetClientRect(&m_rcClient);
}

void CRulerBar::PreSubclassWindow()
{
	if (IsWindow(m_hWnd))
	{
		GetClientRect(&m_rcClient);
		if (m_rcClient.Width() < m_rcClient.Height())
			m_bHorizontal = FALSE;
	}
	CWnd::PreSubclassWindow();
}

BOOL CRulerBar::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dw_style, const RECT& rect,
                       CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	if ((rect.right - rect.left) < (rect.bottom - rect.top))
		m_bHorizontal = FALSE;
	return CWnd::Create(nullptr, _T("RulerBarWnd"), dw_style, rect, pParentWnd, nID);
}

BOOL CRulerBar::Create(CWnd* pParentWnd, ChartData* pDataChartWnd, BOOL bAsXAxis, int dSize, UINT nID)
{
	m_pChartDataWnd = pDataChartWnd;
	CRect rect;
	pDataChartWnd->GetWindowRect(&rect);
	auto rectthis = rect;
	if (bAsXAxis)
	{
		rectthis.top = rect.bottom - 1;
		rectthis.bottom = rectthis.top + dSize;
		m_bHorizontal = TRUE;
	}
	else
	{
		rectthis.right = rect.left - 1;
		rectthis.left = rectthis.right - dSize;
		m_bHorizontal = FALSE;
	}
	return CWnd::Create(_T("RulerBarWnd"), nullptr, WS_CHILD, rectthis, pParentWnd, nID);
}

#define MODEZOOM	0
#define MODEBIAS	1
#define DELTA		10

void CRulerBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bHorizontal && (point.y < DELTA || point.y > m_rcClient.Height() - DELTA))
		SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITVERTICAL));
	else if (m_bHorizontal && (point.x < DELTA || point.x > m_rcClient.Width() - DELTA))
		SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITHORIZONTAL));

	CWnd::OnMouseMove(nFlags, point);
}

void CRulerBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_bHorizontal && (point.y < DELTA || point.y > m_rcClient.Height() - DELTA))
	{
		m_captureMode = MODEZOOM;
		SetCapture();
		m_bCaptured = TRUE;
		oldpt = point;
		m_bBottom = (point.y < DELTA);
	}
	else if (m_bHorizontal && (point.x < DELTA || point.x > m_rcClient.Width() - DELTA))
	{
		m_captureMode = MODEZOOM;
		SetCapture();
		m_bCaptured = TRUE;
		oldpt = point;
		m_bBottom = (point.x < DELTA);
	}
	else
		CWnd::OnLButtonDown(nFlags, point);
}

void CRulerBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bCaptured)
	{
		m_bCaptured = FALSE;
		m_captureMode = 0;
		ReleaseCapture();
		newpt = point;
		const int delta = -(newpt.y - oldpt.y);
		if (m_pChartDataWnd != nullptr)
		{
			CRect prevrect;
			m_pChartDataWnd->GetClientRect(prevrect);
			auto newrect = prevrect;
			if (!m_bHorizontal)
			{
				if (m_bBottom)
					newrect.bottom -= delta;
				else
					newrect.top += delta;
				m_pChartDataWnd->ZoomData(&prevrect, &newrect);
				m_pChartDataWnd->Invalidate();
			}
			else
			{
				if (m_bBottom)
					newrect.left -= delta;
				else
					newrect.right += delta;
			}
		}
	}
	else
		CWnd::OnLButtonUp(nFlags, point);
}
