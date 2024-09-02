#pragma once

#include <afxwin.h>

#define IDC_COMBOEDIT 1001

class GridComboEdit : public CEdit
{
	// Construction
public:
	GridComboEdit();
	BOOL PreTranslateMessage(MSG* pMsg) override;
	~GridComboEdit() override;

protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT n_flags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT n_flags);

	DECLARE_MESSAGE_MAP()
};
