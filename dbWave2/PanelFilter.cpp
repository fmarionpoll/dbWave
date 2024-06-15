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


// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameteres
int CFilterPanel::m_noCol[] = {
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
	ON_COMMAND(ID_APPLYFILTER, OnApplyFilter)
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
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	if (!m_wndFilterView.Create(dwViewStyle, rect_dummy, this, IDC_TREE1))
		return -1; // fail to create

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(
		m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.ReplaceButton(
		ID_RECORD_SORT,
		CMFCToolBarComboBoxButton(ID_RECORD_SORT, /*GetCmdMgr()->GetCmdImage(ID_RECORD_SORT)*/ NULL, CBS_DROPDOWN));

	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);
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
	auto p_wnd_tree = static_cast<CTreeCtrl*>(&m_wndFilterView);
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
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CFilterPanel::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
		return;

	CRect rect_client;
	GetClientRect(rect_client);

	const int cy_tlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rect_client.left, rect_client.top, rect_client.Width(), cy_tlb,
	                          SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFilterView.SetWindowPos(nullptr, rect_client.left + 1, rect_client.top + cy_tlb + 1, rect_client.Width() - 2,
	                             rect_client.Height() - cy_tlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFilterPanel::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect_tree;
	m_wndFilterView.GetWindowRect(rect_tree);
	ScreenToClient(rect_tree);

	rect_tree.InflateRect(1, 1);
	dc.Draw3dRect(rect_tree, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
}

void CFilterPanel::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndFilterView.SetFocus();
}

void CFilterPanel::OnUpdateTree()
{
	m_pDocOld = nullptr;
	InitFilterList();
}

LRESULT CFilterPanel::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	//auto p_app = (CdbWaveApp*)AfxGetApp();
	//short lowp = LOWORD(lParam);
	//short highp = HIWORD(lParam);

	switch (wParam)
	{
	case HINT_ACTIVATEVIEW:
		m_pDoc = reinterpret_cast<CdbWaveDoc*>(lParam);
		if (m_pDoc != m_pDocOld)
			InitFilterList();
		break;

	case HINT_MDIACTIVATE:
		{
			auto* pmain = static_cast<CMainFrame*>(AfxGetMainWnd());
			BOOL b_maximized;
			auto p_child = pmain->MDIGetActive(&b_maximized);
			if (!p_child)
				return NULL;
			const auto p_document = p_child->GetActiveDocument();
			if (!p_document || !p_document->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)))
				return NULL;
			m_pDoc = static_cast<CdbWaveDoc*>(p_document);
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
	m_pDoc = reinterpret_cast<CdbWaveDoc*>(pSender);
	switch (LOWORD(lHint))
	{
	case HINT_CLOSEFILEMODIFIED: // save current file parms
		//m_pDocOld = NULL;
		break;

	case HINT_REQUERY:
		m_pDocOld = nullptr;

	case HINT_DOCHASCHANGED:
	case HINT_DOCMOVERECORD:
	case HINT_REPLACEVIEW:
	default:
		InitFilterList();
		break;
	}
}

