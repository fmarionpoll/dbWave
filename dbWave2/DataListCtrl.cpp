#include "StdAfx.h"

#include "DataListCtrl.h"
#include <../ChartData.h>
#include <../ViewDbTable.h>

#include "DataListCtrl_Row.h"
#include "ViewdbWave.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int DataListCtrl::m_column_width_[] = {
	1,
	10, 300, 15, 30,
	30, 50, 40, 40,
	40, 40
};
CString DataListCtrl::m_column_headers_[] = {
	__T(""),
	__T("#"), __T("data"), __T("insect ID"), __T("sensillum"),
	__T("stim1"), __T("conc1"), __T("stim2"), __T("conc2"),
	__T("spikes"), __T("flag")
};

int DataListCtrl::m_column_format_[] = {
	LVCFMT_LEFT,
	LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER,
	LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER,
	LVCFMT_CENTER, LVCFMT_CENTER
};

int DataListCtrl::m_column_index_[] = {
	0,
	CTRL_COL_INDEX, CTRL_COL_CURVE, CTRL_COL_INSECT, CTRL_COL_SENSI,
	CTRL_COL_STIM1, CTRL_COL_CONC1, CTRL_COL_STIM2, CTRL_COL_CONC2,
	CTRL_COL_NBSPK, CTRL_COL_FLAG
};

BEGIN_MESSAGE_MAP(DataListCtrl, CListCtrl)

	ON_WM_VSCROLL()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDisplayInfo)
	ON_WM_DESTROY()

END_MESSAGE_MAP()

DataListCtrl::DataListCtrl()
= default;

DataListCtrl::~DataListCtrl()
{
	delete_ptr_array();
	SAFE_DELETE(infos.p_empty_bitmap)
}

void DataListCtrl::OnDestroy()
{
	save_columns_width();
}

void DataListCtrl::save_columns_width() const
{
	if (m_width_columns_ != nullptr)
	{
		const auto n_columns_stored = m_width_columns_->GetSize();
		if (n_columns_stored != N_COLUMNS)
			m_width_columns_->SetSize(N_COLUMNS);
		for (auto i = 0; i < N_COLUMNS; i++)
			m_width_columns_->SetAt(i, GetColumnWidth(i));
	}
}

void DataListCtrl::delete_ptr_array()
{
	if (ptr_rows_.GetSize() == NULL)
		return;
	const auto n_rows = ptr_rows_.GetSize();
	for (auto i = 0; i < n_rows; i++)
	{
		const auto* ptr = ptr_rows_.GetAt(i);
		SAFE_DELETE(ptr)
	}
	ptr_rows_.RemoveAll();
}

void DataListCtrl::resize_ptr_array(const int n_items)
{
	if (n_items == ptr_rows_.GetSize())
		return;

	// Resize m_image_list CImageList
	infos.image_list.SetImageCount(n_items);

	// reduce size
	if (ptr_rows_.GetSize() > n_items)
	{
		for (auto i = ptr_rows_.GetSize() - 1; i >= n_items; i--)
			delete ptr_rows_.GetAt(i);
		ptr_rows_.SetSize(n_items);
	}
	// grow size
	else
	{
		const auto size_before_change = ptr_rows_.GetSize();
		ptr_rows_.SetSize(n_items);
		auto index = 0;
		if (size_before_change > 0)
			index = ptr_rows_.GetAt(size_before_change - 1)->index;
		for (auto i = size_before_change; i < n_items; i++)
		{
			auto* ptr = new DataListCtrl_Row;
			ASSERT(ptr != NULL);
			ptr_rows_.SetAt(i, ptr);
			index++;
			ptr->index = index;
		}
	}
}

void DataListCtrl::init_columns(CUIntArray* width_columns)
{
	if (width_columns != nullptr)
	{
		m_width_columns_ = width_columns;
		const auto n_columns = width_columns->GetSize();
		if (n_columns < N_COLUMNS)
			width_columns->SetSize(N_COLUMNS);
		for (auto i = 0; i < n_columns; i++)
			m_column_width_[i] = static_cast<int>(width_columns->GetAt(i));
	}

	for (auto i = 0; i < N_COLUMNS; i++)
	{
		InsertColumn(i, m_column_headers_[i], m_column_format_[i], m_column_width_[i], -1);
	}

	infos.image_width = m_column_width_[CTRL_COL_CURVE];
	infos.image_list.Create(infos.image_width, infos.image_height, ILC_COLOR4, 10, 10);
	SetImageList(&infos.image_list, LVSIL_SMALL);
}

