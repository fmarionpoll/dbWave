// TemplateListWnd.cpp : implementation file
//

#include "StdAfx.h"

//#include "resource.h"
#include "ChartWnd.h"
#include "TemplateWnd.h"
#include "TemplateListWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CTemplateListWnd, CListCtrl, 0 /* schema number*/)

CTemplateListWnd::CTemplateListWnd()
{
	m_tpl0.tInit();
}

CTemplateListWnd::~CTemplateListWnd()
{
	for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
	{
		const auto template_wnd_ptr = templatewnd_ptr_array.GetAt(i);
		delete template_wnd_ptr;
	}
	templatewnd_ptr_array.RemoveAll();
}

CTemplateListWnd& CTemplateListWnd::operator =(const CTemplateListWnd& arg)
{
	if (this != &arg)
	{
		DeleteAllTemplates();
		m_tpl0 = arg.m_tpl0;
		for (auto i = 0; i < arg.templatewnd_ptr_array.GetSize(); i++)
		{
			const auto p_source = arg.GetTemplateWnd(i);
			InsertTemplate(i, 0);
			const auto p_destination = GetTemplateWnd(i);
			*p_destination = *p_source;
		}
	}
	return *this;
}

void CTemplateListWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		const WORD wversion = 1;
		ar << wversion;

		const auto nstrings = 0;
		ar << nstrings;

		const auto nints = 6;
		ar << nints;
		ar << m_tpllen;
		ar << m_tpleft;
		ar << m_tpright;
		ar << m_hitrate;
		ar << m_yextent;
		ar << m_yzero;

		const auto nfloats = 1;
		ar << nfloats;
		ar << m_ktolerance;

		const auto ndoubles = 2;
		ar << ndoubles;
		ar << m_globalstd; // 1
		ar << m_globaldist; // 2

		// serialize templates
		m_tpl0.Serialize(ar);
		ar << templatewnd_ptr_array.GetSize();
		if (templatewnd_ptr_array.GetSize() > 0)
		{
			for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
			{
				auto p_spike_element = GetTemplateWnd(i);
				p_spike_element->Serialize(ar);
			}
		}
	}
	else
	{
		WORD version;
		ar >> version;

		int nstrings;
		ar >> nstrings;

		int nints;
		ar >> nints;
		ar >> m_tpllen;
		ar >> m_tpleft;
		ar >> m_tpright;
		ar >> m_hitrate;
		ar >> m_yextent;
		ar >> m_yzero;

		int nfloats;
		ar >> nfloats;
		ar >> m_ktolerance;

		int ndoubles;
		ar >> ndoubles;
		ar >> m_globalstd; // 1
		ar >> m_globaldist; // 2

		// serialize templates
		m_tpl0.Serialize(ar);
		int nitems;
		ar >> nitems;
		if (nitems > 0)
		{
			for (auto i = 0; i < nitems; i++)
			{
				InsertTemplate(i, 0);
				auto p_destination = GetTemplateWnd(i);
				p_destination->Serialize(ar);
			}
		}
	}
}

int CTemplateListWnd::InsertTemplate(int i, int classID)
{
	// create window control
	const CRect rect_spikes(0, 0, m_tpllen, 64); // dummy position
	auto p_wnd = new (CTemplateWnd);
	ASSERT(p_wnd != NULL);
	p_wnd->Create(_T(""), WS_CHILD | WS_VISIBLE, rect_spikes, this, i);
	p_wnd->m_csID = _T("#");
	p_wnd->m_classID = classID;

	// init parameters
	p_wnd->SetTemplateLength(m_tpllen);
	p_wnd->set_yw_ext_org(m_yextent, m_yzero);
	p_wnd->SetXWExtOrg(m_tpright - m_tpleft + 1, m_tpleft);
	p_wnd->SetbDrawframe(TRUE);
	p_wnd->set_b_use_dib(FALSE);
	p_wnd->m_ktolerance = m_ktolerance;
	p_wnd->m_globalstd = m_globalstd;

	// store data
	const auto index = i;
	templatewnd_ptr_array.InsertAt(index, p_wnd, 1);

	// insert item if window was created
	if (IsWindow(m_hWnd))
	{
		LV_ITEM item;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = reinterpret_cast<LPARAM>(p_wnd);
		/*int index1 = */
		InsertItem(&item);
	}

	// return -1 if something is wrong
	return index;
}

