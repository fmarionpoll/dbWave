#pragma once
#include <afxwin.h>

class CInPlaceEdit : public CEdit
{
public:
    CInPlaceEdit(CWnd* parent, int iItem, int iSubItem, CString sInitText);
    ~CInPlaceEdit() override;

protected:
	BOOL PreTranslateMessage(MSG* pMsg) override;

private:
    int m_iItem = 0;
    int m_iSubItem = 1;
    CString m_sInitText = _T("");
    BOOL m_bESC = false;
    CWnd* m_parent = nullptr;

protected:
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnNcDestroy();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};

