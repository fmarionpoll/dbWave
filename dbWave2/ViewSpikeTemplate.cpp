// SpikeTemplateView.cpp : implementation file
//

#include "StdAfx.h"
//#include <math.h>
//#include "Cscale.h"
//#include "MainFrm.h"
//#include "Copyasdl.h"
//#include "ChildFrm.h"
//#include "ProgDlg.h"
//#include "Lineview.h"
//#include "dbMainTable.h"
#include "scopescr.h"
#include "Editctrl.h"
#include "dbWave.h"
#include "resource.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "spikeshape.h"
#include "TemplateWnd.h"
#include "TemplateListWnd.h"
#include "EditSpikeClassDlg.h"
#include "Editspik.h"
#include "ViewSpikeTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CViewSpikeTemplates, CDaoRecordView)

CViewSpikeTemplates::CViewSpikeTemplates()
	: CDaoRecordView(CViewSpikeTemplates::IDD), m_pSpkDoc(nullptr), m_pSpkList(nullptr), m_lFirst(0), m_lLast(0),
	mdPM(nullptr), mdMO(nullptr), m_psC(nullptr), m_ktagleft(0), m_ktagright(0), m_scrollFilePos_infos()
	, m_bDisplaySingleClass(FALSE)
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
	m_bDisplaySingleClass = FALSE;
	m_binit = FALSE;
	m_ballTempl = FALSE;
	m_ballSort = FALSE;
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

CViewSpikeTemplates::~CViewSpikeTemplates()
{
	// save spkD list i	 changed
	if (m_pSpkDoc != nullptr)
		SaveCurrentSpkFile();	// save file if modified
}

BOOL CViewSpikeTemplates::PreCreateWindow(CREATESTRUCT& cs)
{
	return CDaoRecordView::PreCreateWindow(cs);
}

void CViewSpikeTemplates::OnDestroy()
{
	CDaoRecordView::OnDestroy();
	if (m_templList.GetNtemplates() != 0)
	{
		if (m_psC->ptpl == nullptr)
			m_psC->CreateTPL();
		*(CTemplateListWnd*)m_psC->ptpl = m_templList;
	}
}

#ifdef _DEBUG
void CViewSpikeTemplates::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CViewSpikeTemplates::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CViewSpikeTemplates::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}
#endif //_DEBUG

CDaoRecordset* CViewSpikeTemplates::OnGetRecordset()
{
	return GetDocument()->GetDB_Recordset();
}

void CViewSpikeTemplates::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		AfxGetMainWnd()->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		((CdbWaveApp*)AfxGetApp())->options_viewspikes.bincrflagonsave = ((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck();
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CViewSpikeTemplates::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_binit)
	{
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
}

BOOL CViewSpikeTemplates::OnMove(UINT nIDMoveCommand)
{
	SaveCurrentSpkFile();
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	auto p_document = GetDocument();
	if (p_document->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
		return false;
	}

	p_document->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CViewSpikeTemplates::DoDataExchange(CDataExchange* pDX)
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
	DDX_Check(pDX, IDC_DISPLAYSINGLECLASS, m_bDisplaySingleClass);
}

BEGIN_MESSAGE_MAP(CViewSpikeTemplates, CDaoRecordView)

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
	ON_EN_CHANGE(IDC_HITRATE2, OnEnChangeHitrateSort)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST2, OnKeydownTemplateList)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()

	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CViewSpikeTemplates::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_SORT, &CViewSpikeTemplates::OnBnClickedSort)
	ON_BN_CLICKED(IDC_DISPLAY, &CViewSpikeTemplates::OnBnClickedDisplay)
	ON_EN_CHANGE(IDC_IFIRSTSORTEDCLASS, &CViewSpikeTemplates::OnEnChangeIfirstsortedclass)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB2, &CViewSpikeTemplates::OnTcnSelchangeTab2)
	ON_NOTIFY(NM_CLICK, IDC_TAB2, &CViewSpikeTemplates::OnNMClickTab2)
	ON_BN_CLICKED(IDC_DISPLAYSINGLECLASS, &CViewSpikeTemplates::OnBnClickedDisplaysingleclass)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeTemplates message handlers

void CViewSpikeTemplates::OnSize(UINT nType, int cx, int cy)
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