void CTemplateListWnd::TransferTemplateData()
{
	for (int i = 0; i < templatewnd_ptr_array.GetSize(); i++)
	{
		LV_ITEM item;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = reinterpret_cast<LPARAM>(templatewnd_ptr_array.GetAt(i));
		const auto index = InsertItem(&item);
		ASSERT(index >= 0);
	}
}

int CTemplateListWnd::InsertTemplateData(int i, int classID)
{
	// create window control
	const CRect rect_spikes(1000, 0, m_tpllen, 64 + 1000); // dummy position
	auto p_wnd = new (CTemplateWnd);
	ASSERT(p_wnd != NULL);
	p_wnd->Create(_T(""), WS_CHILD | WS_VISIBLE, rect_spikes, this, i);
	p_wnd->m_csID = _T("#");
	p_wnd->m_classID = classID;

	// init parameters
	p_wnd->SetTemplateLength(m_tpllen);
	p_wnd->set_yw_ext_org(m_yextent, m_yzero);
	p_wnd->SetXWExtOrg(m_tpright - m_tpleft + 1, m_tpleft);
	p_wnd->SetbDrawframe(TRUE);
	p_wnd->set_b_use_dib(FALSE);
	p_wnd->m_ktolerance = m_ktolerance;
	p_wnd->m_globalstd = m_globalstd;

	// store data
	templatewnd_ptr_array.InsertAt(i, p_wnd, 1);

	// get item index
	auto index = -1;
	for (auto j = 0; j < templatewnd_ptr_array.GetSize(); j++)
	{
		if (GetTemplateclassID(j) == classID)
		{
			index = j;
			break;
		}
	}
	return index;
}

void CTemplateListWnd::DeleteAllTemplates()
{
	m_tpl0.tInit();
	if (templatewnd_ptr_array.GetSize() > 0)
	{
		if (IsWindow(m_hWnd))
			DeleteAllItems();
		for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
		{
			const auto p_spike_element = templatewnd_ptr_array.GetAt(i);
			delete p_spike_element;
		}
		templatewnd_ptr_array.RemoveAll();
	}
}

void CTemplateListWnd::SortTemplatesByClass(BOOL bUp)
{
	// sort m_ptpl
	for (auto i = 0; i < templatewnd_ptr_array.GetSize() - 1; i++)
	{
		const auto id_i = GetTemplateclassID(i);
		auto id_jmin = id_i;
		auto jmin = i;
		auto bfound = FALSE;
		for (auto j = i + 1; j < templatewnd_ptr_array.GetSize(); j++)
		{
			const auto id_j = GetTemplateclassID(j);
			if (bUp)
			{
				if (id_j > id_jmin)
				{
					jmin = j;
					id_jmin = id_j;
					bfound = TRUE;
				}
			}
			else
			{
				if (id_j < id_jmin)
				{
					jmin = j;
					id_jmin = id_j;
					bfound = TRUE;
				}
			}
		}
		// exchange items in m_ptpl
		if (bfound)
		{
			const auto p_wnd = templatewnd_ptr_array.GetAt(i);
			templatewnd_ptr_array.SetAt(i, templatewnd_ptr_array.GetAt(jmin));
			templatewnd_ptr_array.SetAt(jmin, p_wnd);
		}
	}
	// affect corresp p_wnd to CListCtrl
	for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
	{
		LVITEM item;
		item.iItem = i;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		GetItem(&item);

		item.iItem = i;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = reinterpret_cast<LPARAM>(templatewnd_ptr_array.GetAt(i));
		SetItem(&item);
	}
}

