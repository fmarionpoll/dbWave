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
	static void EnableSplashScreen(BOOL bEnable = TRUE);
	static void ShowSplashScreen(CWnd* pParentWnd = nullptr);
	static BOOL PreTranslateAppMessage(MSG* pMsg);

	// Overrides

	// Implementation
public:
	~CSplashWnd() override;
	void PostNcDestroy() override;

protected:
	BOOL Create(CWnd* pParentWnd = nullptr);
	void HideSplashScreen();

	static BOOL m_bShowSplashWnd;
	static CSplashWnd* m_pSplashWnd;

	// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};
