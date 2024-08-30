#pragma once
#include "options_view_data.h"


class DlgPrintDataOptions : public CDialog
{
	// Construction
public:
	DlgPrintDataOptions(CWnd* p_parent = nullptr);

	// Dialog Data
	enum { IDD = IDD_PRINTDATADRAWOPTIONS };

	BOOL m_b_clip_rect{ false };
	BOOL m_b_frame_rect{ false };
	BOOL m_b_time_scale_bar{ false };
	BOOL m_b_voltage_scale_bar{ false };
	BOOL m_b_print_selection{ false };
	BOOL m_b_print_spk_bars{ false };
	options_view_data* options_view_data{ nullptr };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;
	DECLARE_MESSAGE_MAP()
};

