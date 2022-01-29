#pragma once

// CScrollBarEx

class CScrollBarEx : public CScrollBar
{
	DECLARE_DYNAMIC(CScrollBarEx)

public:
	CScrollBarEx();
	~CScrollBarEx() override;
	int GetScrollHeight() { return (m_scBarInfo.rcScrollBar.bottom - m_scBarInfo.rcScrollBar.top + 1); }
	int GetScrollWidth() { return (m_scBarInfo.rcScrollBar.right - m_scBarInfo.rcScrollBar.left + 1); }
	int GetThumbSize() { return m_scBarInfo.dxyLineButton; }
	int GetThumbBottom() { return m_scBarInfo.xyThumbBottom; }
	int GetThumbTop() { return m_scBarInfo.xyThumbTop; }

private:
	BOOL m_bCaptured;
	int m_captureMode;
	SCROLLBARINFO m_scBarInfo, m_scBarInfo_old;
	SCROLLINFO m_scInfo, m_scInfo_old;
	CPoint oldpt;

public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
