
#include "StdAfx.h"

#include "PanelProperties.h"
#include "resource.h"
//#include "MainFrm.h"
#include "dbWave.h"
#include "dbEditRecordDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropDockPane - properdies docking panel that allows trapping command messages

IMPLEMENT_DYNAMIC(CPropertiesWnd, CDockablePane)

// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameteres
int CPropertiesWnd::m_noCol[] =  {	
	// ------1
	CH_ACQDATE_DAY,	
	CH_ACQDATE_TIME, 
	CH_DATALEN, 
	CH_PATH_ID, 
	CH_FILENAME,
	CH_PATH2_ID, 
	CH_FILESPK,	
	CH_ACQ_COMMENTS,
	-1,							// 0-6 acquisition: acq_date, datalen, path_ID, filename, path2_ID, filespk, acq_comment 
	// ------2
	CH_IDINSECT, 
	CH_IDSENSILLUM, 
	CH_INSECT_ID,
	CH_SENSILLUM_ID,
	CH_LOCATION_ID, 
	CH_STRAIN_ID, 
	CH_SEX_ID, 
	CH_OPERATOR_ID,	
	-1,							// 7-14 experiment: insectID, sensillumID, insectname_ID, sensillumname_ID, location_ID, strain_ID, sex_ID, operator_ID
	// ------3
	CH_EXPT_ID,	
	CH_STIM_ID,	CH_CONC_ID,	CH_REPEAT,		
	CH_STIM2_ID, CH_CONC2_ID, CH_REPEAT2,			
	-1,							// 15-21 stimulus: expt_ID, stim_ID, conc_ID, repeat, stim2_ID, conc2_ID, repeat2
	// ------4
	CH_NSPIKES,	
	CH_NSPIKECLASSES, 
	CH_FLAG, 
	CH_MORE,
	-1};						// 22-25 measures: n spikes, spikeclasses, flag, more

int CPropertiesWnd::m_propCol[] ={ 	// TRUE = allow edit; list all possible columns
	FALSE,	FALSE,	FALSE,	FALSE,	TRUE,	TRUE,	TRUE,
	TRUE,	FALSE,	FALSE,	FALSE,	TRUE,	TRUE,	TRUE,	TRUE,
	TRUE,	TRUE,	TRUE,	TRUE,	TRUE,	TRUE,	TRUE,	
	TRUE,	TRUE,	TRUE,	TRUE,	FALSE,	FALSE,	TRUE};


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
	if (GetSafeHwnd () == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
		return;

	CRect rectClient;
	GetClientRect(rectClient);
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, 
		rectClient.top + m_wndEditInfosHeight, 
		rectClient.Width(), 
		cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rectClient.left, 
		rectClient.top + m_wndEditInfosHeight + cyTlb, 
		rectClient.Width(), 
		rectClient.Height() - m_wndEditInfosHeight-cyTlb,
		SWP_NOACTIVATE | SWP_NOZORDER);
}


int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy (0, 0, 24, 24);
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
		return -1;      // fail to create
	SetPropListFont();
	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
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
	m_bchangedProperty = FALSE;		// reset flag

	// database general section
	int ipos = m_pDoc->DBGetCurrentRecordPosition() + 1;
	int irows = m_pDoc->DBGetNRecords();
	if (irows == 0)
		return;

	int iprops = m_wndPropList.GetPropertyCount();
	CMFCPropertyGridProperty* pGroup0 = m_wndPropList.GetProperty(0);
	int nsubitems0 = pGroup0->GetSubItemsCount();
	(pGroup0->GetSubItem(0)->SetValue((_variant_t)ipos));
	(pGroup0->GetSubItem(1)->SetValue((_variant_t)irows));

	CMFCPropertyGridProperty* pGroup;
	for (int i=1; i < iprops; i++)
	{
		pGroup = m_wndPropList.GetProperty(i);
		UpdateGroupPropFromTable(pGroup);
	}
	m_bUpdateCombos = FALSE;
}

