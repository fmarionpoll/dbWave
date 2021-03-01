// dbWaveView.cpp : implementation of the CViewdbWave class
//

#include "StdAfx.h"
#include "dbWave.h"
#include "resource.h"

#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"

#include "chart.h"
#include "Editctrl.h"
#include "DataListCtrl.h"

#include "MainFrm.h"
#include "ChildFrm.h"

#include "CViewdbWave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CViewdbWave, CViewdbWaveRecord)

BEGIN_MESSAGE_MAP(CViewdbWave, CViewdbWaveRecord)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_RECORD_PAGEUP, &CViewdbWave::OnRecordPageup)
	ON_COMMAND(ID_RECORD_PAGEDOWN, &CViewdbWave::OnRecordPagedown)
	ON_COMMAND(ID_FILE_PRINT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CDaoRecordView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CDaoRecordView::OnFilePrintPreview)
	ON_BN_CLICKED(IDC_RADIO1, &CViewdbWave::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO3, &CViewdbWave::OnBnClickedRadio3)
	ON_EN_CHANGE(IDC_TIMEFIRST, &CViewdbWave::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &CViewdbWave::OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_AMPLITUDESPAN, &CViewdbWave::OnEnChangeAmplitudespan)
	ON_EN_CHANGE(IDC_SPIKECLASS, &CViewdbWave::OnEnChangeSpikeclass)
	ON_BN_CLICKED(IDC_CHECKFILENAME, &CViewdbWave::OnBnClickedCheckfilename)
	ON_BN_CLICKED(IDC_FILTERCHECK, &CViewdbWave::OnClickMedianFilter)
	ON_BN_CLICKED(IDC_CHECK2, &CViewdbWave::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK1, &CViewdbWave::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_RADIOALLCLASSES, &CViewdbWave::OnBnClickedRadioallclasses)
	ON_BN_CLICKED(IDC_RADIOONECLASS, &CViewdbWave::OnBnClickedRadiooneclass)
	ON_BN_CLICKED(IDC_RADIO2, &CViewdbWave::OnBnClickedDisplaySpikes)
	ON_NOTIFY(HDN_ENDTRACK, 0, &CViewdbWave::OnHdnEndtrackListctrl)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTCTRL, &CViewdbWave::OnLvnColumnclickListctrl)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CViewdbWave::OnTcnSelchangeTab1)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LISTCTRL, &CViewdbWave::OnItemActivateListctrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, &CViewdbWave::OnDblclkListctrl)
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CViewdbWave::OnNMClickTab1)
END_MESSAGE_MAP()

CViewdbWave::CViewdbWave()
	: CViewdbWaveRecord(CViewdbWave::IDD)
	, m_timefirst(0)
	, m_timelast(0)
	, m_amplitudespan(0), m_options_viewdata(nullptr)
{
	m_pSet = nullptr;
	m_bAddMode = FALSE;
	m_bFilterON = TRUE;
	m_bvalidDat = FALSE;
	m_bvalidSpk = FALSE;
	m_dattransform = 0;
	m_binit = FALSE;
	m_spikeclass = 0;
	m_bEnableActiveAccessibility = FALSE;
}

CViewdbWave::~CViewdbWave()
{
}

void CViewdbWave::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_AMPLITUDESPAN, m_amplitudespan);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikeclass);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}


