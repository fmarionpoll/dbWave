#pragma once

// FormatHistogram.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFormatHistogramDlg dialog

class CFormatHistogramDlg : public CDialog
{
	// Construction
public:
	CFormatHistogramDlg(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	enum { IDD = IDD_HIISTOGRAMPARAMETERS };
	float	m_Ymax;
	float	m_xfirst;
	float	m_xlast;
	BOOL m_bYmaxAuto;
	COLORREF m_crHistFill;
	COLORREF m_crHistBorder;
	COLORREF m_crStimFill;
	COLORREF m_crStimBorder;
	COLORREF m_crChartArea;

	// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

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
