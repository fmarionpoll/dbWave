#pragma once
#include "OPTIONS_VIEWDATA.h"


class CPrintMarginsDlg : public CDialog
{
	// Construction
public:
	CPrintMarginsDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_PRINTMARGINS };

	OPTIONS_VIEWDATA* mdPM;
	int m_viewtype;

	// Implementation
protected:
	void SketchPrinterPage();
	void GetPageSize();

	CRect m_pagerect;
	CRect m_bars[8];

	CRect m_rect;
	BOOL m_bCaptured;
	int m_captureMode;
	int m_icapturedBar;

	void DrawBar(CRect* bar, CDC* pdc);
	int IsMouseOverAnyBar(CPoint* point);

	void DoDataExchange(CDataExchange* pDX) override;
	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void OnCommentsoptions();
	afx_msg void OnDrawarea();
	afx_msg void OnDrawoptions();
	afx_msg void OnPagemargins();
	afx_msg void OnPrintersetup();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