void CViewSpikeTemplates::SaveCurrentSpkFile()
{
	// save previous file if anything has changed
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		m_pSpkDoc->OnSaveDocument(GetDocument()->GetDB_CurrentSpkFileName(FALSE));
		m_pSpkDoc->SetModifiedFlag(FALSE);

		GetDocument()->SetDB_nbspikes(m_pSpkList->GetTotalSpikes());
		GetDocument()->SetDB_nbspikeclasses(m_pSpkList->GetNbclasses());

		// change flag is button is checked
		if (((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck())
		{
			int flag = GetDocument()->GetDB_CurrentRecordFlag();
			flag++;
			GetDocument()->SetDB_CurrentRecordFlag(flag);
		}
	}
}

void CViewSpikeTemplates::OnInitialUpdate()
{
	m_stretch.AttachParent(this);		// attach formview pointer
	m_stretch.newProp(IDC_LIST1, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_LIST2, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_LIST3, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB2, XLEQ_XREQ, SZEQ_YBEQ);

	// force resizing
	m_binit = TRUE;

	VERIFY(m_spkForm.SubclassDlgItem(IDC_DISPLAYSPIKE, this));
	VERIFY(mm_spikenoclass.SubclassDlgItem(IDC_EDIT2, this));
	mm_spikenoclass.ShowScrollBar(SB_VERT);
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100, FALSE);
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

	const auto p_app = (CdbWaveApp*)AfxGetApp();	// load browse parameters
	mdPM = &(p_app->options_viewdata);					// viewdata options
	mdMO = &(p_app->options_viewdata_measure);					// measure options
	m_psC = &(p_app->spkC);					// get address of spike classif parms
	if (m_psC->ptpl != nullptr)
		m_templList = *((CTemplateListWnd*)m_psC->ptpl);

	const auto p_dbwave_doc = GetDocument();

	// set bincrflagonsave
	((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->SetCheck(p_app->options_viewspikes.bincrflagonsave);

	CDaoRecordView::OnInitialUpdate();

	// set ctrlTab values and extend its size
	CString cs = _T("Create Templates");
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

	m_pSpkDoc = (CSpikeDoc*)GetDocument();
	m_spkForm.SetPlotMode(PLOT_ONECLASS, 0);
	m_ktagleft = m_spkForm.AddVTtag(m_psC->kleft);
	m_ktagright = m_spkForm.AddVTtag(m_psC->kright);

	UpdateFileParameters();
	UpdateCtrlTab1(0);
}

void CViewSpikeTemplates::UpdateFileParameters()
{
	// init views
	m_pSpkDoc = GetDocument()->OpenCurrentSpikeFile();
	const auto icur = m_pSpkDoc->GetSpkList_CurrentIndex();
	m_pSpkDoc->SetSpkList_AsCurrent(icur);
	m_tabCtrl.DeleteAllItems();

	// load list of detection parameters
	auto j = 0;
	for (auto i = 0; i < m_pSpkDoc->GetSpkList_Size(); i++)
	{
		const auto p_spike_list = m_pSpkDoc->SetSpkList_AsCurrent(i);
		CString cs;
		if (p_spike_list->GetdetectWhat() != DETECT_SPIKES)
			continue;
		cs.Format(_T("#%i %s"), i, static_cast<LPCTSTR>(p_spike_list->GetComment()));
		m_tabCtrl.InsertItem(j, cs);
		j++;
	}
	SelectSpikeList(icur);
}

void CViewSpikeTemplates::SelectSpikeList(int icur)
{
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(icur);
	m_tabCtrl.SetCurSel(icur);

	if (!m_pSpkList->IsClassListValid())
	{
		m_pSpkList->UpdateClassList();
		m_pSpkDoc->SetModifiedFlag();
	}

	// change pointer to select new spike list & test if one spike is selected
	int spikeno = m_pSpkList->m_selspike;
	if (spikeno > m_pSpkList->GetTotalSpikes() - 1 || spikeno < 0)
		spikeno = -1;
	else
	{	// set source class to the class of the selected spike
		m_spikenoclass = m_pSpkList->GetSpikeClass(spikeno);
		m_psC->sourceclass = m_spikenoclass;
	}
	if (m_spikenoclass > 32768)
		m_spikenoclass = 0;

	// prepare display source spikes
	m_spkForm.SetSourceData(m_pSpkList, GetDocument());
	if (m_psC->kleft == 0 && m_psC->kright == 0)
	{
		m_psC->kleft = m_pSpkList->GetSpikePretrig();
		m_psC->kright = m_psC->kleft + m_pSpkList->GetSpikeRefractory();
	}
	if (!m_bDisplaySingleClass)
		m_spkForm.SetPlotMode(PLOT_BLACK, 0);
	else
		m_spkForm.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);

	m_lFirst = 0;
	m_lLast = m_pSpkDoc->GetAcqSize() - 1;
	m_scrollFilePos_infos.nMin = 0;
	m_scrollFilePos_infos.nMax = m_lLast;
	m_spkForm.SetTimeIntervals(m_lFirst, m_lLast);
	m_spkForm.Invalidate();

	SelectSpike(spikeno);
	UpdateLegends();

	DisplayAvg(FALSE, &m_avgList); 
	UpdateTemplates();
}

void CViewSpikeTemplates::UpdateTemplates()
{
	auto n_cmd_show = SW_HIDE;
	if (m_templList.GetNtemplates() > 0)
	{
		if (m_templList.GetImageList(LVSIL_NORMAL) != &m_templList.m_imageList)
		{
			CRect rect;
			m_spkForm.GetClientRect(&rect);
			m_templList.m_imageList.Create(rect.right, rect.bottom, ILC_COLOR8, 4, 1);
			m_templList.SetImageList(&m_templList.m_imageList, LVSIL_NORMAL);
		}
		SetDlgItemInt(IDC_NTEMPLATES, m_templList.GetNtemplates());
		int extent = m_spkForm.GetYWExtent();
		int zero = m_spkForm.GetYWOrg();
		m_templList.SetYWExtOrg(extent , zero);
		m_templList.UpdateTemplateLegends("t");
		m_templList.Invalidate();
		n_cmd_show = SW_SHOW;
	}
	GetDlgItem(IDC_NTEMPLS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_NTEMPLATES)->ShowWindow(n_cmd_show);
}

void CViewSpikeTemplates::UpdateLegends()
{
	if (m_lFirst < 0)
		m_lFirst = 0;
	if (m_lLast <= m_lFirst)
		m_lLast = m_lFirst + 120;
	if (m_lLast >= m_pSpkDoc->GetAcqSize())
		m_lLast = m_pSpkDoc->GetAcqSize() - 1;
	if (m_lFirst > m_lLast)
		m_lFirst = m_lLast - 120;

	// update text abcissa and horizontal scroll position
	m_timefirst = m_lFirst / m_pSpkDoc->GetAcqRate();
	m_timelast = (m_lLast + 1) / m_pSpkDoc->GetAcqRate();

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

void CViewSpikeTemplates::SelectSpike(short spikeno)
{
	m_spkForm.SelectSpikeShape(spikeno);
	m_spikeno = spikeno;
	m_pSpkList->m_selspike = spikeno;
}

LRESULT CViewSpikeTemplates::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	short threshold = LOWORD(lParam);	// value associated

	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		if (threshold > CURSOR_ZOOM)
			threshold = 0;
		SetViewMouseCursor(threshold);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(threshold, 0));
		break;

	case HINT_HITSPIKE:	
		SelectSpike(threshold);
		break;

	case HINT_CHANGEVERTTAG:
		if (threshold == m_ktagleft)
		{
			m_psC->kleft = m_spkForm.GetVTtagVal(m_ktagleft);
		}
		else if (threshold == m_ktagright)
		{
			m_psC->kright = m_spkForm.GetVTtagVal(m_ktagright);
		}
		m_templList.SetTemplateLength(0, m_psC->kleft, m_psC->kright);
		m_templList.Invalidate();
		break;

	case HINT_CHANGEHZLIMITS:		
	case HINT_CHANGEZOOM:
	case HINT_VIEWSIZECHANGED:     
		SetExtentZeroAllDisplay(m_spkForm.GetYWExtent(), m_spkForm.GetYWOrg());
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

void CViewSpikeTemplates::OnEnChangeclassno()
{
	if (mm_spikenoclass.m_bEntryDone)
	{
		const auto spikenoclass = m_spikenoclass;
		switch (mm_spikenoclass.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_spikenoclass++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_spikenoclass--;	break;
		default:;
		}

		mm_spikenoclass.m_bEntryDone = FALSE;	// clear flag
		mm_spikenoclass.m_nChar = 0;			// empty buffer
		mm_spikenoclass.SetSel(0, -1);		// select all text

		if (m_spikenoclass != spikenoclass)	// change display if necessary
		{
			m_spkForm.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);
			m_spkForm.Invalidate();
			UpdateLegends();
		}
	}
}

