#pragma once

// CTransferFilesDlg dialog

class CTransferFilesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTransferFilesDlg)

public:
	CTransferFilesDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CTransferFilesDlg();

	// Dialog Data
	enum { IDD = IDD_TRANSFERFILES };
	CString m_csPathname;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	afx_msg void OnBnClickedButtonpath();

	DECLARE_MESSAGE_MAP()
};
