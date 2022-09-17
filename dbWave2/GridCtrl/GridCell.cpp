#include "StdAfx.h"
#include "GridCell.h"
#include "GridInPlaceEdit.h"
#include "GridCtrl.h"

// GridCell.cpp : implementation file
//
// MFC Grid Control - Main grid cell class
//
// Provides the implementation for the "default" cell type of the
// grid control. Adds in cell editing.
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
// For use with GridCtrl v2.20+
//
// History:
// Eric Woodruff - 20 Feb 2000 - Added PrintCell() plus other minor changes
// Ken Bertelson - 12 Apr 2000 - Split GridCell into GridCell and GridCellBase
// <kenbertelson@hotmail.com>
// C Maunder     - 17 Jun 2000 - Font handling optimsed, Added GridDefaultCell
//
/////////////////////////////////////////////////////////////////////////////




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(GridCell, GridCellBase)

GridCell::GridCell()
{
	GridCell::Reset();
}

GridCell::~GridCell()
{
	delete m_plfFont;
	delete m_pEditWnd;
}

void GridCell::operator=(const GridCell& cell)
{
	if (this != &cell) GridCellBase::operator=(cell);
}

void GridCell::Reset()
{
	GridCellBase::Reset();
}

void GridCell::SetFont(const LOGFONT* plf)
{
	if (plf == nullptr)
	{
		delete m_plfFont;
		m_plfFont = nullptr;
	}
	else
	{
		if (!m_plfFont)
			m_plfFont = new LOGFONT;
		if (m_plfFont)
			memcpy(m_plfFont, plf, sizeof(LOGFONT));
	}
}

LOGFONT* GridCell::GetFont() const
{
	if (m_plfFont == nullptr)
	{
		const auto pDefaultCell = static_cast<GridDefaultCell*>(GetDefaultCell());
		if (!pDefaultCell)
			return nullptr;

		return pDefaultCell->GetFont();
	}

	return m_plfFont;
}

CFont* GridCell::GetFontObject() const
{
	// If the default font is specified, use the default cell implementation
	if (m_plfFont == nullptr)
	{
		const auto pDefaultCell = static_cast<GridDefaultCell*>(GetDefaultCell());
		if (!pDefaultCell)
			return nullptr;

		return pDefaultCell->GetFontObject();
	}
	static CFont Font;
	Font.DeleteObject();
	Font.CreateFontIndirect(m_plfFont);
	return &Font;
}

DWORD GridCell::GetFormat() const
{
	if (m_nFormat == static_cast<DWORD>(-1))
	{
		const auto pDefaultCell = static_cast<GridDefaultCell*>(GetDefaultCell());
		if (!pDefaultCell)
			return 0;

		return pDefaultCell->GetFormat();
	}

	return m_nFormat;
}

UINT GridCell::GetMargin() const
{
	if (m_nMargin == static_cast<UINT>(-1))
	{
		const auto pDefaultCell = static_cast<GridDefaultCell*>(GetDefaultCell());
		if (!pDefaultCell)
			return 0;

		return pDefaultCell->GetMargin();
	}

	return m_nMargin;
}

BOOL GridCell::Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar)
{
	if (m_bEditing)
	{
		if (m_pEditWnd)
			m_pEditWnd->SendMessage(WM_CHAR, nChar);
	}
	else
	{
		DWORD dw_style = ES_LEFT;
		if (GetFormat() & DT_RIGHT)
			dw_style = ES_RIGHT;
		else if (GetFormat() & DT_CENTER)
			dw_style = ES_CENTER;

		m_bEditing = TRUE;

		// InPlaceEdit auto-deletes itself
		GridCtrl* pGrid = GetGrid();
		m_pEditWnd = new GridInPlaceEdit(pGrid, rect, dw_style, nID, nRow, nCol, GetText(), nChar);
	}
	return TRUE;
}

void GridCell::EndEdit()
{
	if (m_pEditWnd)
		((GridInPlaceEdit*)m_pEditWnd)->EndEdit();
}

void GridCell::OnEndEdit()
{
	m_bEditing = FALSE;
	m_pEditWnd = nullptr;
}

