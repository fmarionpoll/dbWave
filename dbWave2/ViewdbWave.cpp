// dbWaveView.cpp : implementation of the CdbWaveView class
//

#include "stdafx.h"
#include "dbWave.h"
#include "resource.h"

#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "spikedoc.h"

#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"
#include "editctrl.h"
#include "spikebar.h"
#include "spikeshape.h"
#include "DataListCtrl.h"

#include "MainFrm.h"
#include "childfrm.h"
#include "progdlg.h"

#include ".\ViewdbWave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CdbWaveView

IMPLEMENT_DYNCREATE(CdbWaveView, CDaoRecordView)

BEGIN_MESSAGE_MAP(CdbWaveView, CDaoRecordView)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_RECORD_PAGEUP, &CdbWaveView::OnRecordPageup)
	ON_COMMAND(ID_RECORD_PAGEDOWN, &CdbWaveView::OnRecordPagedown)
	ON_BN_CLICKED(IDC_FILTERCHECK, &CdbWaveView::OnClickMedianFilter)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LISTCTRL, &CdbWaveView::OnItemActivateListctrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, &CdbWaveView::OnDblclkListctrl)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_FILE_PRINT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CDaoRecordView::OnFilePrintPreview)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTCTRL, &CdbWaveView::OnLvnColumnclickListctrl)
	ON_BN_CLICKED(IDC_RADIO1, &CdbWaveView::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO3, &CdbWaveView::OnBnClickedRadio3)
	ON_EN_CHANGE(IDC_TIMEFIRST, &CdbWaveView::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &CdbWaveView::OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_AMPLITUDESPAN, &CdbWaveView::OnEnChangeAmplitudespan)
	ON_BN_CLICKED(IDC_CHECKFILENAME, &CdbWaveView::OnBnClickedCheckfilename)
	ON_NOTIFY(HDN_ENDTRACK, 0, &CdbWaveView::OnHdnEndtrackListctrl)
	ON_BN_CLICKED(IDC_CHECK2, &CdbWaveView::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK1, &CdbWaveView::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_RADIOALLCLASSES, &CdbWaveView::OnBnClickedRadioallclasses)
	ON_BN_CLICKED(IDC_RADIOONECLASS, &CdbWaveView::OnBnClickedRadiooneclass)
	ON_BN_CLICKED(IDC_RADIO2, &CdbWaveView::OnBnClickedRadio2)
	ON_EN_CHANGE(IDC_SPIKECLASS, &CdbWaveView::OnEnChangeSpikeclass)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CdbWaveView::OnTcnSelchangeTab1)
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CdbWaveView::OnNMClickTab1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CdbWaveView construction/destruction

CdbWaveView::CdbWaveView()
	: CDaoRecordView(CdbWaveView::IDD)
	, m_timefirst(0)
	, m_timelast(0)
	, m_amplitudespan(0)
{
	m_pSet = NULL;
	m_bAddMode=FALSE;
	m_bFilterON = TRUE;
	m_bvalidDat=FALSE;
	m_bvalidSpk=FALSE;
	m_dattransform=0;
	m_binit = FALSE;
	m_spikeclass = 0;
	m_bEnableActiveAccessibility=FALSE;
}

CdbWaveView::~CdbWaveView()
{
}

void CdbWaveView::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_AMPLITUDESPAN, m_amplitudespan);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikeclass);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}

BOOL CdbWaveView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////

