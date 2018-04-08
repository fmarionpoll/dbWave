
#include "stdafx.h"

#include "mainfrm.h"
#include "Resource.h"
#include "PanelFilter.h"
#include "progdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilterPanel

// the numbers here are those of m_pszTableCol - they define the order of appearance of the different parameteres
int CFilterWnd::m_noCol[] = { 
	CH_EXPT_ID,	
	CH_IDINSECT, CH_IDSENSILLUM, CH_INSECT_ID, CH_SENSILLUM_ID, 
	CH_LOCATION_ID, CH_STRAIN_ID, CH_SEX_ID, CH_OPERATOR_ID,
	CH_STIM_ID, CH_CONC_ID,	CH_REPEAT, CH_STIM2_ID, CH_CONC2_ID, CH_REPEAT2,
	CH_FLAG, CH_ACQDATE_DAY, -1 
};		

CFilterWnd::CFilterWnd()
{
}

CFilterWnd::~CFilterWnd()
{
}

BEGIN_MESSAGE_MAP(CFilterWnd, CDockablePane)
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

	//ON_TVN_CHECK(				IDC_TREE1, OnTvnCheckTree)
	//ON_NOTIFY(TVN_ITEMCHANGED,IDC_TREE1, OnTvnItemchangedTree)
	//ON_NOTIFY(NM_CLICK,		IDC_TREE1, OnNMClickTree)
	//ON_NOTIFY(TVN_KEYDOWN,	IDC_TREE1, OnTvnKeydownTree)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CFilterWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS; 
	if (!m_wndFilterView.Create(dwViewStyle, rectDummy, this, IDC_TREE1))
		return -1;      // fail to create

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.ReplaceButton(ID_RECORD_SORT, CMFCToolBarComboBoxButton(ID_RECORD_SORT, /*GetCmdMgr()->GetCmdImage(ID_RECORD_SORT)*/ NULL, CBS_DROPDOWN));

	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);		// All commands will be routed via this control , not via the parent frame:

	AdjustLayout();
	return 0;
}

void CFilterWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFilterWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndFilterView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CFilterWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
		return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFilterView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFilterWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndFilterView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFilterWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndFilterView.SetFocus();
}

void CFilterWnd::OnUpdateTree()
{
	m_pDocOld = NULL;
	InitFilterList();
}

LRESULT CFilterWnd::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	CdbWaveApp* pApp = (CdbWaveApp*)AfxGetApp();
	short lowp = LOWORD(lParam);
	short highp = HIWORD(lParam);

	switch (wParam)
	{
	case HINT_ACTIVATEVIEW:
		//ATLTRACE2(_T("CPropDockPane: OnMyMessage ACTIVATEVIEW\n"));
		m_pDoc = (CdbWaveDoc*)lParam;
		if (m_pDoc != m_pDocOld)
			InitFilterList();
		break;

	case HINT_MDIACTIVATE:
		{	//ATLTRACE2(_T("CPropDockPane: OnMyMessage HINT_MDIACTIVATE\n"));
		CMainFrame* pmain = (CMainFrame*)AfxGetMainWnd();
		BOOL bMaximized;
		CMDIChildWnd* pChild = pmain->MDIGetActive(&bMaximized);
		if (!pChild) 
			return NULL;
		CDocument* pDoc = pChild->GetActiveDocument();
		if (!pDoc || !pDoc->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)))
			return NULL;
		m_pDoc = (CdbWaveDoc*)pDoc;
		InitFilterList();
		}
		break;

	default:
		break;
	}
	return 0L;
}

void CFilterWnd::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	m_pDoc = (CdbWaveDoc*)pSender;
	switch (LOWORD(lHint))
	{
	case HINT_CLOSEFILEMODIFIED:	// save current file parms 
		//m_pDocOld = NULL;
		break;

	case HINT_REQUERY:
		m_pDocOld = NULL;

	case HINT_DOCHASCHANGED:
	case HINT_DOCMOVERECORD:
	case HINT_REPLACEVIEW:
	default:
		InitFilterList();
		break;
	}
}

