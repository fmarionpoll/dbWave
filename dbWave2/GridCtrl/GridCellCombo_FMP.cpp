#include "StdAfx.h"
#include "GridCell.h"
#include "GridCtrl.h"
#include "GridInPlaceList.h"
#include "GridCellCombo_FMP.h"

// GridCellCombo.cpp : implementation file
//
// MFC Grid Control - Main grid cell class
//
// Provides the implementation for a combobox cell type of the
// grid control.
//
// Written by Chris Maunder <chris@codeproject.com>
// Copyright (c) 1998-2005. All Rights Reserved.
//
// Parts of the code contained in this file are based on the original
// GridInPlaceList from http://www.codeguru.com/listview
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
// For use with GridCtrl v2.22+
//
// History:
// 6 Aug 1998 - Added ComboEdit to subclass the edit control - code
//              provided by Roelf Werkman <rdw@inn.nl>. Added nID to
//              the constructor param list.
// 29 Nov 1998 - bug fix in onkeydown (Markus Irtenkauf)
// 13 Mar 2004 - GetCellExtent fixed by Yogurt
//
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE


static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(GridCellComboFMP, GridCell)

GridCellComboFMP::GridCellComboFMP() : GridCell()
{
	m_dwStyle = CBS_DROPDOWN; // CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE, CBS_SORT
	SetStyle(m_dwStyle);
}

GridCellComboFMP::~GridCellComboFMP()
= default;

// Create a control to do the editing
BOOL GridCellComboFMP::Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar)
{
	m_bEditing = TRUE;
	m_pEditWnd = new GridInPlaceList(GetGrid(), rect, GetStyle(), nID, nRow, nCol,
	                                  GetTextClr(), GetBackClr(), m_Strings, GetText(), nChar);
	return TRUE;
}

CWnd* GridCellComboFMP::GetEditWnd() const
{
	if (m_pEditWnd && (m_pEditWnd->GetStyle() & CBS_DROPDOWNLIST) != CBS_DROPDOWNLIST)
		return &((GridInPlaceList*)m_pEditWnd)->m_edit;

	return nullptr;
}

// Cancel the editing.
void GridCellComboFMP::EndEdit()
{
	if (m_pEditWnd)
		((GridInPlaceList*)m_pEditWnd)->EndEdit();
}

CSize GridCellComboFMP::GetCellExtent(CDC* p_dc)
{
	const CSize sizeScroll(GetSystemMetrics(SM_CXVSCROLL), GetSystemMetrics(SM_CYHSCROLL));
	CSize sizeCell(GridCell::GetCellExtent(p_dc));
	sizeCell.cx += sizeScroll.cx;
	sizeCell.cy = max(sizeCell.cy, sizeScroll.cy);
	return sizeCell;
}

// Override draw so that when the cell is selected, a drop arrow is shown in the RHS.
BOOL GridCellComboFMP::Draw(CDC* p_dc, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd /*=TRUE*/)
{
#ifdef _WIN32_WCE
	return GridCell::Draw(p_dc, nRow, nCol, rect, bEraseBkgnd);
#else
	// Cell selected?
	//if ( !IsFixed() && IsFocused())
	if (GetGrid()->IsCellEditable(nRow, nCol) && !IsEditing())
	{
		// Get the size of the scroll box
		const CSize sizeScroll(GetSystemMetrics(SM_CXVSCROLL), GetSystemMetrics(SM_CYHSCROLL));

		// enough room to draw?
		if (sizeScroll.cy < rect.Width() && sizeScroll.cy < rect.Height())
		{
			// Draw control at RHS of cell
			CRect ScrollRect = rect;
			ScrollRect.left = rect.right - sizeScroll.cx;
			ScrollRect.bottom = rect.top + sizeScroll.cy;

			// Do the draw
			p_dc->DrawFrameControl(ScrollRect, DFC_SCROLL, DFCS_SCROLLDOWN);

			// Adjust the remaining space in the cell
			rect.right = ScrollRect.left;
		}
	}

	const CString strTempText = GetText();
	if (IsEditing())
		SetText(_T(""));

	// drop through and complete the cell drawing using the base class' method
	const BOOL bResult = GridCell::Draw(p_dc, nRow, nCol, rect, bEraseBkgnd);

	if (IsEditing())
		SetText(strTempText);

	return bResult;
#endif
}

// For setting the strings that will be displayed in the drop list
void GridCellComboFMP::SetOptions(const CStringArray& ar)
{
	m_Strings.RemoveAll();
	for (int i = 0; i < ar.GetSize(); i++)
		m_Strings.Add(ar[i]);
}

// mimic SetCurSel from CComboBox
// FMP (aug 4, 2004)
// Parameters
// nSelect
// Specifies the zero-based index of the string to select. If –1, any current selection
// in the list box is removed and the edit control is cleared.
// Return Value
// The zero-based index of the item selected if the message is successful. The return value
// is CB_ERR if nSelect is greater than the number of items in the list or if nSelect is set to –1,
// which clears the selection.
//
// Remarks
// If necessary, the list box scrolls the string into view (if the list box is visible).
// The text in the edit control of the combo box is changed to reflect the new selection.
// Any previous selection in the list box is removed.

int GridCellComboFMP::SetCurSel(int sel)
{
	// check index
	if (sel >= m_Strings.GetSize())
		return CB_ERR; // returns error code if out of range

	// if OK, cancel edit, replace content and select
	EndEdit();
	if (sel < 0)
		SetText(_T(""));
	else
		SetText(m_Strings.GetAt(sel));

	return sel;
}

// Call this member function to determine which item in the combo box is selected.
//	int GetCurSel( ) const;
// Return Value
//	The zero-based index of the currently selected item in the list box of a combo box,
//	or CB_ERR if no item is selected.
// Remarks
//	GetCurSel returns an index into the list.

int GridCellComboFMP::GetCurSel()
{
	const CString cs = GetText();
	int current_selection = CB_ERR;
	if (!cs.IsEmpty())
	{
		for (int i = 0; i < m_Strings.GetSize(); i++)
		{
			if (cs.Compare(m_Strings[i]) == 0)
			{
				current_selection = i;
				break;
			}
		}
	}
	return current_selection;
}
