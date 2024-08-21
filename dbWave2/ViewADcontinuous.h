#pragma once

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "ADAcqDataDoc.h"
#include "RulerBar.h"
#include "afxwin.h"
#include "ChartDataAD.h"
#include "StretchControls.h"
#include "DataTranslation_AD.h"
#include "DataTranslation_DA.h"
#include "USBPxxS1Ctl.h"
#include "options_input.h"
#include "options_output.h"
#include "ViewDbTable.h"


class ViewADcontinuous : public ViewDbTable
{
protected:
	ViewADcontinuous();
	~ViewADcontinuous() override;
	DECLARE_DYNCREATE(ViewADcontinuous)

	enum { IDD = IDD_VIEWADCONTINUOUS };

	CdbTableMain* m_p_table_set{ nullptr };
	CString m_boardName;
	DataTranslation_AD m_acq32_ad;
	DataTranslation_DA m_acq32_da;

	RulerBar m_ad_x_ruler_bar;
	RulerBar m_ad_y_ruler_bar;
	CButton m_zoom_button;
	CButton m_bias_button;
	CButton m_un_zoom_button;
	CButton m_button_start_stop_da;
	CButton m_button_sampling_mode;
	CButton m_button_output_channels;
	CButton m_button_write_to_disk;
	CButton m_button_oscilloscope;
	CComboBox m_combo_start_output;
	CComboBox m_combo_ad_card;
	CMFCButton m_btn_start_stop_ad;
	
	BOOL b_ad_write_to_file{ false };
	int m_b_start_out_put_mode{ 0 };
	bool m_da_present{ false };
	bool m_ad_present{ false };

protected:
	ChartDataAD m_chart_data_ad_;
	int m_cursor_state_{ 0 };
	float m_sweep_duration_{ 2 };
	CEditCtrl mm_y_upper_;
	CEditCtrl mm_y_lower_;

	HICON m_h_bias_{ nullptr };
	HICON m_h_zoom_{ nullptr };
	HICON m_h_un_zoom_{ nullptr };
	float m_y_scale_factor_{ 1 };
	int m_v_bar_mode_{ 0 };
	CScrollBar m_scroll_y_;
	COLORREF m_background_color_ = GetSysColor(COLOR_BTNFACE);
	CBrush* m_p_background_brush_ = new CBrush(m_background_color_);

	void on_gain_scroll(UINT n_sb_code, UINT n_pos);
	void on_bias_scroll(UINT n_sb_code, UINT n_pos);
	void update_gain_scroll();
	void update_bias_scroll();
	void set_v_bar_mode(short b_mode);

	void update_start_stop(BOOL b_start);
	void update_radio_buttons();

	// data	parameters
protected:
	BOOL m_b_found_dt_open_layer_dll_{ false };
	BOOL m_b_hide_subsequent_{ false };

	ADAcqDataDoc input_data_file_;
	//AcqDataDoc m_outputDataFile;	
	CStringArray cs_name_array_;
	BOOL b_file_open_{ false };
	CString sz_file_name_;

	BOOL b_ask_erase_{ false };
	BOOL b_changed_{ false };
	BOOL b_simultaneous_start_{ false };

	// DT buffer
	options_input* options_input_data_{ nullptr };
	options_output* options_output_data_{ nullptr };
	BOOL m_start_da_simultaneously_{ false };

	// sweep
	long m_channel_sweep_length{ 0 };
	long m_sweep_length_{ 1000 };
	int m_channel_sweep_start_{ 0 };
	int m_channel_sweep_end_{ 0 };
	int m_channel_sweep_refresh_{ 0 };
	int m_byte_sweep_refresh_{ 0 };
	float m_clock_rate_{ 10000.f };

	// functions for data acquisition
	BOOL find_dt_open_layers_boards();
	BOOL select_dt_open_layers_board(const CString& card_name);

	void set_combo_start_output(int option);

	void stop_acquisition();
	void save_and_close_file();
	BOOL start_acquisition();
	ECODE start_simultaneous_list();
	BOOL start_output();
	void stop_output();
	void init_acquisition_input_file();
	void init_acquisition_display();

	BOOL init_cyber_amp() const;
	BOOL define_experiment();
	void transfer_files_to_database();
	BOOL init_output_da();
	BOOL init_output_ad();
	void update_view_data_final();
	static void display_ol_da_error_message(const CHAR* error_string);
	void chain_dialog(WORD i_id);

	short* adc_transfer(short* source_data, const CWaveFormat* p_w_format);
	void under_sample_buffer(short* p_raw_data_buf, short* p_dt_buf0, const CWaveFormat* p_w_format, const int under_sample_factor);
	void adc_transfer_to_chart(short* p_data_buf, const CWaveFormat* p_w_format);
	void adc_transfer_to_file(CWaveFormat* p_w_format);
	void initialize_amplifiers() const;

	void get_acquisition_parameters_from_data_file();

	// Overrides
	CdbTableMain* OnGetRecordset() override;
	void DoDataExchange(CDataExchange* p_dx) override;
	void attach_controls();
	void OnInitialUpdate() override;
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;
	void OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactive_view) override;

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

public:
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);
	afx_msg HBRUSH OnCtlColor(CDC* p_dc, CWnd* p_wnd, UINT n_ctl_color);
	afx_msg void on_input_channels();
	afx_msg void on_sampling_mode();
	afx_msg void on_hardware_define_experiment();
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void on_buffer_done_adc();
	afx_msg void on_trigger_error_adc();
	afx_msg void on_overrun_error_adc();
	afx_msg void on_queue_done_adc();
	afx_msg void on_buffer_done_dac();
	afx_msg void on_overrun_error_dac();
	afx_msg void on_queue_done_dac();
	afx_msg void on_trigger_error_dac();
	afx_msg void on_bn_clicked_gain_button();
	afx_msg void on_bn_clicked_bias_button();
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void on_bn_clicked_da_parameters2();
	afx_msg void on_cbn_sel_change_combo_board();
	afx_msg void on_bn_clicked_start_stop();
	afx_msg void on_bn_clicked_write_to_disk();
	afx_msg void on_bn_clicked_oscilloscope();
	afx_msg void on_bn_clicked_card_features();
	afx_msg void on_cbn_sel_change_combo_start_output();
	afx_msg void on_bn_clicked_start_stop2();
	afx_msg void on_bn_clicked_un_zoom();
};

