// SpikeTemplateView.cpp : implementation file
//

#include "StdAfx.h"
#include <math.h>
#include "dbWave.h"
#include "resource.h"

#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Editctrl.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"

#include "Spikedoc.h"
#include "spikeshape.h"
#include "TemplateWnd.h"
#include "TemplateListWnd.h"
#include "EditSpikeClassDlg.h"

#include "Editspik.h"
#include "MainFrm.h"
#include "Copyasdl.h"
#include "ChildFrm.h"
#include "ProgDlg.h"

#include ".\ViewSpikeTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeSort_Templates

IMPLEMENT_DYNCREATE(CViewSpikeSort_Templates, CDaoRecordView)

CViewSpikeSort_Templates::CViewSpikeSort_Templates()
	: CDaoRecordView(CViewSpikeSort_Templates::IDD), m_pSpkDoc(nullptr), m_pSpkList(nullptr), m_lFirst(0), m_lLast(0),
	  mdPM(nullptr), mdMO(nullptr), m_psC(nullptr), m_ktagleft(0), m_ktagright(0), m_scrollFilePos_infos()
{
	m_timefirst = 0.0f;
	m_timelast = 0.0f;
	m_hitrate = 0;
	m_ktolerance = 0.0f;
	m_spikenoclass = 0;
	m_hitratesort = 0;
	m_ifirstsortedclass = 0;
	m_ballfiles = FALSE;
	m_spikeno = -1;
	m_ballclasses = FALSE;
	m_binit = FALSE;
	m_ballTempl = FALSE;
	m_ballSort = FALSE;
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

//---------------------------------------------------------------------------

CViewSpikeSort_Templates::~CViewSpikeSort_Templates()
{
	// save spkD list i	 changed
	if (m_pSpkDoc != nullptr)
		SaveCurrentSpkFile();	// save file if modified
}

// --------------------------------------------------------------------------

BOOL CViewSpikeSort_Templates::PreCreateWindow(CREATESTRUCT &cs)
{
// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}


void CViewSpikeSort_Templates::OnDestroy() 
{
	CDaoRecordView::OnDestroy();
	if (m_templList.GetNtemplates() != 0)
	{
		if (m_psC->ptpl == nullptr)
			m_psC->CreateTPL();
		*((CTemplateListWnd*) m_psC->ptpl) = m_templList;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CViewSpikeSort_Templates diagnostics

#ifdef _DEBUG
void CViewSpikeSort_Templates::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CViewSpikeSort_Templates::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CViewSpikeSort_Templates::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeSort_Templates database support

CDaoRecordset* CViewSpikeSort_Templates::OnGetRecordset()
{
	return GetDocument()->DBGetRecordset();
}

//---------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if (bActivate)
	{
		CMainFrame* p_mainframe = (CMainFrame*) AfxGetMainWnd();
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, (LPARAM)pActivateView->GetDocument());
	}
	else
	{
		// set bincrflagonsave
		CdbWaveApp* p_app = (CdbWaveApp*) AfxGetApp();
		p_app->vdS.bincrflagonsave = ((CButton*) GetDlgItem(IDC_INCREMENTFLAG))->GetCheck();
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

//---------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (!m_binit)
		return;

	switch (LOWORD(lHint))
	{
	case HINT_DOCHASCHANGED:		// file has changed?
	case HINT_DOCMOVERECORD:
		UpdateFileParameters();
		break;
	case HINT_CLOSEFILEMODIFIED:	// close modified file: save
		SaveCurrentSpkFile();
		break;
	case HINT_REPLACEVIEW:
	default:
		break;
	}
}

BOOL CViewSpikeSort_Templates::OnMove(UINT nIDMoveCommand) 
{
	SaveCurrentSpkFile();	
	BOOL flag = CDaoRecordView::OnMove(nIDMoveCommand);
	CdbWaveDoc* p_document = GetDocument();
	if (p_document->DBGetCurrentSpkFileName(TRUE).IsEmpty())
	{
		((CChildFrame*)GetParent())->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
		return false;
	}

	p_document->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

//---------------------------------------------------------------------------

void CViewSpikeSort_Templates::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_HITRATE, m_hitrate);
	DDX_Text(pDX, IDC_TOLERANCE, m_ktolerance);
	DDX_Text(pDX, IDC_EDIT2, m_spikenoclass);
	DDX_Text(pDX, IDC_HITRATE2, m_hitratesort);
	DDX_Text(pDX, IDC_IFIRSTSORTEDCLASS, m_ifirstsortedclass);
	DDX_Check(pDX, IDC_CHECK1, m_ballfiles);
	DDX_Control(pDX, IDC_TAB1, m_tab1Ctrl);
	DDX_Control(pDX, IDC_TAB2, m_tabCtrl);
}


//---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CViewSpikeSort_Templates, CDaoRecordView)

	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT2, OnEnChangeclassno)
	ON_EN_CHANGE(IDC_TIMEFIRST, OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, OnEnChangeTimelast)
	ON_WM_HSCROLL()
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)
	ON_COMMAND(ID_FORMAT_ALLDATA, OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_GAINADJUST, OnFormatGainadjust)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, OnFormatCentercurve)
	ON_BN_CLICKED(IDC_BUILD, OnBuildTemplates)
	ON_EN_CHANGE(IDC_HITRATE, OnEnChangeHitrate)
	ON_EN_CHANGE(IDC_TOLERANCE, OnEnChangeTolerance)
	ON_BN_CLICKED(IDC_RADIO1, Onallclasses)
	ON_BN_CLICKED(IDC_RADIO2, Onsingleclass)
	ON_EN_CHANGE(IDC_HITRATE2, OnEnChangeHitrateSort)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST2, OnKeydownTemplateList)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_WM_DESTROY()	
	ON_WM_SETFOCUS()

	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CViewSpikeSort_Templates::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_SORT, &CViewSpikeSort_Templates::OnBnClickedSort)
	ON_BN_CLICKED(IDC_DISPLAY, &CViewSpikeSort_Templates::OnBnClickedDisplay)
	ON_EN_CHANGE(IDC_IFIRSTSORTEDCLASS, &CViewSpikeSort_Templates::OnEnChangeIfirstsortedclass)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB2, &CViewSpikeSort_Templates::OnTcnSelchangeTab2)
	ON_NOTIFY(NM_CLICK, IDC_TAB2, &CViewSpikeSort_Templates::OnNMClickTab2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeSort_Templates message handlers

void CViewSpikeSort_Templates::OnSize(UINT nType, int cx, int cy) 
{
	if (m_binit)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			m_stretch.ResizeControls(nType, cx, cy);
			break;
		default:
			break;
		}
	}
	CDaoRecordView::OnSize(nType, cx, cy);
}


