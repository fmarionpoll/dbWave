// browsefi.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBrowseFileDlg dialog
#pragma once

class CBrowseFileDlg : public CDialog
{
	// Construction
public:
	CBrowseFileDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BROWSEFILE };
	BOOL	m_allchannels;
	BOOL	m_centercurves;
	BOOL	m_completefile;
	BOOL	m_maximizegain;
	BOOL	m_splitcurves;
	BOOL	m_multirowdisplay;
	BOOL	m_keepforeachfile;
	OPTIONS_VIEWDATA* mfBR;

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
