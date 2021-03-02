#pragma once

// CImportOptionsDlg dialog

class CImportOptionsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CImportOptionsDlg)

public:
	CImportOptionsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CImportOptionsDlg();

	// Dialog Data
	enum { IDD = IDD_IMPORTOPTIONS };
	BOOL m_bNewIDs;
	BOOL m_bAllowDuplicateFiles;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
