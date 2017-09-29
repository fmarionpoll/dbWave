#pragma once

// CRuler
class CRuler : public CObject
{
	//DECLARE_SERIAL(CRuler);
public:
	void			SetRange(float* dfirst, float* dlast);
	inline int		GetScaleUnitPixels(int cx) {return (int) (m_dscaleinc* cx / (m_dlast - m_dfirst));};
	inline double	GetScaleIncrement() {return m_dscaleinc;};
	 
	BOOL		m_bHorizontal;

	// defines which data are actually displayed
	double		m_dfirst;			// actual lower value displayed
	double		m_dlast;			// actual higher value displayed

	// defines how to print the tick marks and scale
	double		m_dscaleinc;		// length of the major scale
	double		m_dscalefirst;		// first major scale
	double		m_dscalelast;		// last major scale

protected:
	BOOL AdjustScale();
};

// CRulerBar

class CRulerBar : public CWnd
{
	DECLARE_DYNAMIC(CRulerBar)

public:
	CRulerBar();
	virtual ~CRulerBar();

	void			SetRange(float* dfirst, float* dlast);
	int				GetScaleUnitPixels();
	inline double	GetScaleIncrement() {return m_ruler.m_dscaleinc;};
	
private: 
	COLORREF	m_penColor;
	CFont		m_hFont;
	BOOL		m_bHorizontal;
	CRuler		m_ruler;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void PreSubclassWindow();
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
};

