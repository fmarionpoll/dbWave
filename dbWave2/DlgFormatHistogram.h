#pragma once


class DlgFormatHistogram : public CDialog
{
	// Construction
public:
	DlgFormatHistogram(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_HIISTOGRAMPARAMETERS };

	float m_Ymax{ 0.f };
	float m_xfirst{ 0.f };
	float m_xlast{ 0.f };
	BOOL m_bYmaxAuto{ false };
	COLORREF m_crHistFill{ 0 };
	COLORREF m_crHistBorder{ 0 };
	COLORREF m_crStimFill{ 0 };
	COLORREF m_crStimBorder{ 0 };
	COLORREF m_crChartArea{ 0 };
	// Overrides
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	// Generated message map functions
	afx_msg void OnCheckbYmaxAuto();
	afx_msg void OnButtonHistFill();
	afx_msg void OnPaint();
	afx_msg void OnHistBordercolor();
	afx_msg void OnStimulusFillcolor();
	afx_msg void OnStimulusBordercolor();
	afx_msg void OnBackgroundcolor();

	DECLARE_MESSAGE_MAP()
};
