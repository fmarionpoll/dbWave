#include "StdAfx.h"

#include "MainFrm.h"
#include "resource.h"
#include "PaneldbFilter.h"

#include "dbWave.h"
#include "dbWave_constants.h"
#include "DlgProgress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameters
int PaneldbFilter::m_no_col_[] = {
	CH_EXPT_ID,
	CH_IDINSECT,
	CH_IDSENSILLUM,
	CH_INSECT_ID,
	CH_SENSILLUM_ID,
	CH_LOCATION_ID,
	CH_STRAIN_ID,
	CH_SEX_ID,
	CH_OPERATOR_ID,
	CH_STIM_ID,
	CH_CONC_ID,
	CH_REPEAT,
	CH_STIM2_ID,
	CH_CONC2_ID,
	CH_REPEAT2,
	CH_FLAG,
	CH_ACQDATE_DAY,
	-1
};

PaneldbFilter::PaneldbFilter()
= default;

PaneldbFilter::~PaneldbFilter()
= default;

BEGIN_MESSAGE_MAP(PaneldbFilter, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()

	ON_MESSAGE(WM_MYMESSAGE, on_my_message)

	ON_CBN_SELCHANGE(ID_RECORD_SORT, on_cbn_sel_change_category)
	ON_COMMAND(ID_RECORD_SORT, on_record_sort)
	ON_COMMAND(ID_UPDATE, on_update_tree)
	ON_COMMAND(ID_APPLY_FILTER, on_apply_filter)

	ON_COMMAND(ID_BUTTON_PREVIOUS, select_previous_combo_item)
	ON_COMMAND(ID_BUTTON_NEXT, select_next_combo_item)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PREVIOUS, on_update_bn_update_previous)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_NEXT, on_update_bn_update_next)

	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, on_tvn_sel_changed_filter_tree)
	ON_NOTIFY(NM_CLICK, IDC_TREE1, on_nm_click_filter_tree)

	
END_MESSAGE_MAP()

