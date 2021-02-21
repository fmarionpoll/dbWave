#pragma once

// CEditListDlg dialog

class CEditListDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditListDlg)

public:
	CEditListDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditListDlg();
	CComboBox* pCo;
	CString		m_csNewString;
	CListBox	m_clStrings;
	CStringArray m_csArray;
	int			m_selected;

	// Dialog Data
	enum { IDD = IDD_EDITLISTDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedAdditem();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//	afx_msg void OnBnClickedButton1();
};