CTemplateWnd* CTemplateListWnd::GetTemplateWndForClass(int iclass)
{
	CTemplateWnd* pTemplate = nullptr;
	return pTemplate;
}

void CTemplateListWnd::SortTemplatesByNumberofSpikes(BOOL bUp, BOOL bUpdateClasses, int minclassnb)
{
	// sort m_ptpl
	for (auto i = 0; i < templatewnd_ptr_array.GetSize() - 1; i++)
	{
		const auto p_wndi = templatewnd_ptr_array.GetAt(i);
		auto p_wndjfound = p_wndi;
		const auto id_i = p_wndi->GetNitems();
		auto id_jfound = id_i;
		auto jfound = i;
		auto bfound = FALSE;
		for (auto j = i + 1; j < templatewnd_ptr_array.GetSize(); j++)
		{
			const auto p_wndj = templatewnd_ptr_array.GetAt(j);
			const auto id_j = p_wndj->GetNitems();
			if (bUp)
			{
				if (id_j > id_jfound)
				{
					jfound = j;
					id_jfound = id_j;
					p_wndjfound = p_wndj;
					bfound = TRUE;
				}
			}
			else
			{
				if (id_j < id_jfound)
				{
					jfound = j;
					id_jfound = id_j;
					p_wndjfound = p_wndj;
					bfound = TRUE;
				}
			}
		}
		// exchange items in m_ptpl
		if (bfound)
		{
			templatewnd_ptr_array.SetAt(i, p_wndjfound);
			templatewnd_ptr_array.SetAt(jfound, p_wndi);
		}
	}

	if (bUpdateClasses)
	{
		for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
		{
			const auto p_wndi = templatewnd_ptr_array.GetAt(i);
			p_wndi->m_classID = minclassnb + i;
		}
	}

	// affect corresp p_wnd to CListCtrl
	for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
	{
		LVITEM item;
		item.iItem = i;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		GetItem(&item);

		ASSERT(item.iItem == i);
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = reinterpret_cast<LPARAM>(templatewnd_ptr_array.GetAt(i));
		SetItem(&item);
	}
}

BOOL CTemplateListWnd::DeleteItem(int idelete)
{
	// search corresponding window
	if (IsWindow(m_hWnd))
	{
		CListCtrl::DeleteItem(idelete);
	}
	else
	{
		const auto p_wnd = reinterpret_cast<CTemplateWnd*>(GetItemData(idelete));
		delete p_wnd;
		templatewnd_ptr_array.RemoveAt(idelete);
	}
	return TRUE;
}

BEGIN_MESSAGE_MAP(CTemplateListWnd, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnDeleteitem)
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

void CTemplateListWnd::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	// message received (reflected) : LVN_GETDISPINFO
	auto* p_disp_info = reinterpret_cast<LV_DISPINFO*>(pNMHDR);
	const auto item = p_disp_info->item;

	// get position of display window and move it
	CRect rect;
	GetItemRect(item.iItem, &rect, LVIR_ICON);

	rect.left += 8;
	rect.right -= 8;
	rect.top += 2;
	rect.bottom -= 2;

	auto p_spike_element = reinterpret_cast<CTemplateWnd*>(GetItemData(item.iItem));
	p_spike_element->SetbDrawframe((LVIS_SELECTED == GetItemState(item.iItem, LVIS_SELECTED)));

	CRect rect2;
	p_spike_element->GetWindowRect(&rect2);
	ScreenToClient(&rect2);
	if (rect2 != rect)
		p_spike_element->MoveWindow(&rect);

	*pResult = 0;
}

