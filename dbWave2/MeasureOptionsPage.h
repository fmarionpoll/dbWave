#pragma once
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "OPTIONS_MEASURE.h"

class CMeasureOptionsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureOptionsPage)

	// Construction
public:
	CMeasureOptionsPage();
	~CMeasureOptionsPage() override;

	// Dialog Data
	enum { IDD = IDD_PROPPAGE1 };

	BOOL m_b_extrema{FALSE};
	BOOL m_b_diff_extrema {FALSE};
	BOOL m_b_diff_data_limits {FALSE};
	BOOL m_b_half_rise_time {FALSE};
	BOOL m_b_half_recovery {FALSE};
	BOOL m_b_data_limits {FALSE};
	BOOL m_b_limits_val{FALSE};
	WORD m_w_source_chan{0};
	WORD m_w_stimulus_thresh{0};
	WORD m_w_stimulus_chan{0};
	BOOL m_b_all_files{FALSE};
	int m_ui_source_chan{0};
	float m_f_stimulus_offset{0.0f};
	UINT m_ui_stimulus_threshold{0};
	UINT m_ui_stimulus_chan{0};

	// input parameters
	options_measure* m_p_options_measure{nullptr};
	CdbWaveDoc* m_pdb_doc{nullptr};
	AcqDataDoc* m_p_dat_doc{nullptr};
	ChartData* m_p_chart_data_wnd{nullptr};

	// Overrides
public:
	void OnOK() override;
	BOOL OnKillActive() override;
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	void show_limits_parameters(BOOL b_show) const;
	void show_chan_parameters(BOOL b_show) const;
	void save_options();

	// Generated message map functions
	afx_msg void on_all_channels();
	afx_msg void on_single_channel();
	afx_msg void on_vertical_tags();
	afx_msg void on_horizontal_tags();
	afx_msg void on_stimulus_tag();
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
