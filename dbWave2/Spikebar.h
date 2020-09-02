#pragma once

//CSpikeBarWnd button

class CSpikeBarWnd : public CScopeScreen
{
	//Construction
public:
	CSpikeBarWnd();
	virtual ~CSpikeBarWnd();
	DECLARE_SERIAL(CSpikeBarWnd)
	void Serialize(CArchive& archive) override;

	// parameters

protected:
	int			m_rangemode = RANGE_TIMEINTERVALS;		// display range
	long		m_lFirst = 0;			// time index of first pt displayed
	long		m_lLast = 0;			// time index of last pt displayed
	int			m_spkfirst = 0;			// index first spike
	int			m_spklast = 0;			// index last spike
	int			m_currentclass = 0;		// current class in case of displaying classes
	int			m_selectedspike = -1;	// selected spike (display differently)
	int			m_hitspk = -1;			// no of spike selected
	int			m_selclass{};			// index class selected
	BOOL		m_btrackCurve = false;	// track curve ?
	BOOL		m_bDisplaySignal{};		// flag default:FALSE=bars; TRUE=spikes
	int			m_selpen{};
	int			m_barheight = 10;
	BOOL		m_ballFiles = false;	// display data from all files in CdbWaveDoc*

	CDWordArray* p_envelope_ = nullptr;	// data envelope (should be 4 times greater than size)
	CSpikeList*	p_spikelist_ = nullptr;
	CSpikeDoc*	p_spike_doc_ = nullptr;
	CdbWaveDoc* p_dbwave_doc_ = nullptr;

	// Helpers
public:
	void	SetRangeMode(int mode) { m_rangemode = mode; }
	void	SetSelClass(int selclass) { m_selclass = selclass; }
	void	SetPlotMode(int mode, int selclass) { m_plotmode = mode; m_selclass = selclass; }

	void	SetSourceData(CSpikeList* p_spk_list, CdbWaveDoc* p_document) { p_dbwave_doc_ = p_document; p_spikelist_ = p_spk_list; m_selectedspike = -1; }
	void	SetSourceData(CSpikeList* p_spk_list, CSpikeDoc* p_spkdoc) { p_dbwave_doc_ = nullptr; p_spike_doc_ = p_spkdoc, p_spikelist_ = p_spk_list; m_selectedspike = -1; }
	void	SetSpkList(CSpikeList* p_spk_list) { p_spikelist_ = p_spk_list; }

	void	SetTimeIntervals(long l_first, long l_last) { m_lFirst = l_first; m_lLast = l_last; }
	void	SetCurrentClass(int curcla) { m_currentclass = curcla; }
	void	SetSpkIndexes(int first, int last) { m_spkfirst = first, m_spklast = last; }

	int		GetCurrentClass()	const { return m_currentclass; }
	long	GetTimeFirst()		const { return m_lFirst; }
	long	GetTimeLast()		const { return m_lLast; }
	int		GetSelClass()		const { return m_selclass; }
	int		GetPlotMode()		const { return m_plotmode; }
	int		GetSelectedSpike()	const { return m_selectedspike; }
	int		GetHitSpike()		const { return m_hitspk; }

	void	DisplaySpike(int nospike, BOOL bselect);
	void	DisplayFlaggedSpikes(BOOL bHighlight);
	int		SelectSpike(int spikeno);

	void	SelectSpikesWithinRect(CRect* pRect, UINT nFlags) const;
	BOOL	IsSpikeWithinRange(int spikeno);
	void	CenterCurve();
	void	MaxGain();
	void	MaxCenter();

	void	DisplayAllFiles(BOOL bON, CdbWaveDoc* p_document) { m_ballFiles = bON; p_dbwave_doc_ = p_document; }

	void 	Print(CDC* p_dc, CRect* rect);
	void	PlotDatatoDC(CDC* p_dc) override;
	void	PlotSingleSpkDatatoDC(CDC* p_dc);


protected:
	void	ZoomData(CRect* prevRect, CRect* newRect) override;
	int		DoesCursorHitCurve(CPoint point);
	int		DoesCursorHitCurveInDoc(CPoint point);
	void 	DisplayBars(CDC* p_dc, CRect* rect);
	void	HighlightOneBar(int nospike, CDC* p_dc) const;
	void	DisplayStimulus(CDC* p_dc, CRect* rect) const;

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};
