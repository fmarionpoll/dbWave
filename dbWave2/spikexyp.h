#pragma once

// CSpikeXYpWnd button

class CSpikeXYpWnd : public CScopeScreen
{
//Construction
public:
	CSpikeXYpWnd();
	virtual ~CSpikeXYpWnd();

// parameters
protected:
	int		m_rangemode;		// display range
	long 	m_lFirst;			// time index of first pt displayed
	long 	m_lLast;			// time index of last pt displayed
	int		m_spkfirst{};		// index first spike
	int		m_spklast{};		// index last spike
	int  	m_currentclass;		// current class in case of displaying classes
	int  	m_selectedspike;	// selected spike (display differently)
	int  	m_hitspk;			// no of spike selected
	int		m_selclass{};		// index class selected
	BOOL 	m_btrackCurve;		// track curve ?	
	int		m_rwidth;			// dot width

	CSpikeList*	m_pspikelist_;		// spike list (source data)	

	long time_max_	=0;
	long time_min_	=0;
	int value_max_	=0;
	int value_min_	=0;

// Helpers and public procedures
public:
	void	SetRangeMode(int mode) {m_rangemode = mode;}
	void	SetPlotMode(int mode, int selclass) {m_plotmode = mode; m_selclass = selclass;}
	void	SetSelClass(int selclass) {m_selclass = selclass;}
	void	SetTimeIntervals(long l_first, long l_last) {m_lFirst = l_first;m_lLast = l_last;}
	void	SetCurrentClass(int curcla) {m_currentclass=curcla;}
	void	SetSpkIndexes(int first, int last) {m_spkfirst=first, m_spklast=last;}
	void	SetSpkList(CSpikeList* p_spk_list) {m_pspikelist_ = p_spk_list;}
	
	int		GetCurrentClass() const {return m_currentclass;}
	long	GetTimeFirst() const {return m_lFirst;}
	long	GetTimeLast() const {return m_lLast;}
	int		GetSelectedSpike() const {return m_selectedspike;}
	int		GetHitSpike() const {return m_hitspk;}

	void	SetSourceData(CSpikeList* p_spk_list);
	int		SelectSpike(int spikeno);
	void	DisplaySpike(int nospike, BOOL bselect);	
	BOOL	IsSpikeWithinRange(int spikeno);
	void	MoveHZtagtoVal(int itag, int ival);
	void	MoveVTtagtoVal(int itag, int ival);

	void	PlotDatatoDC(CDC* p_dc) override;

protected:
	void	DrawSelectedSpike(int nospike, int color, CDC* p_dc);
	void	ZoomData(CRect* prevRect, CRect* newRect) override;
	int		DoesCursorHitCurve(CPoint point);
	BOOL	is_spike_within_limits(int ispike);
	void	GetExtents();
	void	HighlightOnePoint(int nospike, CDC* p_dc);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
DECLARE_MESSAGE_MAP()
};

