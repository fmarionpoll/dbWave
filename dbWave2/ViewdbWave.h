#pragma once

#include "DataListCtrl.h"
#include "Editctrl.h"
#include "ViewDbTable.h"


class ViewdbWave : public ViewDbTable
{
protected: // create from serialization only
	DECLARE_DYNCREATE(ViewdbWave)
	ViewdbWave();
	~ViewdbWave() override;

	enum { IDD = IDD_VIEWDBWAVE };
	DataListCtrl m_data_list_ctrl;

protected:
	CEditCtrl mm_spike_class_;
	CEditCtrl mm_time_first_; 
	CEditCtrl mm_time_last_; 
	CEditCtrl mm_amplitude_span_;
	options_view_data* m_options_view_data_  {nullptr};

	boolean b_valid_data_  {false};
	boolean b_valid_spike_  {false};
	boolean b_add_mode_  {false};
	boolean b_filter_on_  {true};
	int m_data_transform_  {0};
	float m_time_first_  {0.};
	float m_time_last_  {0.};
	float m_amplitude_span_  {0.};
	int m_spike_class_  {0};

	void DoDataExchange(CDataExchange* p_dx) override;
	void OnInitialUpdate() override;
	void OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactive_view) override;
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;

	void update_controls();
	void fill_list_box();
	void display_data();
	void display_spikes();
	void display_nothing();
	void subclass_dialog_controls();
	void make_controls_stretchable();

public:
	void delete_records();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT n_type, int cx, int cy);

	afx_msg void on_record_page_up();
	afx_msg void on_record_page_down();
	afx_msg void on_click_median_filter();
	afx_msg void on_item_activate_list_ctrl(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_dbl_clk_list_ctrl(NMHDR* p_nmhdr, LRESULT* p_result);
public:
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);
	afx_msg void on_lvn_column_click_list_ctrl(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_bn_clicked_data();
	afx_msg void on_bn_clicked_display_spikes();
	afx_msg void on_bn_clicked_display_nothing();
	afx_msg void on_en_change_time_first();
	afx_msg void on_en_change_time_last();
	afx_msg void on_en_change_amplitude_span();
	afx_msg void on_bn_clicked_check_filename();
	afx_msg void on_hdn_end_track_list_ctrl(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_bn_clicked_check2();
	afx_msg void on_bn_clicked_check1();
	afx_msg void on_bn_clicked_radio_all_classes();
	afx_msg void on_bn_clicked_radio_one_class();
	afx_msg void on_en_change_spike_class();
};
