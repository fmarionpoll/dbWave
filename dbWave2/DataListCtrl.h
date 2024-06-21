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

	void set_transform_mode(const int i_mode) { m_data_transform = i_mode; }
	void set_display_mode(const int i_mode) { m_display_mode = i_mode; }

	void set_time_intervals(const float t_first, const float t_last)
	{
		m_tFirst = t_first;
		m_tLast = t_last;
	}

	void set_amplitude_span(const float mv_span) { m_mV_span = mv_span; } // TODO ; get extent, mode from current line
	void set_display_file_name(const boolean flag) { m_b_display_file_name = flag; }
	void set_amplitude_adjust_mode(const boolean flag) { m_b_set_mV_span = flag; }
	void set_timespan_adjust_mode(const boolean flag) { m_b_set_time_span = flag; }

	void set_spike_plot_mode(const int spike_plot_mode, const int i_class)
	{
		m_spike_plot_mode = spike_plot_mode;
		m_selected_class = i_class;
	}

	int get_display_mode() const { return m_display_mode; }
	float get_time_first() const { return m_tFirst; }
	float get_time_last() const { return m_tLast; }
	float get_amplitude_span() const { return m_mV_span; }
	int get_spike_plot_mode() const { return m_spike_plot_mode; }
	int get_spike_class() const { return m_selected_class; }

	ChartData* get_chart_data_of_current_record();
	AcqDataDoc* get_visible_rows_acq_data_doc_at(int index) { return ptr_rows[index]->pdataDoc; }
	CSpikeDoc* get_visible_rows_spike_doc_at(int index) { return ptr_rows[index]->pspikeDoc; }
	int get_visible_rows_size() const { return ptr_rows.GetSize(); }

protected:
	CArray<CDataListCtrl_Row*, CDataListCtrl_Row*> ptr_rows;
	CImageList m_image_list;
	static int m_column_width[NCOLS];
	static CString m_column_headers[NCOLS];
	static int m_column_format[NCOLS];
	static int m_column_index[NCOLS];

	CUIntArray* m_width_columns = nullptr;
	CBitmap* m_p_empty_bitmap = nullptr;

	int m_image_width = 400;
	int m_image_height = 50; 
	int m_data_transform = 0;
	int m_display_mode = 1;
	int m_spike_plot_mode = PLOT_BLACK;
	int m_selected_class = 0;
	float m_tFirst = 0.f;
	float m_tLast = 0.f;
	float m_mV_span = 0.f;
	boolean m_b_set_time_span = false;
	boolean m_b_set_mV_span = false;
	boolean m_b_display_file_name = false;

	void delete_ptr_array();
	void save_columns_width() const;
	void resize_ptr_array(int n_items);
	void set_empty_bitmap(boolean b_forced_update = false);
	void display_spike_wnd(CDataListCtrl_Row* ptr, int iImage);
	void display_data_wnd(CDataListCtrl_Row* ptr, int iImage);
	void display_empty_wnd(CDataListCtrl_Row* ptr, int iImage);
	void plot_data(const CDataListCtrl_Row* ptr, ChartData* p_wnd, int iImage);

	// Generated message map functions
	afx_msg void OnGetDisplayInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
