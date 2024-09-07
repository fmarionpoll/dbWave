#include "StdAfx.h"

#include "PaneldbProperties.h"
#include "resource.h"
//#include "MainFrm.h"
#include "dbWave.h"
#include "dbWave_constants.h"
#include "DlgdbEditRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(PaneldbProperties, CDockablePane)

// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameters
int PaneldbProperties::m_no_col_[] = {
	// ------1
	CH_ACQDATE_DAY,
	CH_ACQDATE_TIME,
	CH_DATALEN,
	CH_PATH_ID,
	CH_FILENAME,
	CH_PATH2_ID,
	CH_FILESPK,
	CH_ACQ_COMMENTS,
	-1, // 0-6 acquisition: acq_date, data_len, path_ID, filename, path2_ID, file_spk, acq_comment
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
	// 7-14 experiment: insectID, sensillumID, insect_name_ID, sensillum_name_ID, location_ID, strain_ID, sex_ID, operator_ID
	// ------3
	CH_EXPT_ID,
	CH_STIM_ID, CH_CONC_ID, CH_REPEAT,
	CH_STIM2_ID, CH_CONC2_ID, CH_REPEAT2,
	-1, // 15-21 stimulus: experiment_ID, stimulus_ID, concentration_ID, repeat, stimulus2_ID, concentration2_ID, repeat2
	// ------4
	CH_NSPIKES,
	CH_NSPIKECLASSES,
	CH_FLAG,
	CH_MORE,
	-1
}; // 22-25 measures: n spikes, spike_classes, flag, more

int PaneldbProperties::m_prop_col_[] = {
	// TRUE = allow edit; list all possible columns
	FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE,
	TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE
};

PaneldbProperties::PaneldbProperties()
= default;

PaneldbProperties::~PaneldbProperties()
= default;

BEGIN_MESSAGE_MAP(PaneldbProperties, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()

	ON_COMMAND(ID_EXPAND_ALL, on_expand_all_properties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, on_update_expand_all_properties)
	ON_COMMAND(ID_SORT_PROPERTIES, on_sort_properties)
	ON_UPDATE_COMMAND_UI(ID_SORT_PROPERTIES, on_update_sort_properties)
	ON_BN_CLICKED(IDC_EDIT_INFOS, on_bn_clicked_edit_infos)
	ON_UPDATE_COMMAND_UI(IDC_EDIT_INFOS, on_update_bn_edit_infos)
	ON_BN_CLICKED(IDC_UPDATE_INFOS, on_bn_clicked_update_infos)
	ON_UPDATE_COMMAND_UI(IDC_UPDATE_INFOS, on_update_bn_update_infos)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, on_property_changed)
	ON_MESSAGE(WM_MYMESSAGE, on_my_message)
END_MESSAGE_MAP()

void PaneldbProperties::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
		return;

	CRect rect_client;
	GetClientRect(rect_client);
	const int cy_tlb = wnd_tool_bar_.CalcFixedLayout(FALSE, TRUE).cy;

	wnd_tool_bar_.SetWindowPos(nullptr, rect_client.left,
	                          rect_client.top + m_wnd_edit_infos_height_,
	                          rect_client.Width(),
	                          cy_tlb, SWP_NOACTIVATE | SWP_NOZORDER);

	wnd_property_list_.SetWindowPos(nullptr, rect_client.left,
	                           rect_client.top + m_wnd_edit_infos_height_ + cy_tlb,
	                           rect_client.Width(),
	                           rect_client.Height() - m_wnd_edit_infos_height_ - cy_tlb,
	                           SWP_NOACTIVATE | SWP_NOZORDER);
}

