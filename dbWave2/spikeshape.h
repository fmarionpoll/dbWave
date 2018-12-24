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
	
	BOOL	m_bText;			// allow text default false
	int		m_ndisplayedspikes{};	// number of spikes displayed
	BOOL	m_ballFiles;		// display data from all files in CdbWaveDoc*
	CArray <CPoint, CPoint> polypoints_;		// points displayed with polyline
	CSpikeList*	p_spikelist_ ;			// spike list from which spikes are read
	CdbWaveDoc* p_doc_ ;

// data helpers (inline)
public:
	void SetRangeMode(int mode) {m_rangemode = mode;}
	int  GetRangeMode() const {return m_rangemode;}

	void SetPlotMode(int mode, int selclass) {m_plotmode = mode; m_selclass = selclass;}
	int  GetPlotMode() const {return m_plotmode;}

	void SetSelClass(int selclass) {m_selclass = selclass;}
	int  GetSelClass() const {return m_selclass;}

	void SetTextOption(BOOL bText) {m_bText=bText;}
	BOOL GetTextOption() const {return m_bText;}

	void SetSourceData(CSpikeList* p_spk_list) { p_spikelist_ = p_spk_list; m_selectedspike = -1; }
	void SetSpkList(CSpikeList* p_spk_list) {p_spikelist_ = p_spk_list;}

	void SetCurrentClass(int curcla) {m_currentclass=curcla;}
	int  GetCurrentClass() const {return m_currentclass;}
	
	void SetTimeIntervals(long l_first, long l_last) {m_lFirst = l_first;m_lLast = l_last;}
	long GetTimeFirst() const {return m_lFirst;}
	long GetTimeLast() const {return m_lLast;}

	void SetSpkIndexes(int first, int last) 	{m_spkfirst=first, m_spklast=last;}
	int	GetHitSpike() const { return m_hitspk; }
	int	GetSelectedSpike() const { return m_selectedspike; }
	void DisplayAllFiles(BOOL bON, CdbWaveDoc* p_document) { m_ballFiles = bON; p_doc_ = p_document; }

	// non-inline functions
	void		MoveVTtrack(int itrack, int newval);
	
	float 		GetDisplayMaxMv();
	float 		GetDisplayMinMv();
	float 		GetExtent_mV();
	float 		GetExtent_ms();	

	int	 		SelectSpikeShape(int spikeno);
	BOOL		IsSpikeWithinRange(int spikeno) const;

public:
	void 		Print(CDC* p_dc, CRect* rect);
	int			DisplayExData(short* p_data, int color=9);
	void		DisplayFlaggedSpikes (BOOL bHighLight);
	
protected:	
	void		ZoomData(CRect* r1, CRect* r2);
	void 		InitPolypointAbcissa();
	void 		FillPolypointOrdinates(short* lpSource);
	void		GetExtents();
	void		GetExtentsCurrentSpkList();

	int  		DoesCursorHitCurve(CPoint point) const;
	void		DrawSelectedSpike(int nospike, CDC* p_dc);
	void		DrawFlaggedSpikes(CDC* p_dc);
	void		PlotArraytoDC(CDC* p_dc, short* pspk);
public:
	void		PlotDatatoDC(CDC* p_dc);	
protected:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()	
};
