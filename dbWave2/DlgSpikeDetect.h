#pragma once


#include "ChartData.h"
#include "SpikeDetectArray.h"


class DlgSpikeDetect : public CDialog
{
	// Construction
public:
	DlgSpikeDetect(CWnd* p_parent = nullptr);

	// Dialog Data
	enum { IDD = IDD_SPKDETECTPARM };

	SpikeDetectArray* m_p_detect_settings_array{ nullptr }; // parameter set by caller
	AcqDataDoc* m_db_doc{ nullptr }; 
	int m_i_detect_parameters_dlg{ 0 };
	options_view_data* md_pm{ nullptr };
	ChartData* m_p_chart_data_detect_wnd{ nullptr };
	ChartData* m_p_chart_data_source_wnd{ nullptr };

protected:
	options_detect_spikes* options_detect_spikes_{ nullptr };
	int m_scan_count_{ 0 };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support
	void load_chan_parameters(int chan);
	void save_chan_parameters(int chan);
	void display_detect_from_chan();
	void set_dlg_interface_state(int detect_what) const;
	void update_source_view() const;
	void exchange_parameters(int i_source, int i_destination);
	void set_tab_comment(int i, const CString& cs);
	void update_tab_shift_buttons() const;

	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void on_detect_from_tag();
	afx_msg void on_detect_from_chan();

public:
	CTabCtrl m_c_parameter_tab_ctrl;
	afx_msg void on_tcn_sel_change_parameters_tab(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_bn_clicked_spikes_radio();
	afx_msg void on_bn_clicked_stimulus_radio();
	afx_msg void on_bn_clicked_add_parameters_button();
	afx_msg void on_bn_clicked_del_parameters_button();
	afx_msg void on_en_change_detect_threshold();
	afx_msg void on_cbn_sel_change_detect_channel();
	afx_msg void on_cbn_sel_change_detect_transform();
	afx_msg void on_delta_pos_spin1(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_cbn_sel_change_extract_channel();
	afx_msg void on_en_change_comment();
	afx_msg void on_bn_clicked_shift_right();
	afx_msg void on_bn_clicked_shift_left();

	DECLARE_MESSAGE_MAP()
};
