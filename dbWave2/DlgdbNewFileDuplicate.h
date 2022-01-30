#pragma once


class DlgdbNewFileDuplicate : public CDialog
{
	// Construction
public:
	DlgdbNewFileDuplicate(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DUPLICATEFOUND };

	int m_option {-1};
	LPCTSTR m_pfilein {nullptr};
	CString m_fileout {_T("")};

	// Overrides
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	CString m_csExt {_T("")};
	CString m_csPath{ _T("") };
	CString m_csName{ _T("") };

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void OnRadio3();

	DECLARE_MESSAGE_MAP()
};
