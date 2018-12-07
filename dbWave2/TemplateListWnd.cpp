// TemplateListWnd.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "scopescr.h"
#include "TemplateWnd.h"
#include "TemplateListWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CTemplateListWnd

IMPLEMENT_SERIAL(CTemplateListWnd, CListCtrl, 0 /* schema number*/ )

void CTemplateListWnd::operator = (const CTemplateListWnd& arg)
{	
	DeleteAllTemplates();
	m_tpl0 = arg.m_tpl0;
	for (int i=0; i< arg.m_ptpl.GetSize(); i++)
	{
		CTemplateWnd* pSource = arg.GetTemplateWnd(i);
		InsertTemplate(i, 0);
		CTemplateWnd* pDest = GetTemplateWnd(i);
		*pDest = *pSource;
	}	
}

// ----------------------------------------------------------------------------

void CTemplateListWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	
		WORD wversion = 1;
		ar << wversion;

		int nstrings = 0;
		ar << nstrings;

		int nints = 6;
		ar << nints;		
		ar << m_tpllen;
		ar << m_tpleft;
		ar << m_tpright;
		ar << m_hitrate;
		ar << m_yextent;
		ar << m_yzero;

		int nfloats = 1;
		ar << nfloats;
		ar << m_ktolerance;

		int ndoubles = 2;
		ar << ndoubles;
		ar << m_globalstd;	// 1
		ar << m_globaldist;	// 2		

		// serialize templates
		m_tpl0.Serialize(ar);
		ar << m_ptpl.GetSize();
		if (m_ptpl.GetSize() > 0)
		{
			for (int i=0; i< m_ptpl.GetSize(); i++)
			{
				CTemplateWnd* pS = GetTemplateWnd(i);
				pS->Serialize(ar);
			}
		}
	} 
	else
	{
		WORD version;  ar >> version;
		
		int nstrings = 0;
		ar >> nstrings;

		int nints = 6;
		ar >> nints;		
		ar >> m_tpllen;
		ar >> m_tpleft;
		ar >> m_tpright;
		ar >> m_hitrate;
		ar >> m_yextent;
		ar >> m_yzero;

		int nfloats = 1;
		ar >> nfloats;
		ar >> m_ktolerance;

		int ndoubles = 2;
		ar >> ndoubles;
		ar >> m_globalstd;	// 1
		ar >> m_globaldist;	// 2		

		// serialize templates
		m_tpl0.Serialize(ar);
		int nitems;
		ar >> nitems;
		if (nitems > 0)
		{			
			for (int i=0; i< nitems; i++)
			{				
				InsertTemplate(i, 0);
				CTemplateWnd* pDest = GetTemplateWnd(i);
				pDest->Serialize(ar);
			}
		}
	}
}

// ----------------------------------------------------------------------------

CTemplateListWnd::CTemplateListWnd()
{	
	m_hitrate = 75;
	m_ktolerance = 1.96f;
	m_globalstd = 0.f;
	m_tpllen = 60;
	m_tpleft=0;
	m_tpright = 1;
	m_yextent=0;
	m_yzero=0;
	m_bDragging = FALSE;
	m_pimageListDrag= nullptr;
	m_tpl0.tInit();	
}

// ----------------------------------------------------------------------------

CTemplateListWnd::~CTemplateListWnd()
{
	for (int i=0; i< m_ptpl.GetSize(); i++)
	{
		CTemplateWnd* pS = (CTemplateWnd*) m_ptpl.GetAt(i);
		delete pS;
	}
	m_ptpl.RemoveAll();
}

// ----------------------------------------------------------------------------

