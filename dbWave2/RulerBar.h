#pragma once

// CRuler



// CRulerBar
class CRuler;
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
	BOOL			Create(CWnd* pParentWnd, CChartDataWnd* pDataChartWnd, BOOL bAsXAxis, int dSize, UINT nID);
	void			DrawScalefromRuler(CRuler* pRuler);
	inline void		AttachScopeWnd(CChartDataWnd* pDataChartWnd, BOOL bXaxis) { m_pChartDataWnd = pDataChartWnd; m_bHorizontal = bXaxis; }

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* p_dc);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
