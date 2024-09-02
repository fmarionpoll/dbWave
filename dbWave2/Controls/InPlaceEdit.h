#pragma once
#include <afxwin.h>

class InPlaceEdit : public CEdit
{
public:
    InPlaceEdit(CWnd* parent, int iItem, int iSubItem, CString sInitText);
    InPlaceEdit(CWnd* p_parent, CRect& rect, DWORD dwStyle, UINT nID,
        int nRow, int nColumn, CString sInitText, UINT nFirstChar);
    ~InPlaceEdit() override;

    void EndEdit();

protected:
	BOOL PreTranslateMessage(MSG* pMsg) override;
    virtual void PostNcDestroy();

public:
    CRect   m_Rect;

private:
    int m_iItem = 0;
    int m_iSubItem = 1;
    CString m_sInitText = _T("");
    BOOL m_bESC = false;
    CWnd* m_parent = nullptr;

    int     m_nRow;
    int     m_nColumn;
    UINT    m_nLastChar;
    BOOL    m_bExitOnArrows;

protected:
    afx_msg void OnKillFocus(CWnd* p_new_wnd);
    afx_msg void OnNcDestroy();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT n_flags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()
};