void CdbWaveView::OnInitialUpdate()
{	
	// init document and DaoRecordView
	CdbWaveDoc* pdbDoc = GetDocument();
	m_pSet = &pdbDoc->m_pDB->m_tableSet;
	CDaoRecordView::OnInitialUpdate();
	
	// subclass display controls and attach them to stretch CFolderview
	VERIFY(m_dataListCtrl.SubclassDlgItem(IDC_LISTCTRL, this));
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(mm_amplitudespan.SubclassDlgItem(IDC_AMPLITUDESPAN, this));
	VERIFY(mm_spikeclass.SubclassDlgItem(IDC_SPIKECLASS, this));

	m_stretch.AttachParent(this);	// attach formview pointer
	m_stretch.newProp(IDC_LISTCTRL, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	m_binit = TRUE;

	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp(); 
	mdPM = &pApp->vdP;
	m_dataListCtrl.InitColumns(&mdPM->icolwidth);

	// set how data are displayed
	m_amplitudespan = mdPM->mVspan;
	m_dataListCtrl.SetAmplitudeSpan(mdPM->mVspan);
	
	CheckDlgButton(IDC_CHECKFILENAME, mdPM->bDisplayFileName);
	m_dataListCtrl.SetDisplayFileName(mdPM->bDisplayFileName);
	
	m_timefirst = mdPM->tFirst;
	m_timelast = mdPM->tLast;
	if (m_timefirst != 0.f && m_timelast != 0.f)
		m_dataListCtrl.SetTimeIntervals(m_timefirst, m_timelast);

	CheckDlgButton(IDC_CHECK1, mdPM->bsetTimeSpan);
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(mdPM->bsetTimeSpan);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(mdPM->bsetTimeSpan);
	m_dataListCtrl.SetTimespanAdjustMode(mdPM->bsetTimeSpan);
	
	CheckDlgButton(IDC_CHECK2, mdPM->bsetmVSpan);
	GetDlgItem(IDC_AMPLITUDESPAN)->EnableWindow(mdPM->bsetmVSpan);
	m_dataListCtrl.SetAmplitudeAdjustMode(mdPM->bsetmVSpan);
	
	// adjust size of dataviewlistbox
	UINT cyItemHeight = 50; // n pixels
	int nIndex = 0;
	m_dataListCtrl.SetExtendedStyle
		(m_dataListCtrl.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT 
		| LVS_EX_GRIDLINES  
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_SUBITEMIMAGES);

	// set display mode of m_dataListCtrl
	m_dataListCtrl.SetDisplayMode(mdPM->displaymode);
	switch (mdPM->displaymode)
	{
	case 1:
		((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
		GetDlgItem(IDC_FILTERCHECK)->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_FILTERCHECK))->SetCheck(mdPM->bFilterDat);
		if(mdPM->bFilterDat)	m_dattransform = 13;	// apply median filter to data displayed
		else					m_dattransform = 0;		// no filter (raw data)
		m_dataListCtrl.SetTransformMode(m_dattransform);
		break;
	case 2:
		((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(BST_CHECKED);
		GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(TRUE);
		// display all spike classes
		if (mdPM->bDisplayAllClasses) 
		{
			((CButton*)GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_CHECKED);
			((CButton*)GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_UNCHECKED);
			m_dataListCtrl.SetSpikePlotMode(PLOT_BLACK, m_spikeclass);
		}
		else
		{
			((CButton*)GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_CHECKED);
			m_spikeclass = mdPM->spikeclass;
			mm_spikeclass.EnableWindow(TRUE);
			m_dataListCtrl.SetSpikePlotMode(PLOT_ONECLASSONLY, m_spikeclass);
		}
		break;
	default:
		((CButton*)GetDlgItem(IDC_RADIO3))->SetCheck(BST_CHECKED);
		GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
		break;
	}

	// select the proper record	
	FillListBox();	
	m_dataListCtrl.UpdateCache(-3, -3);
	UpdateControls();
	
	// init display controls
	if (mdPM->displaymode == 2)
	{
		// update tab control
		InitctrlTab();
		m_tabCtrl.ShowWindow(SW_SHOW);
		m_tabCtrl.SetCurSel(GetDocument()->GetcurrentSpkListIndex());
	};
}

/////////////////////////////////////////////////////////////////////////////
// CdbWaveView printing

BOOL CdbWaveView::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;
	
	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	return TRUE;
}

void CdbWaveView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CdbWaveView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CdbWaveView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: add customized printing code here
	if(pInfo->m_bDocObject)
		COleDocObjectItem::OnPrint(this, pInfo, TRUE);
	else
		CView::OnPrint(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////

void CdbWaveView::OnDestroy()
{
	// Deactivate the item on destruction; this is important
	// when a splitter view is being used.
   CDaoRecordView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// adapt the size of child windows when view is resized

void CdbWaveView::OnSize(UINT nType, int cx, int cy)
{
	// adapt size of resizeable controls
	if (m_binit)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			// change size of windows declared to this m_stretch
			m_stretch.ResizeControls(nType, cx, cy);
			break;
		default:
			break;
		}
	}
	// do other resizing
	CDaoRecordView::OnSize(nType, cx, cy);
	if (::IsWindow(m_dataListCtrl.m_hWnd)) {
		CRect rect;
		m_dataListCtrl.GetClientRect(&rect);
		m_dataListCtrl.FitColumnsToSize(rect.Width());
	}
}


/////////////////////////////////////////////////////////////////////////////
// CdbWaveView diagnostics

#ifdef _DEBUG
void CdbWaveView::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CdbWaveView::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CdbWaveView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CdbWaveView database support
CDaoRecordset* CdbWaveView::OnGetRecordset()
{
	return GetDocument()->DBGetRecordset();
}


/////////////////////////////////////////////////////////////////////////////
// CdbWaveView message handlers
void CdbWaveView::UpdateControls()
{
	CdbWaveDoc* pdbDoc = GetDocument();
	CFileStatus status;

	CString filename = pdbDoc->DBGetCurrentDatFileName();
	m_bvalidDat = CFile::GetStatus(filename, status);
	filename = pdbDoc->DBGetCurrentSpkFileName(TRUE);
	m_bvalidSpk = CFile::GetStatus(filename, status);
	
	int ifile = pdbDoc->DBGetCurrentRecordPosition();
	m_dataListCtrl.SetCurSel(ifile);
	m_dataListCtrl.EnsureVisible(ifile, FALSE);
}

BOOL CdbWaveView::OnMove(UINT nIDMoveCommand)
{
	BOOL flag = CDaoRecordView::OnMove(nIDMoveCommand);
	GetDocument()->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);
	return flag;
}

// ----------------------------------------------------------------------------

void CdbWaveView::OnRecordPageup() 
{
	m_dataListCtrl.SendMessage(WM_VSCROLL, SB_PAGEUP, NULL);
}

void CdbWaveView::OnRecordPagedown() 
{
	m_dataListCtrl.SendMessage(WM_VSCROLL, SB_PAGEDOWN, NULL);
}

void CdbWaveView::OnClickMedianFilter() 
{
	if (mdPM->bFilterDat == ((CButton*)GetDlgItem(IDC_FILTERCHECK))->GetCheck())
		return;

	mdPM->bFilterDat = ((CButton*)GetDlgItem(IDC_FILTERCHECK))->GetCheck();
	if(mdPM->bFilterDat)	m_dattransform = 13;
	else					m_dattransform = 0;
	m_dataListCtrl.SetTransformMode(m_dattransform);
	m_dataListCtrl.RefreshDisplay();
}

// ----------------------------------------------------------------------------

void CdbWaveView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CMainFrame* pmF = (CMainFrame*) AfxGetMainWnd();
	// activate view
	if (bActivate)
	{
		// make sure the secondary toolbar is not visible
		// (none is defined for the database)
		if (pmF->m_pSecondToolBar != NULL) 
			pmF->ShowPane(pmF->m_pSecondToolBar, FALSE, FALSE, TRUE);
		// load status
		m_nStatus = ((CChildFrame*)pmF->MDIGetActive())->m_nStatus;
		pmF->PostMessageW(WM_MYMESSAGE, HINT_ACTIVATEVIEW, LPARAM(pActivateView->GetDocument()));
	}
	else
	{
		if (pActivateView != NULL)
			((CChildFrame*)pmF->MDIGetActive())->m_nStatus = m_nStatus; // save status
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

//////////////////////////////////////////////////////////////////////////////

void CdbWaveView::FillListBox()
{
	CdbWaveDoc* pdbDoc = GetDocument();
	int imax = pdbDoc->DBGetNRecords();
	m_dataListCtrl.DeleteAllItems();
	m_dataListCtrl.SetItemCountEx(imax);
}

void CdbWaveView::OnItemActivateListctrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// get item clicked and select it
	NMITEMACTIVATE* pItemActivate = (NMITEMACTIVATE*) pNMHDR;
	if (pItemActivate->iItem >= 0)
		GetDocument()->DBSetCurrentRecordPosition(pItemActivate->iItem);
	GetDocument()->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);
	CDaoRecordView::OnInitialUpdate();

	*pResult = 0;
}

