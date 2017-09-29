
#pragma once

// spikexyp.h headerfile
// CSpikeXYpWnd button


class CSpikeXYpWnd : public CScopeScreen
{
//Construction
public:
	CSpikeXYpWnd();
	virtual ~CSpikeXYpWnd();

/////////////////////////////////////////////////////////////////////////////////////
// parameters
protected:
	int		m_rangemode;		// display range
	long 	m_lFirst;			// time index of first pt displayed
	long 	m_lLast;			// time index of last pt displayed
	int		m_spkfirst;			// index first spike
	int		m_spklast;			// index last spike
	int  	m_currentclass;		// current class in case of displaying classes
	int  	m_selectedspike;	// selected spike (display differently)
	int  	m_hitspk;			// no of spike selected
	int		m_selclass;			// index class selected
	BOOL 	m_btrackCurve;		// track curve ?	
	
	int		m_rwidth;			// dot width

	CWordArray*  m_pparm;		// parameter value
	CWordArray*  m_pclass;		// class value
	CDWordArray* m_piitime;		// time index	
	CSpikeList*	m_pSL;				// spike list (source data)	
	
// Helpers and public procedures
public:
	inline void SetRangeMode(int mode) {m_rangemode = mode;}
	inline void SetPlotMode(int mode, int selclass) {m_plotmode = mode; m_selclass = selclass;}
	inline void SetSelClass(int selclass) {m_selclass = selclass;}
	inline void SetTimeIntervals(long lFirst, long lLast) {m_lFirst = lFirst;m_lLast = lLast;}
	inline void SetCurrentClass(int curcla) {m_currentclass=curcla;}
	inline void SetSpkIndexes(int first, int last) {m_spkfirst=first, m_spklast=last;}
	inline void SetSpkList(CSpikeList* pSpkList) {m_pSL = pSpkList;}
	
	inline int  GetCurrentClass() const {return m_currentclass;}
	inline long GetTimeFirst() const {return m_lFirst;}
	inline long GetTimeLast() const {return m_lLast;}
	inline int	GetSelectedSpike() const {return m_selectedspike;}
	inline int	GetHitSpike() const {return m_hitspk;}

	void SetSourceData(CWordArray* pparms, CDWordArray* piitime, CWordArray* pclass, CSpikeList* pSpkList);
	int	 SelectSpike(int spikeno);
	void DisplaySpike(int nospike, BOOL bselect);	
	BOOL IsSpikeWithinRange(int spikeno);
	void MoveHZtagtoVal(int itag, int ival);
	void MoveVTtagtoVal(int itag, int ival);

// implementation
protected:
	void DrawSelectedSpike(int nospike, int color, CDC* pDC);
	void ZoomData(CRect* prevRect, CRect* newRect);
	int  DoesCursorHitCurve(CPoint point);
public:
	void PlotDatatoDC(CDC* pDC);
protected:
	void GetExtents();
	void HighlightOnePoint(int nospike, CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
	
};

