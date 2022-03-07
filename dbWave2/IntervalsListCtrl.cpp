#include "StdAfx.h"
#include "IntervalsListCtrl.h"



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

void CIntervalsListCtrl::set_sub_item_1(LVITEM& lvi, int iItem, float time_interval, CString& cs)
{
	lvi.iItem = iItem;
	lvi.iSubItem = 1;
	lvi.mask = LVIF_TEXT;
	cs.Format(_T("%10.3f"), time_interval);
	lvi.pszText = const_cast<LPTSTR>((LPCTSTR)cs);
}

void CIntervalsListCtrl::set_list_control_item(int i, float time_interval)
{
	LVITEM lvi{};
	CString cs0, cs1;
	set_sub_item_0(lvi, i, cs0);
	set_sub_item_1(lvi, i, time_interval, cs1);
	SetItem(&lvi);
}

void CIntervalsListCtrl::add_new_item(int i, float time_interval)
{
	LVITEM lvi{};
	CString cs0, cs1;
	set_sub_item_0(lvi, i, cs0);
	InsertItem(&lvi);
	set_sub_item_1(lvi, i, time_interval, cs1);
	SetItem(&lvi);
}

void CIntervalsListCtrl::set_sub_item_0_value(int i)
{
	LVITEM lvi;
	CString cs;
	set_sub_item_0(lvi, i, cs);
	SetItem(&lvi);
}

void CIntervalsListCtrl::set_sub_item_1_value(int index, float time_interval)
{
	LVITEM lvi{};
	CString cs;
	set_sub_item_1(lvi, index, time_interval, cs);

	SetItem(&lvi);
}

int CIntervalsListCtrl::get_index_item_selected()
{
	int item_index = -1;
	auto pos = GetFirstSelectedItemPosition();
	if (pos != nullptr)
		item_index = GetNextSelectedItem(pos);
	return item_index;
}

int CIntervalsListCtrl::select_item(int i)
{
	if (i < 0)
	{
		for (int j = 0; j < GetItemCount(); ++j)
			SetItemState(j, 0, 0);
	}
	else
	{
		SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		SetFocus();
	}
	return i;
}

void CIntervalsListCtrl::set_edit_value()
{
	m_edit_control.SetFocus();
	m_edit_control.SetSel(0, -1, FALSE);
	const int index_selected = get_index_item_selected();
	CString cs = get_sub_item_1(index_selected);
	m_edit_control.SetWindowTextW(cs);
}

float CIntervalsListCtrl::get_edit_value()
{
	CString cs;
	m_edit_control.GetWindowText(cs);
	const float value = _ttof(cs);

	const int item_selected = get_index_item_selected();
	set_sub_item_1_value(item_selected, value);
	return value;
}

void CIntervalsListCtrl::set_active_edit_overlay()
{
	CRect rect;
	const int item_selected = get_index_item_selected();
	GetSubItemRect(item_selected, 1, LVIR_LABEL, rect);
	const int column_width = GetColumnWidth(1);
	rect.right = rect.left + column_width;
	MapWindowPoints(this, rect);
	m_edit_control.MoveWindow(&rect);
	m_edit_control.ShowWindow(SW_SHOW);
	mode_edit = true;
}

void CIntervalsListCtrl::set_inactive_edit_overlay()
{
	m_edit_control.ShowWindow(SW_HIDE);
	mode_edit = false;
}

void CIntervalsListCtrl::set_sub_item_0(LVITEM& lvi, const int item, CString& cs)
{
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	cs.Format(_T("%i"), item);
	lvi.pszText = const_cast<LPTSTR>((LPCTSTR)cs);
	lvi.iImage = item % 2;
}

CString CIntervalsListCtrl::get_sub_item_1(int item)
{
	const int i = GetSelectionMark();
	if (i != -1)
	{
		CString strSubItem = GetItemText(0, 1);
		return strSubItem;
	}
	return nullptr;
}
