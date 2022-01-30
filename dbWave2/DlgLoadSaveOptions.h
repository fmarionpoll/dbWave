#pragma once


class DlgLoadSaveOptions : public CDialog
{
	// Construction
public:
	DlgLoadSaveOptions(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_LOADSAVEOPTIONS };

	CComboBox m_CBnamelist;
	CString m_ddxcomment{ _T("")};
	CStringArray* pParmFiles{ nullptr };
	CString* pcomment{ nullptr };

	CStringArray pFiles;
	CString m_currentFile; 
	int m_cursel{ 0 };
	BOOL m_bfilenamechanged{ false };
	BOOL m_bcommentchanged{ false };

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	int ParmFindString(CString& filename);
	void UpdateFileList();

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void OnSelchangeNamelist();
	void OnCancel() override;
	afx_msg void OnKillfocusNamelist();
	afx_msg void OnEnChangeComment();
	afx_msg void OnEditchangeNamelist();
	afx_msg void OnLoad();
	afx_msg void OnSave();
	afx_msg void OnKillfocusComment();

	DECLARE_MESSAGE_MAP()
};