//---------------------------------------------------------------------------

void CViewSpikeSort_Templates::SaveCurrentSpkFile()
{
	// save previous file if anything has changed
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		m_pSpkDoc->OnSaveDocument(GetDocument()->DBGetCurrentSpkFileName(FALSE));
		m_pSpkDoc->SetModifiedFlag(FALSE);
		
		GetDocument()->Setnbspikes(m_pSpkList->GetTotalSpikes());
		GetDocument()->Setnbspikeclasses(m_pSpkList->GetNbclasses());

		// change flag is button is checked
		if (((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck())
		{
			int flag = GetDocument()->DBGetCurrentRecordFlag();
			flag++;
			GetDocument()->DBSetCurrentRecordFlag(flag);
		}
	}
}


//---------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnInitialUpdate() 
{
	m_stretch.AttachParent(this);		// attach formview pointer
	m_stretch.newProp(IDC_LIST1, 		XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_LIST2, 		XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_LIST3, 		XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB2,			XLEQ_XREQ, SZEQ_YBEQ);

	// force resizing    
	m_binit = TRUE;

	VERIFY(m_spkForm.SubclassDlgItem(IDC_DISPLAYSPIKE, this));
	VERIFY(mm_spikenoclass.SubclassDlgItem(IDC_EDIT2, this));
	mm_spikenoclass.ShowScrollBar(SB_VERT);
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100,FALSE);
	VERIFY(m_templList.SubclassDlgItem(IDC_LIST2, this));
	VERIFY(m_avgList.SubclassDlgItem(IDC_LIST1, this));
	VERIFY(m_avgAllList.SubclassDlgItem(IDC_LIST3, this));

	VERIFY(mm_hitrate.SubclassDlgItem(IDC_HITRATE, this));
	mm_hitrate.ShowScrollBar(SB_VERT);
	VERIFY(mm_hitratesort.SubclassDlgItem(IDC_HITRATE2, this));
	mm_hitratesort.ShowScrollBar(SB_VERT);	
	VERIFY(mm_ktolerance.SubclassDlgItem(IDC_TOLERANCE, this));
	mm_ktolerance.ShowScrollBar(SB_VERT);
	VERIFY(mm_ifirstsortedclass.SubclassDlgItem(IDC_IFIRSTSORTEDCLASS, this));
	mm_ifirstsortedclass.ShowScrollBar(SB_VERT);

	CdbWaveApp* p_app = (CdbWaveApp*) AfxGetApp();	// load browse parameters
	mdPM = &(p_app->vdP);					// viewdata options
	mdMO = &(p_app->vdM);					// measure options
	m_psC= &(p_app->spkC);					// get address of spike classif parms
	if (m_psC->ptpl != nullptr)
		m_templList =  *((CTemplateListWnd*) m_psC->ptpl);

	CdbWaveDoc* p_dbwave_doc = GetDocument();
	
	// set bincrflagonsave
	((CButton*) GetDlgItem(IDC_INCREMENTFLAG))->SetCheck(p_app->vdS.bincrflagonsave);

	CDaoRecordView::OnInitialUpdate();

	// set ctrlTab values and extend its size
	CString cs= _T("Create Templates");
	m_tab1Ctrl.InsertItem(0, cs);
	cs = _T("Sort with Templates");
	m_tab1Ctrl.InsertItem(1, cs);
	cs = _T("Display Average");
	m_tab1Ctrl.InsertItem(2, cs);
	CRect rect;
	m_tab1Ctrl.GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	rect.bottom += 200;
	m_tab1Ctrl.MoveWindow(&rect, TRUE);

	// load data from file
	if (p_dbwave_doc->m_pSpk == nullptr)
	{
		p_dbwave_doc->m_pSpk = new CSpikeDoc;
		ASSERT(p_dbwave_doc->m_pSpk != NULL);
	}
	m_pSpkDoc = p_dbwave_doc->m_pSpk;

	m_hitrate = m_psC->hitrate;
	m_hitratesort = m_psC->hitratesort;
	m_ktolerance = m_psC->ktolerance;

	m_pSpkDoc = (CSpikeDoc*) GetDocument();
	m_spkForm.SetPlotMode(PLOT_ONECLASS, 0);
	m_ktagleft	= m_spkForm.AddVTtag(m_psC->kleft);
	m_ktagright = m_spkForm.AddVTtag(m_psC->kright);

	UpdateFileParameters();	
	((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(1);
	UpdateCtrlTab1(0);
}

//---------------------------------------------------------------------------
// UpdateFileParameters()
//---------------------------------------------------------------------------

void CViewSpikeSort_Templates::UpdateFileParameters()
{
	// init views
	GetDocument()->OpenCurrentSpikeFile();
	m_pSpkDoc = GetDocument()->m_pSpk;
	m_pSpkDoc->SetSpkListCurrent(GetDocument()->GetcurrentSpkListIndex());
	int icur = m_pSpkDoc->GetSpkListCurrentIndex();

	// reset tab control
	m_tabCtrl.DeleteAllItems();
	// load list of detection parameters 
	int j = 0;
	for (int i = 0; i< m_pSpkDoc->GetSpkListSize(); i++)
	{
		CSpikeList* p_spike_list = m_pSpkDoc->SetSpkListCurrent(i);
		CString cs;
		if (p_spike_list->GetdetectWhat() != 0)
			continue;
		cs.Format(_T("#%i %s"), i, (LPCTSTR) p_spike_list->GetComment());
		m_tabCtrl.InsertItem(j, cs);
		j++;
	}
	SelectSpikeList(icur);
}

void CViewSpikeSort_Templates::SelectSpikeList(int icur)
{
	// select spike list 
	GetDocument()->SetcurrentSpkListIndex(icur);
	m_pSpkList = m_pSpkDoc->SetSpkListCurrent(icur);
	m_tabCtrl.SetCurSel(icur);

	if (!m_pSpkList->IsClassListValid())		// if class list not valid:
	{
		m_pSpkList->UpdateClassList();			// rebuild list of classes
		m_pSpkDoc->SetModifiedFlag();			// and set modified flag
	}

	if (m_lFirst < 0)
		m_lFirst = 0;
	if (m_lLast > m_pSpkDoc->GetAcqSize()-1 || m_lLast <= m_lFirst)
		m_lLast = m_pSpkDoc->GetAcqSize()-1;
	m_scrollFilePos_infos.nMin = 0;				// tell HZ scroll what are the limits
	m_scrollFilePos_infos.nMax = m_lLast;

	// change pointer to select new spike list & test if one spike is selected
	int spikeno = m_pSpkList->m_selspike;
	if (spikeno > m_pSpkList->GetTotalSpikes()-1 || spikeno < 0)
		spikeno = -1;
	else	
	{	// set source class to the class of the selected spike
		m_spikenoclass = m_pSpkList->GetSpikeClass(spikeno);		
		m_psC->sourceclass=m_spikenoclass;
	}
	if (m_spikenoclass > 32768)
		m_spikenoclass = 0;

	// prepare display source spikes
	m_spkForm.SetSourceData(m_pSpkList);
	if (m_psC->kleft==0 && m_psC->kright==0)
	{
		m_psC->kleft = m_pSpkList->GetSpikePretrig();
		m_psC->kright = m_psC->kleft +  m_pSpkList->GetSpikeRefractory();
	}
	if (m_ballclasses)
		m_spkForm.SetPlotMode(PLOT_BLACK, 0);
	else	
		m_spkForm.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);

	m_lFirst = 0;
	m_lLast = m_pSpkDoc->GetAcqSize()-1;

	m_spkForm.SetTimeIntervals(m_lFirst, m_lLast);
	m_spkForm.Invalidate();

	SelectSpike(spikeno);
	UpdateLegends();

	DisplayAvg(FALSE, &m_avgList); //&m_ImListAvg);	// not sure this is necessary....
	UpdateTemplates();	
}

//----------------------------------------------------------------------------

void CViewSpikeSort_Templates::UpdateTemplates()
{
	int n_cmd_show = SW_HIDE;
	if (m_templList.GetNtemplates() > 0)
	{
		if(m_templList.GetImageList(LVSIL_NORMAL) != &m_templList.m_imageList) //&m_ImListTpl)
		{
			CRect rect;
			m_spkForm.GetClientRect(&rect);
			m_templList.m_imageList.Create(rect.right, rect.bottom, ILC_COLOR8, 4, 1);
			m_templList.SetImageList(&m_templList.m_imageList, LVSIL_NORMAL);
		}	
		SetDlgItemInt(IDC_NTEMPLATES, m_templList.GetNtemplates());
		m_templList.SetYWExtOrg(m_spkForm.GetYWExtent(), m_spkForm.GetYWOrg());
		m_templList.UpdateTemplateLegends("t");
		m_templList.Invalidate();
		n_cmd_show = SW_SHOW;
	}
	GetDlgItem(IDC_NTEMPLS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_NTEMPLATES)->ShowWindow(n_cmd_show);
}

//----------------------------------------------------------------------------

void CViewSpikeSort_Templates::UpdateLegends()
{
	if (m_lFirst < 0)
		m_lFirst = 0;
	if (m_lLast <= m_lFirst)
		m_lLast = m_lFirst + 120;
	if (m_lLast >= m_pSpkDoc->GetAcqSize())
		m_lLast = m_pSpkDoc->GetAcqSize()-1;
	if (m_lFirst > m_lLast)
		m_lFirst = m_lLast -120;

	// update text abcissa and horizontal scroll position
	m_timefirst = m_lFirst/m_pSpkDoc->GetAcqRate();
	m_timelast = (m_lLast+1)/m_pSpkDoc->GetAcqRate();	

	// store current file settings
	m_pSpkList->m_lFirstSL = m_lFirst;
	m_pSpkList->m_lLastSL = m_lLast;

	// draw dependent buttons
	m_spkForm.SetTimeIntervals(m_lFirst, m_lLast);
	m_spkForm.Invalidate();

	m_pSpkList->m_lFirstSL = m_lFirst;
	m_pSpkList->m_lLastSL = m_lLast;
	UpdateData(FALSE);	// copy view object to controls	
	UpdateScrollBar();
}

// -------------------------------------------------------------------------
// SelectSpike - display selected spike in the form window 

void CViewSpikeSort_Templates::SelectSpike(short spikeno)
{
	m_spkForm.SelectSpikeShape(spikeno);
	m_spikeno = spikeno;
	m_pSpkList->m_selspike = spikeno;
}

/////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------------------------
// message handler for messages from CScopeScreenWnd windows and derived classes

LRESULT CViewSpikeSort_Templates::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	short threshold = LOWORD(lParam);	// value associated	

	switch (wParam)
	{
	case HINT_SETMOUSECURSOR: // ------------- change mouse cursor (all 3 items)	
		if (threshold >CURSOR_ZOOM)	// clip cursor shape to max
			threshold = 0;		
		SetViewMouseCursor(threshold);	// change cursor val in the other button
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(threshold, 0));
		break;

	case HINT_HITSPIKE:		// -------------  spike is selected or deselected
		SelectSpike(threshold);
		break;

	case HINT_CHANGEVERTTAG: // -------------  vertical tag val has changed
		if (threshold == m_ktagleft)		// first tag
		{
			m_psC->kleft = m_spkForm.GetVTtagVal(m_ktagleft);
		}
		else if (threshold == m_ktagright)	// second tag
		{
			m_psC->kright = m_spkForm.GetVTtagVal(m_ktagright);
		}
		m_templList.SetTemplateLength(0, m_psC->kleft, m_psC->kright);
		m_templList.Invalidate();
		break;

	case HINT_CHANGEHZLIMITS:		// abcissa have changed
	case HINT_CHANGEZOOM:
	case HINT_VIEWSIZECHANGED:       // change zoom		
		m_templList.SetYWExtOrg(m_spkForm.GetYWExtent(), m_spkForm.GetYWOrg());
		m_templList.Invalidate();
		m_avgList.SetYWExtOrg(m_spkForm.GetYWExtent(), m_spkForm.GetYWOrg());
		m_avgList.Invalidate();
		UpdateLegends();
		break;

	case HINT_RMOUSEBUTTONDOWN:
		EditSpikeClass(HIWORD(lParam), threshold);
		break;

	default:
		break;		
	}
	return 0L;
}


// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::Onallclasses() 
{
	m_ballclasses = TRUE;
	GetDlgItem(IDC_EDIT2)->ShowWindow(SW_HIDE);
	m_spkForm.SetPlotMode(PLOT_BLACK, m_spikenoclass);
	m_spkForm.Invalidate();
}

void CViewSpikeSort_Templates::Onsingleclass() 
{
	m_ballclasses = FALSE;
	GetDlgItem(IDC_EDIT2)->ShowWindow(SW_SHOW);
	m_spkForm.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);
	m_spkForm.Invalidate();
}


// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnEnChangeclassno() 
{
	if (!mm_spikenoclass.m_bEntryDone)
		return;

	int spikenoclass = m_spikenoclass;
	switch (mm_spikenoclass.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_spikenoclass++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_spikenoclass--;	break;
	}

	mm_spikenoclass.m_bEntryDone=FALSE;	// clear flag
	mm_spikenoclass.m_nChar=0;			// empty buffer
	mm_spikenoclass.SetSel(0, -1);		// select all text
	
	if (m_spikenoclass != spikenoclass)	// change display if necessary
	{		
		m_spkForm.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);
		m_spkForm.Invalidate();
		UpdateLegends();
	}
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnEnChangeTimefirst() 
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

	long l_first = (long) (m_timefirst*m_pSpkDoc->GetAcqRate());
	if (l_first != m_lFirst)
	{
		m_lFirst = l_first;
		UpdateLegends();
	}	
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnEnChangeTimelast() 
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

	long l_last = (long) (m_timelast*m_pSpkDoc->GetAcqRate());
	if (l_last != m_lLast)
	{
		m_lLast = l_last;
		UpdateLegends();
	}
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// get corresponding data
	long totalScroll= m_pSpkDoc->GetAcqSize();
	long pageScroll = (m_lLast - m_lFirst);
	long sbScroll = pageScroll /10;
	if (sbScroll == 0)
		sbScroll = 1;
	long l_first = m_lFirst;
	long l_last = m_lLast;	

	switch (nSBCode)
	{		
	case SB_LEFT:		l_first = 0;	break;			// Scroll to far left.
	case SB_LINELEFT:	l_first -= sbScroll;	break;	// Scroll left.
	case SB_LINERIGHT:	l_first += sbScroll; break;	// Scroll right
	case SB_PAGELEFT:	l_first -= pageScroll; break;// Scroll one page left
	case SB_PAGERIGHT:	l_first += pageScroll; break;// Scroll one page right.
	case SB_RIGHT:		l_first = totalScroll - pageScroll+1; 
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		l_first = (int) nPos;
		break;
	default:
		return;
		break;
	}

	if (l_first < 0)
		l_first = 0;

	l_last = l_first + pageScroll;
	if (l_last >= totalScroll)
	{
		l_last = totalScroll - 1;
		l_first = l_last - pageScroll;
	}
	
	// adjust display
	if (l_first != m_lFirst)
	{
		m_lFirst = l_first;
		m_lLast = l_last;
		UpdateLegends();
	}
	else
		UpdateScrollBar();
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::UpdateScrollBar()
{
	if (m_lFirst == 0 && m_lLast >= m_pSpkDoc->GetAcqSize()-1)
		GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_HIDE);
	else
	{
		GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_SHOW);

		m_scrollFilePos_infos.fMask = SIF_ALL;
		m_scrollFilePos_infos.nPos = m_lFirst;
		m_scrollFilePos_infos.nPage = m_lLast-m_lFirst;
		((CScrollBar*) GetDlgItem(IDC_SCROLLBAR1))->SetScrollInfo(&m_scrollFilePos_infos);
	}
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnFormatAlldata() 
{
	// dots: spk file length
	m_lFirst = 0;
	m_lLast = m_pSpkDoc->GetAcqSize()-1;
	// spikes: center spikes horizontally and adjust hz size of display	
	short x_wo = 0;
	short x_we = m_pSpkList->GetSpikeLength();
	m_spkForm.SetXWExtOrg(x_we, x_wo);
	UpdateLegends();	
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnFormatGainadjust() 
{
	int maxval, minval;
	m_pSpkList->GetTotalMaxMin(TRUE, &maxval, &minval);
	int extent = maxval - minval;  
	int zero = (maxval + minval)/2;
	m_spkForm.SetYWExtOrg(extent, zero);
	m_spkForm.Invalidate();

	m_templList.SetYWExtOrg(extent, zero);
	m_templList.Invalidate();
	m_avgList.SetYWExtOrg(extent, zero);
	m_avgList.Invalidate();
	m_avgAllList.SetYWExtOrg(extent, zero);
	m_avgAllList.Invalidate();
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnFormatCentercurve() 
{
	int maxval, minval;
	m_pSpkList->GetTotalMaxMin(TRUE, &maxval, &minval);
	int extent = m_spkForm.GetYWExtent();
	int zero = (maxval + minval)/2;
	m_spkForm.SetYWExtOrg(extent, zero);
	m_spkForm.Invalidate();
	m_templList.SetYWExtOrg(extent, zero);
	m_templList.Invalidate();
	m_avgList.SetYWExtOrg(extent, zero);
	m_avgList.Invalidate();
	m_avgAllList.SetYWExtOrg(extent, zero);
	m_avgAllList.Invalidate();
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnEnChangeHitrate() 
{
	if (!mm_hitrate.m_bEntryDone)
		return;

	switch (mm_hitrate.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_hitrate++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_hitrate--;
		break;
	}
	
	mm_hitrate.m_bEntryDone=FALSE;
	mm_hitrate.m_nChar=0;
	mm_hitrate.SetSel(0, -1); 	//select all text

	if (m_psC->hitrate != m_hitrate)
	{
		m_psC->hitrate = m_hitrate;		
	}
	UpdateData(FALSE);
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnEnChangeHitrateSort() 
{
	if (!mm_hitratesort.m_bEntryDone)
		return;

	switch (mm_hitratesort.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_hitratesort++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_hitratesort--;
		break;
	}
	
	mm_hitratesort.m_bEntryDone=FALSE;
	mm_hitratesort.m_nChar=0;
	mm_hitratesort.SetSel(0, -1); 	//select all text

	if (m_psC->hitratesort != m_hitratesort)
	{
		m_psC->hitratesort = m_hitratesort;		
	}
	UpdateData(FALSE);
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnEnChangeTolerance() 
{
	if (!mm_ktolerance.m_bEntryDone)
		return;

	switch (mm_ktolerance.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_ktolerance += 0.1f;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_ktolerance -= 0.1f;
		break;
	}
	
	if (m_ktolerance < 0)
		m_ktolerance = -m_ktolerance;
	mm_ktolerance.m_bEntryDone=FALSE;
	mm_ktolerance.m_nChar=0;
	mm_ktolerance.SetSel(0, -1); 	//select all text

	if (m_psC->ktolerance != m_ktolerance)
	{
		m_psC->ktolerance = m_ktolerance;		
	}
	UpdateData(FALSE);	
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::DisplayAvg(BOOL ballfiles, CTemplateListWnd* pTPList) //, CImageList* pImList) 
{	
	// get list of classes	
	pTPList->SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);
	int tpllen = m_psC->kright - m_psC->kleft + 1;

	// define and attach to ImageList to CListCtrl; create 1 item by default
	if (pTPList->GetImageList(LVSIL_NORMAL) != &pTPList->m_imageList)
	{
		CRect rect;
		m_spkForm.GetClientRect(&rect);
		pTPList->m_imageList.Create(rect.right, rect.bottom, ILC_COLOR8, 1, 1);
		pTPList->SetImageList(&pTPList->m_imageList, LVSIL_NORMAL);
	}

	// reinit all templates to zero
	pTPList->DeleteAllItems();
	pTPList->SetTemplateLength(m_pSpkList->GetSpikeLength(), 0, m_pSpkList->GetSpikeLength()-1);
	pTPList->SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);

	int zero = m_spkForm.GetYWOrg();
	int extent = m_spkForm.GetYWExtent();
	if (zero == 0 && extent == 0)
	{
		int maxval, minval;
		m_pSpkList->GetTotalMaxMin(TRUE, &maxval, &minval);
		extent = maxval - minval;  
		zero = (maxval + minval)/2;
		m_spkForm.SetYWExtOrg(extent, zero);
	}
	pTPList->SetYWExtOrg(extent, zero);

	// set file indexes - assume only one file selected
	CdbWaveDoc* p_dbwave_doc = GetDocument();
	int currentfile = p_dbwave_doc->DBGetCurrentRecordPosition(); // index current file	
	int firstfile = currentfile;		// index first file in the series
	int lastfile = currentfile;			// index last file in the series
	// make sure we have the correct spike list here
	int currentlist = m_tabCtrl.GetCurSel();
	CSpikeList* p_spike_element = m_pSpkDoc->SetSpkListCurrent(currentlist);
	
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");

	if (ballfiles)
	{
		firstfile = 0;						// index first file
		lastfile = p_dbwave_doc->DBGetNRecords() -1;	// index last file
	}	
	// loop over files
	int nfiles = lastfile - firstfile +1;
	for (int ifile=firstfile; ifile <= lastfile; ifile++)
	{
		// load file
		p_dbwave_doc->DBSetCurrentRecordPosition(ifile);
		if (!p_dbwave_doc->OpenCurrentSpikeFile())
			continue;
		CString cs;
		cs.Format(_T("%i/%i - "), ifile, lastfile);
		cs += p_dbwave_doc->DBGetCurrentSpkFileName(FALSE);
		p_dbwave_doc->SetTitle(cs);
		m_pSpkDoc = p_dbwave_doc->m_pSpk;
		m_pSpkDoc->SetModifiedFlag(FALSE);
	
		m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist); // load pointer to spike list
		if (!m_pSpkList->IsClassListValid())		// if class list not valid:
		{
			m_pSpkList->UpdateClassList();			// rebuild list of classes
			m_pSpkDoc->SetModifiedFlag();			// and set modified flag
		}
		int nspikes = m_pSpkList->GetTotalSpikes();	

		// add spikes to templates - create templates on the fly
		int jTempl=0;
		for (int i=0; i<nspikes; i++)
		{
			int cla = m_pSpkList->GetSpikeClass(i);
			BOOL bFound = FALSE;
			for (jTempl=0; jTempl < pTPList->GetTemplateDataSize(); jTempl++)
			{
				if (cla == pTPList->GetTemplateclassID(jTempl))
				{
					bFound = TRUE;
					break;
				}
				if (cla < pTPList->GetTemplateclassID(jTempl))
					break;
			}
			// add template if not found - insert it at the proper place
			if (!bFound) // add item if not found
			{
				if (jTempl < 0) 
					jTempl = 0;
				int jTempl0 = jTempl;
				jTempl=pTPList->InsertTemplateData(jTempl, cla);
			}

			// get data and add spike
			short* pSpik = m_pSpkList->GetpSpikeData(i);
			pTPList->tAdd(jTempl, pSpik);	// add spike to template j
			pTPList->tAdd(pSpik);		// add spike to template zero
		}
	}

	// restore current file index
	// end of loop, select current file again if necessary
	if (ballfiles)
	{
		p_dbwave_doc->DBSetCurrentRecordPosition(currentfile);
		p_dbwave_doc->OpenCurrentSpikeFile();
		m_pSpkDoc = p_dbwave_doc->m_pSpk;
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}
	
	// update average
	pTPList->TransferTemplateData();
	pTPList->tGlobalstats();
	pTPList->UpdateTemplateLegends("c");
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnBuildTemplates() 
{
	// set file indexes - assume only one file selected
	CdbWaveDoc* p_dbwave_doc = GetDocument();
	int currentfile = p_dbwave_doc->DBGetCurrentRecordPosition(); // index current file	
	int firstfile = currentfile;		// index first file in the series
	int lastfile = firstfile;			// index last file in the series
	// get current selected list
	int currentlist = m_tabCtrl.GetCurSel();

	// change indexes if ALL files selected
	if (m_ballfiles)
	{
		firstfile = 0;						// index first file
		lastfile = p_dbwave_doc->DBGetNRecords() -1;	// index last file
	}	

	// add as many forms as we have classes	
	int tpllen = m_psC->kright - m_psC->kleft + 1;
	m_templList.DeleteAllItems();	// reinit all templates to zero
	m_templList.SetTemplateLength(m_pSpkList->GetSpikeLength(), m_psC->kleft, m_psC->kright);
	m_templList.SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);

	// compute global std
	// loop over all selected files (or only one file currently selected)
	int nspikes;
	int ifile=0;
	int nfiles = lastfile - firstfile +1;
	int istep = 0;
	CString cscomment;
	CString csfilecomment;

	for (ifile=firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_ballfiles)
		{
			p_dbwave_doc->DBSetCurrentRecordPosition(ifile);
			p_dbwave_doc->OpenCurrentSpikeFile();
			m_pSpkDoc = p_dbwave_doc->m_pSpk;			
		}

		CSpikeList* p_spike_element = m_pSpkDoc->SetSpkListCurrent(currentlist);
		nspikes = p_spike_element->GetTotalSpikes();		
		for (int i=0; i<nspikes; i++)
			m_templList.tAdd(m_pSpkList->GetpSpikeData(i));
	}
	m_templList.tGlobalstats();
	
	// now scan all spikes to build templates
	int ntempl=0;
	double distmin;
	int offsetmin;
	int tplmin;
	int ilast = 0;
	csfilecomment = _T("Second pass - analyze file: ");
	
	for (ifile=firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_ballfiles)
		{
			p_dbwave_doc->DBSetCurrentRecordPosition(ifile);
			p_dbwave_doc->OpenCurrentSpikeFile();
			m_pSpkDoc = p_dbwave_doc->m_pSpk;
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->DBGetCurrentSpkFileName(FALSE);
			p_dbwave_doc->SetTitle(cs);			
		}

		CSpikeList* p_spike_element = m_pSpkDoc->SetSpkListCurrent(currentlist);
		nspikes = p_spike_element->GetTotalSpikes();

		// create template CListCtrl
		for (int i=0; i<nspikes; i++)
		{
			// filter out undesirable spikes
			if (!m_ballclasses)
			{
				if ( m_pSpkList->GetSpikeClass(i) != m_spikenoclass)
					continue;
			}
			long iitime = m_pSpkList->GetSpikeTime(i);
			if (iitime < m_lFirst || iitime > m_lLast)
				continue;

			// get pointer to spike data and search if any template is suitable
			short* pSpik = m_pSpkList->GetpSpikeData(i);		
			BOOL bWithin=FALSE;
			int itpl=0;
			for (itpl = 0; itpl < ntempl; itpl++)
			{
				// exit loop if spike is within template
				bWithin = m_templList.tWithin(itpl, pSpik);
				if (bWithin)
					break;
				// OR exit loop if spike dist is less distant
				m_templList.tMinDist(itpl, pSpik, &offsetmin, &distmin);
				bWithin = (distmin <= m_templList.m_globaldist);
				if (bWithin)
					break;
			}

			// if a template is suitable, find the most likely
			if (bWithin)
			{
				tplmin = itpl;
				distmin = m_templList.m_globaldist;
				double x;
				int offset;
				for (int itpl = 0; itpl < ntempl; itpl++)
				{
					m_templList.tMinDist(itpl, pSpik, &offset, &x);
					if (x < distmin)
					{
						offsetmin = offset;
						distmin = x;
						tplmin = itpl;
					}
				}
			}
			// else (no suitable template), create a new one
			else
			{
				m_templList.InsertTemplate(ntempl, ntempl+m_ifirstsortedclass);
				tplmin = ntempl;
				ntempl++;
			}

			// add spike to the corresp template
			m_templList.tAdd(tplmin, pSpik);	// add spike to template j		
		}
	}

	// end of loop, select current file again if necessary
	if (m_ballfiles)
	{
		p_dbwave_doc->DBSetCurrentRecordPosition(currentfile);
		p_dbwave_doc->OpenCurrentSpikeFile();
		m_pSpkDoc = p_dbwave_doc->m_pSpk;
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	m_templList.SortTemplatesByNumberofSpikes(TRUE, TRUE, m_ifirstsortedclass);
	UpdateTemplates();	
}

// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::SortSpikes() 
{
	// set tolerance to sort tolerance
	m_templList.SetHitRate_Tolerance(&m_hitratesort, &m_ktolerance);

	// set file indexes - assume only one file selected
	CdbWaveDoc* p_dbwave_doc = GetDocument();
	int currentfile = p_dbwave_doc->DBGetCurrentRecordPosition(); // index current file	
	int firstfile = currentfile;		// index first file in the series
	int lastfile = firstfile;			// index last file in the series

	// get current spike list
	int currentlist = m_tabCtrl.GetCurSel();
	
	// change indexes if ALL files selected
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	if (m_ballfiles)
	{
		firstfile = 0;						// index first file
		lastfile = p_dbwave_doc->DBGetNRecords() -1;	// index last file
	}	

	// loop CFrameWnd
	int ntempl = m_templList.GetNtemplates();
	int nfiles = lastfile - firstfile +1;
	for (int ifile=firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_ballfiles)
		{
			p_dbwave_doc->DBSetCurrentRecordPosition(ifile);
			p_dbwave_doc->OpenCurrentSpikeFile();
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->DBGetCurrentSpkFileName(FALSE);
			p_dbwave_doc->SetTitle(cs);
			m_pSpkDoc = p_dbwave_doc->m_pSpk;
			m_pSpkDoc->SetModifiedFlag(FALSE);
			
			m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist); // load pointer to spike list
			if (!m_pSpkList->IsClassListValid())		// if class list not valid:
			{
				m_pSpkList->UpdateClassList();			// rebuild list of classes
				m_pSpkDoc->SetModifiedFlag();			// and set modified flag
			}
		}

		// spike loop
		int nspikes = m_pSpkList->GetTotalSpikes();	// loop over all spikes	
		for (int ispike=0; ispike < nspikes; ispike++)
		{
			// filter out undesirable spikes - i.e. not relevant to the sort
			if (!m_ballclasses)
			{
				// skip spikes that do not belong to selected class
				if ( m_pSpkList->GetSpikeClass(ispike) != m_spikenoclass)
					continue;
			}

			// skip spikes that do not fit into time interval selected
			long iitime = m_pSpkList->GetSpikeTime(ispike);
			if (iitime < m_lFirst || iitime > m_lLast)
				continue;

			// get pointer to spike data and search if any template is suitable
			short* pSpik = m_pSpkList->GetpSpikeData(ispike);
			BOOL bWithin=FALSE;
			double distmin;
			int offsetmin;

			// search first template that meet criteria
			int tplmin=0;
			for (tplmin = 0; tplmin < ntempl; tplmin++)
			{
				// exit loop if spike is within template
				bWithin = m_templList.tWithin(tplmin, pSpik);
				m_templList.tMinDist(tplmin, pSpik, &offsetmin, &distmin);
				if (bWithin)
					break;

				// OR exit loop if spike dist is less distant
				bWithin = (distmin <= m_templList.m_globaldist);
				if (bWithin)
					break;
			}

			// if a template is suitable, find the most likely
			if (bWithin)
			{			
				for (int itpl = tplmin+1; itpl < ntempl; itpl++)
				{
					double x;
					int offset;
					m_templList.tMinDist(itpl, pSpik, &offset, &x);
					if (x < distmin)
					{
						offsetmin = offset;
						distmin = x;
						tplmin = itpl;
					}
				}

				// change spike class ID
				int classID = (m_templList.GetTemplateWnd(tplmin))->m_classID;
				if ( m_pSpkList->GetSpikeClass(ispike) != classID)
				{
					m_pSpkList->SetSpikeClass(ispike, classID);
					m_pSpkDoc->SetModifiedFlag(TRUE);
				}
			}
		}
		if (m_pSpkDoc->IsModified())
		{
			m_pSpkDoc->OnSaveDocument(p_dbwave_doc->DBGetCurrentSpkFileName(FALSE));
			m_pSpkDoc->SetModifiedFlag(FALSE);

			GetDocument()->Setnbspikes(m_pSpkList->GetTotalSpikes());
			GetDocument()->Setnbspikeclasses(m_pSpkList->GetNbclasses());
		}
	}

	// end of loop, select current file again if necessary
	if (m_ballfiles)
	{
		p_dbwave_doc->DBSetCurrentRecordPosition(currentfile);
		p_dbwave_doc->OpenCurrentSpikeFile();
		m_pSpkDoc = p_dbwave_doc->m_pSpk;
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update display: average and spk form
	DisplayAvg(FALSE, &m_avgList);
	m_pSpkList = m_pSpkDoc->GetSpkListCurrent();
	m_spkForm.SetSourceData(m_pSpkList); 
	m_spkForm.Invalidate();
}


// --------------------------------------------------------------------------

void CViewSpikeSort_Templates::OnKeydownTemplateList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	
	// delete selected template
	if (pLVKeyDow->wVKey == VK_DELETE && m_templList.GetSelectedCount() > 0)
	{
		BOOL flag = FALSE;
		int isup = m_templList.GetItemCount();
		int cla = m_ifirstsortedclass;
		for (int i= 0; i< isup; i++)
		{
			UINT state = m_templList.GetItemState(i, LVIS_SELECTED);
			if (state > 0)
			{
				cla = m_templList.GetTemplateclassID(i);
				m_templList.DeleteItem(i);
				flag = TRUE;
				continue;
			}
			if (flag)
			{
				m_templList.SetTemplateclassID(i-1, _T("t"), cla);
				cla++;
			}
		}
	}
	SetDlgItemInt(IDC_NTEMPLATES, m_templList.GetItemCount());
	m_templList.Invalidate();
	*pResult = 0;
}


