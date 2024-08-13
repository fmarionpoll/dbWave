#pragma once
#include "options_view_data.h"


class DlgPrintMargins : public CDialog
{
	// Construction
public:
	DlgPrintMargins(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_PRINTMARGINS };

	options_view_data* mdPM{ nullptr };
	int m_viewtype{ 0 };

	// Implementation
protected:
	void SketchPrinterPage();
	void GetPageSize();

	CRect m_pagerect;
	CRect m_bars[8];

	CRect m_rect;
	BOOL m_bCaptured{ false };
	int m_captureMode{ 0 };
	int m_icapturedBar{ 0 };

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
