#include "StdAfx.h"

#include "MainFrm.h"
#include "resource.h"
#include "PanelFilter.h"

#include "dbWave.h"
#include "dbWave_constants.h"
#include "DlgProgress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameters
int CFilterPanel::m_no_col_[] = {
	CH_EXPT_ID,
	CH_IDINSECT, CH_IDSENSILLUM, CH_INSECT_ID, CH_SENSILLUM_ID,
	CH_LOCATION_ID, CH_STRAIN_ID, CH_SEX_ID, CH_OPERATOR_ID,
	CH_STIM_ID, CH_CONC_ID, CH_REPEAT, CH_STIM2_ID, CH_CONC2_ID, CH_REPEAT2,
	CH_FLAG, CH_ACQDATE_DAY, -1
};

CFilterPanel::CFilterPanel()
= default;

CFilterPanel::~CFilterPanel()
= default;

BEGIN_MESSAGE_MAP(CFilterPanel, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_UPDATE, OnUpdateTree)
	ON_COMMAND(ID_APPLY_FILTER, OnApplyFilter)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)
	ON_COMMAND(ID_RECORD_SORT, OnSortRecords)
	ON_COMMAND(ID_EXPLORER_NEXT, OnSelectNext)
	ON_COMMAND(ID_EXPLORER_PREVIOUS, OnSelectPrevious)

END_MESSAGE_MAP()

int CFilterPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect_dummy;
	rect_dummy.SetRectEmpty();

	// Create view:
	constexpr DWORD dw_view_style = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	if (!m_wnd_filter_view_.Create(dw_view_style, rect_dummy, this, IDC_TREE1))
		return -1; // fail to create

	m_wnd_tool_bar_.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wnd_tool_bar_.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);
	m_wnd_tool_bar_.SetPaneStyle(m_wnd_tool_bar_.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wnd_tool_bar_.SetPaneStyle(
		m_wnd_tool_bar_.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wnd_tool_bar_.ReplaceButton(
		ID_RECORD_SORT,
		CMFCToolBarComboBoxButton(ID_RECORD_SORT, /*GetCmdMgr()->GetCmdImage(ID_RECORD_SORT)*/ NULL, CBS_DROPDOWN));

	m_wnd_tool_bar_.SetOwner(this);
	m_wnd_tool_bar_.SetRouteCommandsViaFrame(FALSE);
	// All commands will be routed via this control , not via the parent frame:

	AdjustLayout();
	return 0;
}

void CFilterPanel::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFilterPanel::OnContextMenu(CWnd* p_wnd, CPoint point)
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

void CFilterPanel::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
		return;

	CRect rect_client;
	GetClientRect(rect_client);

	const int cy_tlb = m_wnd_tool_bar_.CalcFixedLayout(FALSE, TRUE).cy;
	m_wnd_tool_bar_.SetWindowPos(nullptr, rect_client.left, rect_client.top, 
		rect_client.Width(), cy_tlb,SWP_NOACTIVATE | SWP_NOZORDER);
	m_wnd_filter_view_.SetWindowPos(nullptr, rect_client.left + 1, rect_client.top + cy_tlb + 1,
		rect_client.Width() - 2,rect_client.Height() - cy_tlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFilterPanel::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect_tree;
	m_wnd_filter_view_.GetWindowRect(rect_tree);
	ScreenToClient(rect_tree);

	rect_tree.InflateRect(1, 1);
	dc.Draw3dRect(rect_tree, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
}

void CFilterPanel::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wnd_filter_view_.SetFocus();
}

void CFilterPanel::OnUpdateTree()
{
	m_p_doc_old_ = nullptr;
	InitFilterList();
}

LRESULT CFilterPanel::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	//auto p_app = (CdbWaveApp*)AfxGetApp();
	//short low_p = LO_WORD(lParam);
	//short high_p = HI_WORD(lParam);

	switch (wParam)
	{
	case HINT_ACTIVATE_VIEW:
		m_p_doc_ = reinterpret_cast<CdbWaveDoc*>(lParam);
		if (m_p_doc_ != m_p_doc_old_)
			InitFilterList();
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
			InitFilterList();
		}
		break;

	default:
		break;
	}
	return 0L;
}