void CdbWaveView::OnDblclkListctrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	// quit the current view and load spike detection view
	((CChildFrame*) GetParent())->PostMessage(
			WM_COMMAND, 
			(WPARAM) ID_VIEW_SPIKEDETECTION,
			(LPARAM) NULL);
}

void CdbWaveView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (!m_binit)
		return;
		
	switch (LOWORD(lHint))
	{
	case HINT_GETSELECTEDRECORDS:
		{
			CdbWaveDoc*	pDoc = GetDocument();
			pDoc->m_selectedRecords.RemoveAll();
			UINT uSelectedCount = m_dataListCtrl.GetSelectedCount();
			
			// Update all of the selected items.
			if (uSelectedCount > 0)
			{
				pDoc->m_selectedRecords.SetSize(uSelectedCount);
				int nItem = -1;
				for (UINT i=0; i < uSelectedCount; i++)
				{
					nItem = m_dataListCtrl.GetNextItem(nItem, LVNI_SELECTED);
					ASSERT(nItem != -1);
					pDoc->m_selectedRecords.SetAt(i, nItem); 
				}
			}
		}
		break;

	case HINT_SETSELECTEDRECORDS:
		{
			CdbWaveDoc*	pDoc = GetDocument();
			UINT uSelectedCount = pDoc->m_selectedRecords.GetSize();

			// clear previous selection in the CListCtrl if any
			int Item=-1;
			Item = m_dataListCtrl.GetNextItem(Item, LVNI_SELECTED);
			 while (Item != -1) 
			 {
				m_dataListCtrl.SetItemState(Item, 0, LVIS_SELECTED);
				Item = m_dataListCtrl.GetNextItem(Item, LVNI_SELECTED); 
			}
			
			// select items
			if (uSelectedCount > 0)
			{
				for (UINT i=0; i < uSelectedCount; i++)
				{
					Item = pDoc->m_selectedRecords.GetAt(i);
					m_dataListCtrl.SetItemState(Item, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
				}
			}
			m_dataListCtrl.EnsureVisible(Item, FALSE);
		}
		break;

	case HINT_REPLACEVIEW:
		m_dataListCtrl.UpdateCache(-2, -2);
		UpdateControls();
		break;

	case HINT_REQUERY:
		FillListBox();
	case HINT_DOCHASCHANGED:
		m_dataListCtrl.UpdateCache(-1, -1);
	case HINT_DOCMOVERECORD:
	default:
		UpdateControls();
		break;
	}
}