void CViewdbWave::OnInitialUpdate()
{
	// init document and DaoRecordView
	CdbWaveDoc* p_dbwave_doc = GetDocument();
	m_pSet = &p_dbwave_doc->m_pDB->m_mainTableSet;
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

	auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	m_options_viewdata = &p_app->options_viewdata;
	m_dataListCtrl.InitColumns(&m_options_viewdata->icolwidth);

	// set how data are displayed
	m_amplitudespan = m_options_viewdata->mVspan;
	m_dataListCtrl.SetAmplitudeSpan(m_options_viewdata->mVspan);

	CheckDlgButton(IDC_CHECKFILENAME, m_options_viewdata->bDisplayFileName);
	m_dataListCtrl.SetDisplayFileName(m_options_viewdata->bDisplayFileName);

	m_timefirst = m_options_viewdata->tFirst;
	m_timelast = m_options_viewdata->tLast;
	if (m_timefirst != 0.f && m_timelast != 0.f)
		m_dataListCtrl.SetTimeIntervals(m_timefirst, m_timelast);

	CheckDlgButton(IDC_CHECK1, m_options_viewdata->bsetTimeSpan);
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(m_options_viewdata->bsetTimeSpan);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(m_options_viewdata->bsetTimeSpan);
	m_dataListCtrl.SetTimespanAdjustMode(m_options_viewdata->bsetTimeSpan);

	CheckDlgButton(IDC_CHECK2, m_options_viewdata->bsetmVSpan);
	GetDlgItem(IDC_AMPLITUDESPAN)->EnableWindow(m_options_viewdata->bsetmVSpan);
	m_dataListCtrl.SetAmplitudeAdjustMode(m_options_viewdata->bsetmVSpan);

	// adjust size of dataviewlistbox
	//UINT cy_item_height = 50; // n pixels
	//auto n_index = 0;
	m_dataListCtrl.SetExtendedStyle
	(m_dataListCtrl.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT
		| LVS_EX_GRIDLINES
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_SUBITEMIMAGES);

	// set display mode of m_dataListCtrl
	m_dataListCtrl.SetDisplayMode(m_options_viewdata->displaymode);
	switch (m_options_viewdata->displaymode)
	{
	case 1:
		((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
		GetDlgItem(IDC_FILTERCHECK)->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_FILTERCHECK))->SetCheck(m_options_viewdata->bFilterDat);
		if (m_options_viewdata->bFilterDat)	
			m_dattransform = 13;	// apply median filter to data displayed
		else					
			m_dattransform = 0;		// no filter (raw data)
		m_dataListCtrl.SetTransformMode(m_dattransform);
		break;
	case 2:
		((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(BST_CHECKED);
		GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(TRUE);
		// display all spike classes
		if (m_options_viewdata->bDisplayAllClasses)
		{
			((CButton*)GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_CHECKED);
			((CButton*)GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_UNCHECKED);
			m_dataListCtrl.SetSpikePlotMode(PLOT_BLACK, m_spikeclass);
		}
		else
		{
			((CButton*)GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_CHECKED);
			m_spikeclass = m_options_viewdata->spikeclass;
			mm_spikeclass.EnableWindow(TRUE);
			m_dataListCtrl.SetSpikePlotMode(PLOT_ONECLASSONLY, m_spikeclass);
		}
		break;
	default:
		((CButton*)GetDlgItem(IDC_RADIO3))->SetCheck(BST_CHECKED);
		GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
		break;
	}

	// close data file if opened?

	// select the proper record
	fillListBox();
	m_dataListCtrl.UpdateCache(-3, -3);
	updateControls();

	// init display controls
	if (m_options_viewdata->displaymode == 2)
	{
		CSpikeDoc* pSpkDoc = GetDocument()->GetcurrentSpkDocument();
		// update tab control
		m_tabCtrl.InitctrlTabFromSpikeDoc(pSpkDoc);
		m_tabCtrl.ShowWindow(SW_SHOW);
		int icur = pSpkDoc->GetSpkList_CurrentIndex();
		m_tabCtrl.SetCurSel(icur);
	};
}

BOOL CViewdbWave::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	return TRUE;
}

void CViewdbWave::OnBeginPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CViewdbWave::OnEndPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CViewdbWave::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	// TODO: add customized printing code here
	if (pInfo->m_bDocObject)
		COleDocObjectItem::OnPrint(this, pInfo, TRUE);
	else
		CView::OnPrint(p_dc, pInfo);
}

void CViewdbWave::OnDestroy()
{
	// Deactivate the item on destruction; this is important
	// when a splitter view is being used.
	CDaoRecordView::OnDestroy();
}

