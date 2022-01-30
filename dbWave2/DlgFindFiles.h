#pragma once
#include "afxeditbrowsectrl.h"
#include "dbWaveDoc.h"

class DlgFindFiles : public CDialog
{
	// Construction
public:
	DlgFindFiles(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_FINDFILES };

	CComboBox m_fileext;
	CString m_path{ _T("")};
	CMFCEditBrowseCtrl m_mfcbrowsecontrol;
	int m_nfound{ 0 };
	int m_selinit{ 0 };
	CStringArray* m_pfilenames{ nullptr };
	BOOL m_ioption{ 0 };
	BOOL m_banyformat{ false };
	CdbWaveDoc* m_pdbDoc{ nullptr };

	// Overrides
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	CString m_searchString;
	BOOL m_bSubtreeSearch{ false };
	CStringArray m_ppath;
	CString m_csrootSearch;

	void TraverseDirectory(CString path);
	void DisplaynFound();
	void FindFiles(CString path);

	// Generated message map functions
	void OnOK() override;
	BOOL OnInitDialog() override;
	afx_msg void OnSearch();

	DECLARE_MESSAGE_MAP()
};