void CViewSpikeSort_Templates::OnCheck1() 
{	
	UpdateData(TRUE);
}

void CViewSpikeSort_Templates::EditSpikeClass(int controlID, int controlItem)
{
	// find which item has been selected
	CTemplateListWnd* pList = nullptr; 
	BOOL bSpikes = TRUE; 
	BOOL bAllFiles = m_ballfiles;
	if (m_avgList.GetDlgCtrlID() == controlID)
		pList = &m_avgList;
	else if (m_templList.GetDlgCtrlID() == controlID)
	{
		pList = &m_templList;
		bSpikes = FALSE;
	}
	else if (m_avgAllList.GetDlgCtrlID() == controlID)
	{
		pList = &m_avgAllList;
		bAllFiles = TRUE;
	}
	if (pList == nullptr)
		return; 

	// find which icon has been selected and get the key
	int oldclass = pList->GetTemplateclassID(controlItem);	

	// launch edit dlg
	CEditSpikeClassDlg dlg;
	dlg.m_iClass = oldclass;
	if (IDOK == dlg.DoModal() && oldclass != dlg.m_iClass)
	{
		// templates
		if (!bSpikes)
			pList->SetTemplateclassID(controlItem, _T("t"), dlg.m_iClass);
		// spikes
		else
		{
			pList->SetTemplateclassID(controlItem, _T("c"), dlg.m_iClass);

			// set file indexes - assume only one file selected
			CdbWaveDoc* p_dbwave_doc = GetDocument();
			int currentfile = p_dbwave_doc->DBGetCurrentRecordPosition(); // index current file	
			int firstfile = currentfile;		// index first file in the series
			int lastfile = firstfile;			// index last file in the series
			int currentlist = m_tabCtrl.GetCurSel();

			// change indexes if ALL files selected
			int istep = 0;
			CString cscomment;
			CString csfilecomment = _T("Analyze file: ");
			if (bAllFiles)
			{
				firstfile = 0;						// index first file
				lastfile = p_dbwave_doc->DBGetNRecords() -1;	// index last file
			}
			
				// loop CFrameWnd
			int ntempl = m_templList.GetNtemplates();
			int nfiles = lastfile - firstfile +1;
			for (int ifile=firstfile; ifile <= lastfile; ifile++)
			{
				// store nb of spikes within array
				if (bAllFiles)
				{
					p_dbwave_doc->DBSetCurrentRecordPosition(ifile);
					p_dbwave_doc->OpenCurrentSpikeFile();
					CString cs;
					cs.Format(_T("%i/%i - "), ifile, lastfile);
					cs += p_dbwave_doc->DBGetCurrentSpkFileName(FALSE);
					p_dbwave_doc->SetTitle(cs);
					m_pSpkDoc = p_dbwave_doc->m_pSpk;
					m_pSpkDoc->SetModifiedFlag(FALSE);
					m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist);
				}

				// TODO: this should not work - changing SpikeClassID does not change the spike class because UpdateClassList reset classes array to zero
				m_pSpkList->UpdateClassList();			// rebuild list of classes
				m_pSpkList->ChangeSpikeClassID(oldclass, dlg.m_iClass);
				m_pSpkList->UpdateClassList();			// rebuild list of classes
				m_pSpkDoc->SetModifiedFlag(TRUE);
	
				if (m_pSpkDoc->IsModified())
				{
					m_pSpkDoc->OnSaveDocument(p_dbwave_doc->DBGetCurrentSpkFileName(FALSE));
					m_pSpkDoc->SetModifiedFlag(FALSE);

					GetDocument()->Setnbspikes(m_pSpkList->GetTotalSpikes());
					GetDocument()->Setnbspikeclasses(m_pSpkList->GetNbclasses());
				}
			}

			// end of loop, select current file again if necessary
			if (bAllFiles)
			{
				p_dbwave_doc->DBSetCurrentRecordPosition(currentfile);
				p_dbwave_doc->OpenCurrentSpikeFile();
				m_pSpkDoc = p_dbwave_doc->m_pSpk;
				p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
			}
		}
	}
}
void CViewSpikeSort_Templates::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	// switch display
	int iselect = m_tab1Ctrl.GetCurSel();
	UpdateCtrlTab1(iselect);
}

