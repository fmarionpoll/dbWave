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

	CComboBox m_CBparameter;

	float m_t1 = 0.f;
	float m_t2 = 0.f;
	float m_time_unit = 1000.f; // 1=s, 1000f=ms, 1e6=us
	float limit_lower_threshold_ = 0.f;
	float limit_upper_threshold_ = 1.f;
	int m_source_class = 0;
	int m_destination_class = 0;
	float m_timeFirst = 0.f;
	float m_timeLast = 0.f;
	float m_mVMax = 0.f;
	float m_mVMin = 0.f;
	BOOL m_bAllFiles = false;
	int m_spike_index = -1;
	int m_spike_index_class = 0;
	float m_txyright = 1.f;
	float m_txyleft = 0.f;
	float m_mV_bin = 0.01f;

	// Attributes
protected:
	ChartSpikeHist m_chart_histogram; 
	ChartSpikeXY m_chart_measures; 
	ChartSpikeShape m_chart_spike_shapes; 
	ChartSpikeBar m_chart_spike_bars; 

	CEditCtrl mm_t1;
	CEditCtrl mm_t2;
	CEditCtrl mm_limitlower;
	CEditCtrl mm_limitupper;
	CEditCtrl mm_source_class;
	CEditCtrl mm_destination_class;
	CEditCtrl mm_timeFirst;
	CEditCtrl mm_timeLast;
	CEditCtrl mm_mVMin;
	CEditCtrl mm_mVMax;
	CEditCtrl mm_spike_index;
	CEditCtrl mm_spike_index_class;
	CEditCtrl mm_txyright;
	CEditCtrl mm_txyleft;
	CEditCtrl mm_mV_bin;

	ScrollBarEx m_file_scroll; 
	SCROLLINFO m_file_scroll_infos{}; 

	SPKCLASSIF* spkclassif_{};
	OPTIONS_VIEWDATA* options_view_data_{};

	int m_itaglow{};
	int m_itagup{};
	int m_ixyright{};
	int m_ixyleft{};

	int m_spkhist_upper_threshold{};
	int m_spkhist_lower_threshold{};
	int m_spkform_tag_left{};
	int m_spkform_tag_right{};

	float m_vunit = 1000.f; // 1=V, 1000f=mV, 1e6=uV
	float m_delta{};
	int m_parmmax{}; // max of array m_measure_y1
	int m_parmmin{}; // min of array m_measure_y1
	BOOL m_bvalidextrema = false; // tells if m_parmmax & m_parmmin are valid
	long m_lFirst{}; // display first
	long m_lLast{}; // display last
	BOOL m_bMeasureDone = false; // flag m_measure_y1 valid
	int m_divAmplitudeBy = 1; // value to adjust changes in amplitude / filter(s)

	// Operations
public:
	void SetViewMouseCursor(int cursormode)
	{
		m_chart_measures.SetMouseCursorType(cursormode);
		m_chart_spike_shapes.SetMouseCursorType(cursormode);
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
	void select_spike_from_current_list(int spike_index);
	void update_gain();
	void update_file_scroll();
	void scroll_file(UINT nSBCode, UINT nPos);
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