int CTemplateListWnd::InsertTemplate(int i, int classID)
{
	// create window control
	CRect rectSpikes (0, 0, m_tpllen, 64);		// dummy position	
	CTemplateWnd* pWnd = new (CTemplateWnd);
	ASSERT(pWnd != NULL);
	pWnd->Create(_T(""), WS_CHILD |WS_VISIBLE, rectSpikes, this, i);	
	pWnd->m_csID = _T("#");
	pWnd->m_classID = classID;

	// init parameters
	pWnd->SetTemplateLength(m_tpllen);
	pWnd->SetYWExtOrg(m_yextent, m_yzero);
	pWnd->SetXWExtOrg(m_tpright - m_tpleft +1, m_tpleft);
	pWnd->SetbDrawframe(TRUE);
	pWnd->SetbUseDIB(FALSE);
	pWnd->m_ktolerance = m_ktolerance;
	pWnd->m_globalstd = m_globalstd;

	// store data
	int index = i;
	m_ptpl.InsertAt(index, pWnd, 1);

	// insert item if window was created
	if (::IsWindow(m_hWnd))
	{		
		LV_ITEM item;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = (LPARAM) pWnd;
		int index1 = InsertItem(&item);
	}
	
	// return -1 if something is wrong
	return index;
}
void CTemplateListWnd::TransferTemplateData()
{
	for (int i=0; i< m_ptpl.GetSize(); i++)
	{		
		LV_ITEM item;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = (LPARAM) m_ptpl.GetAt(i);
		int index = InsertItem(&item);
		ASSERT(index >= 0);
	}
}
	
int CTemplateListWnd::InsertTemplateData(int i, int classID)
{
	// create window control
	CRect rectSpikes (1000, 0, m_tpllen, 64+1000);		// dummy position	
	CTemplateWnd* pWnd = new (CTemplateWnd);
	ASSERT(pWnd != NULL);
	pWnd->Create(_T(""), WS_CHILD |WS_VISIBLE, rectSpikes, this, i);	
	pWnd->m_csID = _T("#");
	pWnd->m_classID = classID;

	// init parameters
	pWnd->SetTemplateLength(m_tpllen);
	pWnd->SetYWExtOrg(m_yextent, m_yzero);
	pWnd->SetXWExtOrg(m_tpright - m_tpleft +1, m_tpleft);
	pWnd->SetbDrawframe(TRUE);
	pWnd->SetbUseDIB(FALSE);
	pWnd->m_ktolerance = m_ktolerance;
	pWnd->m_globalstd = m_globalstd;

	// store data
	m_ptpl.InsertAt(i, pWnd, 1);

	// get item index
	int index = -1;
	for (int i=0; i< m_ptpl.GetSize(); i++)
	{
		if (GetTemplateclassID(i) == classID)
		{
			index = i;
			break;
		}
	}
	return index;
}

// ----------------------------------------------------------------------------

BOOL CTemplateListWnd::DeleteAllTemplates()
{
	m_tpl0.tInit();
	BOOL flag = TRUE;
	if (m_ptpl.GetSize() > 0)
	{
		if (::IsWindow(m_hWnd))
			DeleteAllItems();
		for (int i=0; i< m_ptpl.GetSize(); i++)
		{
			CTemplateWnd* pS = (CTemplateWnd*) m_ptpl.GetAt(i);
			delete pS;
		}
		m_ptpl.RemoveAll();
	}

	return flag;
}
// ----------------------------------------------------------------------------
void CTemplateListWnd::SortTemplatesByClass(BOOL bUp)
{
	// sort m_ptpl
	for (int i=0; i< m_ptpl.GetSize()-1; i++)
	{
		int IDi = GetTemplateclassID(i);
		int IDjmin = IDi;
		int jmin = i;
		BOOL bfound = FALSE;
		for (int j=i+1; j <m_ptpl.GetSize(); j++)
		{
			int IDj = GetTemplateclassID(j);
			if (bUp)
			{
				if (IDj > IDjmin)
				{
					jmin= j;
					IDjmin = IDj;
					bfound = TRUE;
				}
			}
			else
			{
				if (IDj < IDjmin)
				{
					jmin = j;
					IDjmin = IDj;
					bfound = TRUE;
				}
			}
		}
		// exchange items in m_ptpl
		if (bfound)
		{
			CTemplateWnd* pWnd = (CTemplateWnd*) m_ptpl.GetAt(i);
			m_ptpl.SetAt(i, m_ptpl.GetAt(jmin));
			m_ptpl.SetAt(jmin, pWnd);
		}
	}
	// affect corresp pWnd to CListCtrl
	for (int i=0; i< m_ptpl.GetSize(); i++)
	{
		LVITEM item;
		item.iItem=i;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		GetItem(&item);

		item.iItem=i;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = (LPARAM) m_ptpl.GetAt(i);
		SetItem(&item);
	}
}

