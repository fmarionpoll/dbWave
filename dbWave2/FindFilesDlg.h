
#pragma once
#include "afxeditbrowsectrl.h"
#include "dbWaveDoc.h"

// FindFilesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindFilesDlg dialog

class CFindFilesDlg : public CDialog
{
// Construction
public:
	CFindFilesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_FINDFILES };
	CComboBox			m_fileext;
	CString				m_path;
	CMFCEditBrowseCtrl	m_mfcbrowsecontrol;

	int					m_nfound;
	int					m_selinit;
	CStringArray*		m_pfilenames;
	BOOL				m_ioption;
	BOOL				m_banyformat;
	BOOL				m_bexcludecloud;
	
	CdbWaveDoc*			m_pdbDoc;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	CString			m_searchString;
	BOOL			m_bSubtreeSearch;
	CStringArray	m_ppath;	
	CString			m_csrootSearch;

	void TraverseDirectory (CString path);
	void DisplaynFound();
	void FindFiles (CString path);

	// Generated message map functions
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSearch();
	DECLARE_MESSAGE_MAP()
};
