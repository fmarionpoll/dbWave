#pragma once

class Ruler;
class ChartData;


class RulerBar : public CWnd
{
	DECLARE_DYNAMIC(RulerBar)
public:
	RulerBar();
	~RulerBar() override;
protected:
	void PreSubclassWindow() override;
private:
	COLORREF m_penColor { GetSysColor(COLOR_WINDOWTEXT) };
	CFont m_hFont;
	BOOL m_bHorizontal {-1};
	CRect m_rcClient;
	BOOL m_bCaptured {false};
	BOOL m_bBottom {true};
	int m_captureMode {-1};
	CPoint oldpt;
	CPoint newpt;
	Ruler* m_pRuler { nullptr };
	ChartData* m_pChartDataWnd { nullptr };
	
public:
	BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
	            DWORD dw_style, const RECT& rect, CWnd* pParentWnd,
	            UINT nID, CCreateContext* pContext = nullptr) override;
	BOOL Create(CWnd* pParentWnd, ChartData* pDataChartWnd, BOOL bAsXAxis, int dSize, UINT nID);
	void DrawScalefromRuler(Ruler* pRuler);

	void AttachScopeWnd(ChartData* pDataChartWnd, BOOL bXaxis)
	{
		m_pChartDataWnd = pDataChartWnd;
		m_bHorizontal = bXaxis;
	}

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* p_dc);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
