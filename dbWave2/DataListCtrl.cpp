#include "StdAfx.h"

#include "DataListCtrl.h"
#include "dbTableMain.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "ChartSpikeBar.h"
#include "ViewdbWave.h"
#include "DataListCtrl_Row.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CDataListCtrl::m_column_width[] = {
	1,
	10, 300, 15, 30,
	30, 50, 40, 40,
	40, 40
};
CString CDataListCtrl::m_column_headers[] = {
	__T(""),
	__T("#"), __T("data"), __T("insect ID"), __T("sensillum"),
	__T("stim1"), __T("conc1"), __T("stim2"), __T("conc2"),
	__T("spikes"), __T("flag")
};

int CDataListCtrl::m_column_format[] = {
	LVCFMT_LEFT,
	LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER,
	LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER,
	LVCFMT_CENTER, LVCFMT_CENTER
};

int CDataListCtrl::m_column_index[] = {
	0,
	COL_INDEX, COL_CURVE, COL_INSECT, COL_SENSI,
	COL_STIM1, COL_CONC1, COL_STIM2, COL_CONC2,
	COL_NBSPK, COL_FLAG
};

BEGIN_MESSAGE_MAP(CDataListCtrl, CListCtrl)

	ON_WM_VSCROLL()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_WM_DESTROY()

END_MESSAGE_MAP()

CDataListCtrl::CDataListCtrl()
= default;

CDataListCtrl::~CDataListCtrl()
{
	delete_ptr_array();
	SAFE_DELETE(m_p_empty_bitmap)
}

void CDataListCtrl::OnDestroy()
{
	save_columns_width();
}

void CDataListCtrl::save_columns_width() const
{
	if (m_p_columns_width != nullptr)
	{
		const auto n_columns_stored = m_p_columns_width->GetSize();
		if (n_columns_stored != NCOLS)
			m_p_columns_width->SetSize(NCOLS);
		for (auto i = 0; i < NCOLS; i++)
			m_p_columns_width->SetAt(i, GetColumnWidth(i));
	}
}

void CDataListCtrl::delete_ptr_array()
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

void CDataListCtrl::resize_ptr_array(int nitems)
{
	if (nitems == ptr_rows.GetSize())
		return;

	// Resize m_image_list CImageList
	m_image_list.SetImageCount(nitems);

	// reduce size
	if (ptr_rows.GetSize() > nitems)
	{
		for (auto i = ptr_rows.GetSize() - 1; i >= nitems; i--)
			delete ptr_rows.GetAt(i);
		ptr_rows.SetSize(nitems);
	}
	// grow size
	else
	{
		const auto size_before_change = ptr_rows.GetSize();
		ptr_rows.SetSize(nitems);
		auto index = 0;
		if (size_before_change > 0)
			index = ptr_rows.GetAt(size_before_change - 1)->index;
		for (auto i = size_before_change; i < nitems; i++)
		{
			auto* ptr = new CDataListCtrl_Row;
			ASSERT(ptr != NULL);
			ptr_rows.SetAt(i, ptr);
			index++;
			ptr->index = index;
		}
	}
}

void CDataListCtrl::InitColumns(CUIntArray* picolwidth)
{
	if (picolwidth != nullptr)
	{
		m_p_columns_width = picolwidth;
		const auto ncol_stored = picolwidth->GetSize();
		if (ncol_stored < NCOLS)
			picolwidth->SetSize(NCOLS);
		for (auto i = 0; i < ncol_stored; i++)
			m_column_width[i] = picolwidth->GetAt(i);
	}

	for (int nCol = 0; nCol < NCOLS; nCol++)
	{
		InsertColumn(nCol, m_column_headers[nCol], m_column_format[nCol], m_column_width[nCol], -1);
	}

	m_image_width = m_column_width[COL_CURVE];
	m_image_list.Create(m_image_width, m_image_height, /*ILC_COLORDDB*/ ILC_COLOR4, 10, 10);
	SetImageList(&m_image_list, LVSIL_SMALL);
}

void CDataListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	// check if first if the requested line is stored into the buffer
	auto ifirst_array = 0;
	auto ilast_array = 0;
	if (ptr_rows.GetSize() > 0)
	{
		ifirst_array = ptr_rows.GetAt(0)->index;
		ilast_array = ptr_rows.GetAt(ptr_rows.GetUpperBound())->index;
	}

	// is item within the cache?
	auto* p_disp_info = reinterpret_cast<LV_DISPINFO*>(pNMHDR);
	LV_ITEM* p_item = &(p_disp_info)->item;
	*pResult = 0;
	const auto i_item_index = p_item->iItem;

	// item before first visible item? selected item becomes first (scroll up)
	if (i_item_index < ifirst_array)
	{
		ifirst_array = i_item_index;
		ilast_array = ifirst_array + GetCountPerPage() - 1;
		UpdateCache(ifirst_array, ilast_array);
	}
	// item after last visible item? iItem becomes last visible (scroll down)
	else if (i_item_index > ilast_array)
	{
		ilast_array = i_item_index;
		ifirst_array = ilast_array - GetCountPerPage() + 1;
		UpdateCache(ifirst_array, ilast_array);
	}
	else if (ptr_rows.GetSize() == 0)
		UpdateCache(ifirst_array, ilast_array);

	// now, the requested item is in the cache
	// get data from database
	const auto pdb_doc = static_cast<CViewdbWave*>(GetParent())->GetDocument();
	if (pdb_doc == nullptr)
		return;

	const int i_first_visible = ptr_rows.GetAt(0)->index;
	auto i_cache_index = i_item_index - i_first_visible;
	if (i_cache_index > (ptr_rows.GetSize() - 1))
		i_cache_index = ptr_rows.GetSize() - 1;

	auto* ptr = ptr_rows.GetAt(i_cache_index);

	if (p_item->mask & LVIF_TEXT) //valid text buffer?
	{
		CString cs;
		auto flag = TRUE;
		switch (p_item->iSubItem)
		{
		case COL_CURVE: flag = FALSE;
			break;
		case COL_INDEX: cs.Format(_T("%i"), ptr->index);
			break;
		case COL_INSECT: cs.Format(_T("%i"), ptr->insectID);
			break;
		case COL_SENSI: cs = ptr->csSensillumname;
			break;
		case COL_STIM1: cs = ptr->csStim1;
			break;
		case COL_CONC1: cs = ptr->csConc1;
			break;
		case COL_STIM2: cs = ptr->csStim2;
			break;
		case COL_CONC2: cs = ptr->csConc2;
			break;
		case COL_NBSPK: cs = ptr->csNspk;
			break;
		case COL_FLAG: cs = ptr->csFlag;
			break;
		default: flag = FALSE;
			break;
		}
		if (flag)
			lstrcpy(p_item->pszText, cs);
	}

	// display images
	if (p_item->mask & LVIF_IMAGE
		&& p_item->iSubItem == COL_CURVE)
		p_item->iImage = i_cache_index;
}

