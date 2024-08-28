#pragma once
#include "ChartSpikeShape.h"
#include "ViewDbTable.h"
#include "Editctrl.h"
#include "OPTIONS_MEASURE.h"
#include "options_spk_classification.h"
#include "TemplateListWnd.h"


class ViewSpikeTemplates : public ViewDbTable
{
protected:
	DECLARE_DYNCREATE(ViewSpikeTemplates)
	ViewSpikeTemplates();
	~ViewSpikeTemplates() override;
public:
	enum { IDD = IDD_VIEWSPKTEMPLATES };

	float m_t1 { 0.f};
	float m_t2  {6.f};
	float t_unit  {1000.f}; // 1:s, 1000f:ms, 1e6:us
	float time_first  {0.};
	float time_last  {0.};
	int hit_rate  {0};
	float k_tolerance  {0.};
	int spike_no_class  {0};
	int hit_rate_sort  {0};
	int i_first_sorted_class  {0};
	BOOL m_b_all_files  {false};
	BOOL b_all_templates  {false};
	BOOL b_all_sort  {false};
	BOOL b_display_single_class  {false};
	CTabCtrl m_tab1_ctrl{};

protected:
	CEditCtrl mm_t1_;
	CEditCtrl mm_t2_;
	CEditCtrl mm_hit_rate_;
	CEditCtrl mm_hit_rate_sort_;
	CEditCtrl mm_k_tolerance_;
	CEditCtrl mm_spike_no_class_;
	CEditCtrl mm_time_first_;
	CEditCtrl mm_time_last_;
	CEditCtrl mm_i_first_sorted_class_;

	CTemplateListWnd m_avg_list_{};
	CTemplateListWnd m_template_list_{};
	CTemplateListWnd m_avg_all_list_{};

	ChartSpikeShape m_chart_spk_wnd_shape_;

	options_view_data* options_view_data_  {nullptr}; 
	options_measure* options_view_data_measure_  {nullptr}; 
	options_spk_classification* spike_classification_parameters_  {nullptr}; 
	SCROLLINFO scroll_file_pos_infos_{};
	long l_first_  {0};
	long l_last_  {0};
	int spk_form_tag_left_  {0};
	int spk_form_tag_right_  {0};
	int spike_no_  {-1};

public:
	void set_view_mouse_cursor(const int cursor_mode) { m_chart_spk_wnd_shape_.set_mouse_cursor_type(cursor_mode); }

	BOOL OnMove(UINT n_id_move_command) override;
protected:
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;

	// Implementation
protected:
	void define_sub_classed_items();
	void define_stretch_parameters();
	void update_file_parameters(); // reset parameters for new file
	void update_templates();
	void update_legends();
	void select_spike(db_spike& spike_sel);
	void update_scrollbar();
	void select_spike_list(int index_current);
	void edit_spike_class(int control_id, int control_item);
	void display_avg(boolean b_all_files, CTemplateListWnd* template_list); 
	void sort_spikes();
	void update_ctrl_tab1(int i_select);
	void set_extent_zero_all_display(int extent, int zero);
	void update_spike_file();
	float convert_spike_index_to_time(const int index) const
	{
		return static_cast<float>(index) * t_unit / p_spk_list->get_acq_sampling_rate();
	}
	int convert_time_to_spike_index(const float time) const
	{
		return static_cast<int>(time * p_spk_list->get_acq_sampling_rate() / t_unit);
	}

public:
	// Generated message map functions
	afx_msg void on_en_change_class();
	afx_msg void on_en_change_time_first();
	afx_msg void on_en_change_time_last();
	afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);
	afx_msg void on_format_all_data();
	afx_msg void on_format_gain_adjust();
	afx_msg void on_format_center_curve();
	afx_msg void on_build_templates();
	afx_msg void on_en_change_hit_rate();
	afx_msg void on_en_change_tolerance();
	afx_msg void on_en_change_hit_rate_sort();
	afx_msg void on_keydown_template_list(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_check1();
	afx_msg void OnDestroy();

	//afx_msg void OnAllClasses();
	//afx_msg void OnSingleClass();
	//afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnLButtonClickedTab(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void on_bn_clicked_sort();
	afx_msg void on_bn_clicked_display();
	afx_msg void on_en_change_i_first_sorted_class();

	afx_msg void on_tcn_sel_change_tab2(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_nm_click_tab2(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_bn_clicked_display_single_class();
	afx_msg void on_en_change_t1();
	afx_msg void on_en_change_t2();

	DECLARE_MESSAGE_MAP()
};
