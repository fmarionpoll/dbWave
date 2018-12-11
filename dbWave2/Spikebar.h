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
	int			m_spkfirst{};			// index first spike
	int			m_spklast;			// index last spike
	int			m_currentclass;		// current class in case of displaying classes
	int			m_selectedspike;	// selected spike (display differently)
	int			m_hitspk;			// no of spike selected	
	int			m_selclass{};			// index class selected	
	BOOL		m_btrackCurve;		// track curve ?
	CSpikeList*	m_pSL;				// spike list (source data)	
	CSpikeDoc*	m_pSDoc{};			// spike doc source (source data; stim)
	BOOL		m_bDisplaySignal{};	// flag default:FALSE=bars; TRUE=spikes
	CDWordArray* m_penvelope;		// data envelope (should be 4 times greater than size)
	int			m_selpen{};
	int			m_barheight;
	BOOL		m_ballFiles;		// display data from all files in CdbWaveDoc*
	CdbWaveDoc* m_pDoc;
	
// Helpers
public:
	inline void SetRangeMode(int mode) {m_rangemode = mode;}
	inline void SetSelClass(int selclass) {m_selclass = selclass;}
	inline void SetPlotMode(int mode, int selclass) {m_plotmode = mode; m_selclass = selclass;}

	inline void SetSourceData(CSpikeList* pSpkList, CSpikeDoc* pSpkDoc) {m_pSDoc = pSpkDoc; m_pSL = pSpkList; m_selectedspike=-1; }
	inline void SetSpkList(CSpikeList* pSpkList) {m_pSL = pSpkList;}
	inline void SetTimeIntervals(long lFirst, long lLast) {m_lFirst = lFirst; m_lLast = lLast;}
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

	inline void	DisplayAllFiles(BOOL bON, CdbWaveDoc* pDoc) { m_ballFiles = bON; m_pDoc = pDoc; }

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

