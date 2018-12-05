// RulerBar.cpp : implementation file
//

#include "stdafx.h"
#include "lineview.h"
#include "RulerBar.h"
#include ".\rulerbar.h"

#include <math.h>
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CRuler -------------------------------------------------------------------- lower/upper
CRuler::CRuler()
{
	m_bHorizontal = TRUE;
	m_dfirst = 0.;
	m_dlast = 0.;
	m_dscaleinc = 0.;
	m_dscalefirst = 0.;
	m_dscalelast = 0.;
}

CRuler::~CRuler()
{
}

void CRuler::SetRange (float* dfirst, float* dlast)
{
	m_dfirst = *dfirst;
	m_dlast  = *dlast;
	if (m_dfirst > m_dlast)
	{
		double x = m_dfirst;
		m_dfirst = m_dlast;
		m_dlast = x;
	}
	AdjustScale();
}

void CRuler::UpdateRange(float * dfirst, float * dlast)
{
	if (*dfirst != m_dfirst || *dlast != m_dlast)
		SetRange(dfirst, dlast);
}

BOOL CRuler::AdjustScale()
{
	// cf Bramley M. (2000) Data-Based Axis Determination. C/C++ Users Journal 18(7) 20-24
	// http://drdobbs.com/184401258

	double range = m_dlast - m_dfirst;

	// deal with repeated values
	if (range == 0)
	{
		m_dfirst = ceil (m_dlast) - 1;
		m_dlast = m_dfirst + 1;
		m_dscaleinc = 1;
		return TRUE;
	}

	// compute increment
	double exponent = ceil(log10(range / 10));
	m_dscaleinc = pow(10, exponent);

	// set min scale
	m_dscalefirst = ((long) (m_dfirst / m_dscaleinc)) * m_dscaleinc;
	if (m_dscalefirst > m_dfirst)
		m_dscalefirst -= m_dscaleinc;
	// set max scale
	m_dscalelast = m_dscalefirst;
	int i = 0;
	do {
		++i;
		m_dscalelast += m_dscaleinc;
	}
	while (m_dscalelast < m_dlast);

	// adjust for too few tickmarks
	int iTOOFEW = 5;
	if (i < iTOOFEW)
	{
		m_dscaleinc /= 2;
		if ((m_dscalefirst + m_dscaleinc) <= m_dfirst)
			m_dscalefirst += m_dscaleinc;
		if ((m_dscalelast - m_dscaleinc) >= m_dlast)
			m_dscalelast -= m_dscaleinc;
	}
	return TRUE;
}

// -------------------------------------------------------------------------
// CRulerBar

IMPLEMENT_DYNAMIC(CRulerBar, CWnd)

CRulerBar::CRulerBar()
{
	m_penColor = ::GetSysColor(COLOR_WINDOWTEXT);
	m_hFont.CreateFont(12, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH|FF_ROMAN, _T("Arial"));
	m_bHorizontal = -1;
	m_pLineViewWnd=NULL;
	m_pRuler = NULL;
	m_bCaptured = FALSE;
	m_captureMode = -1;
	m_bBottom = true;
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
	DrawScalefromRuler(NULL);
}

