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
// CGridInPlaceList from http://www.codeguru.com/listview
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
// For use with CGridCtrl v2.22+
//
// History:
// 6 Aug 1998 - Added CComboEdit to subclass the edit control - code 
//              provided by Roelf Werkman <rdw@inn.nl>. Added nID to 
//              the constructor param list.
// 29 Nov 1998 - bug fix in onkeydown (Markus Irtenkauf)
// 13 Mar 2004 - GetCellExtent fixed by Yogurt
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GridCell.h"
#include "GridCtrl.h"
#include "GridCellCombo_FMP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CComboEdit

CComboEdit::CComboEdit()
{
}

CComboEdit::~CComboEdit()
{
}

// Stoopid win95 accelerator key problem workaround - Matt Weagle.
BOOL CComboEdit::PreTranslateMessage(MSG* pMsg) 
{
	// Make sure that the keystrokes continue to the appropriate handlers
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		::TranslateMessage(pMsg);
		::DispatchMessage(pMsg);
		return TRUE;
	}	

	// Catch the Alt key so we don't choke if focus is going to an owner drawn button
	if (pMsg->message == WM_SYSCHAR)
		return TRUE;

	return CEdit::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CComboEdit, CEdit)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboEdit message handlers

void CComboEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	
	CGridInPlaceList* pOwner = (CGridInPlaceList*) GetOwner();  // This MUST be a CGridInPlaceList
	if (pOwner)
		pOwner->EndEdit();	
}

void CComboEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		 nChar == VK_DOWN  || nChar == VK_UP   ||
		 nChar == VK_RIGHT || nChar == VK_LEFT) &&
		(GetKeyState(VK_CONTROL) < 0 && GetDlgCtrlID() == IDC_COMBOEDIT))
	{
		CWnd* pOwner = GetOwner();
		if (pOwner)
			pOwner->SendMessage(WM_KEYDOWN, nChar, nRepCnt+ (((DWORD)nFlags)<<16));
		return;
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CComboEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE) 
	{
		CWnd* pOwner = GetOwner();
		if (pOwner)
			pOwner->SendMessage(WM_KEYUP, nChar, nRepCnt + (((DWORD)nFlags)<<16));
		return;
	}

	if (nChar == VK_TAB || nChar == VK_RETURN || nChar == VK_ESCAPE)
	{
		CWnd* pOwner = GetOwner();
		if (pOwner)
			pOwner->SendMessage(WM_KEYUP, nChar, nRepCnt + (((DWORD)nFlags)<<16));
		return;
	}

	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}


/////////////////////////////////////////////////////////////////////////////
// CGridInPlaceList

CGridInPlaceList::CGridInPlaceList(CWnd* pParent, CRect& rect, DWORD dw_style, UINT nID,
						   int nRow, int nColumn, 
						   COLORREF crFore, COLORREF crBack,
						   CStringArray& Items, CString sInitText, 
						   UINT nFirstChar)
{
	m_crForeClr = crFore;
	m_crBackClr = crBack;

	m_nNumLines = 4;
	m_sInitText = sInitText;
	m_nRow		= nRow;
	m_nCol      = nColumn;
	m_nLastChar = 0; 
	m_bExitOnArrows = FALSE; //(nFirstChar != VK_LBUTTON);	// If mouse click brought us here,

	// Create the combobox
	DWORD dwComboStyle = WS_BORDER|WS_CHILD|WS_VISIBLE|WS_VSCROLL|
						 CBS_AUTOHSCROLL | dw_style;
	int nHeight = rect.Height();
	rect.bottom = rect.bottom + m_nNumLines*nHeight + ::GetSystemMetrics(SM_CYHSCROLL);
	if (!Create(dwComboStyle, rect, pParent, nID)) return;

	// Add the strings
	for (int i = 0; i < Items.GetSize(); i++) 
		AddString(Items[i]);

	SetFont(pParent->GetFont());
	SetItemHeight(-1, nHeight);

	int nMaxLength = GetCorrectDropWidth();
	/*
	if (nMaxLength > rect.GetRectWidth())
		rect.right = rect.left + nMaxLength;
	// Resize the edit window and the drop down window
	MoveWindow(rect);
	*/

	SetDroppedWidth(nMaxLength);

	SetHorizontalExtent(0); // no horz scrolling

	// Set the initial text to m_sInitText
	if (::IsWindow(m_hWnd) && SelectString(-1, m_sInitText) == CB_ERR) 
		SetWindowText(m_sInitText);		// No text selected, so restore what was there before

	ShowDropDown();

	// Subclass the combobox edit control if style includes CBS_DROPDOWN
	if ((dw_style & CBS_DROPDOWNLIST) != CBS_DROPDOWNLIST)
	{
		m_comboedit.SubclassDlgItem(IDC_COMBOEDIT, this);
		SetFocus();
		switch (nFirstChar)
		{
			case VK_LBUTTON: 
			case VK_RETURN:   m_comboedit.SetSel((int)_tcslen(m_sInitText), -1); return;
			case VK_BACK:     m_comboedit.SetSel((int)_tcslen(m_sInitText), -1); break;
			case VK_DOWN: 
			case VK_UP:   
			case VK_RIGHT:
			case VK_LEFT:  
			case VK_NEXT:  
			case VK_PRIOR: 
			case VK_HOME:  
			case VK_END:      m_comboedit.SetSel(0,-1); return;
			default:          m_comboedit.SetSel(0,-1);
		}
		SendMessage(WM_CHAR, nFirstChar);
	}
	else
		SetFocus();
}

