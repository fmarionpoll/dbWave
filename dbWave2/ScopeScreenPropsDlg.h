#pragma once

// ScopeScreenPropsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScopeScreenPropsDlg dialog

class CScopeScreenPropsDlg : public CDialog
{
// Construction
public:
	CScopeScreenPropsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_SCOPEPROPERTIES };
	int		m_xyticks;
	int		m_xytickline;
	int		m_xcells;
	int		m_ycells;

	CScopeScreen* m_pscope;
	COLORREF m_crScopeFill;
	COLORREF m_crScopeGrid;

	CEditCtrl	mm_xcells;
	CEditCtrl	mm_ycells;
	CEditCtrl	mm_xyticks;
	CEditCtrl	mm_xytickline;
	
// Overrides

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeXCells();
	afx_msg void OnEnChangeXYTicks();
	afx_msg void OnEnChangeXYTicksLine();
	afx_msg void OnEnChangeYCells();
	afx_msg void OnBackgroundColor();
	afx_msg void OnGridColor();
	DECLARE_MESSAGE_MAP()
};
