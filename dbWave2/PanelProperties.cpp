#include "StdAfx.h"

#include "PanelProperties.h"
#include "resource.h"
//#include "MainFrm.h"
#include "dbWave.h"
#include "dbWave_constants.h"
#include "DlgdbEditRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CPropertiesPanel, CDockablePane)

// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameters
int CPropertiesPanel::m_noCol[] = {
	// ------1
	CH_ACQDATE_DAY,
	CH_ACQDATE_TIME,
	CH_DATALEN,
	CH_PATH_ID,
	CH_FILENAME,
	CH_PATH2_ID,
	CH_FILESPK,
	CH_ACQ_COMMENTS,
	-1, // 0-6 acquisition: acq_date, datalen, path_ID, filename, path2_ID, filespk, acq_comment
	// ------2
	CH_IDINSECT,
	CH_IDSENSILLUM,
	CH_INSECT_ID,
	CH_SENSILLUM_ID,
	CH_LOCATION_ID,
	CH_STRAIN_ID,
	CH_SEX_ID,
	CH_OPERATOR_ID,
	-1,
	// 7-14 experiment: insectID, sensillumID, insectname_ID, sensillumname_ID, location_ID, strain_ID, sex_ID, operator_ID
	// ------3
	CH_EXPT_ID,
	CH_STIM_ID, CH_CONC_ID, CH_REPEAT,
	CH_STIM2_ID, CH_CONC2_ID, CH_REPEAT2,
	-1, // 15-21 stimulus: expt_ID, stim_ID, conc_ID, repeat, stim2_ID, conc2_ID, repeat2
	// ------4
	CH_NSPIKES,
	CH_NSPIKECLASSES,
	CH_FLAG,
	CH_MORE,
	-1
}; // 22-25 measures: n spikes, spikeclasses, flag, more

int CPropertiesPanel::m_prop_col_[] = {
	// TRUE = allow edit; list all possible columns
	FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE,
	TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE
};

CPropertiesPanel::CPropertiesPanel()
{
	m_wndEditInfosHeight = 0;
	m_pDoc = nullptr;
	m_pDocOld = nullptr;
	m_bUpdateCombos = FALSE;
	m_bchangedProperty = FALSE;
}

CPropertiesPanel::~CPropertiesPanel()
{
}

BEGIN_MESSAGE_MAP(CPropertiesPanel, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORT_PROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORT_PROPERTIES, OnUpdateSortProperties)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_BN_CLICKED(IDC_EDIT_INFOS, OnBnClickedEditinfos)
	ON_UPDATE_COMMAND_UI(IDC_EDIT_INFOS, OnUpdateBnEditinfos)
	ON_BN_CLICKED(IDC_UPDATE_INFOS, OnBnClickedUpdateinfos)
	ON_UPDATE_COMMAND_UI(IDC_UPDATE_INFOS, OnUpdateBnUpdateinfos)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)
END_MESSAGE_MAP()

