// DataListCtrl.cpp : implementation file
//

#include "StdAfx.h"

#include "DataListCtrl.h"

#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "scopescr.h"
//#include "spikeshape.h"
//#include "Envelope.h" 
//#include "chanlistitem.h"
#include "Lineview.h"
#include "Spikebar.h"
#include "ViewdbWave.h"

#include "DataListCtrlRowObject.h"
//#include "DataListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CDataListCtrl::m_icolwidth[] = { 1, 10, 300, 15, 30, 30, 50, 40, 40, 40, 40 };

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
	DeletePtrArray();
}

void CDataListCtrl::OnDestroy() 
{
	if (m_picolwidth != nullptr)
	{
		const auto nbcols_stored = m_picolwidth->GetSize();
		if (nbcols_stored != NCOLS)
			m_picolwidth->SetSize(NCOLS);
		for (auto i=0; i< NCOLS; i++)
			m_picolwidth->SetAt(i, GetColumnWidth(i));
	}
	SAFE_DELETE(m_pEmptyBitmap);
	CListCtrl::OnDestroy();
}

// delete objects then the array

void CDataListCtrl::DeletePtrArray()
{
	if (datalistctrlrowobject_prt_array.GetSize() == NULL)
		return;
	const auto imax = datalistctrlrowobject_prt_array.GetSize();
	for (auto i=0; i< imax; i++)
	{
		auto* ptr = datalistctrlrowobject_prt_array.GetAt(i);
		SAFE_DELETE(ptr);
	}
	datalistctrlrowobject_prt_array.RemoveAll();
}

// change size of the array
// delete items or add items accordingly

void CDataListCtrl::ResizePtrArray(int nitems)
{
	if (nitems == datalistctrlrowobject_prt_array.GetSize())
		return;

	// Resize m_imagelist CImageList
	m_imagelist.SetImageCount(nitems);

	// reduce size
	if (datalistctrlrowobject_prt_array.GetSize() > nitems)
	{
		for (int i=datalistctrlrowobject_prt_array.GetSize()-1; i>= nitems; i--)
			delete datalistctrlrowobject_prt_array.GetAt(i);
		datalistctrlrowobject_prt_array.SetSize(nitems);
	}
	// grow size
	else
	{
		const auto lowernew = datalistctrlrowobject_prt_array.GetSize();
		datalistctrlrowobject_prt_array.SetSize(nitems);
		int index = 0;
		if (lowernew > 0)
			index = datalistctrlrowobject_prt_array.GetAt(lowernew-1)->index;
		for (int i=lowernew; i< nitems; i++)
		{
			auto* ptr = new CDataListCtrlRowObject;
			ASSERT(ptr != NULL);
			datalistctrlrowobject_prt_array.SetAt(i, ptr);
			index++;
			ptr->index = index;
		}
	}
}

BEGIN_MESSAGE_MAP(CDataListCtrl, CListCtrl)
	
	ON_WM_DESTROY()
	ON_WM_VSCROLL()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataListCtrl message handlers