void CViewSpikeTemplates::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone)
	{
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

		const auto l_first = static_cast<long>(m_timefirst * m_pSpkDoc->GetAcqRate());
		if (l_first != m_lFirst)
		{
			m_lFirst = l_first;
			UpdateLegends();
		}
	}
}

void CViewSpikeTemplates::OnEnChangeTimelast()
{
	if (mm_timelast.m_bEntryDone)
	{
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

		const auto l_last = static_cast<long>(m_timelast * m_pSpkDoc->GetAcqRate());
		if (l_last != m_lLast)
		{
			m_lLast = l_last;
			UpdateLegends();
		}
	}
}

void CViewSpikeTemplates::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// get corresponding data
	const auto total_scroll = m_pSpkDoc->GetAcqSize();
	const long page_scroll = (m_lLast - m_lFirst);
	auto sb_scroll = page_scroll / 10;
	if (sb_scroll == 0)
		sb_scroll = 1;
	long l_first = m_lFirst;
	switch (nSBCode)
	{
	case SB_LEFT:		l_first = 0;	break;			// Scroll to far left.
	case SB_LINELEFT:	l_first -= sb_scroll;	break;	// Scroll left.
	case SB_LINERIGHT:	l_first += sb_scroll; break;	// Scroll right
	case SB_PAGELEFT:	l_first -= page_scroll; break;// Scroll one page left
	case SB_PAGERIGHT:	l_first += page_scroll; break;// Scroll one page right.
	case SB_RIGHT:		l_first = total_scroll - page_scroll + 1;
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		l_first = static_cast<int>(nPos);
		break;
	default:
		return;
	}

	if (l_first < 0)
		l_first = 0;

	long l_last = l_first + page_scroll;
	if (l_last >= total_scroll)
	{
		l_last = total_scroll - 1;
		l_first = l_last - page_scroll;
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

void CViewSpikeTemplates::UpdateScrollBar()
{
	if (m_lFirst == 0 && m_lLast >= m_pSpkDoc->GetAcqSize() - 1)
		GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_HIDE);
	else
	{
		GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_SHOW);
		m_scrollFilePos_infos.fMask = SIF_ALL;
		m_scrollFilePos_infos.nPos = m_lFirst;
		m_scrollFilePos_infos.nPage = m_lLast - m_lFirst;
		((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollInfo(&m_scrollFilePos_infos);
	}
}

void CViewSpikeTemplates::OnFormatAlldata()
{
	// dots: spk file length
	m_lFirst = 0;
	m_lLast = m_pSpkDoc->GetAcqSize() - 1;
	// spikes: center spikes horizontally and adjust hz size of display
	const short x_wo = 0;
	const short x_we = m_pSpkList->GetSpikeLength();
	m_spkForm.SetXWExtOrg(x_we, x_wo);
	UpdateLegends();
}

void CViewSpikeTemplates::OnFormatGainadjust()
{
	int maxval, minval;
	m_pSpkList->GetTotalMaxMin(TRUE, &maxval, &minval);
	const auto extent = maxval - minval;
	const auto zero = (maxval + minval) / 2;

	SetExtentZeroAllDisplay(extent, zero);
}

void CViewSpikeTemplates::OnFormatCentercurve()
{
	int maxval, minval;
	m_pSpkList->GetTotalMaxMin(TRUE, &maxval, &minval);
	const auto extent = m_spkForm.GetYWExtent();
	const auto zero = (maxval + minval) / 2;
	SetExtentZeroAllDisplay(extent, zero);
}

void CViewSpikeTemplates::SetExtentZeroAllDisplay(int extent, int zero) {
	m_spkForm.SetYWExtOrg(extent, zero);
	m_spkForm.Invalidate();
	m_templList.SetYWExtOrg(extent, zero);
	m_templList.Invalidate();
	m_avgList.SetYWExtOrg(extent, zero);
	m_avgList.Invalidate();
	m_avgAllList.SetYWExtOrg(extent, zero);
	m_avgAllList.Invalidate();
}

void CViewSpikeTemplates::OnEnChangeHitrate()
{
	if (mm_hitrate.m_bEntryDone)
	{
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
		default:;
		}

		mm_hitrate.m_bEntryDone = FALSE;
		mm_hitrate.m_nChar = 0;
		mm_hitrate.SetSel(0, -1); 	//select all text

		if (m_psC->hitrate != m_hitrate)
		{
			m_psC->hitrate = m_hitrate;
		}
		UpdateData(FALSE);
	}
}

