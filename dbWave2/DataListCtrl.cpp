#include "StdAfx.h"

#include "DataListCtrl.h"
#include <../ChartData.h>
#include <../dbTableView.h>

#include "DataListCtrl_Row.h"
#include "ViewdbWave.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int DataListCtrl::m_column_width[] = {
	1,
	10, 300, 15, 30,
	30, 50, 40, 40,
	40, 40
};
CString DataListCtrl::m_column_headers[] = {
	__T(""),
	__T("#"), __T("data"), __T("insect ID"), __T("sensillum"),
	__T("stim1"), __T("conc1"), __T("stim2"), __T("conc2"),
	__T("spikes"), __T("flag")
};

int DataListCtrl::m_column_format[] = {
	LVCFMT_LEFT,
	LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER,
	LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER,
	LVCFMT_CENTER, LVCFMT_CENTER
};

int DataListCtrl::m_column_index[] = {
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
	SAFE_DELETE(m_p_empty_bitmap)
}

void DataListCtrl::OnDestroy()
{
	save_columns_width();
}

void DataListCtrl::save_columns_width() const
{
	if (m_width_columns != nullptr)
	{
		const auto n_columns_stored = m_width_columns->GetSize();
		if (n_columns_stored != NCOLS)
			m_width_columns->SetSize(NCOLS);
		for (auto i = 0; i < NCOLS; i++)
			m_width_columns->SetAt(i, GetColumnWidth(i));
	}
}

void DataListCtrl::delete_ptr_array()
{
	if (ptr_rows.GetSize() == NULL)
		return;
	const auto n_rows = ptr_rows.GetSize();
	for (auto i = 0; i < n_rows; i++)
	{
		auto* ptr = ptr_rows.GetAt(i);
		SAFE_DELETE(ptr)
	}
	ptr_rows.RemoveAll();
}