CGridInPlaceList::~CGridInPlaceList()
{
}

void CGridInPlaceList::EndEdit()
{
	CString str;
	if (::IsWindow(m_hWnd))
		GetWindowText(str);
 
	// Send Notification to parent
	GV_DISPINFO dispinfo;

	dispinfo.hdr.hwndFrom = GetSafeHwnd();
	dispinfo.hdr.idFrom   = GetDlgCtrlID();
	dispinfo.hdr.code     = GVN_ENDLABELEDIT;
 
	dispinfo.item.mask    = LVIF_TEXT|LVIF_PARAM;
	dispinfo.item.row     = m_nRow;
	dispinfo.item.col     = m_nCol;
	dispinfo.item.strText = str;
	dispinfo.item.lParam  = (LPARAM) m_nLastChar; 
 
	CWnd* pOwner = GetOwner();
	if (IsWindow(pOwner->GetSafeHwnd()))
		pOwner->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dispinfo );
 
	// Close this window (PostNcDestroy will delete this)
	if (::IsWindow(m_hWnd))
		PostMessage(WM_CLOSE, 0, 0);
}

int CGridInPlaceList::GetCorrectDropWidth()
{
	const int nMaxWidth = 200;  // don't let the box be bigger than this

	// Reset the dropped width
	int nNumEntries = GetCount();
	int nWidth = 0;
	CString str;

	CClientDC dc(this);
	int nSave = dc.SaveDC();
	dc.SelectObject(GetFont());

	int nScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL);
	for (int i = 0; i < nNumEntries; i++)
	{
		GetLBText(i, str);
		int nLength = dc.GetTextExtent(str).cx + nScrollWidth;
		nWidth = max(nWidth, nLength);
	}
	
	// Add margin space to the calculations
	nWidth += dc.GetTextExtent(_T("0")).cx;

	dc.RestoreDC(nSave);

	nWidth = min(nWidth, nMaxWidth);

	return nWidth;
	//SetDroppedWidth(nWidth);
}

/*
// Fix by Ray (raybie@Exabyte.COM)
void CGridInPlaceList::OnSelendOK() 
{
	int iIndex = GetCurSel(); 
	if( iIndex != CB_ERR) 
	{ 
		CString strLbText; 
		GetLBText( iIndex, strLbText); 
 
		if (!((GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)) 
		   m_edit.SetWindowText( strLbText); 
	} 
 
	GetParent()->SetFocus(); 	
}
*/

void CGridInPlaceList::PostNcDestroy() 
{
	CComboBox::PostNcDestroy();

	delete this;
}

BEGIN_MESSAGE_MAP(CGridInPlaceList, CComboBox)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropdown)
	ON_WM_GETDLGCODE()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGridInPlaceList message handlers

UINT CGridInPlaceList::OnGetDlgCode() 
{
	return DLGC_WANTALLKEYS;
}

void CGridInPlaceList::OnDropdown() 
{
	SetDroppedWidth(GetCorrectDropWidth());
}

void CGridInPlaceList::OnKillFocus(CWnd* pNewWnd) 
{
	CComboBox::OnKillFocus(pNewWnd);

	if (GetSafeHwnd() == pNewWnd->GetSafeHwnd())
		return;

	// Only end editing on change of focus if we're using the CBS_DROPDOWNLIST style
	if ((GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)
		EndEdit();
}

// If an arrow key (or associated) is pressed, then exit if
//  a) The Ctrl key was down, or
//  b) m_bExitOnArrows == TRUE
void CGridInPlaceList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		 nChar == VK_DOWN  || nChar == VK_UP   ||
		 nChar == VK_RIGHT || nChar == VK_LEFT) &&
		(m_bExitOnArrows || GetKeyState(VK_CONTROL) < 0))
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus();
		return;
	}

	CComboBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

// Need to keep a lookout for Tabs, Esc and Returns.
void CGridInPlaceList::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE) 
		SetWindowText(m_sInitText);	// restore previous text

	if (nChar == VK_TAB || nChar == VK_RETURN || nChar == VK_ESCAPE)
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus();	// This will destroy this window
		return;
	}

	CComboBox::OnKeyUp(nChar, nRepCnt, nFlags);
}