int PaneldbProperties::OnCreate(const LPCREATESTRUCT lp_create_struct)
{
	if (CDockablePane::OnCreate(lp_create_struct) == -1)
		return -1;

	const CRect rect_dummy(0, 0, 24, 24);
	if (!wnd_property_list_.Create(WS_VISIBLE | WS_CHILD, rect_dummy, this, 2))
		return -1; // fail to create
	set_prop_list_font();
	init_prop_list();

	wnd_tool_bar_.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	wnd_tool_bar_.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);

	wnd_tool_bar_.CleanUpLockedImages();
	wnd_tool_bar_.LoadBitmap(the_app.hi_color_icons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	wnd_tool_bar_.SetPaneStyle(wnd_tool_bar_.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	wnd_tool_bar_.SetPaneStyle(
		wnd_tool_bar_.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	// All commands will be routed via this control, not via the parent frame:
	wnd_tool_bar_.SetOwner(this);
	wnd_tool_bar_.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void PaneldbProperties::OnSize(const UINT n_type, const int cx, int cy)
{
	CDockablePane::OnSize(n_type, cx, cy);
	AdjustLayout();
}

void PaneldbProperties::on_expand_all_properties()
{
	wnd_property_list_.ExpandAll();
}

void PaneldbProperties::on_update_expand_all_properties(CCmdUI* /* pCmdUI */)
{
}

void PaneldbProperties::on_sort_properties()
{
	wnd_property_list_.SetAlphabeticMode(!wnd_property_list_.IsAlphabeticMode());
}

void PaneldbProperties::on_update_sort_properties(CCmdUI* p_cmd_ui)
{
	p_cmd_ui->SetCheck(wnd_property_list_.IsAlphabeticMode());
}

void PaneldbProperties::update_prop_list()
{
	m_b_changed_property_ = FALSE; // reset flag

	// database general section
	const int current_record_position = m_p_doc_->db_get_current_record_position() + 1;
	const int n_records = m_p_doc_->db_get_records_count();
	if (n_records == 0)
		return;

	const auto property_count = wnd_property_list_.GetPropertyCount();
	const auto p_group0 = wnd_property_list_.GetProperty(0);
	(p_group0->GetSubItem(0)->SetValue(static_cast<_variant_t>(current_record_position)));
	(p_group0->GetSubItem(1)->SetValue(static_cast<_variant_t>(n_records)));

	for (auto i = 1; i < property_count; i++)
	{
		const auto p_group = wnd_property_list_.GetProperty(i);
		update_group_prop_from_table(p_group);
	}
	m_b_update_combos_ = FALSE;
}

void PaneldbProperties::update_group_prop_from_table(CMFCPropertyGridProperty* p_group) const
{
	auto p_db = m_p_doc_->db_table;
	DB_ITEMDESC desc;
	const auto n_sub_items = p_group->GetSubItemsCount();
	CdbTableAssociated* p2_linked_set;

	for (auto i = 0; i < n_sub_items; i++)
	{
		auto p_prop = p_group->GetSubItem(i);
		const int i_column = p_prop->GetData();
		p_db->get_record_item_value(i_column, &desc);
		p_prop->ResetOriginalValue();
		switch (p_db->m_main_table_set.m_desc[i_column].data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			p_prop->SetValue(desc.cs_val);
			p_prop->SetOriginalValue(desc.cs_val);
			p2_linked_set = p_db->m_main_table_set.m_desc[i_column].p_linked_set;
			if (m_b_update_combos_ || (p_prop->GetOptionCount() != p2_linked_set->GetRecordCount()))
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
			p_prop->SetValue(desc.l_val);
			p_prop->SetOriginalValue(desc.l_val);
			break;
		case FIELD_TEXT:
			p_prop->SetValue(desc.cs_val);
			p_prop->SetOriginalValue(desc.cs_val);
			break;
		case FIELD_DATE_HMS:
			desc.cs_val = desc.o_val.Format(VAR_TIMEVALUEONLY);
			p_prop->SetValue(desc.cs_val);
			p_prop->SetOriginalValue(desc.cs_val);
			break;
		case FIELD_DATE_YMD:
			desc.cs_val = desc.o_val.Format(VAR_DATEVALUEONLY);
			p_prop->SetValue(desc.cs_val);
			p_prop->SetOriginalValue(desc.cs_val);
			break;
		default:
			break;
		}
	}
}

void PaneldbProperties::update_table_from_prop()
{
	const auto p_database = m_p_doc_->db_table;
	const auto p_main_table_set = &p_database->m_main_table_set;
	m_b_changed_property_ = FALSE; 
	p_main_table_set->Edit();

	const auto property_count = wnd_property_list_.GetPropertyCount();
	for (auto i = 1; i < property_count; i++)
	{
		const auto p_group = wnd_property_list_.GetProperty(i);
		update_table_from_group_prop(p_group);
	}
	p_main_table_set->Update();
}

void PaneldbProperties::update_table_from_group_prop(const CMFCPropertyGridProperty* p_group)
{
	const auto p_database = m_p_doc_->db_table;
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
			record_item_descriptor->cs_val = prop_val.bstrVal;
			p_prop->ResetOriginalValue();
			p_prop->SetOriginalValue(record_item_descriptor->cs_val);
			p_prop->SetValue(record_item_descriptor->cs_val);
			break;
		case FIELD_TEXT:
			record_item_descriptor->cs_val = prop_val.bstrVal;
			p_prop->ResetOriginalValue();
			p_prop->SetOriginalValue(record_item_descriptor->cs_val);
			p_prop->SetValue(record_item_descriptor->cs_val);
			break;
		case FIELD_LONG:
			record_item_descriptor->l_val = prop_val.lVal;
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

void PaneldbProperties::init_prop_list()
{
	// exit if doc is not defined
	if (!m_p_doc_ || m_p_doc_ == nullptr)
		return;

	if (m_p_doc_old_ == m_p_doc_) //NULL)
	{
		update_prop_list();
		return;
	}

	// first time init
	m_p_doc_old_ = m_p_doc_;
	wnd_property_list_.RemoveAll();

	// housekeeping
	wnd_property_list_.EnableHeaderCtrl(FALSE);
	wnd_property_list_.EnableDescriptionArea(TRUE);
	wnd_property_list_.SetVSDotNetLook(TRUE);
	wnd_property_list_.SetGroupNameFullWidth(TRUE);
	wnd_property_list_.MarkModifiedProperties(TRUE, TRUE);

	// get pointer to database specific object which contains record_sets
	const auto p_database = m_p_doc_->db_table;
	auto m_i_id = ID_BASE;

	// ------------------------------------------------------
	const auto p_group0 = new CMFCPropertyGridProperty(_T("Database"));
	p_group0->SetData(m_i_id);
	m_i_id++; // iID = 1000
	const int record_position = p_database->m_main_table_set.GetAbsolutePosition() + 1;
	const int records_count = p_database->m_main_table_set.get_records_count();
	auto p_prop = new CMFCPropertyGridProperty(_T("current record"), static_cast<_variant_t>(record_position),
	                                           _T("current record in the database (soft index)"));
	p_prop->SetData(m_i_id);
	m_i_id++; // iID = 1001
	p_group0->AddSubItem(p_prop);
	p_prop = new CMFCPropertyGridProperty(_T("total records"), static_cast<_variant_t>(records_count),
	                                      _T("number of records in the database"));
	p_prop->SetData(m_i_id);
	m_i_id++; // iID = 1002
	p_group0->AddSubItem(p_prop);
	wnd_property_list_.AddProperty(p_group0);

	// ------------------------------------------------------ database content
	const auto p_group1 = new CMFCPropertyGridProperty(_T("Acquisition"));
	p_prop->SetData(m_i_id);
	m_i_id++; // iID = 1003
	auto i_col0 = init_group_from_table(p_group1, 0);
	wnd_property_list_.AddProperty(p_group1);

	const auto p_group2 = new CMFCPropertyGridProperty(_T("Experimental conditions"));
	p_prop->SetData(m_i_id);
	m_i_id++; // iID = 1004
	i_col0 = init_group_from_table(p_group2, i_col0);
	wnd_property_list_.AddProperty(p_group2);

	const auto p_group3 = new CMFCPropertyGridProperty(_T("Stimulus"));
	p_prop->SetData(m_i_id);
	m_i_id++; // iID = 1005
	i_col0 = init_group_from_table(p_group3, i_col0);
	wnd_property_list_.AddProperty(p_group3);

	const auto p_group4 = new CMFCPropertyGridProperty(_T("Measures"));
	p_prop->SetData(m_i_id);
	//m__i_id++;		// iID = 1005
	/*i_col0 =*/
	init_group_from_table(p_group4, i_col0);
	wnd_property_list_.AddProperty(p_group4);

	if (p_database && m_p_doc_->db_get_records_count() > 0)
	{
		m_b_update_combos_ = TRUE;
		update_prop_list();
	}
}

// look at elements indexes stored into m_nCol and create property grid
// indexes are either positive or -1 (-1 is used as a stop tag)
// init all elements pointed at within m_noCol table from element i_col0 to element = -1 (stop value)
// returns next position after the stop tag

int PaneldbProperties::init_group_from_table(CMFCPropertyGridProperty* p_group, int i_col0) const
{
	const auto p_database = m_p_doc_->db_table;
	p_database->m_main_table_set.get_records_count();
	constexpr int i_col1 = sizeof(m_no_col_) / sizeof(int);
	if (i_col0 > i_col1) i_col0 = i_col1 - 1;
	int i;

	for (i = i_col0; i < i_col1; i++)
	{
		const auto i_desc_tab = m_no_col_[i];
		if (i_desc_tab < 0)
			break;

		DB_ITEMDESC desc;
		desc.cs_val = _T("undefined");
		desc.l_val = 0;
		desc.data_code_number = p_database->m_main_table_set.m_desc[i_desc_tab].data_code_number;

		CMFCPropertyGridProperty* p_prop;
		CString cs_comment;
		CString cs_title = CdbTable::m_column_properties[i_desc_tab].description;

		switch (desc.data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			cs_comment = _T("Field indirect text");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.cs_val, cs_comment, i_desc_tab);
			break;
		case FIELD_LONG:
			cs_comment = _T("Field long");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.l_val, cs_comment, i_desc_tab);
			break;
		case FIELD_TEXT:
			cs_comment = _T("Field text");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.cs_val, cs_comment, i_desc_tab);
			break;
		case FIELD_DATE:
		case FIELD_DATE_HMS:
		case FIELD_DATE_YMD:
			cs_comment = _T("Field date");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.cs_val, cs_comment, i_desc_tab);
			break;
		default:
			cs_comment = _T("Field type unknown");
			CString cs_value = cs_comment;
			p_prop = new CMFCPropertyGridProperty(cs_title, cs_value, cs_comment, i_desc_tab);
			break;
		}

		// add a few infos
		p_prop->AllowEdit(m_prop_col_[i_desc_tab]);
		p_prop->SetData(i_desc_tab);
		p_group->AddSubItem(p_prop);
	}
	return i + 1;
}

