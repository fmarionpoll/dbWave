#pragma once
#include "ChartSpikeShape.h"
#include "dbTableView.h"
#include "Editctrl.h"
#include "OPTIONS_DATA_MEASURE.h"
#include "SPK_CLASSIF.h"
#include "TemplateListWnd.h"


class ViewSpikeTemplates : public dbTableView
{
protected:
	DECLARE_DYNCREATE(ViewSpikeTemplates)
	ViewSpikeTemplates();
	~ViewSpikeTemplates() override;
public:
	enum { IDD = IDD_VIEWSPKTEMPLATES };

	float m_t1 = 0.f;
	float m_t2 = 6.f;
	float t_unit = 1000.f; // 1=s, 1000f=ms, 1e6=us
	float time_first = 0.;
	float time_last = 0.;
	int hit_rate = 0;
	float k_tolerance = 0.;
	int spike_no_class = 0;
	int hit_rate_sort = 0;
	int i_first_sorted_class = 0;
	BOOL m_b_all_files = false;
	BOOL b_all_templates = false;
	BOOL b_all_sort = false;
	BOOL b_display_single_class = false;
	CTabCtrl m_tab1_ctrl{};

protected:
	CEditCtrl mm_t1_;
	CEditCtrl mm_t2_;
	CEditCtrl mm_hit_rate_;
	CEditCtrl mm_hit_rate_sort_;
	CEditCtrl mm_k_tolerance_;
	CEditCtrl mm_spike_no_class_;
	CEditCtrl mm_time_first_; // first abscissa value
	CEditCtrl mm_time_last_; // last abscissa value
	CEditCtrl mm_i_first_sorted_class_;

	CTemplateListWnd m_avg_list_{};
	CTemplateListWnd m_template_list_{};
	CTemplateListWnd m_avg_all_list_{};

	ChartSpikeShape m_chart_spk_wnd_shape_;

	OPTIONS_VIEWDATA* options_view_data_ = nullptr; 
	OPTIONS_DATA_MEASURE* options_view_data_measure_ = nullptr; 
	SPK_CLASSIF* spike_classification_parameters_ = nullptr; 
	SCROLLINFO scroll_file_pos_infos_{};
	long l_first_ = 0;
	long l_last_ = 0;
	int spk_form_tag_left_ = 0;
	int spk_form_tag_right_ = 0;
	int spike_no_ = -1;

public:
	void set_view_mouse_cursor(const int cursor_mode) { m_chart_spk_wnd_shape_.set_mouse_cursor_type(cursor_mode); }

public:
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
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
		return static_cast<float>(index) * t_unit / m_pSpkList->get_acq_sampling_rate();
	}
	int convert_time_to_spike_index(const float time) const
	{
		return static_cast<int>(time * m_pSpkList->get_acq_sampling_rate() / t_unit);
	}

public:
	// Generated message map functions
	afx_msg void OnEnChangeclassno();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnFormatCentercurve();
	afx_msg void OnBuildTemplates();
	afx_msg void OnEnChangeHitrate();
	afx_msg void OnEnChangeTolerance();
	afx_msg void OnEnChangeHitrateSort();
	afx_msg void OnKeydownTemplateList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheck1();
	afx_msg void OnDestroy();

	//afx_msg void OnAllClasses();
	//afx_msg void OnSingleClass();
	//afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnLButtonClickedTab(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnBnClickedSort();
	afx_msg void OnBnClickedDisplay();
	afx_msg void OnEnChangeIfirstsortedclass();

	afx_msg void OnTcnSelchangeTab2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTab2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedDisplaysingleclass();
	afx_msg void OnEnChangeT1();
	afx_msg void OnEnChangeT2();

	DECLARE_MESSAGE_MAP()
};