void CViewSpikeSort_Templates::UpdateCtrlTab1(int iselect)
{
	WORD iTempl = SW_SHOW;
	WORD iAvg = SW_HIDE;
	WORD iSort = SW_HIDE;
	switch (iselect)
	{
	case 0:
		m_avgAllList.ShowWindow(SW_HIDE);
		m_templList.ShowWindow(SW_SHOW);
		break;
	case 1:
		iTempl = SW_HIDE;
		iSort = SW_SHOW;
		iAvg = SW_HIDE;
		m_avgAllList.ShowWindow(SW_HIDE);
		m_templList.ShowWindow(SW_SHOW);
		break;
	case 2:
		iTempl = SW_HIDE;
		iSort = SW_HIDE;
		iAvg = SW_SHOW;
		m_avgAllList.ShowWindow(SW_SHOW);
		m_templList.ShowWindow(SW_HIDE);
		break;
	default:
		break;
	}

	// build templates 
	GetDlgItem(IDC_STATIC3)->ShowWindow(iTempl);
	GetDlgItem(IDC_HITRATE)->ShowWindow(iTempl);
	GetDlgItem(IDC_STATIC4)->ShowWindow(iTempl);
	GetDlgItem(IDC_TOLERANCE)->ShowWindow(iTempl);
	GetDlgItem(IDC_BUILD)->ShowWindow(iTempl);
	GetDlgItem(IDC_LOAD_SAVE)->ShowWindow(iTempl);
	GetDlgItem(IDC_CHECK1)->ShowWindow(iTempl);
	GetDlgItem(IDC_NTEMPLS)->ShowWindow(iTempl);
	GetDlgItem(IDC_NTEMPLATES)->ShowWindow(iTempl);
	GetDlgItem(IDC_TFIRSTSORTEDCLASS)->ShowWindow(iTempl);
	GetDlgItem(IDC_IFIRSTSORTEDCLASS)->ShowWindow(iTempl);

	// sort spikes using templates
	GetDlgItem(IDC_STATIC6)->ShowWindow(iSort);
	GetDlgItem(IDC_HITRATE2)->ShowWindow(iSort);
	GetDlgItem(IDC_SORT)->ShowWindow(iSort);
	GetDlgItem(IDC_CHECK2)->ShowWindow(iSort);
	
	// display average (total)
	GetDlgItem(IDC_DISPLAY)->ShowWindow(iAvg);
}

void CViewSpikeSort_Templates::OnBnClickedSort()
{
	SortSpikes();
}

void CViewSpikeSort_Templates::OnBnClickedDisplay()
{
	DisplayAvg(TRUE, &m_avgAllList); //, &m_ImListAll);
}


void CViewSpikeSort_Templates::OnEnChangeIfirstsortedclass()
{
	if (!mm_ifirstsortedclass.m_bEntryDone)
		return;

	switch (mm_ifirstsortedclass.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_ifirstsortedclass++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_ifirstsortedclass--;
		break;
	}
	
	mm_ifirstsortedclass.m_bEntryDone=FALSE;
	mm_ifirstsortedclass.m_nChar=0;
	mm_ifirstsortedclass.SetSel(0, -1); 	//select all text

	// change class of all templates
	//SetTemplateclassID(int item, LPCSTR pszType, int classID)
	m_templList.UpdateTemplateBaseClassID(m_ifirstsortedclass);
	UpdateData(FALSE);
}

void CViewSpikeSort_Templates::OnTcnSelchangeTab2(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	SelectSpikeList(icursel);
	*pResult = 0;
}

void CViewSpikeSort_Templates::OnNMClickTab2(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	SelectSpikeList(icursel);
	*pResult = 0;
}
