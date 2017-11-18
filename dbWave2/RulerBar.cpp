// RulerBar.cpp : implementation file
//

#include "stdafx.h"
#include "RulerBar.h"
#include ".\rulerbar.h"

#include <math.h>
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CRuler -------------------------------------------------------------------- lower/upper

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
	m_dscaleinc = pow((double) 10, ceil (log10(range/10)));

	// set max scale
	m_dscalelast = ((long) (m_dlast / m_dscaleinc)) * m_dscaleinc;
	if (m_dscalelast < m_dlast)
		m_dscalelast += m_dscaleinc;
	// set min scale
	m_dscalefirst = m_dscalelast;
	int i = 0;
	do {
		++i;
		m_dscalefirst -= m_dscaleinc;
	}
	while (m_dscalefirst > m_dfirst);

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
	m_bHorizontal = TRUE;
}

CRulerBar::~CRulerBar()
{
}

BEGIN_MESSAGE_MAP(CRulerBar, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CRulerBar message handlers

void CRulerBar::OnPaint()
{
	CPaintDC dc(this);
	CRect rcClient;
	GetClientRect(&rcClient);
	// exit if the length is not properly defined
	if (m_ruler.m_dlast == m_ruler.m_dfirst)
		return;

	CPen aPen;
	aPen.CreatePen(PS_SOLID, 0, m_penColor);
	CPen* pOldPen = dc.SelectObject(&aPen);	
	CFont* pOldFont= dc.SelectObject(&m_hFont);
	CString str;

	// draw ticks and legends
	int tickBigHeight, tickSmallHeight;
	if (!m_bHorizontal)
	{
		tickBigHeight = rcClient.Width() /2 -2;	
		tickSmallHeight = tickBigHeight/2;	
	}
	else
	{
		tickBigHeight = rcClient.Height() /2 -2;
		tickSmallHeight = tickBigHeight/2;
	}

	// draw solid background
	dc.IntersectClipRect(rcClient);	
	dc.FillSolidRect(rcClient, ::GetSysColor(COLOR_3DFACE));

	// draw baseline on the right side
	if (!m_bHorizontal)
	{
		dc.MoveTo(rcClient.right-1, rcClient.top);
		dc.LineTo(rcClient.right-1, rcClient.bottom);
	}
	else
	{
		dc.MoveTo(rcClient.left, rcClient.top);
		dc.LineTo(rcClient.right, rcClient.top);
	}

	// draw scale
	double dpos = floor(m_ruler.m_dscalefirst); // floor(m_ruler.m_dfirst);
	double dlen = m_ruler.m_dlast - m_ruler.m_dfirst;
	double smallscaleinc = m_ruler.m_dscaleinc / 5.;
	dc.SetBkMode(TRANSPARENT);

	while (dpos <= m_ruler.m_dlast)									// =
	{
		// display small ticks
		double dsmallpos = dpos;									// =
		int tickPos;
		for (int i= 0; i<4; i++)									// =
		{
			dsmallpos += smallscaleinc;								// =
			double ratio = (m_ruler.m_dlast - dsmallpos) / dlen;	// =
			if (!m_bHorizontal)
			{
				tickPos = (int)(rcClient.Height() * (m_ruler.m_dlast - dsmallpos) / dlen); 
				dc.MoveTo(rcClient.right, tickPos);
				dc.LineTo(rcClient.right -tickSmallHeight, tickPos);
			}
			else
			{
				tickPos = (int) (rcClient.Width() * (dsmallpos - m_ruler.m_dfirst) / dlen);
				dc.MoveTo(tickPos, rcClient.top);
				dc.LineTo(tickPos, rcClient.top+tickSmallHeight);
			}
		}

		// display large ticks and text
		if (!m_bHorizontal)
			tickPos = (int) (rcClient.Height() * (m_ruler.m_dlast - dpos) / dlen);
		else
			tickPos = (int) (rcClient.Width() * (dpos - m_ruler.m_dfirst) / dlen);

		if (tickPos >= 0)											// =
		{
			str.Format(_T("%g"), dpos);								// =
			CSize size = dc.GetTextExtent(str);						// =
			int x, y;
			if (!m_bHorizontal)
			{
				// tick
				dc.MoveTo(rcClient.right, tickPos);
				dc.LineTo(rcClient.right - tickBigHeight, tickPos);
				// text
				if( dpos != 0. && fabs(dpos) < 1E-10 )				// prevent "bad" zero
				   dpos = 0 ;
				x = rcClient.right - tickBigHeight - size.cx -2;
				y = tickPos - (size.cy/2);
			}
			else
			{
				dc.MoveTo(tickPos, rcClient.top);
				dc.LineTo(tickPos, rcClient.top+tickBigHeight);
				x = tickPos - (size.cx/2);
				if (x < 0) 
					x = 0;
				if (x + size.cx > rcClient.right)
					x = rcClient.right - size.cx;
				y = rcClient.top+tickBigHeight +1;
			}
			dc.TextOut(x, y, str);									// = 
		}
		dpos += m_ruler.m_dscaleinc;								// =
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
}

void CRulerBar::PreSubclassWindow()
{
	if (IsWindow(m_hWnd))
	{
		CRect rect;
		GetClientRect(&rect);
		if (rect.Width()  < rect.Height() )
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

int	CRulerBar::GetScaleUnitPixels() 
{
	if (!IsWindow(m_hWnd))
		return 0;
	CRect rect;
	GetClientRect(rect);
	if (m_bHorizontal)
		return (int) (m_ruler.m_dscaleinc*rect.right / (m_ruler.m_dlast - m_ruler.m_dfirst));
	else
		return (int) (m_ruler.m_dscaleinc*rect.bottom / (m_ruler.m_dlast - m_ruler.m_dfirst));
}

void CRulerBar::SetRange(float* dfirst, float* dlast)
{
	m_ruler.SetRange(dfirst, dlast);
	Invalidate();
}