void CViewdbWave::OnSize(const UINT n_type, const int cx, const int cy)
{
	// adapt size of resizeable controls
	if (m_binit)
	{
		switch (n_type)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			// change size of windows declared to this m_stretch
			m_stretch.ResizeControls(n_type, cx, cy);
			break;
		default:
			break;
		}
	}
	// do other resizing
	CDaoRecordView::OnSize(n_type, cx, cy);
	if (::IsWindow(m_dataListCtrl.m_hWnd)) {
		CRect rect;
		m_dataListCtrl.GetClientRect(&rect);
		m_dataListCtrl.FitColumnsToSize(rect.Width());
	}
}

#ifdef _DEBUG
void CViewdbWave::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CViewdbWave::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CViewdbWave::GetDocument()
// non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}
#endif //_DEBUG

CDaoRecordset* CViewdbWave::OnGetRecordset()
{
	return GetDocument()->GetDB_Recordset();
}

void CViewdbWave::updateControls()
{
	auto pdb_doc = GetDocument();
	CFileStatus status;

	auto filename = pdb_doc->GetDB_CurrentDatFileName();
	m_bvalidDat = CFile::GetStatus(filename, status);

	filename = pdb_doc->GetDB_CurrentSpkFileName(TRUE);
	m_bvalidSpk = CFile::GetStatus(filename, status);

	const int ifile = pdb_doc->GetDB_CurrentRecordPosition();
	m_dataListCtrl.SetCurSel(ifile);
	m_dataListCtrl.EnsureVisible(ifile, FALSE);

	if (m_options_viewdata->displaymode == 2) {
		CSpikeDoc* pSpkDoc = GetDocument()->OpenCurrentSpikeFile();
		if (pSpkDoc != nullptr)
		{
			const auto curr_listsize = pSpkDoc->GetSpkList_Size();
			if (m_tabCtrl.GetItemCount() < curr_listsize)
				m_tabCtrl.InitctrlTabFromSpikeDoc(pSpkDoc);
		}
	}
}

