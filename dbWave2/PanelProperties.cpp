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

/////////////////////////////////////////////////////////////////////////////
// CPropDockPane - properdies docking panel that allows trapping command messages

IMPLEMENT_DYNAMIC(CPropertiesWnd, CDockablePane)

// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameteres
int CPropertiesWnd::m_noCol[] = {
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

int CPropertiesWnd::m_propCol[] = {
	// TRUE = allow edit; list all possible columns
	FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE,
	TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE,
	TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE
};

CPropertiesWnd::CPropertiesWnd()
{
	m_wndEditInfosHeight = 0;
	m_pDoc = nullptr;
	m_pDocOld = nullptr;
	m_bUpdateCombos = FALSE;
	m_bchangedProperty = FALSE;
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_BN_CLICKED(IDC_EDITINFOS, OnBnClickedEditinfos)
	ON_UPDATE_COMMAND_UI(IDC_EDITINFOS, OnUpdateBnEditinfos)
	ON_BN_CLICKED(IDC_UPDATEINFOS, OnBnClickedUpdateinfos)
	ON_UPDATE_COMMAND_UI(IDC_UPDATEINFOS, OnUpdateBnUpdateinfos)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
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

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	const CRect rect_dummy(0, 0, 24, 24);
	//const DWORD dw_view_style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rect_dummy, this, 2))
		return -1; // fail to create
	SetPropListFont();
	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(
		m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM |
			CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::UpdatePropList()
{
	m_bchangedProperty = FALSE; // reset flag

	// database general section
	const int ipos = m_pDoc->DB_GetCurrentRecordPosition() + 1;
	const int irows = m_pDoc->DB_GetNRecords();
	if (irows == 0)
		return;

	const auto iprops = m_wndPropList.GetPropertyCount();
	const auto p_group0 = m_wndPropList.GetProperty(0);
	//int nsubitems0 = p_group0->GetSubItemsCount();
	(p_group0->GetSubItem(0)->SetValue(static_cast<_variant_t>(ipos)));
	(p_group0->GetSubItem(1)->SetValue(static_cast<_variant_t>(irows)));

	for (auto i = 1; i < iprops; i++)
	{
		const auto p_group = m_wndPropList.GetProperty(i);
		UpdateGroupPropFromTable(p_group);
	}
	m_bUpdateCombos = FALSE;
}

void CPropertiesWnd::UpdateGroupPropFromTable(CMFCPropertyGridProperty* pGroup) const
{
	auto p_db = m_pDoc->m_pDB;
	DB_ITEMDESC desc;
	const auto n_sub_items = pGroup->GetSubItemsCount();
	CdbTableAssociated* p2linkedSet;

	for (auto i = 0; i < n_sub_items; i++)
	{
		auto p_prop = pGroup->GetSubItem(i);
		const int i_column = p_prop->GetData();
		p_db->GetRecordItemValue(i_column, &desc);
		p_prop->ResetOriginalValue();
		switch (p_db->m_mainTableSet.m_desc[i_column].data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			p_prop->SetValue(desc.csVal);
			p_prop->SetOriginalValue(desc.csVal);
			p2linkedSet = p_db->m_mainTableSet.m_desc[i_column].plinkedSet;
			if (m_bUpdateCombos || (p_prop->GetOptionCount() != p2linkedSet->GetRecordCount()))
			{
				p_prop->RemoveAllOptions();
				COleVariant var_value1;
				if (p2linkedSet->IsOpen() && !p2linkedSet->IsBOF())
				{
					COleVariant var_value0;
					p2linkedSet->MoveFirst();
					while (!p2linkedSet->IsEOF())
					{
						p2linkedSet->GetFieldValue(0, var_value0);
						CString cs = var_value0.bstrVal;
						if (!cs.IsEmpty())
							p_prop->AddOption(cs, TRUE);
						p2linkedSet->MoveNext();
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

void CPropertiesWnd::UpdateTableFromProp()
{
	auto p_database = m_pDoc->m_pDB;
	auto p_maintable_set = &p_database->m_mainTableSet;
	m_bchangedProperty = FALSE; // reseet flag
	p_maintable_set->Edit();

	const auto iprops = m_wndPropList.GetPropertyCount();
	for (auto i = 1; i < iprops; i++)
	{
		const auto p_group = m_wndPropList.GetProperty(i);
		UpdateTableFromGroupProp(p_group);
	}
	p_maintable_set->Update();
}

void CPropertiesWnd::UpdateTableFromGroupProp(CMFCPropertyGridProperty* pGroup)
{
	auto p_database = m_pDoc->m_pDB;
	const auto nsubitems = pGroup->GetSubItemsCount();

	for (auto i = 0; i < nsubitems; i++)
	{
		auto p_prop = pGroup->GetSubItem(i);
		if (!p_prop->IsModified())
			continue;

		const int icol = p_prop->GetData();
		const auto prop_val = p_prop->GetValue();
		auto pdesc = p_database->GetRecordItemDescriptor(icol);
		if (pdesc == nullptr)
			continue;

		switch (pdesc->data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			pdesc->csVal = prop_val.bstrVal;
			p_prop->ResetOriginalValue();
			p_prop->SetOriginalValue(pdesc->csVal);
			p_prop->SetValue(pdesc->csVal);
			break;
		case FIELD_TEXT:
			pdesc->csVal = prop_val.bstrVal;
			p_prop->ResetOriginalValue();
			p_prop->SetOriginalValue(pdesc->csVal);
			p_prop->SetValue(pdesc->csVal);
			break;
		case FIELD_LONG:
			pdesc->lVal = prop_val.lVal;
			p_prop->ResetOriginalValue();
			p_prop->SetOriginalValue(prop_val.lVal);
			p_prop->SetValue(prop_val.lVal);
			break;
		//case FIELD_DATE:
		default:
			break;
		}
		p_database->SetRecordItemValue(icol, pdesc);
	}
}

#define ID_BASE	1000

void CPropertiesWnd::InitPropList()
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

	// house keeping
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea(TRUE);
	m_wndPropList.SetVSDotNetLook(TRUE);
	m_wndPropList.SetGroupNameFullWidth(TRUE);
	m_wndPropList.MarkModifiedProperties(TRUE, TRUE);

	// get pointer to database specific object which contains recordsets
	auto p_database = m_pDoc->m_pDB;
	auto m__i_id = ID_BASE;

	// ------------------------------------------------------
	auto p_group0 = new CMFCPropertyGridProperty(_T("Database"));
	p_group0->SetData(m__i_id);
	m__i_id++; // iID = 1000
	const int ipos = p_database->m_mainTableSet.GetAbsolutePosition() + 1;
	const int irows = p_database->m_mainTableSet.GetNRecords();
	auto p_prop = new CMFCPropertyGridProperty(_T("current record"), static_cast<_variant_t>(ipos),
	                                           _T("current record in the database (soft index)"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1001
	p_group0->AddSubItem(p_prop);
	p_prop = new CMFCPropertyGridProperty(_T("total records"), static_cast<_variant_t>(irows),
	                                      _T("number of records in the database"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1002
	p_group0->AddSubItem(p_prop);
	m_wndPropList.AddProperty(p_group0);

	// ------------------------------------------------------ database content
	const auto p_group1 = new CMFCPropertyGridProperty(_T("Acquisition"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1003
	auto icol0 = InitGroupFromTable(p_group1, 0);
	m_wndPropList.AddProperty(p_group1);

	const auto p_group2 = new CMFCPropertyGridProperty(_T("Experimental conditions"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1004
	icol0 = InitGroupFromTable(p_group2, icol0);
	m_wndPropList.AddProperty(p_group2);

	const auto p_group3 = new CMFCPropertyGridProperty(_T("Stimulus"));
	p_prop->SetData(m__i_id);
	m__i_id++; // iID = 1005
	icol0 = InitGroupFromTable(p_group3, icol0);
	m_wndPropList.AddProperty(p_group3);

	const auto p_group4 = new CMFCPropertyGridProperty(_T("Measures"));
	p_prop->SetData(m__i_id);
	//m__i_id++;		// iID = 1005
	/*icol0 =*/
	InitGroupFromTable(p_group4, icol0);
	m_wndPropList.AddProperty(p_group4);

	if (p_database && m_pDoc->DB_GetNRecords() > 0)
	{
		m_bUpdateCombos = TRUE;
		UpdatePropList();
	}
}

// look at elements indexes stored into m_nCol and create property grid
// indexes are either positive or -1 (-1 is used as a stop tag)
// init all elements pointed at within m_noCol table from element icol0 to element = -1 (stop value)
// returns next position after the stop tag

int CPropertiesWnd::InitGroupFromTable(CMFCPropertyGridProperty* pGroup, int icol0)
{
	auto p_database = m_pDoc->m_pDB;
	/*int nrecords = */
	p_database->m_mainTableSet.GetNRecords();
	const int icol1 = sizeof(m_noCol) / sizeof(int);
	if (icol0 > icol1) icol0 = icol1 - 1;
	int i;

	for (i = icol0; i < icol1; i++)
	{
		const auto idesctab = m_noCol[i];
		if (idesctab < 0)
			break;

		DB_ITEMDESC desc;
		desc.csVal = _T("undefined");
		desc.lVal = 0;
		desc.data_code_number = p_database->m_mainTableSet.m_desc[idesctab].data_code_number;

		CMFCPropertyGridProperty* p_prop = nullptr;
		CString cs_comment;
		CString cs_title = CdbTable::m_desctab[idesctab].description;

		switch (desc.data_code_number)
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			cs_comment = _T("Field indirect text");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.csVal, cs_comment, idesctab);
			break;
		case FIELD_LONG:
			cs_comment = _T("Field long");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.lVal, cs_comment, idesctab);
			break;
		case FIELD_TEXT:
			cs_comment = _T("Field text");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.csVal, cs_comment, idesctab);
			break;
		case FIELD_DATE:
		case FIELD_DATE_HMS:
		case FIELD_DATE_YMD:
			cs_comment = _T("Field date");
			p_prop = new CMFCPropertyGridProperty(cs_title, desc.csVal, cs_comment, idesctab);
			break;
		default:
			cs_comment = _T("Field type unknown");
			CString csValue = cs_comment;
			p_prop = new CMFCPropertyGridProperty(cs_title, csValue, cs_comment, idesctab);
			break;
		}

		// add a few infos
		p_prop->AllowEdit(m_propCol[idesctab]);
		p_prop->SetData(idesctab);
		pGroup->AddSubItem(p_prop);
	}
	return i + 1;
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
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

void CPropertiesWnd::OnUpdateBnEditinfos(CCmdUI* pCmdUI)
{
}

void CPropertiesWnd::OnBnClickedEditinfos()
{
	m_pDoc->UpdateAllViews(nullptr, HINT_GETSELECTEDRECORDS, nullptr);
	DlgdbEditRecord dlg;
	dlg.m_pdbDoc = m_pDoc;
	if (IDOK == dlg.DoModal())
	{
		m_pDoc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
		m_pDoc->UpdateAllViews(nullptr, HINT_DOCHASCHANGED, nullptr);
	}
}

void CPropertiesWnd::OnUpdateBnUpdateinfos(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bchangedProperty);
}

void CPropertiesWnd::OnBnClickedUpdateinfos()
{
	const auto l_index = m_pDoc->DB_GetCurrentRecordPosition();
	UpdateTableFromProp();
	m_pDoc->DB_SetCurrentRecordPosition(l_index);
	m_pDoc->UpdateAllViews(nullptr, HINT_DOCHASCHANGED, nullptr);
}

LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM, LPARAM lParam)
{
	//auto p_prop = reinterpret_cast<CMFCPropertyGridProperty*>(lParam);
	m_bchangedProperty = TRUE;
	return 0;
}

LRESULT CPropertiesWnd::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	//auto p_app = (CdbWaveApp*)AfxGetApp();
	//short lowp = LOWORD(lParam);
	//short highp = HIWORD(lParam);

	switch (wParam)
	{
	case HINT_ACTIVATEVIEW:
		m_pDoc = reinterpret_cast<CdbWaveDoc*>(lParam);
		if (m_pDoc != m_pDocOld)
			InitPropList();
		break;

	case HINT_MDIACTIVATE:
		{
			auto* pmain = static_cast<CMDIFrameWndEx*>(AfxGetMainWnd());
			BOOL b_maximized;
			auto p_child = pmain->MDIGetActive(&b_maximized);
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

void CPropertiesWnd::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	m_pDoc = reinterpret_cast<CdbWaveDoc*>(pSender);

	switch (LOWORD(lHint))
	{
	case HINT_REQUERY:
	case HINT_DOCHASCHANGED:
	case HINT_DOCMOVERECORD:
		InitPropList();
		break;

	case HINT_CLOSEFILEMODIFIED: // save current file parms
		m_pDocOld = nullptr;
		break;

	case HINT_REPLACEVIEW:
	default:
		InitPropList();
		break;
	}
}