void CFilterPanel::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	m_p_doc_ = reinterpret_cast<CdbWaveDoc*>(pSender);
	switch (LOWORD(lHint))
	{
	case HINT_CLOSE_FILE_MODIFIED: // save current file parameters
		//m_p_doc_old_ = NULL;
		break;

	case HINT_REQUERY:
		m_p_doc_old_ = nullptr;

	case HINT_DOC_HAS_CHANGED:
	case HINT_DOC_MOVE_RECORD:
	case HINT_REPLACE_VIEW:
	default:
		InitFilterList();
		break;
	}
}

void CFilterPanel::InitFilterList()
{
	if (m_p_doc_old_ == m_p_doc_)
		return;
	m_p_doc_old_ = m_p_doc_;

	const auto p_db = m_p_doc_->db_table;
	ASSERT(p_db);

	// setup dialog box
	DlgProgress dlg;
	dlg.Create();
	dlg.SetStep(1);

	// fill items of the combo (column heads to sort data)
	dlg.SetStatus(_T("List categories available..."));

	const auto p_combo = static_cast<CMFCToolBarComboBoxButton*>(m_wnd_tool_bar_.GetButton(3));
	ASSERT(ID_RECORD_SORT == m_wnd_tool_bar_.GetItemID(3));
	if (p_combo->GetCount() <= 0)
	{
		for (auto i = 0; i < N_TABLE_COLUMNS; i++)
			p_combo->AddSortedItem(CdbTable::m_column_properties[i].description, i);
	}
	p_combo->SelectItem(p_db->m_mainTableSet.m_strSort);

	// fill items of the tree
	dlg.SetStatus(_T("Populate categories..."));
	if (p_db->m_mainTableSet.IsBOF() && p_db->m_mainTableSet.IsEOF())
		return;
	m_wnd_filter_view_.LockWindowUpdate(); // prevent screen update (and flicker)
	if (m_wnd_filter_view_.GetCount() > 0)
		m_wnd_filter_view_.DeleteAllItems();

	auto i = 0;
	p_db->m_mainTableSet.BuildAndSortIDArrays();

	CString cs_comment;
	while (m_no_col_[i] > 0)
	{
		const auto i_col = m_no_col_[i];
		const auto p_desc = p_db->get_record_item_descriptor(i_col);
		m_h_tree_item_[i] = m_wnd_filter_view_.InsertItem(CdbTable::m_column_properties[i_col].description, TVI_ROOT); 
		m_wnd_filter_view_.SetItemData(m_h_tree_item_[i], m_no_col_[i]); // save table index into head of the list

		cs_comment.Format(_T("Category %i: "), i);
		cs_comment += CdbTable::m_column_properties[i_col].description;
		dlg.SetStatus(cs_comment);

		// collect data (array of unique descriptors)
		switch (p_desc->data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			PopulateItemFromLinkedTable(p_desc);
			break;
		case FIELD_LONG:
			PopulateItemFromTableLong(p_desc);
			break;
		case FIELD_DATE_YMD:
			PopulateItemFromTablewithDate(p_desc);
			break;
		default:
			break;
		}

		// create subitems
		auto i_sum = 0;
		HTREEITEM h_tree_item = nullptr;
		TVCS_CHECKSTATE b_check = TVCS_UNCHECKED;
		auto n_items = 0;
		for (auto j = 0; j < p_desc->csElementsArray.GetSize(); j++)
		{
			cs_comment.Format(_T("Create subitem %i"), j);
			dlg.SetStatus(cs_comment);

			h_tree_item = m_wnd_filter_view_.InsertItem(p_desc->csElementsArray.GetAt(j), m_h_tree_item_[i]);
			b_check = TVCS_CHECKED;
			auto cs_element_j = p_desc->csElementsArray.GetAt(j);
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
			m_wnd_filter_view_.SetCheck(h_tree_item, b_check);
			i_sum += b_check; // count number of positive checks (no check=0, check = 1)
			n_items++;
		}
		// trick needed here because if the first item is checked and not the others, then the parent stays in the initial state
		// namely "checked" (because at that moment it did not have other children)
		if (i_sum == 1 && h_tree_item != nullptr)
		{
			m_wnd_filter_view_.SetCheck(h_tree_item, TVCS_CHECKED);
			m_wnd_filter_view_.SetCheck(h_tree_item, b_check);
		}
		if (i_sum < n_items)
			m_wnd_filter_view_.Expand(m_h_tree_item_[i], TVE_EXPAND);
		i++;
	}
	m_wnd_filter_view_.UnlockWindowUpdate();
}

