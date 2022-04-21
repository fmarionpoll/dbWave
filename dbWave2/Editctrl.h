#pragma once


class CEditCtrl : public CEdit
{
	// Construction
public:
	CEditCtrl();
	~CEditCtrl() override;

	BOOL m_bEntryDone{ false };
	UINT m_nChar{ 0 };
	
	BOOL ProcessKeys(UINT nChar);
	void OnEnChange(CFormView* parent_wnd, float& parameter, float delta_up, float delta_down);
	void OnEnChange(CFormView* parent_wnd, int& parameter, int delta_up, int delta_down);

protected:
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	DECLARE_MESSAGE_MAP()
};
