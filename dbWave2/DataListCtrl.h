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

	void InitColumns(CUIntArray* width_columns = nullptr);
	void SetCurSel(int record_position);
	void UpdateCache(int index_first, int index_last);
	void RefreshDisplay();
	void ResizeSignalColumn(int n_pixels);
	void FitColumnsToSize(int n_pixels);

	void SetTransformMode(int imode) { m_data_transform = imode; }
	void SetDisplayMode(int imode) { m_display_mode = imode; }

	void SetTimeIntervals(float tfirst, float tlast)
	{
		m_tFirst = tfirst;
		m_tLast = tlast;
	}

	void SetAmplitudeSpan(float mv_span) { m_mV_span = mv_span; } // TODO ; get extent, mode from current line
	void SetDisplayFileName(boolean flag) { m_b_display_file_name = flag; }
	void SetAmplitudeAdjustMode(boolean flag) { m_b_set_mV_span = flag; }
	void SetTimespanAdjustMode(boolean flag) { m_b_set_time_span = flag; }

	void SetSpikePlotMode(const int spike_plot_mode, const int iclass)
	{
		m_spike_plot_mode = spike_plot_mode;
		m_selected_class = iclass;
	}

	int GetDisplayMode() const { return m_display_mode; }
	float GetTimeFirst() const { return m_tFirst; }
	float GetTimeLast() const { return m_tLast; }
	float GetAmplitudeSpan() const { return m_mV_span; }
	int GetSpikePlotMode() const { return m_spike_plot_mode; }
	int GetSpikeClass() const { return m_selected_class; }

	ChartData* GetDataViewCurrentRecord();
	AcqDataDoc* GetVisibleRowsAcqDataDocAt(int index) { return ptr_rows[index]->pdataDoc; }
	CSpikeDoc* GetVisibleRowsSpikeDocAt(int index) { return ptr_rows[index]->pspikeDoc; }
	int GetVisibleRowsSize() { return ptr_rows.GetSize(); }

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