void CFilterWnd::InitFilterList()
{
	if (m_pDocOld == m_pDoc)
		return;
	m_pDocOld = m_pDoc;

	CdbWdatabase* pDB = m_pDoc->m_pDB;
	ASSERT(pDB);

	// setup dialog box
	CProgressDlg dlg;
	dlg.Create();
	int istep = 0;
	dlg.SetStep(1);
	
	// fill items of the combo (column heads to sort data)
	dlg.SetStatus(_T("List categories available..."));
	CMFCToolBarComboBoxButton* pCombo = (CMFCToolBarComboBoxButton*)m_wndToolBar.GetButton(3);
	ASSERT(ID_RECORD_SORT == m_wndToolBar.GetItemID(3));
	if (pCombo->GetCount() <= 0)
	{
		int k;
		for (int i = 0; i < NTABLECOLS; i++)
			k = pCombo->AddSortedItem(pDB->m_desctab[i].szDescriptor, i);
	}
	int isel = pCombo->SelectItem(pDB->m_tableSet.m_strSort);

	// fill items of the tree
	dlg.SetStatus(_T("Populate categories..."));
	if (pDB->m_tableSet.IsBOF() && pDB->m_tableSet.IsEOF())
		return;
	m_wndFilterView.LockWindowUpdate();		// prevent screen upate (and flicker)
	if (m_wndFilterView.GetCount() > 0)
		m_wndFilterView.DeleteAllItems();

	int i = 0;
	pDB->m_tableSet.BuildAndSortIDArrays();

	CString csComment;
	while (m_noCol[i] > 0)
	{
		csComment.Format(_T("category %i"), i);
		dlg.SetStatus(csComment);

		int icol = m_noCol[i];
		DB_ITEMDESC* pdesc = pDB->GetRecordItemDescriptor(icol);
		m_htreeitem[i] = m_wndFilterView.InsertItem(pDB->m_desctab[icol].szDescriptor, TVI_ROOT); //hRoot);
		m_wndFilterView.SetItemData(m_htreeitem[i], m_noCol[i]);		// save table index into head of the list

		// collect data (array of unique descriptors)
		switch (pdesc->typeLocal) 
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
		int isum = 0;
		HTREEITEM htreeItem = NULL;
		TVCS_CHECKSTATE bcheck;
		int nitems = 0;
		for (int j = 0; j < pdesc->csElementsArray.GetSize(); j++)
		{
			csComment.Format(_T("create subitem %i"), j);
			dlg.SetStatus(csComment);

			htreeItem = m_wndFilterView.InsertItem(pdesc->csElementsArray.GetAt(j), m_htreeitem[i]);
			bcheck = TVCS_CHECKED;
			CString csElmtj = pdesc->csElementsArray.GetAt(j);
			if (pdesc->bFilter2)
			{
				bcheck = TVCS_UNCHECKED;
				for (int k = 0; k< pdesc->csfilterParam2.GetSize(); k++)
				{
					if (csElmtj.CompareNoCase(pdesc->csfilterParam2.GetAt(k)) == 0)
					{
						bcheck = TVCS_CHECKED;
						break;
					}
				}
			}
			else if (pdesc->bFilter1)
			{
				if (csElmtj.CompareNoCase(pdesc->csfilterParam1) != 0)
					bcheck = TVCS_UNCHECKED;
			}
			m_wndFilterView.SetCheck(htreeItem, bcheck);
			isum += bcheck;	// count number of positive checks (no check=0, check = 1)
			nitems++;
		}
		// trick needed here because if the first item is checked and not the others, then the parent stays in the initial state
		// namely "checked" (because at that moment it did not have other children)
		if (isum == 1 && htreeItem != NULL)
		{
			m_wndFilterView.SetCheck(htreeItem, TVCS_CHECKED);
			m_wndFilterView.SetCheck(htreeItem, bcheck);
		}
		if (isum < nitems)
			m_wndFilterView.Expand(m_htreeitem[i], TVE_EXPAND);
		i++;
	}
	m_wndFilterView.UnlockWindowUpdate();
}

void CFilterWnd::PopulateItemFromTableLong(DB_ITEMDESC* pdesc)
{
	CString cs;		// to construct insect and sensillum number (for example)
	CString str;	// to store FindFirst filter
	CdbMainTable* pSet = &m_pDoc->m_pDB->m_tableSet;
	CString cscolhead = pdesc->csColName;
	int arraySize = pdesc->liArray.GetSize();
	if (pdesc->bFilter2)
	{
		return;
	}
	else if (pdesc->bFilter1)
	{
		pdesc->csfilterParam1.Format(_T("%i"), pdesc->lfilterParam1);
	}
	else
	{
		pdesc->csElementsArray.RemoveAll();
		for (int i = 0; i < arraySize; i++)
		{
			long iID = pdesc->liArray.GetAt(i);
			// add string only if found into pSet...
			str.Format(_T("%s=%li"), (LPCTSTR) cscolhead, iID);
			BOOL flag = pSet->FindFirst(str);
			if (flag != 0)
			{
				cs.Format(_T("%i"), iID);
				pdesc->csElementsArray.Add(cs);
				if (pdesc->bFilter1 && pdesc->lfilterParam1 != iID)
				{
					pdesc->csfilterParam1.Format(_T("%i"), iID);
				}
			}
		}
	}
}

