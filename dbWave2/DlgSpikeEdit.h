#pragma once

#include "ChartData.h"
#include "ChartSpikeShape.h"

class DlgSpikeEdit : public CDialog
{
public:
	DlgSpikeEdit(CWnd* pParent = nullptr);

	enum { IDD = IDD_EDITSPIKE };

	int spike_class{ 0 };
	int spike_index{ 0 };
	BOOL b_artefact{ false };
	int display_ratio{ 0 };
	int yv_extent{ 0 };

	CEditCtrl mm_spike_index;
	CEditCtrl mm_spike_class;
	CEditCtrl mm_display_ratio;
	CEditCtrl mm_yv_extent;

	CdbWaveDoc* db_wave_doc { nullptr };
	int spike_chan { 0 };
	CWnd* m_parent { nullptr }; 
	int x_extent { 0 };
	int y_extent { 0 };
	int x_zero { 0 };
	int y_zero { 0 };
	BOOL b_changed { 0 };

protected:
	SpikeList* p_spk_list_ { nullptr };
	AcqDataDoc* p_acq_data_doc_ { nullptr };
	int spk_pre_trigger_{ 0 };
	int spk_length_{ 0 };
	int view_data_len_{ 0 };
	CDWordArray intervals_to_highlight_spikes_;
	ChartSpikeShape chart_spike_shape_;
	ChartData chart_data_;
	long ii_time_old_{ 0 };
	long ii_time_{ 0 };
	CScrollBar m_h_scroll_;
	CScrollBar m_v_scroll_;
	SCROLLINFO m_h_scroll_infos_{}; 
	SCROLLINFO m_v_scroll_infos_{};

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	void load_spike_parameters();
	void load_source_view_data();
	void load_spike_from_data(int shift);
	void update_spike_scroll();

	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void OnEnChangespike_index();
	afx_msg void OnEnChangeSpikeclass();
	afx_msg void OnArtefact();
	afx_msg void OnEnChangeDisplayratio();
	afx_msg void OnEnChangeYextent();
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	DECLARE_MESSAGE_MAP()
};
