// CG: This file was added by the Splash Screen component.
// Splash.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CSplashWnd::m_b_show_splash_wnd_;
CSplashWnd* CSplashWnd::m_p_splash_wnd_;

CSplashWnd::CSplashWnd()
{
}

CSplashWnd::~CSplashWnd()
{
	// Clear the static window pointer.
	ASSERT(m_p_splash_wnd_ == this);
	m_p_splash_wnd_ = nullptr;
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CSplashWnd::enable_splash_screen(BOOL b_enable /*= TRUE*/)
{
	m_b_show_splash_wnd_ = b_enable;
}

void CSplashWnd::show_splash_screen(CWnd* p_parent_wnd /*= NULL*/)
{
	if (!m_b_show_splash_wnd_ || m_p_splash_wnd_ != nullptr)
		return;

	// Allocate a new splash screen, and create the window.
	m_p_splash_wnd_ = new CSplashWnd;
	if (!m_p_splash_wnd_->Create(p_parent_wnd))
		delete m_p_splash_wnd_;
	else
		m_p_splash_wnd_->UpdateWindow();
}

BOOL CSplashWnd::PreTranslateAppMessage(MSG* p_msg)
{
	if (m_p_splash_wnd_ == nullptr)
		return FALSE;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (p_msg->message == WM_KEYDOWN ||
		p_msg->message == WM_SYSKEYDOWN ||
		p_msg->message == WM_LBUTTONDOWN ||
		p_msg->message == WM_RBUTTONDOWN ||
		p_msg->message == WM_MBUTTONDOWN ||
		p_msg->message == WM_NCLBUTTONDOWN ||
		p_msg->message == WM_NCRBUTTONDOWN ||
		p_msg->message == WM_NCMBUTTONDOWN)
	{
		m_p_splash_wnd_->hide_splash_screen();
		return TRUE; // message handled here
	}

	return FALSE; // message not handled
}

BOOL CSplashWnd::Create(CWnd* p_parent_wnd /*= NULL*/)
{
	if (!m_bitmap.LoadBitmap(IDB_SPLASH))
		return FALSE;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);
	return CreateEx(0,
	                AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
	                nullptr, WS_POPUP | WS_VISIBLE, 0, 0, bm.bmWidth, bm.bmHeight, p_parent_wnd->GetSafeHwnd(), nullptr);
}

void CSplashWnd::hide_splash_screen()
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
	hide_splash_screen();
}