void CFilterPanel::PopulateItemFromTableLong(DB_ITEMDESC* pdesc)
{
	const auto p_set = &m_p_doc_->db_table->m_mainTableSet;
	const auto cs_col_head = pdesc->header_name;
	const auto array_size = pdesc->liArray.GetSize();
	if (pdesc->b_array_filter)
	{
		return;
	}
	if (pdesc->b_single_filter)
	{
		pdesc->cs_param_single_filter.Format(_T("%i"), pdesc->l_param_single_filter);
	}
	else
	{
		CString str;
		CString cs;
		pdesc->csElementsArray.RemoveAll();
		for (auto i = 0; i < array_size; i++)
		{
			const auto i_id = pdesc->liArray.GetAt(i);
			// add string only if found into p_main_table_set...
			str.Format(_T("%s=%li"), (LPCTSTR)cs_col_head, i_id);
			const auto flag = p_set->FindFirst(str);
			if (flag != 0)
			{
				cs.Format(_T("%i"), i_id);
				pdesc->csElementsArray.Add(cs);
				if (pdesc->b_single_filter && pdesc->l_param_single_filter != i_id)
				{
					pdesc->cs_param_single_filter.Format(_T("%i"), i_id);
				}
			}
		}
	}
}

void CFilterPanel::PopulateItemFromLinkedTable(DB_ITEMDESC* pdesc)
{
	auto str2 = pdesc->header_name;
	ASSERT(!str2.IsEmpty());

	auto p_linked_set = pdesc->plinkedSet;
	auto p_set = &m_p_doc_->db_table->m_mainTableSet;
	if (pdesc->b_array_filter)
		return;

	if (pdesc->b_single_filter)
	{
		pdesc->cs_param_single_filter = p_linked_set->get_string_from_id(pdesc->l_param_single_filter);
	}
	else
	{
		// loop over the whole content of the attached table
		pdesc->csElementsArray.RemoveAll();
		pdesc->liArray.RemoveAll();
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
				cs.Format(_T("%s=%li"), (LPCTSTR)pdesc->header_name, i_id);
				const auto flag = p_set->FindFirst(cs);
				if (flag != 0)
				{
					InsertAlphabetic(var_value0.bstrVal, pdesc->csElementsArray);
					pdesc->liArray.Add(i_id);
				}
				p_linked_set->MoveNext();
			}
		}
	}
}

void CFilterPanel::PopulateItemFromTablewithDate(DB_ITEMDESC* pdesc)
{
	CString cs; // to construct date
	const auto cs_column_head = pdesc->header_name;
	const auto p_main_table_set = &m_p_doc_->db_table->m_mainTableSet;
	const auto array_size = p_main_table_set->m_desc[CH_ACQDATE_DAY].tiArray.GetSize();

	if (pdesc->b_array_filter)
	{
		return;
	}
	if (pdesc->b_single_filter)
	{
		cs = pdesc->date_time_param_single_filter.Format(VAR_DATEVALUEONLY);
		pdesc->cs_param_single_filter = cs;
	}
	else
	{
		CString str;
		pdesc->csElementsArray.RemoveAll();
		for (auto i = 0; i < array_size; i++)
		{
			auto o_time = p_main_table_set->m_desc[CH_ACQDATE_DAY].tiArray.GetAt(i);
			cs = o_time.Format(_T("%m/%d/%y")); // filter needs to be constructed as month-day-year
			str.Format(_T("%s=#%s#"), (LPCTSTR)cs_column_head, (LPCTSTR)cs);
			const auto flag = p_main_table_set->FindFirst(str);
			if (flag != 0) // add string only if found into p_main_table_set...
			{
				cs = o_time.Format(VAR_DATEVALUEONLY);
				pdesc->csElementsArray.Add(cs);
			}
		}
	}
}

void CFilterPanel::InsertAlphabetic(const CString& cs, CStringArray& csArray)
{
	auto k = 0;
	for (auto i = 0; i < csArray.GetSize(); i++, k++)
	{
		const auto& cs_comp = csArray.GetAt(k);
		const auto j = cs.CompareNoCase(cs_comp);
		if (j < 0)
			break;
	}
	csArray.InsertAt(k, cs);
}

