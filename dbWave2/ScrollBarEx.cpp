// ScrollBarEx.cpp : implementation file
// extended scrollbar - catches mouse when it passes over the left and right bar
// of the scroll button (HZ bar)

#include "StdAfx.h"
#include "resource.h"
#include "ScrollBarEx.h"

#include "dbWave_constants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ScrollBarEx

IMPLEMENT_DYNAMIC(ScrollBarEx, CScrollBar)

ScrollBarEx::ScrollBarEx() : m_scBarInfo_old(), m_scInfo(), m_scInfo_old()
{
	m_bCaptured = FALSE;
	m_captureMode = 0;
	m_scBarInfo.cbSize = sizeof(SCROLLBARINFO);
}

ScrollBarEx::~ScrollBarEx()
= default;

BEGIN_MESSAGE_MAP(ScrollBarEx, CScrollBar)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// ScrollBarEx message handlers

void ScrollBarEx::OnMouseMove(UINT n_flags, CPoint point)
{
	if (!GetScrollBarInfo(&m_scBarInfo))
		return;

	if (m_captureMode == MOVE_RIGHT
		|| m_captureMode == MOVE_LEFT
		|| (point.x >= m_scBarInfo.xyThumbTop - SPLIT_SIZE - TRACK_SIZE && point.x <= m_scBarInfo.xyThumbTop + SPLIT_SIZE +
			TRACK_SIZE)
		|| (point.x >= m_scBarInfo.xyThumbBottom - SPLIT_SIZE - TRACK_SIZE && point.x <= m_scBarInfo.xyThumbBottom +
			SPLIT_SIZE + TRACK_SIZE))
		SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITHORIZONTAL));

	if (m_bCaptured)
	{
		// move limits up to cursor, according to selected mode
		switch (m_captureMode)
		{
		case MOVE_BOTTOM:
			if (point.x > m_scBarInfo.xyThumbTop)
			{
				m_scBarInfo.xyThumbBottom = point.x;
				// change page
				m_scInfo.nPage = MulDiv(
					(m_scBarInfo.xyThumbBottom - m_scBarInfo.xyThumbTop),
					m_scInfo_old.nPage,
					(m_scBarInfo_old.xyThumbBottom - m_scBarInfo_old.xyThumbTop));
				// update  barinfo
				m_scInfo.fMask = SIF_PAGE | SIF_POS;
				SetScrollInfo(&m_scInfo);
			}
			break;
		case MOVE_TOP:
			if (point.x < m_scBarInfo.xyThumbBottom)
			{
				m_scBarInfo.xyThumbTop = point.x;
				// change page
				m_scInfo.nPage = MulDiv(
					(m_scBarInfo.xyThumbBottom - m_scBarInfo.xyThumbTop),
					m_scInfo_old.nPage,
					(m_scBarInfo_old.xyThumbBottom - m_scBarInfo_old.xyThumbTop));
				m_scInfo.nPos = m_scInfo_old.nPos + (m_scInfo_old.nPage - m_scInfo.nPage);
				// update  barinfo
				m_scInfo.fMask = SIF_PAGE | SIF_POS;
				SetScrollInfo(&m_scInfo);
			}
			break;
		case MOVE_BOTH:
			m_scBarInfo.xyThumbTop = m_scBarInfo_old.xyThumbTop + point.x - oldpt.x;
			m_scBarInfo.xyThumbBottom = m_scBarInfo_old.xyThumbBottom + point.x - oldpt.x;
			m_scInfo.nPos = MulDiv(m_scBarInfo.xyThumbTop,
			                       m_scInfo_old.nPos,
			                       m_scBarInfo_old.xyThumbTop);
		// update  barinfo
			m_scInfo.fMask = SIF_PAGE | SIF_POS;
			SetScrollInfo(&m_scInfo);
			break;
		default:
			break;
		}

		//only invalidate if mouse drags something
		if (m_captureMode > 0)
			GetParent()->SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, 0), (LPARAM)m_hWnd);
	}
	else
		CScrollBar::OnMouseMove(n_flags, point);
}

void ScrollBarEx::OnLButtonDown(UINT n_flags, CPoint point)
{
	// capture mouse, get current scroll max and min values
	if (!GetScrollBarInfo(&m_scBarInfo))
		return;

	m_scBarInfo_old = m_scBarInfo;
	const auto flag = GetScrollInfo(&m_scInfo, SIF_ALL);
	ASSERT(flag);
	m_scInfo_old = m_scInfo;

	// click on the left limit of thumb
	if (point.x >= m_scBarInfo.xyThumbBottom - SPLIT_SIZE - TRACK_SIZE
		&& point.x <= m_scBarInfo.xyThumbBottom + SPLIT_SIZE + TRACK_SIZE)
	{
		m_captureMode = MOVE_BOTTOM;
		SetCapture();
		m_bCaptured = TRUE;
	}
	// click on right limit of thumb
	else if (point.x >= m_scBarInfo.xyThumbTop - SPLIT_SIZE - TRACK_SIZE
		&& point.x <= m_scBarInfo.xyThumbTop + SPLIT_SIZE + TRACK_SIZE)
	{
		m_captureMode = MOVE_TOP;
		SetCapture();
		m_bCaptured = TRUE;
	}
	// click on thumb; it is necessary to catch this event because the standard routine
	// does not transfert nPos as (int) but as a 16 bits value only
	else if (point.x <= m_scBarInfo.xyThumbBottom - SPLIT_SIZE - TRACK_SIZE
		&& point.x >= m_scBarInfo.xyThumbTop - SPLIT_SIZE - TRACK_SIZE)
	{
		m_captureMode = MOVE_BOTH;
		SetCapture();
		m_bCaptured = TRUE;
		oldpt = point;
	}
	else
		CScrollBar::OnLButtonDown(n_flags, point);
}

void ScrollBarEx::OnLButtonUp(UINT n_flags, CPoint point)
{
	if (m_bCaptured)
	{
		m_bCaptured = FALSE;
		m_captureMode = 0;
		ReleaseCapture();
		GetParent()->SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, 0), (LPARAM)m_hWnd);
	}
	else
		CScrollBar::OnLButtonUp(n_flags, point);
}
