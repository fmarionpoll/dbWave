#pragma once

//CSpikeBarWnd button

class CSpikeBarWnd : public CScopeScreen
{
//Construction
public:
	CSpikeBarWnd();
	virtual ~CSpikeBarWnd();
	DECLARE_SERIAL( CSpikeBarWnd )
	virtual void Serialize( CArchive& archive );

/////////////////////////////////////////////////////////////////
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
	CSpikeList*	p_spike_list_ = nullptr;		// spike list (source data)	
	CSpikeDoc*	p_spike_doc_ = nullptr;	// spike doc source (source data; stim)
	CdbWaveDoc* p_dbwave_doc_ = nullptr;
	
// Helpers
public:
	inline void SetRangeMode(int mode) {m_rangemode = mode;}
	inline void SetSelClass(int selclass) {m_selclass = selclass;}
	inline void SetPlotMode(int mode, int selclass) {m_plotmode = mode; m_selclass = selclass;}

	inline void SetSourceData(CSpikeList* p_spk_list, CSpikeDoc* p_spk_doc) {p_spike_doc_ = p_spk_doc; p_spike_list_ = p_spk_list; m_selectedspike=-1; }
	inline void SetSpkList(CSpikeList* p_spk_list) {p_spike_list_ = p_spk_list;}
	inline void SetTimeIntervals(long l_first, long l_last) {m_lFirst = l_first; m_lLast = l_last;}
	inline void SetCurrentClass(int curcla) {m_currentclass=curcla;}
	inline void SetSpkIndexes(int first, int last) {m_spkfirst=first, m_spklast=last;}

	inline int  GetCurrentClass() {return m_currentclass;}
	inline long GetTimeFirst() const {return m_lFirst;}
	inline long GetTimeLast() const {return m_lLast;}
	inline int  GetSelClass() const {return m_selclass;}
	inline int  GetPlotMode() const {return m_plotmode;}
	inline int	GetSelectedSpike() const {return m_selectedspike;}
	inline int	GetHitSpike() const {return m_hitspk;}

	void	DisplaySpike(int nospike, BOOL bselect);
	void	DisplayFlaggedSpikes(BOOL bHighlight);
	int		SelectSpike(int spikeno);

	void	SelectSpikesWithinRect(CRect* pRect, UINT nFlags);
	BOOL	IsSpikeWithinRange(int spikeno);
	void	CenterCurve();
	void	MaxGain();
	void	MaxCenter();

	inline void	DisplayAllFiles(BOOL bON, CdbWaveDoc* pDoc) { m_ballFiles = bON; p_dbwave_doc_ = pDoc; }

// implementation
public:
	void 	Print(CDC* pDC, CRect* rect);	

protected:
	virtual void ZoomData(CRect* prevRect, CRect* newRect);
public:
	virtual void PlotDatatoDC(CDC* pDC);
protected:
	int		DoesCursorHitCurve(CPoint point);

	void 	DisplayBars(CDC* pDC, CRect* rect);
	void	HighlightOneBar(int nospike, CDC* pDC);
	void	DisplayStim(CDC* pDC, CRect* rect);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
	
};