void CViewSpikeTemplates::OnEnChangeHitrateSort()
{
	if (mm_hitratesort.m_bEntryDone)
	{
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
		default:;
		}

		mm_hitratesort.m_bEntryDone = FALSE;
		mm_hitratesort.m_nChar = 0;
		mm_hitratesort.SetSel(0, -1); 	//select all text

		if (m_psC->hitratesort != m_hitratesort)
		{
			m_psC->hitratesort = m_hitratesort;
		}
		UpdateData(FALSE);
	}
}

void CViewSpikeTemplates::OnEnChangeTolerance()
{
	if (mm_ktolerance.m_bEntryDone)
	{
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
		default:;
		}

		if (m_ktolerance < 0)
			m_ktolerance = -m_ktolerance;
		mm_ktolerance.m_bEntryDone = FALSE;
		mm_ktolerance.m_nChar = 0;
		mm_ktolerance.SetSel(0, -1); 	//select all text

		if (m_psC->ktolerance != m_ktolerance)
		{
			m_psC->ktolerance = m_ktolerance;
		}
		UpdateData(FALSE);
	}
}

void CViewSpikeTemplates::DisplayAvg(BOOL ballfiles, CTemplateListWnd* pTPList) //, CImageList* pImList)
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
	pTPList->SetTemplateLength(m_pSpkList->GetSpikeLength(), 0, m_pSpkList->GetSpikeLength() - 1);
	pTPList->SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);

	int zero = m_spkForm.GetYWOrg();
	int extent = m_spkForm.GetYWExtent();
	if (zero == 0 && extent == 0)
	{
		int maxval, minval;
		m_pSpkList->GetTotalMaxMin(TRUE, &maxval, &minval);
		extent = maxval - minval;
		zero = (maxval + minval) / 2;
		m_spkForm.SetYWExtOrg(extent, zero);
	}
	pTPList->SetYWExtOrg(extent, zero);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition(); // index current file
	auto firstfile = currentfile;		// index first file in the series
	auto lastfile = currentfile;			// index last file in the series
	// make sure we have the correct spike list here
	const auto currentlist = m_tabCtrl.GetCurSel();
	m_pSpkDoc->SetSpkList_AsCurrent(currentlist);

	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	if (ballfiles)
	{
		firstfile = 0;						// index first file
		lastfile = p_dbwave_doc->GetDB_NRecords() - 1;	// index last file
	}
	// loop over files
	for (auto ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// load file
		p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
		auto pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
		if (pSpkDoc == nullptr)
			continue;
		CString cs;
		cs.Format(_T("%i/%i - "), ifile, lastfile);
		cs += p_dbwave_doc->GetDB_CurrentSpkFileName(FALSE);
		p_dbwave_doc->SetTitle(cs);
		pSpkDoc->SetModifiedFlag(FALSE);

		auto pSpkList = pSpkDoc->SetSpkList_AsCurrent(currentlist); // load pointer to spike list
		if (!pSpkList->IsClassListValid())		// if class list not valid:
		{
			pSpkList->UpdateClassList();		// rebuild list of classes
			pSpkDoc->SetModifiedFlag();			// and set modified flag
		}
		const auto nspikes = pSpkList->GetTotalSpikes();

		// add spikes to templates - create templates on the fly
		int j_templ;
		for (auto i = 0; i < nspikes; i++)
		{
			const auto cla = pSpkList->GetSpikeClass(i);
			auto b_found = FALSE;
			for (j_templ = 0; j_templ < pTPList->GetTemplateDataSize(); j_templ++)
			{
				if (cla == pTPList->GetTemplateclassID(j_templ))
				{
					b_found = TRUE;
					break;
				}
				if (cla < pTPList->GetTemplateclassID(j_templ))
					break;
			}
			// add template if not found - insert it at the proper place
			if (!b_found) // add item if not found
			{
				if (j_templ < 0)
					j_templ = 0;
				j_templ = pTPList->InsertTemplateData(j_templ, cla);
			}

			// get data and add spike
			const auto p_spik = pSpkList->GetpSpikeData(i);
			pTPList->tAdd(j_templ, p_spik);	// add spike to template j
			pTPList->tAdd(p_spik);		// add spike to template zero
		}
	}

	// end of loop, select current file again if necessary
	if (ballfiles)
	{
		p_dbwave_doc->SetDB_CurrentRecordPosition(currentfile);
		m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update average
	pTPList->TransferTemplateData();
	pTPList->tGlobalstats();
	pTPList->UpdateTemplateLegends("c");
}

