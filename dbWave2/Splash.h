#pragma once

// CG: This file was added by the Splash Screen component.

// Splash.h : header file
//

/////////////////////////////////////////////////////////////////////////////
//   Splash Screen class

class CSplashWnd : public CWnd
{
	// Construction
protected:
	CSplashWnd();

	// Attributes:
public:
	CBitmap m_bitmap;

	// Operations
public:
	static void enable_splash_screen(BOOL b_enable = TRUE);
	static void show_splash_screen(CWnd* p_parent_wnd = nullptr);
	static BOOL PreTranslateAppMessage(MSG* p_msg);

	// Overrides

	// Implementation
public:
	~CSplashWnd() override;
	void PostNcDestroy() override;

protected:
	BOOL Create(CWnd* p_parent_wnd = nullptr);
	void hide_splash_screen();

	static BOOL m_b_show_splash_wnd_;
	static CSplashWnd* m_p_splash_wnd_;

	// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};
