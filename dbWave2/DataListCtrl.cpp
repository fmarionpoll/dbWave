#include "StdAfx.h"

#include "DataListCtrl.h"
#include "dbTableMain.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "ChartSpikeBar.h"
#include "CViewdbWave.h"

#include "DataListCtrl_Row.h"
//#include "DataListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CDataListCtrl::m_colwidth[] = {
	1,
	10, 300, 15, 30,
	30, 50, 40, 40,
	40, 40
};
CString CDataListCtrl::m_colheaders[] = {
	__T(""),
	__T("#"), __T("data"), __T("insect ID"), __T("sensillum"),
	__T("stim1"), __T("conc1"), __T("stim2"), __T("conc2"),
	__T("spikes"), __T("flag")
};

int CDataListCtrl::m_colfmt[] = {
	LVCFMT_LEFT,
	LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER,
	LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER, LVCFMT_CENTER,
	LVCFMT_CENTER, LVCFMT_CENTER
};

int CDataListCtrl::m_colindex[] = {
	0,
	COL_INDEX, COL_CURVE, COL_INSECT, COL_SENSI,
	COL_STIM1, COL_CONC1, COL_STIM2, COL_CONC2,
	COL_NBSPK, COL_FLAG
};

/////////////////////////////////////////////////////////////////////////////
// CDataListCtrl

CDataListCtrl::CDataListCtrl()
{
	m_cx = 400;
	m_cy = 50;
	m_dattransform = 0;
	m_picolwidth = nullptr;
	m_displaymode = 1;
	m_pEmptyBitmap = nullptr;
	m_tFirst = 0.f;
	m_tLast = 0.f;
	m_mVspan = 0.f;
	m_bDisplayFileName = FALSE;
	m_bsetTimeSpan = FALSE;
	m_bsetmVSpan = FALSE;
	m_spikeplotmode = PLOT_BLACK;
	m_selclass = 0;
}

CDataListCtrl::~CDataListCtrl()
{
	deletePtrArray();
}

void CDataListCtrl::OnDestroy()
{
	if (m_picolwidth != nullptr)
	{
		const auto nbcols_stored = m_picolwidth->GetSize();
		if (nbcols_stored != NCOLS)
			m_picolwidth->SetSize(NCOLS);
		for (auto i = 0; i < NCOLS; i++)
			m_picolwidth->SetAt(i, GetColumnWidth(i));
	}
	SAFE_DELETE(m_pEmptyBitmap);
	//CListCtrl::OnDestroy();
}

void CDataListCtrl::deletePtrArray()
{
	if (ptrArray.GetSize() == NULL)
		return;
	const auto imax = ptrArray.GetSize();
	for (auto i = 0; i < imax; i++)
	{
		auto* ptr = ptrArray.GetAt(i);
		SAFE_DELETE(ptr);
	}
	ptrArray.RemoveAll();
}

void CDataListCtrl::resizePtrArray(int nitems)
{
	if (nitems == ptrArray.GetSize())
		return;

	// Resize m_imagelist CImageList
	m_imagelist.SetImageCount(nitems);

	// reduce size
	if (ptrArray.GetSize() > nitems)
	{
		for (auto i = ptrArray.GetSize() - 1; i >= nitems; i--)
			delete ptrArray.GetAt(i);
		ptrArray.SetSize(nitems);
	}
	// grow size
	else
	{
		const auto lowernew = ptrArray.GetSize();
		ptrArray.SetSize(nitems);
		auto index = 0;
		if (lowernew > 0)
			index = ptrArray.GetAt(lowernew - 1)->index;
		for (auto i = lowernew; i < nitems; i++)
		{
			auto* ptr = new CDataListCtrl_Row;
			ASSERT(ptr != NULL);
			ptrArray.SetAt(i, ptr);
			index++;
			ptr->index = index;
		}
	}
}

void CDataListCtrl::InitColumns(CUIntArray* picolwidth)
{
	if (picolwidth != nullptr)
	{
		m_picolwidth = picolwidth;
		const auto ncol_stored = picolwidth->GetSize();
		if (ncol_stored < NCOLS)
			picolwidth->SetSize(NCOLS);
		for (auto i = 0; i < ncol_stored; i++)
			m_colwidth[i] = picolwidth->GetAt(i);
	}

	for (int nCol = 0; nCol < NCOLS; nCol++)
	{
		InsertColumn(nCol, m_colheaders[nCol], m_colfmt[nCol], m_colwidth[nCol], -1);
	}

	m_cx = m_colwidth[COL_CURVE];
	m_imagelist.Create(m_cx, m_cy, /*ILC_COLORDDB*/ ILC_COLOR4, 10, 10);
	SetImageList(&m_imagelist, LVSIL_SMALL);
}

