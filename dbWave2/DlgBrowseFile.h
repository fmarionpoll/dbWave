#pragma once
#include "options_view_data.h"

class DlgBrowseFile : public CDialog
{
	// Construction
public:
	DlgBrowseFile(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_BROWSEFILE };

	BOOL m_all_channels { false };
	BOOL m_center_curves{ false };
	BOOL m_complete_file{ false };
	BOOL m_maximize_gain{ false };
	BOOL m_split_curves{ false };
	BOOL m_multi_row_display{ false };
	BOOL m_keep_for_each_file{ false };
	options_view_data* p_options_view_data{ nullptr };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
