#pragma once


class DlgdbNewFileDuplicate : public CDialog
{
	// Construction
public:
	DlgdbNewFileDuplicate(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DUPLICATEFOUND };

	int m_option {-1};
	LPCTSTR m_p_file_in {nullptr};
	CString m_file_out {_T("")};

	// Overrides
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	CString m_cs_ext_ {_T("")};
	CString m_cs_path_{ _T("") };
	CString m_cs_name_{ _T("") };

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void on_radio3();

	DECLARE_MESSAGE_MAP()
};
