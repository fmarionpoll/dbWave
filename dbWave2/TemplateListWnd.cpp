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
	m_tpl0_.t_init();
}

CTemplateListWnd::~CTemplateListWnd()
{
	for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
	{
		const auto template_wnd_ptr = template_wnd_ptr_array_.GetAt(i);
		delete template_wnd_ptr;
	}
	template_wnd_ptr_array_.RemoveAll();
}

CTemplateListWnd& CTemplateListWnd::operator =(const CTemplateListWnd& arg)
{
	if (this != &arg)
	{
		delete_all_templates();
		m_tpl0_ = arg.m_tpl0_;
		for (auto i = 0; i < arg.template_wnd_ptr_array_.GetSize(); i++)
		{
			const auto p_source = arg.get_template_wnd(i);
			insert_template(i, 0);
			const auto p_destination = get_template_wnd(i);
			*p_destination = *p_source;
		}
	}
	return *this;
}

void CTemplateListWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		constexpr WORD w_version = 1;
		ar << w_version;

		constexpr auto n_string = 0;
		ar << n_string;

		constexpr auto n_int = 6;
		ar << n_int;
		ar << m_tpl_len_;
		ar << m_tpl_left_;
		ar << m_tpl_right_;
		ar << m_hit_rate_;
		ar << m_y_extent_;
		ar << m_y_zero_;

		constexpr auto n_float = 1;
		ar << n_float;
		ar << m_k_tolerance_;

		constexpr auto n_double = 2;
		ar << n_double;
		ar << m_global_std; // 1
		ar << m_global_dist; // 2

		// serialize templates
		m_tpl0_.Serialize(ar);
		ar << template_wnd_ptr_array_.GetSize();
		if (template_wnd_ptr_array_.GetSize() > 0)
		{
			for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
			{
				const auto p_spike_element = get_template_wnd(i);
				p_spike_element->Serialize(ar);
			}
		}
	}
	else
	{
		WORD version;
		ar >> version;

		int n_string;
		ar >> n_string;

		int n_int;
		ar >> n_int;
		ar >> m_tpl_len_;
		ar >> m_tpl_left_;
		ar >> m_tpl_right_;
		ar >> m_hit_rate_;
		ar >> m_y_extent_;
		ar >> m_y_zero_;

		int n_float;
		ar >> n_float;
		ar >> m_k_tolerance_;

		int n_double;
		ar >> n_double;
		ar >> m_global_std; // 1
		ar >> m_global_dist; // 2

		// serialize templates
		m_tpl0_.Serialize(ar);
		int n_items;
		ar >> n_items;
		if (n_items > 0)
		{
			for (auto i = 0; i < n_items; i++)
			{
				insert_template(i, 0);
				const auto p_destination = get_template_wnd(i);
				p_destination->Serialize(ar);
			}
		}
	}
}

