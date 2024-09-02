//////////////////////////////////////////////////////////////////////
// InPlaceEdit.h : header file
//
// MFC Grid Control - inplace editing class
//
// Written by Chris Maunder <chris@codeproject.com>
// Copyright (c) 1998-2005. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name and all copyright
// notices remains intact.
//
// An email letting me know how you are using it would be nice as well.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with GridCtrl v2.10+
//
//////////////////////////////////////////////////////////////////////

#pragma once

class GridInPlaceEdit : public CEdit
{
public:
	GridInPlaceEdit(CWnd* p_parent, CRect& rect, DWORD dw_style, UINT nID,
	             int nRow, int nColumn, CString sInitText, UINT nFirstChar);
	~GridInPlaceEdit() override;

	void EndEdit();

private:
	int m_nRow;
	int m_nColumn;
	CString m_sInitText;
	UINT m_nLastChar;
	BOOL m_bExitOnArrows;
	CRect m_Rect;

protected:
	BOOL PreTranslateMessage(MSG* pMsg) override;
	void PostNcDestroy() override;

	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT n_flags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT n_flags);
	afx_msg UINT OnGetDlgCode();
	DECLARE_MESSAGE_MAP()
};
