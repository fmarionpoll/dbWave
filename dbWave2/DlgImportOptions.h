#pragma once

// DlgImportOptions dialog

class DlgImportOptions : public CDialogEx
{
	DECLARE_DYNAMIC(DlgImportOptions)

public:
	DlgImportOptions(CWnd* pParent = nullptr); // standard constructor
	~DlgImportOptions() override;

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
