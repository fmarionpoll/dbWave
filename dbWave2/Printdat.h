#pragma once

// printdat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrintDataOptionsDlg dialog

class CPrintDataOptionsDlg : public CDialog
{
	// Construction
public:
	CPrintDataOptionsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PRINTDATADRAWOPTIONS };
	BOOL	m_bClipRect;
	BOOL	m_bFrameRect;
	BOOL	m_bTimeScaleBar;
	BOOL	m_bVoltageScaleBar;
	BOOL	m_bPrintSelection;
	BOOL	m_bPrintSpkBars;
	OPTIONS_VIEWDATA* mdPM;

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CPrintDataCommentsDlg dialog

class CPrintDataCommentsDlg : public CDialog
{
	// Construction
public:
	CPrintDataCommentsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PRINTDATACOMMENTSOPTIONS };
	BOOL	m_bacqcomment;
	BOOL	m_bacqdatetime;
	BOOL	m_bchanscomment;
	BOOL	m_bchansettings;
	BOOL	m_bdocname;
	int		m_fontsize;
	int		m_textseparator;
	OPTIONS_VIEWDATA* mdPM;

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
