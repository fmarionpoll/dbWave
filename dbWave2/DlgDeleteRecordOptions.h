#pragma once

// CDeleteRecordOptionsDlg dialog

class CDeleteRecordOptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CDeleteRecordOptionsDlg)

public:
	CDeleteRecordOptionsDlg(CWnd* pParent = nullptr); // standard constructor
	~CDeleteRecordOptionsDlg() override;

	// Dialog Data
	enum { IDD = IDD_DELRECORDOPTIONS };

public:
	BOOL m_bKeepChoice;
	BOOL m_bDeleteFile;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
