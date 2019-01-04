#pragma once

//CSpikeBarWnd button

class CSpikeBarWnd : public CScopeScreen
{
//Construction
public:
	CSpikeBarWnd();
	virtual ~CSpikeBarWnd();
	DECLARE_SERIAL( CSpikeBarWnd )
	void Serialize( CArchive& archive ) override;

// parameters

protected:
	int			m_rangemode;		// display range	
	long		m_lFirst;			// time index of first pt displayed
	long		m_lLast;			// time index of last pt displayed
	int			m_spkfirst{};		// index first spike
	int			m_spklast;			// index last spike
	int			m_currentclass;		// current class in case of displaying classes
	int			m_selectedspike;	// selected spike (display differently)
	int			m_hitspk;			// no of spike selected	
	int			m_selclass{};		// index class selected	
	BOOL		m_btrackCurve;		// track curve ?
	BOOL		m_bDisplaySignal{};	// flag default:FALSE=bars; TRUE=spikes
	int			m_selpen{};
	int			m_barheight;
	BOOL		m_ballFiles;		// display data from all files in CdbWaveDoc*
	CDWordArray* p_envelope_;		// data envelope (should be 4 times greater than size)
	CSpikeList*	p_spike_list_ = nullptr;	// spike list (source data)	
	CSpikeDoc*	p_spike_doc_ = nullptr;		// spike doc source (source data; stim)
	CdbWaveDoc* p_dbwave_doc_ = nullptr;
	
// Helpers
public:
	void	SetRangeMode(int mode)				{m_rangemode = mode;}
	void	SetSelClass(int selclass)			{m_selclass = selclass;}
	void	SetPlotMode(int mode, int selclass) {m_plotmode = mode; m_selclass = selclass;}

	void	SetSourceData(CSpikeList* p_spk_list, CSpikeDoc* p_spk_doc) {p_spike_doc_ = p_spk_doc; p_spike_list_ = p_spk_list; m_selectedspike=-1; }
	void	SetSpkList(CSpikeList* p_spk_list) {p_spike_list_ = p_spk_list;}
	void	SetTimeIntervals(long l_first, long l_last) {m_lFirst = l_first; m_lLast = l_last;}
	void	SetCurrentClass(int curcla)		{m_currentclass=curcla;}
	void	SetSpkIndexes(int first, int last) {m_spkfirst=first, m_spklast=last;}

	int		GetCurrentClass()	{return m_currentclass;}
	long	GetTimeFirst()		const {return m_lFirst;}
	long	GetTimeLast()		const {return m_lLast;}
	int		GetSelClass()		const {return m_selclass;}
	int		GetPlotMode()		const {return m_plotmode;}
	int		GetSelectedSpike()	const {return m_selectedspike;}
	int		GetHitSpike()		const {return m_hitspk;}

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

