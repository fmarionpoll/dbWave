// datacomm.h : header file
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDataCommentsDlg dialog

class CDataCommentsDlg : public CDialog
{
// Construction
public:
	CDataCommentsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DATACOMMENTS };
	BOOL	m_bacqchans;
	BOOL	m_bacqcomments;
	BOOL	m_bacqdate;
	BOOL	m_bacqtime;
	BOOL	m_bfilesize;
	BOOL	m_bacqchsetting;
	BOOL	m_bdatabasecols;
	BOOL	m_btoExcel;
	OPTIONS_VIEWDATA* m_pvO;

// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};
