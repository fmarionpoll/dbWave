#include "StdAfx.h"

#include "DataListCtrl.h"
#include <../ChartData.h>
#include <../dbTableView.h>

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
	SAFE_DELETE(m_p_empty_bitmap_)
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
	if (ptr_rows.GetSize() == NULL)
		return;
	const auto n_rows = ptr_rows.GetSize();
	for (auto i = 0; i < n_rows; i++)
	{
		const auto* ptr = ptr_rows.GetAt(i);
		SAFE_DELETE(ptr)
	}
	ptr_rows.RemoveAll();
}

void DataListCtrl::resize_ptr_array(const int n_items)
{
	if (n_items == ptr_rows.GetSize())
		return;

	// Resize m_image_list CImageList
	m_image_list_.SetImageCount(n_items);

	// reduce size
	if (ptr_rows.GetSize() > n_items)
	{
		for (auto i = ptr_rows.GetSize() - 1; i >= n_items; i--)
			delete ptr_rows.GetAt(i);
		ptr_rows.SetSize(n_items);
	}
	// grow size
	else
	{
		const auto size_before_change = ptr_rows.GetSize();
		ptr_rows.SetSize(n_items);
		auto index = 0;
		if (size_before_change > 0)
			index = ptr_rows.GetAt(size_before_change - 1)->index;
		for (auto i = size_before_change; i < n_items; i++)
		{
			auto* ptr = new CDataListCtrl_Row;
			ASSERT(ptr != NULL);
			ptr_rows.SetAt(i, ptr);
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

	m_image_width_ = m_column_width_[CTRL_COL_CURVE];
	m_image_list_.Create(m_image_width_, m_image_height_, ILC_COLOR4, 10, 10);
	SetImageList(&m_image_list_, LVSIL_SMALL);
}

void DataListCtrl::OnGetDisplayInfo(NMHDR* p_nmhdr, LRESULT* p_result)
{
	auto first_array = 0;
	auto last_array = 0;
	if (ptr_rows.GetSize() > 0)
	{
		first_array = ptr_rows.GetAt(0)->index;
		last_array = ptr_rows.GetAt(ptr_rows.GetUpperBound())->index;
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
	else if (ptr_rows.GetSize() == 0)
		update_cache(first_array, last_array);

	// now, the requested item is in the cache
	// get data from database
	const auto pdb_doc = static_cast<ViewdbWave*>(GetParent())->GetDocument();
	if (pdb_doc == nullptr)
		return;

	const int i_first_visible = ptr_rows.GetAt(0)->index;
	auto i_cache_index = item_index - i_first_visible;
	if (i_cache_index > (ptr_rows.GetSize() - 1))
		i_cache_index = ptr_rows.GetSize() - 1;

	const auto* row = ptr_rows.GetAt(i_cache_index);

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
	if (inb_visible != ptr_rows.GetSize())
	{
		// if cache size increases, erase old information (set flag)
		if (inb_visible > ptr_rows.GetSize())
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
	auto n_to_rebuild = ptr_rows.GetSize(); // number of items to refresh
	auto new1 = 0;
	auto i_first_array = 0;
	if (ptr_rows.GetSize() > 0)
		i_first_array = ptr_rows.GetAt(0)->index;
	const auto difference = index_first - i_first_array;

	// change indexes according to case
	// scroll up (go forwards i.e. towards indexes of higher value)
	if (!b_forced_update)
	{
		auto source1 = 0;
		auto dest1 = 0;
		auto delta = 0;
		auto n_to_transfer = 0;
		if (difference > 0 && difference < ptr_rows.GetSize())
		{
			delta = 1; // copy forward
			n_to_transfer = ptr_rows.GetSize() - difference;
			n_to_rebuild -= n_to_transfer;
			source1 = difference;
			dest1 = 0;
			new1 = n_to_transfer;
		}
		// scroll down (go backwards i.e. towards indexes of lower value)
		else if (difference < 0 && -difference < ptr_rows.GetSize())
		{
			delta = -1;
			n_to_transfer = ptr_rows.GetSize() + difference;
			n_to_rebuild -= n_to_transfer;
			source1 = n_to_transfer - 1;
			dest1 = ptr_rows.GetSize() - 1;
			new1 = 0;
		}

		// n to Transfer 
		auto source = source1;
		auto dest = dest1;
		while (n_to_transfer > 0)
		{
			// exchange objects
			auto* p_source = ptr_rows.GetAt(source);
			const auto p_dest = ptr_rows.GetAt(dest);
			ptr_rows.SetAt(dest, p_source);
			ptr_rows.SetAt(source, p_dest);
			m_image_list_.Copy(dest, source, ILCF_SWAP);

			// update indexes
			source += delta;
			dest += delta;
			n_to_transfer--;
		}
	}

	// set file and update file names
	int index = new1;

	// left, top, right, bottom
	set_empty_bitmap();
	DB_ITEMDESC desc;

	while (n_to_rebuild > 0)
	{
		// get data; create object if null
		const auto row = ptr_rows.GetAt(index);

		// create line view and spike superposition
		row->index = index + index_first;
		if (db_wave_doc->db_set_current_record_position(row->index))
		{
			db_wave_doc->open_current_data_file();
			db_wave_doc->open_current_spike_file();
		}
		row->cs_datafile_name = db_wave_doc->db_get_current_dat_file_name(TRUE);
		row->cs_spike_file_name = db_wave_doc->db_get_current_spk_file_name(TRUE);
		const auto database = db_wave_doc->db_table;

		database->get_record_item_value(CH_IDINSECT, &desc);
		row->insect_id = desc.lVal;

		// column: stimulus, concentration, type = load indirect data from database
		database->get_record_item_value(CH_STIM_ID, &desc);
		row->cs_stimulus1 = desc.csVal;
		database->get_record_item_value(CH_CONC_ID, &desc);
		row->cs_concentration1 = desc.csVal;
		database->get_record_item_value(CH_STIM2_ID, &desc);
		row->cs_stimulus2 = desc.csVal;
		database->get_record_item_value(CH_CONC2_ID, &desc);
		row->cs_concentration2 = desc.csVal;

		database->get_record_item_value(CH_SENSILLUM_ID, &desc);
		row->cs_sensillum_name = desc.csVal;

		database->get_record_item_value(CH_FLAG, &desc);
		row->cs_flag.Format(_T("%i"), desc.lVal);

		// column: number of spike = verify that spike file is defined, if yes, load nb spikes
		if (db_wave_doc->db_get_current_spk_file_name(TRUE).IsEmpty())
			row->cs_n_spikes.Empty();
		else
		{
			database->get_record_item_value(CH_NSPIKES, &desc);
			const int n_spikes = desc.lVal;
			database->get_record_item_value(CH_NSPIKECLASSES, &desc);
			row->cs_n_spikes.Format(_T("%i (%i classes)"), n_spikes, desc.lVal);
		}

		// build bitmap corresponding to data/spikes/nothing
		switch (m_display_mode_)
		{
		// data mode
		case 1:
			display_data_wnd(row, index);
			break;
		// spike bars
		case 2:
			display_spike_wnd(row, index);
			break;
		default:
			display_empty_wnd(row, index);
			break;
		}
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

void DataListCtrl::set_empty_bitmap(const boolean b_forced_update)
{
	if (m_p_empty_bitmap_ != nullptr && !b_forced_update)
		return;

	SAFE_DELETE(m_p_empty_bitmap_)
	m_p_empty_bitmap_ = new CBitmap;

	CWindowDC dc(this);
	CDC mem_dc;
	VERIFY(mem_dc.CreateCompatibleDC(&dc));
	m_p_empty_bitmap_->CreateBitmap(m_image_width_, m_image_height_, 
		dc.GetDeviceCaps(PLANES), 
		dc.GetDeviceCaps(BITSPIXEL), nullptr);
	mem_dc.SelectObject(m_p_empty_bitmap_);
	mem_dc.SetMapMode(dc.GetMapMode());

	CBrush brush(RGB(204, 204, 204)); //light gray
	mem_dc.SelectObject(&brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	mem_dc.SelectObject(&pen);
	const auto rect_data = CRect(1, 0, m_image_width_, m_image_height_);
	mem_dc.Rectangle(&rect_data);
}

void DataListCtrl::refresh_display()
{
	if (ptr_rows.GetSize() == NULL)
		return;
	const int first_row = ptr_rows.GetAt(0)->index;
	const int last_row = ptr_rows.GetAt(ptr_rows.GetUpperBound())->index;
	set_empty_bitmap();

	const auto n_rows = ptr_rows.GetSize();
	for (auto index = 0; index < n_rows; index++)
	{
		auto* ptr = ptr_rows.GetAt(index);
		if (ptr == nullptr)
			continue;
		switch (m_display_mode_)
		{
		case 1:
			display_data_wnd(ptr, index);
			break;
		case 2:
			display_spike_wnd(ptr, index);
			break;
		default:
			display_empty_wnd(ptr, index);
			break;
		}
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
		static_cast<dbTableView*>(GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case SB_LINEDOWN:
		static_cast<dbTableView*>(GetParent())->OnMove(ID_RECORD_NEXT);
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
		static_cast<dbTableView*>(GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case VK_DOWN:
		static_cast<dbTableView*>(GetParent())->OnMove(ID_RECORD_NEXT);
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
		if (n_item >= 0 && n_item < ptr_rows.GetSize())
			ptr = ptr_rows.GetAt(n_item)->p_data_chart_wnd;
	}
	return ptr;
}

void DataListCtrl::display_data_wnd(CDataListCtrl_Row* ptr, int i_image)
{
	// create objects if necessary : CLineView and AcqDataDoc
	if (ptr->p_data_chart_wnd == nullptr)
	{
		ptr->p_data_chart_wnd = new ChartData;
		ASSERT(ptr->p_data_chart_wnd != NULL);
		ptr->p_data_chart_wnd->Create(_T("DATAWND"), WS_CHILD, CRect(0, 0, m_image_width_, m_image_height_), this, i_image * 100);
		ptr->p_data_chart_wnd->set_b_use_dib(FALSE);
	}
	const auto p_wnd = ptr->p_data_chart_wnd;
	p_wnd->set_string(ptr->cs_comment);

	// open data document
	if (ptr->p_data_doc == nullptr)
	{
		ptr->p_data_doc = new AcqDataDoc;
		ASSERT(ptr->p_data_doc != NULL);
	}
	if (ptr->cs_datafile_name.IsEmpty() || !ptr->p_data_doc->open_document(ptr->cs_datafile_name))
	{
		p_wnd->remove_all_channel_list_items();
		auto comment = _T("File name: ") + ptr->cs_datafile_name;
		comment += _T(" -- data not available");
		p_wnd->set_string(comment);
	}
	else
	{
		if (ptr->cs_n_spikes.IsEmpty())
			p_wnd->get_scope_parameters()->cr_scope_fill = ChartData::get_color(2);
		else
			p_wnd->get_scope_parameters()->cr_scope_fill = ChartData::get_color(15);

		ptr->p_data_doc->read_data_infos();
		ptr->cs_comment = ptr->p_data_doc->get_wave_format()->get_comments(_T(" "));
		p_wnd->attach_data_file(ptr->p_data_doc);
		p_wnd->load_all_channels(m_data_transform_);
		p_wnd->load_data_within_window(b_set_time_span_, m_t_first_, m_t_last_);
		p_wnd->adjust_gain(b_set_mv_span_, m_m_v_span_);

		ptr->p_data_doc->acq_close_file();
	}
	plot_data(ptr, p_wnd, i_image);
}


void DataListCtrl::plot_data(const CDataListCtrl_Row* ptr, ChartData* p_wnd, int i_image)
{
	p_wnd->set_bottom_comment(b_display_file_name_, ptr->cs_datafile_name);
	CRect client_rect;
	p_wnd->GetClientRect(&client_rect);

	CBitmap bitmap_plot;
	const auto p_dc = p_wnd->GetDC();
	CDC mem_dc;
	VERIFY(mem_dc.CreateCompatibleDC(p_dc));
	bitmap_plot.CreateBitmap(client_rect.right, client_rect.bottom, p_dc->GetDeviceCaps(PLANES),
		p_dc->GetDeviceCaps(BITSPIXEL), nullptr);
	mem_dc.SelectObject(&bitmap_plot);
	mem_dc.SetMapMode(p_dc->GetMapMode());

	p_wnd->plot_data_to_dc(&mem_dc);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
	mem_dc.MoveTo(1, 0);
	mem_dc.LineTo(1, client_rect.bottom);
	m_image_list_.Replace(i_image, &bitmap_plot, nullptr);
}

void DataListCtrl::display_spike_wnd(CDataListCtrl_Row* ptr, const int i_image)
{
	// create spike window and spike document if necessary
	if (ptr->p_spike_chart_wnd == nullptr)
	{
		ptr->p_spike_chart_wnd = new ChartSpikeBar;
		ASSERT(ptr->p_spike_chart_wnd != NULL);
		ptr->p_spike_chart_wnd->Create(_T("SPKWND"), WS_CHILD, CRect(0, 0, m_image_width_, m_image_height_), this, ptr->index * 1000);
		ptr->p_spike_chart_wnd->set_b_use_dib(FALSE);
	}
	const auto chart_spike_bar = ptr->p_spike_chart_wnd;

	// open spike document
	if (ptr->p_spike_doc == nullptr)
	{
		ptr->p_spike_doc = new CSpikeDoc;
		ASSERT(ptr->p_spike_doc != NULL);
	}

	if (ptr->cs_spike_file_name.IsEmpty() || !ptr->p_spike_doc->OnOpenDocument(ptr->cs_spike_file_name))
	{
		m_image_list_.Replace(i_image, m_p_empty_bitmap_, nullptr);
	}
	else
	{
		const auto p_parent = static_cast<ViewdbWave*>(GetParent());
		int i_tab = p_parent->m_tabCtrl.GetCurSel();
		if (i_tab < 0)
			i_tab = 0;
		const auto p_spk_list = ptr->p_spike_doc->set_spike_list_current_index(i_tab);

		chart_spike_bar->set_source_data(p_spk_list, p_parent->GetDocument());
		chart_spike_bar->set_plot_mode(m_spike_plot_mode_, m_selected_class_);

		long l_first = 0;
		auto l_last = ptr->p_spike_doc->get_acq_size();
		if (b_set_time_span_)
		{
			const auto sampling_rate = ptr->p_spike_doc->get_acq_rate();
			l_first = static_cast<long>(m_t_first_ * sampling_rate);
			l_last = static_cast<long>(m_t_last_ * sampling_rate);
		}

		chart_spike_bar->set_time_intervals(l_first, l_last);
		if (b_set_mv_span_)
		{
			const auto volts_per_bin = p_spk_list->get_acq_volts_per_bin();
			const auto y_we = static_cast<int>(m_m_v_span_ / 1000.f / volts_per_bin);
			const auto y_wo = p_spk_list->get_acq_bin_zero();
			chart_spike_bar->set_yw_ext_org(y_we, y_wo);
		}
		chart_spike_bar->set_bottom_comment(b_display_file_name_, ptr->cs_spike_file_name);
		CRect client_rect;
		chart_spike_bar->GetClientRect(&client_rect);

		CBitmap bitmap_plot;
		const auto p_dc = chart_spike_bar->GetDC();
		CDC mem_dc;
		VERIFY(mem_dc.CreateCompatibleDC(p_dc));
		bitmap_plot.CreateBitmap(client_rect.right, 
			client_rect.bottom, 
			p_dc->GetDeviceCaps(PLANES),
			p_dc->GetDeviceCaps(BITSPIXEL), 
			nullptr);
		mem_dc.SelectObject(&bitmap_plot);
		mem_dc.SetMapMode(p_dc->GetMapMode());

		chart_spike_bar->set_display_all_files(false);
		chart_spike_bar->plot_data_to_dc(&mem_dc);

		CPen pen;
		pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		mem_dc.MoveTo(1, 0);
		mem_dc.LineTo(1, client_rect.bottom);
		m_image_list_.Replace(i_image, &bitmap_plot, nullptr);
	}
}

void DataListCtrl::display_empty_wnd(CDataListCtrl_Row* ptr, const int i_image)
{
	m_image_list_.Replace(i_image, m_p_empty_bitmap_, nullptr);
}

void DataListCtrl::resize_signal_column(const int n_pixels)
{
	m_column_width_[CTRL_COL_CURVE] = n_pixels;
	m_image_list_.DeleteImageList();
	m_image_width_ = m_column_width_[CTRL_COL_CURVE];
	m_image_list_.Create(m_image_width_, m_image_height_, ILC_COLOR4, 10, 10);
	SetImageList(&m_image_list_, LVSIL_SMALL);
	m_image_list_.SetImageCount(ptr_rows.GetSize());

	for (int i = 0; i < ptr_rows.GetSize(); i++)
	{
		auto* ptr = ptr_rows.GetAt(i);
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