int CTemplateListWnd::insert_template(const int i, const int class_id)
{
	// create window control
	const CRect rect_spikes(0, 0, m_tpl_len_, 64); // dummy position
	auto p_wnd = new (CTemplateWnd);
	ASSERT(p_wnd != NULL);
	p_wnd->Create(_T(""), WS_CHILD | WS_VISIBLE, rect_spikes, this, i);
	p_wnd->m_cs_id = _T("#");
	p_wnd->m_class_id = class_id;

	// init parameters
	p_wnd->set_template_length(m_tpl_len_);
	p_wnd->set_yw_ext_org(m_y_extent_, m_y_zero_);
	p_wnd->set_xw_ext_org(m_tpl_right_ - m_tpl_left_ + 1, m_tpl_left_);
	p_wnd->set_b_draw_frame(TRUE);
	p_wnd->set_b_use_dib(FALSE);
	p_wnd->m_k_tolerance = m_k_tolerance_;
	p_wnd->m_global_std = m_global_std;

	// store data
	const auto index = i;
	template_wnd_ptr_array_.InsertAt(index, p_wnd, 1);

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

void CTemplateListWnd::transfer_template_data()
{
	for (int i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
	{
		LV_ITEM item;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.iImage = I_IMAGECALLBACK;
		item.pszText = nullptr;
		item.lParam = reinterpret_cast<LPARAM>(template_wnd_ptr_array_.GetAt(i));
		const auto index = InsertItem(&item);
		ASSERT(index >= 0);
	}
}

int CTemplateListWnd::insert_template_data(const int i, const int class_id)
{
	// create window control
	const CRect rect_spikes(1000, 0, m_tpl_len_, 64 + 1000); // dummy position
	auto p_wnd = new (CTemplateWnd);
	ASSERT(p_wnd != NULL);
	p_wnd->Create(_T(""), WS_CHILD | WS_VISIBLE, rect_spikes, this, i);
	p_wnd->m_cs_id = _T("#");
	p_wnd->m_class_id = class_id;

	// init parameters
	p_wnd->set_template_length(m_tpl_len_);
	p_wnd->set_yw_ext_org(m_y_extent_, m_y_zero_);
	p_wnd->set_xw_ext_org(m_tpl_right_ - m_tpl_left_ + 1, m_tpl_left_);
	p_wnd->set_b_draw_frame(TRUE);
	p_wnd->set_b_use_dib(FALSE);
	p_wnd->m_k_tolerance = m_k_tolerance_;
	p_wnd->m_global_std = m_global_std;

	// store data
	template_wnd_ptr_array_.InsertAt(i, p_wnd, 1);

	// get item index
	auto index = -1;
	for (auto j = 0; j < template_wnd_ptr_array_.GetSize(); j++)
	{
		if (get_template_class_id(j) == class_id)
		{
			index = j;
			break;
		}
	}
	return index;
}

void CTemplateListWnd::delete_all_templates()
{
	m_tpl0_.t_init();
	if (template_wnd_ptr_array_.GetSize() > 0)
	{
		if (IsWindow(m_hWnd))
			DeleteAllItems();
		for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
		{
			const auto p_spike_element = template_wnd_ptr_array_.GetAt(i);
			delete p_spike_element;
		}
		template_wnd_ptr_array_.RemoveAll();
	}
}

void CTemplateListWnd::sort_templates_by_class(const BOOL b_up)
{
	// sort m_p_tpl
	for (auto i = 0; i < template_wnd_ptr_array_.GetSize() - 1; i++)
	{
		const auto id_i = get_template_class_id(i);
		auto id_j_min = id_i;
		auto j_min = i;
		auto b_found = FALSE;
		for (auto j = i + 1; j < template_wnd_ptr_array_.GetSize(); j++)
		{
			const auto id_j = get_template_class_id(j);
			if (b_up)
			{
				if (id_j > id_j_min)
				{
					j_min = j;
					id_j_min = id_j;
					b_found = TRUE;
				}
			}
			else
			{
				if (id_j < id_j_min)
				{
					j_min = j;
					id_j_min = id_j;
					b_found = TRUE;
				}
			}
		}
		// exchange items in m_p_tpl
		if (b_found)
		{
			const auto p_wnd = template_wnd_ptr_array_.GetAt(i);
			template_wnd_ptr_array_.SetAt(i, template_wnd_ptr_array_.GetAt(j_min));
			template_wnd_ptr_array_.SetAt(j_min, p_wnd);
		}
	}
	// affect corresponding p_wnd to CListCtrl
	for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
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
		item.lParam = reinterpret_cast<LPARAM>(template_wnd_ptr_array_.GetAt(i));
		SetItem(&item);
	}
}

CTemplateWnd* CTemplateListWnd::get_template_wnd_for_class(int i_class)
{
	CTemplateWnd* p_template = nullptr;
	return p_template;
}

void CTemplateListWnd::sort_templates_by_number_of_spikes(const BOOL b_up, const BOOL b_update_classes, const int min_class_nb)
{
	// sort m_p_tpl
	for (auto i = 0; i < template_wnd_ptr_array_.GetSize() - 1; i++)
	{
		const auto p_wnd_i = template_wnd_ptr_array_.GetAt(i);
		auto p_wnd_j_found = p_wnd_i;
		const auto id_i = p_wnd_i->get_n_items();
		auto id_j_found = id_i;
		auto j_found = i;
		auto b_found = FALSE;
		for (auto j = i + 1; j < template_wnd_ptr_array_.GetSize(); j++)
		{
			const auto p_wnd_j = template_wnd_ptr_array_.GetAt(j);
			const auto id_j = p_wnd_j->get_n_items();
			if (b_up)
			{
				if (id_j > id_j_found)
				{
					j_found = j;
					id_j_found = id_j;
					p_wnd_j_found = p_wnd_j;
					b_found = TRUE;
				}
			}
			else
			{
				if (id_j < id_j_found)
				{
					j_found = j;
					id_j_found = id_j;
					p_wnd_j_found = p_wnd_j;
					b_found = TRUE;
				}
			}
		}
		// exchange items in m_p_tpl
		if (b_found)
		{
			template_wnd_ptr_array_.SetAt(i, p_wnd_j_found);
			template_wnd_ptr_array_.SetAt(j_found, p_wnd_i);
		}
	}

	if (b_update_classes)
	{
		for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
		{
			const auto p_wnd_i = template_wnd_ptr_array_.GetAt(i);
			p_wnd_i->m_class_id = min_class_nb + i;
		}
	}

	// affect corresponding p_wnd to CListCtrl
	for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
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
		item.lParam = reinterpret_cast<LPARAM>(template_wnd_ptr_array_.GetAt(i));
		SetItem(&item);
	}
}