void CdbWaveView::DeleteRecords()
{
	// save index current file
	long currentindex = GetDocument()->DBGetCurrentRecordPosition()-1;
	if (currentindex <0)
		currentindex = 0;

	// loop on Cdatalistctrl to delete all selected items
	CdbWaveDoc* pdbDoc = GetDocument();
	POSITION pos = m_dataListCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		AfxMessageBox(_T("No item selected: delete operation failed"));
		return;
	}
	else 
	{
		// assume that no one else accesses to the database at the same time
		int ndel = 0;
		// delete file names from database
		while (pos)
		{
			int nItem = m_dataListCtrl.GetNextSelectedItem(pos);
			pdbDoc->DBSetCurrentRecordPosition(nItem - ndel);
			pdbDoc->DBDeleteCurrentRecord();
			ndel ++;
	   }
	}

	// If the recordset is now empty, clear the fields left over
	// from the deleted record
	/*if (m_pSet->IsBOF())
		m_pSet->SetFieldNull(NULL);
	m_pSet->RefreshQuery();*/

	pdbDoc->DBSetCurrentRecordPosition(currentindex);
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}

void CdbWaveView::OnLvnColumnclickListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	CString filter0= m_pSet->GetSQL();
	CString cs;
	CdbWaveDoc* pdbDoc = GetDocument();
	switch(pNMLV->iSubItem)
	{
	case COL_CURVE:	cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_DATALEN].csColName; break;		// datalen
	case COL_INDEX:	cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_ID].csColName; break;			// ID
	case COL_SENSI:	cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_SENSILLUM_ID].csColName; break;	// sensillum_ID
	case COL_STIM1:	cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_STIM_ID].csColName; break;		// stim_ID
	case COL_CONC1:	cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_CONC_ID].csColName; break;		// conc_ID
	case COL_STIM2:	cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_STIM2_ID].csColName; break;	// stim2_ID
	case COL_CONC2:	cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_CONC2_ID].csColName; break;	// conc2_ID
	case COL_NBSPK: cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_NSPIKES].csColName; break;		// nspikes
	case COL_FLAG: cs = pdbDoc->m_pDB->m_tableSet.m_desc[CH_FLAG].csColName; break;			// flag
	default:
		break;
	}
	m_pSet->m_strSort = cs;
	m_pSet->Requery();
	GetDocument()->UpdateAllViews(NULL, HINT_REQUERY, NULL);
	*pResult = 0;
}

