#pragma once

// dbNewFileDuplicateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CdbNewFileDuplicateDlg dialog

class CdbNewFileDuplicateDlg : public CDialog
{
// Construction
public:
	CdbNewFileDuplicateDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DUPLICATEFOUND };
	int		m_option;
	LPCTSTR 	m_pfilein;
	CString		m_fileout;

// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	CString m_csExt;
	CString	m_csPath;
	CString m_csName;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadio3();
	DECLARE_MESSAGE_MAP()
};