void CViewSpikeTemplates::OnBuildTemplates()
{
	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition(); // index current file
	auto firstfile = currentfile;		// index first file in the series
	auto lastfile = firstfile;			// index last file in the series
	// get current selected list
	const auto currentlist = m_tabCtrl.GetCurSel();

	// change indexes if ALL files selected
	if (m_ballfiles)
	{
		firstfile = 0;						// index first file
		lastfile = p_dbwave_doc->GetDB_NRecords() - 1;	// index last file
	}

	// add as many forms as we have classes
	m_templList.DeleteAllItems();	// reinit all templates to zero
	m_templList.SetTemplateLength(m_pSpkList->GetSpikeLength(), m_psC->kleft, m_psC->kright);
	m_templList.SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);

	// compute global std
	// loop over all selected files (or only one file currently selected)
	int nspikes;
	int ifile;
	CString cscomment;

	for (ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_ballfiles)
		{
			p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
			m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
		}

		const auto spike_list = m_pSpkDoc->SetSpkList_AsCurrent(currentlist);
		nspikes = spike_list->GetTotalSpikes();
		for (auto i = 0; i < nspikes; i++)
			m_templList.tAdd(m_pSpkList->GetpSpikeData(i));
	}
	m_templList.tGlobalstats();

	// now scan all spikes to build templates
	auto ntempl = 0;
	double distmin;
	int offsetmin;
	int tplmin;
	CString csfilecomment = _T("Second pass - analyze file: ");

	for (ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_ballfiles)
		{
			p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
			p_dbwave_doc->OpenCurrentSpikeFile();
			m_pSpkDoc = p_dbwave_doc->m_pSpk;
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->GetDB_CurrentSpkFileName(FALSE);
			p_dbwave_doc->SetTitle(cs);
		}

		auto spike_list = m_pSpkDoc->SetSpkList_AsCurrent(currentlist);
		nspikes = spike_list->GetTotalSpikes();

		// create template CListCtrl
		for (auto i = 0; i < nspikes; i++)
		{
			// filter out undesirable spikes
			if (m_bDisplaySingleClass)
			{
				if (m_pSpkList->GetSpikeClass(i) != m_spikenoclass)
					continue;
			}
			const auto iitime = m_pSpkList->GetSpikeTime(i);
			if (iitime < m_lFirst || iitime > m_lLast)
				continue;

			// get pointer to spike data and search if any template is suitable
			auto* p_spik = m_pSpkList->GetpSpikeData(i);
			auto b_within = FALSE;
			int itpl;
			for (itpl = 0; itpl < ntempl; itpl++)
			{
				// exit loop if spike is within template
				b_within = m_templList.tWithin(itpl, p_spik);
				if (b_within)
					break;
				// OR exit loop if spike dist is less distant
				m_templList.tMinDist(itpl, p_spik, &offsetmin, &distmin);
				b_within = (distmin <= m_templList.m_globaldist);
				if (b_within)
					break;
			}

			// if a template is suitable, find the most likely
			if (b_within)
			{
				tplmin = itpl;
				distmin = m_templList.m_globaldist;
				double x;
				int offset;
				for (auto itpl2 = 0; itpl2 < ntempl; itpl2++)
				{
					m_templList.tMinDist(itpl2, p_spik, &offset, &x);
					if (x < distmin)
					{
						offsetmin = offset;
						distmin = x;
						tplmin = itpl2;
					}
				}
			}
			// else (no suitable template), create a new one
			else
			{
				m_templList.InsertTemplate(ntempl, ntempl + m_ifirstsortedclass);
				tplmin = ntempl;
				ntempl++;
			}

			// add spike to the corresp template
			m_templList.tAdd(tplmin, p_spik);	// add spike to template j
		}
	}

	// end of loop, select current file again if necessary
	if (m_ballfiles)
	{
		p_dbwave_doc->SetDB_CurrentRecordPosition(currentfile);
		m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	m_templList.SortTemplatesByNumberofSpikes(TRUE, TRUE, m_ifirstsortedclass);
	UpdateTemplates();
}

