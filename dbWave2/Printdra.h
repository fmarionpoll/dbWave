#pragma once

// printdra.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrintDrawAreaDlg dialog

class CPrintDrawAreaDlg : public CDialog
{
	// Construction
public:
	CPrintDrawAreaDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PRINTDRAWAREA };
	int		m_HeightDoc;
	int		m_heightSeparator;
	int		m_WidthDoc;
	int		m_spkheight;
	int		m_spkwidth;
	OPTIONS_VIEWDATA* mdPM;

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bFilterDat;
};
