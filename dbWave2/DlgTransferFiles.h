#pragma once


class DlgTransferFiles : public CDialogEx
{
	DECLARE_DYNAMIC(DlgTransferFiles)

public:
	DlgTransferFiles(CWnd* p_parent = nullptr); // standard constructor
	~DlgTransferFiles() override;

	// Dialog Data
	enum { IDD = IDD_TRANSFERFILES };

	CString m_csPathname{ _T("")};

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

public:
	afx_msg void on_bn_clicked_button_path();

	DECLARE_MESSAGE_MAP()
};
