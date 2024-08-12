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

protected:
	BOOL b_all_files_{ false };

	CComboBox combo_measure_type_;
	CEditCtrl mm_spike_index_;
	CEditCtrl mm_spike_index_class_;
	int spike_index_{ -1 };
	int spike_class_{ 0 };

	ChartSpikeShape chart_shape_;
	CEditCtrl mm_shape_t1_ms_;
	CEditCtrl mm_shape_t2_ms_;
	float shape_t1_ms_{ 0.f };
	float shape_t2_ms_{ 0.5f };
	int shape_t1_{};
	int shape_t2_{};
	void change_chart_shape_vt_tag(const int tag_index);

	CEditCtrl mm_source_class_;
	CEditCtrl mm_destination_class_;
	int sort_source_class_{ 0 };
	int sort_destination_class_{ 0 };

	ChartSpikeBar chart_spike_bar_; 
	ScrollBarEx m_file_scroll_;
	SCROLLINFO m_file_scroll_infos_{};
	CEditCtrl mm_time_first_s_;
	CEditCtrl mm_time_last_s_;
	float time_first_s_{ 0.f };
	float time_last_s_{ 0.f };
	long l_first_{};
	long l_last_{};

	ChartSpikeHist chart_histogram_;
	CEditCtrl mm_lower_threshold_mv_;
	CEditCtrl mm_upper_threshold_mv_;
	CEditCtrl mm_histogram_bin_mv_;
	float lower_threshold_mv_{ 0.f };
	float upper_threshold_mv_{ 1.f };
	float histogram_bin_mv_{ 0.1f };
	int tag_index_hist_low_{};
	int tag_index_hist_up_{};
	void build_histogram();
	void change_chart_histogram_vt_tag(const int tag_index);
	void update_cursors_from_lower_threshold_mv();
	void update_cursors_from_upper_threshold_mv();

	ChartSpikeXY chart_measures_;
	CEditCtrl mm_measure_min_mv_;
	CEditCtrl mm_measure_max_mv_;
	CEditCtrl mm_t_xy_right_;
	float measure_min_mv_{ 0.f };
	float measure_max_mv_{ 0.f };
	int m_i_xy_right_{};
	int m_i_xy_left_{};
	int tag_index_measures_low_{};
	int tag_index_measures_up_{};
	void change_chart_measure_hz_tag(const int value);
	void activate_mode4();
	void show_controls_for_mode4(int n_cmd_show) const;
	void change_chart_measure_vt_tag(const int value);
	
	CEditCtrl mm_t_xy_left_;
	float t_xy_right_{ 1.f };
	float t_xy_left_{ 0.f };

	SPK_CLASSIF* spike_classification_{};
	OPTIONS_VIEWDATA* options_view_data_{};

	const float time_unit_{ 1000.f }; // 1=s, 1000f=ms, 1e6=us
	const float mv_unit_ {1000.f};		// 1=V, 1000f=mV, 1e6=uV
	float delta_mv_{};
	float delta_ms_{};

	int m_measure_y1_max_{}; 
	int m_measure_y1_min_{}; 
	boolean b_valid_extrema_ {false}; 
	boolean b_measure_done_ {false};

	// Operations
public:
	void set_view_mouse_cursor(const int cursor_mode)
	{
		chart_measures_.set_mouse_cursor_type(cursor_mode);
		chart_shape_.set_mouse_cursor_type(cursor_mode);
	}

protected:
	void init_charts_from_saved_parameters();
	void define_sub_classed_items();
	void define_stretch_parameters();
	void update_file_parameters();
	void select_spike();
	void select_spike(db_spike& spike_sel);

	void load_current_spike_file();
	void update_legends();

	void update_gain();
	void update_file_scroll();
	void scroll_file(UINT n_sb_code, UINT n_pos);
	void select_spike_list(int current_index);
	
	void clear_flag_all_spikes();
	void check_valid_threshold_limits();

	boolean open_dat_and_spk_files_of_selected_spike(const db_spike& spike_coords);
	void all_charts_invalidate();
	void all_charts_set_spike_list(SpikeList* spk_list);

	void set_mouse_cursor(int value);
	void change_file_time_first_and_last();
	void hit_spike();
	
	void save_windows_properties_to_options();

	void gain_adjust();
	void gain_adjust_shape_and_bars();
	void gain_adjust_xy_and_histogram();
	void center_curve();

	// Generated message map functions
public:
	afx_msg void on_select_change_measure_type();
	afx_msg void on_sort();

	afx_msg LRESULT on_my_message(WPARAM code, LPARAM l_param);
	afx_msg void on_measure_parameters_from_spikes();
	afx_msg void on_view_all_data_on_abscissa();
	afx_msg void on_format_center_curve();
	afx_msg void on_format_gain_adjust();
	afx_msg void on_format_split_curves();
	afx_msg void on_tools_edit_spikes();
	afx_msg void on_select_all_files();
	afx_msg void on_tools_align_spikes();
	afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);

	afx_msg void on_en_change_lower_threshold();
	afx_msg void on_en_change_upper_threshold();
	afx_msg void on_en_change_shape_t1();
	afx_msg void on_en_change_shape_t2();
	afx_msg void on_en_change_source_class();
	afx_msg void on_en_change_destination_class();
	afx_msg void on_en_change_time_first();
	afx_msg void on_en_change_time_last();
	afx_msg void on_en_change_min_mv();
	afx_msg void on_en_change_max_mv();
	afx_msg void on_en_change_spike_index();
	afx_msg void on_en_change_spike_class();
	afx_msg void on_en_change_edit_left2();
	afx_msg void on_en_change_edit_right2();
	afx_msg void on_en_change_hist_bin_ms();

	DECLARE_MESSAGE_MAP()
};
