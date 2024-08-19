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

#include "DataListCtrl_Infos.h"
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

	void set_transform_mode(const int i_mode) { infos.data_transform = i_mode; }
	void set_display_mode(const int i_mode) { infos.display_mode = i_mode; }

	void set_time_intervals(const float t_first_new, const float t_last_new)
	{
		infos.t_first = t_first_new;
		infos.t_last = t_last_new;
	}

	void set_amplitude_span(const float mv_span_new) { infos.mv_span = mv_span_new; }
	// TODO: get extent, mode from current line
	void set_display_file_name(const boolean flag) { infos.b_display_file_name = flag; }
	void set_amplitude_adjust_mode(const boolean flag) { infos.b_set_mv_span = flag; }
	void set_timespan_adjust_mode(const boolean flag) { infos.b_set_time_span = flag; }

	void set_spike_plot_mode(const int spike_plot_mode_new, const int i_class)
	{
		infos.spike_plot_mode = spike_plot_mode_new;
		infos.selected_class = i_class;
	}

	int get_display_mode() const { return infos.display_mode; }
	float get_time_first() const { return infos.t_first; }
	float get_time_last() const { return infos.t_last; }
	float get_amplitude_span() const { return infos.mv_span; }
	int get_spike_plot_mode() const { return infos.spike_plot_mode; }
	int get_spike_class() const { return infos.selected_class; }

	ChartData* get_chart_data_of_current_record();
	AcqDataDoc* get_visible_rows_acq_data_doc_at(const int index) { return ptr_rows_[index]->p_data_doc; }
	CSpikeDoc* get_visible_rows_spike_doc_at(const int index) { return ptr_rows_[index]->p_spike_doc; }
	int get_visible_rows_size() const { return ptr_rows_.GetSize(); }

protected:
	CArray<DataListCtrl_Row*, DataListCtrl_Row*> ptr_rows_;
	static int m_column_width_[N_COLUMNS];
	static CString m_column_headers_[N_COLUMNS];
	static int m_column_format_[N_COLUMNS];
	static int m_column_index_[N_COLUMNS];
	CUIntArray* m_width_columns_ {nullptr};

public:
	DataListCtrlInfos infos;

protected:
	void delete_ptr_array();
	void save_columns_width() const;
	void resize_ptr_array(int n_items);
	void build_empty_bitmap(boolean b_forced_update = false);

	// Generated message map functions
	afx_msg void OnGetDisplayInfo(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);
	afx_msg void OnKeyUp(UINT n_char, UINT n_rep_cnt, UINT n_flags);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