void CFilterPanel::BuildFilterItemIndirectionFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem)
{
	auto i = 0;
	for (auto item = startItem; item != nullptr; item = m_wnd_filter_view_.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wnd_filter_view_.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			auto cs = m_wnd_filter_view_.GetItemText(item);
			for (auto j = 0; j < pdesc->liArray.GetSize(); j++)
			{
				const auto li = pdesc->liArray.GetAt(j);
				auto str = pdesc->plinkedSet->get_string_from_id(li);
				if (str == cs)
				{
					pdesc->l_param_filter_array.Add(li);
					pdesc->cs_array_filter.Add(cs);
					break;
				}
			}
		}
	}
}

void CFilterPanel::BuildFilterItemLongFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem)
{
	auto i = 0;
	for (auto item = startItem; item != nullptr; item = m_wnd_filter_view_.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wnd_filter_view_.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			auto cs = m_wnd_filter_view_.GetItemText(item);
			const auto li = pdesc->liArray.GetAt(i);
			pdesc->l_param_filter_array.Add(li);
			pdesc->cs_array_filter.Add(cs);
		}
	}
}

void CFilterPanel::BuildFilterItemDateFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem)
{
	auto i = 0;
	for (auto item = startItem; item != nullptr; item = m_wnd_filter_view_.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wnd_filter_view_.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			auto cs_filter_checked = m_wnd_filter_view_.GetItemText(item);
			COleDateTime o_time;
			o_time.ParseDateTime(cs_filter_checked);
			pdesc->data_time_array_filter.Add(o_time);
			pdesc->cs_array_filter.Add(cs_filter_checked);
		}
	}
}

void CFilterPanel::OnApplyFilter()
{
	if (!m_p_doc_)
		return;
	auto p_db = m_p_doc_->db_table;

	auto i = 0;
	while (m_no_col_[i] > 0)
	{
		const auto h_parent = m_h_tree_item_[i];
		i++;

		const int i_col = m_wnd_filter_view_.GetItemData(h_parent);
		const auto p_desc = p_db->get_record_item_descriptor(i_col);
		//if root is checked (or unchecked), it means no item is selected - remove flag
		const auto state_root = m_wnd_filter_view_.GetCheck(h_parent);
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
				BuildFilterItemIndirectionFromTree(p_desc, start_item);
				break;
			case FIELD_LONG:
				BuildFilterItemLongFromTree(p_desc, start_item);
				break;
			case FIELD_DATE_YMD:
				BuildFilterItemDateFromTree(p_desc, start_item);
				break;
			default:
				ASSERT(false);
				break;
			}
		}
	}

	// update recordset and tell other views...
	p_db->m_mainTableSet.BuildFilters();
	p_db->m_mainTableSet.RefreshQuery();
	m_p_doc_->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
}

void CFilterPanel::OnSortRecords()
{
	auto p_database = m_p_doc_->db_table;
	ASSERT(p_database);
	const auto p_combo = static_cast<CMFCToolBarComboBoxButton*>(m_wnd_tool_bar_.GetButton(3));
	ASSERT(ID_RECORD_SORT == m_wnd_tool_bar_.GetItemID(3));

	const auto i_sel = p_combo->GetCurSel();
	ASSERT(i_sel != CB_ERR);
	const int i = p_combo->GetItemData(i_sel);
	p_database->m_mainTableSet.m_strSort = CdbTable::m_column_properties[i].header_name;

	p_database->m_mainTableSet.RefreshQuery();
	m_p_doc_->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
}

void CFilterPanel::SelectNext(BOOL bNext)
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
		const auto state = static_cast<CQuadStateTree*>(p_tree)->GetCheck(h_kid);
		if (state == TVCS_CHECKED)
		{
			h_last_selected = h_kid;
			n_selected++;
		}
		count++;
	}
	while ((h_kid = p_tree->GetNextSiblingItem(h_kid)));

	// if single selection select next item on the list and deselect current; update
	if (n_selected == 1)
	{
		HTREEITEM h_next;
		if (bNext)
			h_next = p_tree->GetNextSiblingItem(h_last_selected);
		else
			h_next = p_tree->GetPrevSiblingItem(h_last_selected);
		if (h_next == nullptr)
			return;
		static_cast<CQuadStateTree*>(p_tree)->SetCheck(h_next, TVCS_CHECKED);
		static_cast<CQuadStateTree*>(p_tree)->SetCheck(h_last_selected, TVCS_UNCHECKED);
		OnApplyFilter();
	}
}

void CFilterPanel::OnSelectNext()
{
	SelectNext(TRUE);
}

void CFilterPanel::OnSelectPrevious()
{
	SelectNext(FALSE);
}