HBRUSH CGridInPlaceList::CtlColor(CDC* /*p_dc*/, UINT /*nCtlColor*/) 
{
	/*
	static CBrush brush(m_crBackClr);
	p_dc->SetTextColor(m_crForeClr);
	p_dc->SetBkMode(TRANSPARENT);
	return (HBRUSH) brush.GetSafeHandle();
	*/
	
	// TODO: Return a non-NULL brush if the parent's handler should not be called
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
// CGridCellCombo 
/////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNCREATE(CGridCellCombo, CGridCell)

CGridCellCombo::CGridCellCombo() : CGridCell()
{
	m_dwStyle = CBS_DROPDOWN;  // CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE, CBS_SORT
	SetStyle(m_dwStyle); 
}

// Create a control to do the editing
BOOL CGridCellCombo::Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar)
{
	m_bEditing = TRUE;
	
	// CGridInPlaceList auto-deletes itself
	m_pEditWnd = new CGridInPlaceList(GetGrid(), rect, GetStyle(), nID, nRow, nCol, 
								  GetTextClr(), GetBackClr(), m_Strings, GetText(), nChar);
	return TRUE;
}

CWnd* CGridCellCombo::GetEditWnd() const
{
	if (m_pEditWnd && (m_pEditWnd->GetStyle() & CBS_DROPDOWNLIST) != CBS_DROPDOWNLIST )
		return &(((CGridInPlaceList*)m_pEditWnd)->m_comboedit);

	return nullptr;
}


CSize CGridCellCombo::GetCellExtent(CDC* p_dc)
{    
	CSize sizeScroll (GetSystemMetrics(SM_CXVSCROLL), GetSystemMetrics(SM_CYHSCROLL));    
	CSize sizeCell (CGridCell::GetCellExtent(p_dc));    
	sizeCell.cx += sizeScroll.cx;    
	sizeCell.cy = max(sizeCell.cy,sizeScroll.cy);    
	return sizeCell;
}

// Cancel the editing.
void CGridCellCombo::EndEdit()
{
	if (m_pEditWnd)
		((CGridInPlaceList*)m_pEditWnd)->EndEdit();
}

// Override draw so that when the cell is selected, a drop arrow is shown in the RHS.
BOOL CGridCellCombo::Draw(CDC* p_dc, int nRow, int nCol, CRect rect,  BOOL bEraseBkgnd /*=TRUE*/)
{
#ifdef _WIN32_WCE
	return CGridCell::Draw(p_dc, nRow, nCol, rect,  bEraseBkgnd);
#else
	// Cell selected?
	//if ( !IsFixed() && IsFocused())
	if (GetGrid()->IsCellEditable(nRow, nCol) && !IsEditing())
	{
		// Get the size of the scroll box
		CSize sizeScroll(GetSystemMetrics(SM_CXVSCROLL), GetSystemMetrics(SM_CYHSCROLL));

		// enough room to draw?
		if (sizeScroll.cy < rect.Width() && sizeScroll.cy < rect.Height())
		{
			// Draw control at RHS of cell
			CRect ScrollRect = rect;
			ScrollRect.left   = rect.right - sizeScroll.cx;
			ScrollRect.bottom = rect.top + sizeScroll.cy;

			// Do the draw 
			p_dc->DrawFrameControl(ScrollRect, DFC_SCROLL, DFCS_SCROLLDOWN);

			// Adjust the remaining space in the cell
			rect.right = ScrollRect.left;
		}
	}

	CString strTempText = GetText();
	if (IsEditing())
		SetText(_T(""));

	// drop through and complete the cell drawing using the base class' method
	BOOL bResult = CGridCell::Draw(p_dc, nRow, nCol, rect,  bEraseBkgnd);

	if (IsEditing())
		SetText(strTempText);

	return bResult;
#endif
}

// For setting the strings that will be displayed in the drop list
void CGridCellCombo::SetOptions(const CStringArray& ar)
{ 
	m_Strings.RemoveAll();
	for (int i = 0; i < ar.GetSize(); i++)
		m_Strings.Add(ar[i]);
}

// mimic SetCurSel from CComboBox
// FMP (aug 4, 2004)
// Parameters
// nSelect 
// Specifies the zero-based index of the string to select. If �1, any current selection 
// in the list box is removed and the edit control is cleared. 
// Return Value
// The zero-based index of the item selected if the message is successful. The return value 
// is CB_ERR if nSelect is greater than the number of items in the list or if nSelect is set to �1, 
// which clears the selection.
// 
// Remarks
// If necessary, the list box scrolls the string into view (if the list box is visible). 
// The text in the edit control of the combo box is changed to reflect the new selection. 
// Any previous selection in the list box is removed.


int CGridCellCombo::SetCurSel(int sel)
{
	// check index
	if (sel >= m_Strings.GetSize())
		return CB_ERR;		// returns error code if out of range

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

int CGridCellCombo::GetCurSel( )
{
	CString cs = GetText();
	int cursel= CB_ERR;	
	if (!cs.IsEmpty())
	{
		for (int i=0; i< m_Strings.GetSize(); i++)
		{
			if (cs.Compare(m_Strings[i]) == 0)
			{
				cursel = i;
				break;
			}
		}
	}
	return cursel;
}