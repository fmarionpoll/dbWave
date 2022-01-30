#pragma once


class DlgTransferFiles : public CDialogEx
{
	DECLARE_DYNAMIC(DlgTransferFiles)

public:
	DlgTransferFiles(CWnd* pParent = nullptr); // standard constructor
	~DlgTransferFiles() override;

	// Dialog Data
	enum { IDD = IDD_TRANSFERFILES };

	CString m_csPathname{ _T("")};

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

public:
	afx_msg void OnBnClickedButtonpath();

	DECLARE_MESSAGE_MAP()
};
