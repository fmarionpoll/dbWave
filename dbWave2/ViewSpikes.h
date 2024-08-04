#pragma once

#include "SpikeClassListBox.h"
#include "ScrollBarEx.h"
#include "dbTableView.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "OPTIONS_VIEW_DATA_MEASURE.h"
#include "SPK_CLASSIF.h"
#include "SPKDETECTPARM.h"

class ViewSpikes : public dbTableView
{
protected:
	DECLARE_DYNCREATE(ViewSpikes)
	ViewSpikes();
	~ViewSpikes() override;

	// Form Data
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

	CButton set_zoom;

	// Attributes
protected:
	ChartData chart_data_wnd_;
	SpikeClassListBox spike_class_listbox_;
	int max_classes_ = 1;

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

	int zoom_factor_ = 1; 
	HICON h_bias_ = nullptr;
	HICON h_zoom_ = nullptr;
	float y_scale_factor_ = 0.f; 
	int v_bar_mode_ = 0; 
	CScrollBar scrollbar_y_; 

	AcqDataDoc* p_data_doc_ = nullptr; 

	BOOL b_init_source_view_ = true;
	int l_first_ = 0;
	int l_last_ = -1;

	CDWordArray highlighted_intervals_;
	boolean b_add_spike_mode_ = false;

	int y_we_ = 1;
	int y_wo_ = 0;
	int pt_vt_ = -1;
	CRect rect_vt_track_ = CRect(0, 0, 0, 0);
	float jitter_ = 0.f;
	boolean b_dummy_ = true;

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
	void scroll_bias(UINT nSBCode, UINT nPos);
	void set_v_bar_mode(short b_mode);
	BOOL add_spike_to_list(long ii_time, BOOL check_if_spike_nearby);
	void set_add_spikes_mode(int mouse_cursor_type);
	void select_spike_list(int current_selection);
	void set_track_rectangle();
	void scroll_file(UINT n_sb_code, UINT n_pos);
	void update_file_scroll();
	void center_data_display_on_spike(int spike_no);
	void set_mouse_cursor(short param_value);

	void change_zoom(LPARAM lParam);
	void adjust_y_zoom_to_max_min(BOOL b_force_search_max_min);
	void zoom_on_preset_interval(int ii_start);

	// public interface to view
public:
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* p_info) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* p_deactivate_view) override;

	// page format printing parameters (pixel unit)
	OPTIONS_VIEWDATA* options_view_data_ = nullptr; 
	OPTIONS_VIEW_DATA_MEASURE* options_view_data_measure_ = nullptr; 
	SPK_CLASSIF* spk_classification_parameters_ = nullptr;
	SPKDETECTPARM* spk_detection_parameters_ = nullptr; 

	void print_file_bottom_page(CDC* p_dc, const CPrintInfo* p_info);
	CString print_convert_file_index(long l_first, long l_last) const;
	void print_compute_page_size();
	CString print_get_file_infos();
	CString print_bars(CDC* p_dc, const CRect* rect) const;
	long print_get_file_series_index_from_page(int page, int* file);
	static float print_change_unit(float x_val, CString* x_unit, float* x_scale_factor);

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
	afx_msg void OnGAINbutton();
	afx_msg void OnBIASbutton();
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void OnArtefact();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHScrollLeft();
	afx_msg void OnHScrollRight();
	afx_msg void OnBnClickedSameclass();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void on_zoom();
};