void PaneldbFilter::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
		return;

	CRect rect_client;
	GetClientRect(rect_client);

	const int cy_tlb = m_wnd_tool_bar_.CalcFixedLayout(FALSE, TRUE).cy;
	m_wnd_tool_bar_.SetWindowPos(nullptr, rect_client.left, 
							rect_client.top,
							rect_client.Width(), 
							cy_tlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wnd_filter_view_.SetWindowPos(nullptr, rect_client.left + 1, 
							rect_client.top + cy_tlb + 1,
							rect_client.Width() - 2, 
							rect_client.Height() - cy_tlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

int PaneldbFilter::OnCreate(const LPCREATESTRUCT lp_create_struct)
{
	if (CDockablePane::OnCreate(lp_create_struct) == -1)
		return -1;

	CRect rect_dummy;
	rect_dummy.SetRectEmpty();

	// Create view for filter items
	constexpr DWORD dw_view_style = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	if (!m_wnd_filter_view_.Create(dw_view_style, rect_dummy, this, IDC_TREE1))
		return -1;

	// Create toolbar with a combobox
	m_wnd_tool_bar_.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wnd_tool_bar_.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	m_wnd_tool_bar_.SetPaneStyle(m_wnd_tool_bar_.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wnd_tool_bar_.SetPaneStyle(
		m_wnd_tool_bar_.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wnd_tool_bar_.ReplaceButton(ID_RECORD_SORT,
		CMFCToolBarComboBoxButton(ID_RECORD_SORT, /*GetCmdMgr()->GetCmdImage(ID_RECORD_SORT)*/ NULL, CBS_DROPDOWN));

	// All commands will be routed via this control, not via the parent frame:
	m_wnd_tool_bar_.SetOwner(this);
	m_wnd_tool_bar_.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void PaneldbFilter::OnSize(const UINT n_type, const int cx, const int cy)
{
	CDockablePane::OnSize(n_type, cx, cy);
	AdjustLayout();
}

void PaneldbFilter::OnContextMenu(CWnd* p_wnd, const CPoint point)
{
	const auto p_wnd_tree = static_cast<CTreeCtrl*>(&m_wnd_filter_view_);
	ASSERT_VALID(p_wnd_tree);

	if (p_wnd != p_wnd_tree)
	{
		CDockablePane::OnContextMenu(p_wnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		auto pt_tree = point;
		p_wnd_tree->ScreenToClient(&pt_tree);

		UINT flags = 0;
		const auto h_tree_item = p_wnd_tree->HitTest(pt_tree, &flags);
		if (h_tree_item != nullptr)
		{
			p_wnd_tree->SelectItem(h_tree_item);
		}
	}

	p_wnd_tree->SetFocus();
	the_app.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void PaneldbFilter::OnPaint()
{
	CPaintDC dc(this);
	CRect rect_tree;
	m_wnd_filter_view_.GetWindowRect(rect_tree);
	ScreenToClient(rect_tree);

	rect_tree.InflateRect(1, 1);
	dc.Draw3dRect(rect_tree, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
}

void PaneldbFilter::OnSetFocus(CWnd* p_old_wnd)
{
	CDockablePane::OnSetFocus(p_old_wnd);
	m_wnd_filter_view_.SetFocus();
}

void PaneldbFilter::on_update_tree()
{
	m_p_doc_old_ = nullptr;
	init_filter_list();
}

LRESULT PaneldbFilter::on_my_message(const WPARAM w_param, const LPARAM l_param)
{
	//auto p_app = (CdbWaveApp*)AfxGetApp();
	//short low_p = LO_WORD(lParam);
	//short high_p = HI_WORD(lParam);

	switch (w_param)
	{
	case HINT_ACTIVATE_VIEW:
		m_p_doc_ = reinterpret_cast<CdbWaveDoc*>(l_param);
		if (m_p_doc_ != m_p_doc_old_)
			init_filter_list();
		break;

	case HINT_MDI_ACTIVATE:
		{
			const auto* p_main = static_cast<CMainFrame*>(AfxGetMainWnd());
			BOOL b_maximized;
			const auto p_child = p_main->MDIGetActive(&b_maximized);
			if (!p_child)
				return NULL;
			const auto p_document = p_child->GetActiveDocument();
			if (!p_document || !p_document->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)))
				return NULL;
			m_p_doc_ = static_cast<CdbWaveDoc*>(p_document);
			init_filter_list();
		}
		break;

	default:
		break;
	}
	return 0L;
}

void PaneldbFilter::OnUpdate(CView* p_sender, const LPARAM l_hint, CObject* p_hint)
{
	m_p_doc_ = reinterpret_cast<CdbWaveDoc*>(p_sender);
	switch (LOWORD(l_hint))
	{
	case HINT_CLOSE_FILE_MODIFIED:
		break;

	case HINT_REQUERY:
		m_p_doc_old_ = nullptr;

	case HINT_DOC_HAS_CHANGED:
	case HINT_DOC_MOVE_RECORD:
	case HINT_REPLACE_VIEW:
	default:
		init_filter_list();
		break;
	}
}

void PaneldbFilter::fill_combo_with_categories(const CdbTable* p_db) const
{
	const auto p_combo = m_wnd_tool_bar_.get_combo();
	ASSERT(ID_RECORD_SORT == m_wnd_tool_bar_.GetItemID(3));

	if (p_combo->GetCount() <= 0)
	{
		for (auto i = 0; i < N_TABLE_COLUMNS; i++)
			p_combo->AddSortedItem(CdbTable::m_column_properties[i].description, i);
	}
	p_combo->SelectItem(p_db->m_main_table_set.m_strSort);

}

DB_ITEMDESC* PaneldbFilter::create_tree_category(CdbTable* p_db, const int i) 
{
	const auto i_col = m_no_col_[i];
	m_h_tree_item_[i] = m_wnd_filter_view_.InsertItem(CdbTable::m_column_properties[i_col].description, TVI_ROOT);
	m_wnd_filter_view_.SetItemData(m_h_tree_item_[i], m_no_col_[i]);
	const auto p_desc = p_db->get_record_item_descriptor(i_col);
	// collect data (array of unique descriptors)
	switch (p_desc->data_code_number)
	{
	case FIELD_IND_TEXT:
	case FIELD_IND_FILEPATH:
		populate_item_from_linked_table(p_desc);
		break;
	case FIELD_LONG:
		populate_item_from_table_long(p_desc);
		break;
	case FIELD_DATE_YMD:
		populate_item_from_table_with_date(p_desc);
		break;
	default:
		break;
	}

	return p_desc;
}

HTREEITEM PaneldbFilter::create_tree_subitem_element(const DB_ITEMDESC* p_desc, const int i, const int j)
{
	const HTREEITEM h_tree_item = m_wnd_filter_view_.InsertItem(p_desc->cs_elements_array.GetAt(j), m_h_tree_item_[i]);
	TVCS_CHECKSTATE b_check = TVCS_CHECKED;
	const auto cs_element_j = p_desc->cs_elements_array.GetAt(j);
	if (p_desc->b_array_filter)
	{
		b_check = TVCS_UNCHECKED;
		for (auto k = 0; k < p_desc->cs_array_filter.GetSize(); k++)
		{
			if (cs_element_j.CompareNoCase(p_desc->cs_array_filter.GetAt(k)) == 0)
			{
				b_check = TVCS_CHECKED;
				break;
			}
		}
	}
	else if (p_desc->b_single_filter)
	{
		if (cs_element_j.CompareNoCase(p_desc->cs_param_single_filter) != 0)
			b_check = TVCS_UNCHECKED;
	}
	m_wnd_filter_view_.set_check(h_tree_item, b_check);
	return h_tree_item;
}

void PaneldbFilter::create_tree_subitem(const DB_ITEMDESC* p_desc, const int i)
{
	// create subitems
	auto i_sum = 0;
	auto n_items = 0;
	HTREEITEM h_tree_item = nullptr;
	TVCS_CHECKSTATE b_check = TVCS_UNCHECKED;
	for (auto j = 0; j < p_desc->cs_elements_array.GetSize(); j++)
	{
		h_tree_item = create_tree_subitem_element(p_desc, i, j);
		b_check = m_wnd_filter_view_.get_check(h_tree_item);
		i_sum += b_check; // count number of positive checks (no check=0, check = 1)
		n_items++;
	}

	// trick needed here because if the first item is checked and not the others,
	// then the parent stays in the initial state
	// namely "checked" (because at that moment it did not have other children)
	if (i_sum == 1 && h_tree_item != nullptr)
	{
		m_wnd_filter_view_.set_check(h_tree_item, TVCS_CHECKED);
		m_wnd_filter_view_.set_check(h_tree_item, b_check);
	}
	if (i_sum < n_items)
		m_wnd_filter_view_.Expand(m_h_tree_item_[i], TVE_EXPAND);
}

void PaneldbFilter::init_filter_list()
{
	if (m_p_doc_old_ == m_p_doc_)
		return;

	m_p_doc_old_ = m_p_doc_;

	const auto p_db = m_p_doc_->db_table;
	ASSERT(p_db);

	// get all categories into combo
	fill_combo_with_categories(p_db);

	// fill items of the tree
	if (p_db->m_main_table_set.IsBOF() && p_db->m_main_table_set.IsEOF())
		return;

	m_wnd_filter_view_.LockWindowUpdate();
	m_wnd_filter_view_.DeleteAllItems();

	p_db->m_main_table_set.build_and_sort_id_arrays();
	auto i = 0;
	while (m_no_col_[i] > 0)
	{
		const auto* p_desc = create_tree_category(p_db, i);
		create_tree_subitem(p_desc, i);
		i++;
	}
	m_wnd_filter_view_.UnlockWindowUpdate();
}

void PaneldbFilter::populate_item_from_table_long(DB_ITEMDESC* p_desc) const
{
	const auto p_set = &m_p_doc_->db_table->m_main_table_set;
	const auto cs_col_head = p_desc->header_name;
	const auto array_size = p_desc->li_array.GetSize();
	if (p_desc->b_array_filter)
	{
		return;
	}
	if (p_desc->b_single_filter)
	{
		p_desc->cs_param_single_filter.Format(_T("%i"), p_desc->l_param_single_filter);
	}
	else
	{
		CString str;
		CString cs;
		p_desc->cs_elements_array.RemoveAll();
		for (auto i = 0; i < array_size; i++)
		{
			const auto i_id = p_desc->li_array.GetAt(i);
			// add string only if found into p_main_table_set...
			str.Format(_T("%s=%li"), (LPCTSTR)cs_col_head, i_id);
			const auto flag = p_set->FindFirst(str);
			if (flag != 0)
			{
				cs.Format(_T("%i"), i_id);
				p_desc->cs_elements_array.Add(cs);
				if (p_desc->b_single_filter && p_desc->l_param_single_filter != i_id)
				{
					p_desc->cs_param_single_filter.Format(_T("%i"), i_id);
				}
			}
		}
	}
}

void PaneldbFilter::populate_item_from_linked_table(DB_ITEMDESC* p_desc) const
{
	auto str2 = p_desc->header_name;
	ASSERT(!str2.IsEmpty());

	auto p_linked_set = p_desc->p_linked_set;
	auto p_set = &m_p_doc_->db_table->m_main_table_set;
	if (p_desc->b_array_filter)
		return;

	if (p_desc->b_single_filter)
	{
		p_desc->cs_param_single_filter = p_linked_set->get_string_from_id(p_desc->l_param_single_filter);
	}
	else
	{
		// loop over the whole content of the attached table
		p_desc->cs_elements_array.RemoveAll();
		p_desc->li_array.RemoveAll();
		if (p_linked_set->IsOpen() && !p_linked_set->IsBOF())
		{
			CString cs;
			COleVariant var_value0, var_value1;
			p_linked_set->MoveFirst();
			while (!p_linked_set->IsEOF())
			{
				p_linked_set->GetFieldValue(0, var_value0);
				p_linked_set->GetFieldValue(1, var_value1);
				const auto i_id = var_value1.lVal;
				// add string only if found into p_main_table_set...
				cs.Format(_T("%s=%li"), (LPCTSTR)p_desc->header_name, i_id);
				const auto flag = p_set->FindFirst(cs);
				if (flag != 0)
				{
					insert_alphabetic(var_value0.bstrVal, p_desc->cs_elements_array);
					p_desc->li_array.Add(i_id);
				}
				p_linked_set->MoveNext();
			}
		}
	}
}

void PaneldbFilter::populate_item_from_table_with_date(DB_ITEMDESC* p_desc) const
{
	CString cs; // to construct date
	const auto cs_column_head = p_desc->header_name;
	const auto p_main_table_set = &m_p_doc_->db_table->m_main_table_set;
	const auto array_size = p_main_table_set->m_desc[CH_ACQDATE_DAY].ti_array.GetSize();

	if (p_desc->b_array_filter)
	{
		return;
	}
	if (p_desc->b_single_filter)
	{
		cs = p_desc->date_time_param_single_filter.Format(VAR_DATEVALUEONLY);
		p_desc->cs_param_single_filter = cs;
	}
	else
	{
		CString str;
		p_desc->cs_elements_array.RemoveAll();
		for (auto i = 0; i < array_size; i++)
		{
			auto o_time = p_main_table_set->m_desc[CH_ACQDATE_DAY].ti_array.GetAt(i);
			cs = o_time.Format(_T("%m/%d/%y")); // filter needs to be constructed as month-day-year
			str.Format(_T("%s=#%s#"), (LPCTSTR)cs_column_head, (LPCTSTR)cs);
			const auto flag = p_main_table_set->FindFirst(str);
			if (flag != 0) // add string only if found into p_main_table_set...
			{
				cs = o_time.Format(VAR_DATEVALUEONLY);
				p_desc->cs_elements_array.Add(cs);
			}
		}
	}
}

void PaneldbFilter::insert_alphabetic(const CString& cs, CStringArray& cs_array)
{
	auto k = 0;
	for (auto i = 0; i < cs_array.GetSize(); i++, k++)
	{
		const auto& cs_comp = cs_array.GetAt(k);
		const auto j = cs.CompareNoCase(cs_comp);
		if (j < 0)
			break;
	}
	cs_array.InsertAt(k, cs);
}

void PaneldbFilter::build_filter_item_indirection_from_tree(DB_ITEMDESC* p_desc, const HTREEITEM start_item) const
{
	auto i = 0;
	for (auto item = start_item; item != nullptr; item = m_wnd_filter_view_.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wnd_filter_view_.get_check(item);
		if (state == TVCS_CHECKED)
		{
			auto cs = m_wnd_filter_view_.GetItemText(item);
			for (auto j = 0; j < p_desc->li_array.GetSize(); j++)
			{
				const auto li = p_desc->li_array.GetAt(j);
				auto str = p_desc->p_linked_set->get_string_from_id(li);
				if (str == cs)
				{
					p_desc->l_param_filter_array.Add(li);
					p_desc->cs_array_filter.Add(cs);
					break;
				}
			}
		}
	}
}

void PaneldbFilter::build_filter_item_long_from_tree(DB_ITEMDESC* p_desc, const HTREEITEM start_item) const
{
	auto i = 0;
	for (auto item = start_item; item != nullptr; item = m_wnd_filter_view_.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wnd_filter_view_.get_check(item);
		if (state == TVCS_CHECKED)
		{
			auto cs = m_wnd_filter_view_.GetItemText(item);
			const auto li = p_desc->li_array.GetAt(i);
			p_desc->l_param_filter_array.Add(li);
			p_desc->cs_array_filter.Add(cs);
		}
	}
}

void PaneldbFilter::build_filter_item_date_from_tree(DB_ITEMDESC* p_desc, const HTREEITEM start_item) const
{
	auto i = 0;
	for (auto item = start_item; item != nullptr; item = m_wnd_filter_view_.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wnd_filter_view_.get_check(item);
		if (state == TVCS_CHECKED)
		{
			auto cs_filter_checked = m_wnd_filter_view_.GetItemText(item);
			COleDateTime o_time;
			o_time.ParseDateTime(cs_filter_checked);
			p_desc->data_time_array_filter.Add(o_time);
			p_desc->cs_array_filter.Add(cs_filter_checked);
		}
	}
}

void PaneldbFilter::on_apply_filter()
{
	if (!m_p_doc_)
		return;
	const auto p_db = m_p_doc_->db_table;

	auto i = 0;
	while (m_no_col_[i] > 0)
	{
		const auto h_parent = m_h_tree_item_[i];
		i++;

		const int i_col = static_cast<int>(m_wnd_filter_view_.GetItemData(h_parent));
		const auto p_desc = p_db->get_record_item_descriptor(i_col);

		//if root is checked (or unchecked), it means no item is selected - remove flag
		const auto state_root = m_wnd_filter_view_.get_check(h_parent);
		if ((state_root == TVCS_CHECKED) || (state_root == TVCS_UNCHECKED))
		{
			p_desc->b_array_filter = FALSE;
		}
		// else if foot is un-determinate build filter
		else
		{
			p_desc->b_array_filter = TRUE;
			p_desc->l_param_filter_array.RemoveAll();
			p_desc->cs_array_filter.RemoveAll();
			p_desc->data_time_array_filter.RemoveAll();
			const auto start_item = m_wnd_filter_view_.GetNextItem(h_parent, TVGN_CHILD);
			switch (p_desc->data_code_number)
			{
			case FIELD_IND_TEXT:
			case FIELD_IND_FILEPATH:
				build_filter_item_indirection_from_tree(p_desc, start_item);
				break;
			case FIELD_LONG:
				build_filter_item_long_from_tree(p_desc, start_item);
				break;
			case FIELD_DATE_YMD:
				build_filter_item_date_from_tree(p_desc, start_item);
				break;
			default:
				ASSERT(false);
				break;
			}
		}
	}

	// update recordset and tell other views...
	p_db->m_main_table_set.build_filters();
	p_db->m_main_table_set.refresh_query();
	m_p_doc_->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
}

//void PaneldbFilter::on_sort_records()
//{
//	const auto p_database = m_p_doc_->db_table;
//	ASSERT(p_database);
//	const auto p_combo = static_cast<CMFCToolBarComboBoxButton*>(m_wnd_tool_bar_.GetButton(3));
//	ASSERT(ID_RECORD_SORT == m_wnd_tool_bar_.GetItemID(3));
//
//	const auto i_sel = p_combo->GetCurSel();
//	ASSERT(i_sel != CB_ERR);
//	const int i = p_combo->GetItemData(i_sel);
//	p_database->m_main_table_set.m_strSort = CdbTable::m_column_properties[i].header_name;
//
//	p_database->m_main_table_set.refresh_query();
//	m_p_doc_->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
//}

void PaneldbFilter::on_tvn_sel_changed_filter_tree(NMHDR* p_nmhdr, LRESULT* p_result)
{
	
	NM_TREEVIEW* p_nm_tree_view = (NM_TREEVIEW*) p_nmhdr;
	auto* p_new_item = &p_nm_tree_view->itemNew;
	TRACE("changed filter \n");
}

void PaneldbFilter::on_nm_click_filter_tree(NMHDR* p_nmhdr, LRESULT* p_result)
{
	//const auto i_cur_sel = spk_list_tab_ctrl.GetCurSel();
	//SendMessage(WM_MYMESSAGE, HINT_VIEW_TAB_CHANGE, MAKELPARAM(i_cur_sel, 0));
	//*p_result = 0;
	TRACE("click filter \n");
}

void  PaneldbFilter::on_cbn_sel_change_category()
{
	TRACE("change category from combobox \n");
}

void PaneldbFilter::on_update_bn_update_previous(CCmdUI* p_cmd_ui)
{
	//p_cmd_ui->Enable(m_b_changed_property_);
	p_cmd_ui->Enable(TRUE);
}

void PaneldbFilter::on_update_bn_update_next(CCmdUI* p_cmd_ui)
{
	//p_cmd_ui->Enable(m_b_changed_property_);
	p_cmd_ui->Enable(TRUE);
}

void PaneldbFilter::select_previous_combo_item()
{
	TRACE("got to previous");
	select_next_filter_item(false);
}

void PaneldbFilter::select_next_combo_item()
{
	TRACE("go to next");
	select_next_filter_item(true);
}

void PaneldbFilter::select_next_filter_item(boolean b_next)
{
	// get combo selected item
	const int index = m_wnd_tool_bar_.get_combo()->GetCurSel();
	const CString description = m_wnd_tool_bar_.get_combo()->GetItem(index);
	const int column_index = static_cast<int>(m_wnd_tool_bar_.get_combo()->GetItemData(index));
	// select corresponding tree item
	auto h_item = select_tree_item(column_index);
	const auto p_tree = static_cast<CTreeCtrl*>(&m_wnd_filter_view_);
	ASSERT_VALID(p_tree);
	p_tree->SelectItem(h_item);
	select_next(b_next);
}

HTREEITEM PaneldbFilter::select_tree_item(const int col_requested)
{
	auto i = 0;
	HTREEITEM item_found = nullptr;
	const auto p_db = m_p_doc_->db_table;
	const auto p_desc = p_db->get_record_item_descriptor(col_requested);

	while (m_no_col_[i] > 0)
	{
		const auto h_parent = m_h_tree_item_[i];
		const int i_col = static_cast<int>(m_wnd_filter_view_.GetItemData(h_parent));
		if (i_col == col_requested)
		{
			item_found = h_parent;
			break;
		}
		i++;
	}
	return item_found;
}


void PaneldbFilter::select_next(const boolean b_next)
{
	const auto p_tree = static_cast<CTreeCtrl*>(&m_wnd_filter_view_);
	ASSERT_VALID(p_tree);

	auto h_item = p_tree->GetSelectedItem();
	if (!p_tree->ItemHasChildren(h_item))
		h_item = p_tree->GetParentItem(h_item);
	if (h_item == nullptr)
		return;

	auto count = 0;
	HTREEITEM h_last_selected = nullptr;
	auto n_selected = 0;
	auto h_kid = p_tree->GetChildItem(h_item);
	do
	{
		const auto state = static_cast<CQuadStateTree*>(p_tree)->get_check(h_kid);
		if (state == TVCS_CHECKED)
		{
			h_last_selected = h_kid;
			n_selected++;
		}
		count++;
	} while ((h_kid = p_tree->GetNextSiblingItem(h_kid)));

	// if single selection select next item on the list and deselect current; update
	if (n_selected == 1)
	{
		HTREEITEM h_next;
		if (b_next)
			h_next = p_tree->GetNextSiblingItem(h_last_selected);
		else
			h_next = p_tree->GetPrevSiblingItem(h_last_selected);
		if (h_next == nullptr)
			return;
		static_cast<CQuadStateTree*>(p_tree)->set_check(h_next, TVCS_CHECKED);
		static_cast<CQuadStateTree*>(p_tree)->set_check(h_last_selected, TVCS_UNCHECKED);
		on_apply_filter();
	}
}

void PaneldbFilter::on_record_sort()
{
	const auto p_database = m_p_doc_->db_table;
	ASSERT(p_database);
	const auto p_combo = static_cast<CMFCToolBarComboBoxButton*>(m_wnd_tool_bar_.GetButton(3));
	ASSERT(ID_RECORD_SORT == m_wnd_tool_bar_.GetItemID(3));

	const auto i_sel = p_combo->GetCurSel();
	ASSERT(i_sel != CB_ERR);
	const int i = p_combo->GetItemData(i_sel);
	//p_database->m_main_table_set.m_strSort = CdbTable::m_column_properties[i].header_name;

	//p_database->m_main_table_set.refresh_query();
	//m_p_doc_->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
}