BOOL CTemplateListWnd::delete_item(const int item_index)
{
	// search corresponding window
	if (IsWindow(m_hWnd))
	{
		CListCtrl::DeleteItem(item_index);
	}
	else
	{
		const auto p_wnd = reinterpret_cast<CTemplateWnd*>(GetItemData(item_index));
		delete p_wnd;
		template_wnd_ptr_array_.RemoveAt(item_index);
	}
	return TRUE;
}

BEGIN_MESSAGE_MAP(CTemplateListWnd, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, on_get_display_info)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, on_delete_item)
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, on_begin_drag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

void CTemplateListWnd::on_get_display_info(NMHDR* p_nmhdr, LRESULT* p_result)
{
	// message received (reflected) : LVN_GETDISPINFO
	const auto* p_display_info = reinterpret_cast<LV_DISPINFO*>(p_nmhdr);
	const auto item = p_display_info->item;

	// get position of display window and move it
	CRect rect;
	GetItemRect(item.iItem, &rect, LVIR_ICON);

	rect.left += 8;
	rect.right -= 8;
	rect.top += 2;
	rect.bottom -= 2;

	auto p_spike_element = reinterpret_cast<CTemplateWnd*>(GetItemData(item.iItem));
	p_spike_element->set_b_draw_frame((LVIS_SELECTED == GetItemState(item.iItem, LVIS_SELECTED)));

	CRect rect2;
	p_spike_element->GetWindowRect(&rect2);
	ScreenToClient(&rect2);
	if (rect2 != rect)
		p_spike_element->MoveWindow(&rect);

	*p_result = 0;
}

void CTemplateListWnd::on_delete_item(NMHDR* p_nmhdr, LRESULT* p_result)
{
	// message received (reflected) : LVN_DELETEITEM
	auto* p_nm_list_view = reinterpret_cast<NM_LISTVIEW*>(p_nmhdr);

	// delete corresponding window object
	const CTemplateWnd* p_spike_element = reinterpret_cast<CTemplateWnd*>(p_nm_list_view->lParam);
	if (p_spike_element != nullptr)
	{
		// search corresponding window
		auto item = -1;
		for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
		{
			if (template_wnd_ptr_array_.GetAt(i) == p_spike_element)
			{
				item = i;
				break;
			}
		}
		delete p_spike_element;
		template_wnd_ptr_array_.RemoveAt(item);
		p_nm_list_view->lParam = NULL;
	}
	*p_result = 0;
}

