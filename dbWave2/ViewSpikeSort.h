#pragma once
#include "ChartSpikeBar.h"
#include "ChartSpikeHist.h"
#include "ChartSpikeShape.h"
#include "ChartSpikeXY.h"
#include "dbTableView.h"
#include "Editctrl.h"
#include "ScrollBarEx.h"
#include "SPKCLASSIF.h"


class ViewSpikeSort : public dbTableView
{
	DECLARE_DYNCREATE(ViewSpikeSort)
protected:
	ViewSpikeSort();
	~ViewSpikeSort() override;

	// Form Data
public:
	enum { IDD = IDD_VIEWSPKSORT1 };

	CComboBox m_combo_parameter;

	float m_t1 = 0.f;
	float m_t2 = 0.f;
	float m_time_unit = 1000.f; // 1=s, 1000f=ms, 1e6=us
	float limit_lower_threshold = 0.f;
	float limit_upper_threshold = 1.f;
	int m_source_class = 0;
	int m_destination_class = 0;
	float m_time_first = 0.f;
	float m_time_last = 0.f;
	float m_mv_max = 0.f;
	float m_mv_min = 0.f;
	float m_mv_bin = 0.01f;

	BOOL m_b_all_files = false;
	int m_spike_index = -1;
	int m_spike_index_class = 0;
	float m_t_xy_right = 1.f;
	float m_t_xy_left = 0.f;
	

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
	CEditCtrl mm_mv_bin_;

	ScrollBarEx m_file_scroll_; 
	SCROLLINFO m_file_scroll_infos_{}; 

	SPKCLASSIF* spike_classification_parameters_{};
	OPTIONS_VIEWDATA* options_view_data_{};

	int m_i_tag_low_{};
	int m_i_tag_up_{};
	int m_i_xy_right_{};
	int m_i_xy_left_{};

	int m_spk_hist_upper_threshold_{};
	int m_spk_hist_lower_threshold_{};
	int m_spk_form_tag_left_{};
	int m_spk_form_tag_right_{};

	float mv_unit_ = 1000.f; // 1=V, 1000f=mV, 1e6=uV
	float m_delta_{};
	int m_measure_y1_max_{}; // max of array m_measure_y1
	int m_measure_y1_min_{}; // min of array m_measure_y1
	BOOL b_valid_extrema_ = false; // tells if m_measure_y1_max_ & m_measure_y1_min_ are valid
	long l_first_{}; // display first
	long l_last_{}; // display last
	BOOL b_measure_done_ = false; // flag m_measure_y1 valid
	int div_amplitude_by_ = 1; // value to adjust changes in amplitude / filter(s)

	// Operations
public:
	void SetViewMouseCursor(const int cursor_mode)
	{
		chart_xt_measures_.set_mouse_cursor_type(cursor_mode);
		chart_spike_shape_.set_mouse_cursor_type(cursor_mode);
	}

	// Overrides
public:
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void define_sub_classed_items();
	void define_stretch_parameters();
	void OnInitialUpdate() override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	void update_file_parameters();
	void update_spike_file();
	void update_legends();
	void select_spike(db_spike& spike_sel);
	void update_gain();
	void update_file_scroll();
	void scroll_file(UINT n_sb_code, UINT n_pos);
	void select_spike_list(int current_index);
	void activate_mode4();
	void build_histogram();
	void clear_flag_all_spikes();
	void check_valid_threshold_limits();

	// Generated message map functions
public:
	afx_msg void OnSelchangeParameter();
	afx_msg void OnSort();
	afx_msg LRESULT OnMyMessage(WPARAM code, LPARAM lParam);
	afx_msg void OnMeasure();
	afx_msg void OnFormatAlldata();

	afx_msg void OnFormatCentercurve();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnFormatSplitcurves();
	afx_msg void OnToolsEdittransformspikes();
	afx_msg void OnSelectAllFiles();
	afx_msg void OnToolsAlignspikes();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnEnChangeLower();
	afx_msg void OnEnChangeUpper();
	afx_msg void OnEnChangeT1();
	afx_msg void OnEnChangeT2();
	afx_msg void OnEnChangeSourceSpikeClass();
	afx_msg void OnEnChangeDestinationSpikeClass();
	afx_msg void OnEnChangeTimeFirst();
	afx_msg void OnEnChangeTimeLast();
	afx_msg void OnEnChangemVMin();
	afx_msg void OnEnChangemVMax();
	afx_msg void OnEnChangeNOspike();
	afx_msg void OnEnChangeSpikeClass();
	afx_msg void OnEnChangeEditLeft2();
	afx_msg void OnEnChangeEditRight2();
	afx_msg void OnEnChangeNBins();

	DECLARE_MESSAGE_MAP()
};