void CDataListCtrl::InitColumns(CUIntArray* picolwidth)
{
	if (picolwidth != nullptr)
	{
		m_picolwidth = picolwidth;
		const auto ncol_stored = picolwidth->GetSize();
		if (ncol_stored < NCOLS)
			picolwidth->SetSize(NCOLS);
		for (auto i=0; i< ncol_stored; i++)
			m_icolwidth[i] = picolwidth->GetAt(i);
	}

	LVCOLUMN lvcol;
	const UINT mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	lvcol.mask = mask;
	/*
valid members: LVCF_FMT - fmt | LVCF_SUBITEM - iSubItem | LVCF_TEXT - pszText | LVCF_WIDTH - cx | LVCF_ORDER - iOrder 
LVCF_IMAGE		Version 4.70. The iImage member is valid. 
*/
	auto i_order = 0;		// trick to allow displaying rec
	lvcol.pszText = __T("");
	lvcol.fmt = LVCFMT_LEFT;
	lvcol.iOrder = i_order;
	lvcol.cx = 1;
	InsertColumn(i_order, &lvcol);
	
	ASSERT(i_order == 0);		// make sure the first item is 0

	i_order = COL_INDEX;			// 0
	lvcol.pszText = __T("#");
	lvcol.fmt = LVCFMT_CENTER;
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);

	i_order = COL_CURVE;			// 1
	lvcol.pszText = __T("data");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);

	i_order = COL_INSECT;			// 2
	lvcol.pszText = __T("insect ID");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);

	// -----------------------------

	i_order++;			// 3
	lvcol.pszText = __T("sensillum");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);

	i_order++;			// 4
	lvcol.pszText = __T("stim1");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);
	
	i_order++;			// 5
	lvcol.pszText = __T("conc1");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);

	i_order++;			// 6
	lvcol.pszText = __T("stim2");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);
	
	i_order++;			// 7
	lvcol.pszText = __T("conc2");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);

	i_order++;			// 8
	lvcol.pszText = _T("spikes");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);

	i_order++;			// 9
	lvcol.pszText = __T("flag");
	lvcol.iOrder = i_order;
	lvcol.cx = m_icolwidth[i_order];
	InsertColumn(i_order, &lvcol);

	m_cx = m_icolwidth[COL_CURVE];
	m_imagelist.Create( m_cx, m_cy, /*ILC_COLORDDB*/ ILC_COLOR4, 10, 10 ); 
	SetImageList(&m_imagelist, LVSIL_SMALL );

	// change format of column zero
	//LV_COLUMN lvColumn;
	//memset(&lvColumn, 0, sizeof(lvColumn));
	//lvColumn.mask = LVCF_FMT;
	//GetColumn(0, &lvColumn);
	//lvColumn.fmt = LVCFMT_CENTER;
	//SetColumn(0, &lvColumn);

	//CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	//ASSERT(NULL != pHeaderCtrl);
}


void CDataListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// check if first if the requested line is stored into the buffer
	auto ifirst_array = 0;
	auto ilast_array = 0;
	if (datalistctrlrowobject_prt_array.GetSize() > 0)
	{
		ifirst_array = datalistctrlrowobject_prt_array.GetAt(0)->index;
		ilast_array = datalistctrlrowobject_prt_array.GetAt(datalistctrlrowobject_prt_array.GetUpperBound())->index;
	}

	// is item within the cache?
	auto* p_disp_info = reinterpret_cast<LV_DISPINFO*>(pNMHDR);
	LV_ITEM* p_item= &(p_disp_info)->item;
	*pResult = 0;
	const auto i_item_index = p_item->iItem;

	// item before first visible item? selected item becomes first (scroll up)
	if (i_item_index < ifirst_array)
	{
		ifirst_array = i_item_index;
		ilast_array = ifirst_array + GetCountPerPage() -1;
		UpdateCache(ifirst_array, ilast_array);
	}
	// item after last visible item? iItem becomes last visible (scroll down)
	else if (i_item_index > ilast_array)
	{
		ilast_array = i_item_index;
		ifirst_array = ilast_array - GetCountPerPage() +1;
		UpdateCache(ifirst_array, ilast_array);
	}
	else if (datalistctrlrowobject_prt_array.GetSize() == 0)
		UpdateCache(ifirst_array, ilast_array);
	
	// now, the requested item is in the cache
	// get data from database
	const auto pdb_doc =  ((CViewdbWave*) GetParent())->GetDocument();
	if (pdb_doc == nullptr)
		return;

	const int ifirstvisible = datalistctrlrowobject_prt_array.GetAt(0)->index;
	auto icache_index = i_item_index - ifirstvisible;
	if (icache_index > (datalistctrlrowobject_prt_array.GetSize() -1))
		icache_index = datalistctrlrowobject_prt_array.GetSize() -1;

	auto* ptr = datalistctrlrowobject_prt_array.GetAt(icache_index);

	if (p_item->mask & LVIF_TEXT) //valid text buffer?
	{
		CString cs;
		auto flag = TRUE;
		switch(p_item->iSubItem)
		{
		case COL_CURVE:	flag = FALSE;  break;
		case COL_INDEX:	cs.Format(_T("%i"), ptr->index); break;
		case COL_INSECT:cs.Format(_T("%i"), ptr->insectID); break;
		case COL_SENSI:	cs = ptr->csSensillumname; break;
		case COL_STIM1:	cs = ptr->csStim1; break;
		case COL_CONC1:	cs = ptr->csConc1; break;
		case COL_STIM2:	cs = ptr->csStim2; break;
		case COL_CONC2:	cs = ptr->csConc2; break;
		case COL_NBSPK:	cs = ptr->csNspk; break;
		case COL_FLAG:	cs = ptr->csFlag; break;
		default:		flag = FALSE;  break;
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
	const auto curpos = GetNextItem(-1, flag );
	
	// exit if it is the same
	if (curpos == recposition)
		return;

	// focus new
	if (curpos >= 0)
		SetItemState(curpos, 0, LVIS_SELECTED|LVIS_FOCUSED);

	SetItemState(recposition, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	EnsureVisible(recposition, FALSE);
}

// Update data in cache
// adjust size of the cache if necessary
// create objects if necessary
// scroll or load new data
	
void CDataListCtrl::UpdateCache (int ifirst, int ilast)
{
	auto ifirst_array = 0;
	if (datalistctrlrowobject_prt_array.GetSize() > 0)
	{
		ifirst_array = datalistctrlrowobject_prt_array.GetAt(0)->index;
	}

	// adjust number of items in the array and adjust ifirst and ilast
	const auto inb_visible = ilast - ifirst +1;
	
	// check ifirst & ilast
	if (ifirst < 0)
	{
		ifirst = 0;
		ilast = inb_visible - 1;
	}
	if (ilast < 0 ||  ilast >= GetItemCount( ))
	{
		ilast = GetItemCount( ) - 1;
		ifirst = ilast - inb_visible +1;
	}

	// resize array if the number of visible records is different from the number of lines / listcontrol
	auto b_forced_update = FALSE;
	if (inb_visible != datalistctrlrowobject_prt_array.GetSize())
	{
		// if cache size increases, erase old information (set flag)
		if (inb_visible > datalistctrlrowobject_prt_array.GetSize())
			b_forced_update = TRUE;
		// if cache size decreases, just delete extra rows
		ResizePtrArray(inb_visible);
	}

	// get data file pointer and pointer to database
	auto p_dbwave_doc = ((CViewdbWave*) GetParent())->GetDocument();
	if (p_dbwave_doc == nullptr)
		return;
	const int indexcurrentfile = p_dbwave_doc->DBGetCurrentRecordPosition();	// save current file

	// which update is necessary?
	// conditions for out of range (renew all items)
	auto idelta = 0;							// flag (copy forwards or backwards)
	auto n_to_transfer = 0;					// number of items to copy
	auto n_to_rebuild = datalistctrlrowobject_prt_array.GetSize();	// number of items to refresh
	auto idest1 = 0;							// index first item to be exchg with source
	auto isource1= 0;						// index first source item
	auto inew1 = 0;
	const auto difference = ifirst - ifirst_array;

	// change indexes according to case
	// scroll up (go forwards i.e. towards indexes of higher value)
	if (!b_forced_update)
	{
		if (difference > 0 && difference < datalistctrlrowobject_prt_array.GetSize())
		{
			idelta = 1;							// copy forward
			n_to_transfer = datalistctrlrowobject_prt_array.GetSize() - difference;
			n_to_rebuild -= n_to_transfer;
			isource1 = difference;;
			idest1 = 0;
			inew1 = n_to_transfer;
		}
		// scroll down (go backwards i.e. towards indexes of lower value)
		else if (difference < 0 && -difference < datalistctrlrowobject_prt_array.GetSize())
		{
			idelta = -1;
			n_to_transfer = datalistctrlrowobject_prt_array.GetSize() + difference;
			n_to_rebuild -= n_to_transfer;
			isource1 = n_to_transfer-1;
			idest1 = datalistctrlrowobject_prt_array.GetSize() -1;
			inew1 = 0;
		}

		// else: scroll out of range
	
		//ntoTransfer = 0; ////////////////////////////////////////
		auto isource = isource1;
		auto idest = idest1;
		while (n_to_transfer > 0)
		{
			// exchange objects 
			auto* p_source = datalistctrlrowobject_prt_array.GetAt(isource);
			const auto p_dest =  datalistctrlrowobject_prt_array.GetAt(idest);
			datalistctrlrowobject_prt_array.SetAt(idest, p_source);
			datalistctrlrowobject_prt_array.SetAt(isource, p_dest);
			m_imagelist.Copy(idest, isource, ILCF_SWAP);

			// update indexes
			isource += idelta;
			idest	+= idelta;
			n_to_transfer--;
		}
	}

	// set file and update filenames
	// the following lines are fed with new data (data are obsolete)
	//ntoRebuild = m_nbptrItems; //////////////////////////////
	//inew1 = 0; //////////////////////////////////////////////
	int index = inew1;

	// left, top, right, bottom
	SetEmptyBitmap();
	DB_ITEMDESC desc;

	while (n_to_rebuild > 0)	
	{
		// get data; create object if null
		auto ptr = datalistctrlrowobject_prt_array.GetAt(index);

		// create lineview and spike superposition
		ptr->index = index+ifirst;
		p_dbwave_doc->DBSetCurrentRecordPosition(ptr->index);		// select the other file
		ptr->csDatafileName = p_dbwave_doc->DBGetCurrentDatFileName(TRUE);
		ptr->csSpikefileName = p_dbwave_doc->DBGetCurrentSpkFileName(TRUE);
		auto pDB = p_dbwave_doc->m_pDB;
		pDB->GetRecordItemValue(CH_IDINSECT, &desc);
		ptr->insectID = desc.lVal;

		// column: stim, conc, type = load indirect data from database
		pDB->GetRecordItemValue(CH_STIM_ID, &desc);
		ptr->csStim1 = desc.csVal; 
		pDB->GetRecordItemValue(CH_CONC_ID, &desc);
		ptr->csConc1 = desc.csVal;
		pDB->GetRecordItemValue(CH_STIM2_ID, &desc);
		ptr->csStim2 = desc.csVal;
		pDB->GetRecordItemValue(CH_CONC2_ID, &desc);
		ptr->csConc2 = desc.csVal;

		pDB->GetRecordItemValue(CH_SENSILLUM_ID, &desc);
		ptr->csSensillumname = desc.csVal;

		pDB->GetRecordItemValue(CH_FLAG, &desc);
		ptr->csFlag.Format(_T("%i"), desc.lVal);

		// colum: number of spike = verify that spike file is defined, if yes, load nb spikes
		if (p_dbwave_doc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ptr->csNspk.Empty();
		else
		{
			pDB->GetRecordItemValue(CH_NSPIKES, &desc);
			const int nspikes = desc.lVal;
			pDB->GetRecordItemValue(CH_NSPIKECLASSES, &desc);
			ptr->csNspk.Format(_T("%i (%i classes)"), nspikes, desc.lVal);
		}

		// build bitmap corresponding to data/spikes/nothing
		switch (m_displaymode)
		{
		// data mode
		case 1:
			DisplayDataWnd(ptr, index);
			break;
		// spike bars
		case 2:
			DisplaySpikeWnd(ptr, index);
			break;
		default:
			DisplayEmptyWnd(ptr, index);
			break;
		}
		index++;
		n_to_rebuild --;
	}

	// restore document conditions
	if (indexcurrentfile >= 0)
		p_dbwave_doc->DBSetCurrentRecordPosition(indexcurrentfile);
}

///////////////////////////////////////////////////////////////////////////////
void CDataListCtrl::SetEmptyBitmap(const BOOL b_forced_update)
{
	if (m_pEmptyBitmap != nullptr && !b_forced_update)
		return;

	SAFE_DELETE(m_pEmptyBitmap);
	m_pEmptyBitmap = new CBitmap;

	CWindowDC dc(this);
	CDC mem_dc; 
	VERIFY(mem_dc.CreateCompatibleDC(&dc));
	m_pEmptyBitmap->CreateBitmap(m_cx, m_cy,  dc.GetDeviceCaps(PLANES), dc.GetDeviceCaps(BITSPIXEL), nullptr);
	mem_dc.SelectObject(m_pEmptyBitmap);
	mem_dc.SetMapMode(dc.GetMapMode());

	CBrush brush(RGB(204, 204, 204));		//light gray 
	mem_dc.SelectObject(&brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // black
	mem_dc.SelectObject(&pen);
	auto rect_data   = CRect(1, 0, m_cx, m_cy); 
	mem_dc.Rectangle(&rect_data);
}


void CDataListCtrl::RefreshDisplay()
{
	if (datalistctrlrowobject_prt_array.GetSize() == NULL)
		return;
	const int ifirst_array =  datalistctrlrowobject_prt_array.GetAt(0)->index;
	const int ilast_array = datalistctrlrowobject_prt_array.GetAt(datalistctrlrowobject_prt_array.GetUpperBound())->index;

	// left, top, right, bottom
	SetEmptyBitmap();

	const auto nrows = datalistctrlrowobject_prt_array.GetSize();
	for (auto index = 0; index < nrows; index++)
	{
		auto* ptr = datalistctrlrowobject_prt_array.GetAt(index);
		if (ptr == nullptr)
			continue;
		switch (m_displaymode)
		{
		case 1:
			DisplayDataWnd(ptr, index);
			break;
		case 2:
			DisplaySpikeWnd(ptr, index);
			break;
		default:
			DisplayEmptyWnd(ptr, index);
			break;
		}
	}
	RedrawItems(ifirst_array, ilast_array);		// CCtrlList standard function
	Invalidate();
	UpdateWindow();
}


void CDataListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	switch (nSBCode)
	{
	case SB_LINEUP:
		((CDaoRecordView*)GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case SB_LINEDOWN:
		((CDaoRecordView*)GetParent())->OnMove(ID_RECORD_NEXT);
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
	case VK_PRIOR:	// page up
		SendMessage(WM_VSCROLL, SB_PAGEUP, NULL);
		break;
	case VK_NEXT: // page down
		SendMessage(WM_VSCROLL, SB_PAGEDOWN, NULL);
		break;
	case VK_UP:
		((CDaoRecordView*)GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case VK_DOWN:
		((CDaoRecordView*)GetParent())->OnMove(ID_RECORD_NEXT);
		break;

	default:
		CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
		break;
	}
}

void CDataListCtrl::DisplayDataWnd (CDataListCtrlRowObject* ptr, int iImage)
{
	// create objects if necessary : CLineView and CAcqDataDoc
	if (ptr->pdataWnd == nullptr)
	{
		ptr->pdataWnd = new CLineViewWnd;
		ASSERT(ptr->pdataWnd != NULL);
		ptr->pdataWnd->Create(_T("DATAWND"), WS_CHILD, CRect(0, 0, m_cx, m_cy), this, iImage*100);
		ptr->pdataWnd->SetbUseDIB(FALSE); //(TRUE );
	}
	CLineViewWnd* pWnd = ptr->pdataWnd;

	// get data file descriptor
	pWnd->SetString(ptr->cs_comment);

	// open data document
	if (ptr->pdataDoc == nullptr)
	{
		ptr->pdataDoc = new CAcqDataDoc;
		ASSERT(ptr->pdataDoc != NULL);
	}
	if (!ptr->pdataDoc->OnOpenDocument(ptr->csDatafileName))
	// if not available: consider here that requested document is not reachable
	// tell it to lineview and display "data not available"...
	{
		pWnd->RemoveAllChanlistItems();
		auto comment = _T("File name: ") + ptr->csDatafileName;
		comment += _T(" -- data not available");
		pWnd->SetString(comment);
	}
	// if available, load data into CLineViewWnd object
	else
	{
		//if (!TestIfSpikesWereDetected(ptr->csDatafileName))
		if (ptr->csNspk.IsEmpty())
			pWnd->m_parms.crScopeFill = pWnd->GetColor(2);
		else 
			pWnd->m_parms.crScopeFill = pWnd->GetColor(15);

		ptr->pdataDoc->ReadDataInfos();
		pWnd->AttachDataFile(ptr->pdataDoc, 0);

		// display all channels : loop through all doc channels & add if necessary
		CWaveFormat* pwaveFormat = ptr->pdataDoc->GetpWaveFormat();
		ptr->cs_comment = pwaveFormat->GetComments(_T(" "));
		const int ndocchans = pwaveFormat->scan_count;
		auto lnvchans = pWnd->GetChanlistSize();
		// add channels if value is zero 
		// channels were all removed if file was not found in an earlier round
		for (auto jdocchan = 0; jdocchan < ndocchans; jdocchan++)
		{										// check if present in the list
			auto b_present=FALSE;				// pessimistic
			for (auto j = lnvchans-1; j>= 0; j--)// check all channels / display list
			{									// test if this data chan is present + no transf
				if (pWnd->GetChanlistSourceChan(j) == jdocchan)			  
				{
					b_present = TRUE;			// the wanted chan is present: stop loopint through disp list
					pWnd->SetChanlistTransformMode(j, m_dattransform);
					break;						// and examine next doc channel
				}
			}
			if (b_present == FALSE)				// no display chan contains that doc chan
			{
				pWnd->AddChanlistItem(jdocchan, m_dattransform);	// add: chan, transform
				lnvchans++;
			}
			pWnd->SetChanlistColor(jdocchan, jdocchan);
		}

		// load real data from file and update time parameters
		const auto npixels = pWnd->Width();
		const int doc_chan_len = ptr->pdataDoc->GetDOCchanLength();
		long l_first = 0;
		long l_last = doc_chan_len-1;
		if (m_bsetTimeSpan)
		{
			const auto p_wave_format = ptr->pdataDoc->GetpWaveFormat();
			const auto samplingrate = p_wave_format->chrate;
			l_first = static_cast<long>(m_tFirst * samplingrate);
			l_last = static_cast<long>(m_tLast * samplingrate);
			if (l_last == l_first)
				l_last++;
		}
		pWnd->ResizeChannels(npixels, l_last - l_first +1);
		pWnd->GetDataFromDoc(l_first, l_last);

		// maximize gain & split curves
		auto j = lnvchans-1;	
		int max, min;
		for (auto i=j; i>= 0; i--)
		{
			pWnd->GetChanlistMaxMin(i, &max, &min);
			auto ispan = max-min+1;
			int iextent;// = pWnd->GetChanlistYextent(i);
			int izero; // = pWnd->GetChanlistYzero(i);
			if(m_bsetmVSpan > 0.f)
			{
				ispan = pWnd->ConvertChanlistVoltstoDataBins(i, m_mVspan / 1000.f) ;
				izero = pWnd->GetChanlistBinZero(i);
				iextent = ispan;
			}
			else
			{
				// split curves
				iextent = MulDiv(ispan, 11*lnvchans, 10);
				izero = (max+min)/2 - MulDiv(iextent, j, lnvchans*2);
			}
			j -= 2;
			pWnd->SetChanlistYextent(i, iextent);
			pWnd->SetChanlistYzero(i, izero);
		}
	}

	// plot data
	pWnd->SetBottomComment(m_bDisplayFileName, ptr->csDatafileName);
	CRect client_rect;
	pWnd->GetClientRect(&client_rect);

	CBitmap bitmap_plot;
	const auto p_dc = pWnd->GetDC();
	CDC mem_dc;
	VERIFY(mem_dc.CreateCompatibleDC(p_dc));
	bitmap_plot.CreateBitmap(client_rect.right, client_rect.bottom, p_dc->GetDeviceCaps(PLANES), p_dc->GetDeviceCaps(BITSPIXEL), nullptr);
	mem_dc.SelectObject(&bitmap_plot);
	mem_dc.SetMapMode(p_dc->GetMapMode());

	pWnd->PlotDatatoDC(&mem_dc);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
	mem_dc.MoveTo(1, 0);
	mem_dc.LineTo(1, client_rect.bottom);
	m_imagelist.Replace(iImage, &bitmap_plot, nullptr);
}


void CDataListCtrl::DisplaySpikeWnd (CDataListCtrlRowObject* ptr, int iImage)
{
	// create spike window and spike document if necessary
	if (ptr->pspikeWnd == nullptr)
	{
		ptr->pspikeWnd = new CSpikeBarWnd;
		ASSERT(ptr->pspikeWnd != NULL);
		ptr->pspikeWnd->Create(_T("SPKWND"), WS_CHILD, CRect(0, 0, m_cx, m_cy), this, ptr->index*1000);
		ptr->pspikeWnd->SetbUseDIB(FALSE); //(TRUE);
	}

	auto p_wnd = ptr->pspikeWnd;
	const auto pdb_doc = ((CViewdbWave*)GetParent())->GetDocument();

	// open spike document
	if (ptr->pspikeDoc == nullptr)
	{
		ptr->pspikeDoc = new CSpikeDoc;
		ASSERT(ptr->pspikeDoc != NULL);
	}
	auto cs_spike_name (ptr->csSpikefileName);
	const LPCTSTR pcs_spike_name = cs_spike_name;
	if (cs_spike_name.IsEmpty() || !ptr->pspikeDoc->OnOpenDocument(pcs_spike_name))
	// consider here that requested,document is not reachable
	// tell it to spikeview and display "data not available"...
	{
		m_imagelist.Replace(iImage, m_pEmptyBitmap, nullptr);
	}
	else
	{
		const auto pspk_list = ptr->pspikeDoc->SetSpkListCurrent(pdb_doc->GetcurrentSpkListIndex());
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
		if(m_bsetmVSpan > 0.f)
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
		bitmap_plot.CreateBitmap(client_rect.right, client_rect.bottom, p_dc->GetDeviceCaps(PLANES), p_dc->GetDeviceCaps(BITSPIXEL), nullptr);
		mem_dc.SelectObject(&bitmap_plot);
		mem_dc.SetMapMode(p_dc->GetMapMode());

		p_wnd->PlotDatatoDC(&mem_dc);
		CPen pen;
		pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
		mem_dc.MoveTo(1, 0);
		mem_dc.LineTo(1, client_rect.bottom);
		m_imagelist.Replace(iImage, &bitmap_plot, nullptr);
	}
}


void CDataListCtrl::DisplayEmptyWnd (CDataListCtrlRowObject* ptr, int iImage)
{
	m_imagelist.Replace(iImage, m_pEmptyBitmap, nullptr);
}


void CDataListCtrl::ResizeSignalColumn(int npixels)
{
	m_icolwidth[COL_CURVE] = npixels;
	m_imagelist.DeleteImageList();
	m_cx = m_icolwidth[COL_CURVE];
	m_imagelist.Create( m_cx, m_cy, /*ILC_COLORDDB*/ ILC_COLOR4, 10, 10 ); 
	SetImageList( &m_imagelist, LVSIL_SMALL );
	m_imagelist.SetImageCount(datalistctrlrowobject_prt_array.GetSize());

	for (int i=0; i< datalistctrlrowobject_prt_array.GetSize(); i++)
	{
		auto* ptr = datalistctrlrowobject_prt_array.GetAt(i);
		SAFE_DELETE(ptr->pdataWnd);
		SAFE_DELETE(ptr->pspikeWnd);
	}
	RefreshDisplay();
}

void CDataListCtrl::FitColumnsToSize(int npixels)
{
	// compute size of fixed columns
	auto fixedsize = 0;
	for (auto i : m_icolwidth)
	{
		fixedsize += i;
	}
	fixedsize -= m_icolwidth[COL_CURVE];
	const auto signalcolsize = npixels - fixedsize;
	if (signalcolsize != m_icolwidth[COL_CURVE] && signalcolsize > 2) 
	{
		SetColumnWidth(COL_CURVE, signalcolsize);
		ResizeSignalColumn(signalcolsize);
	}
}