void CTemplateListWnd::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult)
{
	// message received (reflected) : LVN_DELETEITEM
	auto* p_nm_list_view = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);

	// delete corresponding window object
	auto* p_spike_element = reinterpret_cast<CTemplateWnd*>(p_nm_list_view->lParam);
	if (p_spike_element != nullptr)
	{
		// search corresponding window
		auto item = -1;
		for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
		{
			if (templatewnd_ptr_array.GetAt(i) == p_spike_element)
			{
				item = i;
				break;
			}
		}
		delete p_spike_element;
		templatewnd_ptr_array.RemoveAt(item);
		p_nm_list_view->lParam = NULL;
	}
	*pResult = 0;
}

void CTemplateListWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
	RedrawItems(0, templatewnd_ptr_array.GetSize() - 1);
}

void CTemplateListWnd::UpdateTemplateLegends(LPCSTR pszType)
{
	TCHAR sz_item[20];
	for (auto i = templatewnd_ptr_array.GetSize() - 1; i >= 0; i--)
	{
		auto ptplWnd = GetTemplateWnd(i);
		ptplWnd->m_csID = pszType;

		wsprintf(sz_item, _T("%s%i n_spk=%i"), static_cast<LPCTSTR>(ptplWnd->m_csID), ptplWnd->m_classID,
		         ptplWnd->GetNitems());
		SetItemText(i, 0, sz_item);
	}
}

void CTemplateListWnd::UpdateTemplateBaseClassID(int inewlowestclassID)
{
	// first, get lowest template class ID
	if (templatewnd_ptr_array.GetSize() < 1)
		return;
	const auto ptpl_wnd = GetTemplateWnd(0);
	auto lowest_id = ptpl_wnd->m_classID;
	for (auto i = templatewnd_ptr_array.GetSize() - 1; i >= 0; i--)
	{
		const auto ptpl1_wnd = GetTemplateWnd(i);
		if (lowest_id > ptpl1_wnd->m_classID)
			lowest_id = ptpl1_wnd->m_classID;
	}
	// now change the id of each template and update it's text
	TCHAR sz_item[20];
	const auto delta = lowest_id - inewlowestclassID;
	for (auto i = templatewnd_ptr_array.GetSize() - 1; i >= 0; i--)
	{
		const auto ptpl2_wnd = GetTemplateWnd(i);
		ptpl2_wnd->m_classID = ptpl2_wnd->m_classID - delta;
		wsprintf(sz_item, _T("%s%i n_spk=%i"), static_cast<LPCTSTR>(ptpl2_wnd->m_csID), ptpl2_wnd->m_classID,
		         ptpl2_wnd->GetNitems());
		SetItemText(i, 0, sz_item);
	}
}

void CTemplateListWnd::SetTemplateclassID(int item, LPCTSTR pszType, int classID)
{
	CString cs_item;
	auto ptplWnd = GetTemplateWnd(item);
	ptplWnd->m_classID = classID;
	CString cs = pszType;
	if (!cs.IsEmpty())
		ptplWnd->m_csID = cs;
	cs_item.Format(_T("%s%i n_spk=%i"), pszType, classID, ptplWnd->GetNitems());
	SetItemText(item, 0, cs_item);
}

BOOL CTemplateListWnd::tInit(int i)
{
	const BOOL flag = (i >= 0) && (i < templatewnd_ptr_array.GetSize());
	if (flag)
		GetTemplateWnd(i)->tInit();
	return flag;
}

BOOL CTemplateListWnd::tAdd(short* p_source)
{
	m_tpl0.tAddSpikeTopSum(p_source);
	return TRUE;
}

BOOL CTemplateListWnd::tAdd(int i, short* p_source)
{
	const BOOL flag = (i >= 0) && (i < templatewnd_ptr_array.GetSize());
	if (flag)
	{
		GetTemplateWnd(i)->tAddSpikeTopSum(p_source);
	}
	return flag;
}

