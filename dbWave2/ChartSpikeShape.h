#pragma once


constexpr auto DISPLAY_TIMEINTERVAL = 0; // mask: ON: display spikes within spk index range/OFF=within time interval;
constexpr auto DISPLAY_INDEXRANGE = 1; // mask: ON: display spikes within spk index range/OFF=within time interval;
constexpr auto DISP_SELECTEDCLASS = 2; // mask: ON: display only selected spike/OFF=all classes

class ChartSpikeShapeWnd : public ChartWnd
{
	//Construction
public:
	ChartSpikeShapeWnd();
	~ChartSpikeShapeWnd() override;
	DECLARE_SERIAL(ChartSpikeShapeWnd)
	void Serialize(CArchive& ar) override;

	// parameters

protected:
	int m_rangemode = RANGE_TIMEINTERVALS; // display range (time OR storage index)
	long m_lFirst = 0; // time first (real time = index/sampling rate)
	long m_lLast = 0; // time last
	int m_spkfirst{}; // index first spike
	int m_spklast{}; // index last spike
	int m_currentclass = -1; // selected class (different color) (-1 = display all)
	int m_selectedspike = -1;  // selected spike (display differently)
	int m_colorselectedspike = RED_COLOR; // color selected spike (index / color table)
	int m_hitspk = -1; // index spike
	int m_selclass{}; // index class selected
	BOOL m_btrackCurve = FALSE; // track curve ?
	BOOL m_bText = FALSE; // allow text default false
	int m_ndisplayedspikes{}; // number of spikes displayed
	BOOL m_ballFiles = FALSE; // display data from all files in CdbWaveDoc*
	CArray<CPoint, CPoint> polypoints_; // points displayed with polyline

	SpikeList* p_spikelist_ = nullptr;
	CdbWaveDoc* p_dbwave_doc_ = nullptr;

	// Helpers
public:
	void SetRangeMode(int mode) { m_rangemode = mode; }
	int GetRangeMode() const { return m_rangemode; }

	void SetPlotMode(int mode, int selclass)
	{
		m_plotmode = mode;
		m_selclass = selclass;
	}

	int GetPlotMode() const { return m_plotmode; }

	void SetSelClass(int selclass) { m_selclass = selclass; }
	int GetSelClass() const { return m_selclass; }

	void SetTextOption(BOOL bText) { m_bText = bText; }
	BOOL GetTextOption() const { return m_bText; }

	void SetSourceData(SpikeList* p_spk_list, CdbWaveDoc* p_document)
	{
		p_dbwave_doc_ = p_document;
		p_spikelist_ = p_spk_list;
		m_selectedspike = -1;
	}

	void SetSpkList(SpikeList* p_spk_list) { p_spikelist_ = p_spk_list; }

	void SetCurrentClass(int curcla) { m_currentclass = curcla; }
	int GetCurrentClass() const { return m_currentclass; }

	void SetTimeIntervals(long l_first, long l_last)
	{
		m_lFirst = l_first;
		m_lLast = l_last;
	}

	long GetTimeFirst() const { return m_lFirst; }
	long GetTimeLast() const { return m_lLast; }

	void SetSpkIndexes(int first, int last) { m_spkfirst = first, m_spklast = last; }
	int GetHitSpike() const { return m_hitspk; }
	int GetSelectedSpike() const { return m_selectedspike; }

	void DisplayAllFiles(BOOL bON, CdbWaveDoc* p_document)
	{
		m_ballFiles = bON;
		p_dbwave_doc_ = p_document;
	}

	// non-inline functions
	void MoveVTtrack(int itrack, int newval);

	float GetDisplayMaxMv();
	float GetDisplayMinMv();
	float GetExtent_mV();
	float GetExtent_ms();

	int SelectSpikeShape(int spikeno);
	BOOL IsSpikeWithinRange(int spikeno) const;

	void Print(CDC* p_dc, CRect* rect);
	int DisplayExData(short* p_data, int color = 9);
	void DisplayFlaggedSpikes(BOOL bHighLight);
	void PlotDataToDC(CDC* p_dc) override;
	void ZoomData(CRect* prevRect, CRect* newRect) override;

protected:
	void initPolypointAbcissa();
	void fillPolypointOrdinates(short* lpSource);
	void getExtents();
	void getExtentsCurrentSpkList();

	int hitCurve(CPoint point) override;
	int hitCurveInDoc(CPoint point);
	void drawSelectedSpike(int nospike, CDC* p_dc);
	void drawFlaggedSpikes(CDC* p_dc);
	void plotArraytoDC(CDC* p_dc, short* pspk);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
