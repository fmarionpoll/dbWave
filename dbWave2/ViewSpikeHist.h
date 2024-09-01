#pragma once
#include "ViewDbTable.h"
#include "StretchControls.h"

class ViewSpikeHist : public ViewDbTable
{
	DECLARE_DYNCREATE(ViewSpikeHist)
protected:
	ViewSpikeHist();
	~ViewSpikeHist() override;
	// Form Data
public:
	enum { IDD = IDD_VIEWSPKTIMESERIES };

	CComboBox m_cb_hist_type;
	float m_time_first{0.f};
	float m_time_last{2.5f};
	int m_spike_class{0};
	int m_dot_height{0};
	int m_row_height{0};
	float m_bin_isi_ms{0.f};
	int m_n_bins_isi{0};
	float m_time_bin_ms{0.f};

	CSpikeDoc* p_spike_doc{nullptr};

	// Attributes
protected:
	CStretchControl m_stretch_;
	
	CEditCtrl mm_time_bin_ms_; 
	CEditCtrl mm_bin_isi_ms_; 
	CEditCtrl mm_n_bins_isi_; 

	CEditCtrl mm_time_first_;
	CEditCtrl mm_time_last_; 
	CEditCtrl mm_spike_class_;
	CEditCtrl mm_dot_height_; 
	CEditCtrl mm_row_height_; 
	int m_b_hist_type_{0};
	SCROLLINFO m_scroll_file_pos_infos_{};
	options_view_spikes* options_view_spikes_{nullptr};
	options_view_data* options_view_data_{nullptr};

	long* m_p_psth_{nullptr}; // histogram data (pointer to array)
	int m_size_psth_{0}; // n_bins within histogram
	long m_n_psth_{0};
	long* m_p_isi_{nullptr};
	int m_size_isi_{0};
	long m_n_isi_{0};
	long* m_p_array_isi_{nullptr};
	int m_size_array_isi_{0};

	CRect m_display_rect_{CRect(0, 0, 0, 0)};
	CPoint m_top_left_; // top position of display area
	BOOL m_initiated_{false}; // flag / initial settings
	BOOL m_b_modified_{true}; // flag ON-> compute data
	CBitmap* m_p_bitmap_{nullptr}; // temp bitmap used to improve display speed
	int m_n_files_{1}; // nb of files used to build histogram
	const float t1000_{1000.f};
	BOOL m_b_print_{false};
	CRect m_comment_rect_;

	LOGFONT m_log_font_{}; 
	CFont m_font_print_;
	int m_rect_ratio_{100};
	float m_x_first_{0.f};
	float m_x_last_{0.f};

	LOGFONT m_log_font_display_{}; 
	CFont m_font_display_{}; 

	// Overrides
public:
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL OnMove(UINT n_id_move_command) override;
protected:
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;
	void DoDataExchange(CDataExchange* p_dx) override;
	void OnInitialUpdate() override;
	void OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactive_view) override;
	void OnDraw(CDC* p_dc) override;
	void OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	BOOL OnPreparePrinting(CPrintInfo* p_info) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;

protected:
	void build_data();
	void get_file_infos(CString& str_comment);
	void display_histogram(CDC* p_dc, const CRect* p_rect);
	void display_dot(CDC* p_dc, CRect* p_rect);
	void display_psth_autocorrelation(CDC* p_dc, CRect* p_rect);
	void display_stimulus(CDC* p_dc, const CRect* p_rect, const long* l_first, const long* l_last) const;
	void build_data_and_display();
	void show_controls(int i_select);
	void select_spk_list(int i_cur_sel, BOOL b_refresh_interface = FALSE);

	// Implementation
protected:
	long plot_histogram(CDC* p_dc, CRect* p_display_rect, int n_bins, long* p_hist0, int orientation = 0, int b_type = 0);

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	// Generated message map functions
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void on_en_change_time_first();
	afx_msg void on_en_change_time_last();
	afx_msg void on_en_change_time_bin();
	afx_msg void on_en_change_bin_isi();
	afx_msg void on_click_all_files();
	afx_msg void on_absolute_time();
	afx_msg void on_relative_time();
	afx_msg void on_click_one_class();
	afx_msg void on_click_all_classes();
	afx_msg void on_en_change_spike_class();
	afx_msg void on_en_change_n_bins();
	afx_msg void on_en_change_row_height();
	afx_msg void on_en_change_dot_height();
	afx_msg void on_format_histogram();
	afx_msg void on_click_cycle_hist();
	afx_msg void on_edit_copy();
	afx_msg void on_sel_change_histogram_type();
	afx_msg void on_en_change_edit_n_stimuli_per_cycle();
	afx_msg void OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void on_en_change_edit_lock_on_stim();

public:
	CTabCtrl m_tabCtrl;
	afx_msg void on_nm_click_tab1(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_tcn_sel_change_tab1(NMHDR* p_nmhdr, LRESULT* p_result);
	DECLARE_MESSAGE_MAP()
};

