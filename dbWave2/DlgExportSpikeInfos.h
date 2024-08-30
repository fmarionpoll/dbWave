#pragma once
#include "options_view_spikes.h"


class DlgExportSpikeInfos : public CDialog
{
	// Construction
public:
	DlgExportSpikeInfos(CWnd* p_parent = nullptr);

	// Dialog Data
	enum { IDD = IDD_EXPORTSPIKEINFOS };

	BOOL m_b_acq_ch_settings {false};
	BOOL m_b_acq_comments {false};
	BOOL m_b_acq_date {false};
	float m_time_end {0.0f};
	float m_time_bin {0.0f};
	float m_time_start {0.0f};

	int m_class_nb {0};
	int m_class_nb2 {0};
	BOOL m_b_total_spikes {false};
	BOOL m_b_spk_comments {false};
	int m_i_spike_class_options {-1};
	int m_n_bins {1};
	float m_hist_ampl_v_max {0.0f};
	float m_hist_ampl_v_min {0.0f};
	int m_hist_ampl_n_bins {0};
	int m_i_export_options {-1};
	BOOL m_b_export_zero {false};
	BOOL m_b_export_pivot {false};
	BOOL m_b_export_to_excel {true};
	int m_i_stimulus_index {0};
	BOOL m_b_relation {true};
	options_view_spikes* options_view_spikes {nullptr};
	BOOL m_b_hist {SW_SHOW};
	BOOL m_b_hist_ampl{ SW_SHOW };

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* p_dx) override; 
	void display_hist_ampl_parameters(int b_display);
	void display_hist_parameters(int b_display);

	// Implementation
protected:
	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void on_class_filter();
	afx_msg void on_en_change_n_bins();
	afx_msg void on_en_change_bin_size();
	afx_msg void onclick_psth();
	afx_msg void onclick_isi();
	afx_msg void onclick_autocorrelation();
	afx_msg void onclick_others();
	afx_msg void onclick_ampl_histogram();
public:
	afx_msg void on_bn_clicked_radio5();
	afx_msg void on_bn_clicked_radio4();
	afx_msg void on_bn_clicked_spike_points();

	DECLARE_MESSAGE_MAP()
};
