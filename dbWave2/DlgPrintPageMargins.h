#pragma once
#include "options_view_data.h"

class DlgPrintPageMargins : public CDialog
{
	// Construction
public:
	DlgPrintPageMargins(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_PRINTPAGEMARGINS };

	int m_left_page_margin{ 0 };
	int m_bottom_page_margin{ 0 };
	int m_right_page_margin{ 0 };
	int m_top_page_margin{ 0 };
	options_view_data* options_view_data{ nullptr };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