void CFilterWnd::PopulateItemFromLinkedTable(DB_ITEMDESC* pdesc)
{
	CString cs;
	CString str2 = pdesc->csColName;
	ASSERT(!str2.IsEmpty());

	CdbIndexTable* plinkedSet = pdesc->plinkedSet;
	CdbMainTable* pSet = &m_pDoc->m_pDB->m_tableSet;
	if (pdesc->bFilter2)
	{
		return;
	}
	else if (pdesc->bFilter1)
	{
		pdesc->csfilterParam1 = plinkedSet->GetStringFromID(pdesc->lfilterParam1);
	}
	else
	{
		// loop over the whole content of the attached table
		pdesc->csElementsArray.RemoveAll();
		pdesc->liArray.RemoveAll();
		if (plinkedSet->IsOpen() && !plinkedSet->IsBOF())
		{
			COleVariant varValue0, varValue1;
			plinkedSet->MoveFirst();
			while (!plinkedSet->IsEOF())
			{
				plinkedSet->GetFieldValue(0, varValue0);
				plinkedSet->GetFieldValue(1, varValue1);
				long iID = varValue1.lVal;
				// add string only if found into pSet...
				cs.Format(_T("%s=%li"), (LPCTSTR) pdesc->csColName, iID);
				BOOL flag = pSet->FindFirst(cs);
				if (flag != 0)
				{
					InsertAlphabetic(varValue0.bstrVal, pdesc->csElementsArray);
					pdesc->liArray.Add(iID);
				}
				plinkedSet->MoveNext();
			}
		}
	}
}

void CFilterWnd::PopulateItemFromTablewithDate(DB_ITEMDESC* pdesc)
{
	CString cs;		// to construct date
	CString cscolhead = pdesc->csColName;
	CString str;	// to construct filter
	CdbMainTable* pSet = &m_pDoc->m_pDB->m_tableSet;
	int arraySize = pSet->m_desc[CH_ACQDATE_DAY].tiArray.GetSize();

	if (pdesc->bFilter2)
	{
		return;
	}
	else if (pdesc->bFilter1)
	{
		cs = pdesc->otfilterParam1.Format(VAR_DATEVALUEONLY);
		pdesc->csfilterParam1 = cs;
	}
	else
	{
		pdesc->csElementsArray.RemoveAll();
		for (int i = 0; i < arraySize; i++)
		{
			COleDateTime oTime = pSet->m_desc[CH_ACQDATE_DAY].tiArray.GetAt(i);
			cs = oTime.Format(_T("%m/%d/%y"));		// filter needs to be constructed as month-day-year
			str.Format(_T("%s=#%s#"), (LPCTSTR) cscolhead, (LPCTSTR) cs);
			BOOL flag = pSet->FindFirst(str);
			if (flag != 0)							// add string only if found into pSet...
			{
				cs = oTime.Format(VAR_DATEVALUEONLY);
				pdesc->csElementsArray.Add(cs);
			}
		}
	}
}

void  CFilterWnd::InsertAlphabetic(CString cs, CStringArray &csArray)
{
	int i = 0;
	for (i; i < csArray.GetSize(); i++)
	{
		CString cscomp = csArray.GetAt(i);
		int j = cs.CompareNoCase(cscomp);
		if (j < 0)
			break;
	}
	csArray.InsertAt(i, cs);
}

void CFilterWnd::BuildFilterItemIndirectionFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem)
{
	int i = 0;
	for (HTREEITEM item = startItem; item != NULL; item = m_wndFilterView.GetNextItem(item, TVGN_NEXT), i++)
	{
		TVCS_CHECKSTATE state = m_wndFilterView.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			CString cs = m_wndFilterView.GetItemText(item);
			for (int j = 0; j < pdesc->liArray.GetSize(); j++)
			{
				long li = pdesc->liArray.GetAt(j);
				CString str = pdesc->plinkedSet->GetStringFromID(li);
				if (str == cs)
				{
					pdesc->lfilterParam2.Add(li);
					pdesc->csfilterParam2.Add(cs);
					break;
				}
			}
		}
	}
}

void CFilterWnd::BuildFilterItemLongFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem)
{
	
	int i = 0;
	for (HTREEITEM item = startItem; item != NULL; item = m_wndFilterView.GetNextItem(item, TVGN_NEXT), i++)
	{
		TVCS_CHECKSTATE state = m_wndFilterView.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			CString cs = m_wndFilterView.GetItemText(item);
			long li = pdesc->liArray.GetAt(i);
			pdesc->lfilterParam2.Add(li);
			pdesc->csfilterParam2.Add(cs);
		}
	}
}

