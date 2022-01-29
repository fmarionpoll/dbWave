#pragma once
#include "OPTIONS_VIEWDATA.h"

class CPrintPageMarginsDlg : public CDialog
{
	// Construction
public:
	CPrintPageMarginsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PRINTPAGEMARGINS };
	int		m_leftPageMargin;
	int		m_bottomPageMargin;
	int		m_rightPageMargin;
	int		m_topPageMargin;
	OPTIONS_VIEWDATA* mdPM;

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
