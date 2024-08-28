#pragma once

#include "ChartWnd.h"
#if !defined mytype
#define mytype	double
#endif

class CTemplateWnd : public ChartWnd
{
	DECLARE_SERIAL(CTemplateWnd)

	//Construction
public:
	CTemplateWnd();
	~CTemplateWnd() override;
	CTemplateWnd& operator =(const CTemplateWnd& arg);
	void Serialize(CArchive& ar) override;

	// user defined parameters
	CString m_cs_id {};
	int m_class_id {0};
	float m_k_tolerance {1.96f}; // user defined value
	double m_global_std {0.f}; // parameter to be set by parent
	double m_power {0}; // power of the signal

	int get_n_items() const { return m_n_items_; }

	void set_global_std(const double* global_std)
	{
		m_global_std = *global_std;
		m_b_valid_ = FALSE;
	}

	void set_k_tolerance(const float* k_tolerance)
	{
		m_k_tolerance = *k_tolerance;
		m_b_valid_ = FALSE;
	}

protected:
	BOOL m_b_valid_ {FALSE}; // tell if pMean data are valid
	int m_tpl_len_ {0}; // length of templates array
	// template array - intermediary results : sum, sum2
	// length = total spike length (defined by m_tpl_len)
	mytype* m_p_sum_array_ {nullptr}; // array declared for longs
	mytype* m_p_sum0_{ nullptr }; // array with sum (X)
	mytype* m_p_sum20_{ nullptr }; // array with sum (X * X)
	long m_n_items_ {0}; // n elements

	// template array - results and envelope
	// length = total spike length
	// mean, sup value, low value (as computed by tSetdisplayData())
	// combined arrays
	int* m_p_avg_ {nullptr}; // first point of the array
	int* m_p_max0_ {nullptr}; // upper limit
	int* m_p_min0_ {nullptr}; // lower limit

	// Operations
public:
	void set_template_length(int len, int extent = -1, int org = -1);
	void t_set_display_data(); // fill pMean data
	void t_init(); // init parameters, erase data pSum & pMean
	void t_add_spike_top_sum(int* p_source);
	double t_power_of_p_sum();
	BOOL t_get_number_of_points_within(int* p_source, const int* hit_rate);
	double t_min_dist(int* p_source, int* i_offset_min, BOOL b_jitter = FALSE);
	void t_global_stats(double* g_std, double* g_dist);

protected:
	double t_dist(int* p_source) const;
	void delete_arrays();

	// Implementation : display
protected:
	CArray<CPoint, CPoint> m_pts_avg_;
	CArray<CPoint, CPoint> m_pts_area_; // points displayed with polyline

	void init_polypoint_abscissa();
	void fill_ordinates_at_scale(BOOL bScale);
	void get_extents();
public:
	void plot_data_to_dc(CDC* p_dc) override;

	// Generated message map functions
protected:
	afx_msg void OnLButtonDblClk(UINT n_flags, CPoint point);
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point);
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point);
	afx_msg void OnMouseMove(UINT n_flags, CPoint point);
	afx_msg void OnRButtonDown(UINT n_flags, CPoint point);
	afx_msg void OnRButtonUp(UINT n_flags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* p_wnd, UINT n_hit_test, UINT message);
	DECLARE_MESSAGE_MAP()
};
