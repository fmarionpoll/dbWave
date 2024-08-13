#pragma once


#define CTRL_COL_INDEX		1
#define CTRL_COL_CURVE		CTRL_COL_INDEX+1
#define CTRL_COL_INSECT		CTRL_COL_INDEX+2
#define CTRL_COL_SENSI		CTRL_COL_INDEX+3
#define CTRL_COL_STIM1		CTRL_COL_INDEX+4
#define CTRL_COL_CONC1		CTRL_COL_INDEX+5
#define CTRL_COL_STIM2		CTRL_COL_INDEX+6
#define CTRL_COL_CONC2		CTRL_COL_INDEX+7
#define CTRL_COL_NBSPK		CTRL_COL_INDEX+8
#define CTRL_COL_FLAG		CTRL_COL_INDEX+9
#include "DataListCtrl_Row.h"

class DataListCtrl : public CListCtrl
{
public:
	DataListCtrl();
	~DataListCtrl() override;

	void init_columns(CUIntArray* width_columns = nullptr);
	void set_cur_sel(int record_position);
	void update_cache(int index_first, int index_last);
	void refresh_display();
	void resize_signal_column(int n_pixels);
	void fit_columns_to_size(int n_pixels);

	void set_transform_mode(const int i_mode) { m_data_transform_ = i_mode; }
	void set_display_mode(const int i_mode) { m_display_mode_ = i_mode; }

	void set_time_intervals(const float t_first, const float t_last)
	{
		m_t_first_ = t_first;
		m_t_last_ = t_last;
	}

	void set_amplitude_span(const float mv_span) { m_m_v_span_ = mv_span; } // TODO ; get extent, mode from current line
	void set_display_file_name(const boolean flag) { m_b_display_file_name_ = flag; }
	void set_amplitude_adjust_mode(const boolean flag) { m_b_set_mv_span_ = flag; }
	void set_timespan_adjust_mode(const boolean flag) { m_b_set_time_span_ = flag; }

	void set_spike_plot_mode(const int spike_plot_mode, const int i_class)
	{
		m_spike_plot_mode_ = spike_plot_mode;
		m_selected_class_ = i_class;
	}

	int get_display_mode() const { return m_display_mode_; }
	float get_time_first() const { return m_t_first_; }
	float get_time_last() const { return m_t_last_; }
	float get_amplitude_span() const { return m_m_v_span_; }
	int get_spike_plot_mode() const { return m_spike_plot_mode_; }
	int get_spike_class() const { return m_selected_class_; }

	ChartData* get_chart_data_of_current_record();
	AcqDataDoc* get_visible_rows_acq_data_doc_at(int index) { return ptr_rows[index]->p_data_doc; }
	CSpikeDoc* get_visible_rows_spike_doc_at(int index) { return ptr_rows[index]->p_spike_doc; }
	int get_visible_rows_size() const { return ptr_rows.GetSize(); }

protected:
	CArray<CDataListCtrl_Row*, CDataListCtrl_Row*> ptr_rows;
	CImageList m_image_list_;
	static int m_column_width_[N_COLUMNS];
	static CString m_column_headers_[N_COLUMNS];
	static int m_column_format_[N_COLUMNS];
	static int m_column_index_[N_COLUMNS];

	CUIntArray* m_width_columns_ = nullptr;
	CBitmap* m_p_empty_bitmap_ = nullptr;

	int m_image_width_ = 400;
	int m_image_height_ = 50; 
	int m_data_transform_ = 0;
	int m_display_mode_ = 1;
	int m_spike_plot_mode_ = PLOT_BLACK;
	int m_selected_class_ = 0;
	float m_t_first_ = 0.f;
	float m_t_last_ = 0.f;
	float m_m_v_span_ = 0.f;
	boolean m_b_set_time_span_ = false;
	boolean m_b_set_mv_span_ = false;
	boolean m_b_display_file_name_ = false;

	void delete_ptr_array();
	void save_columns_width() const;
	void resize_ptr_array(int n_items);
	void set_empty_bitmap(boolean b_forced_update = false);
	void display_spike_wnd(CDataListCtrl_Row* ptr, int i_image);
	void display_data_wnd(CDataListCtrl_Row* ptr, int i_image);
	void display_empty_wnd(CDataListCtrl_Row* ptr, const int i_image);
	void plot_data(const CDataListCtrl_Row* ptr, ChartData* p_wnd, int i_image);

	// Generated message map functions
	afx_msg void OnGetDisplayInfo(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void OnKeyUp(UINT n_char, UINT n_rep_cnt, UINT n_flags);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
