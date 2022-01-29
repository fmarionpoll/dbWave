#pragma once

// CEditListDlg dialog

class CDlgEditList : public CDialog
{
	DECLARE_DYNAMIC(CDlgEditList)

public:
	CDlgEditList(CWnd* pParent = nullptr); // standard constructor
	~CDlgEditList() override;
	CComboBox* pCo;
	CString m_csNewString;
	CListBox m_clStrings;
	CStringArray m_csArray;
	int m_selected;

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
	//	afx_msg void OnBnClickedButton1();

	DECLARE_MESSAGE_MAP()
};
