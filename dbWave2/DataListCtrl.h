#pragma once

#include "ChartWnd.h"
#include "Envelope.h"
#include "chanlistitem.h"
#include "ChartData.h"
#include "DataListCtrl_Row.h"

#define COL_INDEX		1
#define COL_CURVE		COL_INDEX+1
#define COL_INSECT		COL_INDEX+2
#define COL_SENSI		COL_INDEX+3
#define COL_STIM1		COL_INDEX+4
#define COL_CONC1		COL_INDEX+5
#define COL_STIM2		COL_INDEX+6
#define COL_CONC2		COL_INDEX+7
#define COL_NBSPK		COL_INDEX+8
#define COL_FLAG		COL_INDEX+9

class CDataListCtrl : public CListCtrl
{
public:
	CDataListCtrl();
	~CDataListCtrl() override;

	void InitColumns(CUIntArray* picolwidth = nullptr);
	void SetCurSel(int recposition);
	void UpdateCache(int ifirst, int ilast);
	void RefreshDisplay();
	void ResizeSignalColumn(int npixels);
	void FitColumnsToSize(int npixels);

	void SetTransformMode(int imode) { m_data_transform = imode; }
	void SetDisplayMode(int imode) { m_display_mode = imode; }

	void SetTimeIntervals(float tfirst, float tlast)
	{
		m_tFirst = tfirst;
		m_tLast = tlast;
	}

	void SetAmplitudeSpan(float vspan) { m_mV_span = vspan; } // TODO ; get extent, mode from current line
	void SetDisplayFileName(BOOL flag) { m_b_display_file_name = flag; }
	void SetAmplitudeAdjustMode(BOOL flag) { m_b_set_mV_span = flag; }
	void SetTimespanAdjustMode(BOOL flag) { m_b_set_time_span = flag; }

	void SetSpikePlotMode(int imode, int iclass)
	{
		m_spike_plot_mode = imode;
		m_selected_class = iclass;
	}

	int GetDisplayMode() const { return m_display_mode; }
	float GetTimeFirst() const { return m_tFirst; }
	float GetTimeLast() const { return m_tLast; }
	float GetAmplitudeSpan() const { return m_mV_span; }
	int GetSpikePlotMode() const { return m_spike_plot_mode; }
	int GetSpikeClass() const { return m_selected_class; }

	ChartData* GetDataViewCurrentRecord();
	int GetVisibleRowsSize() { return ptr_rows.GetSize(); }
	AcqDataDoc* GetVisibleRowsAcqDataDocAt(int index) { return ptr_rows[index]->pdataDoc; }
	CSpikeDoc* GetVisibleRowsSpikeDocAt(int index) { return ptr_rows[index]->pspikeDoc; }

protected:
	CArray<CDataListCtrl_Row*, CDataListCtrl_Row*> ptr_rows;
	CImageList m_image_list;
	static int m_column_width[NCOLS];
	static CString m_column_headers[NCOLS];
	static int m_column_format[NCOLS];
	static int m_column_index[NCOLS];

	CUIntArray* m_p_columns_width = nullptr;
	CBitmap* m_p_empty_bitmap = nullptr;

	int m_cx = 400; // image height
	int m_cy = 50; // image width
	int m_data_transform = 0;
	int m_display_mode = 1;
	int m_spike_plot_mode =PLOT_BLACK;
	int m_selected_class = 0;
	float m_tFirst = 0.f;
	float m_tLast = 0.f;
	float m_mV_span = 0.f;
	BOOL m_b_set_time_span = false;
	BOOL m_b_set_mV_span = false;
	BOOL m_b_display_file_name = false;

	void deletePtrArray();
	void resizePtrArray(int nitems);
	void setEmptyBitmap(BOOL bForcedUpdate = FALSE);
	void displaySpikeWnd(CDataListCtrl_Row* ptr, int iImage);
	void displayDataWnd(CDataListCtrl_Row* ptr, int iImage);
	void displayEmptyWnd(CDataListCtrl_Row* ptr, int iImage);

	// Generated message map functions
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
};