void CDataListCtrl::SetCurSel(int recposition)
{
	// get current item which has the focus
	constexpr auto flag = LVNI_FOCUSED | LVNI_ALL;
	const auto current_position = GetNextItem(-1, flag);

	// exit if it is the same
	if (current_position == recposition)
		return;

	// focus new
	if (current_position >= 0)
		SetItemState(current_position, 0, LVIS_SELECTED | LVIS_FOCUSED);

	SetItemState(recposition, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	EnsureVisible(recposition, FALSE);
}

// Update data in cache
// adjust size of the cache if necessary
// create objects if necessary
// scroll or load new data

void CDataListCtrl::UpdateCache(int ifirst, int ilast)
{
	// adjust number of items in the array and adjust ifirst and ilast
	const auto inb_visible = ilast - ifirst + 1;
	if (ifirst < 0)
	{
		ifirst = 0;
		ilast = inb_visible - 1;
	}
	if (ilast < 0 || ilast >= GetItemCount())
	{
		ilast = GetItemCount() - 1;
		ifirst = ilast - inb_visible + 1;
	}

	// resize array if the number of visible records is different from the number of lines / listcontrol
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
	const auto p_dbwave_doc = static_cast<CViewdbWave*>(GetParent())->GetDocument();
	if (p_dbwave_doc == nullptr)
		return;
	const int index_current_file = p_dbwave_doc->GetDB_CurrentRecordPosition();

	// which update is necessary?
	// conditions for out of range (renew all items)
	auto n_to_rebuild = ptr_rows.GetSize(); // number of items to refresh
	auto new1 = 0;
	auto i_first_array = 0;
	if (ptr_rows.GetSize() > 0)
		i_first_array = ptr_rows.GetAt(0)->index;
	const auto difference = ifirst - i_first_array;

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

		// else: scroll out of range

		//ntoTransfer = 0; ////////////////////////////////////////
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

	// set file and update filenames
	int index = new1;

	// left, top, right, bottom
	set_empty_bitmap();
	DB_ITEMDESC desc;

	while (n_to_rebuild > 0)
	{
		// get data; create object if null
		auto ptr = ptr_rows.GetAt(index);

		// create lineview and spike superposition
		ptr->index = index + ifirst;
		p_dbwave_doc->SetDB_CurrentRecordPosition(ptr->index); // select the other file
		ptr->csDatafileName = p_dbwave_doc->GetDB_CurrentDatFileName(TRUE);
		ptr->csSpikefileName = p_dbwave_doc->GetDB_CurrentSpkFileName(TRUE);
		auto p_database = p_dbwave_doc->m_pDB;
		p_database->GetRecordItemValue(CH_IDINSECT, &desc);
		ptr->insectID = desc.lVal;

		// column: stim, conc, type = load indirect data from database
		p_database->GetRecordItemValue(CH_STIM_ID, &desc);
		ptr->csStim1 = desc.csVal;
		p_database->GetRecordItemValue(CH_CONC_ID, &desc);
		ptr->csConc1 = desc.csVal;
		p_database->GetRecordItemValue(CH_STIM2_ID, &desc);
		ptr->csStim2 = desc.csVal;
		p_database->GetRecordItemValue(CH_CONC2_ID, &desc);
		ptr->csConc2 = desc.csVal;

		p_database->GetRecordItemValue(CH_SENSILLUM_ID, &desc);
		ptr->csSensillumname = desc.csVal;

		p_database->GetRecordItemValue(CH_FLAG, &desc);
		ptr->csFlag.Format(_T("%i"), desc.lVal);

		// colum: number of spike = verify that spike file is defined, if yes, load nb spikes
		if (p_dbwave_doc->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
			ptr->csNspk.Empty();
		else
		{
			p_database->GetRecordItemValue(CH_NSPIKES, &desc);
			const int nspikes = desc.lVal;
			p_database->GetRecordItemValue(CH_NSPIKECLASSES, &desc);
			ptr->csNspk.Format(_T("%i (%i classes)"), nspikes, desc.lVal);
		}

		// build bitmap corresponding to data/spikes/nothing
		switch (m_display_mode)
		{
		// data mode
		case 1:
			display_data_wnd(ptr, index);
			break;
		// spike bars
		case 2:
			display_spike_wnd(ptr, index);
			break;
		default:
			display_empty_wnd(ptr, index);
			break;
		}
		index++;
		n_to_rebuild--;
	}

	// restore document conditions
	if (index_current_file >= 0)
		p_dbwave_doc->SetDB_CurrentRecordPosition(index_current_file);
}

///////////////////////////////////////////////////////////////////////////////
void CDataListCtrl::set_empty_bitmap(const BOOL b_forced_update)
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
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // black
	mem_dc.SelectObject(&pen);
	auto rect_data = CRect(1, 0, m_image_width, m_image_height);
	mem_dc.Rectangle(&rect_data);
}

void CDataListCtrl::RefreshDisplay()
{
	if (ptr_rows.GetSize() == NULL)
		return;
	const int ifirst_row = ptr_rows.GetAt(0)->index;
	const int ilast_row = ptr_rows.GetAt(ptr_rows.GetUpperBound())->index;

	// left, top, right, bottom
	set_empty_bitmap();

	const auto nrows = ptr_rows.GetSize();
	for (auto index = 0; index < nrows; index++)
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
	RedrawItems(ifirst_row, ilast_row); // CCtrlList standard function
	Invalidate();
	UpdateWindow();
}

void CDataListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	switch (nSBCode)
	{
	case SB_LINEUP:
		static_cast<CdbTableView*>(GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case SB_LINEDOWN:
		static_cast<CdbTableView*>(GetParent())->OnMove(ID_RECORD_NEXT);
		break;
	default:
		CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
		break;
	}
}

void CDataListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
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
		static_cast<CdbTableView*>(GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case VK_DOWN:
		static_cast<CdbTableView*>(GetParent())->OnMove(ID_RECORD_NEXT);
		break;

	default:
		CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
		break;
	}
}