void CViewSpikeTemplates::SortSpikes()
{
	// set tolerance to sort tolerance
	m_templList.SetHitRate_Tolerance(&m_hitratesort, &m_ktolerance);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition(); // index current file
	auto firstfile = currentfile;		// index first file in the series
	auto lastfile = firstfile;			// index last file in the series
	const auto currentlist = m_tabCtrl.GetCurSel();

	// change indexes if ALL files selected
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	if (m_ballfiles)
	{
		firstfile = 0;						// index first file
		lastfile = p_dbwave_doc->GetDB_NRecords() - 1;	// index last file
	}

	// loop CFrameWnd
	const auto ntempl = m_templList.GetNtemplates();
	for (auto ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_ballfiles)
		{
			p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
			m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->GetDB_CurrentSpkFileName(FALSE);
			p_dbwave_doc->SetTitle(cs);
			m_pSpkDoc->SetModifiedFlag(FALSE);

			m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(currentlist); // load pointer to spike list
			if (!m_pSpkList->IsClassListValid())		// if class list not valid:
			{
				m_pSpkList->UpdateClassList();			// rebuild list of classes
				m_pSpkDoc->SetModifiedFlag();			// and set modified flag
			}
		}

		// spike loop
		const auto nspikes = m_pSpkList->GetTotalSpikes();	// loop over all spikes
		for (auto ispike = 0; ispike < nspikes; ispike++)
		{
			// filter out undesirable spikes - i.e. not relevant to the sort
			if (m_bDisplaySingleClass)
			{
				// skip spikes that do not belong to selected class
				if (m_pSpkList->GetSpikeClass(ispike) != m_spikenoclass)
					continue;
			}

			// skip spikes that do not fit into time interval selected
			const auto iitime = m_pSpkList->GetSpikeTime(ispike);
			if (iitime < m_lFirst || iitime > m_lLast)
				continue;

			// get pointer to spike data and search if any template is suitable
			const auto p_spik = m_pSpkList->GetpSpikeData(ispike);
			auto b_within = FALSE;
			double distmin;
			int offsetmin;

			// search first template that meet criteria
			int tplmin;
			for (tplmin = 0; tplmin < ntempl; tplmin++)
			{
				// exit loop if spike is within template
				b_within = m_templList.tWithin(tplmin, p_spik);
				m_templList.tMinDist(tplmin, p_spik, &offsetmin, &distmin);
				if (b_within)
					break;

				// OR exit loop if spike dist is less distant
				b_within = (distmin <= m_templList.m_globaldist);
				if (b_within)
					break;
			}

			// if a template is suitable, find the most likely
			if (b_within)
			{
				for (auto itpl = tplmin + 1; itpl < ntempl; itpl++)
				{
					double x;
					int offset;
					m_templList.tMinDist(itpl, p_spik, &offset, &x);
					if (x < distmin)
					{
						offsetmin = offset;
						distmin = x;
						tplmin = itpl;
					}
				}

				// change spike class ID
				const auto class_id = (m_templList.GetTemplateWnd(tplmin))->m_classID;
				if (m_pSpkList->GetSpikeClass(ispike) != class_id)
				{
					m_pSpkList->SetSpikeClass(ispike, class_id);
					m_pSpkDoc->SetModifiedFlag(TRUE);
				}
			}
		}
		if (m_pSpkDoc->IsModified())
		{
			m_pSpkDoc->OnSaveDocument(p_dbwave_doc->GetDB_CurrentSpkFileName(FALSE));
			m_pSpkDoc->SetModifiedFlag(FALSE);

			GetDocument()->SetDB_nbspikes(m_pSpkList->GetTotalSpikes());
			GetDocument()->SetDB_nbspikeclasses(m_pSpkList->GetNbclasses());
		}
	}

	// end of loop, select current file again if necessary
	if (m_ballfiles)
	{
		p_dbwave_doc->SetDB_CurrentRecordPosition(currentfile);
		m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update display: average and spk form
	DisplayAvg(FALSE, &m_avgList);
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	m_spkForm.SetSourceData(m_pSpkList, GetDocument());
	m_spkForm.Invalidate();
}