void CTemplateListWnd::OnVScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar* p_scroll_bar)
{
	CListCtrl::OnVScroll(n_sb_code, n_pos, p_scroll_bar);
	RedrawItems(0, template_wnd_ptr_array_.GetSize() - 1);
}

void CTemplateListWnd::update_template_legends(LPCSTR psz_type)
{
	for (auto i = template_wnd_ptr_array_.GetSize() - 1; i >= 0; i--)
	{
		TCHAR sz_item[20];
		const auto p_tpl_wnd = get_template_wnd(i);
		p_tpl_wnd->m_cs_id = psz_type;

		wsprintf(sz_item, _T("%s%i n_spk=%i"), static_cast<LPCTSTR>(p_tpl_wnd->m_cs_id), p_tpl_wnd->m_class_id,
		         p_tpl_wnd->get_n_items());
		SetItemText(i, 0, sz_item);
	}
}

void CTemplateListWnd::update_template_base_class_id(const int i_new_lowest_class)
{
	// first, get the lowest template class ID
	if (template_wnd_ptr_array_.GetSize() < 1)
		return;
	const auto p_tpl_wnd = get_template_wnd(0);
	auto lowest_id = p_tpl_wnd->m_class_id;
	for (auto i = template_wnd_ptr_array_.GetSize() - 1; i >= 0; i--)
	{
		const auto p_tpl1_wnd = get_template_wnd(i);
		if (lowest_id > p_tpl1_wnd->m_class_id)
			lowest_id = p_tpl1_wnd->m_class_id;
	}
	// now change the id of each template and update its text
	const auto delta = lowest_id - i_new_lowest_class;
	for (auto i = template_wnd_ptr_array_.GetSize() - 1; i >= 0; i--)
	{
		TCHAR sz_item[20];
		const auto p_tpl2_wnd = get_template_wnd(i);
		p_tpl2_wnd->m_class_id = p_tpl2_wnd->m_class_id - delta;
		wsprintf(sz_item, _T("%s%i n_spk=%i"), static_cast<LPCTSTR>(p_tpl2_wnd->m_cs_id), p_tpl2_wnd->m_class_id,
		         p_tpl2_wnd->get_n_items());
		SetItemText(i, 0, sz_item);
	}
}

void CTemplateListWnd::set_template_class_id(const int item, const LPCTSTR psz_type, const int class_id)
{
	CString cs_item;
	const auto p_tpl_wnd = get_template_wnd(item);
	p_tpl_wnd->m_class_id = class_id;
	const CString cs = psz_type;
	if (!cs.IsEmpty())
		p_tpl_wnd->m_cs_id = cs;
	cs_item.Format(_T("%s%i n_spk=%i"), psz_type, class_id, p_tpl_wnd->get_n_items());
	SetItemText(item, 0, cs_item);
}

BOOL CTemplateListWnd::t_init(const int i) const
{
	const BOOL flag = (i >= 0) && (i < template_wnd_ptr_array_.GetSize());
	if (flag)
		get_template_wnd(i)->t_init();
	return flag;
}

BOOL CTemplateListWnd::t_add(int* p_source)
{
	m_tpl0_.t_add_spike_top_sum(p_source);
	return TRUE;
}

BOOL CTemplateListWnd::t_add(const int i, int* p_source) const
{
	const BOOL flag = (i >= 0) && (i < template_wnd_ptr_array_.GetSize());
	if (flag)
	{
		get_template_wnd(i)->t_add_spike_top_sum(p_source);
	}
	return flag;
}

BOOL CTemplateListWnd::t_power(const int i, double* power_of_sum) const
{
	const BOOL flag = (i >= 0) && (i < template_wnd_ptr_array_.GetSize());
	if (flag)
		*power_of_sum = get_template_wnd(i)->t_power_of_p_sum();
	return flag;
}

BOOL CTemplateListWnd::t_within(const int i, int* p_source) const
{
	BOOL flag = i >= 0 && (i < template_wnd_ptr_array_.GetSize());
	if (flag)
		flag = get_template_wnd(i)->t_get_number_of_points_within(p_source, &m_hit_rate_);
	return flag;
}

