#pragma once
#include "Editctrl.h"


class DlgXYParameters : public CDialog
{
	// Construction
public:
	DlgXYParameters(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_XYPARAMETERS };

	int m_xextent{ 0 };
	int m_xzero{ 0 };
	int m_yextent{ 0 };
	int m_yzero{ 0 };
	BOOL m_bDisplayspikes{ false };
	BOOL m_bDisplaybars{ false };
	BOOL m_bDisplaysource{ false };
	BOOL m_xparam{ true };
	BOOL m_yparam{ true };

	CEditCtrl mm_xextent;
	CEditCtrl mm_xzero;
	CEditCtrl mm_yextent;
	CEditCtrl mm_yzero;

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	BOOL OnInitDialog() override;
	afx_msg void OnEnChangeXextent();
	afx_msg void OnEnChangeXzero();
	afx_msg void OnEnChangeYextent();
	afx_msg void OnEnChangeYzero();

	DECLARE_MESSAGE_MAP()
};
