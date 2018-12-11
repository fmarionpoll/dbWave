#pragma once

// spkhistp.h : header file
// CSpikeHistWnd button
//
// CSpikeHistWnd build, stores and display a list of histograms
// each histogram has the same abcissa limits (same nb of bins also)
// they are intended to store and display histograms of parameters measured
// from spike classes and follow the same display conventions as CSpikFormButton
// and CSpikBarsButton.
// one histogram (DWORD array) is composed of
//		index 0 = "class" index (int)
//		index 1 to nbins = data

class CSpikeHistWnd : public CScopeScreen
{
// Construction
public:
	CSpikeHistWnd();
    virtual ~CSpikeHistWnd();

/////////////////////////////////////////////////////////////////////////////
// parameters
protected:
	CPtrArray	m_pHistarray;	// array of DWord array containing histogram
    CSpikeList* m_pSL;

	long 	m_lFirst;			// time index of first pt displayed
	long 	m_lLast;			// time index of last pt displayed
	int  	m_hitspk;			// no of spike selected
	int		m_selclass;			// index class selected
	
    int  	m_binsize{};			// size of one bin
    int 	m_abcissaminval;	// minimum value from which histogram is built (abcissa)
    int 	m_abcissamaxval;	// maximum value (abcissa max)
    int  	m_nbins;			// n bins within histogram    

   	DWORD 	m_lmax;				// value max   	
	int  	m_imax{};				// index max
	int  	m_ifirst{};			// index first interval with data
	int  	m_ilast{};			// index last interval with data
	
	
// Helpers and public procedures
public:	
	inline void SetPlotMode(int mode, int selclass) {m_plotmode = mode; m_selclass = selclass;}
	
	inline void SetTimeIntervals(long lFirst, long lLast) {m_lFirst = lFirst;m_lLast = lLast;}
	inline long GetTimeFirst() const {return m_lFirst;}
	inline long GetTimeLast() const {return m_lLast;}	
	
	inline int GetBinsize() const { return m_binsize;}
	inline int GetBinMinval() const { return m_abcissaminval;}
	inline int GetBinMaxVal() const { return m_abcissamaxval;}
	inline int GetnBins() const {return m_nbins;}	
	inline int   GetHistMaxPos() const {return m_imax;}
	inline DWORD GetHistMax() const {return m_lmax;}
		
	void BuildHistFromWordArray(CWordArray* pVal, CDWordArray* pTime, CWordArray* pspkclass,
				long lFirst, long lLast, int max, int min, int nbins,
    			BOOL bNew=TRUE);

	void RemoveHistData();
	LPTSTR ExportAscii(LPTSTR lp);					// export ascii data
	void MoveVTtagtoVal(int itag, int ival);
	void MoveHZtagtoVal(int itag, int ival);

	inline void SetSpkList(CSpikeList* pSpkList) {m_pSL = pSpkList;}

// implementation
protected:
	void ZoomData(CRect* r1, CRect* r2);
	int  DoesCursorHitCurve(CPoint point);
	void ReSize_And_Clear_Histograms(int nbins, int max, int min);
	void GetHistogLimits(int ihist);
    void GetClassArray(int iclass, CDWordArray*& pDW);
public:
	void PlotDatatoDC(CDC* pDC);
protected:
	void GetExtents();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};                     