void DataListCtrl::resize_ptr_array(int n_items)
{
	if (n_items == ptr_rows.GetSize())
		return;

	// Resize m_image_list CImageList
	m_image_list.SetImageCount(n_items);

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

void DataListCtrl::InitColumns(CUIntArray* width_columns)
{
	if (width_columns != nullptr)
	{
		m_width_columns = width_columns;
		const auto n_columns = width_columns->GetSize();
		if (n_columns < NCOLS)
			width_columns->SetSize(NCOLS);
		for (auto i = 0; i < n_columns; i++)
			m_column_width[i] = static_cast<int>(width_columns->GetAt(i));
	}

	for (auto i = 0; i < NCOLS; i++)
	{
		InsertColumn(i, m_column_headers[i], m_column_format[i], m_column_width[i], -1);
	}

	m_image_width = m_column_width[CTRL_COL_CURVE];
	m_image_list.Create(m_image_width, m_image_height, ILC_COLOR4, 10, 10);
	SetImageList(&m_image_list, LVSIL_SMALL);
}

void DataListCtrl::OnGetDisplayInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	// check if first if the requested line is stored into the buffer
	auto first_array = 0;
	auto last_array = 0;
	if (ptr_rows.GetSize() > 0)
	{
		first_array = ptr_rows.GetAt(0)->index;
		last_array = ptr_rows.GetAt(ptr_rows.GetUpperBound())->index;
	}

	// is item within the cache?
	auto* display_info = reinterpret_cast<LV_DISPINFO*>(pNMHDR);
	LV_ITEM* item = &(display_info)->item;
	*pResult = 0;
	const auto item_index = item->iItem;

	// item before first visible item? selected item becomes first (scroll up)
	if (item_index < first_array)
	{
		first_array = item_index;
		last_array = first_array + GetCountPerPage() - 1;
		UpdateCache(first_array, last_array);
	}
	// item after last visible item? iItem becomes last visible (scroll down)
	else if (item_index > last_array)
	{
		last_array = item_index;
		first_array = last_array - GetCountPerPage() + 1;
		UpdateCache(first_array, last_array);
	}
	else if (ptr_rows.GetSize() == 0)
		UpdateCache(first_array, last_array);

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
		case CTRL_COL_INSECT: cs.Format(_T("%i"), row->insectID);
			break;
		case CTRL_COL_SENSI: cs = row->csSensillumname;
			break;
		case CTRL_COL_STIM1: cs = row->csStim1;
			break;
		case CTRL_COL_CONC1: cs = row->csConc1;
			break;
		case CTRL_COL_STIM2: cs = row->csStim2;
			break;
		case CTRL_COL_CONC2: cs = row->csConc2;
			break;
		case CTRL_COL_NBSPK: cs = row->csNspk;
			break;
		case CTRL_COL_FLAG: cs = row->csFlag;
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

void DataListCtrl::SetCurSel(int record_position)
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

void DataListCtrl::UpdateCache(int index_first, int index_last)
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
	const int index_current_file = db_wave_doc->DB_GetCurrentRecordPosition();

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
			m_image_list.Copy(dest, source, ILCF_SWAP);

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
		db_wave_doc->DB_SetCurrentRecordPosition(row->index); 
		row->csDatafileName = db_wave_doc->DB_GetCurrentDatFileName(TRUE);
		row->csSpikefileName = db_wave_doc->DB_GetCurrentSpkFileName(TRUE);
		const auto database = db_wave_doc->m_pDB;
		database->GetRecordItemValue(CH_IDINSECT, &desc);
		row->insectID = desc.lVal;

		// column: stim, conc, type = load indirect data from database
		database->GetRecordItemValue(CH_STIM_ID, &desc);
		row->csStim1 = desc.csVal;
		database->GetRecordItemValue(CH_CONC_ID, &desc);
		row->csConc1 = desc.csVal;
		database->GetRecordItemValue(CH_STIM2_ID, &desc);
		row->csStim2 = desc.csVal;
		database->GetRecordItemValue(CH_CONC2_ID, &desc);
		row->csConc2 = desc.csVal;

		database->GetRecordItemValue(CH_SENSILLUM_ID, &desc);
		row->csSensillumname = desc.csVal;

		database->GetRecordItemValue(CH_FLAG, &desc);
		row->csFlag.Format(_T("%i"), desc.lVal);

		// column: number of spike = verify that spike file is defined, if yes, load nb spikes
		if (db_wave_doc->DB_GetCurrentSpkFileName(TRUE).IsEmpty())
			row->csNspk.Empty();
		else
		{
			database->GetRecordItemValue(CH_NSPIKES, &desc);
			const int n_spikes = desc.lVal;
			database->GetRecordItemValue(CH_NSPIKECLASSES, &desc);
			row->csNspk.Format(_T("%i (%i classes)"), n_spikes, desc.lVal);
		}

		// build bitmap corresponding to data/spikes/nothing
		switch (m_display_mode)
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
	if (index_current_file >= 0)
		db_wave_doc->DB_SetCurrentRecordPosition(index_current_file);
}

void DataListCtrl::set_empty_bitmap(const boolean b_forced_update)
{
	if (m_p_empty_bitmap != nullptr && !b_forced_update)
		return;

	SAFE_DELETE(m_p_empty_bitmap)
	m_p_empty_bitmap = new CBitmap;

	CWindowDC dc(this);
	CDC mem_dc;
	VERIFY(mem_dc.CreateCompatibleDC(&dc));
	m_p_empty_bitmap->CreateBitmap(m_image_width, m_image_height, dc.GetDeviceCaps(PLANES), dc.GetDeviceCaps(BITSPIXEL), nullptr);
	mem_dc.SelectObject(m_p_empty_bitmap);
	mem_dc.SetMapMode(dc.GetMapMode());

	CBrush brush(RGB(204, 204, 204)); //light gray
	mem_dc.SelectObject(&brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	mem_dc.SelectObject(&pen);
	const auto rect_data = CRect(1, 0, m_image_width, m_image_height);
	mem_dc.Rectangle(&rect_data);
}

void DataListCtrl::RefreshDisplay()
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
		switch (m_display_mode)
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

void DataListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	switch (nSBCode)
	{
	case SB_LINEUP:
		static_cast<dbTableView*>(GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case SB_LINEDOWN:
		static_cast<dbTableView*>(GetParent())->OnMove(ID_RECORD_NEXT);
		break;
	default:
		CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
		break;
	}
}

void DataListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_PRIOR: // page up
		SendMessage(WM_VSCROLL, SB_PAGEUP, NULL);
		break;
	case VK_NEXT: // page down
		SendMessage(WM_VSCROLL, SB_PAGEDOWN, NULL);
		break;
	case VK_UP:
		static_cast<dbTableView*>(GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case VK_DOWN:
		static_cast<dbTableView*>(GetParent())->OnMove(ID_RECORD_NEXT);
		break;

	default:
		CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
		break;
	}
}