/////////////////////////////////////////////////////////////////////////////
// CDataListCtrl message handlers

BEGIN_MESSAGE_MAP(CDataListCtrl, CListCtrl)

	ON_WM_DESTROY()
	ON_WM_VSCROLL()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)

END_MESSAGE_MAP()


void CDataListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	// check if first if the requested line is stored into the buffer
	auto ifirst_array = 0;
	auto ilast_array = 0;
	if (ptrArray.GetSize() > 0)
	{
		ifirst_array = ptrArray.GetAt(0)->index;
		ilast_array = ptrArray.GetAt(ptrArray.GetUpperBound())->index;
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
	else if (ptrArray.GetSize() == 0)
		UpdateCache(ifirst_array, ilast_array);

	// now, the requested item is in the cache
	// get data from database
	const auto pdb_doc = static_cast<CViewdbWave*>(GetParent())->GetDocument();
	if (pdb_doc == nullptr)
		return;

	const int ifirstvisible = ptrArray.GetAt(0)->index;
	auto icache_index = i_item_index - ifirstvisible;
	if (icache_index > (ptrArray.GetSize() - 1))
		icache_index = ptrArray.GetSize() - 1;

	auto* ptr = ptrArray.GetAt(icache_index);

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
		p_item->iImage = icache_index;
}

void CDataListCtrl::SetCurSel(int recposition)
{
	// get current item which has the focus
	const auto flag = LVNI_FOCUSED | LVNI_ALL;
	const auto curpos = GetNextItem(-1, flag);

	// exit if it is the same
	if (curpos == recposition)
		return;

	// focus new
	if (curpos >= 0)
		SetItemState(curpos, 0, LVIS_SELECTED | LVIS_FOCUSED);

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
	if (inb_visible != ptrArray.GetSize())
	{
		// if cache size increases, erase old information (set flag)
		if (inb_visible > ptrArray.GetSize())
			b_forced_update = TRUE;
		// if cache size decreases, just delete extra rows
		resizePtrArray(inb_visible);
	}

	// get data file pointer and pointer to database
	auto p_dbwave_doc = static_cast<CViewdbWave*>(GetParent())->GetDocument();
	if (p_dbwave_doc == nullptr)
		return;
	const int indexcurrentfile = p_dbwave_doc->GetDB_CurrentRecordPosition();

	// which update is necessary?
	// conditions for out of range (renew all items)
	auto idelta = 0; // flag (copy forwards or backwards)
	auto n_to_transfer = 0; // number of items to copy
	auto n_to_rebuild = ptrArray.GetSize(); // number of items to refresh
	auto idest1 = 0; // index first item to be exchg with source
	auto isource1 = 0; // index first source item
	auto inew1 = 0;
	auto ifirst_array = 0;
	if (ptrArray.GetSize() > 0)
		ifirst_array = ptrArray.GetAt(0)->index;
	const auto difference = ifirst - ifirst_array;

	// change indexes according to case
	// scroll up (go forwards i.e. towards indexes of higher value)
	if (!b_forced_update)
	{
		if (difference > 0 && difference < ptrArray.GetSize())
		{
			idelta = 1; // copy forward
			n_to_transfer = ptrArray.GetSize() - difference;
			n_to_rebuild -= n_to_transfer;
			isource1 = difference;
			idest1 = 0;
			inew1 = n_to_transfer;
		}
		// scroll down (go backwards i.e. towards indexes of lower value)
		else if (difference < 0 && -difference < ptrArray.GetSize())
		{
			idelta = -1;
			n_to_transfer = ptrArray.GetSize() + difference;
			n_to_rebuild -= n_to_transfer;
			isource1 = n_to_transfer - 1;
			idest1 = ptrArray.GetSize() - 1;
			inew1 = 0;
		}

		// else: scroll out of range

		//ntoTransfer = 0; ////////////////////////////////////////
		auto isource = isource1;
		auto idest = idest1;
		while (n_to_transfer > 0)
		{
			// exchange objects
			auto* p_source = ptrArray.GetAt(isource);
			const auto p_dest = ptrArray.GetAt(idest);
			ptrArray.SetAt(idest, p_source);
			ptrArray.SetAt(isource, p_dest);
			m_imagelist.Copy(idest, isource, ILCF_SWAP);

			// update indexes
			isource += idelta;
			idest += idelta;
			n_to_transfer--;
		}
	}

	// set file and update filenames
	// the following lines are fed with new data (data are obsolete)
	//ntoRebuild = m_nbptrItems; //////////////////////////////
	//inew1 = 0; //////////////////////////////////////////////
	int index = inew1;

	// left, top, right, bottom
	setEmptyBitmap();
	DB_ITEMDESC desc;

	while (n_to_rebuild > 0)
	{
		// get data; create object if null
		auto ptr = ptrArray.GetAt(index);

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
		switch (m_displaymode)
		{
		// data mode
		case 1:
			displayDataWnd(ptr, index);
			break;
		// spike bars
		case 2:
			displaySpikeWnd(ptr, index);
			break;
		default:
			displayEmptyWnd(ptr, index);
			break;
		}
		index++;
		n_to_rebuild--;
	}

	// restore document conditions
	if (indexcurrentfile >= 0)
		p_dbwave_doc->SetDB_CurrentRecordPosition(indexcurrentfile);
}

