#pragma once
#include "options_view_data.h"

class DlgDataComments : public CDialog
{
public:
	DlgDataComments(CWnd* p_parent = nullptr); 

	// Dialog Data
	enum { IDD = IDD_DATACOMMENTS };

	BOOL b_acq_channels{ false };
	BOOL b_acq_comments{ false };
	BOOL b_acq_date{ false };
	BOOL b_acq_time{ false };
	BOOL b_file_size{ false };
	BOOL b_acq_channel_setting{ false };
	BOOL b_data_base_cols{ false };
	BOOL b_to_excel{ false };
	options_view_data* p_options_view_data{ nullptr };

protected:
	void DoDataExchange(CDataExchange* pDX) override; 

	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
};