void CPropertiesPanel::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
		return;

	CRect rect_client;
	GetClientRect(rect_client);
	const int cy_tlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rect_client.left,
	                          rect_client.top + m_wndEditInfosHeight,
	                          rect_client.Width(),
	                          cy_tlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rect_client.left,
	                           rect_client.top + m_wndEditInfosHeight + cy_tlb,
	                           rect_client.Width(),
	                           rect_client.Height() - m_wndEditInfosHeight - cy_tlb,
	                           SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	const CRect rect_dummy(0, 0, 24, 24);
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rect_dummy, this, 2))
		return -1; // fail to create
	SetPropListFont();
	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(the_app.hi_color_icons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(
		m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	// All commands will be routed via this control, not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesPanel::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesPanel::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesPanel::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesPanel::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesPanel::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesPanel::UpdatePropList()
{
	m_bchangedProperty = FALSE; // reset flag

	// database general section
	const int current_record_position = m_pDoc->db_get_current_record_position() + 1;
	const int n_records = m_pDoc->db_get_n_records();
	if (n_records == 0)
		return;

	const auto property_count = m_wndPropList.GetPropertyCount();
	const auto p_group0 = m_wndPropList.GetProperty(0);
	(p_group0->GetSubItem(0)->SetValue(static_cast<_variant_t>(current_record_position)));
	(p_group0->GetSubItem(1)->SetValue(static_cast<_variant_t>(n_records)));

	for (auto i = 1; i < property_count; i++)
	{
		const auto p_group = m_wndPropList.GetProperty(i);
		UpdateGroupPropFromTable(p_group);
	}
	m_bUpdateCombos = FALSE;
}

void CPropertiesPanel::UpdateGroupPropFromTable(CMFCPropertyGridProperty* p_group) const
{
	auto p_db = m_pDoc->db_table;
	DB_ITEMDESC desc;
	const auto n_sub_items = p_group->GetSubItemsCount();
	CdbTableAssociated* p2_linked_set;

	for (auto i = 0; i < n_sub_items; i++)
	{
		auto p_prop = p_group->GetSubItem(i);
		const int i_column = p_prop->GetData();
		p_db->get_record_item_value(i_column, &desc);
		p_prop->ResetOriginalValue();
		switch (p_db->m_mainTableSet.m_desc[i_column].data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			p_prop->SetValue(desc.csVal);
			p_prop->SetOriginalValue(desc.csVal);
			p2_linked_set = p_db->m_mainTableSet.m_desc[i_column].plinkedSet;
			if (m_bUpdateCombos || (p_prop->GetOptionCount() != p2_linked_set->GetRecordCount()))
			{
				p_prop->RemoveAllOptions();
				COleVariant var_value1;
				if (p2_linked_set->IsOpen() && !p2_linked_set->IsBOF())
				{
					COleVariant var_value0;
					p2_linked_set->MoveFirst();
					while (!p2_linked_set->IsEOF())
					{
						p2_linked_set->GetFieldValue(0, var_value0);
						CString cs = var_value0.bstrVal;
						if (!cs.IsEmpty())
							p_prop->AddOption(cs, TRUE);
						p2_linked_set->MoveNext();
					}
				}
			}
			break;
		case FIELD_LONG:
			p_prop->SetValue(desc.lVal);
			p_prop->SetOriginalValue(desc.lVal);
			break;
		case FIELD_TEXT:
			p_prop->SetValue(desc.csVal);
			p_prop->SetOriginalValue(desc.csVal);
			break;
		case FIELD_DATE_HMS:
			desc.csVal = desc.oVal.Format(VAR_TIMEVALUEONLY);
			p_prop->SetValue(desc.csVal);
			p_prop->SetOriginalValue(desc.csVal);
			break;
		case FIELD_DATE_YMD:
			desc.csVal = desc.oVal.Format(VAR_DATEVALUEONLY);
			p_prop->SetValue(desc.csVal);
			p_prop->SetOriginalValue(desc.csVal);
			break;
		default:
			break;
		}
	}
}

void CPropertiesPanel::UpdateTableFromProp()
{
	const auto p_database = m_pDoc->db_table;
	const auto p_main_table_set = &p_database->m_mainTableSet;
	m_bchangedProperty = FALSE; 
	p_main_table_set->Edit();

	const auto property_count = m_wndPropList.GetPropertyCount();
	for (auto i = 1; i < property_count; i++)
	{
		const auto p_group = m_wndPropList.GetProperty(i);
		UpdateTableFromGroupProp(p_group);
	}
	p_main_table_set->Update();
}

void CPropertiesPanel::UpdateTableFromGroupProp(const CMFCPropertyGridProperty* p_group)
{
	const auto p_database = m_pDoc->db_table;
	const auto sub_items_count = p_group->GetSubItemsCount();

	for (auto i = 0; i < sub_items_count; i++)
	{
		const auto p_prop = p_group->GetSubItem(i);
		if (!p_prop->IsModified())
			continue;

		const int prop_data_id = static_cast<int>(p_prop->GetData());
		const auto prop_val = p_prop->GetValue();
		const auto record_item_descriptor = p_database->get_record_item_descriptor(prop_data_id);
		if (record_item_descriptor == nullptr)
			continue;

		switch (record_item_descriptor->data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			record_item_descriptor->csVal = prop_val.bstrVal;
			p_prop->ResetOriginalValue();
			p_prop->SetOriginalValue(record_item_descriptor->csVal);
			p_prop->SetValue(record_item_descriptor->csVal);
			break;
		case FIELD_TEXT:
			record_item_descriptor->csVal = prop_val.bstrVal;
			p_prop->ResetOriginalValue();
			p_prop->SetOriginalValue(record_item_descriptor->csVal);
			p_prop->SetValue(record_item_descriptor->csVal);
			break;
		case FIELD_LONG:
			record_item_descriptor->lVal = prop_val.lVal;
			p_prop->ResetOriginalValue();
			p_prop->SetOriginalValue(prop_val.lVal);
			p_prop->SetValue(prop_val.lVal);
			break;
		//case FIELD_DATE:
		default:
			break;
		}
		p_database->set_record_item_value(prop_data_id, record_item_descriptor);
	}
}

#define ID_BASE	1000

void CPropertiesPanel::InitPropList()
{
	// exit if doc is not defined
	if (!m_pDoc || m_pDoc == nullptr)
		return;

	if (m_pDocOld == m_pDoc) //NULL)
	{
		UpdatePropList();
		return;
	}

	// first time init
	m_pDocOld = m_pDoc;
	m_wndPropList.RemoveAll();

	// housekeeping
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea(TRUE);
	m_wndPropList.SetVSDotNetLook(TRUE);
	m_wndPropList.SetGroupNameFullWidth(TRUE);
	m_wndPropList.MarkModifiedProperties(TRUE, TRUE);

	// get pointer to database specific object which contains recordsets
	const auto p_database = m_pDoc->db_table;
	auto m__i_id = ID_BASE;

	// ------------------------------------------------------
	const auto p_group0 = new CMFCPropertyGridProperty(_T("Database"));
	p_group0->SetData(m__i_id);
	m__i_id++; // iID = 1000
	const int record_position = p_database->m_mainTableSet.GetAbsolutePosition() + 1;
	const int records_count = p_database->m_mainTableSet.GetNRecords();
	auto p_prop = new CMFCPropertyGridProperty(_T("current record"), static_cast<_variant_t>(record_position),
	                                           _T("current record in the database (soft index)"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1001
	p_group0->AddSubItem(p_prop);
	p_prop = new CMFCPropertyGridProperty(_T("total records"), static_cast<_variant_t>(records_count),
	                                      _T("number of records in the database"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1002
	p_group0->AddSubItem(p_prop);
	m_wndPropList.AddProperty(p_group0);

	// ------------------------------------------------------ database content
	const auto p_group1 = new CMFCPropertyGridProperty(_T("Acquisition"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1003
	auto i_col0 = InitGroupFromTable(p_group1, 0);
	m_wndPropList.AddProperty(p_group1);

	const auto p_group2 = new CMFCPropertyGridProperty(_T("Experimental conditions"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1004
	i_col0 = InitGroupFromTable(p_group2, i_col0);
	m_wndPropList.AddProperty(p_group2);

	const auto p_group3 = new CMFCPropertyGridProperty(_T("Stimulus"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1005
	i_col0 = InitGroupFromTable(p_group3, i_col0);
	m_wndPropList.AddProperty(p_group3);

	const auto p_group4 = new CMFCPropertyGridProperty(_T("Measures"));
	p_prop->SetData(m__i_id);
	//m__i_id++;		// iID = 1005
	/*i_col0 =*/
	InitGroupFromTable(p_group4, i_col0);
	m_wndPropList.AddProperty(p_group4);

	if (p_database && m_pDoc->db_get_n_records() > 0)
	{
		m_bUpdateCombos = TRUE;
		UpdatePropList();
	}
}

// look at elements indexes stored into m_nCol and create property grid
// indexes are either positive or -1 (-1 is used as a stop tag)
// init all elements pointed at within m_noCol table from element icol0 to element = -1 (stop value)
// returns next position after the stop tag

int CPropertiesPanel::InitGroupFromTable(CMFCPropertyGridProperty* pGroup, int icol0)
{
	const auto p_database = m_pDoc->db_table;
	p_database->m_mainTableSet.GetNRecords();
	constexpr int i_col1 = sizeof(m_noCol) / sizeof(int);
	if (icol0 > i_col1) icol0 = i_col1 - 1;
	int i;

	for (i = icol0; i < i_col1; i++)
	{
		const auto i_desc_tab = m_noCol[i];
		if (i_desc_tab < 0)
			break;

		DB_ITEMDESC desc;
		desc.csVal = _T("undefined");
		desc.lVal = 0;
		desc.data_code_number = p_database->m_mainTableSet.m_desc[i_desc_tab].data_code_number;

		CMFCPropertyGridProperty* p_prop = nullptr;
		CString cs_comment;
		CString cs_title = CdbTable::m_column_properties[i_desc_tab].description;

		switch (desc.data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			cs_comment = _T("Field indirect text");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.csVal, cs_comment, i_desc_tab);
			break;
		case FIELD_LONG:
			cs_comment = _T("Field long");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.lVal, cs_comment, i_desc_tab);
			break;
		case FIELD_TEXT:
			cs_comment = _T("Field text");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.csVal, cs_comment, i_desc_tab);
			break;
		case FIELD_DATE:
		case FIELD_DATE_HMS:
		case FIELD_DATE_YMD:
			cs_comment = _T("Field date");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.csVal, cs_comment, i_desc_tab);
			break;
		default:
			cs_comment = _T("Field type unknown");
			CString csValue = cs_comment;
			p_prop = new CMFCPropertyGridProperty(cs_title, csValue, cs_comment, i_desc_tab);
			break;
		}

		// add a few infos
		p_prop->AllowEdit(m_prop_col_[i_desc_tab]);
		p_prop->SetData(i_desc_tab);
		pGroup->AddSubItem(p_prop);
	}
	return i + 1;
}

void CPropertiesPanel::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesPanel::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesPanel::SetPropListFont()
{
	DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);
	afxGlobalData.GetNonClientMetrics(info);
	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;
	m_fntPropList.CreateFontIndirect(&lf);
	m_wndPropList.SetFont(&m_fntPropList);
}

void CPropertiesPanel::OnUpdateBnEditinfos(CCmdUI* pCmdUI)
{
}

void CPropertiesPanel::OnBnClickedEditinfos()
{
	m_pDoc->update_all_views_db_wave(nullptr, HINT_GET_SELECTED_RECORDS, nullptr);
	DlgdbEditRecord dlg;
	dlg.m_pdbDoc = m_pDoc;
	if (IDOK == dlg.DoModal())
	{
		m_pDoc->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
		m_pDoc->update_all_views_db_wave(nullptr, HINT_DOC_HAS_CHANGED, nullptr);
	}
}

void CPropertiesPanel::OnUpdateBnUpdateinfos(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bchangedProperty);
}

void CPropertiesPanel::OnBnClickedUpdateinfos()
{
	const auto l_index = m_pDoc->db_get_current_record_position();
	UpdateTableFromProp();
	m_pDoc->db_set_current_record_position(l_index);
	m_pDoc->update_all_views_db_wave(nullptr, HINT_DOC_HAS_CHANGED, nullptr);
}

LRESULT CPropertiesPanel::OnPropertyChanged(WPARAM, LPARAM lParam)
{
	//auto p_prop = reinterpret_cast<CMFCPropertyGridProperty*>(lParam);
	m_bchangedProperty = TRUE;
	return 0;
}

LRESULT CPropertiesPanel::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	//auto p_app = (CdbWaveApp*)AfxGetApp();
	//short lowp = LOWORD(lParam);
	//short highp = HIWORD(lParam);

	switch (wParam)
	{
	case HINT_ACTIVATE_VIEW:
		m_pDoc = reinterpret_cast<CdbWaveDoc*>(lParam);
		if (m_pDoc != m_pDocOld)
			InitPropList();
		break;

	case HINT_MDI_ACTIVATE:
		{
			const auto* main_window_frame = static_cast<CMDIFrameWndEx*>(AfxGetMainWnd());
			BOOL b_maximized;
			const auto p_child = main_window_frame->MDIGetActive(&b_maximized);
			if (!p_child) return NULL;
			const auto p_document = p_child->GetActiveDocument();
			if (!p_document || !p_document->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)))
				return NULL;
			m_pDoc = static_cast<CdbWaveDoc*>(p_document);
			if (m_pDoc != m_pDocOld)
			{
				m_bUpdateCombos = TRUE;
				InitPropList();
			}
		}
		break;

	default:
		break;
	}
	return 0L;
}

void CPropertiesPanel::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	m_pDoc = reinterpret_cast<CdbWaveDoc*>(pSender);
	switch (LOWORD(lHint))
	{
	case HINT_CLOSE_FILE_MODIFIED:
		m_pDocOld = nullptr;
		break;
	case HINT_REQUERY:
	case HINT_DOC_HAS_CHANGED:
	case HINT_DOC_MOVE_RECORD:
	case HINT_REPLACE_VIEW:
	default:
		InitPropList();
		break;
	}
}