void CTemplateListWnd::SortTemplatesByNumberofSpikes(BOOL bUp, BOOL bUpdateClasses, int minclassnb)
{
	// sort m_ptpl
	for (int i=0; i< m_ptpl.GetSize()-1; i++)
	{
		CTemplateWnd* pWndi = (CTemplateWnd*) m_ptpl.GetAt(i);
		CTemplateWnd* pWndjfound = pWndi;
		int IDi = pWndi->GetNitems();	
		int IDjfound = IDi;
		int jfound = i;
		BOOL bfound = FALSE;
		for (int j=i+1; j <m_ptpl.GetSize(); j++)
		{
			CTemplateWnd* pWndj = (CTemplateWnd*) m_ptpl.GetAt(j);
			int IDj = pWndj->GetNitems();	
			if (bUp)
			{
				if (IDj > IDjfound)
				{
						jfound = j;
						IDjfound = IDj;
						pWndjfound = pWndj;
						bfound = TRUE;
				}
			}
			else
			{
				if (IDj < IDjfound)
				{
					jfound = j;
					IDjfound = IDj;
					pWndjfound = pWndj;
					bfound = TRUE;
				}
			}
		}
		// exchange items in m_ptpl
		if (bfound)
		{			
			m_ptpl.SetAt(i, pWndjfound);
			m_ptpl.SetAt(jfound, pWndi);
		}

	}


	if (bUpdateClasses)
	{
		for (int i=0; i< m_ptpl.GetSize(); i++)
		{
			CTemplateWnd* pWndi = (CTemplateWnd*) m_ptpl.GetAt(i);
			pWndi->m_classID = minclassnb + i;
		}
	}

	// affect corresp pWnd to CListCtrl
	for (int i=0; i< m_ptpl.GetSize(); i++)
	{
		LVITEM item;
		item.iItem=i;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		GetItem(&item);

		ASSERT(item.iItem==i);
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = (LPARAM) m_ptpl.GetAt(i);
		SetItem(&item);
	}
}
// ----------------------------------------------------------------------------

BOOL CTemplateListWnd::DeleteItem(int idelete)
{
	// search corresponding window
	if (::IsWindow(m_hWnd))
	{
		CListCtrl::DeleteItem(idelete);
	}
	else
	{
		CTemplateWnd* pWnd = (CTemplateWnd* ) GetItemData(idelete);	
		delete pWnd;
		m_ptpl.RemoveAt(idelete);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTemplateListWnd, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnDeleteitem)
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateListWnd message handlers


// ---------------------------------------------------------------------

void CTemplateListWnd::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// message received (reflected) : LVN_GETDISPINFO
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM item = pDispInfo->item;

	// get position of display window and move it
	CRect rect;
	GetItemRect(item.iItem, &rect, LVIR_ICON);

	rect.left += 8;
	rect.right -= 8;
	rect.top += 2;
	rect.bottom -= 2;

	CTemplateWnd* pS = (CTemplateWnd* ) GetItemData(item.iItem);
	pS->SetbDrawframe((LVIS_SELECTED == GetItemState (item.iItem, LVIS_SELECTED)));

	CRect rect2;
	pS->GetWindowRect(&rect2);
	ScreenToClient(&rect2);
	if (rect2 != rect)
		pS->MoveWindow(&rect);

	*pResult = 0;
}

// ---------------------------------------------------------------------

