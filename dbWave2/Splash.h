#pragma once



class CSplashWnd : public CWnd
{
	// Construction
protected:
	CSplashWnd();

	// Attributes:
public:
	CBitmap m_bitmap;

	// Operations
	static void EnableSplashScreen(BOOL bEnable = TRUE);
	static void ShowSplashScreen(CWnd* pParentWnd = nullptr);
	static BOOL PreTranslateAppMessage(MSG* pMsg);


	~CSplashWnd();
	virtual void PostNcDestroy();

protected:
	BOOL Create(CWnd* pParentWnd = nullptr);
	void HideSplashScreen();

	static BOOL			m_bShowSplashWnd;
	static CSplashWnd* m_pSplashWnd;

	// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};