void CRulerBar::DrawScalefromRuler(CRuler * pRuler)
{
	CPaintDC dc(this);

	// exit if the length is not properly defined
	if (pRuler == NULL)
	{
		if (m_pRuler != NULL)
			pRuler = m_pRuler;
		else if (m_pLineViewWnd != NULL)
		{
			if (m_rcClient.Height() > m_rcClient.Width())
				m_pRuler = &m_pLineViewWnd->m_yRuler;
			else
				m_pRuler = &m_pLineViewWnd->m_xRuler;
			pRuler = m_pRuler;
		}
		else
			return;
	}
	if (pRuler->m_dlast == pRuler->m_dfirst)
	{
		return;
	}

	CPen aPen;
	aPen.CreatePen(PS_SOLID, 0, m_penColor);
	CPen* pOldPen = dc.SelectObject(&aPen);	
	CFont* pOldFont= dc.SelectObject(&m_hFont);
	CString str;

	// draw ticks and legends
	int tickBigHeight, tickSmallHeight;
	if (!m_bHorizontal)
		tickBigHeight = m_rcClient.Width() /2 -2;
	else
		tickBigHeight = m_rcClient.Height() /2 -2;
	tickSmallHeight = tickBigHeight / 2;

	// draw solid background
	dc.IntersectClipRect(m_rcClient);
	dc.FillSolidRect(m_rcClient, ::GetSysColor(COLOR_3DFACE));

	// draw baseline on the right side
	if (!m_bHorizontal)
	{
		dc.MoveTo(m_rcClient.right-1, m_rcClient.top);
		dc.LineTo(m_rcClient.right-1, m_rcClient.bottom);
	}
	else
	{
		dc.MoveTo(m_rcClient.left, m_rcClient.top);
		dc.LineTo(m_rcClient.right, m_rcClient.top);
	}

	// draw scale
	double dpos = floor(pRuler->m_dscalefirst); 
	double dlen = pRuler->m_dlast - pRuler->m_dfirst;
	double smallscaleinc = pRuler->m_dscaleinc / 5.;
	dc.SetBkMode(TRANSPARENT);

	while (dpos <= pRuler->m_dlast)									// =
	{
		// display small ticks
		double dsmallpos = dpos;									// =
		int tickPos;
		for (int i= 0; i<4; i++)									// =
		{
			dsmallpos += smallscaleinc;								// =
			double ratio = (pRuler->m_dlast - dsmallpos) / dlen;	// =
			if (!m_bHorizontal)
			{
				tickPos = (int)(m_rcClient.Height() * (pRuler->m_dlast - dsmallpos) / dlen);
				dc.MoveTo(m_rcClient.right, tickPos);
				dc.LineTo(m_rcClient.right -tickSmallHeight, tickPos);
			}
			else
			{
				tickPos = (int) (m_rcClient.Width() * (dsmallpos - pRuler->m_dfirst) / dlen);
				dc.MoveTo(tickPos, m_rcClient.top);
				dc.LineTo(tickPos, m_rcClient.top+tickSmallHeight);
			}
		}

		// display large ticks and text
		if (!m_bHorizontal)
			tickPos = (int) (m_rcClient.Height() * (pRuler->m_dlast - dpos) / dlen);
		else
			tickPos = (int) (m_rcClient.Width() * (dpos - pRuler->m_dfirst) / dlen);

		if (tickPos >= 0)											// =
		{
			str.Format(_T("%g"), dpos);								// =
			CSize size = dc.GetTextExtent(str);						// =
			int x, y;
			if (!m_bHorizontal)
			{
				// tick
				dc.MoveTo(m_rcClient.right, tickPos);
				dc.LineTo(m_rcClient.right - tickBigHeight, tickPos);
				// text
				if( dpos != 0. && fabs(dpos) < 1E-10 )				// prevent "bad" zero
				   dpos = 0 ;
				x = m_rcClient.right - tickBigHeight - size.cx -2;
				y = tickPos - (size.cy/2);
			}
			else
			{
				dc.MoveTo(tickPos, m_rcClient.top);
				dc.LineTo(tickPos, m_rcClient.top+tickBigHeight);
				x = tickPos - (size.cx/2);
				if (x < 0) 
					x = 0;
				if (x + size.cx > m_rcClient.right)
					x = m_rcClient.right - size.cx;
				y = m_rcClient.top+tickBigHeight +1;
			}
			dc.TextOut(x, y, str);									// = 
		}
		dpos += pRuler->m_dscaleinc;								// =
	}

	// restore objects used in this routine
	dc.SelectObject(pOldPen);										// =
	dc.SelectObject(pOldFont);										// =
}

BOOL CRulerBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CWnd::OnEraseBkgnd(pDC);
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
		if (m_rcClient.Width()  < m_rcClient.Height() )
			m_bHorizontal = FALSE;
	}
	CWnd::PreSubclassWindow();
}

BOOL CRulerBar::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	if ((rect.right - rect.left)  < (rect.bottom - rect.top))
		m_bHorizontal = FALSE;
	return CWnd::Create(NULL, _T("RulerBarWnd"), dwStyle, rect, pParentWnd, nID);
}

BOOL CRulerBar::Create(CWnd* pParentWnd, CLineViewWnd* pLineViewWnd, BOOL bAsXAxis, int dSize, UINT nID)
{
	m_pLineViewWnd = pLineViewWnd;
	CRect rect;
	pLineViewWnd->GetWindowRect(&rect);
	CRect rectthis = rect;
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
	return CWnd::Create(_T("RulerBarWnd"), NULL, WS_CHILD, rectthis, pParentWnd, nID);
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
		int delta = -(newpt.y - oldpt.y);
		if (m_pLineViewWnd != NULL)
		{
			CRect prevrect;
			m_pLineViewWnd->GetClientRect(prevrect);
			CRect newrect = prevrect;
			if (!m_bHorizontal) {
				if (m_bBottom)
					newrect.bottom -= delta;
				else
					newrect.top += delta;
				m_pLineViewWnd->ZoomData(&prevrect, &newrect);
				m_pLineViewWnd->Invalidate();
			}
			else {
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
