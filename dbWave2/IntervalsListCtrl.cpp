#include "StdAfx.h"
#include "IntervalsListCtrl.h"
#include "InPlaceEdit.h"

IMPLEMENT_DYNCREATE(CIntervalsListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CIntervalsListCtrl, CListCtrl)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY()
END_MESSAGE_MAP()


void CIntervalsListCtrl::init_listbox(const CString header1, int size1, const CString header2, int size2)
{
	const DWORD dw_style = GetExtendedStyle();
	SetExtendedStyle(dw_style | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// Add bitmap to display stim on/off
	m_image_list = new CImageList;
	m_image_list->Create(16, 16, ILC_COLOR, 2, 2);
	CBitmap bm1, bm2;
	bm1.LoadBitmap(IDB_STIMON);
	m_image_list->Add(&bm1, nullptr);
	bm2.LoadBitmap(IDB_STIMOFF);
	m_image_list->Add(&bm2, nullptr);

	// add 2 columns (icon & time)
	LVCOLUMN lv_column;
	lv_column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	lv_column.iOrder = 1;
	lv_column.cx = size2;
	lv_column.pszText = const_cast<LPTSTR>((LPCTSTR)header2);
	lv_column.fmt = LVCFMT_LEFT;
	InsertColumn(0, &lv_column);

	lv_column.iOrder = 0;
	lv_column.cx = size1;
	lv_column.pszText = const_cast<LPTSTR>((LPCTSTR)header1);
	lv_column.fmt = LVCFMT_LEFT;
	InsertColumn(0, &lv_column);

	SetImageList(m_image_list, LVSIL_SMALL);
	
}

void CIntervalsListCtrl::set_item(int index, float time_interval)
{
	LVITEM lvi{};
	CString cs0, cs1;
	set_sub_item_0_value(lvi, index, cs0);
	set_sub_item_1_value(lvi, index, time_interval, cs1);
	SetItem(&lvi);
}

void CIntervalsListCtrl::set_item_index(int index)
{
	LVITEM lvi;
	CString cs;
	set_sub_item_0_value(lvi, index, cs);
	SetItem(&lvi);
}

void CIntervalsListCtrl::set_item_value(int index, float time_interval)
{
	LVITEM lvi{};
	CString cs;
	set_sub_item_1_value(lvi, index, time_interval, cs);
	SetItem(&lvi);
}

void CIntervalsListCtrl::add_new_item(int index, float time_interval)
{
	LVITEM lvi{};
	CString cs0, cs1;
	set_sub_item_0_value(lvi, index, cs0);
	InsertItem(&lvi);
	set_sub_item_1_value(lvi, index, time_interval, cs1);
	SetItem(&lvi);
}

int CIntervalsListCtrl::get_index_item_selected() const
{
	int item_index = -1;
	auto pos = GetFirstSelectedItemPosition();
	if (pos != nullptr)
		item_index = GetNextSelectedItem(pos);
	return item_index;
}

int CIntervalsListCtrl::select_item(const int index)
{
	if (index < 0)
	{
		for (int j = 0; j < GetItemCount(); ++j)
			SetItemState(j, 0, 0);
	}
	else
	{
		SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		SetFocus();
	}
	return index;
}

float CIntervalsListCtrl::get_item_value(int item) const
{
	CString cs = GetItemText(item, 1);
	const float value = static_cast<float>(_ttof(cs));
	return value;
}

int CIntervalsListCtrl::get_item_index(int item) const
{
	CString cs = GetItemText(item, 0);
	const int value = _ttoi(cs);
	return value;
}

void CIntervalsListCtrl::set_edit_overlay_value(int item_selected)
{
	CString cs = GetItemText(item_selected, 1);
	m_p_edit->SetWindowTextW(cs);
}

float CIntervalsListCtrl::get_edit_value()
{
	CString cs;
	m_p_edit->GetWindowText(cs);
	const int item_selected = get_index_item_selected();
	SetItemText(item_selected, 1, cs);
	const auto value = static_cast<float>(_ttof(cs));
	return value;
}

void CIntervalsListCtrl::enable_edit_overlay()
{
	const int item_selected = get_index_item_selected();
	if (item_selected < 0) return;

	CString cs = GetItemText(item_selected, 1);
	m_p_edit = new CInPlaceEdit (item_selected, 1, cs);
	set_edit_overlay_value(item_selected);
	move_edit_overlay_over_selected_item(item_selected);
	init_edit_overlay();

	mode_edit = true;
}

void CIntervalsListCtrl::move_edit_overlay_over_selected_item(int item_selected)
{
	CRect rect;
	GetSubItemRect(item_selected, 1, LVIR_LABEL, rect);
	const int column_width = GetColumnWidth(1);
	rect.right = rect.left + column_width;
	MapWindowPoints(this, rect);
	m_p_edit->MoveWindow(&rect);
}

void CIntervalsListCtrl::init_edit_overlay()
{
	m_p_edit->ShowWindow(SW_SHOW);
	m_p_edit->SetFocus();
	m_p_edit->SetSel(0, -1, FALSE);
}

void CIntervalsListCtrl::disable_edit_overlay()
{
	m_p_edit->CloseWindow();
	mode_edit = false;
	delete m_p_edit;
}

void CIntervalsListCtrl::validate_edit_overlay()
{
	CString cs;
	m_p_edit->GetWindowText(cs);
	const float value = static_cast<float>(_ttof(cs));

	const int item_selected = get_index_item_selected();
	set_item_value(item_selected, value);
}

void CIntervalsListCtrl::set_sub_item_0_value(LVITEM& lvi, const int item, CString& cs)
{
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	cs.Format(_T("%i"), item);
	lvi.pszText = const_cast<LPTSTR>((LPCTSTR)cs);
	lvi.iImage = item % 2;
}

void CIntervalsListCtrl::set_sub_item_1_value(LVITEM& lvi, int iItem, float time_interval, CString& cs)
{
	lvi.iItem = iItem;
	lvi.iSubItem = 1;
	lvi.mask = LVIF_TEXT;
	cs.Format(_T("%10.3f"), time_interval);
	lvi.pszText = const_cast<LPTSTR>((LPCTSTR)cs);
}

// ------------------------------------

int CIntervalsListCtrl::GetRowFromPoint(CPoint& point, int* col) const
{
	int column = 0;
	int row = HitTest(point, NULL);

	if (col) *col = 0;
	if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
		return row;

	// Get the top and bottom row visible   
	row = GetTopIndex();
	int bottom = row + GetCountPerPage();

	if (bottom > GetItemCount())
	{
		bottom = GetItemCount();
	}

	// Get the number of columns    
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();

	// Loop through the visible rows    
	for (; row <= bottom; row++)
	{
		// Get bounding rectangle of item and check whether point falls in it.
		CRect rect;
		GetItemRect(row, &rect, LVIR_BOUNDS);

		if (rect.PtInRect(point))
		{
			// Find the column      
			for (column = 0; column < nColumnCount; column++)
			{
				int colwidth = GetColumnWidth(column);

				if (point.x >= rect.left && point.x <= (rect.left + colwidth))
				{
					if (col) *col = column;
					return row;
				}

				rect.left += colwidth;
			}
		}
	}

	return -1;
}

// from codeguru:
// https://www.codeguru.com/cplusplus/editable-subitems/

// HitTestEx	- Determine the row index and column index for a point
// Returns	- the row index or -1 if point is not over a row
// point	- point to be tested.
// col		- to hold the column index
int CIntervalsListCtrl::HitTestEx(CPoint& point, int* col) const
{
	int colnum = 0;
	int row = HitTest(point, NULL);

	if (col) *col = 0;

	// Make sure that the ListView is in LVS_REPORT
	if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
		return row;

	// Get the top and bottom row visible
	row = GetTopIndex();
	int bottom = row + GetCountPerPage();
	if (bottom > GetItemCount())
		bottom = GetItemCount();

	// Get the number of columns
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();

	// Loop through the visible rows
	for (; row <= bottom; row++)
	{
		// Get bounding rect of item and check whether point falls in it.
		CRect rect;
		GetItemRect(row, &rect, LVIR_BOUNDS);
		if (rect.PtInRect(point))
		{
			// Now find the column
			for (colnum = 0; colnum < nColumnCount; colnum++)
			{
				int colwidth = GetColumnWidth(colnum);
				if (point.x >= rect.left
					&& point.x <= (rect.left + colwidth))
				{
					if (col) *col = colnum;
					return row;
				}
				rect.left += colwidth;
			}
		}
	}
	return -1;
}

CEdit* CIntervalsListCtrl::EditSubLabel(int nItem, int nCol)
{
	if (!EnsureVisible(nItem, TRUE)) 
		return NULL;

	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if (nCol >= nColumnCount || GetColumnWidth(nCol) < 5) 
		return NULL;

	// Get the column offset        
	int offset = 0;
	for (int i = 0; i < nCol; i++)
	{
		offset += GetColumnWidth(i);
	}

	CRect rect;
	GetItemRect(nItem, &rect, LVIR_BOUNDS);

	// Scroll horizontally if we need to expose the column  
	CRect rcClient;
	GetClientRect(&rcClient);

	if (offset + rect.left < 0 || offset + rect.left > rcClient.right)
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll(size);
		rect.left -= size.cx;
	}

	// Get Column alignment 
	LV_COLUMN lvcol;
	lvcol.mask = LVCF_FMT;
	GetColumn(nCol, &lvcol);
	DWORD dwStyle;

	if ((lvcol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
	{
		dwStyle = ES_LEFT;
	}
	else if ((lvcol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
	{
		dwStyle = ES_RIGHT;
	}
	else
	{
		dwStyle = ES_CENTER;
	}

	rect.left += offset + 4;
	rect.right = rect.left + GetColumnWidth(nCol) - 3;

	if (rect.right > rcClient.right)
	{
		rect.right = rcClient.right;
	}

	dwStyle |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;

	CEdit* pEdit = new CInPlaceEdit(nItem, nCol, GetItemText(nItem, nCol));
	pEdit->Create(dwStyle, rect, this, IDC_LIST1);

	return pEdit;
}

void CIntervalsListCtrl::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto* plvDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* plvItem = &plvDispInfo->item;

	if (plvItem->pszText != NULL)
	{
		SetItemText(plvItem->iItem, plvItem->iSubItem, plvItem->pszText);
	}
	*pResult = FALSE;
}

void CIntervalsListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	int index;
	CListCtrl::OnLButtonDown(nFlags, point);

	int colnum;
	if ((index = HitTestEx(point, &colnum)) != -1)
	{
		UINT flag = LVIS_FOCUSED;
		if ((GetItemState(index, flag) & flag) == flag && colnum > 0)
		{
			// Add check for LVS_EDITLABELS
			if (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS)
				m_p_edit = EditSubLabel(index, colnum);
		}
		else
			SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED,
				LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void CIntervalsListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (GetFocus() != this) SetFocus();
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CIntervalsListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (GetFocus() != this) SetFocus();
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}
