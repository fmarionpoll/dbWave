#pragma once


// CTransferFilesDlg dialog

class CTransferFilesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTransferFilesDlg)

public:
	CTransferFilesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTransferFilesDlg();

// Dialog Data
	enum { IDD = IDD_TRANSFERFILES };
	
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonpath();
	CString m_csPathname;
};
