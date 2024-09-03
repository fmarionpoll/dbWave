#pragma once

#include "SpikeClassListBox.h"
#include "ScrollBarEx.h"
#include "ViewDbTable.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "OPTIONS_MEASURE.h"
#include "options_spk_classification.h"
#include "options_detect_spikes.h"

class ViewSpikes : public ViewDbTable
{
protected:
	DECLARE_DYNCREATE(ViewSpikes)
	ViewSpikes();
	~ViewSpikes() override;

	// Form Data
	enum { IDD = IDD_VIEWSPIKES };

	float m_time_first  {0.f};
	float m_time_last  {0.f};
	int m_spike_index  {-1};
	int m_spike_class  {0};
	float m_zoom  {.2f};
	int m_class_source  {0};
	int m_class_destination  {1};
	BOOL m_b_reset_zoom { true};
	BOOL m_b_artefact { false};
	BOOL m_b_keep_same_class  {false};
	float m_jitter_ms  {1.f};

	CButton set_zoom;

	// Attributes
protected:
	ChartData chart_data_wnd_;
	SpikeClassListBox spike_class_listbox_;
	int max_classes_  {1};

	CEditCtrl mm_spike_index_;
	CEditCtrl mm_spike_class_;
	CEditCtrl mm_time_first_; 
	CEditCtrl mm_time_last_; 
	CEditCtrl mm_zoom_;
	CEditCtrl mm_class_source_;
	CEditCtrl mm_class_destination_;
	CEditCtrl mm_jitter_ms_;

	ScrollBarEx file_scrollbar_; 
	SCROLLINFO file_scroll_infos_{}; 

	int zoom_factor_  {1}; 
	HICON h_bias_ { nullptr};
	HICON h_zoom_  {nullptr};
	float y_scale_factor_  {0.f}; 
	int v_bar_mode_  {0}; 
	CScrollBar scrollbar_y_; 

	AcqDataDoc* p_data_doc_  {nullptr}; 

	BOOL b_init_source_view_  {true};
	int l_first_  {0};
	int l_last_  {-1};

	CDWordArray highlighted_intervals_;
	boolean b_add_spike_mode_  {false};

	int y_we_  {1};
	int y_wo_  {0};
	int pt_vt_  {-1};
	CRect rect_vt_track_  {CRect(0, 0, 0, 0)};
	float jitter_ { 0.f};
	boolean b_dummy_  {true};

	// Implementation
	void update_file_parameters(BOOL b_update_interface = TRUE);
	void update_legends(BOOL b_update_interface);
	void update_data_file(BOOL b_update_interface);
	void update_spike_file(BOOL b_update_interface);
	void update_gain_scroll();
	void update_bias_scroll();

	void select_spike(db_spike& spike_selected);
	void define_sub_classed_items();
	void define_stretch_parameters();
	void scroll_gain(UINT n_sb_code, UINT n_pos);
	void scroll_bias(UINT n_sb_code, UINT n_pos);
	void set_v_bar_mode(short b_mode);
	BOOL add_spike_to_list(long ii_time, BOOL check_if_spike_nearby);
	void set_add_spikes_mode(int mouse_cursor_type);
	void select_spike_list(int current_selection);
	void set_track_rectangle();
	void scroll_file(UINT n_sb_code, UINT n_pos);
	void update_file_scroll();
	void center_data_display_on_spike(int spike_no);
	void set_mouse_cursor(short param_value);

	void change_zoom(LPARAM l_param);
	void adjust_y_zoom_to_max_min(BOOL b_force_search_max_min);
	void zoom_on_preset_interval(int ii_start);

	// public interface to view
public:
	BOOL OnMove(UINT n_id_move_command) override;
protected:
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;
	void DoDataExchange(CDataExchange* p_dx) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* p_info) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnPrint(CDC* p_dc, CPrintInfo* p_info) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactivate_view) override;

	// page format printing parameters (pixel unit)
	options_view_data* options_view_data_ = nullptr; 
	options_measure* options_view_data_measure_ = nullptr; 
	options_spk_classification* spk_classification_parameters_ = nullptr;
	options_detect_spikes* spk_detection_parameters_ = nullptr; 

	void print_file_bottom_page(CDC* p_dc, const CPrintInfo* p_info);
	CString print_convert_file_index(long l_first, long l_last) const;
	void print_compute_page_size();
	CString print_get_file_infos();
	CString print_bars(CDC* p_dc, const CRect* rect) const;
	long print_get_file_series_index_from_page(int page, int* file);
	static float print_change_unit(float x_val, CString* x_unit, float* x_scale_factor);

	// Generated message map functions
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);

	afx_msg void on_format_all_data();
	afx_msg void on_format_center_curve();
	afx_msg void on_format_gain_adjust();
	afx_msg void on_format_previous_frame();
	afx_msg void on_format_next_frame();

	afx_msg void on_tools_edit_spikes();

	afx_msg void on_en_change_no_spike();
	afx_msg void on_en_change_spike_class();
	afx_msg void on_en_change_time_first();
	afx_msg void on_en_change_time_last();
	afx_msg void on_en_change_source_class();
	afx_msg void on_en_change_dest_class();
	afx_msg void on_en_change_jitter();
	afx_msg void on_en_change_zoom();

	afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void OnMouseMove(UINT n_flags, CPoint point);
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point);
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point);

	afx_msg void on_edit_copy();
	afx_msg void on_gain_button();
	afx_msg void on_bias_button();
	afx_msg void on_artefact();
	afx_msg void on_h_scroll_left();
	afx_msg void on_h_scroll_right();
	afx_msg void on_bn_clicked_same_class();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void on_zoom();
};