void PaneldbProperties::OnSetFocus(CWnd* p_old_wnd)
{
	CDockablePane::OnSetFocus(p_old_wnd);
	wnd_property_list_.SetFocus();
}

void PaneldbProperties::OnSettingChange(UINT u_flags, LPCTSTR lpsz_section)
{
	CDockablePane::OnSettingChange(u_flags, lpsz_section);
	set_prop_list_font();
}

void PaneldbProperties::set_prop_list_font()
{
	DeleteObject(m_fnt_prop_list_.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);
	afxGlobalData.GetNonClientMetrics(info);
	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;
	m_fnt_prop_list_.CreateFontIndirect(&lf);
	wnd_property_list_.SetFont(&m_fnt_prop_list_);
}

void PaneldbProperties::on_update_bn_edit_infos(CCmdUI* p_cmd_ui)
{
}

void PaneldbProperties::on_bn_clicked_edit_infos()
{
	m_p_doc_->update_all_views_db_wave(nullptr, HINT_GET_SELECTED_RECORDS, nullptr);
	DlgdbEditRecord dlg;
	dlg.m_pdb_doc = m_p_doc_;
	if (IDOK == dlg.DoModal())
	{
		m_p_doc_->update_all_views_db_wave(nullptr, HINT_REQUERY, nullptr);
		m_p_doc_->update_all_views_db_wave(nullptr, HINT_DOC_HAS_CHANGED, nullptr);
	}
}

