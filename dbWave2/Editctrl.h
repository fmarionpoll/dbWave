#pragma once


class CEditCtrl : public CEdit
{
	// Construction
public:
	CEditCtrl();

	// Attributes
public:
	BOOL	m_bEntryDone;
	UINT	m_nChar;

	// Operations
public:
protected:

	// Implementation
public:
	virtual ~CEditCtrl();
	BOOL ProcessKeys(UINT nChar);

	// Generated message map functions
protected:
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	DECLARE_MESSAGE_MAP()
};