BOOL CTemplateListWnd::tPower(int i, double* xpower)
{
	const BOOL flag = (i >= 0) && (i < templatewnd_ptr_array.GetSize());
	if (flag)
		*xpower = GetTemplateWnd(i)->tPowerOfpSum();
	return flag;
}

BOOL CTemplateListWnd::tWithin(int i, short* p_source)
{
	BOOL flag = i >= 0 && (i < templatewnd_ptr_array.GetSize());
	if (flag)
		flag = GetTemplateWnd(i)->tGetNumberOfPointsWithin(p_source, &m_hitrate);
	return flag;
}

BOOL CTemplateListWnd::tMinDist(int i, short* p_source, int* poffsetmin, double* pdistmin)
{
	const BOOL flag = i >= 0 && (i < templatewnd_ptr_array.GetSize());
	if (flag)
	{
		*pdistmin = GetTemplateWnd(i)->tMinDist(p_source, poffsetmin);
	}
	return flag;
}

void CTemplateListWnd::tGlobalstats()
{
	m_tpl0.tGlobalstats(&m_globalstd, &m_globaldist);
	for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
	{
		GetTemplateWnd(i)->SetGlobalstd(&m_globalstd);
	}
}

void CTemplateListWnd::SetTemplateLength(int spklen, int tpleft, int tpright)
{
	if (tpleft != m_tpleft || tpright != m_tpright)
	{
		m_tpleft = tpleft;
		m_tpright = tpright;
	}

	const auto len = tpright - tpleft + 1;
	m_tpllen = spklen;
	for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
		GetTemplateWnd(i)->SetTemplateLength(spklen, len, m_tpleft);
	m_tpl0.SetTemplateLength(spklen, len, m_tpleft);
	m_tpl0.tInit();
}

void CTemplateListWnd::SetHitRate_Tolerance(int* phitrate, float* ptolerance)
{
	m_hitrate = *phitrate;
	if (m_ktolerance != *ptolerance)
	{
		m_ktolerance = *ptolerance;
		for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
		{
			CTemplateWnd* ptemplate = GetTemplateWnd(i);
			ptemplate->SetkTolerance(&m_ktolerance);
		}
		m_tpl0.m_ktolerance = m_ktolerance;
	}
}

void CTemplateListWnd::SetYWExtOrg(int extent, int zero)
{
	m_yextent = extent;
	m_yzero = zero;
	for (auto i = 0; i < templatewnd_ptr_array.GetSize(); i++)
	{
		CTemplateWnd* ptemplate = GetTemplateWnd(i);
		ptemplate->set_yw_ext_org(extent, zero);
	}
}

void CTemplateListWnd::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt_item, pt_image;
	auto* pnm_list_view = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);

	ASSERT(!m_bDragging);
	m_bDragging = TRUE;
	m_iItemDrag = pnm_list_view->iItem;

	CPoint ptAction = pnm_list_view->ptAction;
	GetItemPosition(m_iItemDrag, &pt_item); // ptItem is relative to (0,0) and not the view origin
	GetOrigin(&m_ptOrigin);

	// Update image list to make sure all images are loaded
	const auto p_i = GetImageList(LVSIL_NORMAL); // Image list with large icons.
	const auto nimage = p_i->GetImageCount();
	auto ifirst = 0;
	if (nimage == 0 || m_iItemDrag > nimage + 1)
	{
		if (m_iItemDrag > nimage + 1)
			ifirst = nimage;
		// CImageList
		auto h_i = p_i->m_hImageList;
		//for (int i=ifirst; i<m_ptpl.GetRectSize(); i++)
		//{
		//	//int ii = ImageList_Add(hI, GetTemplateWnd(i)->GetBitmapPlotHandle(), NULL);
		//	int ii = ImageList_Add(hI, NULL, NULL);
		//	ASSERT(ii != -1);
		//}
	}

	ASSERT(m_pimageListDrag == NULL);
	m_pimageListDrag = CreateDragImage(m_iItemDrag, &pt_image);

	m_sizeDelta = ptAction - pt_image; // difference between cursor pos and image pos
	m_ptHotSpot = ptAction - pt_item + m_ptOrigin; // calculate hotspot for the cursor
	CImageList::DragShowNolock(TRUE); // lock updates and show drag image
	m_pimageListDrag->SetDragCursorImage(0, m_ptHotSpot); // define the hot spot for the new cursor image
	m_pimageListDrag->BeginDrag(0, CPoint(0, 0));
	ptAction -= m_sizeDelta;
	CImageList::DragEnter(this, ptAction);
	CImageList::DragMove(ptAction); // move image to overlap original icon

	//m_pimageListDrag->DragShowNolock(TRUE);  // lock updates and show drag image
	//m_pimageListDrag->SetDragCursorImage(0, m_ptHotSpot);  // define the hot spot for the new cursor image
	//m_pimageListDrag->BeginDrag(0, CPoint(0, 0));
	//ptAction -= m_sizeDelta;
	//m_pimageListDrag->DragEnter(this, ptAction);
	//m_pimageListDrag->DragMove(ptAction);  // move image to overlap original icon

	SetCapture();
}

void CTemplateListWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	int i_item;
	LV_ITEM lvitem;

	auto l_style = GetWindowLong(m_hWnd, GWL_STYLE);
	l_style &= LVS_TYPEMASK; // drag will do different things in list and report mode
	if (m_bDragging)
	{
		ASSERT(m_pimageListDrag != NULL);
		CImageList::DragMove(point - m_sizeDelta); // move the image
		//m_pimageListDrag->DragMove(point - m_sizeDelta);  // move the image

		if ((i_item = HitTest(point)) != -1)
		{
			m_iItemDrop = i_item;
			CImageList::DragLeave(this); // unlock the window and hide drag image
			//m_pimageListDrag->DragLeave(this); // unlock the window and hide drag image
			if (l_style == LVS_REPORT || l_style == LVS_LIST)
			{
				lvitem.iItem = i_item;
				lvitem.iSubItem = 0;
				lvitem.mask = LVIF_STATE;
				lvitem.stateMask = LVIS_DROPHILITED; // highlight the drop target
				SetItem(&lvitem);
			}
			point -= m_sizeDelta;
			CImageList::DragEnter(this, point); // lock updates and show drag image
			//m_pimageListDrag->DragEnter(this, point);  // lock updates and show drag image
		}
	}
	CListCtrl::OnMouseMove(nFlags, point);
}

void CTemplateListWnd::OnButtonUp(CPoint point)
{
	if (m_bDragging) // end of the drag operation
	{
		CString cstr;

		const auto l_style = GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK;
		m_bDragging = FALSE;

		ASSERT(m_pimageListDrag != NULL);
		CImageList::DragLeave(this);
		CImageList::EndDrag();
		delete m_pimageListDrag;
		m_pimageListDrag = nullptr;

		if (l_style == LVS_REPORT && m_iItemDrop != m_iItemDrag)
		{
			cstr = GetItemText(m_iItemDrag, 0);
			SetItemText(m_iItemDrop, 1, cstr);
		}
		else if (l_style == LVS_LIST && m_iItemDrop != m_iItemDrag)
		{
			cstr = GetItemText(m_iItemDrop, 0);
			cstr += _T("**");
			SetItemText(m_iItemDrop, 0, cstr);
		}
		else if (l_style == LVS_ICON || l_style == LVS_SMALLICON)
		{
			point -= m_ptHotSpot;
			point += m_ptOrigin;
			SetItemPosition(m_iItemDrag, point);
		}
		ReleaseCapture();
	}
}

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
	const auto item = FindItem(&lvInfo, -1);

	CListCtrl::OnRButtonDown(nFlags, point);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_RMOUSEBUTTONDOWN, MAKELONG(item, GetDlgCtrlID()));
}