ChartData* CDataListCtrl::GetDataViewCurrentRecord()
{
	UINT uSelectedCount = GetSelectedCount();
	int nItem = -1;
	ChartData* ptr = nullptr;

	// Update all of the selected items.
	if (uSelectedCount > 0)
	{
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		ASSERT(nItem != -1);
		nItem -= GetTopIndex();
		if (nItem >= 0 && nItem < ptr_rows.GetSize())
			ptr = ptr_rows.GetAt(nItem)->pDataChartWnd;
	}
	return ptr;
}

void CDataListCtrl::display_data_wnd(CDataListCtrl_Row* ptr, int iImage)
{
	// create objects if necessary : CLineView and AcqDataDoc
	if (ptr->pDataChartWnd == nullptr)
	{
		ptr->pDataChartWnd = new ChartData;
		ASSERT(ptr->pDataChartWnd != NULL);
		ptr->pDataChartWnd->Create(_T("DATAWND"), WS_CHILD, CRect(0, 0, m_image_width, m_image_height), this, iImage * 100);
		ptr->pDataChartWnd->SetbUseDIB(FALSE);
	}
	auto p_wnd = ptr->pDataChartWnd;
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
		p_wnd->AttachDataFile(ptr->pdataDoc);

		// display all channels : loop through all doc channels & add if necessary
		CWaveFormat* pwaveFormat = ptr->pdataDoc->GetpWaveFormat();
		ptr->cs_comment = pwaveFormat->GetComments(_T(" "));
		const int ndocchans = pwaveFormat->scan_count;
		auto lnvchans = p_wnd->GetChanlistSize();
		// add channels if value is zero
		// channels were all removed if file was not found in an earlier round
		for (auto jdocchan = 0; jdocchan < ndocchans; jdocchan++)
		{
			// check if present in the list
			auto b_present = FALSE; // pessimistic
			for (auto j = lnvchans - 1; j >= 0; j--) // check all channels / display list
			{
				// test if this data chan is present + no transf
				if (p_wnd->GetChanlistItem(j)->GetSourceChan() == jdocchan)
				{
					b_present = TRUE; // the wanted chan is present: stop loopint through disp list
					p_wnd->SetChanlistTransformMode(j, m_data_transform);
					break; // and examine next doc channel
				}
			}
			if (b_present == FALSE)
			{
				p_wnd->AddChanlistItem(jdocchan, m_data_transform);
				lnvchans++;
			}
			p_wnd->GetChanlistItem(jdocchan)->SetColor(jdocchan);
		}

		// load real data from file and update time parameters
		const auto npixels = p_wnd->GetRectWidth();
		const int doc_chan_len = ptr->pdataDoc->GetDOCchanLength();
		long l_first = 0;
		long l_last = doc_chan_len - 1;
		if (m_b_set_time_span)
		{
			const auto p_wave_format = ptr->pdataDoc->GetpWaveFormat();
			const auto samplingrate = p_wave_format->sampling_rate_per_channel;
			l_first = static_cast<long>(m_tFirst * samplingrate);
			l_last = static_cast<long>(m_tLast * samplingrate);
			if (l_last == l_first)
				l_last++;
		}
		p_wnd->ResizeChannels(npixels, l_last - l_first + 1);
		p_wnd->GetDataFromDoc(l_first, l_last);

		// maximize gain & split curves
		auto j = lnvchans - 1;
		int max, min;
		for (auto i = j; i >= 0; i--)
		{
			CChanlistItem* pchan = p_wnd->GetChanlistItem(i);
			pchan->GetMaxMin(&max, &min);
			auto ispan = max - min + 1;
			int iextent;
			int izero;
			if (m_b_set_mV_span)
			{
				ispan = pchan->ConvertVoltsToDataBins(m_mV_span / 1000.f);
				izero = pchan->GetDataBinZero();
				iextent = ispan;
			}
			else
			{
				// split curves
				iextent = MulDiv(ispan, 11 * lnvchans, 10);
				izero = (max + min) / 2 - MulDiv(iextent, j, lnvchans * 2);
			}
			j -= 2;
			pchan->SetYextent(iextent);
			pchan->SetYzero(izero);
		}
		ptr->pdataDoc->AcqCloseFile();
	}

	// plot data
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

	p_wnd->PlotDatatoDC(&mem_dc);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
	mem_dc.MoveTo(1, 0);
	mem_dc.LineTo(1, client_rect.bottom);
	m_image_list.Replace(iImage, &bitmap_plot, nullptr);
}