void CFilterPanel::InitFilterList()
{
	if (m_pDocOld == m_pDoc)
		return;
	m_pDocOld = m_pDoc;

	auto p_db = m_pDoc->m_pDB;
	ASSERT(p_db);

	// setup dialog box
	DlgProgress dlg;
	dlg.Create();
	dlg.SetStep(1);

	// fill items of the combo (column heads to sort data)
	dlg.SetStatus(_T("List categories available..."));
	auto p_combo = static_cast<CMFCToolBarComboBoxButton*>(m_wndToolBar.GetButton(3));
	ASSERT(ID_RECORD_SORT == m_wndToolBar.GetItemID(3));
	if (p_combo->GetCount() <= 0)
	{
		for (auto i = 0; i < N_TABLE_COLUMNS; i++)
		{
			//p_combo->AddSortedItem(p_db->m_column_properties[i].descriptor, i);
			p_combo->AddSortedItem(CdbTable::m_column_properties[i].description, i);
		}
	}
	p_combo->SelectItem(p_db->m_mainTableSet.m_strSort);

	// fill items of the tree
	dlg.SetStatus(_T("Populate categories..."));
	if (p_db->m_mainTableSet.IsBOF() && p_db->m_mainTableSet.IsEOF())
		return;
	m_wndFilterView.LockWindowUpdate(); // prevent screen upate (and flicker)
	if (m_wndFilterView.GetCount() > 0)
		m_wndFilterView.DeleteAllItems();

	auto i = 0;
	p_db->m_mainTableSet.BuildAndSortIDArrays();

	CString cs_comment;
	while (m_noCol[i] > 0)
	{
		const auto icol = m_noCol[i];
		const auto pdesc = p_db->GetRecordItemDescriptor(icol);
		m_htreeitem[i] = m_wndFilterView.InsertItem(CdbTable::m_column_properties[icol].description, TVI_ROOT); //hRoot);
		//m_htreeitem[i] = m_wndFilterView.InsertItem(p_db->m_column_properties[icol].descriptor, TVI_ROOT); //hRoot);
		m_wndFilterView.SetItemData(m_htreeitem[i], m_noCol[i]); // save table index into head of the list

		cs_comment.Format(_T("Category %i: "), i);
		cs_comment += CdbTable::m_column_properties[icol].description;
		//cs_comment += p_db->m_column_properties[icol].descriptor;
		dlg.SetStatus(cs_comment);

		// collect data (array of unique descriptors)
		switch (pdesc->data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			PopulateItemFromLinkedTable(pdesc);
			break;
		case FIELD_LONG:
			PopulateItemFromTableLong(pdesc);
			break;
		case FIELD_DATE_YMD:
			PopulateItemFromTablewithDate(pdesc);
			break;
		default:
			break;
		}

		// create subitems
		auto isum = 0;
		HTREEITEM htree_item = nullptr;
		TVCS_CHECKSTATE bcheck = TVCS_UNCHECKED;
		auto nitems = 0;
		for (auto j = 0; j < pdesc->csElementsArray.GetSize(); j++)
		{
			cs_comment.Format(_T("Create subitem %i"), j);
			dlg.SetStatus(cs_comment);

			htree_item = m_wndFilterView.InsertItem(pdesc->csElementsArray.GetAt(j), m_htreeitem[i]);
			bcheck = TVCS_CHECKED;
			auto cs_elmtj = pdesc->csElementsArray.GetAt(j);
			if (pdesc->b_array_filter)
			{
				bcheck = TVCS_UNCHECKED;
				for (auto k = 0; k < pdesc->cs_array_filter.GetSize(); k++)
				{
					if (cs_elmtj.CompareNoCase(pdesc->cs_array_filter.GetAt(k)) == 0)
					{
						bcheck = TVCS_CHECKED;
						break;
					}
				}
			}
			else if (pdesc->b_single_filter)
			{
				if (cs_elmtj.CompareNoCase(pdesc->cs_param_single_filter) != 0)
					bcheck = TVCS_UNCHECKED;
			}
			m_wndFilterView.SetCheck(htree_item, bcheck);
			isum += bcheck; // count number of positive checks (no check=0, check = 1)
			nitems++;
		}
		// trick needed here because if the first item is checked and not the others, then the parent stays in the initial state
		// namely "checked" (because at that moment it did not have other children)
		if (isum == 1 && htree_item != nullptr)
		{
			m_wndFilterView.SetCheck(htree_item, TVCS_CHECKED);
			m_wndFilterView.SetCheck(htree_item, bcheck);
		}
		if (isum < nitems)
			m_wndFilterView.Expand(m_htreeitem[i], TVE_EXPAND);
		i++;
	}
	m_wndFilterView.UnlockWindowUpdate();
}

