#pragma once
#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "Spikelist.h"
#include "Spikedoc.h"
#include "dbWaveDoc.h"


struct RowStruct
{
	CString* comment;
	ChartSpikeShape* chart_shapes;
	ChartSpikeBar* chart_bars;
};

class SpikeClassListBox : public CListBox
{
public:
	SpikeClassListBox();
	~SpikeClassListBox() override;
protected:
	BOOL m_bText = true;
	BOOL m_bSpikes = true;
	BOOL m_bBars = true;
	int m_left_column_width = 20;
	int m_row_height = 20;
	int m_widthSeparator = 5;
	int m_widthText = -1;
	int m_widthSpikes = -1;
	int m_widthBars = -1;
	int m_topIndex = -1;
	COLORREF m_clrText = RGB(0, 0, 0);
	COLORREF m_clrBkgnd;
	CBrush m_brBkgnd;
	int m_cursorIndexMax = 3;

	// logic to drag spikes
	BOOL m_is_spike_hit = false; // one spike is hit
	int m_spike_hit = 0; // spike selected & hit
	int m_spike_hit_row = 0; // selected row from which spike is hit

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
	int GetHowManyClassesInCurrentSpikeList();
	long GetTimeFirst() const { return m_lFirst; }
	long GetTimeLast() const { return m_lLast; }

	void SetRowHeight(int rowheight);
	void SetLeftColWidth(int rowwidth);
	void SetColsWidth(int coltext, int colspikes, int colseparator);

	int GetRowHeight() const { return m_row_height; }
	int GetLeftColWidth() const { return m_left_column_width; }
	int GetColsTextWidth() const { return m_widthText; }
	int GetColsSpikesWidth() const { return m_widthSpikes; }
	int GetColsTimeWidth() const { return m_widthBars; }
	int GetColsSeparatorWidth() const { return m_widthSeparator; }
	float GetExtent_mV();

	void SetYzoom(int y_we, int y_wo);
	void SetXzoom(int x_we, int x_wo);

	int GetYWExtent(); 
	int GetYWOrg();
	int GetXWExtent(); 
	int GetXWOrg(); 

	int SelectSpike(int spike_no);
	void ChangeSpikeClass(int spike_no, int new_class);
	int SetMouseCursorType(int cursor_m);
	void XorTempVTtag(int x_point);
	void ResetBarsXortag();
	void ReflectBarsMouseMoveMessg(HWND hwnd);
	void SetCursorMaxOnDblClick(int imax) { m_cursorIndexMax = imax; }

	void PrintItem(CDC* p_dc, CRect* rect1, CRect* rect2, CRect* rect3, int i);

protected:
	void update_string(void* pptr, int i_class, int nb_spikes);
	void remove_spike_from_row(int spike_no);
	void add_spike_to_row(int spike_no);
	int get_row_index_of_spike_class(int spike_class) const;

	// Implementation
public:
	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) override;
	void DrawItem(LPDRAWITEMSTRUCT lpDIS) override;
	int CompareItem(LPCOMPAREITEMSTRUCT lpCIS) override;
	void DeleteItem(LPDELETEITEMSTRUCT lpDI) override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH CtlColor(CDC* p_dc, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
