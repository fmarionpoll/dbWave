#pragma once
#include "OPTIONS_VIEWDATA.h"

class CBrowseFileDlg : public CDialog
{
	// Construction
public:
	CBrowseFileDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_BROWSEFILE };

	BOOL m_allchannels;
	BOOL m_centercurves;
	BOOL m_completefile;
	BOOL m_maximizegain;
	BOOL m_splitcurves;
	BOOL m_multirowdisplay;
	BOOL m_keepforeachfile;
	OPTIONS_VIEWDATA* mfBR;

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
