#pragma once


/////////////////////////////////////////////////////////////////////////////
// CSpikeClassListBox window

class CSpikeClassListBox : public CListBox
{
// Construction
public:
	CSpikeClassListBox();

// Attributes
public:

protected:
	BOOL 		m_bText;
	BOOL 		m_bSpikes;
	BOOL 		m_bBars;
	int			m_leftcolwidth;
	int			m_rowheight;
	int			m_widthText;
	int			m_widthSpikes;
	int			m_widthBars;
	int			m_widthSeparator;
	COLORREF	m_clrText;
	COLORREF	m_clrBkgnd;
	CBrush		m_brBkgnd;
	int			m_topIndex;

	// logic to drag spikes
	BOOL		m_bHitspk;		// one spike is hitted
	int			m_selspikeLB;		// spike selected & hitted
	int			m_oldsel;		// selected row from which spike is hitted

	long		m_lFirst;
	long		m_lLast;
	CSpikeList* m_pSList;
	CSpikeDoc*	m_pSDoc;
	HWND		m_hwndBarsReflect;

// Operations
public:
	void SetSourceData(CSpikeList* pSList, CSpikeDoc* pSDoc);
	void SetSpkList(CSpikeList* p_spike_list);
	inline CSpikeList* GetSpkListCurrent() const {return m_pSList;};
	inline CSpikeDoc* GetSpikeDoc() const {return m_pSDoc;};

	void SetTimeIntervals(long l_first, long l_last);
	long GetTimeFirst() const {return m_lFirst;}
	long GetTimeLast() const {return m_lLast;}

	void SetRowHeight(int rowheight);
	void SetLeftColWidth(int rowwidth);
	void SetColsWidth(int coltext, int colspikes, int colseparator);

	inline int GetRowHeight() const {return m_rowheight;}
	inline int GetLeftColWidth() const {return m_leftcolwidth;}
	inline int GetColsTextWidth() const {return m_widthText;}
	inline int GetColsSpikesWidth() const {return m_widthSpikes;}
	inline int GetColsTimeWidth() const {return m_widthBars;}
	inline int GetColsSeparatorWidth() const {return m_widthSeparator;}
	float 	   GetExtent_mV();

	void SetYzoom(int y_we, int y_wo);
	void SetXzoom(int x_we, int x_wo);

	int GetYWExtent();	// load display parameters
	int GetYWOrg();		// ordinates
	int GetXWExtent();	// and
	int GetXWOrg();		// abcissa
	
	int	 SelectSpike(int spikeno);
	void ChangeSpikeClass(int spikeno, int newclass);
	int  SetMouseCursorType(int cursorm);
	void XorTempVTtag(int xpoint);
	void ResetBarsXortag ();
	void ReflectBarsMouseMoveMessg(HWND hwnd);
	
	void PrintItem(CDC* pDC, CRect* prect1, CRect* prect2, CRect* prect3,int i );

protected:
	void UpdateString(void* pptr, int iclass, int nbspikes);

// Overrides

// Implementation
public:
	virtual ~CSpikeClassListBox();
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCIS);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);	
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

