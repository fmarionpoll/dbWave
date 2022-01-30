#pragma once


class DlgEditList : public CDialog
{
	DECLARE_DYNAMIC(CDlgEditList)

public:
	DlgEditList(CWnd* pParent = nullptr); // standard constructor
	~DlgEditList() override;
	CComboBox* pCo {nullptr};
	CString m_csNewString {_T("")};
	CListBox m_clStrings;
	CStringArray m_csArray;
	int m_selected {0};

	// Dialog Data
	enum { IDD = IDD_EDITLISTDLG };

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

public:
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedAdditem();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
