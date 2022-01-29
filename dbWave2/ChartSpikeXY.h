#pragma once
#include "dbWaveDoc.h"

// CChartSpikeXYWnd button

class CChartSpikeXYWnd : public ChartWnd
{
	//Construction
public:
	CChartSpikeXYWnd();
	~CChartSpikeXYWnd() override;

	// parameters
protected:
	int m_rangemode = RANGE_TIMEINTERVALS; // display range
	long m_lFirst = 0; // time index of first pt displayed
	long m_lLast = 0; // time index of last pt displayed
	int m_spkfirst{}; // index first spike
	int m_spklast{}; // index last spike
	int m_currentclass = 0; // current class in case of displaying classes
	int m_selectedspike = -1; // selected spike (display differently)
	int m_hitspk = -1; // no of spike selected
	int m_selclass{}; // index class selected
	BOOL m_btrackCurve = false; // track curve ?
	int m_rwidth = 6; // dot width
	BOOL m_ballFiles = false; // display data from all files in CdbWaveDoc*

	CSpikeList* p_spikelist_ = nullptr;
	CSpikeDoc* p_spike_doc_ = nullptr;
	CdbWaveDoc* p_dbwave_doc_ = nullptr;

	long time_max_ = 0;
	long time_min_ = 0;
	int value_max_ = 0;
	int value_min_ = 0;

	// Helpers and public procedures
public:
	void SetRangeMode(int mode) { m_rangemode = mode; }

	void SetPlotMode(int mode, int selclass)
	{
		m_plotmode = mode;
		m_selclass = selclass;
	}

	void SetSelClass(int selclass) { m_selclass = selclass; }

	void SetTimeIntervals(long l_first, long l_last)
	{
		m_lFirst = l_first;
		m_lLast = l_last;
	}

	void SetCurrentClass(int curcla) { m_currentclass = curcla; }
	void SetSpkIndexes(int first, int last) { m_spkfirst = first, m_spklast = last; }

	int GetCurrentClass() const { return m_currentclass; }
	long GetTimeFirst() const { return m_lFirst; }
	long GetTimeLast() const { return m_lLast; }
	int GetSelectedSpike() const { return m_selectedspike; }
	int GetHitSpike() const { return m_hitspk; }

	void SetSourceData(CSpikeList* p_spk_list, CdbWaveDoc* p_document)
	{
		p_dbwave_doc_ = p_document;
		p_spikelist_ = p_spk_list;
		m_selectedspike = -1;
	}

	void SetSpkList(CSpikeList* p_spk_list) { p_spikelist_ = p_spk_list; }

	int SelectSpike(int spikeno);
	void DisplaySpike(int nospike, BOOL bselect);
	BOOL IsSpikeWithinRange(int spikeno);
	void MoveHZtagtoVal(int itag, int ival);
	void MoveVTtagtoVal(int itag, int ival);

	void DisplayAllFiles(BOOL bON, CdbWaveDoc* p_document)
	{
		m_ballFiles = bON;
		p_dbwave_doc_ = p_document;
	}

	void PlotDatatoDC(CDC* p_dc) override;
	void ZoomData(CRect* prevRect, CRect* newRect) override;

protected:
	void drawSelectedSpike(int nospike, int color, CDC* p_dc);
	int hitCurve(CPoint point) override;
	int hitCurveInDoc(CPoint point);
	BOOL is_spike_within_limits(int ispike);
	void getExtents();
	void highlightOnePoint(int nospike, CDC* p_dc);
	void displayHZtags(CDC* p_dc);
	void displayVTtags(CDC* p_dc);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
