// DataListCtrl.cpp : implementation file
//

#include "stdafx.h"

#include "DataListCtrl.h"

#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "spikedoc.h"
#include "scopescr.h"
#include "spikeshape.h"
#include "Envelope.h" 
#include "chanlistitem.h"
#include "lineview.h"
#include "spikebar.h"
#include "ViewdbWave.h"

#include "DataListCtrlRowObject.h"
#include ".\datalistctrl.h"

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
	m_picolwidth = NULL;
	m_displaymode = 1;
	m_pEmptyBitmap = NULL;
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
	if (m_picolwidth != NULL)
	{
		int nbcols_stored = m_picolwidth->GetSize();
		if (nbcols_stored != NCOLS)
			m_picolwidth->SetSize(NCOLS);
		for (int i=0; i< NCOLS; i++)
			m_picolwidth->SetAt(i, GetColumnWidth(i));
	}
	SAFE_DELETE(m_pEmptyBitmap);
	CListCtrl::OnDestroy();
}

// delete objects then the array

void CDataListCtrl::DeletePtrArray()
{
	if (m_ptrArray.GetSize() == NULL)
		return;
	int imax = m_ptrArray.GetSize();
	for (int i=0; i< imax; i++)
	{
		CDataListCtrlRowObject* ptr = (CDataListCtrlRowObject*) m_ptrArray.GetAt(i);
		SAFE_DELETE(ptr);
	}
	m_ptrArray.RemoveAll();
}

// change size of the array
// delete items or add items accordingly