BOOL CTemplateListWnd::t_min_dist(const int i, int* p_source, int* offset_min, double* dist_min) const
{
	const BOOL flag = i >= 0 && (i < template_wnd_ptr_array_.GetSize());
	if (flag)
	{
		*dist_min = get_template_wnd(i)->t_min_dist(p_source, offset_min);
	}
	return flag;
}

void CTemplateListWnd::t_global_stats()
{
	m_tpl0_.t_global_stats(&m_global_std, &m_global_dist);
	for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
	{
		get_template_wnd(i)->set_global_std(&m_global_std);
	}
}

void CTemplateListWnd::set_template_length(const int spk_len, const int tp_left, const int tp_right)
{
	if (tp_left != m_tpl_left_ || tp_right != m_tpl_right_)
	{
		m_tpl_left_ = tp_left;
		m_tpl_right_ = tp_right;
	}

	const auto len = tp_right - tp_left + 1;
	m_tpl_len_ = spk_len;
	for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
		get_template_wnd(i)->set_template_length(spk_len, len, m_tpl_left_);
	m_tpl0_.set_template_length(spk_len, len, m_tpl_left_);
	m_tpl0_.t_init();
}

void CTemplateListWnd::set_hit_rate_tolerance(const int* p_hit_rate, const float* p_tolerance)
{
	m_hit_rate_ = *p_hit_rate;
	if (m_k_tolerance_ != *p_tolerance)
	{
		m_k_tolerance_ = *p_tolerance;
		for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
		{
			CTemplateWnd* p_template = get_template_wnd(i);
			p_template->set_k_tolerance(&m_k_tolerance_);
		}
		m_tpl0_.m_k_tolerance = m_k_tolerance_;
	}
}

void CTemplateListWnd::set_y_w_ext_org(int extent, int zero)
{
	m_y_extent_ = extent;
	m_y_zero_ = zero;
	for (auto i = 0; i < template_wnd_ptr_array_.GetSize(); i++)
	{
		CTemplateWnd* p_template = get_template_wnd(i);
		p_template->set_yw_ext_org(extent, zero);
	}
}

void CTemplateListWnd::on_begin_drag(NMHDR* p_nmhdr, LRESULT* p_result)
{
	CPoint pt_item, pt_image;
	const auto* pnm_list_view = reinterpret_cast<NM_LISTVIEW*>(p_nmhdr);

	ASSERT(!m_b_dragging_);
	m_b_dragging_ = TRUE;
	m_i_item_drag_ = pnm_list_view->iItem;

	CPoint ptAction = pnm_list_view->ptAction;
	GetItemPosition(m_i_item_drag_, &pt_item); // ptItem is relative to (0,0) and not the view origin
	GetOrigin(&m_pt_origin_);

	// Update image list to make sure all images are loaded
	const auto p_i = GetImageList(LVSIL_NORMAL);
	const auto image_count = p_i->GetImageCount();
	if (image_count == 0 || m_i_item_drag_ > image_count + 1)
	{
		auto i_first = 0;
		if (m_i_item_drag_ > image_count + 1)
			i_first = image_count;
		// CImageList
		auto h_i = p_i->m_hImageList;
		//for (int i=ifirst; i<m_ptpl.GetRectSize(); i++)
		//{
		//	//int ii = ImageList_Add(hI, GetTemplateWnd(i)->GetBitmapPlotHandle(), NULL);
		//	int ii = ImageList_Add(hI, NULL, NULL);
		//	ASSERT(ii != -1);
		//}
	}

	ASSERT(m_p_image_list_drag_ == NULL);
	m_p_image_list_drag_ = CreateDragImage(m_i_item_drag_, &pt_image);

	m_size_delta_ = ptAction - pt_image; // difference between cursor pos and image pos
	m_pt_hot_spot_ = ptAction - pt_item + m_pt_origin_; // calculate hotspot for the cursor
	CImageList::DragShowNolock(TRUE); // lock updates and show drag image
	m_p_image_list_drag_->SetDragCursorImage(0, m_pt_hot_spot_); // define the hot spot for the new cursor image
	m_p_image_list_drag_->BeginDrag(0, CPoint(0, 0));
	ptAction -= m_size_delta_;
	CImageList::DragEnter(this, ptAction);
	CImageList::DragMove(ptAction); // move image to overlap original icon

	//m_p_image_List_Drag->Drag_Show_No_lock(TRUE);  // lock updates and show drag image
	//m_p_image_List_Drag->Set_Drag_Cursor_Image(0, m_ptHotSpot);  // define the hot spot for the new cursor image
	//m_p_image_List_Drag->Begin_Drag(0, CPoint(0, 0));
	//pt_Action -= m_sizeDelta;
	//m_p_image_List_Drag->Drag_Enter(this, ptAction);
	//m_p_image_List_Drag->Drag_Move(ptAction);  // move image to overlap original icon

	SetCapture();
}