void CDataListCtrl::display_spike_wnd(CDataListCtrl_Row* ptr, int iImage)
{
	// create spike window and spike document if necessary
	if (ptr->pSpikeChartWnd == nullptr)
	{
		ptr->pSpikeChartWnd = new CChartSpikeBarWnd;
		ASSERT(ptr->pSpikeChartWnd != NULL);
		ptr->pSpikeChartWnd->Create(_T("SPKWND"), WS_CHILD, CRect(0, 0, m_image_width, m_image_height), this, ptr->index * 1000);
		ptr->pSpikeChartWnd->SetbUseDIB(FALSE);
	}
	auto p_wnd = ptr->pSpikeChartWnd;

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
		auto pParent = static_cast<CViewdbWave*>(GetParent());
		int iTab = pParent->m_tabCtrl.GetCurSel();
		if (iTab < 0)
			iTab = 0;
		const auto pspk_list = ptr->pspikeDoc->SetSpkList_AsCurrent(iTab);
		p_wnd->SetSourceData(pspk_list, ptr->pspikeDoc);
		p_wnd->SetPlotMode(m_spike_plot_mode, m_selected_class);
		long l_first = 0;
		auto l_last = ptr->pspikeDoc->GetAcqSize();
		if (m_b_set_time_span)
		{
			const auto sampling_rate = ptr->pspikeDoc->GetAcqRate();
			l_first = static_cast<long>(m_tFirst * sampling_rate);
			l_last = static_cast<long>(m_tLast * sampling_rate);
		}

		p_wnd->SetTimeIntervals(l_first, l_last);
		if (m_b_set_mV_span)
		{
			const auto vperbin = pspk_list->GetAcqVoltsperBin();
			const auto y_we = static_cast<int>(m_mV_span / 1000.f / vperbin);
			const auto y_wo = pspk_list->GetAcqBinzero();
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
		p_wnd->PlotSingleSpkDatatoDC(&mem_dc);

		CPen pen;
		pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
		mem_dc.MoveTo(1, 0);
		mem_dc.LineTo(1, client_rect.bottom);
		m_image_list.Replace(iImage, &bitmap_plot, nullptr);
	}
}

void CDataListCtrl::display_empty_wnd(CDataListCtrl_Row* ptr, int iImage)
{
	m_image_list.Replace(iImage, m_p_empty_bitmap, nullptr);
}

void CDataListCtrl::ResizeSignalColumn(int npixels)
{
	m_column_width[COL_CURVE] = npixels;
	m_image_list.DeleteImageList();
	m_image_width = m_column_width[COL_CURVE];
	m_image_list.Create(m_image_width, m_image_height, /*ILC_COLORDDB*/ ILC_COLOR4, 10, 10);
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

void CDataListCtrl::FitColumnsToSize(int npixels)
{
	// compute size of fixed columns
	auto fixedsize = 0;
	for (auto i : m_column_width)
	{
		fixedsize += i;
	}
	fixedsize -= m_column_width[COL_CURVE];
	const auto signalcolsize = npixels - fixedsize;
	if (signalcolsize != m_column_width[COL_CURVE] && signalcolsize > 2)
	{
		SetColumnWidth(COL_CURVE, signalcolsize);
		ResizeSignalColumn(signalcolsize);
	}
}