void CDataListCtrl::ResizePtrArray(int nitems)
{
	if (nitems == m_ptrArray.GetSize())
		return;

	// Resize m_imagelist CImageList
	m_imagelist.SetImageCount(nitems);

	// reduce size
	if (m_ptrArray.GetSize() > nitems)
	{
		for (int i=m_ptrArray.GetSize()-1; i>= nitems; i--)
			delete (CDataListCtrlRowObject*) m_ptrArray.GetAt(i);
		m_ptrArray.SetSize(nitems);
	}
	// grow size
	else
	{
		int lowernew = m_ptrArray.GetSize();
		m_ptrArray.SetSize(nitems);
		int index = 0;
		if (lowernew > 0)
			index = ((CDataListCtrlRowObject*) m_ptrArray.GetAt(lowernew-1))->index;
		for (int i=lowernew; i< nitems; i++)
		{
			CDataListCtrlRowObject* ptr = new CDataListCtrlRowObject;
			ASSERT(ptr != NULL);
			m_ptrArray.SetAt(i, ptr);
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
	if (picolwidth != NULL)
	{
		m_picolwidth = picolwidth; 
		int ncol_stored = picolwidth->GetSize();
		if (ncol_stored < NCOLS)
			picolwidth->SetSize(NCOLS);
		for (int i=0; i< ncol_stored; i++)
			m_icolwidth[i] = picolwidth->GetAt(i);
	}

	LVCOLUMN lvcol;
	UINT mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	lvcol.mask = mask;
	int iOrder;
/*
valid members: LVCF_FMT - fmt | LVCF_SUBITEM - iSubItem | LVCF_TEXT - pszText | LVCF_WIDTH - cx | LVCF_ORDER - iOrder 
LVCF_IMAGE		Version 4.70. The iImage member is valid. 
*/
	iOrder = 0;		// trick to allow displaying rec
	lvcol.pszText = _T("");
	lvcol.fmt = LVCFMT_LEFT;
	lvcol.iOrder = iOrder;
	lvcol.cx = 1;
	InsertColumn(iOrder, &lvcol);
	
	ASSERT(iOrder == 0);		// make sure the first item is 0

	iOrder = COL_INDEX;			// 0
	lvcol.pszText = _T("#");
	lvcol.fmt = LVCFMT_CENTER;
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);

	iOrder = COL_CURVE;			// 1
	lvcol.pszText = _T("data");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);

	iOrder = COL_INSECT;			// 2
	lvcol.pszText = _T("insect ID");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);

	// -----------------------------

	iOrder++;			// 3
	lvcol.pszText = _T("sensillum");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);

	iOrder++;			// 4
	lvcol.pszText = _T("stim1");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);
	
	iOrder++;			// 5
	lvcol.pszText = _T("conc1");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);

	iOrder++;			// 6
	lvcol.pszText = _T("stim2");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);
	
	iOrder++;			// 7
	lvcol.pszText = _T("conc2");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);

	iOrder++;			// 8
	lvcol.pszText = _T("spikes");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);

	iOrder++;			// 9
	lvcol.pszText = _T("flag");
	lvcol.iOrder = iOrder;
	lvcol.cx = m_icolwidth[iOrder];
	InsertColumn(iOrder, &lvcol);

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
	int ifirst_array = 0;
	int ilast_array = 0;
	if (m_ptrArray.GetSize() > 0)
	{
		ifirst_array = ((CDataListCtrlRowObject*) m_ptrArray.GetAt(0))->index;
		ilast_array = ((CDataListCtrlRowObject*) m_ptrArray.GetAt(m_ptrArray.GetUpperBound()))->index;
	}

	// is item within the cache?
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;
	*pResult = 0;
	int iItemIndex = pItem->iItem;

	// item before first visible item? selected item becomes first (scroll up)
	if (iItemIndex < ifirst_array)
	{
		ifirst_array = iItemIndex;
		ilast_array = ifirst_array + GetCountPerPage() -1;
		UpdateCache(ifirst_array, ilast_array);
	}
	// item after last visible item? iItem becomes last visible (scroll down)
	else if (iItemIndex > ilast_array)
	{
		ilast_array = iItemIndex;
		ifirst_array = ilast_array - GetCountPerPage() +1;
		UpdateCache(ifirst_array, ilast_array);
	}
	else if (m_ptrArray.GetSize() == 0)
		UpdateCache(ifirst_array, ilast_array);
	
	// now, the requested item is in the cache
	// get data from database
	CdbWaveDoc* pdbDoc = ((CdbWaveView*) GetParent())->GetDocument();
	if (pdbDoc == NULL)
		return;

	int ifirstvisible = ((CDataListCtrlRowObject*)  m_ptrArray.GetAt(0))->index;
	int icacheIndex = iItemIndex - ifirstvisible;
	if (icacheIndex > (m_ptrArray.GetSize() -1))
		icacheIndex = m_ptrArray.GetSize() -1;
	
	CDataListCtrlRowObject* ptr = (CDataListCtrlRowObject*)  m_ptrArray.GetAt(icacheIndex);

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		CString cs;
		BOOL flag = TRUE;
		switch(pItem->iSubItem)
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
			lstrcpy(pItem->pszText, cs);
	}

	// display images
	if (pItem->mask & LVIF_IMAGE
		&& pItem->iSubItem == COL_CURVE)
		pItem->iImage = icacheIndex;
}