void CViewSpikeTemplates::OnKeydownTemplateList(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto* p_lv_key_dow = reinterpret_cast<LV_KEYDOWN*>(pNMHDR);

	// delete selected template
	if (p_lv_key_dow->wVKey == VK_DELETE && m_templList.GetSelectedCount() > 0)
	{
		auto flag = FALSE;
		const auto isup = m_templList.GetItemCount();
		auto cla = m_ifirstsortedclass;
		for (auto i = 0; i < isup; i++)
		{
			const auto state = m_templList.GetItemState(i, LVIS_SELECTED);
			if (state > 0)
			{
				cla = m_templList.GetTemplateclassID(i);
				m_templList.DeleteItem(i);
				flag = TRUE;
				continue;
			}
			if (flag)
			{
				m_templList.SetTemplateclassID(i - 1, _T("t"), cla);
				cla++;
			}
		}
	}
	SetDlgItemInt(IDC_NTEMPLATES, m_templList.GetItemCount());
	m_templList.Invalidate();
	*pResult = 0;
}

void CViewSpikeTemplates::OnCheck1()
{
	UpdateData(TRUE);
}

void CViewSpikeTemplates::EditSpikeClass(int controlID, int controlItem)
{
	// find which item has been selected
	CTemplateListWnd* pList = nullptr;
	auto b_spikes = TRUE;
	auto b_all_files = m_ballfiles;
	if (m_avgList.GetDlgCtrlID() == controlID)
		pList = &m_avgList;
	else if (m_templList.GetDlgCtrlID() == controlID)
	{
		pList = &m_templList;
		b_spikes = FALSE;
	}
	else if (m_avgAllList.GetDlgCtrlID() == controlID)
	{
		pList = &m_avgAllList;
		b_all_files = TRUE;
	}
	if (pList == nullptr)
		return;

	// find which icon has been selected and get the key
	const auto oldclass = pList->GetTemplateclassID(controlItem);

	// launch edit dlg
	CEditSpikeClassDlg dlg;
	dlg.m_iClass = oldclass;
	if (IDOK == dlg.DoModal() && oldclass != dlg.m_iClass)
	{
		// templates
		if (!b_spikes)
			pList->SetTemplateclassID(controlItem, _T("t"), dlg.m_iClass);
		// spikes
		else
		{
			pList->SetTemplateclassID(controlItem, _T("c"), dlg.m_iClass);

			// set file indexes - assume only one file selected
			auto p_dbwave_doc = GetDocument();
			const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition(); // index current file
			auto firstfile = currentfile;		// index first file in the series
			auto lastfile = firstfile;			// index last file in the series
			const auto currentlist = m_tabCtrl.GetCurSel();

			// change indexes if ALL files selected
			CString cscomment;
			CString csfilecomment = _T("Analyze file: ");
			if (b_all_files)
			{
				firstfile = 0;						// index first file
				lastfile = p_dbwave_doc->GetDB_NRecords() - 1;	// index last file
			}

			// loop CFrameWnd
			for (auto ifile = firstfile; ifile <= lastfile; ifile++)
			{
				// store nb of spikes within array
				if (b_all_files)
				{
					p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
					m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
					CString cs;
					cs.Format(_T("%i/%i - "), ifile, lastfile);
					cs += p_dbwave_doc->GetDB_CurrentSpkFileName(FALSE);
					p_dbwave_doc->SetTitle(cs);
					m_pSpkDoc->SetModifiedFlag(FALSE);
					m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(currentlist);
				}

				// TODO: this should not work - changing SpikeClassID does not change the spike class because UpdateClassList reset classes array to zero
				m_pSpkList->UpdateClassList();			// rebuild list of classes
				m_pSpkList->ChangeSpikeClassID(oldclass, dlg.m_iClass);
				m_pSpkList->UpdateClassList();			// rebuild list of classes
				m_pSpkDoc->SetModifiedFlag(TRUE);

				if (m_pSpkDoc->IsModified())
				{
					m_pSpkDoc->OnSaveDocument(p_dbwave_doc->GetDB_CurrentSpkFileName(FALSE));
					m_pSpkDoc->SetModifiedFlag(FALSE);

					GetDocument()->SetDB_nbspikes(m_pSpkList->GetTotalSpikes());
					GetDocument()->SetDB_nbspikeclasses(m_pSpkList->GetNbclasses());
				}
			}

			// end of loop, select current file again if necessary
			if (b_all_files)
			{
				p_dbwave_doc->SetDB_CurrentRecordPosition(currentfile);
				m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
				p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
			}
		}
	}
}