ChartData* DataListCtrl::GetDataViewCurrentRecord()
{
	const UINT n_selected_items = GetSelectedCount();
	int nItem = -1;
	ChartData* ptr = nullptr;

	// Update all of the selected items.
	if (n_selected_items > 0)
	{
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		ASSERT(nItem != -1);
		nItem -= GetTopIndex();
		if (nItem >= 0 && nItem < ptr_rows.GetSize())
			ptr = ptr_rows.GetAt(nItem)->pDataChartWnd;
	}
	return ptr;
}

void DataListCtrl::display_data_wnd(CDataListCtrl_Row* ptr, int iImage)
{
	// create objects if necessary : CLineView and AcqDataDoc
	if (ptr->pDataChartWnd == nullptr)
	{
		ptr->pDataChartWnd = new ChartData;
		ASSERT(ptr->pDataChartWnd != NULL);
		ptr->pDataChartWnd->Create(_T("DATAWND"), WS_CHILD, CRect(0, 0, m_image_width, m_image_height), this, iImage * 100);
		ptr->pDataChartWnd->SetbUseDIB(FALSE);
	}
	const auto p_wnd = ptr->pDataChartWnd;
	p_wnd->SetString(ptr->cs_comment);

	// open data document
	if (ptr->pdataDoc == nullptr)
	{
		ptr->pdataDoc = new AcqDataDoc;
		ASSERT(ptr->pdataDoc != NULL);
	}
	if (ptr->csDatafileName.IsEmpty() || !ptr->pdataDoc->OnOpenDocument(ptr->csDatafileName))
	{
		p_wnd->RemoveAllChanlistItems();
		auto comment = _T("File name: ") + ptr->csDatafileName;
		comment += _T(" -- data not available");
		p_wnd->SetString(comment);
	}
	else
	{
		if (ptr->csNspk.IsEmpty())
			p_wnd->GetScopeParameters()->crScopeFill = p_wnd->GetColor(2);
		else
			p_wnd->GetScopeParameters()->crScopeFill = p_wnd->GetColor(15);

		ptr->pdataDoc->ReadDataInfos();
		ptr->cs_comment = ptr->pdataDoc->GetpWaveFormat()->GetComments(_T(" "));
		p_wnd->AttachDataFile(ptr->pdataDoc);
		p_wnd->load_all_channels(m_data_transform);
		p_wnd->load_data_within_window(m_b_set_time_span, m_tFirst, m_tLast);
		p_wnd->adjust_gain(m_b_set_mV_span, m_mV_span);

		ptr->pdataDoc->AcqCloseFile();
	}
	plot_data(ptr, p_wnd, iImage);
}


void DataListCtrl::plot_data(const CDataListCtrl_Row* ptr, ChartData* p_wnd, int iImage)
{
	p_wnd->SetBottomComment(m_b_display_file_name, ptr->csDatafileName);
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

	p_wnd->PlotDataToDC(&mem_dc);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
	mem_dc.MoveTo(1, 0);
	mem_dc.LineTo(1, client_rect.bottom);
	m_image_list.Replace(iImage, &bitmap_plot, nullptr);
}