void CDataListCtrl::SetCurSel(int recposition)
{
	// get current item which has the focus
	int flag = LVNI_FOCUSED | LVNI_ALL;
	int curpos = GetNextItem(-1, flag );
	
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
	int ifirst_array = 0;
	int ilast_array = 0;
	if (m_ptrArray.GetSize() > 0)
	{
		ifirst_array = ((CDataListCtrlRowObject*) m_ptrArray.GetAt(0))->index;
		ilast_array = ((CDataListCtrlRowObject*) m_ptrArray.GetAt(m_ptrArray.GetUpperBound()))->index;
	}

	// adjust number of items in the array and adjust ifirst and ilast
	int inb_visible = ilast - ifirst +1;
	
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
	BOOL bForcedUpdate = FALSE;
	if (inb_visible != m_ptrArray.GetSize())
	{
		// if cache size increases, erase old information (set flag)
		if (inb_visible > m_ptrArray.GetSize())
			bForcedUpdate = TRUE;
		// if cache size decreases, just delete extra rows
		ResizePtrArray(inb_visible);
	}

	// get data file pointer and pointer to database
	CdbWaveDoc* pdbDoc = ((CdbWaveView*) GetParent())->GetDocument();
	if (pdbDoc == NULL)
		return;
	int indexcurrentfile = pdbDoc->DBGetCurrentRecordPosition();	// save current file

	// which update is necessary?
	// conditions for out of range (renew all items)
	int idelta = 0;							// flag (copy forwards or backwards)
	int ntoTransfer = 0;					// number of items to copy
	int ntoRebuild = m_ptrArray.GetSize();	// number of items to refresh
	int idest1 = 0;							// index first item to be exchg with source
	int isource1= 0;						// index first source item
	int	inew1 = 0;
	int inew2 = ntoRebuild-1;				// index last item to renew
	int difference = ifirst - ifirst_array;

	// change indexes according to case
	// scroll up (go forwards i.e. towards indexes of higher value)
	if (!bForcedUpdate)
	{
		if (difference > 0 && difference < m_ptrArray.GetSize())
		{
			idelta = 1;							// copy forward
			ntoTransfer = m_ptrArray.GetSize() - difference;
			ntoRebuild -= ntoTransfer;
			isource1 = difference;;
			idest1 = 0;
			inew1 = ntoTransfer;
		}
		// scroll down (go backwards i.e. towards indexes of lower value)
		else if (difference < 0 && -difference < m_ptrArray.GetSize())
		{
			idelta = -1;
			ntoTransfer = m_ptrArray.GetSize() + difference;
			ntoRebuild -= ntoTransfer;
			isource1 = ntoTransfer-1;
			idest1 = m_ptrArray.GetSize() -1;
			inew1 = 0;
		}

		// else: scroll out of range
	
		//ntoTransfer = 0; ////////////////////////////////////////
		int isource = isource1;
		int idest = idest1;
		while (ntoTransfer > 0)
		{
			// exchange objects 
			CDataListCtrlRowObject* pSource = (CDataListCtrlRowObject*) m_ptrArray.GetAt(isource);
			CDataListCtrlRowObject* pDest = (CDataListCtrlRowObject*) m_ptrArray.GetAt(idest);
			m_ptrArray.SetAt(idest, pSource);
			m_ptrArray.SetAt(isource, pDest);
			m_imagelist.Copy(idest, isource, ILCF_SWAP);

			// update indexes
			isource += idelta;
			idest	+= idelta;
			ntoTransfer--;
		}
	}

	// set file and update filenames
	// the following lines are fed with new data (data are obsolete)
	//ntoRebuild = m_nbptrItems; //////////////////////////////
	//inew1 = 0; //////////////////////////////////////////////
	int index = inew1;

	// left, top, right, bottom
	SetEmptyBitmap();
	CDataListCtrlRowObject* ptr;
	DB_ITEMDESC desc;

	while (ntoRebuild > 0)	
	{
		// get data; create object if null
		ptr = (CDataListCtrlRowObject*) m_ptrArray.GetAt(index);

		// create lineview and spike superposition
		ptr->index = index+ifirst;
		pdbDoc->DBSetCurrentRecordPosition(ptr->index);		// select the other file
		ptr->csDatafileName = pdbDoc->DBGetCurrentDatFileName(TRUE);
		ptr->csSpikefileName = pdbDoc->DBGetCurrentSpkFileName(TRUE);
		CdbWdatabase*	pDB = pdbDoc->m_pDB;
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
		if (pdbDoc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ptr->csNspk.Empty();
		else
		{
			pDB->GetRecordItemValue(CH_NSPIKES, &desc);
			int nspikes = desc.lVal;
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
		ntoRebuild --;
	}

	// restore document conditions
	if (indexcurrentfile >= 0)
		pdbDoc->DBSetCurrentRecordPosition(indexcurrentfile);
}

///////////////////////////////////////////////////////////////////////////////
void CDataListCtrl::SetEmptyBitmap(BOOL bForcedUpdate)
{
	if (m_pEmptyBitmap != NULL && !bForcedUpdate)
		return;

	SAFE_DELETE(m_pEmptyBitmap);
	m_pEmptyBitmap = new CBitmap;

	CWindowDC dc(this);
	CDC memDC; 
	VERIFY(memDC.CreateCompatibleDC(&dc));
	m_pEmptyBitmap->CreateBitmap(m_cx, m_cy,  dc.GetDeviceCaps(PLANES), dc.GetDeviceCaps(BITSPIXEL), NULL);
	CBitmap *pBmp = memDC.SelectObject(m_pEmptyBitmap);
	memDC.SetMapMode(dc.GetMapMode());

	CBrush brush(RGB(204, 204, 204));		//light gray 
	memDC.SelectObject(&brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // black
	memDC.SelectObject(&pen);
	CRect rectData   = CRect(1, 0, m_cx, m_cy); 
	memDC.Rectangle(&rectData);
}


void CDataListCtrl::RefreshDisplay()
{
	if (m_ptrArray.GetSize() == NULL)
		return;
	int ifirst_array = ((CDataListCtrlRowObject*) m_ptrArray.GetAt(0))->index;
	int ilast_array = ((CDataListCtrlRowObject*) m_ptrArray.GetAt(m_ptrArray.GetUpperBound()))->index;

	// left, top, right, bottom
	SetEmptyBitmap();

	int nrows = m_ptrArray.GetSize();
	for (int index = 0; index < nrows; index++)
	{
		CDataListCtrlRowObject* ptr = (CDataListCtrlRowObject*) m_ptrArray.GetAt(index);
		if (ptr == NULL)
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
		((CDaoRecordView*) GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case SB_LINEDOWN:
		((CDaoRecordView*) GetParent())->OnMove(ID_RECORD_NEXT);
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
		((CDaoRecordView*) GetParent())->OnMove(ID_RECORD_PREV);
		break;
	case VK_DOWN:
		((CDaoRecordView*) GetParent())->OnMove(ID_RECORD_NEXT);
		break;

	default:
		CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
		break;
	}
}


void CDataListCtrl::DisplayDataWnd (CDataListCtrlRowObject* ptr, int iImage)
{
	// create objects if necessary : CLineView and CAcqDataDoc
	if (ptr->pdataWnd == NULL)
	{
		ptr->pdataWnd = new CLineViewWnd;
		ASSERT(ptr->pdataWnd != NULL);
		ptr->pdataWnd->Create(_T("DATAWND"), WS_CHILD, CRect(0, 0, m_cx, m_cy), this, iImage*100);
		ptr->pdataWnd->SetbUseDIB(FALSE); //(TRUE );
	}
	CLineViewWnd* pWnd = ptr->pdataWnd;

	// get data file descriptor
	pWnd->SetString(ptr->csComment);

	// open data document
	if (ptr->pdataDoc == NULL)
	{
		ptr->pdataDoc = new CAcqDataDoc;
		ASSERT(ptr->pdataDoc != NULL);
	}
	if (!ptr->pdataDoc->OnOpenDocument(ptr->csDatafileName))
	// if not available: consider here that requested document is not reachable
	// tell it to lineview and display "data not available"...
	{
		pWnd->RemoveAllChanlistItems();
		CString comment = _T("File name: ") + ptr->csDatafileName;
		comment += _T(" -- data not available");
		pWnd->SetString(comment);
	}
	// if available, load data into CLineViewWnd object
	else
	{
		ptr->pdataDoc->ReadDataInfos();
		pWnd->AttachDataFile(ptr->pdataDoc, 0);

		// display all channels : loop through all doc channels & add if necessary
		CWaveFormat* pwaveFormat = ptr->pdataDoc->GetpWaveFormat();
		ptr->csComment = pwaveFormat->GetComments(_T(" "));
		int ndocchans = pwaveFormat->scan_count;
		int lnvchans = pWnd->GetChanlistSize();
		// add channels if value is zero 
		// channels were all removed if file was not found in an earlier round
		for (int jdocchan = 0; jdocchan < ndocchans; jdocchan++)
		{										// check if present in the list
			BOOL bPresent=FALSE;				// pessimistic
			for (int j = lnvchans-1; j>= 0; j--)// check all channels / display list
			{									// test if this data chan is present + no transf
				if (pWnd->GetChanlistSourceChan(j) == jdocchan)			  
				{
					bPresent = TRUE;			// the wanted chan is present: stop loopint through disp list
					pWnd->SetChanlistTransformMode(j, m_dattransform);
					break;						// and examine next doc channel
				}
			}
			if (bPresent == FALSE)				// no display chan contains that doc chan
			{
				pWnd->AddChanlistItem(jdocchan, m_dattransform);	// add: chan, transform
				lnvchans++;
			}
			pWnd->SetChanlistColor(jdocchan, jdocchan);
		}

		// load real data from file and update time parameters
		int npixels = pWnd->Width();
		int docChanLen = ptr->pdataDoc->GetDOCchanLength();
		long lFirst = 0;
		long lLast = docChanLen-1;
		if (m_bsetTimeSpan)
		{
			CWaveFormat* pwaveFormat = ptr->pdataDoc->GetpWaveFormat();
			float samplingrate = pwaveFormat->chrate;
			lFirst = (long) (m_tFirst * samplingrate);
			lLast = (long) (m_tLast * samplingrate);
			if (lLast == lFirst)
				lLast++;
		}
		pWnd->ResizeChannels(npixels, lLast - lFirst +1);
		pWnd->GetDataFromDoc(lFirst, lLast);

		// maximize gain & split curves
		int j = lnvchans-1;	
		int max, min;
		for (int i=j; i>= 0; i--)
		{
			pWnd->GetChanlistMaxMin(i, &max, &min);
			int ispan = max-min+1;
			int iextent = pWnd->GetChanlistYextent(i);
			int izero = pWnd->GetChanlistYzero(i);
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
	CRect clientRect;
	pWnd->GetClientRect(&clientRect);

	CBitmap bitmapPlot;
	CDC* pDC = pWnd->GetDC();
	CDC memDC;
	VERIFY(memDC.CreateCompatibleDC(pDC));
	bitmapPlot.CreateBitmap(clientRect.right, clientRect.bottom, pDC->GetDeviceCaps(PLANES), pDC->GetDeviceCaps(BITSPIXEL), NULL);
	CBitmap *pBmp = memDC.SelectObject(&bitmapPlot);
	memDC.SetMapMode(pDC->GetMapMode());

	pWnd->PlotDatatoDC(&memDC);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
	memDC.MoveTo(1, 0);
	memDC.LineTo(1, clientRect.bottom);
	m_imagelist.Replace(iImage, &bitmapPlot, NULL);
}


void CDataListCtrl::DisplaySpikeWnd (CDataListCtrlRowObject* ptr, int iImage)
{
	// create spike window and spike document if necessary
	if (ptr->pspikeWnd == NULL)
	{
		ptr->pspikeWnd = new CSpikeBarWnd;
		ASSERT(ptr->pspikeWnd != NULL);
		ptr->pspikeWnd->Create(_T("SPKWND"), WS_CHILD, CRect(0, 0, m_cx, m_cy), this, ptr->index*1000);
		ptr->pspikeWnd->SetbUseDIB(FALSE); //(TRUE);
	}

	CSpikeBarWnd* pWnd = ptr->pspikeWnd;
	CdbWaveDoc* pdbDoc = ((CdbWaveView*) GetParent())->GetDocument();

	// open spike document
	if (ptr->pspikeDoc == NULL)
	{
		ptr->pspikeDoc = new CSpikeDoc;
		ASSERT(ptr->pspikeDoc != NULL);
	}
	CString csSpikeName (ptr->csSpikefileName);
	LPCTSTR pcsSpikeName = csSpikeName;
	if (!ptr->pspikeDoc->OnOpenDocument(pcsSpikeName))
	// consider here that requested,document is not reachable
	// tell it to spikeview and display "data not available"...
	{
		m_imagelist.Replace(iImage, m_pEmptyBitmap, NULL);
	}
	else
	{
		CSpikeList* pspkList = ptr->pspikeDoc->SetSpkListCurrent(pdbDoc->GetcurrentSpkListIndex());
		pWnd->SetSourceData(pspkList, ptr->pspikeDoc);
		pWnd->SetPlotMode(m_spikeplotmode, m_selclass);
		long lFirst = 0;
		long lLast = ptr->pspikeDoc->GetAcqSize();
		if (m_bsetTimeSpan)
		{
			float samplingrate = ptr->pspikeDoc->GetAcqRate();
			lFirst = (long) (m_tFirst * samplingrate);
			lLast = (long) (m_tLast * samplingrate);
		}

		pWnd->SetTimeIntervals(lFirst, lLast);
		int yWE = 0;
		int yWO = 0; 
		if(m_bsetmVSpan > 0.f)
		{
			float vperbin = pspkList->GetAcqVoltsperBin();
			yWE = (int) (m_mVspan / 1000.f / vperbin);
			yWO = pspkList->GetAcqBinzero();
			pWnd->SetYWExtOrg(yWE, yWO);
		}
		pWnd->SetBottomComment(m_bDisplayFileName, ptr->csSpikefileName);

		CRect clientRect;
		pWnd->GetClientRect(&clientRect);

		CBitmap bitmapPlot;
		CDC* pDC = pWnd->GetDC();
		CDC memDC;
		VERIFY(memDC.CreateCompatibleDC(pDC));
		bitmapPlot.CreateBitmap(clientRect.right, clientRect.bottom, pDC->GetDeviceCaps(PLANES), pDC->GetDeviceCaps(BITSPIXEL), NULL);
		CBitmap *pBmp = memDC.SelectObject(&bitmapPlot);
		memDC.SetMapMode(pDC->GetMapMode());

		pWnd->PlotDatatoDC(&memDC);
		CPen pen;
		pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // black//RGB(0, 0, 0)); // black
		memDC.MoveTo(1, 0);
		memDC.LineTo(1, clientRect.bottom);
		m_imagelist.Replace(iImage, &bitmapPlot, NULL);
	}
}


void CDataListCtrl::DisplayEmptyWnd (CDataListCtrlRowObject* ptr, int iImage)
{
	m_imagelist.Replace(iImage, m_pEmptyBitmap, NULL);
}


void CDataListCtrl::ResizeSignalColumn(int npixels)
{
	m_icolwidth[COL_CURVE] = npixels;
	m_imagelist.DeleteImageList();
	m_cx = m_icolwidth[COL_CURVE];
	m_imagelist.Create( m_cx, m_cy, /*ILC_COLORDDB*/ ILC_COLOR4, 10, 10 ); 
	SetImageList( &m_imagelist, LVSIL_SMALL );
	m_imagelist.SetImageCount(m_ptrArray.GetSize());

	for (int i=0; i< m_ptrArray.GetSize(); i++)
	{
		CDataListCtrlRowObject* ptr = (CDataListCtrlRowObject*) m_ptrArray.GetAt(i);
		SAFE_DELETE(ptr->pdataWnd);
		SAFE_DELETE(ptr->pspikeWnd);
	}
	RefreshDisplay();
}

void CDataListCtrl::FitColumnsToSize(int npixels)
{
	// compute size of fixed columns
	int fixedsize = 0;
	for (int i = 0; i < NCOLS; i++) {
		fixedsize += m_icolwidth[i];
	}
	fixedsize -= m_icolwidth[COL_CURVE];
	int signalcolsize = npixels - fixedsize;
	if (signalcolsize != m_icolwidth[COL_CURVE] && signalcolsize > 2) 
	{
		SetColumnWidth(COL_CURVE, signalcolsize);
		ResizeSignalColumn(signalcolsize);
	}
}
