#pragma once

// lsoption.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoadSaveOptionsDlg dialog

class CLoadSaveOptionsDlg : public CDialog
{
	// Construction
public:
	CLoadSaveOptionsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LOADSAVEOPTIONS };
	CComboBox	m_CBnamelist;
	CString	m_ddxcomment;
	CStringArray* pParmFiles;
	CString* pcomment;

	CStringArray pFiles;
	CString		m_currentFile;			//
	int			m_cursel;				// current file selected
	BOOL		m_bfilenamechanged;		// file name was modified
	BOOL		m_bcommentchanged;		// comment was modified

// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	int	 ParmFindString(CString& filename);
	void UpdateFileList();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeNamelist();
	virtual void OnCancel();
	afx_msg void OnKillfocusNamelist();
	afx_msg void OnEnChangeComment();
	afx_msg void OnEditchangeNamelist();
	afx_msg void OnLoad();
	afx_msg void OnSave();
	afx_msg void OnKillfocusComment();

	DECLARE_MESSAGE_MAP()
};
