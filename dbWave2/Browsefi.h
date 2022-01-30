#pragma once
#include "OPTIONS_VIEWDATA.h"

class CBrowseFileDlg : public CDialog
{
	// Construction
public:
	CBrowseFileDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_BROWSEFILE };

	BOOL m_allchannels { false };
	BOOL m_centercurves{ false };
	BOOL m_completefile{ false };
	BOOL m_maximizegain{ false };
	BOOL m_splitcurves{ false };
	BOOL m_multirowdisplay{ false };
	BOOL m_keepforeachfile{ false };
	OPTIONS_VIEWDATA* mfBR{ nullptr };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
