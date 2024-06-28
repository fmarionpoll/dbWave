#pragma once

#include "RowItem.h"
#include "SpikeClassListBoxContext.h"


class SpikeClassListBox : public CListBox
{
public:
	SpikeClassListBox();
	~SpikeClassListBox() override;

protected:
	BOOL m_bText = true;
	BOOL m_bSpikes = true;
	BOOL m_bBars = true;
	SpikeClassListBoxContext context;
	int m_cursorIndexMax = 3;

	long m_lFirst = 0;
	long m_lLast = 0;
	SpikeList* m_spike_list = nullptr;
	CSpikeDoc* m_spike_doc = nullptr;
	CdbWaveDoc* m_dbwave_doc = nullptr;
	HWND m_hwnd_bars_reflect = nullptr;

public:
	void set_source_data(SpikeList* pSList, CdbWaveDoc* pdbDoc);
	void SetSpkList(SpikeList* p_spike_list);

	void SetTimeIntervals(long l_first, long l_last);
	int count_classes_in_current_spike_list() const;
	long GetTimeFirst() const { return m_lFirst; }
	long GetTimeLast() const { return m_lLast; }

	void SetRowHeight(int row_height);
	void SetLeftColumnWidth(int row_width);
	void SetColumnsWidth(int width_spikes, int width_separator);

	int GetRowHeight() const { return context.m_row_height; }
	int GetLeftColumnWidth() const { return context.m_left_column_width; }
	int GetColumnsTextWidth() const { return context.m_widthText; }
	int GetColumnsSpikesWidth() const { return context.m_widthSpikes; }
	int GetColumnsTimeWidth() const { return context.m_widthBars; }
	int GetColumnsSeparatorWidth() const { return context.m_widthSeparator; }
	float GetExtent_mV() const;

	void SetYzoom(int y_we, int y_wo) const;
	void SetXzoom(int x_we, int x_wo) const;

	int GetYWExtent() const; 
	int GetYWOrg() const;
	int GetXWExtent() const; 
	int GetXWOrg() const; 

	int select_spike(db_spike& spike_selected);
	void ChangeSpikeClass(int spike_no, int new_class_id);
	int SetMouseCursorType(int cursor_m) const;
	void XorTempVTtag(int x_point) const;
	void ResetBarsXortag() const;
	void ReflectBarsMouseMoveMessg(HWND hwnd);
	void SetCursorMaxOnDblClick(const int i_max) { m_cursorIndexMax = i_max; }

	void PrintItem(CDC* p_dc, CRect* rect1, CRect* rect2, CRect* rect3, int i) const;

protected:
	void remove_spike_from_row(int spike_no);
	void add_spike_to_row(int spike_no);
	int get_row_index_of_spike_class(int spike_class) const;
	RowItem* add_row_item(int class_id, int i_id);
	void update_rows_from_spike_list();
	RowItem* get_row_item(int i) const { if (i >= GetCount() || i < 0) i = 0;;  return (RowItem*)GetItemData(i); }
	void set_horizontal_limits(int row_selected);
	void set_y_zoom(int row_selected) const;
	void set_class_of_dropped_spike(int row_selected);

	// Implementation
public:
	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) override;
	void DrawItem(LPDRAWITEMSTRUCT lpDIS) override;
	int CompareItem(LPCOMPAREITEMSTRUCT lpCIS) override;
	void DeleteItem(LPDELETEITEMSTRUCT lpDI) override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH CtlColor(CDC* p_dc, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
