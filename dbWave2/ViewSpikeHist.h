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

	CComboBox m_cbHistType;
	float m_timefirst{0.f};
	float m_timelast{2.5f};
	int m_spikeclass{0};
	int m_dotheight{0};
	int m_rowheight{0};
	float m_binISIms{0.f};
	int m_nbinsISI{0};
	float m_timebinms{0.f};

	CSpikeDoc* p_spike_doc_{nullptr};

	// Attributes
protected:
	CStretchControl m_stretch;
	BOOL m_binit{false};
	CEditCtrl mm_timebinms; // bin size (ms)
	CEditCtrl mm_binISIms; // bin size (ms)
	CEditCtrl mm_nbinsISI; // nbins ISI

	CEditCtrl mm_timefirst; // first abscissa value
	CEditCtrl mm_timelast; // last abscissa value
	CEditCtrl mm_spikeclass; // selected spike class
	CEditCtrl mm_dotheight; // dot height
	CEditCtrl mm_rowheight; // row height
	int m_bhistType{0};
	SCROLLINFO m_scrollFilePos_infos{};
	options_view_spikes* m_pvdS{nullptr};
	options_view_data* mdPM{nullptr};

	long* m_pPSTH{nullptr}; // histogram data (pointer to array)
	int m_sizepPSTH{0}; // nbins within histogram
	long m_nPSTH{0};
	long* m_pISI{nullptr};
	int m_sizepISI{0};
	long m_nISI{0};
	long* m_parrayISI{nullptr};
	int m_sizeparrayISI{0};

	CRect m_displayRect{CRect(0, 0, 0, 0)};
	CPoint m_topleft; // top position of display area
	BOOL m_initiated{false}; // flag / initial settings
	BOOL m_bmodified{true}; // flag ON-> compute data
	CBitmap* m_pbitmap{nullptr}; // temp bitmap used to improve display speed
	int m_nfiles{1}; // nb of files used to build histogram
	const float t1000{1000.f};
	BOOL m_bPrint{false};
	CRect m_commentRect;

	LOGFONT m_logFont{}; // onbegin/onendPrinting
	CFont m_fontPrint;
	int m_rectratio{100};
	float m_xfirst{0.f};
	float m_xlast{0.f};

	LOGFONT m_logFontDisp{}; // onbegin/onendPrinting
	CFont m_fontDisp{}; // display font

	// Overrides
public:
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL OnMove(UINT n_id_move_command) override;
protected:
	void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint) override;
	void DoDataExchange(CDataExchange* pDX) override;
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
	afx_msg void OnSelchangeHistogramtype();
	afx_msg void OnEnChangeEditnstipercycle();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeEditlockonstim();

public:
	CTabCtrl m_tabCtrl;
	afx_msg void OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