void CFilterPanel::PopulateItemFromTableLong(DB_ITEMDESC* pdesc)
{
	CString cs; // to construct insect and sensillum number (for example)
	CString str; // to store FindFirst filter
	auto p_set = &m_pDoc->m_pDB->m_mainTableSet;
	const auto cscolhead = pdesc->header_name;
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
		pdesc->csElementsArray.RemoveAll();
		for (auto i = 0; i < array_size; i++)
		{
			const auto i_id = pdesc->liArray.GetAt(i);
			// add string only if found into p_maintable_set...
			str.Format(_T("%s=%li"), (LPCTSTR)cscolhead, i_id);
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
	CString cs;
	auto str2 = pdesc->header_name;
	ASSERT(!str2.IsEmpty());

	auto plinked_set = pdesc->plinkedSet;
	auto p_set = &m_pDoc->m_pDB->m_mainTableSet;
	if (pdesc->b_array_filter)
		return;

	if (pdesc->b_single_filter)
	{
		pdesc->cs_param_single_filter = plinked_set->GetStringFromID(pdesc->l_param_single_filter);
	}
	else
	{
		// loop over the whole content of the attached table
		pdesc->csElementsArray.RemoveAll();
		pdesc->liArray.RemoveAll();
		if (plinked_set->IsOpen() && !plinked_set->IsBOF())
		{
			COleVariant var_value0, var_value1;
			plinked_set->MoveFirst();
			while (!plinked_set->IsEOF())
			{
				plinked_set->GetFieldValue(0, var_value0);
				plinked_set->GetFieldValue(1, var_value1);
				const auto i_id = var_value1.lVal;
				// add string only if found into p_maintable_set...
				cs.Format(_T("%s=%li"), (LPCTSTR)pdesc->header_name, i_id);
				const auto flag = p_set->FindFirst(cs);
				if (flag != 0)
				{
					InsertAlphabetic(var_value0.bstrVal, pdesc->csElementsArray);
					pdesc->liArray.Add(i_id);
				}
				plinked_set->MoveNext();
			}
		}
	}
}

void CFilterPanel::PopulateItemFromTablewithDate(DB_ITEMDESC* pdesc)
{
	CString cs; // to construct date
	const auto cscolhead = pdesc->header_name;
	CString str; // to construct filter
	auto p_maintable_set = &m_pDoc->m_pDB->m_mainTableSet;
	const auto array_size = p_maintable_set->m_desc[CH_ACQDATE_DAY].tiArray.GetSize();

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
		pdesc->csElementsArray.RemoveAll();
		for (auto i = 0; i < array_size; i++)
		{
			auto o_time = p_maintable_set->m_desc[CH_ACQDATE_DAY].tiArray.GetAt(i);
			cs = o_time.Format(_T("%m/%d/%y")); // filter needs to be constructed as month-day-year
			str.Format(_T("%s=#%s#"), (LPCTSTR)cscolhead, (LPCTSTR)cs);
			const auto flag = p_maintable_set->FindFirst(str);
			if (flag != 0) // add string only if found into p_maintable_set...
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
		const auto& cscomp = csArray.GetAt(k);
		const auto j = cs.CompareNoCase(cscomp);
		if (j < 0)
			break;
	}
	csArray.InsertAt(k, cs);
}

void CFilterPanel::BuildFilterItemIndirectionFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem)
{
	auto i = 0;
	for (auto item = startItem; item != nullptr; item = m_wndFilterView.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wndFilterView.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			auto cs = m_wndFilterView.GetItemText(item);
			for (auto j = 0; j < pdesc->liArray.GetSize(); j++)
			{
				const auto li = pdesc->liArray.GetAt(j);
				auto str = pdesc->plinkedSet->GetStringFromID(li);
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
	for (auto item = startItem; item != nullptr; item = m_wndFilterView.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wndFilterView.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			auto cs = m_wndFilterView.GetItemText(item);
			const auto li = pdesc->liArray.GetAt(i);
			pdesc->l_param_filter_array.Add(li);
			pdesc->cs_array_filter.Add(cs);
		}
	}
}

void CFilterPanel::BuildFilterItemDateFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem)
{
	auto i = 0;
	for (auto item = startItem; item != nullptr; item = m_wndFilterView.GetNextItem(item, TVGN_NEXT), i++)
	{
		const auto state = m_wndFilterView.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			auto cs_filter_checked = m_wndFilterView.GetItemText(item);
			COleDateTime o_time;
			o_time.ParseDateTime(cs_filter_checked);
			pdesc->data_time_array_filter.Add(o_time);
			pdesc->cs_array_filter.Add(cs_filter_checked);
		}
	}
}