void CTemplateListWnd::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// message received (reflected) : LVN_DELETEITEM
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// delete corresponding window object
	CTemplateWnd* pS = (CTemplateWnd*) pNMListView->lParam;	
	if (pS != nullptr)
	{
		// search corresponding window
		int item=-1;
		for (int i=0; i< m_ptpl.GetSize(); i++)
		{
			if ((CTemplateWnd*) m_ptpl.GetAt(i) == pS)
			{
				item=i;
				break;
			}
		}
		pS->DestroyWindow();
		delete pS;
		m_ptpl.RemoveAt(item);
		pNMListView->lParam = NULL;
	}
	*pResult = 0;
}

// ---------------------------------------------------------------------

void CTemplateListWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{	
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
	RedrawItems(0, m_ptpl.GetSize()-1);
}

// ---------------------------------------------------------------------

void CTemplateListWnd::UpdateTemplateLegends(LPCSTR pszType)
{
	TCHAR szItem[20]; 
	for (int i= m_ptpl.GetSize()-1; i>= 0; i--)
	{
		CTemplateWnd* ptplWnd = GetTemplateWnd(i);
		ptplWnd->m_csID = pszType;

		wsprintf(szItem, _T("%s%i n_spk=%i"), (LPCTSTR) ptplWnd->m_csID, ptplWnd->m_classID, ptplWnd->GetNitems());
		SetItemText(i, 0, szItem);
	}
}

void CTemplateListWnd::UpdateTemplateBaseClassID(int inewlowestclassID)
{ 
	// first, get lowest template class ID
	if (m_ptpl.GetSize() <1)
		return;
	CTemplateWnd* ptplWnd = GetTemplateWnd(0);
	int lowestID = ptplWnd->m_classID;
	for (int i= m_ptpl.GetSize()-1; i>= 0; i--)
	{
		CTemplateWnd* ptplWnd = GetTemplateWnd(i);
		if (lowestID > ptplWnd->m_classID)
			lowestID =  ptplWnd->m_classID; 
	}
	// now change the id of each template and update it's text
	TCHAR szItem[20];
	int delta = lowestID -inewlowestclassID;
	for (int i= m_ptpl.GetSize()-1; i>= 0; i--)
	{
		CTemplateWnd* ptplWnd = GetTemplateWnd(i);
		ptplWnd->m_classID = ptplWnd->m_classID - delta; 
		wsprintf(szItem, _T("%s%i n_spk=%i"), (LPCTSTR) ptplWnd->m_csID, ptplWnd->m_classID, ptplWnd->GetNitems());
		SetItemText(i, 0, szItem);
	}
}
	
// ---------------------------------------------------------------------

void CTemplateListWnd::SetTemplateclassID(int item, LPCTSTR pszType, int classID)
{
	CString csItem;
	CTemplateWnd* ptplWnd = GetTemplateWnd(item);
	ptplWnd->m_classID = classID;
	CString cs = pszType;
	if (!cs.IsEmpty())
		ptplWnd->m_csID = cs;
	csItem.Format(_T("%s%i n_spk=%i"), pszType, classID, ptplWnd->GetNitems());
	SetItemText(item, 0, csItem);
}

// ---------------------------------------------------------------------

BOOL CTemplateListWnd::tInit(int i)
{
	BOOL flag = (i>= 0) && (i < m_ptpl.GetSize());
	if (flag)
		GetTemplateWnd(i)->tInit();
	return flag;
}

// ---------------------------------------------------------------------
BOOL CTemplateListWnd::tAdd(short* pSource)
{
	m_tpl0.tAdd(pSource);
	return TRUE;
}

// ---------------------------------------------------------------------

BOOL CTemplateListWnd::tAdd(int i, short* pSource)
{
	BOOL flag = (i>= 0) && (i < m_ptpl.GetSize());
	if (flag)
	{
		GetTemplateWnd(i)->tAdd(pSource);		
	}
	return flag;
}

// ---------------------------------------------------------------------

