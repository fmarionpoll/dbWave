#pragma once

#include "SpikeClassListBox.h"
#include "ScrollBarEx.h"
#include "dbTableView.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "OPTIONS_VIEWDATAMEASURE.h"
#include "SPKCLASSIF.h"
#include "SPKDETECTPARM.h"

class ViewSpikes : public dbTableView
{
protected:
	DECLARE_DYNCREATE(ViewSpikes)
	ViewSpikes();
	~ViewSpikes() override;

	// Form Data
public:
	enum { IDD = IDD_VIEWSPIKES };

	float m_time_first = 0.f;
	float m_time_last = 0.f;
	int m_spike_index = -1;
	int m_spike_class = 0;
	float m_zoom = .2f;
	int m_class_source = 0;
	int m_class_destination = 1;
	BOOL m_b_reset_zoom = true;
	BOOL m_b_artefact = false;
	BOOL m_b_keep_same_class = false;
	float m_jitter_ms = 1.f;

	// Attributes
protected:
	ChartData m_ChartDataWnd;
	SpikeClassListBox m_spikeClassListBox;
	int m_max_classes = 1;

	CEditCtrl mm_spike_index;
	CEditCtrl mm_spike_class;
	CEditCtrl mm_time_first; 
	CEditCtrl mm_time_last; 
	CEditCtrl mm_zoom;
	CEditCtrl mm_class_source;
	CEditCtrl mm_class_destination;
	CEditCtrl mm_jitter_ms;

	ScrollBarEx m_file_scroll; 
	SCROLLINFO m_file_scroll_infos{}; 

	int m_zoom_factor = 1; 
	HICON m_hBias = nullptr;
	HICON m_hZoom = nullptr;
	float m_y_scale_factor = 0.f; 
	int m_VBarMode = 0; 
	CScrollBar m_scroll_y; 

	AcqDataDoc* p_data_doc_ = nullptr; 

	BOOL m_bInitSourceView = true;
	int m_lFirst = 0;
	int m_lLast = -1;

	CDWordArray m_highlighted_intervals;
	BOOL m_b_add_spike_mode = false;

	int m_yWE = 1;
	int m_yWO = 0;
	int m_ptVT = -1;
	CRect m_rectVTtrack = CRect(0, 0, 0, 0);
	float m_jitter = 0.f;
	BOOL m_bdummy = TRUE;

	// Implementation
	void update_file_parameters(BOOL b_update_interface = TRUE);
	void update_legends(BOOL b_update_interface);
	void update_data_file(BOOL b_update_interface);
	void update_spike_file(BOOL b_update_interface);
	void update_gain_scroll();
	void update_bias_scroll();
	void adjust_y_zoom_to_max_min(BOOL b_force_search_max_min);
	void select_spike(const db_spike& spike_selected);
	void define_sub_classed_items();
	void define_stretch_parameters();
	void zoom_on_preset_interval(int ii_start);
	void scroll_gain(UINT nSBCode, UINT nPos);
	void scroll_bias(UINT nSBCode, UINT nPos);
	void set_v_bar_mode(short bMode);
	BOOL add_spike_to_list(long ii_time, BOOL check_if_spike_nearby);
	void set_add_spikes_mode(int mouse_cursor_type);
	void select_spike_list(int current_selection);
	void set_track_rectangle();
	void scroll_file(UINT nSBCode, UINT nPos);
	void update_file_scroll();
	void center_data_display_on_spike(int spike_no);
	void set_mouse_cursor(short param_value);
	void change_zoom(LPARAM lParam);

	// public interface to view
public:
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* p_deactivate_view) override;

	// page format printing parameters (pixel unit)
	OPTIONS_VIEWDATA* options_viewdata = nullptr; // view data options
	OPTIONS_VIEWDATAMEASURE* mdMO = nullptr; // measure options
	SPKCLASSIF* m_psC = nullptr;
	SPKDETECTPARM* m_pspkDP = nullptr; // spike detection parameters

protected:
	void PrintFileBottomPage(CDC* p_dc, const CPrintInfo* pInfo);
	CString PrintConvertFileIndex(long l_first, long l_last);
	void PrintComputePageSize();
	CString PrintGetFileInfos();
	CString PrintBars(CDC* p_dc, const CRect* rect);
	long PrintGetFileSeriesIndexFromPage(int page, int* file);
	float PrintChangeUnit(float xVal, CString* xUnit, float* xScalefactor);

protected:
	// Generated message map functions
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);

	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatCentercurve();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnFormatPreviousframe();
	afx_msg void OnFormatNextframe();

	afx_msg void OnToolsEdittransformspikes();

	afx_msg void OnEnChangeNOspike();
	afx_msg void OnEnChangeSpikenoclass();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnEnChangeSourceclass();
	afx_msg void OnEnChangeDestclass();
	afx_msg void OnEnChangeJitter();
	afx_msg void OnEnChangeZoom();

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditCopy();
	afx_msg void OnZoom();
	afx_msg void OnGAINbutton();
	afx_msg void OnBIASbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnArtefact();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHScrollLeft();
	afx_msg void OnHScrollRight();
	afx_msg void OnBnClickedSameclass();

	DECLARE_MESSAGE_MAP()
};