void DataListCtrl::OnGetDisplayInfo(NMHDR* p_nmhdr, LRESULT* p_result)
{
	auto first_array = 0;
	auto last_array = 0;
	if (ptr_rows_.GetSize() > 0)
	{
		first_array = ptr_rows_.GetAt(0)->index;
		last_array = ptr_rows_.GetAt(ptr_rows_.GetUpperBound())->index;
	}

	// is item within the cache?
	auto* display_info = reinterpret_cast<LV_DISPINFO*>(p_nmhdr);
	LV_ITEM* item = &(display_info)->item;
	*p_result = 0;
	const auto item_index = item->iItem;

	// item before first visible item? selected item becomes first (scroll up)
	if (item_index < first_array)
	{
		first_array = item_index;
		last_array = first_array + GetCountPerPage() - 1;
		update_cache(first_array, last_array);
	}
	// item after last visible item? iItem becomes last visible (scroll down)
	else if (item_index > last_array)
	{
		last_array = item_index;
		first_array = last_array - GetCountPerPage() + 1;
		update_cache(first_array, last_array);
	}
	else if (ptr_rows_.GetSize() == 0)
		update_cache(first_array, last_array);

	// now, the requested item is in the cache
	// get data from database
	const auto pdb_doc = static_cast<ViewdbWave*>(GetParent())->GetDocument();
	if (pdb_doc == nullptr)
		return;

	const int i_first_visible = ptr_rows_.GetAt(0)->index;
	auto i_cache_index = item_index - i_first_visible;
	if (i_cache_index > (ptr_rows_.GetSize() - 1))
		i_cache_index = ptr_rows_.GetSize() - 1;

	const auto* row = ptr_rows_.GetAt(i_cache_index);

	if (item->mask & LVIF_TEXT) //valid text buffer?
	{
		CString cs;
		auto flag = TRUE;
		switch (item->iSubItem)
		{
		case CTRL_COL_CURVE: flag = FALSE;
			break;
		case CTRL_COL_INDEX: cs.Format(_T("%i"), row->index);
			break;
		case CTRL_COL_INSECT: cs.Format(_T("%i"), row->insect_id);
			break;
		case CTRL_COL_SENSI: cs = row->cs_sensillum_name;
			break;
		case CTRL_COL_STIM1: cs = row->cs_stimulus1;
			break;
		case CTRL_COL_CONC1: cs = row->cs_concentration1;
			break;
		case CTRL_COL_STIM2: cs = row->cs_stimulus2;
			break;
		case CTRL_COL_CONC2: cs = row->cs_concentration2;
			break;
		case CTRL_COL_NBSPK: cs = row->cs_n_spikes;
			break;
		case CTRL_COL_FLAG: cs = row->cs_flag;
			break;
		default: flag = FALSE;
			break;
		}
		if (flag)
			lstrcpy(item->pszText, cs);
	}

	// display images
	if (item->mask & LVIF_IMAGE
		&& item->iSubItem == CTRL_COL_CURVE)
		item->iImage = i_cache_index;
}

