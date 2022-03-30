#pragma once

#include <afxwin.h>

#define IDC_COMBOEDIT 1001

class ComboEdit : public CEdit
{
	// Construction
public:
	ComboEdit();
	BOOL PreTranslateMessage(MSG* pMsg) override;
	~ComboEdit() override;

protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
};