///////////////////////////////////////////////////////////////////////////////
void CDataListCtrl::setEmptyBitmap(const BOOL b_forced_update)
{
	if (m_pEmptyBitmap != nullptr && !b_forced_update)
		return;

	SAFE_DELETE(m_pEmptyBitmap);
	m_pEmptyBitmap = new CBitmap;

	CWindowDC dc(this);
	CDC mem_dc;
	VERIFY(mem_dc.CreateCompatibleDC(&dc));
	m_pEmptyBitmap->CreateBitmap(m_cx, m_cy, dc.GetDeviceCaps(PLANES), dc.GetDeviceCaps(BITSPIXEL), nullptr);
	mem_dc.SelectObject(m_pEmptyBitmap);
	mem_dc.SetMapMode(dc.GetMapMode());

	CBrush brush(RGB(204, 204, 204)); //light gray
	mem_dc.SelectObject(&brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // black
	mem_dc.SelectObject(&pen);
	auto rect_data = CRect(1, 0, m_cx, m_cy);
	mem_dc.Rectangle(&rect_data);
}

void CDataListCtrl::RefreshDisplay()
{
	if (ptrArray.GetSize() == NULL)
		return;
	const int ifirst_row = ptrArray.GetAt(0)->index;
	const int ilast_row = ptrArray.GetAt(ptrArray.GetUpperBound())->index;

	// left, top, right, bottom
	setEmptyBitmap();

	const auto nrows = ptrArray.GetSize();
	for (auto index = 0; index < nrows; index++)
	{
		auto* ptr = ptrArray.GetAt(index);
		if (ptr == nullptr)
			continue;
		switch (m_displaymode)
		{
		case 1:
			displayDataWnd(ptr, index);
			break;
		case 2:
			displaySpikeWnd(ptr, index);
			break;
		default:
			displayEmptyWnd(ptr, index);
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
		if (nItem >= 0 && nItem < ptrArray.GetSize())
			ptr = ptrArray.GetAt(nItem)->pDataChartWnd;
	}
	return ptr;
}

void CDataListCtrl::displayDataWnd(CDataListCtrl_Row* ptr, int iImage)
{
	// create objects if necessary : CLineView and AcqDataDoc
	if (ptr->pDataChartWnd == nullptr)
	{
		ptr->pDataChartWnd = new ChartData;
		ASSERT(ptr->pDataChartWnd != NULL);
		ptr->pDataChartWnd->Create(_T("DATAWND"), WS_CHILD, CRect(0, 0, m_cx, m_cy), this, iImage * 100);
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
					p_wnd->SetChanlistTransformMode(j, m_dattransform);
					break; // and examine next doc channel
				}
			}
			if (b_present == FALSE)
			{
				p_wnd->AddChanlistItem(jdocchan, m_dattransform);
				lnvchans++;
			}
			p_wnd->GetChanlistItem(jdocchan)->SetColor(jdocchan);
		}

		// load real data from file and update time parameters
		const auto npixels = p_wnd->GetRectWidth();
		const int doc_chan_len = ptr->pdataDoc->GetDOCchanLength();
		long l_first = 0;
		long l_last = doc_chan_len - 1;
		if (m_bsetTimeSpan)
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
			if (m_bsetmVSpan > 0.f)
			{
				ispan = pchan->ConvertVoltsToDataBins(m_mVspan / 1000.f);
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
	p_wnd->SetBottomComment(m_bDisplayFileName, ptr->csDatafileName);
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
	m_imagelist.Replace(iImage, &bitmap_plot, nullptr);
}

void CDataListCtrl::displaySpikeWnd(CDataListCtrl_Row* ptr, int iImage)
{
	// create spike window and spike document if necessary
	if (ptr->pSpikeChartWnd == nullptr)
	{
		ptr->pSpikeChartWnd = new CChartSpikeBarWnd;
		ASSERT(ptr->pSpikeChartWnd != NULL);
		ptr->pSpikeChartWnd->Create(_T("SPKWND"), WS_CHILD, CRect(0, 0, m_cx, m_cy), this, ptr->index * 1000);
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
		m_imagelist.Replace(iImage, m_pEmptyBitmap, nullptr);
	}
	else
	{
		auto pParent = static_cast<CViewdbWave*>(GetParent());
		int iTab = pParent->m_tabCtrl.GetCurSel();
		if (iTab < 0)
			iTab = 0;
		const auto pspk_list = ptr->pspikeDoc->SetSpkList_AsCurrent(iTab);
		p_wnd->SetSourceData(pspk_list, ptr->pspikeDoc);
		p_wnd->SetPlotMode(m_spikeplotmode, m_selclass);
		long l_first = 0;
		auto l_last = ptr->pspikeDoc->GetAcqSize();
		if (m_bsetTimeSpan)
		{
			const auto samplingrate = ptr->pspikeDoc->GetAcqRate();
			l_first = static_cast<long>(m_tFirst * samplingrate);
			l_last = static_cast<long>(m_tLast * samplingrate);
		}

		p_wnd->SetTimeIntervals(l_first, l_last);
		if (m_bsetmVSpan > 0.f)
		{
			const auto vperbin = pspk_list->GetAcqVoltsperBin();
			const auto y_we = static_cast<int>(m_mVspan / 1000.f / vperbin);
			const auto y_wo = pspk_list->GetAcqBinzero();
			p_wnd->SetYWExtOrg(y_we, y_wo);
		}
		p_wnd->SetBottomComment(m_bDisplayFileName, ptr->csSpikefileName);

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
		m_imagelist.Replace(iImage, &bitmap_plot, nullptr);
	}
}

void CDataListCtrl::displayEmptyWnd(CDataListCtrl_Row* ptr, int iImage)
{
	m_imagelist.Replace(iImage, m_pEmptyBitmap, nullptr);
}

void CDataListCtrl::ResizeSignalColumn(int npixels)
{
	m_colwidth[COL_CURVE] = npixels;
	m_imagelist.DeleteImageList();
	m_cx = m_colwidth[COL_CURVE];
	m_imagelist.Create(m_cx, m_cy, /*ILC_COLORDDB*/ ILC_COLOR4, 10, 10);
	SetImageList(&m_imagelist, LVSIL_SMALL);
	m_imagelist.SetImageCount(ptrArray.GetSize());

	for (int i = 0; i < ptrArray.GetSize(); i++)
	{
		auto* ptr = ptrArray.GetAt(i);
		SAFE_DELETE(ptr->pDataChartWnd);
		SAFE_DELETE(ptr->pSpikeChartWnd);
	}
	RefreshDisplay();
}

void CDataListCtrl::FitColumnsToSize(int npixels)
{
	// compute size of fixed columns
	auto fixedsize = 0;
	for (auto i : m_colwidth)
	{
		fixedsize += i;
	}
	fixedsize -= m_colwidth[COL_CURVE];
	const auto signalcolsize = npixels - fixedsize;
	if (signalcolsize != m_colwidth[COL_CURVE] && signalcolsize > 2)
	{
		SetColumnWidth(COL_CURVE, signalcolsize);
		ResizeSignalColumn(signalcolsize);
	}
}
