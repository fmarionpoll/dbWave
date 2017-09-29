#pragma once

// CImportOptionsDlg dialog

class CImportOptionsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CImportOptionsDlg)

public:
	CImportOptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImportOptionsDlg();

// Dialog Data
	enum { IDD = IDD_IMPORTOPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bNewIDs;
	BOOL m_bAllowDuplicateFiles;
};