BOOL CViewdbWave::OnMove(UINT nIDMoveCommand)
{
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	GetDocument()->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CViewdbWave::OnRecordPageup()
{
	m_dataListCtrl.SendMessage(WM_VSCROLL, SB_PAGEUP, NULL);
}

void CViewdbWave::OnRecordPagedown()
{
	m_dataListCtrl.SendMessage(WM_VSCROLL, SB_PAGEDOWN, NULL);
}

void CViewdbWave::OnClickMedianFilter()
{
	if (m_options_viewdata->bFilterDat == ((CButton*)GetDlgItem(IDC_FILTERCHECK))->GetCheck())
		return;

	m_options_viewdata->bFilterDat = ((CButton*)GetDlgItem(IDC_FILTERCHECK))->GetCheck();
	if (m_options_viewdata->bFilterDat)	
		m_dattransform = 13;
	else					
		m_dattransform = 0;
	m_dataListCtrl.SetTransformMode(m_dattransform);
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	auto* p_mainframe = (CMainFrame*)AfxGetMainWnd();
	if (bActivate)
	{
		// make sure the secondary toolbar is not visible
		// (none is defined for the database)
		if (p_mainframe->m_pSecondToolBar != nullptr)
			p_mainframe->ShowPane(p_mainframe->m_pSecondToolBar, FALSE, FALSE, TRUE);
		// load status
		m_nStatus = ((CChildFrame*)p_mainframe->MDIGetActive())->m_nStatus;
		p_mainframe->PostMessageW(WM_MYMESSAGE, HINT_ACTIVATEVIEW, LPARAM(pActivateView->GetDocument()));
	}
	else
	{
		CChartDataWnd* pDataChartWnd = m_dataListCtrl.GetDataViewCurrentRecord();
		if (pDataChartWnd != nullptr)
		{
			((CdbWaveApp*)AfxGetApp())->options_viewdata.viewdata = *(pDataChartWnd->GetScopeParameters());
		}
		if (pActivateView != nullptr)
			((CChildFrame*)p_mainframe->MDIGetActive())->m_nStatus = m_nStatus;
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CViewdbWave::fillListBox()
{
	m_dataListCtrl.DeleteAllItems();
	const int imax = GetDocument()->GetDB_NRecords();
	m_dataListCtrl.SetItemCountEx(imax);
}

void CViewdbWave::OnItemActivateListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	// get item clicked and select it
	const auto p_item_activate = (NMITEMACTIVATE*)pNMHDR;
	if (p_item_activate->iItem >= 0)
		GetDocument()->SetDB_CurrentRecordPosition(p_item_activate->iItem);
	GetDocument()->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	CDaoRecordView::OnInitialUpdate();
	*pResult = 0;
}

void CViewdbWave::OnDblclkListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	// quit the current view and switch to spike detection view
	((CChildFrame*)GetParent())->PostMessage(WM_COMMAND, WPARAM(ID_VIEW_SPIKEDETECTION), LPARAM(NULL));
}

void CViewdbWave::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	switch (LOWORD(lHint))
	{
	case HINT_GETSELECTEDRECORDS:
	{
		auto p_document = GetDocument();
		p_document->m_selectedRecords.RemoveAll();
		const auto u_selected_count = m_dataListCtrl.GetSelectedCount();

		// Update all of the selected items.
		if (u_selected_count > 0)
		{
			p_document->m_selectedRecords.SetSize(u_selected_count);
			auto n_item = -1;
			for (UINT i = 0; i < u_selected_count; i++)
			{
				n_item = m_dataListCtrl.GetNextItem(n_item, LVNI_SELECTED);
				ASSERT(n_item != -1);
				p_document->m_selectedRecords.SetAt(i, n_item);
			}
		}
	}
	break;

	case HINT_SETSELECTEDRECORDS:
	{
		const auto p_document = GetDocument();
		const UINT u_selected_count = p_document->m_selectedRecords.GetSize();

		// clear previous selection in the CListCtrl if any
		auto item = -1;
		item = m_dataListCtrl.GetNextItem(item, LVNI_SELECTED);
		while (item != -1)
		{
			m_dataListCtrl.SetItemState(item, 0, LVIS_SELECTED);
			item = m_dataListCtrl.GetNextItem(item, LVNI_SELECTED);
		}

		// select items
		if (u_selected_count > 0)
		{
			for (UINT i = 0; i < u_selected_count; i++)
			{
				item = p_document->m_selectedRecords.GetAt(i);
				m_dataListCtrl.SetItemState(item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}
		m_dataListCtrl.EnsureVisible(item, FALSE);
	}
	break;

	case HINT_REPLACEVIEW:
		m_dataListCtrl.UpdateCache(-2, -2);
		updateControls();
		break;

	case HINT_REQUERY:
		fillListBox();
	case HINT_DOCHASCHANGED:
		m_dataListCtrl.UpdateCache(-1, -1);
	case HINT_DOCMOVERECORD:
	default:
		updateControls();
		break;
	}
}

void CViewdbWave::DeleteRecords()
{
	// save index current file
	auto currentindex = GetDocument()->GetDB_CurrentRecordPosition() - 1;
	if (currentindex < 0)
		currentindex = 0;

	// loop on Cdatalistctrl to delete all selected items
	auto pdb_doc = GetDocument();
	auto pos = m_dataListCtrl.GetFirstSelectedItemPosition();
	if (pos == nullptr)
	{
		AfxMessageBox(_T("No item selected: delete operation failed"));
		return;
	}
	else
	{
		// assume that no one else accesses to the database at the same time
		auto ndel = 0;
		// delete file names from database
		while (pos)
		{
			const auto n_item = m_dataListCtrl.GetNextSelectedItem(pos);
			pdb_doc->SetDB_CurrentRecordPosition(n_item - ndel);
			pdb_doc->DBDeleteCurrentRecord();
			ndel++;
		}
	}

	// If the recordset is now empty, clear the fields left over
	// from the deleted record
	/*if (m_pSet->IsBOF())
		m_pSet->SetFieldNull(NULL);
	m_pSet->RefreshQuery();*/

	pdb_doc->SetDB_CurrentRecordPosition(currentindex);
	pdb_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CViewdbWave::OnLvnColumnclickListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	auto filter0 = m_pSet->GetSQL();
	CString cs;
	const auto pdb_doc = GetDocument();
	switch (pNMLV->iSubItem)
	{
	case COL_CURVE:	cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_DATALEN].csColName; break;		// datalen
	case COL_INDEX:	cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_ID].csColName; break;			// ID
	case COL_SENSI:	cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_SENSILLUM_ID].csColName; break;	// sensillum_ID
	case COL_STIM1:	cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_STIM_ID].csColName; break;		// stim_ID
	case COL_CONC1:	cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_CONC_ID].csColName; break;		// conc_ID
	case COL_STIM2:	cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_STIM2_ID].csColName; break;	// stim2_ID
	case COL_CONC2:	cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_CONC2_ID].csColName; break;	// conc2_ID
	case COL_NBSPK: cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_NSPIKES].csColName; break;		// nspikes
	case COL_FLAG: cs = pdb_doc->m_pDB->m_mainTableSet.m_desc[CH_FLAG].csColName; break;			// flag
	default:
		break;
	}
	m_pSet->m_strSort = cs;
	m_pSet->Requery();
	GetDocument()->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	*pResult = 0;
}

