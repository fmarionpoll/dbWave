#pragma once

// CRuler

class CRuler : public CObject
{
public:
	CRuler();
	virtual ~CRuler();
	void			SetRange(float* dfirst, float* dlast);
	inline int		GetScaleUnitPixels(int cx) { return (int)(m_dscaleinc * cx / (m_dlast - m_dfirst)); };
	inline double	GetScaleIncrement() { return m_dscaleinc; };
	void			UpdateRange(float* dfirst, float* dlast);

	BOOL	m_bHorizontal;		// orientation
	double	m_dfirst;			// actual lower value displayed
	double	m_dlast;			// actual higher value displayed
	double	m_dscaleinc;		// length of the major scale
	double	m_dscalefirst;		// first major scale
	double	m_dscalelast;		// last major scale

protected:
	BOOL	AdjustScale();
};

// CRulerBar
class CChartDataWnd;

class CRulerBar : public CWnd
{
	DECLARE_DYNAMIC(CRulerBar)
public:
	CRulerBar();
	virtual ~CRulerBar();
protected:
	virtual void	PreSubclassWindow();
private:
	COLORREF		m_penColor;
	CFont			m_hFont;
	BOOL			m_bHorizontal;
	CRect			m_rcClient;
	BOOL			m_bCaptured;
	BOOL			m_bBottom;
	int				m_captureMode;
	CPoint			oldpt;
	CPoint			newpt;
	CRuler*			m_pRuler;
	CChartDataWnd*	m_pChartDataWnd;

public:
	virtual BOOL	Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
		DWORD dw_style, const RECT& rect, CWnd* pParentWnd,
		UINT nID, CCreateContext* pContext = nullptr);
	BOOL			Create(CWnd* pParentWnd, CChartDataWnd* pChartDataWnd, BOOL bAsXAxis, int dSize, UINT nID);
	void			DrawScalefromRuler(CRuler* pRuler);
	inline void		AttachScopeWnd(CChartDataWnd* pChartDataWnd, BOOL bXaxis) { m_pChartDataWnd = pChartDataWnd; m_bHorizontal = bXaxis; }

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* p_dc);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
