#pragma once

// CImportOptionsDlg dialog

class CImportOptionsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CImportOptionsDlg)

public:
	CImportOptionsDlg(CWnd* pParent = nullptr); // standard constructor
	~CImportOptionsDlg() override;

	// Dialog Data
	enum { IDD = IDD_IMPORTOPTIONS };

	BOOL m_bNewIDs = false;
	BOOL m_bAllowDuplicateFiles = false;
	BOOL m_bReadColumns = false;
	BOOL m_bHeader = false;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
