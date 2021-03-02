#pragma once
#include "Editctrl.h"

// xyparame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// XYParametersDlg dialog

class XYParametersDlg : public CDialog
{
	// Construction
public:
	XYParametersDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_XYPARAMETERS };
	int		m_xextent;
	int		m_xzero;
	int		m_yextent;
	int		m_yzero;
	BOOL	m_bDisplayspikes;
	BOOL	m_bDisplaybars;
	BOOL	m_bDisplaysource;
	BOOL m_xparam;
	BOOL m_yparam;

	CEditCtrl mm_xextent;
	CEditCtrl mm_xzero;
	CEditCtrl mm_yextent;
	CEditCtrl mm_yzero;

	// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeXextent();
	afx_msg void OnEnChangeXzero();
	afx_msg void OnEnChangeYextent();
	afx_msg void OnEnChangeYzero();

	DECLARE_MESSAGE_MAP()
};