void CViewSpikeTemplates::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	// switch display
	const auto iselect = m_tab1Ctrl.GetCurSel();
	UpdateCtrlTab1(iselect);
}

void CViewSpikeTemplates::UpdateCtrlTab1(int iselect)
{
	WORD i_templ = SW_SHOW;
	WORD i_avg = SW_HIDE;
	WORD i_sort = SW_HIDE;
	switch (iselect)
	{
	case 0:
		m_avgAllList.ShowWindow(SW_HIDE);
		m_templList.ShowWindow(SW_SHOW);
		break;
	case 1:
		i_templ = SW_HIDE;
		i_sort = SW_SHOW;
		i_avg = SW_HIDE;
		m_avgAllList.ShowWindow(SW_HIDE);
		m_templList.ShowWindow(SW_SHOW);
		break;
	case 2:
		i_templ = SW_HIDE;
		i_sort = SW_HIDE;
		i_avg = SW_SHOW;
		m_avgAllList.ShowWindow(SW_SHOW);
		m_templList.ShowWindow(SW_HIDE);
		break;
	default:
		break;
	}

	// build templates
	GetDlgItem(IDC_STATIC3)->ShowWindow(i_templ);
	GetDlgItem(IDC_HITRATE)->ShowWindow(i_templ);
	GetDlgItem(IDC_STATIC4)->ShowWindow(i_templ);
	GetDlgItem(IDC_TOLERANCE)->ShowWindow(i_templ);
	GetDlgItem(IDC_BUILD)->ShowWindow(i_templ);
	GetDlgItem(IDC_LOAD_SAVE)->ShowWindow(i_templ);
	GetDlgItem(IDC_CHECK1)->ShowWindow(i_templ);
	GetDlgItem(IDC_NTEMPLS)->ShowWindow(i_templ);
	GetDlgItem(IDC_NTEMPLATES)->ShowWindow(i_templ);
	GetDlgItem(IDC_TFIRSTSORTEDCLASS)->ShowWindow(i_templ);
	GetDlgItem(IDC_IFIRSTSORTEDCLASS)->ShowWindow(i_templ);

	// sort spikes using templates
	GetDlgItem(IDC_STATIC6)->ShowWindow(i_sort);
	GetDlgItem(IDC_HITRATE2)->ShowWindow(i_sort);
	GetDlgItem(IDC_SORT)->ShowWindow(i_sort);
	GetDlgItem(IDC_CHECK2)->ShowWindow(i_sort);

	// display average (total)
	GetDlgItem(IDC_DISPLAY)->ShowWindow(i_avg);
}

void CViewSpikeTemplates::OnBnClickedSort()
{
	SortSpikes();
}

void CViewSpikeTemplates::OnBnClickedDisplay()
{
	DisplayAvg(TRUE, &m_avgAllList); //, &m_ImListAll);
}

void CViewSpikeTemplates::OnEnChangeIfirstsortedclass()
{
	if (mm_ifirstsortedclass.m_bEntryDone)
	{
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
		default:;
		}

		mm_ifirstsortedclass.m_bEntryDone = FALSE;
		mm_ifirstsortedclass.m_nChar = 0;
		mm_ifirstsortedclass.SetSel(0, -1); 	//select all text

		// change class of all templates
		//SetTemplateclassID(int item, LPCSTR pszType, int classID)
		m_templList.UpdateTemplateBaseClassID(m_ifirstsortedclass);
		UpdateData(FALSE);
	}
}

void CViewSpikeTemplates::OnTcnSelchangeTab2(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	SelectSpikeList(icursel);
	*pResult = 0;
}

void CViewSpikeTemplates::OnNMClickTab2(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	SelectSpikeList(icursel);
	*pResult = 0;
}

void CViewSpikeTemplates::OnBnClickedDisplaysingleclass()
{
	UpdateData(TRUE);
	if (m_bDisplaySingleClass) {
		GetDlgItem(IDC_EDIT2)->ShowWindow(SW_SHOW);
		m_spkForm.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);
	}
	else {
		GetDlgItem(IDC_EDIT2)->ShowWindow(SW_HIDE);
		m_spkForm.SetPlotMode(PLOT_BLACK, m_spikenoclass);
	}
	m_spkForm.Invalidate();
}
