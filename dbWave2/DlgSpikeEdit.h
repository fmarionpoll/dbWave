#pragma once

#include "ChartData.h"
#include "ChartSpikeShape.h"

class DlgSpikeEdit : public CDialog
{
public:
	DlgSpikeEdit(CWnd* p_parent = nullptr);

	enum { IDD = IDD_EDITSPIKE };

	int dlg_spike_class{ 0 };
	int dlg_spike_index{ 0 };
	BOOL b_artefact{ false };
	int dlg_display_ratio { 20 };
	int dlg_yv_extent { 0 };

	CEditCtrl mm_dlg_spike_index;
	CEditCtrl mm_dlg_spike_class;
	CEditCtrl mm_dlg_display_ratio;
	CEditCtrl mm_dlg_yv_extent;

	CdbWaveDoc* db_wave_doc { nullptr };
	SpikeList* p_spk_list{ nullptr };
	int spike_chan { 0 };
	CWnd* m_parent { nullptr }; 
	int x_extent { 0 };
	int y_extent { 0 };
	int x_zero { 0 };
	int y_zero { 0 };
	BOOL b_changed { 0 };

protected:
	AcqDataDoc* p_acq_data_doc_ { nullptr };
	int spk_pre_trigger_{ 0 };
	int spk_length_{ 0 };
	int view_data_len_{ 0 };
	db_spike spike_sel_{};
	CDWordArray intervals_to_highlight_spikes_;
	ChartSpikeShape chart_spike_shape_;
	ChartData chart_data_;
	long ii_time_old_{ 0 };
	long ii_time_{ 0 };
	CScrollBar m_h_scroll_;
	CScrollBar m_v_scroll_;
	SCROLLINFO m_h_scroll_infos_{}; 
	SCROLLINFO m_v_scroll_infos_{};

	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support
	void load_spike_parameters();
	void load_source_view_data();
	void load_spike_from_data(int shift);
	void update_spike_scroll();
	void init_chart_spike();
	void init_chart_data();

	// Generated message map functions
	BOOL OnInitDialog() override;

	afx_msg void on_en_change_spike_index();
	afx_msg void on_en_change_spike_class();
	afx_msg void on_artefact();
	afx_msg void on_en_change_display_ratio();
	afx_msg void on_en_change_y_extent();
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);

	DECLARE_MESSAGE_MAP()
};
