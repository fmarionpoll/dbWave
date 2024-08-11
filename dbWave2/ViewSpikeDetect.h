#pragma once
#include "ChartData.h"
#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "./Controls/cdxCRotBevelLine.h"
#include "dbTableView.h"
#include "Editctrl.h"


class ViewSpikeDetection : public dbTableView
{
protected:
	DECLARE_DYNCREATE(ViewSpikeDetection)
	ViewSpikeDetection();

	// Form Data
public:
	enum { IDD = IDD_VIEWSPKDETECTION };

	RulerBar m_xspkdscale;
	CComboBox m_CBdetectWhat;
	CComboBox m_CBdetectChan;
	CComboBox m_CBtransform;
	CComboBox m_CBtransform2;
	float m_timefirst = 0.f;
	float m_timelast = 0.f;
	CString m_datacomments{};
	int m_spike_index = -1;
	BOOL m_bartefact = false;
	float m_thresholdval = 0.f;
	int m_ichanselected = 0;
	int m_ichanselected2 = 0;
	cdxCRotBevelLine m_bevel1;
	cdxCRotBevelLine m_bevel2;
	cdxCRotBevelLine m_bevel3;

	// form variables
protected:
	int m_scan_count_doc = -1;
	CDWordArray m_DWintervals;

	ChartData m_chart_data_filtered;
	ChartData m_chart_data_source;
	ChartSpikeBar m_chart_spike_bar;
	ChartSpikeShape m_chart_spike_shape;

	ScrollBarEx m_filescroll;
	SCROLLINFO m_filescroll_infos{};

	CEditCtrl mm_spikeno;
	CEditCtrl mm_thresholdval;
	CEditCtrl mm_timefirst;
	CEditCtrl mm_timelast;
	CEditCtrl mm_spkWndDuration;
	CEditCtrl mm_spkWndAmplitude;
	CEditCtrl mm_ichanselected;
	CEditCtrl mm_ichanselected2;

	int m_zoom_integer = 0;
	SPKDETECTARRAY* m_pArrayFromApp = nullptr;
	SpikeDetectArray m_spk_detect_array_current;
	SPKDETECTPARM* m_p_detect_parameters = nullptr;
	int m_i_detect_parameters = 0;

	OPTIONS_VIEWDATA* options_view_data = nullptr;
	OPTIONS_MEASURE* options_view_data_measure = nullptr;

	float m_samplingRate = 0.f;
	float m_spkWndAmplitude = 1.f;
	float m_spkWndDuration = 6.f;
	BOOL m_bValidThreshold = false;
	BOOL m_bDetected = false;

	int m_cursor_state = 0;

public:
	void set_view_mouse_cursor(const int cursor_mode)
	{
		m_chart_spike_bar.set_mouse_cursor_type(cursor_mode);
		m_chart_spike_shape.set_mouse_cursor_type(cursor_mode);
		m_chart_data_filtered.set_mouse_cursor_type(cursor_mode);
		m_chart_data_source.set_mouse_cursor_type(cursor_mode);
	}

	// Implementation
protected:
	void detect_all(BOOL b_all); // detect from current set of parms or from all
	int	 detect_method_1(WORD channel_index); // spike detection, method 1 / m_spkD chan channel
	int  detect_stimulus_1(int channel_index); // stimulus detection
	void serialize_windows_state(BOOL save, int tab_index = -1);
	void align_display_to_current_spike();
	void highlight_spikes(BOOL flag = TRUE);
	void scroll_file(UINT n_sb_code, UINT n_pos);
	void select_spike_no(db_spike& spike_sel, BOOL b_multiple_selection);
	BOOL check_detection_settings();

	void update_combo_box();
	void update_combos_detect_and_transforms();
	void define_stretch_parameters();
	void define_sub_classed_items();

	void update_detection_parameters();
	void update_detection_controls();
	void update_detection_settings(int i_sel_parameters);
	void update_file_parameters(BOOL b_update_interface = TRUE);
	boolean update_data_file(BOOL b_update_interface);
	void update_spike_file(BOOL b_update_interface = TRUE);
	void update_file_scroll();
	void update_legends();
	void update_number_of_spikes();
	void update_legend_detection_wnd();
	void update_spike_shape_window_scale(BOOL b_set_from_controls = TRUE);
	void update_vt_tags();
	void update_spike_display();