void CTemplateListWnd::OnMouseMove(const UINT n_flags, CPoint point)
{
	LV_ITEM lv_item;

	auto l_style = GetWindowLong(m_hWnd, GWL_STYLE);
	l_style &= LVS_TYPEMASK; // drag will do different things in list and report mode
	if (m_b_dragging_)
	{
		int i_item;
		ASSERT(m_p_image_list_drag_ != NULL);
		CImageList::DragMove(point - m_size_delta_); // move the image
		//m_p_image_Lis_tDrag->DragMove(point - m_sizeDelta);  // move the image

		if ((i_item = HitTest(point)) != -1)
		{
			m_i_item_drop_ = i_item;
			CImageList::DragLeave(this); // unlock the window and hide drag image
			//m_p_image_List_Drag->DragLeave(this); // unlock the window and hide drag image
			if (l_style == LVS_REPORT || l_style == LVS_LIST)
			{
				lv_item.iItem = i_item;
				lv_item.iSubItem = 0;
				lv_item.mask = LVIF_STATE;
				lv_item.stateMask = LVIS_DROPHILITED; // highlight the drop target
				SetItem(&lv_item);
			}
			point -= m_size_delta_;
			CImageList::DragEnter(this, point); // lock updates and show drag image
			//m_p_image_List_Drag->DragEnter(this, point);  // lock updates and show drag image
		}
	}
	CListCtrl::OnMouseMove(n_flags, point);
}

void CTemplateListWnd::OnButtonUp(CPoint point)
{
	if (m_b_dragging_) // end of the drag operation
	{
		CString cs_str;

		const auto l_style = GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK;
		m_b_dragging_ = FALSE;

		ASSERT(m_p_image_list_drag_ != NULL);
		CImageList::DragLeave(this);
		CImageList::EndDrag();
		delete m_p_image_list_drag_;
		m_p_image_list_drag_ = nullptr;

		if (l_style == LVS_REPORT && m_i_item_drop_ != m_i_item_drag_)
		{
			cs_str = GetItemText(m_i_item_drag_, 0);
			SetItemText(m_i_item_drop_, 1, cs_str);
		}
		else if (l_style == LVS_LIST && m_i_item_drop_ != m_i_item_drag_)
		{
			cs_str = GetItemText(m_i_item_drop_, 0);
			cs_str += _T("**");
			SetItemText(m_i_item_drop_, 0, cs_str);
		}
		else if (l_style == LVS_ICON || l_style == LVS_SMALLICON)
		{
			point -= m_pt_hot_spot_;
			point += m_pt_origin_;
			SetItemPosition(m_i_item_drag_, point);
		}
		ReleaseCapture();
	}
}

void CTemplateListWnd::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	OnButtonUp(point);
	CListCtrl::OnLButtonUp(n_flags, point);
}

void CTemplateListWnd::OnRButtonDown(const UINT n_flags, const CPoint point)
{
	LVFINDINFO lv_info;
	lv_info.flags = LVFI_NEARESTXY;
	lv_info.pt = point;
	const auto item = FindItem(&lv_info, -1);

	CListCtrl::OnRButtonDown(n_flags, point);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_R_MOUSE_BUTTON_DOWN, MAKELONG(item, GetDlgCtrlID()));
}