BOOL CTemplateListWnd::tPower(int i, double* xpower)
{
	BOOL flag = (i>= 0) && (i < m_ptpl.GetSize());
	if (flag)
		*xpower = GetTemplateWnd(i)->tPower();
	return flag;
}

// ---------------------------------------------------------------------

BOOL CTemplateListWnd::tWithin(int i, short* pSource)
{
	BOOL flag = (i>= 0) && (i < m_ptpl.GetSize());	
	if (flag)
		flag = GetTemplateWnd(i)->tWithin(pSource, &m_hitrate);
	return flag;
}

// ---------------------------------------------------------------------

BOOL CTemplateListWnd::tMinDist(int i, short* pSource, int* poffsetmin, double* pdistmin)
{
	BOOL flag = (i>= 0) && (i < m_ptpl.GetSize());
	if (flag)
	{
		*pdistmin = GetTemplateWnd(i)->tMinDist(pSource, poffsetmin);
	}
	return flag;
}

// ---------------------------------------------------------------------

void CTemplateListWnd::tGlobalstats() 
{
	m_tpl0.tGlobalstats(&m_globalstd, &m_globaldist);
	for (int i=0; i< m_ptpl.GetSize(); i++)
	{
		GetTemplateWnd(i)->SetGlobalstd(&m_globalstd);
	}
}

// ---------------------------------------------------------------------

void CTemplateListWnd::SetTemplateLength(int spklen, int tpleft, int tpright)
{
	if (tpleft != m_tpleft || tpright != m_tpright)
	{
		m_tpleft = tpleft;
		m_tpright = tpright;		
	}
	
	int len = tpright - tpleft +1;
	m_tpllen = spklen;
	for (int i=0; i< m_ptpl.GetSize(); i++)
		GetTemplateWnd(i)->SetTemplateLength(spklen, len, m_tpleft);
	m_tpl0.SetTemplateLength(spklen, len, m_tpleft);
	m_tpl0.tInit();
}

// ---------------------------------------------------------------------

void CTemplateListWnd::SetHitRate_Tolerance(int* phitrate, float* ptolerance)
{
	m_hitrate = *phitrate;
	if (m_ktolerance != *ptolerance)
	{
		m_ktolerance = *ptolerance;
		for (int i=0; i< m_ptpl.GetSize(); i++)
			GetTemplateWnd(i)->SetkTolerance(&m_ktolerance);
		m_tpl0.m_ktolerance = m_ktolerance;
	}
}

// ---------------------------------------------------------------------

void CTemplateListWnd::SetYWExtOrg(int extent, int zero)
{
	m_yextent = extent;
	m_yzero = zero;
	for (int i=0; i< m_ptpl.GetSize(); i++)
		GetTemplateWnd(i)->SetYWExtOrg(extent, zero);
}

// ---------------------------------------------------------------------

void CTemplateListWnd::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint	ptItem, ptAction, ptImage;
	NM_LISTVIEW* pnmListView = (NM_LISTVIEW*)pNMHDR;

	ASSERT(!m_bDragging);
	m_bDragging = TRUE;	
	m_iItemDrag = pnmListView->iItem;

	ptAction = pnmListView->ptAction;
	GetItemPosition(m_iItemDrag, &ptItem);  // ptItem is relative to (0,0) and not the view origin
	GetOrigin(&m_ptOrigin);

	// Update image list to make sure all images are loaded
	CImageList* pI = GetImageList(LVSIL_NORMAL); // Image list with large icons.
	int nimage = pI->GetImageCount();
	HIMAGELIST hI;
	if (nimage == 0 || m_iItemDrag > nimage +1)
	{
		int ifirst = 0;
		if (m_iItemDrag > nimage +1)
			ifirst = nimage;
		// CImageList
		hI = pI->m_hImageList;
		//for (int i=ifirst; i<m_ptpl.GetSize(); i++)
		//{
		//	//int ii = ImageList_Add(hI, GetTemplateWnd(i)->GetBitmapPlotHandle(), NULL);
		//	int ii = ImageList_Add(hI, NULL, NULL);
		//	ASSERT(ii != -1);
		//}
	}

	ASSERT(m_pimageListDrag == NULL);
	m_pimageListDrag = CreateDragImage(m_iItemDrag, &ptImage);

	m_sizeDelta = ptAction - ptImage;   // difference between cursor pos and image pos
	m_ptHotSpot = ptAction - ptItem + m_ptOrigin;  // calculate hotspot for the cursor
	m_pimageListDrag->DragShowNolock(TRUE);  // lock updates and show drag image
	m_pimageListDrag->SetDragCursorImage(0, m_ptHotSpot);  // define the hot spot for the new cursor image
	m_pimageListDrag->BeginDrag(0, CPoint(0, 0));
	ptAction -= m_sizeDelta;
	m_pimageListDrag->DragEnter(this, ptAction);
	m_pimageListDrag->DragMove(ptAction);  // move image to overlap original icon
	SetCapture();
}

