#pragma once
#include "OPTIONS_VIEWDATA.h"

class CDataCommentsDlg : public CDialog
{
	// Construction
public:
	CDataCommentsDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DATACOMMENTS };

	BOOL m_bacqchans;
	BOOL m_bacqcomments;
	BOOL m_bacqdate;
	BOOL m_bacqtime;
	BOOL m_bfilesize;
	BOOL m_bacqchsetting;
	BOOL m_bdatabasecols;
	BOOL m_btoExcel;
	OPTIONS_VIEWDATA* m_pvO;

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
