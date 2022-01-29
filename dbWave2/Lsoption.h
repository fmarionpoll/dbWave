#pragma once


class CLoadSaveOptionsDlg : public CDialog
{
	// Construction
public:
	CLoadSaveOptionsDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_LOADSAVEOPTIONS };

	CComboBox m_CBnamelist;
	CString m_ddxcomment;
	CStringArray* pParmFiles;
	CString* pcomment;

	CStringArray pFiles;
	CString m_currentFile; //
	int m_cursel; // current file selected
	BOOL m_bfilenamechanged; // file name was modified
	BOOL m_bcommentchanged; // comment was modified

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