void PaneldbProperties::on_update_bn_update_infos(CCmdUI* p_cmd_ui)
{
	p_cmd_ui->Enable(m_b_changed_property_);
}

void PaneldbProperties::on_bn_clicked_update_infos()
{
	const auto l_index = m_p_doc_->db_get_current_record_position();
	update_table_from_prop();
	BOOL b_success = m_p_doc_->db_set_current_record_position(l_index);
	m_p_doc_->update_all_views_db_wave(nullptr, HINT_DOC_HAS_CHANGED, nullptr);
}

LRESULT PaneldbProperties::on_property_changed(WPARAM, LPARAM l_param)
{
	m_b_changed_property_ = TRUE;
	return 0;
}

LRESULT PaneldbProperties::on_my_message(WPARAM w_param, const LPARAM l_param)
{
	switch (w_param)
	{
	case HINT_ACTIVATE_VIEW:
		m_p_doc_ = reinterpret_cast<CdbWaveDoc*>(l_param);
		if (m_p_doc_ != m_p_doc_old_)
			init_prop_list();
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
			m_p_doc_ = static_cast<CdbWaveDoc*>(p_document);
			if (m_p_doc_ != m_p_doc_old_)
			{
				m_b_update_combos_ = TRUE;
				init_prop_list();
			}
		}
		break;

	default:
		break;
	}
	return 0L;
}

void PaneldbProperties::OnUpdate(CView* p_sender, const LPARAM l_hint, CObject* p_hint)
{
	m_p_doc_ = reinterpret_cast<CdbWaveDoc*>(p_sender);
	switch (LOWORD(l_hint))
	{
	case HINT_CLOSE_FILE_MODIFIED:
		m_p_doc_old_ = nullptr;
		break;
	case HINT_REQUERY:
	case HINT_DOC_HAS_CHANGED:
	case HINT_DOC_MOVE_RECORD:
	case HINT_REPLACE_VIEW:
	default:
		init_prop_list();
		break;
	}
}