// ---------------------------------------------------------------------

void CTemplateListWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	long		lStyle;
	int			iItem;
	LV_ITEM		lvitem;

	lStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	lStyle &= LVS_TYPEMASK;  // drag will do different things in list and report mode
	if (m_bDragging)
	{
		ASSERT(m_pimageListDrag != NULL);
		m_pimageListDrag->DragMove(point - m_sizeDelta);  // move the image

		if ((iItem = HitTest(point)) != -1)
		{
			m_iItemDrop = iItem;
			m_pimageListDrag->DragLeave(this); // unlock the window and hide drag image
			if (lStyle == LVS_REPORT || lStyle == LVS_LIST)
			{
				lvitem.iItem = iItem;
				lvitem.iSubItem = 0;
				lvitem.mask = LVIF_STATE;
				lvitem.stateMask = LVIS_DROPHILITED;  // highlight the drop target
				SetItem(&lvitem);
			}
			point -= m_sizeDelta;
			m_pimageListDrag->DragEnter(this, point);  // lock updates and show drag image
		}
	}
	CListCtrl::OnMouseMove(nFlags, point);
}

// ---------------------------------------------------------------------

void CTemplateListWnd::OnButtonUp(CPoint point)
{
	if (m_bDragging)  // end of the drag operation
	{
		long		lStyle;
		CString		cstr;

		lStyle = GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK; 
		m_bDragging = FALSE;

		ASSERT(m_pimageListDrag != NULL);
		m_pimageListDrag->DragLeave(this);
		m_pimageListDrag->EndDrag();
		delete m_pimageListDrag;
		m_pimageListDrag = nullptr;

		if (lStyle == LVS_REPORT && m_iItemDrop != m_iItemDrag)  
		{
			cstr = GetItemText(m_iItemDrag, 0);
			SetItemText(m_iItemDrop, 1, cstr);  // drop subitem text is dragged main item text
		}
		
		if (lStyle == LVS_LIST && m_iItemDrop != m_iItemDrag)  //add ** to the drop item text
		{
			cstr = GetItemText(m_iItemDrop, 0);
			cstr += _T("**");
			SetItemText(m_iItemDrop, 0, cstr);
		}
	
		if (lStyle == LVS_ICON || lStyle == LVS_SMALLICON)  // move the icon
		{
			point -= m_ptHotSpot;  // the icon should be drawn exactly where the image is
			point += m_ptOrigin;
			SetItemPosition(m_iItemDrag, point);  // just move the dragged item
		}

		::ReleaseCapture();
	}
}

// ---------------------------------------------------------------------

void CTemplateListWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	OnButtonUp(point);
	CListCtrl::OnLButtonUp(nFlags, point);
}

void CTemplateListWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	LVFINDINFO lvInfo;
	lvInfo.flags = LVFI_NEARESTXY;
	lvInfo.pt = point;
	int item = FindItem(&lvInfo, -1);
	
	CListCtrl::OnRButtonDown(nFlags, point);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_RMOUSEBUTTONDOWN, MAKELONG(item, GetDlgCtrlID()));
}