void CdbWaveView::OnBnClickedRadio1()
{
	((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
	mdPM->displaymode = 1;
	m_dataListCtrl.SetDisplayMode(mdPM->displaymode);
	m_dataListCtrl.RefreshDisplay();
	m_tabCtrl.ShowWindow(SW_HIDE);

	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
}

void CdbWaveView::InitctrlTab()
{
	// reset tab control
	m_tabCtrl.DeleteAllItems();

	// load list of detection parameters 
	int j = 0;
	if (GetDocument() ->OpenCurrentSpikeFile())
	{
		int curr_listsize = GetDocument()->GetcurrentSpkListSize();
		for (int i = 0; i< curr_listsize; i++)
		{
			CSpikeList* pSL = GetDocument()->m_pSpk->SetSpkListCurrent(i);
			if (!pSL)
				continue;
			CString cs;
			if (pSL->GetdetectWhat() != 0)
				continue;
			cs.Format(_T("#%i %s"), i, (LPCTSTR) pSL->GetComment());
			m_tabCtrl.InsertItem(j, cs);
			//m_tabCtrl.SetItemData(j, i);		// save list item
			j++;
		}
	}
}

void CdbWaveView::OnBnClickedRadio3()
{
	((CButton*)GetDlgItem(IDC_RADIO3))->SetCheck(BST_CHECKED);
	mdPM->displaymode = 0;
	m_tabCtrl.ShowWindow(SW_HIDE);
	m_dataListCtrl.SetDisplayMode(mdPM->displaymode);
	m_dataListCtrl.RefreshDisplay();

	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
}

void CdbWaveView::OnEnChangeTimefirst()
{
	if (!mm_timefirst.m_bEntryDone)
		return;

	switch (mm_timefirst.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_timefirst++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_timefirst--;
		break;
	}

	mm_timefirst.m_bEntryDone=FALSE;
	mm_timefirst.m_nChar=0;
	mm_timefirst.SetSel(0, -1); 	//select all text
	mdPM->tFirst = m_timefirst;
	if (m_timefirst > m_timelast)
		m_timefirst = 0.f;
	m_dataListCtrl.SetTimeIntervals(m_timefirst, m_timelast);
	m_dataListCtrl.RefreshDisplay();
}

void CdbWaveView::OnEnChangeTimelast()
{
	if (!mm_timelast.m_bEntryDone)
		return;

	switch (mm_timelast.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_timelast++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_timelast--;
		break;
	}

	mm_timelast.m_bEntryDone=FALSE;
	mm_timelast.m_nChar=0;
	mm_timelast.SetSel(0, -1); 	//select all text
	mdPM->tLast = m_timelast;
	m_dataListCtrl.SetTimeIntervals(m_timefirst, m_timelast);
	m_dataListCtrl.RefreshDisplay();
}

void CdbWaveView::OnEnChangeAmplitudespan()
{
	if (!mm_amplitudespan.m_bEntryDone)
		return;

	float y = m_amplitudespan;
	CString cs;
	switch (mm_amplitudespan.m_nChar)
	{		
	case VK_RETURN:			
		mm_amplitudespan.GetWindowText(cs);
		y = (float) _ttof (cs);
		break;
	case VK_UP:
	case VK_PRIOR:
		y++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		y--;
		break;
	}
	
	// update the dialog control
	mm_amplitudespan.m_bEntryDone=FALSE;
	mm_amplitudespan.m_nChar=0;
	mm_amplitudespan.SetSel(0, -1); 	//select all text
	cs.Format(_T("%.3f"), y);
	GetDlgItem(IDC_AMPLITUDESPAN)->SetWindowText(cs);
	m_amplitudespan = y;
	mdPM->mVspan = y;
	m_dataListCtrl.SetAmplitudeSpan(y);
	m_dataListCtrl.RefreshDisplay();
}

void CdbWaveView::OnBnClickedCheckfilename()
{
	mdPM->bDisplayFileName = IsDlgButtonChecked(IDC_CHECKFILENAME);
	m_dataListCtrl.SetDisplayFileName(mdPM->bDisplayFileName);
	m_dataListCtrl.RefreshDisplay();
}

void CdbWaveView::OnHdnEndtrackListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	if (phdr->iItem == COL_CURVE)
		m_dataListCtrl.ResizeSignalColumn(phdr->pitem->cxy);
	*pResult = 0;
}

void CdbWaveView::OnBnClickedCheck2()
{
	mdPM->bsetmVSpan = IsDlgButtonChecked(IDC_CHECK2);
	m_dataListCtrl.SetAmplitudeAdjustMode(mdPM->bsetmVSpan);
	m_dataListCtrl.RefreshDisplay();
	GetDlgItem(IDC_AMPLITUDESPAN)->EnableWindow(mdPM->bsetmVSpan);
}

void CdbWaveView::OnBnClickedCheck1()
{
	mdPM->bsetTimeSpan = IsDlgButtonChecked(IDC_CHECK1);
	// check intervals to avoid crash
	if (mdPM->bsetTimeSpan)
	{
		if (m_timefirst == m_timelast)
		{
			m_timelast++;
			mdPM->tLast = m_timelast ;
			UpdateData(FALSE);
		}
		m_dataListCtrl.SetTimeIntervals(m_timefirst, m_timelast);
	}
	// update data display
	m_dataListCtrl.SetTimespanAdjustMode(mdPM->bsetTimeSpan);
	m_dataListCtrl.RefreshDisplay();
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(mdPM->bsetTimeSpan);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(mdPM->bsetTimeSpan);
}

void CdbWaveView::OnBnClickedRadioallclasses()
{
	(CWnd*)(GetDlgItem(IDC_SPIKECLASS))->EnableWindow(FALSE);
	mdPM->bDisplayAllClasses = TRUE;
	m_dataListCtrl.SetSpikePlotMode(PLOT_BLACK, m_spikeclass);
	m_dataListCtrl.RefreshDisplay();
}

void CdbWaveView::OnBnClickedRadiooneclass()
{
	(CWnd*)(GetDlgItem(IDC_SPIKECLASS))->EnableWindow(TRUE);
	mdPM->bDisplayAllClasses = FALSE;
	m_dataListCtrl.SetSpikePlotMode(PLOT_ONECLASSONLY, m_spikeclass);
	m_dataListCtrl.RefreshDisplay();
}

void CdbWaveView::OnBnClickedRadio2()
{
	((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(BST_CHECKED);

	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(TRUE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);
	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);

	mdPM->displaymode = 2;
	// update tab control
	InitctrlTab();

	m_tabCtrl.ShowWindow(SW_SHOW);
	m_tabCtrl.SetCurSel(GetDocument()->GetcurrentSpkListIndex());

	// display spikes
	m_dataListCtrl.SetDisplayMode(mdPM->displaymode);
	if (mdPM->bDisplayAllClasses) 
	{
		((CButton*)GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_CHECKED);
		((CButton*)GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_UNCHECKED);
		m_dataListCtrl.SetSpikePlotMode(PLOT_BLACK, 0);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_CHECKED);
		m_spikeclass = mdPM->spikeclass;
		m_dataListCtrl.SetSpikePlotMode(PLOT_ONECLASSONLY, m_spikeclass);
	}
	m_dataListCtrl.RefreshDisplay();
}

void CdbWaveView::OnEnChangeSpikeclass()
{
	if (!mm_spikeclass.m_bEntryDone)
		return;

	int spikeclassoption = m_spikeclass;
	switch (mm_spikeclass.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);	
		break;
	case VK_UP:
	case VK_PRIOR:
		m_spikeclass++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_spikeclass--;
		break;
	}	
	mm_spikeclass.m_bEntryDone=FALSE;
	mm_spikeclass.m_nChar=0;
	mm_spikeclass.SetSel(0, -1); 	//select all text
	mdPM->spikeclass = m_spikeclass;
	UpdateData(FALSE);
	m_dataListCtrl.SetSpikePlotMode(PLOT_ONECLASSONLY, m_spikeclass);
	m_dataListCtrl.RefreshDisplay();
}

void CdbWaveView::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	GetDocument()->SetcurrentSpkListIndex(icursel);
	m_dataListCtrl.RefreshDisplay();
	*pResult = 0;
}

void CdbWaveView::OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	*pResult = 0;
}

