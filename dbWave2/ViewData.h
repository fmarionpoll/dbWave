#pragma once

#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "afxwin.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "ViewDbTable.h"


class ViewData : public ViewDbTable
{
protected:
	DECLARE_DYNCREATE(ViewData)
	ViewData();
	~ViewData() override;

	// Form Data
	enum { IDD = IDD_VIEWDATA };

	int m_channel_selected  {0};
	float m_first_hz_cursor  {0.};
	float m_second_hz_cursor  {0.};
	float m_difference_second_minus_first  {0.};
	float m_time_first_abscissa  {0.};
	float m_time_last_abscissa  {0.};
	float m_float_n_digits  {1000.}; // 10(000) -> n digits displayed
	boolean m_b_init_comment  {true};

	ChartData chart_data{}; 
	CEditCtrl mm_first_hz_cursor; 
	CEditCtrl mm_second_hz_cursor;
	CEditCtrl mm_difference_second_minus_first; 
	CEditCtrl mm_time_first_abscissa;
	CEditCtrl mm_time_last_abscissa;
	CComboBox m_combo_select_chan;
	RulerBar m_adc_y_ruler_bar;
	RulerBar m_ruler_bar_abscissa;

protected:
	// parameters related to data display and to document
	AcqDataDoc* m_p_dat_  {nullptr};
	boolean m_b_valid_doc_  {false};
	float m_sampling_rate_  {1.};
	int m_cursor_state_  {0};
	int m_v_bar_pixel_ratio_  {30}; 
	int m_h_bar_pixel_ratio_  {10}; 
	int m_current_file_index_  {0};

	HICON m_h_bias_  {nullptr};
	HICON m_h_zoom_  {nullptr};
	int scan_count_  {0};
	float channel_rate_  {0.};

	BOOL b_common_scale_ {false};

	ScrollBarEx file_scroll_bar_;
	SCROLLINFO file_scroll_bar_infos_{};
	int m_v_bar_mode_ {0};
	CScrollBar scroll_y_;

	options_view_data* options_view_data_  {nullptr};
	options_measure* options_data_measures_  {nullptr};

	void print_file_bottom_page(CDC* p_dc, const CPrintInfo* p_info);
	CString convert_file_index(long l_first, long l_last) const;
	void compute_printer_page_size();
	CString get_file_infos();
	CString print_bars(CDC* p_dc, const CRect* rect) const;
	BOOL get_file_series_index_from_page(int page, int& file_number, long& l_first);
	BOOL print_get_next_row(int& file_number, long& l_first, long& very_last);
	void save_modified_file();
	void update_file_parameters(BOOL b_update_interface = TRUE);
	void update_channels_display_parameters();
	void chain_dialog(WORD i_id);
	int print_get_n_pages();

	void on_file_scroll(UINT n_sb_code, UINT n_pos);
	void on_gain_scroll(UINT n_sb_code, UINT n_pos);
	void update_y_extent(int i_chan, int y_extent);
	void update_y_zero(int i_chan, int y_bias);
	void on_bias_scroll(UINT n_sb_code, UINT n_pos);
	void update_gain_scroll();
	void update_bias_scroll();
	void set_v_bar_mode(int b_mode);
	void update_file_scroll();
	void update_legends(int legends_options);
	void update_horizontal_tags_value();
	void set_mouse_cursor(int low_parameter);
	void add_vertical_cursors_from_defined_rectangle();
	void add_horizontal_cursors_from_defined_rectangle();
	void update_controls_according_to_cursor_selected();
	void update_channel(int channel);
	void measure_properties(int item);
	void define_sub_classed_items();
	void define_stretch_parameters();

	// Overrides
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;
	void DoDataExchange(CDataExchange* p_dx) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* p_info) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnPrint(CDC* p_dc, CPrintInfo* p_info) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* p_info) override;

	// Implementation
public:
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);

	afx_msg void on_clicked_bias();
	afx_msg void on_clicked_gain();
	afx_msg void on_format_x_scale();
	afx_msg void on_format_y_scale();
	afx_msg void on_edit_copy();
	afx_msg void on_update_edit_copy(CCmdUI* p_cmd_ui);
	afx_msg void on_tools_data_series();
	afx_msg void adc_on_hardware_channels_dlg();
	afx_msg void adc_on_hardware_intervals_dlg();
	afx_msg void on_center_curve();
	afx_msg void on_gain_adjust_curve();
	afx_msg void on_split_curves();
	afx_msg void on_first_frame();
	afx_msg void on_last_frame();
	afx_msg void on_view_all_data();
	afx_msg void on_format_data_series_attributes();
	afx_msg void on_tools_measure_mode();
	afx_msg void on_tools_measure();
	afx_msg void on_tools_vertical_tags();
	afx_msg void on_tools_horizontal_cursors();
	afx_msg void on_update_tools_horizontal_cursors(CCmdUI* p_cmd_ui);
	afx_msg void on_update_tools_vertical_tags(CCmdUI* p_cmd_ui);
	afx_msg void adc_on_hardware_define_experiment();
	afx_msg void on_en_change_time_first();
	afx_msg void on_en_change_time_last();
	afx_msg void on_cbn_sel_change_combo_chan();

	DECLARE_MESSAGE_MAP()
};