void DataListCtrl::display_spike_wnd(CDataListCtrl_Row* ptr, int iImage)
{
	// create spike window and spike document if necessary
	if (ptr->pSpikeChartWnd == nullptr)
	{
		ptr->pSpikeChartWnd = new ChartSpikeBar;
		ASSERT(ptr->pSpikeChartWnd != NULL);
		ptr->pSpikeChartWnd->Create(_T("SPKWND"), WS_CHILD, CRect(0, 0, m_image_width, m_image_height), this, ptr->index * 1000);
		ptr->pSpikeChartWnd->SetbUseDIB(FALSE);
	}
	const auto p_wnd = ptr->pSpikeChartWnd;

	// open spike document
	if (ptr->pspikeDoc == nullptr)
	{
		ptr->pspikeDoc = new CSpikeDoc;
		ASSERT(ptr->pspikeDoc != NULL);
	}

	if (ptr->csSpikefileName.IsEmpty() || !ptr->pspikeDoc->OnOpenDocument(ptr->csSpikefileName))
	{
		m_image_list.Replace(iImage, m_p_empty_bitmap, nullptr);
	}
	else
	{
		const auto pParent = static_cast<ViewdbWave*>(GetParent());
		int iTab = pParent->m_tabCtrl.GetCurSel();
		if (iTab < 0)
			iTab = 0;
		const auto pspk_list = ptr->pspikeDoc->set_spk_list_as_current(iTab);
		p_wnd->SetSourceData_spklist_spikedoc(pspk_list, ptr->pspikeDoc);
		p_wnd->set_plot_mode(m_spike_plot_mode, m_selected_class);
		long l_first = 0;
		auto l_last = ptr->pspikeDoc->get_acq_size();
		if (m_b_set_time_span)
		{
			const auto sampling_rate = ptr->pspikeDoc->get_acq_rate();
			l_first = static_cast<long>(m_tFirst * sampling_rate);
			l_last = static_cast<long>(m_tLast * sampling_rate);
		}

		p_wnd->SetTimeIntervals(l_first, l_last);
		if (m_b_set_mV_span)
		{
			const auto volts_per_bin = pspk_list->get_acq_voltsper_bin();
			const auto y_we = static_cast<int>(m_mV_span / 1000.f / volts_per_bin);
			const auto y_wo = pspk_list->get_acq_binzero();
			p_wnd->SetYWExtOrg(y_we, y_wo);
		}
		p_wnd->SetBottomComment(m_b_display_file_name, ptr->csSpikefileName);

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

		//if (pdb_doc != nullptr)
		p_wnd->PlotSingleSpkDataToDC(&mem_dc);

		CPen pen;
		pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
		mem_dc.MoveTo(1, 0);
		mem_dc.LineTo(1, client_rect.bottom);
		m_image_list.Replace(iImage, &bitmap_plot, nullptr);
	}
}

void DataListCtrl::display_empty_wnd(CDataListCtrl_Row* ptr, int iImage)
{
	m_image_list.Replace(iImage, m_p_empty_bitmap, nullptr);
}

void DataListCtrl::ResizeSignalColumn(const int n_pixels)
{
	m_column_width[CTRL_COL_CURVE] = n_pixels;
	m_image_list.DeleteImageList();
	m_image_width = m_column_width[CTRL_COL_CURVE];
	m_image_list.Create(m_image_width, m_image_height, ILC_COLOR4, 10, 10);
	SetImageList(&m_image_list, LVSIL_SMALL);
	m_image_list.SetImageCount(ptr_rows.GetSize());

	for (int i = 0; i < ptr_rows.GetSize(); i++)
	{
		auto* ptr = ptr_rows.GetAt(i);
		SAFE_DELETE(ptr->pDataChartWnd)
		SAFE_DELETE(ptr->pSpikeChartWnd)
	}
	RefreshDisplay();
}

void DataListCtrl::FitColumnsToSize(int n_pixels)
{
	// compute width of fixed columns
	auto fixed_width = 0;
	for (const auto i : m_column_width)
	{
		fixed_width += i;
	}
	fixed_width -= m_column_width[CTRL_COL_CURVE];
	const auto signal_column_width = n_pixels - fixed_width;
	if (signal_column_width != m_column_width[CTRL_COL_CURVE] && signal_column_width > 2)
	{
		SetColumnWidth(CTRL_COL_CURVE, signal_column_width);
		ResizeSignalColumn(signal_column_width);
	}
}
