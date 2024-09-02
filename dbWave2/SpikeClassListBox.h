#pragma once

#include "SpikeClassRowItem.h"
#include "SpikeClassListBoxContext.h"


class SpikeClassListBox : public CListBox
{
public:
	SpikeClassListBox();
	~SpikeClassListBox() override;

protected:
	BOOL m_b_text_  {true};
	BOOL m_b_spikes_  {true};
	BOOL m_b_bars_  {true};
	SpikeClassListBoxContext context_;
	int m_cursor_index_max_  {3};

	long m_l_first_  {0};
	long m_l_last_  {0};
	SpikeList* m_spike_list_  {nullptr};
	CSpikeDoc* m_spike_doc_  {nullptr};
	CdbWaveDoc* m_dbwave_doc_  {nullptr};
	HWND h_wnd_bars_reflect_  {nullptr};

public:
	void set_source_data(SpikeList* p_s_list, CdbWaveDoc* pdb_doc);
	void set_spk_list(SpikeList* p_spike_list);

	void set_time_intervals(long l_first, long l_last);
	int count_classes_in_current_spike_list() const;
	long get_time_first() const { return m_l_first_; }
	long get_time_last() const { return m_l_last_; }

	void set_row_height(int row_height);
	void set_left_column_width(int row_width);
	void set_columns_width(int width_spikes, int width_separator);

	int get_row_height() const { return context_.m_row_height; }
	int get_left_column_width() const { return context_.m_left_column_width; }
	int get_columns_text_width() const { return context_.m_width_text; }
	int get_columns_spikes_width() const { return context_.m_width_spikes; }
	int get_columns_time_width() const { return context_.m_width_bars; }
	int get_columns_separator_width() const { return context_.m_width_separator; }
	float get_extent_mv() const;

	void set_y_zoom(int y_we, int y_wo) const;
	void set_x_zoom(int x_we, int x_wo) const;

	int get_yw_extent() const; 
	int get_yw_org() const;
	int get_xw_extent() const; 
	int get_xw_org() const; 

	int select_spike(db_spike& spike_selected);
	void change_spike_class(int spike_no, int new_class_id);
	int set_mouse_cursor_type(int cursor_m) const;
	void xor_temp_vt_tag(int x_point) const;

	void reflect_bar_mouse_move_message(HWND h_wnd);
	void set_cursor_max_on_dbl_click(const int i_max) { m_cursor_index_max_ = i_max; }

	void print_item(CDC* p_dc, CRect* rect1, const CRect* rect2, const CRect* rect3, int i) const;

protected:
	void remove_spike_from_row(int spike_no);
	void add_spike_to_row(int spike_no);
	int get_row_index_of_spike_class(int spike_class) const;
	SpikeClassRowItem* add_row_item(int class_id, int i_id);
	void update_rows_from_spike_list();
	SpikeClassRowItem* get_row_item(int i) const { if (i >= GetCount() || i < 0) i = 0;  return reinterpret_cast<SpikeClassRowItem*>(GetItemData(i)); }
	void set_horizontal_limits(int row_selected);
	void set_y_zoom(int row_selected) const;
	void set_class_of_dropped_spike(int row_selected) const;

	// Implementation
public:
	void MeasureItem(LPMEASUREITEMSTRUCT lp_mis) override;
	void DrawItem(LPDRAWITEMSTRUCT lp_dis) override;
	int CompareItem(LPCOMPAREITEMSTRUCT lp_cis) override;
	void DeleteItem(LPDELETEITEMSTRUCT lp_di) override;

	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg LRESULT OnMyMessage(WPARAM w_param, LPARAM l_param);
	afx_msg HBRUSH CtlColor(CDC* p_dc, UINT n_ctl_color);
	afx_msg void OnMouseMove(UINT n_flags, CPoint point);
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point);
	afx_msg void OnRButtonUp(UINT n_flags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