void DataListCtrl::set_cur_sel(const int record_position)
{
	// get current item which has the focus
	constexpr auto flag = LVNI_FOCUSED | LVNI_ALL;
	const auto current_position = GetNextItem(-1, flag);

	// exit if it is the same
	if (current_position != record_position)
	{
		// focus new
		if (current_position >= 0)
			SetItemState(current_position, 0, LVIS_SELECTED | LVIS_FOCUSED);

		SetItemState(record_position, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		EnsureVisible(record_position, FALSE);
	}
}

// Update data in cache
// adjust size of the cache if necessary
// create objects if necessary
// scroll or load new data

void DataListCtrl::update_cache(int index_first, int index_last)
{
	// adjust number of items in the array and adjust index_first and index_last
	const auto inb_visible = index_last - index_first + 1;
	if (index_first < 0)
	{
		index_first = 0;
		index_last = inb_visible - 1;
	}
	if (index_last < 0 || index_last >= GetItemCount())
	{
		index_last = GetItemCount() - 1;
		index_first = index_last - inb_visible + 1;
	}

	// resize array if the number of visible records < all records
	auto b_forced_update = FALSE;
	if (inb_visible != ptr_rows_.GetSize())
	{
		// if cache size increases, erase old information (set flag)
		if (inb_visible > ptr_rows_.GetSize())
			b_forced_update = TRUE;
		// if cache size decreases, just delete extra rows
		resize_ptr_array(inb_visible);
	}

	// get data file pointer and pointer to database
	const auto db_wave_doc = static_cast<ViewdbWave*>(GetParent())->GetDocument();
	if (db_wave_doc == nullptr)
		return;

	const int index_current_file = db_wave_doc->db_get_current_record_position();

	// which update is necessary?
	// conditions for out of range (renew all items)
	auto n_to_rebuild = ptr_rows_.GetSize(); // number of items to refresh
	auto new1 = 0;
	auto i_first_array = 0;
	if (ptr_rows_.GetSize() > 0)
		i_first_array = ptr_rows_.GetAt(0)->index;
	const auto difference = index_first - i_first_array;

	// change indexes according to case
	// scroll up (go forwards i.e. towards indexes of higher value)
	if (!b_forced_update)
	{
		auto source1 = 0;
		auto dest1 = 0;
		auto delta = 0;
		auto n_to_transfer = 0;
		if (difference > 0 && difference < ptr_rows_.GetSize())
		{
			delta = 1; // copy forward
			n_to_transfer = ptr_rows_.GetSize() - difference;
			n_to_rebuild -= n_to_transfer;
			source1 = difference;
			dest1 = 0;
			new1 = n_to_transfer;
		}
		// scroll down (go backwards i.e. towards indexes of lower value)
		else if (difference < 0 && -difference < ptr_rows_.GetSize())
		{
			delta = -1;
			n_to_transfer = ptr_rows_.GetSize() + difference;
			n_to_rebuild -= n_to_transfer;
			source1 = n_to_transfer - 1;
			dest1 = ptr_rows_.GetSize() - 1;
			new1 = 0;
		}

		// n to transfer 
		auto source = source1;
		auto dest = dest1;
		while (n_to_transfer > 0)
		{
			// exchange objects
			auto* p_source = ptr_rows_.GetAt(source);
			const auto p_dest = ptr_rows_.GetAt(dest);
			ptr_rows_.SetAt(dest, p_source);
			ptr_rows_.SetAt(source, p_dest);
			infos.image_list.Copy(dest, source, ILCF_SWAP);

			// update indexes
			source += delta;
			dest += delta;
			n_to_transfer--;
		}
	}

	// set file and update file names
	int index = new1;

	// left, top, right, bottom
	build_empty_bitmap();
	infos.parent = this;

	while (n_to_rebuild > 0)
	{
		const auto row = ptr_rows_.GetAt(index);
		row->index = index + index_first;
		row->build_row(db_wave_doc);
		row->display_row(&infos, index);
		index++;
		n_to_rebuild--;
	}

	// restore document conditions
	if (index_current_file >= 0) {
		if (db_wave_doc->db_set_current_record_position(index_current_file))
		{
			db_wave_doc->open_current_data_file();
			db_wave_doc->open_current_spike_file();
		}
	}
}

void DataListCtrl::build_empty_bitmap(const boolean b_forced_update)
{
	if (infos.p_empty_bitmap != nullptr && !b_forced_update)
		return;

	SAFE_DELETE(infos.p_empty_bitmap)
		infos.p_empty_bitmap = new CBitmap;

	CWindowDC dc(this);
	CDC mem_dc;
	VERIFY(mem_dc.CreateCompatibleDC(&dc));
	infos.p_empty_bitmap->CreateBitmap(infos.image_width, infos.image_height,
		dc.GetDeviceCaps(PLANES), 
		dc.GetDeviceCaps(BITSPIXEL), nullptr);
	mem_dc.SelectObject(infos.p_empty_bitmap);
	mem_dc.SetMapMode(dc.GetMapMode());

	CBrush brush(RGB(204, 204, 204)); //light gray
	mem_dc.SelectObject(&brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	mem_dc.SelectObject(&pen);
	const auto rect_data = CRect(1, 0, infos.image_width, infos.image_height);
	mem_dc.Rectangle(&rect_data);
}

void DataListCtrl::refresh_display()
{
	if (ptr_rows_.GetSize() == NULL)
		return;
	const int first_row = ptr_rows_.GetAt(0)->index;
	const int last_row = ptr_rows_.GetAt(ptr_rows_.GetUpperBound())->index;
	build_empty_bitmap();

	const auto n_rows = ptr_rows_.GetSize();
	infos.parent = this;
	for (auto index = 0; index < n_rows; index++)
	{
		auto* row = ptr_rows_.GetAt(index);
		if (row == nullptr)
			continue;
		row->display_row(&infos, index);
	}
	RedrawItems(first_row, last_row);
	Invalidate();
	UpdateWindow();
}

void DataListCtrl::OnVScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar* p_scroll_bar)
{
	switch (n_sb_code)
	{
	case SB_LINEUP:
		static_cast<ViewDbTable*>(GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case SB_LINEDOWN:
		static_cast<ViewDbTable*>(GetParent())->OnMove(ID_RECORD_NEXT);
		break;
	default:
		CListCtrl::OnVScroll(n_sb_code, n_pos, p_scroll_bar);
		break;
	}
}

void DataListCtrl::OnKeyUp(UINT n_char, UINT n_rep_cnt, UINT n_flags)
{
	switch (n_char)
	{
	case VK_PRIOR: 
		SendMessage(WM_VSCROLL, SB_PAGEUP, NULL);
		break;
	case VK_NEXT: 
		SendMessage(WM_VSCROLL, SB_PAGEDOWN, NULL);
		break;
	case VK_UP:
		static_cast<ViewDbTable*>(GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case VK_DOWN:
		static_cast<ViewDbTable*>(GetParent())->OnMove(ID_RECORD_NEXT);
		break;

	default:
		CListCtrl::OnKeyUp(n_char, n_rep_cnt, n_flags);
		break;
	}
}

ChartData* DataListCtrl::get_chart_data_of_current_record()
{
	const UINT n_selected_items = GetSelectedCount();
	int n_item = -1;
	ChartData* ptr = nullptr;

	// get ptr of first item selected
	if (n_selected_items > 0)
	{
		n_item = GetNextItem(n_item, LVNI_SELECTED);
		ASSERT(n_item != -1);
		n_item -= GetTopIndex();
		if (n_item >= 0 && n_item < ptr_rows_.GetSize())
			ptr = ptr_rows_.GetAt(n_item)->p_data_chart_wnd;
	}
	return ptr;
}

void DataListCtrl::resize_signal_column(const int n_pixels)
{
	m_column_width_[CTRL_COL_CURVE] = n_pixels;
	infos.image_list.DeleteImageList();
	infos.image_width = m_column_width_[CTRL_COL_CURVE];
	infos.image_list.Create(infos.image_width, infos.image_height, ILC_COLOR4, 10, 10);
	SetImageList(&infos.image_list, LVSIL_SMALL);
	infos.image_list.SetImageCount(ptr_rows_.GetSize());

	for (int i = 0; i < ptr_rows_.GetSize(); i++)
	{
		auto* ptr = ptr_rows_.GetAt(i);
		SAFE_DELETE(ptr->p_data_chart_wnd)
		SAFE_DELETE(ptr->p_spike_chart_wnd)
	}
	refresh_display();
}

void DataListCtrl::fit_columns_to_size(const int n_pixels)
{
	// compute width of fixed columns
	auto fixed_width = 0;
	for (const auto i : m_column_width_)
	{
		fixed_width += i;
	}
	fixed_width -= m_column_width_[CTRL_COL_CURVE];
	const auto signal_column_width = n_pixels - fixed_width;
	if (signal_column_width != m_column_width_[CTRL_COL_CURVE] && signal_column_width > 2)
	{
		SetColumnWidth(CTRL_COL_CURVE, signal_column_width);
		resize_signal_column(signal_column_width);
	}
}