void CViewdbWave::OnBnClickedRadio1()
{
	((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
	m_options_viewdata->displaymode = 1;
	m_dataListCtrl.SetDisplayMode(m_options_viewdata->displaymode);
	m_dataListCtrl.RefreshDisplay();
	m_tabCtrl.ShowWindow(SW_HIDE);

	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
}

void CViewdbWave::OnBnClickedRadio3()
{
	((CButton*)GetDlgItem(IDC_RADIO3))->SetCheck(BST_CHECKED);
	m_options_viewdata->displaymode = 0;
	m_tabCtrl.ShowWindow(SW_HIDE);
	m_dataListCtrl.SetDisplayMode(m_options_viewdata->displaymode);
	m_dataListCtrl.RefreshDisplay();

	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
}

void CViewdbWave::OnEnChangeTimefirst()
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
	default:;
	}

	mm_timefirst.m_bEntryDone = FALSE;
	mm_timefirst.m_nChar = 0;
	mm_timefirst.SetSel(0, -1); 	//select all text
	m_options_viewdata->tFirst = m_timefirst;
	if (m_timefirst > m_timelast)
		m_timefirst = 0.f;
	m_dataListCtrl.SetTimeIntervals(m_timefirst, m_timelast);
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnEnChangeTimelast()
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
	default:;
	}

	mm_timelast.m_bEntryDone = FALSE;
	mm_timelast.m_nChar = 0;
	mm_timelast.SetSel(0, -1); 	//select all text
	m_options_viewdata->tLast = m_timelast;
	m_dataListCtrl.SetTimeIntervals(m_timefirst, m_timelast);
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnEnChangeAmplitudespan()
{
	if (!mm_amplitudespan.m_bEntryDone)
		return;

	auto y = m_amplitudespan;
	CString cs;
	switch (mm_amplitudespan.m_nChar)
	{
	case VK_RETURN:
		mm_amplitudespan.GetWindowText(cs);
		y = float(_ttof(cs));
		break;
	case VK_UP:
	case VK_PRIOR:
		y++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		y--;
		break;
	default:;
	}

	// update the dialog control
	mm_amplitudespan.m_bEntryDone = FALSE;
	mm_amplitudespan.m_nChar = 0;
	mm_amplitudespan.SetSel(0, -1); 	//select all text
	cs.Format(_T("%.3f"), y);
	GetDlgItem(IDC_AMPLITUDESPAN)->SetWindowText(cs);
	m_amplitudespan = y;
	m_options_viewdata->mVspan = y;
	m_dataListCtrl.SetAmplitudeSpan(y);
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnBnClickedCheckfilename()
{
	m_options_viewdata->bDisplayFileName = IsDlgButtonChecked(IDC_CHECKFILENAME);
	m_dataListCtrl.SetDisplayFileName(m_options_viewdata->bDisplayFileName);
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnHdnEndtrackListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	if (phdr->iItem == COL_CURVE)
		m_dataListCtrl.ResizeSignalColumn(phdr->pitem->cxy);
	*pResult = 0;
}

void CViewdbWave::OnBnClickedCheck2()
{
	m_options_viewdata->bsetmVSpan = IsDlgButtonChecked(IDC_CHECK2);
	m_dataListCtrl.SetAmplitudeAdjustMode(m_options_viewdata->bsetmVSpan);
	m_dataListCtrl.RefreshDisplay();
	GetDlgItem(IDC_AMPLITUDESPAN)->EnableWindow(m_options_viewdata->bsetmVSpan);
}

void CViewdbWave::OnBnClickedCheck1()
{
	m_options_viewdata->bsetTimeSpan = IsDlgButtonChecked(IDC_CHECK1);
	// check intervals to avoid crash
	if (m_options_viewdata->bsetTimeSpan)
	{
		if (m_timefirst == m_timelast)
		{
			m_timelast++;
			m_options_viewdata->tLast = m_timelast;
			UpdateData(FALSE);
		}
		m_dataListCtrl.SetTimeIntervals(m_timefirst, m_timelast);
	}
	// update data display
	m_dataListCtrl.SetTimespanAdjustMode(m_options_viewdata->bsetTimeSpan);
	m_dataListCtrl.RefreshDisplay();
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(m_options_viewdata->bsetTimeSpan);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(m_options_viewdata->bsetTimeSpan);
}

void CViewdbWave::OnBnClickedRadioallclasses()
{
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
	m_options_viewdata->bDisplayAllClasses = TRUE;
	m_dataListCtrl.SetSpikePlotMode(PLOT_BLACK, m_spikeclass);
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnBnClickedRadiooneclass()
{
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);
	m_options_viewdata->bDisplayAllClasses = FALSE;
	m_dataListCtrl.SetSpikePlotMode(PLOT_ONECLASSONLY, m_spikeclass);
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnBnClickedDisplaySpikes()
{
	((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(BST_CHECKED);

	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(TRUE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);
	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);

	// display spikes
	m_options_viewdata->displaymode = 2;
	m_dataListCtrl.SetDisplayMode(m_options_viewdata->displaymode);
	if (m_options_viewdata->bDisplayAllClasses)
	{
		((CButton*)GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_CHECKED);
		((CButton*)GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_UNCHECKED);
		m_dataListCtrl.SetSpikePlotMode(PLOT_BLACK, 0);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_UNCHECKED);
		((CButton*)GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_CHECKED);
		m_spikeclass = m_options_viewdata->spikeclass;
		m_dataListCtrl.SetSpikePlotMode(PLOT_ONECLASSONLY, m_spikeclass);
	}
	m_dataListCtrl.RefreshDisplay();

	// update tab control
	int nrows = m_dataListCtrl.GetVisibleRowsSize();
	if (nrows > 0) {
		const auto pSpkDoc = m_dataListCtrl.GetVisibleRowsSpikeDocAt(0);
		if (pSpkDoc->GetSpkList_Size() > 1) {
			m_tabCtrl.InitctrlTabFromSpikeDoc(pSpkDoc);
			m_tabCtrl.ShowWindow(SW_SHOW);
			m_tabCtrl.SetCurSel(pSpkDoc->GetSpkList_CurrentIndex());
			m_tabCtrl.Invalidate();
		}
	}
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnEnChangeSpikeclass()
{
	if (!mm_spikeclass.m_bEntryDone)
		return;

	//auto spikeclassoption = m_spikeclass;
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
	default:;
	}
	mm_spikeclass.m_bEntryDone = FALSE;
	mm_spikeclass.m_nChar = 0;
	mm_spikeclass.SetSel(0, -1); 	//select all text
	m_options_viewdata->spikeclass = m_spikeclass;
	UpdateData(FALSE);
	m_dataListCtrl.SetSpikePlotMode(PLOT_ONECLASSONLY, m_spikeclass);
	m_dataListCtrl.RefreshDisplay();
}

void CViewdbWave::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	GetDocument()->GetcurrentSpkDocument()->SetSpkList_AsCurrent(icursel);
	m_dataListCtrl.RefreshDisplay();
	*pResult = 0;
}

void CViewdbWave::OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto icursel = m_tabCtrl.GetCurSel();
	*pResult = 0;
}