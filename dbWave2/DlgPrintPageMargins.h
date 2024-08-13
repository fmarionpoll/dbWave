#pragma once
#include "OPTIONS_VIEW_DATA.h"

class DlgPrintPageMargins : public CDialog
{
	// Construction
public:
	DlgPrintPageMargins(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_PRINTPAGEMARGINS };

	int m_leftPageMargin{ 0 };
	int m_bottomPageMargin{ 0 };
	int m_rightPageMargin{ 0 };
	int m_topPageMargin{ 0 };
	OPTIONS_VIEW_DATA* mdPM{ nullptr };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