	// public interface to view
public:
	// Overrides
	BOOL OnMove(UINT n_id_move_command) override;
protected:
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;
	void DoDataExchange(CDataExchange* p_dx) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* p_info) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnPrint(CDC* p_dc, CPrintInfo* p_info) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnActivateView(BOOL activate, CView* activated_view, CView* de_activated_view) override;

	// Implementation
public:
	~ViewSpikeDetection() override;
	// print view
protected:
	void print_file_bottom_page(CDC* p_dc, const CPrintInfo* p_info);
	CString print_convert_file_index(long l_first, long l_last) const;
	void print_compute_page_size();

	CString print_get_file_infos();
	CString print_data_bars(CDC* p_dc, const ChartData* p_data_chart_wnd, const CRect* p_rect);
	CString print_spk_shape_bars(CDC* p_dc, const CRect* prect, BOOL bAll);
	void print_create_font();
	BOOL print_get_file_series_index_from_page(int page, int& file_number, long& l_first);
	BOOL print_get_next_row(int& file_index, long& l_first, long& very_last);
	int print_get_n_pages();
	void print_data_cartridge(CDC* p_dc, ChartData* p_data_chart_wnd, const CRect* p_rect);

	// gain and bias setting: data and functions
	HICON m_hBias = nullptr;
	HICON m_hZoom = nullptr;
	CScrollBar m_scrolly;
	float m_yscaleFactor = 0.f;
	int m_VBarMode = 0;

	HICON m_hBias2 = nullptr;
	HICON m_hZoom2 = nullptr;
	CScrollBar m_scrolly2;
	float m_yscaleFactor2 = 0.f;
	int m_VBarMode2 = 0;

	void on_gain_scroll(UINT n_sb_code, UINT n_pos, int i_id);
	void on_bias_scroll(UINT n_sb_code, UINT n_pos, int i_id);
	void update_gain_scroll(int i_id);
	void update_bias_scroll(int i_id);
	void set_v_bar_mode(short b_mode, int i_id);
	void update_tabs();

	// Generated message map functions
public:
	afx_msg void on_measure_all();
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);
	afx_msg void on_first_frame();
	afx_msg void on_last_frame();
	afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void on_sel_change_detect_chan();
	afx_msg void on_sel_change_transform();
	afx_msg void on_sel_change_detect_mode();
	afx_msg void on_clear();
	afx_msg void on_en_change_spike_no();
	afx_msg void on_artefact();
	afx_msg void on_format_x_scale();
	afx_msg void on_format_all_data();
	afx_msg void on_format_y_scale_center_curve();
	afx_msg void on_format_y_scale_gain_adjust();
	afx_msg void on_format_split_curves();
	afx_msg void on_en_change_threshold();
	afx_msg void on_en_change_time_first();
	afx_msg void on_en_change_time_last();
	afx_msg void on_tools_detection_parameters();
	afx_msg void on_tools_edit_transform_spikes();
	afx_msg void on_tools_data_series();
	afx_msg void on_edit_copy();
	afx_msg void on_file_save();
	afx_msg void on_measure();

	afx_msg void on_bn_clicked_bias_button();
	afx_msg void on_bn_clicked_gain_button();
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void on_en_change_spk_wnd_amplitude();
	afx_msg void on_en_change_spk_wnd_length();
	afx_msg void on_bn_clicked_locate_button();

	afx_msg void on_sel_change_tab(NMHDR* pNMHDR, LRESULT* p_result);
	afx_msg void on_bn_clicked_clear_all();
	afx_msg void on_tools_edit_stimulus();
	afx_msg void on_en_change_chan_selected();
	afx_msg void on_bn_clicked_gain2();
	afx_msg void on_bn_clicked_bias2();
	afx_msg void on_en_change_chan_selected_2();
	afx_msg void on_cbn_sel_change_transform_2();

	DECLARE_MESSAGE_MAP()
};
