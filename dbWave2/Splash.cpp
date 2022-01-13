// CG: This file was added by the Splash Screen component.
// Splash.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL		CSplashWnd::m_bShowSplashWnd;
CSplashWnd* CSplashWnd::m_pSplashWnd;

CSplashWnd::CSplashWnd()
{
}

CSplashWnd::~CSplashWnd()
{
	// Clear the static window pointer.
	ASSERT(m_pSplashWnd == this);
	m_pSplashWnd = nullptr;
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CSplashWnd::EnableSplashScreen(BOOL bEnable /*= TRUE*/)
{
	m_bShowSplashWnd = bEnable;
}

void CSplashWnd::ShowSplashScreen(CWnd* pParentWnd /*= NULL*/)
{
	if (!m_bShowSplashWnd || m_pSplashWnd != nullptr)
		return;

	// Allocate a new splash screen, and create the window.
	m_pSplashWnd = new CSplashWnd;
	if (!m_pSplashWnd->Create(pParentWnd))
		delete m_pSplashWnd;
	else
		m_pSplashWnd->UpdateWindow();
}

BOOL CSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
	if (m_pSplashWnd == nullptr)
		return FALSE;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
		pMsg->message == WM_SYSKEYDOWN ||
		pMsg->message == WM_LBUTTONDOWN ||
		pMsg->message == WM_RBUTTONDOWN ||
		pMsg->message == WM_MBUTTONDOWN ||
		pMsg->message == WM_NCLBUTTONDOWN ||
		pMsg->message == WM_NCRBUTTONDOWN ||
		pMsg->message == WM_NCMBUTTONDOWN)
	{
		m_pSplashWnd->HideSplashScreen();
		return TRUE;	// message handled here
	}

	return FALSE;	// message not handled
}

BOOL CSplashWnd::Create(CWnd* pParentWnd /*= NULL*/)
{
	if (!m_bitmap.LoadBitmap(IDB_SPLASH))
		return FALSE;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);
	return CreateEx(0,
		AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		nullptr, WS_POPUP | WS_VISIBLE, 0, 0, bm.bmWidth, bm.bmHeight, pParentWnd->GetSafeHwnd(), nullptr);
}

void CSplashWnd::HideSplashScreen()
{
	// Destroy the window, and update the mainframe.
	DestroyWindow();
	AfxGetMainWnd()->UpdateWindow();
}

void CSplashWnd::PostNcDestroy()
{
	// Free the C++ class.
	delete this;
}

int CSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CenterWindow();
	SetTimer(1, 2000, nullptr);
	return 0;
}

void CSplashWnd::OnPaint()
{
	CPaintDC dc(this);

	CDC dc_image;
	if (!dc_image.CreateCompatibleDC(&dc))
		return;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);
	const auto p_old_bitmap = dc_image.SelectObject(&m_bitmap);
	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dc_image, 0, 0, SRCCOPY);
	dc_image.SelectObject(p_old_bitmap);
}

void CSplashWnd::OnTimer(UINT nIDEvent)
{
	HideSplashScreen();
}