#pragma once

/////////////////////////////////////////////////////////////////////////////
// GridCellCombo.h : header file
//
// MFC Grid Control - Grid combo cell class header file
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
// For use with GridCtrl v2.10
//
//////////////////////////////////////////////////////////////////////


class GridCellComboFMP : public GridCell
{
	friend class GridCtrl;
	DECLARE_DYNCREATE(GridCellComboFMP)

	GridCellComboFMP();
	~GridCellComboFMP();

	BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar) override;
	CWnd* GetEditWnd() const override;
	void EndEdit() override;

	CSize GetCellExtent(CDC* p_dc) override;

	void SetOptions(const CStringArray& ar);
	void SetStyle(DWORD dw_style) { m_dwStyle = dw_style; }
	DWORD GetStyle() const { return m_dwStyle; }
	int SetCurSel(int sel);
	int GetCurSel();

protected:
	BOOL Draw(CDC* p_dc, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE) override;

	CStringArray	m_Strings;
	DWORD			m_dwStyle;
};