void CFilterPanel::OnApplyFilter()
{
	if (!m_pDoc)
		return;
	auto p_db = m_pDoc->m_pDB;

	auto i = 0;
	while (m_noCol[i] > 0)
	{
		const auto h_parent = m_htreeitem[i];
		i++;

		const int icol = m_wndFilterView.GetItemData(h_parent);
		auto pdesc = p_db->GetRecordItemDescriptor(icol);
		//if root is checked (or unchecked), it means no item is selected - remove flag
		const auto state_root = m_wndFilterView.GetCheck(h_parent);
		if ((state_root == TVCS_CHECKED) || (state_root == TVCS_UNCHECKED))
		{
			pdesc->b_array_filter = FALSE;
		}
		// else if foot is undeterminate build filter
		else
		{
			pdesc->b_array_filter = TRUE;
			pdesc->l_param_filter_array.RemoveAll();
			pdesc->cs_array_filter.RemoveAll();
			pdesc->data_time_array_filter.RemoveAll();
			const auto start_item = m_wndFilterView.GetNextItem(h_parent, TVGN_CHILD);
			switch (pdesc->data_code_number)
			{
			case FIELD_IND_TEXT:
			case FIELD_IND_FILEPATH:
				BuildFilterItemIndirectionFromTree(pdesc, start_item);
				break;
			case FIELD_LONG:
				BuildFilterItemLongFromTree(pdesc, start_item);
				break;
			case FIELD_DATE_YMD:
				BuildFilterItemDateFromTree(pdesc, start_item);
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
	m_pDoc->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
}

void CFilterPanel::OnSortRecords()
{
	auto p_database = m_pDoc->m_pDB;
	ASSERT(p_database);
	const auto p_combo = static_cast<CMFCToolBarComboBoxButton*>(m_wndToolBar.GetButton(3));
	ASSERT(ID_RECORD_SORT == m_wndToolBar.GetItemID(3));

	const auto isel = p_combo->GetCurSel();
	ASSERT(isel != CB_ERR);
	const int i = p_combo->GetItemData(isel);
	p_database->m_mainTableSet.m_strSort = p_database->m_column_properties[i].header_name;

	p_database->m_mainTableSet.RefreshQuery();
	m_pDoc->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
}

void CFilterPanel::SelectNext(BOOL bNext)
{
	const auto p_tree = static_cast<CTreeCtrl*>(&m_wndFilterView);
	ASSERT_VALID(p_tree);
	auto h_item = p_tree->GetSelectedItem();
	if (!p_tree->ItemHasChildren(h_item))
		h_item = p_tree->GetParentItem(h_item);
	if (h_item == nullptr)
		return;

	auto count = 0;
	HTREEITEM hlastselected = nullptr;
	auto nselected = 0;
	auto h_kid = p_tree->GetChildItem(h_item);
	do
	{
		const auto state = static_cast<CQuadStateTree*>(p_tree)->GetCheck(h_kid);
		if (state == TVCS_CHECKED)
		{
			hlastselected = h_kid;
			nselected++;
		}
		count++;
	}
	while ((h_kid = p_tree->GetNextSiblingItem(h_kid)));

	// if single selection select next item on the list and deselect current; update
	if (nselected == 1)
	{
		HTREEITEM h_next;
		if (bNext)
			h_next = p_tree->GetNextSiblingItem(hlastselected);
		else
			h_next = p_tree->GetPrevSiblingItem(hlastselected);
		if (h_next == nullptr)
			return;
		static_cast<CQuadStateTree*>(p_tree)->SetCheck(h_next, TVCS_CHECKED);
		static_cast<CQuadStateTree*>(p_tree)->SetCheck(hlastselected, TVCS_UNCHECKED);
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