void CPropertiesWnd::UpdateGroupPropFromTable(CMFCPropertyGridProperty* pGroup)
{
	CdbWdatabase* pDB = m_pDoc->m_pDB;
	DB_ITEMDESC desc;
	int nsubitems = pGroup->GetSubItemsCount();
	CdbIndexTable* p2linkedSet;

	for (int i = 0; i < nsubitems; i++) 
	{
		CMFCPropertyGridProperty* pProp = pGroup->GetSubItem(i);
		int icol = pProp->GetData();
		pDB->GetRecordItemValue(icol, &desc);
		pProp->ResetOriginalValue();
		switch (pDB->m_mainTableSet.m_desc[icol].typeLocal) 
		{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			pProp->SetValue(desc.csVal);
			pProp->SetOriginalValue(desc.csVal);
			p2linkedSet = pDB->m_mainTableSet.m_desc[icol].plinkedSet;
			if (m_bUpdateCombos || (pProp->GetOptionCount() != p2linkedSet->GetRecordCount()))
			{
				pProp->RemoveAllOptions();
				COleVariant varValue0, varValue1;
				BOOL flag;
				if (p2linkedSet->IsOpen() && !p2linkedSet->IsBOF())
				{
					p2linkedSet->MoveFirst();
					while (!p2linkedSet->IsEOF())
					{
						p2linkedSet->GetFieldValue(0, varValue0);
						CString cs = varValue0.bstrVal;
						if (!cs.IsEmpty())
							flag = pProp->AddOption(cs, TRUE);
						p2linkedSet->MoveNext();
					}
				}
			}
			break;
		case FIELD_LONG:
			pProp->SetValue(desc.lVal);
			pProp->SetOriginalValue(desc.lVal);
			//if (icol == CH_REPEAT)
			//	ATLTRACE2("value read = %i\n", pdesc->lVal);
			break;
		case FIELD_TEXT:
			pProp->SetValue(desc.csVal);
			pProp->SetOriginalValue(desc.csVal);
			break;
		case FIELD_DATE_HMS:
			desc.csVal = desc.oVal.Format(VAR_TIMEVALUEONLY);
			pProp->SetValue(desc.csVal);
			pProp->SetOriginalValue(desc.csVal);
			break;
		case FIELD_DATE_YMD:
			desc.csVal = desc.oVal.Format(VAR_DATEVALUEONLY);
			pProp->SetValue(desc.csVal);
			pProp->SetOriginalValue(desc.csVal);
			break;

		default:
			break;
		}
	}
}

void CPropertiesWnd::UpdateTableFromProp()
{
	CdbWdatabase* pDB = m_pDoc->m_pDB;
	CdbMainTable* pSet = &pDB->m_mainTableSet;
	m_bchangedProperty = FALSE;		// reseet flag
	pSet->Edit();

	int iprops = m_wndPropList.GetPropertyCount();
	CMFCPropertyGridProperty* pGroup;
	for (int i = 1; i < iprops; i++)
	{
		pGroup = m_wndPropList.GetProperty(i);
		UpdateTableFromGroupProp(pGroup);
	}
	pSet->Update();
}

