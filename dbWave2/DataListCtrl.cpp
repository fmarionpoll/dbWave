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
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, on_get_display_info)
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
	if (rows_.GetSize() == NULL)
		return;
	const auto n_rows = rows_.GetSize();
	for (auto i = 0; i < n_rows; i++)
	{
		const auto* ptr = rows_.GetAt(i);
		SAFE_DELETE(ptr)
	}
	rows_.RemoveAll();
}

boolean DataListCtrl::rows_array_set_size(const int rows_count)
{
	auto b_forced_update = false;
	if (rows_count == rows_.GetSize())
		return b_forced_update;

	// if cache size decreases, just delete extra rows
	infos.image_list.SetImageCount(rows_count);
	if (rows_.GetSize() > rows_count)
	{
		for (auto i = rows_.GetSize() - 1; i >= rows_count; i--)
			delete rows_.GetAt(i);
		rows_.SetSize(rows_count);
	}
	// if cache size increases, erase old information (set flag)
	else
	{
		b_forced_update = true;
		const auto size_before_change = rows_.GetSize();
		rows_.SetSize(rows_count);
		auto index = 0;
		if (size_before_change > 0)
			index = rows_.GetAt(size_before_change - 1)->index +1;
		for (auto i = size_before_change; i < rows_count; i++)
		{
			auto* row = new DataListCtrl_Row;
			ASSERT(row != NULL);
			rows_.SetAt(i, row);
			row->index = index;
			index++;
		}
	}

	return b_forced_update;
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

void DataListCtrl::on_get_display_info(NMHDR* p_nmhdr, LRESULT* p_result)
{
	auto first_array = 0;
	auto last_array = 0;
	if (rows_.GetSize() > 0)
	{
		const int upper_index = rows_.GetUpperBound();
		first_array = rows_.GetAt(0)->index;
		last_array = rows_.GetAt(upper_index)->index;
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
	else if (rows_.GetSize() == 0)
		update_cache(first_array, last_array);

	// now, the requested item is in the cache
	// get data from database
	const auto pdb_doc = static_cast<ViewdbWave*>(GetParent())->GetDocument();
	if (pdb_doc == nullptr)
		return;

	const int i_first_visible = rows_.GetAt(0)->index;
	auto i_cache_index = item_index - i_first_visible;
	if (i_cache_index > (rows_.GetSize() - 1))
		i_cache_index = rows_.GetSize() - 1;

	const auto* row = rows_.GetAt(i_cache_index);

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
	if (item->mask & LVIF_IMAGE && item->iSubItem == CTRL_COL_CURVE)
		item->iImage = i_cache_index;
}

void DataListCtrl::set_current_selection(const int record_position)
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

int DataListCtrl::cache_adjust_boundaries(int& index_first, int& index_last) const
{
	const int inb_visible = index_last - index_first + 1;
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
	return inb_visible;
}

void DataListCtrl::cache_shift_rows_positions(const int source1, const int dest1, int rows_count_to_exchange, const int delta)
{
	auto source = source1;
	auto dest = dest1;
	while (rows_count_to_exchange > 0)
	{
		// exchange objects
		auto* p_source = rows_.GetAt(source);
		const auto p_dest = rows_.GetAt(dest);
		rows_.SetAt(dest, p_source);
		rows_.SetAt(source, p_dest);
		infos.image_list.Copy(dest, source, ILCF_SWAP);

		// update indexes
		source += delta;
		dest += delta;
		rows_count_to_exchange--;
	}
}

void DataListCtrl::cache_build_rows(const int new1, const int index_first, int n_rows_to_build, CdbWaveDoc* db_wave_doc)
{
	build_empty_bitmap();

	infos.parent = this;
	int image_index = new1;
	while (n_rows_to_build > 0)
	{
		const auto row = rows_.GetAt(image_index);
		row->index = image_index + index_first;
		row->attach_database_record(db_wave_doc);
		row->set_display_parameters(&infos, image_index);

		image_index++;
		n_rows_to_build--;
	}
}

void DataListCtrl::update_cache(int index_first, int index_last)
{
	const auto rows_count = cache_adjust_boundaries(index_first, index_last);
	const auto b_forced_update = rows_array_set_size(rows_count);
	const auto db_wave_doc = static_cast<ViewdbWave*>(GetParent())->GetDocument();
	if (db_wave_doc == nullptr)
		return;

	// which update is necessary?
	const int index_current_file = db_wave_doc->db_get_current_record_position();

	// set conditions for out of range (renew all items)
	auto n_rows_to_build = rows_.GetSize(); 
	auto new1 = 0;
	auto rows_index_first = 0;
	if (rows_.GetSize() > 0)
		rows_index_first = rows_.GetAt(0)->index;
	const auto offset = index_first - rows_index_first;

	// change indexes according to case
	if (!b_forced_update)
	{
		auto source1 = 0;
		auto dest1 = 0;
		auto delta = 0;
		auto n_rows_to_shift = 0;
		// scroll up (go forwards i.e. towards indexes of higher value)
		if (offset > 0 && offset < rows_.GetSize())
		{
			delta = 1; // copy forward
			n_rows_to_shift = rows_.GetSize() - offset;
			n_rows_to_build -= n_rows_to_shift;
			source1 = offset;
			dest1 = 0;
			new1 = n_rows_to_shift;
		}
		// scroll down (go backwards i.e. towards indexes of lower value)
		else if (offset < 0 && -offset < rows_.GetSize())
		{
			delta = -1;
			n_rows_to_shift = rows_.GetSize() + offset;
			n_rows_to_build -= n_rows_to_shift;
			source1 = n_rows_to_shift - 1;
			dest1 = rows_.GetSize() - 1;
			new1 = 0;
		}

		cache_shift_rows_positions( source1, dest1, n_rows_to_shift,   delta);
	}

	cache_build_rows(new1, index_first, n_rows_to_build, db_wave_doc);

	// restore document conditions
	if (index_current_file >= 0) 
	{
		BOOL b_success = db_wave_doc->db_set_current_record_position(index_current_file);
		{
			//db_wave_doc->open_current_data_file();
			//db_wave_doc->open_current_spike_file();
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

	CBrush brush(col_silver); 
	mem_dc.SelectObject(&brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, col_black);
	mem_dc.SelectObject(&pen);
	const auto rect_data = CRect(1, 0, infos.image_width, infos.image_height);
	mem_dc.Rectangle(&rect_data);
}

void DataListCtrl::refresh_display()
{
	if (rows_.GetSize() == NULL)
		return;

	const int first_row = rows_.GetAt(0)->index;
	const int last_row = rows_.GetAt(rows_.GetUpperBound())->index;
	build_empty_bitmap(true);

	const auto n_rows = rows_.GetSize();
	infos.parent = this;
	for (auto image_index = 0; image_index < n_rows; image_index++)
	{
		auto* row = rows_.GetAt(image_index);
		if (row == nullptr)
			continue;
		row->set_display_parameters(&infos, image_index);
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
		if (n_item >= 0 && n_item < rows_.GetSize())
			ptr = rows_.GetAt(n_item)->p_chart_data_wnd;
	}
	return ptr;
}

void DataListCtrl::resize_signal_column(const int n_pixels)
{
	m_column_width_[CTRL_COL_CURVE] = n_pixels;
	infos.image_list.DeleteImageList();
	infos.image_width = n_pixels;
	infos.image_list.Create(infos.image_width, infos.image_height, ILC_COLOR4, 10, 10);
	SetImageList(&infos.image_list, LVSIL_SMALL);
	infos.image_list.SetImageCount(rows_.GetSize());

	for (int i = 0; i < rows_.GetSize(); i++)
	{
		auto* ptr = rows_.GetAt(i);
		SAFE_DELETE(ptr->p_chart_data_wnd)
		SAFE_DELETE(ptr->p_chart_spike_wnd)
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
