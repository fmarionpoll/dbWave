#pragma once
#include "ChartSpikeBar.h"
#include "ChartSpikeHist.h"
#include "ChartSpikeShape.h"
#include "ChartSpikeXY.h"
#include "dbTableView.h"
#include "Editctrl.h"
#include "ScrollBarEx.h"
#include "SPK_CLASSIF.h"


class ViewSpikeSort : public dbTableView
{
	DECLARE_DYNCREATE(ViewSpikeSort)
protected:
	ViewSpikeSort();
	~ViewSpikeSort() override;
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;
	void DoDataExchange(CDataExchange* p_dx) override;
	void OnInitialUpdate() override;
	void OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactive_view) override;

public:
	BOOL OnMove(UINT n_id_move_command) override;

	// Form Data
	enum { IDD = IDD_VIEWSPKSORT1 };

	CComboBox m_combo_parameter;

	float spike_shape_t1 { 0.f } ;
	float spike_shape_t2 { 0.5f };
	float m_time_unit { 1000.f }; // 1=s, 1000f=ms, 1e6=us

	float histogram_lower_threshold { 0.f };
	float histogram_upper_threshold {1.f};
	float histogram_bin_mv{ 0.1f };

	int sort_source_class {0};
	int sort_destination_class {0};

	float display_bars_time_left {0.f};
	float display_bars_time_last {0.f};

	float xy_max_amplitude_mv {0.f};
	float xy_min_amplitude_mv  {0.f};

	BOOL b_all_files  {false};
	int m_spike_index  {-1};
	int m_spike_index_class {0};
	float t_xy_right  {1.f};
	float t_xy_left {0.f};

	// Attributes
protected:
	ChartSpikeHist chart_histogram_; 
	ChartSpikeXY chart_xt_measures_; 
	ChartSpikeShape chart_spike_shape_; 
	ChartSpikeBar chart_spike_bar_; 

	CEditCtrl mm_t1_;
	CEditCtrl mm_t2_;
	CEditCtrl mm_limit_lower_;
	CEditCtrl mm_limit_upper_;
	CEditCtrl mm_source_class_;
	CEditCtrl mm_destination_class_;
	CEditCtrl mm_time_first_;
	CEditCtrl mm_time_last_;
	CEditCtrl mm_mv_min_;
	CEditCtrl mm_mv_max_;
	CEditCtrl mm_spike_index_;
	CEditCtrl mm_spike_index_class_;
	CEditCtrl mm_t_xy_right_;
	CEditCtrl mm_t_xy_left_;
	CEditCtrl mm_histogram_bin_mv_;

	ScrollBarEx m_file_scroll_; 
	SCROLLINFO m_file_scroll_infos_{}; 

	SPK_CLASSIF* spike_classification_parameters_{};
	OPTIONS_VIEWDATA* options_view_data_{};

	int m_i_tag_low_ {};
	int m_i_tag_up_ {};
	int m_i_xy_right_ {};
	int m_i_xy_left_ {};

	int m_spk_hist_upper_threshold_{};
	int m_spk_hist_lower_threshold_{};
	int m_spk_form_tag_left_{};
	int m_spk_form_tag_right_{};

	float mv_unit_ {1000.f}; // 1=V, 1000f=mV, 1e6=uV
	float m_delta_mv_{};
	float m_delta_t_{};
	int m_measure_y1_max_{}; // max of array m_measure_y1
	int m_measure_y1_min_{}; // min of array m_measure_y1
	boolean b_valid_extrema_ {false}; // tells if m_measure_y1_max_ & m_measure_y1_min_ are valid
	long l_first_{}; 
	long l_last_{}; 
	boolean b_measure_done_ {false}; 
	int div_amplitude_by_  {1}; // value to adjust changes in amplitude / filter(s)

	// Operations
public:
	void set_view_mouse_cursor(const int cursor_mode)
	{
		chart_xt_measures_.set_mouse_cursor_type(cursor_mode);
		chart_spike_shape_.set_mouse_cursor_type(cursor_mode);
	}

protected:
	void load_saved_parameters();
	void define_sub_classed_items();
	void define_stretch_parameters();
	void update_file_parameters();
	void update_spike_file();
	void update_legends();
	void select_spike(db_spike& spike_sel);
	void update_gain();
	void update_file_scroll();
	void scroll_file(UINT n_sb_code, UINT n_pos);
	void select_spike_list(int current_index);
	void activate_mode4();
	void show_controls_for_mode4(int n_cmd_show) const;
	void build_histogram();
	void clear_flag_all_spikes();
	void check_valid_threshold_limits();
	boolean open_dat_and_spk_files_of_selected_spike(const db_spike& spike_coords);

	void all_charts_invalidate();
	void all_charts_set_spike_list(SpikeList* spk_list);

	void set_mouse_cursor(short short_value);
	void change_hz_limits();
	void hit_spike();
	void change_vertical_tag_spike_shape(short short_value);
	void change_vertical_tag_histogram(short short_value);
	void change_vertical_tag_xy_chart(short short_value);
	void change_horizontal_tag_xy_chart(short short_value);
	void save_windows_properties_to_options();

	void gain_adjust_shape_and_bars();
	void gain_adjust_xy_and_histogram();

	// Generated message map functions
public:
	afx_msg void on_select_change_parameter();
	afx_msg void on_sort();

	afx_msg LRESULT on_my_message(WPARAM code, LPARAM l_param);
	afx_msg void on_measure();
	afx_msg void on_view_all_data_on_abscissa();
	afx_msg void on_format_center_curve();
	afx_msg void on_format_gain_adjust();
	afx_msg void on_format_split_curves();
	afx_msg void on_tools_edit_spikes();
	afx_msg void on_select_all_files();
	afx_msg void on_tools_align_spikes();
	afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);

	afx_msg void on_en_change_lower();
	afx_msg void on_en_change_upper();
	afx_msg void on_en_change_t1();
	afx_msg void on_en_change_t2();
	afx_msg void on_en_change_source_spike_class();
	afx_msg void on_en_change_destination_spike_class();
	afx_msg void on_en_change_time_first();
	afx_msg void on_en_change_time_last();
	afx_msg void on_en_change_mv_min();
	afx_msg void on_en_change_mv_max();
	afx_msg void on_en_change_no_spike();
	afx_msg void on_en_change_spike_class();
	afx_msg void on_en_change_edit_left2();
	afx_msg void on_en_change_edit_right2();
	afx_msg void on_en_change_hist_bin_ms();

	DECLARE_MESSAGE_MAP()
};
