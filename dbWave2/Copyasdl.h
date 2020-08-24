// copyasdl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCopyAsDlg dialog
#pragma once

class CCopyAsDlg : public CDialog
{
	// Construction
public:
	CCopyAsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_COPYAS };
	int		m_nabcissa;
	int		m_nordinates;
	int m_ioption;
	int m_iunit;
	BOOL m_bgraphics;

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