void CPropertiesWnd::UpdateTableFromGroupProp(CMFCPropertyGridProperty* pGroup)
{
	CdbWdatabase* pDB = m_pDoc->m_pDB;
	DB_ITEMDESC* pdesc;
	int nsubitems = pGroup->GetSubItemsCount();

	for (int i = 0; i < nsubitems; i++) {
		CMFCPropertyGridProperty* pProp = pGroup->GetSubItem(i);
		if (!pProp->IsModified())
			continue;

		int icol = pProp->GetData();
		COleVariant propVal = pProp->GetValue();
		pdesc = pDB->GetRecordItemDescriptor(icol);
		if (pdesc == nullptr)
			continue;

		switch (pdesc->typeLocal)
		{
			case FIELD_IND_TEXT:
			case FIELD_IND_FILEPATH:
				pdesc->csVal = propVal.bstrVal;
				pProp->ResetOriginalValue();
				pProp->SetOriginalValue(pdesc->csVal);
				pProp->SetValue(pdesc->csVal);
				break;
			case FIELD_TEXT:
				pdesc->csVal = propVal.bstrVal;
				pProp->ResetOriginalValue();
				pProp->SetOriginalValue(pdesc->csVal);
				pProp->SetValue(pdesc->csVal);
				break;
			case FIELD_LONG:
				pdesc->lVal = propVal.lVal;
				pProp->ResetOriginalValue();
				pProp->SetOriginalValue(propVal.lVal);
				pProp->SetValue(propVal.lVal);
				break;
			//case FIELD_DATE:
			default:
				break;
		}
		pDB->SetRecordItemValue(icol, pdesc);
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
	CdbWdatabase* pDB = m_pDoc->m_pDB;
	int m_iID = ID_BASE;

	// ------------------------------------------------------
	CMFCPropertyGridProperty* pGroup0 = new CMFCPropertyGridProperty(_T("Database"));
	pGroup0->SetData(m_iID); 
	m_iID++;	// iID = 1000
	int ipos = pDB->m_mainTableSet.GetAbsolutePosition() +1;
	int irows = pDB->m_mainTableSet.GetNRecords();
	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("current record"), (_variant_t)ipos, _T("current record in the database (soft index)"));
	pProp->SetData(m_iID); 
	m_iID++;		// iID = 1001
	pGroup0->AddSubItem(pProp);
	pProp = new CMFCPropertyGridProperty(_T("total records"), (_variant_t)irows, _T("number of records in the database"));
	pProp->SetData(m_iID); 
	m_iID++;		// iID = 1002
	pGroup0->AddSubItem(pProp);
	m_wndPropList.AddProperty(pGroup0);

	// ------------------------------------------------------ database content
	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("Acquisition"));
	pProp->SetData(m_iID);
	m_iID++;		// iID = 1003
	int icol0 = InitGroupFromTable(pGroup1, 0);
	m_wndPropList.AddProperty(pGroup1);

	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("Experimental conditions"));
	pProp->SetData(m_iID); 
	m_iID++;		// iID = 1004
	icol0 = InitGroupFromTable(pGroup2, icol0);
	m_wndPropList.AddProperty(pGroup2);

	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(_T("Stimulus"));
	pProp->SetData(m_iID); 
	m_iID++;		// iID = 1005
	icol0 = InitGroupFromTable(pGroup3, icol0);
	m_wndPropList.AddProperty(pGroup3);

	CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(_T("Measures"));
	pProp->SetData(m_iID); 
	m_iID++;		// iID = 1005
	icol0 = InitGroupFromTable(pGroup4, icol0);
	m_wndPropList.AddProperty(pGroup4);

	if (pDB && m_pDoc->DBGetNRecords()>0)
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
	CdbWdatabase* pDB = m_pDoc->m_pDB;
	int nrecords = pDB->m_mainTableSet.GetNRecords();
	int icol1 = sizeof(m_noCol) / sizeof(int);
	if (icol0 > icol1) icol0 = icol1-1;
	int i;

	for (i = icol0; i < icol1; i++)
	{
		int idesctab = m_noCol[i];
		if (idesctab < 0)
			break; 
		
		DB_ITEMDESC desc;
		desc.csVal = _T("undefined");
		desc.lVal = 0;
		desc.typeLocal = pDB->m_mainTableSet.m_desc[idesctab].typeLocal;
	
		CMFCPropertyGridProperty* pProp;
		CString csComment;
		CString csTitle = pDB->m_desctab[idesctab].szDescriptor;
	
		switch (desc.typeLocal) {
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			csComment = _T("Field indirect text");
			pProp = new CMFCPropertyGridProperty(csTitle, desc.csVal, csComment, idesctab);
			break;
		case FIELD_LONG:
			csComment = _T("Field long");
			pProp = new CMFCPropertyGridProperty(csTitle, desc.lVal, csComment, idesctab);
			break;
		case FIELD_TEXT:
			csComment = _T("Field text");
			pProp = new CMFCPropertyGridProperty(csTitle, desc.csVal, csComment, idesctab);
			break;
		case FIELD_DATE:
		case FIELD_DATE_HMS:
		case FIELD_DATE_YMD:
			csComment = _T("Field date");
			pProp = new CMFCPropertyGridProperty(csTitle, desc.csVal, csComment, idesctab);
			break;
		default:
			csComment = _T("Field type unknown");
			CString csValue = csComment;
			pProp = new CMFCPropertyGridProperty(csTitle, csValue, csComment, idesctab);
			break;
		}

		// add a few infos
		pProp->AllowEdit(m_propCol[idesctab]);
		pProp->SetData(idesctab);
		pGroup->AddSubItem(pProp);
	}
	return i+1;
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
	::DeleteObject(m_fntPropList.Detach());

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
	CdbEditRecordDlg dlg;
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
	long lIndex = m_pDoc->DBGetCurrentRecordPosition();
	UpdateTableFromProp();
	m_pDoc->DBSetCurrentRecordPosition(lIndex);
	m_pDoc->UpdateAllViews(nullptr, HINT_DOCHASCHANGED, nullptr);
}

LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM, LPARAM lParam)
{
	CMFCPropertyGridProperty* pProp = (CMFCPropertyGridProperty*)lParam;
	m_bchangedProperty = TRUE;
	return 0;
}

LRESULT CPropertiesWnd::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	CdbWaveApp* p_app = (CdbWaveApp*)AfxGetApp();
	short lowp = LOWORD(lParam);
	short highp = HIWORD(lParam);

	switch (wParam)
	{
	case HINT_ACTIVATEVIEW:
		m_pDoc = (CdbWaveDoc*)lParam;
		if (m_pDoc != m_pDocOld)
			InitPropList();
		break;

	case HINT_MDIACTIVATE:
		{	
			CMDIFrameWndEx* pmain = (CMDIFrameWndEx*)AfxGetMainWnd();
			BOOL bMaximized;
			CMDIChildWnd* pChild = pmain->MDIGetActive(&bMaximized);
			if (!pChild) return NULL;
			CDocument* pDoc = pChild->GetActiveDocument();
			if (!pDoc || !pDoc->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)))
				return NULL;
			m_pDoc = (CdbWaveDoc*) pDoc;
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
	m_pDoc = (CdbWaveDoc*)pSender;

	switch (LOWORD(lHint))
	{
	case HINT_REQUERY:
	case HINT_DOCHASCHANGED:
	case HINT_DOCMOVERECORD:
		InitPropList();
		break;

	case HINT_CLOSEFILEMODIFIED:	// save current file parms 
		m_pDocOld = nullptr;
		break;

	case HINT_REPLACEVIEW:
	default:
		InitPropList();
		break;
	}
}

