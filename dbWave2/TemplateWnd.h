#pragma once

#include "chart.h"
#if !defined mytype
#define mytype	double
#endif

class CTemplateWnd : public CChartWnd
{
	DECLARE_SERIAL(CTemplateWnd)

	//Construction
public:
	CTemplateWnd();
	virtual			~CTemplateWnd() override;
	CTemplateWnd& operator = (const CTemplateWnd& arg);
	void			Serialize(CArchive& ar) override;

	// Attributes
public:
	// user defined parameters
	CString			m_csID;
	int				m_classID;
	float			m_ktolerance;	// user defined value
	double			m_globalstd;	// parameter to be set by parent
	double			m_power;		// power of the signal

	inline	int		GetNitems() const { return m_nitems; }
	inline	void	SetGlobalstd(double* globalstd) { m_globalstd = *globalstd; m_bValid = FALSE; }
	inline	void	SetkTolerance(float* ktolerance) { m_ktolerance = *ktolerance; m_bValid = FALSE; }

protected:
	BOOL			m_bValid;		// tell if pMean data are valid
	int				m_tpllen;		// length of templates array
	// template array - intermediary results : sum, sum2
	// length = total spike length (defined by m_tpllen)
	mytype* m_pSumArray;	// array declared for longs
	mytype* m_pSUM0;		// array with sum (X)
	mytype* m_pSUM20;		// array with sum (X * X)
	long			m_nitems;		// n elements

	// template array - results and envelope
	// length = total spike length
	// mean, sup value, low value (as computed by tSetdisplayData())
	// combined arrays
	int* m_pAvg;			// first point of the array
	int* m_pMax0;		// upper limit
	int* m_pMin0;		// lower limit

	// Operations
public:
	void			SetTemplateLength(int len, int extent = -1, int org = -1);
	void			tSetdisplayData();	// fill pMean data
	void			tInit();			// init parameters, erase data pSum & pMean
	void			tAddSpikeTopSum(short* p_source);
	double			tPowerOfpSum();
	BOOL			tGetNumberOfPointsWithin(short* p_source, int* hitrate);
	double			tMinDist(short* p_source, int* ioffsetmin, BOOL bJitter = FALSE);
	void			tGlobalstats(double* gstd, double* gdist);

protected:
	double			tDist(short* p_source);
	void			DeleteArrays();

	// Implementation : display
protected:
	CArray<CPoint, CPoint> m_ptsAvg;
	CArray<CPoint, CPoint> m_ptsArea;		// points displayed with polyline

	void 			InitPolypointAbcissa();
	void 			FillOrdinatesAtscale(BOOL bScale);
	void			GetExtents();
public:
	void			PlotDatatoDC(CDC* p_dc) override;

	// Generated message map functions
protected:
	afx_msg void	OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void	OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void	OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL	OnSetCursor(CWnd* p_wnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};
