#pragma once


class CdbNewFileDuplicateDlg : public CDialog
{
	// Construction
public:
	CdbNewFileDuplicateDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DUPLICATEFOUND };

	int m_option;
	LPCTSTR m_pfilein;
	CString m_fileout;

	// Overrides
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	CString m_csExt;
	CString m_csPath;
	CString m_csName;

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void OnRadio3();

	DECLARE_MESSAGE_MAP()
};
