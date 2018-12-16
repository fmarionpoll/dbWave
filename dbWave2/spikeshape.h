#pragma once

// SpikeShape.h headerfile
// CSpikeShapeWnd button

#define DISPLAY_TIMEINTERVAL 0	// mask: ON: display spikes within spk index range/OFF=within time interval
#define DISPLAY_INDEXRANGE	 1	// mask: ON: display spikes within spk index range/OFF=within time interval
#define DISP_SELECTEDCLASS	2	// mask: ON: display only selected spike/OFF=all classes

class CSpikeShapeWnd : public CScopeScreen
{
//Construction
public:
	CSpikeShapeWnd();
	virtual ~CSpikeShapeWnd();
	DECLARE_SERIAL( CSpikeShapeWnd)
	virtual void Serialize( CArchive& archive );

/////////////////////////////////////////////////////////////////////////////////////
// data display operations
//form variable
protected:
	int		m_rangemode;		// display range (time OR storage index)
	long	m_lFirst;			// time first (real time = index/sampling rate)
	long	m_lLast;			// time last
	int		m_spkfirst{};			// index first spike
	int		m_spklast{};			// index last spike
	int		m_currentclass;		// selected class (different color) (-1 = display all)
	int		m_selectedspike;	// selected spike (display differently)
	int		m_colorselectedspike; // color selected spike (index / color table)
	int		m_hitspk;			// index spike
	int		m_selclass{};			// index class selected
	BOOL 	m_btrackCurve;		// track curve ?
	
	CSpikeList*	m_pSL;			// spike list from which spikes are read
	CDWordArray m_polypts;		// points displayed with polyline
	BOOL	m_bText;			// allow text default false
	int		m_ndisplayedspikes{};	// number of spikes displayed
	BOOL		m_ballFiles;		// display data from all files in CdbWaveDoc*
	CdbWaveDoc* m_pDoc;

// data helpers
public:
	inline void SetRangeMode(int mode) {m_rangemode = mode;}
	inline int  GetRangeMode() const {return m_rangemode;}

	inline void SetPlotMode(int mode, int selclass) 
				{m_plotmode = mode; m_selclass = selclass;}
	inline int  GetPlotMode() const {return m_plotmode;}

	inline void SetSelClass(int selclass) {m_selclass = selclass;}
	inline int  GetSelClass() const {return m_selclass;}

	inline void SetTextOption(BOOL bText) {m_bText=bText;}
	inline BOOL GetTextOption() const {return m_bText;}

	inline void SetSourceData(CSpikeList* p_spk_list) {m_pSL = p_spk_list; m_selectedspike=-1;}
	inline void SetSpkList(CSpikeList* p_spk_list) {m_pSL = p_spk_list;}

	inline void SetCurrentClass(int curcla) {m_currentclass=curcla;}
	inline int  GetCurrentClass() const {return m_currentclass;}
	
	inline void SetTimeIntervals(long l_first, long l_last) 
					{m_lFirst = l_first;m_lLast = l_last;}
	inline long GetTimeFirst() const {return m_lFirst;}
	inline long GetTimeLast() const {return m_lLast;}

	inline void SetSpkIndexes(int first, int last) 	{m_spkfirst=first, m_spklast=last;}
	void		MoveVTtrack(int itrack, int newval);
	
	float 		GetDisplayMaxMv();
	float 		GetDisplayMinMv();
	float 		GetExtent_mV();
	float 		GetExtent_ms();	

	int	 		SelectSpikeShape(int spikeno);
	BOOL		IsSpikeWithinRange(int spikeno);
	inline int	GetHitSpike() const {return m_hitspk;}
	inline int	GetSelectedSpike() const {return m_selectedspike;}

	inline void	DisplayAllFiles(BOOL bON, CdbWaveDoc* pDoc) { m_ballFiles = bON; m_pDoc = pDoc; }

public:
	void 		Print(CDC* pDC, CRect* rect);
	int			DisplayExData(short* pData, int color=9);
	void		DisplayFlaggedSpikes (BOOL bHighLight);
	
protected:	
	void		ZoomData(CRect* r1, CRect* r2);
	void 		InitPolypointAbcissa();
	void 		FillPolypointOrdinates(short* lpSource);
	void		GetExtents();

	int  		DoesCursorHitCurve(CPoint point);
	void		DrawSelectedSpike(int nospike, CDC* pDC);
	void		DrawFlaggedSpikes(CDC* pDC);
	void		PlotArraytoDC(CDC* pDC, short* pspk);
public:
	void		PlotDatatoDC(CDC* pDC);	
protected:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()	
};
