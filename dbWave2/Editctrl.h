#pragma once


class CEditCtrl : public CEdit
{
	// Construction
public:
	CEditCtrl();
	~CEditCtrl() override;

	BOOL m_b_entry_done{ false };
	UINT m_n_char{ 0 };
	
	BOOL process_keys(UINT n_char);
	void on_en_change(CWnd* parent_wnd, float& parameter, float delta_up, float delta_down);
	void on_en_change(CWnd* parent_wnd, int& parameter, int delta_up, int delta_down);
	void on_en_change(CWnd* parent_wnd, UINT& parameter, UINT delta_up, UINT delta_down);

protected:
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT n_char, UINT n_rep_cnt, UINT n_flags);
	afx_msg void OnChar(UINT n_char, UINT n_rep_cnt, UINT n_flags);
	afx_msg void OnSetFocus(CWnd* p_old_wnd);
	afx_msg void OnKillFocus(CWnd* p_new_wnd);
	afx_msg void OnVScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar);

	DECLARE_MESSAGE_MAP()
};
