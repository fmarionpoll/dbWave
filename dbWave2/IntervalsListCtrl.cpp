#include "StdAfx.h"
#include "IntervalsListCtrl.h"
#include "InPlaceEdit.h"
#include "GridCtrl.h"

IMPLEMENT_DYNCREATE(CIntervalsListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CIntervalsListCtrl, CListCtrl)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_INPLACE_CONTROL, on_end_label_edit)
END_MESSAGE_MAP()


CIntervalsListCtrl::~CIntervalsListCtrl()
{
	SAFE_DELETE(m_image_list)
}

void CIntervalsListCtrl::init_listbox(const CString& header1, const int size1, const CString& header2, const int size2)
{
	const DWORD dw_style = GetExtendedStyle();
	SetExtendedStyle(dw_style | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// Add bitmap to display stimulus on/off
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

void CIntervalsListCtrl::set_item(const int index, const float time_interval)
{
	LVITEM lvi{};
	CString cs0, cs1;
	set_sub_item_0_value(lvi, index, cs0);
	set_sub_item_1_value(lvi, index, time_interval, cs1);
	SetItem(&lvi);
}

void CIntervalsListCtrl::set_item_index(const int index)
{
	LVITEM lvi;
	CString cs;
	set_sub_item_0_value(lvi, index, cs);
	SetItem(&lvi);
}

void CIntervalsListCtrl::set_item_value(const int index, const float time_interval)
{
	LVITEM lvi{};
	CString cs;
	set_sub_item_1_value(lvi, index, time_interval, cs);
	SetItem(&lvi);
}

void CIntervalsListCtrl::add_new_item(const int index, const float time_interval)
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

float CIntervalsListCtrl::get_item_value(const int item) const
{
	const CString cs = GetItemText(item, 1);
	const float value = static_cast<float>(_ttof(cs));
	return value;
}

int CIntervalsListCtrl::get_item_index(const int item) const
{
	const CString cs = GetItemText(item, 0);
	const int value = _ttoi(cs);
	return value;
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

void CIntervalsListCtrl::set_sub_item_1_value(LVITEM& lvi, const int i_item, const float time_interval, CString& cs)
{
	lvi.iItem = i_item;
	lvi.iSubItem = 1;
	lvi.mask = LVIF_TEXT;
	cs.Format(_T("%10.3f"), time_interval);
	lvi.pszText = const_cast<LPTSTR>((LPCTSTR)cs);
}

// ------------------------------------

// from code_guru:
// https://www.codeguru.com/cplusplus/editable-subitems/

// HitTestEx	- Determine the row index and column index for a point
// Returns	- list ctrl_row = row index (-1 if point is not over a row) + list ctrl_column
// point	- point to be tested.
// col		- to hold the column index

int CIntervalsListCtrl::hit_test_ex(const CPoint& point_to_be_tested, int* column) const
{
	int row = HitTest(point_to_be_tested, nullptr);
	if (column) *column = 0;
	if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
		return row;

	row = GetTopIndex();
	int bottom = row + GetCountPerPage();
	if (bottom > GetItemCount())
		bottom = GetItemCount();

	const auto p_header = static_cast<CHeaderCtrl*>(GetDlgItem(0));
	const int column_count = p_header->GetItemCount();

	// Loop through the visible rows
	for (; row <= bottom; row++)
	{
		// Get bounding rect of item and check whether point_to_be_tested falls in it.
		CRect rect;
		GetItemRect(row, &rect, LVIR_BOUNDS);
		if (rect.PtInRect(point_to_be_tested))
		{
			// Now find the column
			for (int column_index = 0; column_index < column_count; column_index++)
			{
				const int column_width = GetColumnWidth(column_index);
				if (point_to_be_tested.x >= rect.left
					&& point_to_be_tested.x <= (rect.left + column_width))
				{
					if (column) *column = column_index;
					return row;
				}
				rect.left += column_width;
			}
		}
	}
	return -1;
}

CEdit* CIntervalsListCtrl::edit_sub_label(const int n_item, const int n_col)
{
	if (!EnsureVisible(n_item, TRUE)) 
		return nullptr;

	const auto p_header = static_cast<CHeaderCtrl*>(GetDlgItem(0));
	const int column_count = p_header->GetItemCount();
	if (n_col >= column_count || GetColumnWidth(n_col) < 5) 
		return nullptr;

	// Get the column offset        
	int column_left_offset = 0;
	for (int i = 0; i < n_col; i++)
		column_left_offset += GetColumnWidth(i);

	CRect rect;
	GetItemRect(n_item, &rect, LVIR_BOUNDS);

	// Scroll horizontally if we need to expose the column  
	CRect rc_client;
	GetClientRect(&rc_client);

	if (column_left_offset + rect.left < 0 || column_left_offset + rect.left > rc_client.right)
	{
		CSize size;
		size.cx = column_left_offset + rect.left;
		size.cy = 0;
		Scroll(size);
		rect.left -= size.cx;
	}

	// Get Column alignment 
	LV_COLUMN lv_column;
	lv_column.mask = LVCF_FMT;
	GetColumn(n_col, &lv_column);
	DWORD dw_style;

	if ((lv_column.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
		dw_style = ES_LEFT;
	else if ((lv_column.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
		dw_style = ES_RIGHT;
	else
		dw_style = ES_CENTER;

	rect.left += column_left_offset + 4;
	rect.right = rect.left + GetColumnWidth(n_col) - 3;

	if (rect.right > rc_client.right)
		rect.right = rc_client.right;

	dw_style |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;

	const auto pEdit = new InPlaceEdit(this, n_item, n_col, GetItemText(n_item, n_col));
	pEdit->Create(dw_style, rect, this, IDC_INPLACE_CONTROL);

	return pEdit;
}

void CIntervalsListCtrl::on_end_label_edit(NMHDR* p_nmhdr, LRESULT* p_result)
{
	const auto* p_lv_display_info = (LV_DISPINFO*)p_nmhdr;
	const LV_ITEM* p_lv_item = &p_lv_display_info->item;

	if (p_lv_item->pszText != nullptr)
		SetItemText(p_lv_item->iItem, p_lv_item->iSubItem, p_lv_item->pszText);
	
	*p_result = FALSE;
}

void CIntervalsListCtrl::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	CListCtrl::OnLButtonDown(n_flags, point);

	int column;
	int row;
	if ((row = hit_test_ex(point, &column)) != -1)
	{
		constexpr UINT flag = LVIS_FOCUSED;
		if ((GetItemState(row, flag) & flag) == flag && column > 0)
		{
			if (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS)
				m_p_edit = edit_sub_label(row, column);
		}
		else
			SetItemState(row, LVIS_SELECTED | LVIS_FOCUSED,
				LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void CIntervalsListCtrl::OnHScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar* p_scroll_bar)
{
	if (GetFocus() != this) SetFocus();
	CListCtrl::OnHScroll(n_sb_code, n_pos, p_scroll_bar);
}

void CIntervalsListCtrl::OnVScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar* p_scroll_bar)
{
	if (GetFocus() != this) SetFocus();
	CListCtrl::OnVScroll(n_sb_code, n_pos, p_scroll_bar);
}