void CFilterWnd::BuildFilterItemDateFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem)
{
	int i = 0;
	for (HTREEITEM item = startItem; item != NULL; item = m_wndFilterView.GetNextItem(item, TVGN_NEXT), i++)
	{
		TVCS_CHECKSTATE state = m_wndFilterView.GetCheck(item);
		if (state == TVCS_CHECKED)
		{
			CString csFilterChecked = m_wndFilterView.GetItemText(item);
			COleDateTime oTime;
			oTime.ParseDateTime(csFilterChecked);
			pdesc->otfilterParam2.Add(oTime);
			pdesc->csfilterParam2.Add(csFilterChecked);
		}
	}
}

void CFilterWnd::OnApplyFilter()
{
	if (!m_pDoc)
		return;
	CdbWdatabase* pDB = m_pDoc->m_pDB;

	int i = 0;
	while (m_noCol[i] > 0)
	{
		HTREEITEM hParent = m_htreeitem[i];
		i++;

		int icol = m_wndFilterView.GetItemData(hParent);
		DB_ITEMDESC* pdesc = pDB->GetRecordItemDescriptor(icol);
		//if root is checked (or unchecked), it means no item is selected - remove flag
		TVCS_CHECKSTATE stateRoot = m_wndFilterView.GetCheck(hParent);
		if ((stateRoot == TVCS_CHECKED) || (stateRoot == TVCS_UNCHECKED))
		{
			pdesc->bFilter2 = FALSE;
		}
		// else if foot is undeterminate build filter 
		else
		{
			pdesc->bFilter2 = TRUE;
			pdesc->lfilterParam2.RemoveAll();
			pdesc->csfilterParam2.RemoveAll();
			pdesc->otfilterParam2.RemoveAll();
			HTREEITEM startItem = m_wndFilterView.GetNextItem(hParent, TVGN_CHILD);
			switch (pdesc->typeLocal)
			{
			case FIELD_IND_TEXT:
			case FIELD_IND_FILEPATH:
				BuildFilterItemIndirectionFromTree(pdesc, startItem);
				break;
			case FIELD_LONG:
				BuildFilterItemLongFromTree(pdesc, startItem);
				break;
			case FIELD_DATE_YMD:
				BuildFilterItemDateFromTree(pdesc, startItem);
				break;
			default:
				ASSERT(false);
				break;
			}
		}
	}

	// update recordset and tell other views...
	pDB->m_tableSet.BuildFilters();
	pDB->m_tableSet.RefreshQuery();
	m_pDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}

void CFilterWnd::OnSortRecords()
{
	CdbWdatabase* pDB = m_pDoc->m_pDB;
	ASSERT(pDB);
	CMFCToolBarComboBoxButton* pCombo = (CMFCToolBarComboBoxButton*)m_wndToolBar.GetButton(3);
	ASSERT(ID_RECORD_SORT == m_wndToolBar.GetItemID(3));

	int isel = pCombo->GetCurSel();
	ASSERT(isel != CB_ERR);
	int i = pCombo->GetItemData(isel);
	pDB->m_tableSet.m_strSort = pDB->m_desctab[i].szTableCol;

	pDB->m_tableSet.RefreshQuery();
	m_pDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}

void CFilterWnd::SelectNext(BOOL bNext)
{
	CTreeCtrl* pTree = (CTreeCtrl*)&m_wndFilterView;
	ASSERT_VALID(pTree);
	HTREEITEM hItem = pTree->GetSelectedItem();
	if (!pTree->ItemHasChildren(hItem))
		hItem = pTree->GetParentItem(hItem);
	if (hItem == NULL)
		return;

	int count = 0;
	int lastselected = -1;
	HTREEITEM hlastselected = NULL;
	int nselected = 0;
	HTREEITEM hKid = pTree->GetChildItem(hItem);
	do
	{
		TVCS_CHECKSTATE state = ((CQuadStateTree*)pTree)->GetCheck(hKid);
		if (state == TVCS_CHECKED)
		{
			hlastselected = hKid;
			lastselected = count;
			nselected++;
		}
		count++;
	} while (hKid = pTree->GetNextSiblingItem(hKid));

	// if single selection select next item on the list and deselect current; update
	if (nselected == 1)
	{
		HTREEITEM hNext;
		if (bNext)
			hNext = pTree->GetNextSiblingItem(hlastselected);
		else
			hNext = pTree->GetPrevSiblingItem(hlastselected);
		if (hNext == NULL)
			return;
		BOOL  bcheck = ((CQuadStateTree*)pTree)->SetCheck(hNext, TVCS_CHECKED);
		BOOL  bucheck = ((CQuadStateTree*)pTree)->SetCheck(hlastselected, TVCS_UNCHECKED);
		OnApplyFilter();
	}
}

void CFilterWnd::OnSelectNext()
{
	SelectNext(TRUE);
}

void CFilterWnd::OnSelectPrevious()
{
	SelectNext(FALSE);
}
