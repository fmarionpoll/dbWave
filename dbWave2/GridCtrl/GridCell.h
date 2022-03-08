/////////////////////////////////////////////////////////////////////////////
// GridCell.h : header file
//
// MFC Grid Control - Grid cell class header file
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
// For use with CGridCtrl v2.20+
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CGridCtrl;
#include "GridCellBase.h"

// Each cell contains one of these. Fields "row" and "column" are not stored since we
// will usually have acces to them in other ways, and they are an extra 8 bytes per
// cell that is probably unnecessary.

class CGridCell : public CGridCellBase
{
	friend class CGridCtrl;
	DECLARE_DYNCREATE(CGridCell)

	CGridCell();
	~CGridCell() override;

	void operator=(const CGridCell& cell);

	void SetText(LPCTSTR sz_text) override { m_strText = sz_text; }
	void SetImage(int nImage) override { m_nImage = nImage; }
	void SetData(LPARAM lParam) override { m_lParam = lParam; }
	void SetGrid(CGridCtrl* pGrid) override { m_pGrid = pGrid; }

	void SetFormat(DWORD nFormat) override { m_nFormat = nFormat; }
	void SetTextClr(COLORREF clr) override { m_crFgClr = clr; }
	void SetBackClr(COLORREF clr) override { m_crBkClr = clr; }
	void SetFont(const LOGFONT* plf) override;
	void SetMargin(UINT nMargin) override { m_nMargin = nMargin; }
	CWnd* GetEditWnd() const override { return m_pEditWnd; }

	void SetCoords(int /*nRow*/, int /*nCol*/) override
	{
	} // don't need to know the row and
	// column for base implementation

	LPCTSTR GetText() const override { return (m_strText.IsEmpty()) ? _T("") : static_cast<LPCTSTR>(m_strText); }
	int GetImage() const override { return m_nImage; }
	LPARAM GetData() const override { return m_lParam; }
	CGridCtrl* GetGrid() const override { return m_pGrid; }

	DWORD GetFormat() const override;
	COLORREF GetTextClr() const override { return m_crFgClr; } // TODO: change to use default cell
	COLORREF GetBackClr() const override { return m_crBkClr; }
	LOGFONT* GetFont() const override;
	CFont* GetFontObject() const override;
	UINT GetMargin() const override;

	BOOL IsEditing() const override { return m_bEditing; }
	BOOL IsDefaultFont() const override { return (m_plfFont == nullptr); }
	void Reset() override;

	// editing cells
public:
	BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar) override;
	void EndEdit() override;
protected:
	void OnEndEdit() override;

protected:
	CString m_strText{};			// Cell text (or binary data if you wish...)
	LPARAM m_lParam{};				// 32-bit value to associate with item
	int m_nImage = 0;				// Index of the list view item�s icon
	DWORD m_nFormat{};
	COLORREF m_crFgClr{};
	COLORREF m_crBkClr{};
	LOGFONT* m_plfFont = nullptr;
	UINT m_nMargin{};

	BOOL m_bEditing{};

	CGridCtrl* m_pGrid{};
	CWnd* m_pEditWnd{};
};

