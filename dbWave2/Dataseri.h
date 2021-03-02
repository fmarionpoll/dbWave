// dataseri.h : header file
//
#pragma once

#include "afxcolorbutton.h"

/////////////////////////////////////////////////////////////////////////////
// CDataSeriesFormatDlg dialog

class CDataSeriesFormatDlg : public CDialog
{
	// Construction
public:
	CDataSeriesFormatDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DATASERIESFORMAT };
	CListBox	m_listseries;
	float	m_maxmv;
	float	m_minmv;
	CChartDataWnd* m_pChartDataWnd;	// data contours
	CAcqDataDoc* m_pdbDoc;		// data document
	int 	m_listindex;        	// same as lineview:Chanlist
	int		m_yzero;
	int		m_yextent;
	float	m_mVperbin;
	int		m_binzero;
	CMFCColorButton m_colorbutton;

	// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void GetParams(int index);
	void SetParams(int index);

	// Implementation
protected:

	// Generated message map functions
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListseries();

	DECLARE_MESSAGE_MAP()
};
