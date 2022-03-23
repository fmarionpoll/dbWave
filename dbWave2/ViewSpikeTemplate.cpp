#include "StdAfx.h"
#include "ChartWnd.h"
#include "Editctrl.h"
#include "dbWave.h"
#include "resource.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "ChartSpikeShape.h"
#include "TemplateWnd.h"
#include "TemplateListWnd.h"
#include "DlgEditSpikeClass.h"
#include "ViewSpikeTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CViewSpikeTemplates, CdbTableView)

CViewSpikeTemplates::CViewSpikeTemplates()
	: CdbTableView(IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

CViewSpikeTemplates::~CViewSpikeTemplates()
{
	// save spkD list i	 changed
	if (m_pSpkDoc != nullptr)
		saveCurrentSpkFile(); // save file if modified
}

void CViewSpikeTemplates::DoDataExchange(CDataExchange* pDX)
{
	CdbTableView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_T1, m_t1);
	DDX_Text(pDX, IDC_T2, m_t2);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_HITRATE, m_hitrate);
	DDX_Text(pDX, IDC_TOLERANCE, m_ktolerance);
	DDX_Text(pDX, IDC_EDIT2, m_spikenoclass);
	DDX_Text(pDX, IDC_HITRATE2, m_hitratesort);
	DDX_Text(pDX, IDC_IFIRSTSORTEDCLASS, m_ifirstsortedclass);
	DDX_Check(pDX, IDC_CHECK1, m_bAllFiles);
	DDX_Control(pDX, IDC_TAB1, m_tab1Ctrl);
	DDX_Control(pDX, IDC_TAB2, m_tabCtrl);
	DDX_Check(pDX, IDC_DISPLAYSINGLECLASS, m_bDisplaySingleClass);
}

BEGIN_MESSAGE_MAP(CViewSpikeTemplates, CdbTableView)

	ON_MESSAGE(WM_MYMESSAGE, &CViewSpikeTemplates::OnMyMessage)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT2, &CViewSpikeTemplates::OnEnChangeclassno)
	ON_EN_CHANGE(IDC_TIMEFIRST, &CViewSpikeTemplates::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &CViewSpikeTemplates::OnEnChangeTimelast)
	ON_COMMAND(ID_FORMAT_ALLDATA, &CViewSpikeTemplates::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &CViewSpikeTemplates::OnFormatGainadjust)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &CViewSpikeTemplates::OnFormatCentercurve)
	ON_BN_CLICKED(IDC_BUILD, &CViewSpikeTemplates::OnBuildTemplates)
	ON_EN_CHANGE(IDC_HITRATE, &CViewSpikeTemplates::OnEnChangeHitrate)
	ON_EN_CHANGE(IDC_TOLERANCE, &CViewSpikeTemplates::OnEnChangeTolerance)
	ON_EN_CHANGE(IDC_HITRATE2, &CViewSpikeTemplates::OnEnChangeHitrateSort)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST2, &CViewSpikeTemplates::OnKeydownTemplateList)
	ON_BN_CLICKED(IDC_CHECK1, &CViewSpikeTemplates::OnCheck1)
	ON_BN_CLICKED(IDC_SORT, &CViewSpikeTemplates::OnBnClickedSort)
	ON_BN_CLICKED(IDC_DISPLAY, &CViewSpikeTemplates::OnBnClickedDisplay)
	ON_EN_CHANGE(IDC_IFIRSTSORTEDCLASS, &CViewSpikeTemplates::OnEnChangeIfirstsortedclass)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB2, &CViewSpikeTemplates::OnTcnSelchangeTab2)
	ON_NOTIFY(NM_CLICK, IDC_TAB2, &CViewSpikeTemplates::OnNMClickTab2)
	ON_BN_CLICKED(IDC_DISPLAYSINGLECLASS, &CViewSpikeTemplates::OnBnClickedDisplaysingleclass)
	ON_EN_CHANGE(IDC_T1, &CViewSpikeTemplates::OnEnChangeT1)
	ON_EN_CHANGE(IDC_T2, &CViewSpikeTemplates::OnEnChangeT2)
END_MESSAGE_MAP()

void CViewSpikeTemplates::OnDestroy()
{
	if (m_templList.GetNtemplates() != 0)
	{
		if (m_psC->ptpl == nullptr)
			m_psC->CreateTPL();
		*static_cast<CTemplateListWnd*>(m_psC->ptpl) = m_templList;
	}
	CdbTableView::OnDestroy();
}

BOOL CViewSpikeTemplates::OnMove(UINT nIDMoveCommand)
{
	saveCurrentSpkFile();
	return CdbTableView::OnMove(nIDMoveCommand);
}

void CViewSpikeTemplates::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_binit)
	{
		switch (LOWORD(lHint))
		{
		case HINT_DOCHASCHANGED: // file has changed?
		case HINT_DOCMOVERECORD:
			updateFileParameters();
			break;
		case HINT_CLOSEFILEMODIFIED: // close modified file: save
			saveCurrentSpkFile();
			break;
		case HINT_REPLACEVIEW:
		default:
			break;
		}
	}
}

void CViewSpikeTemplates::defineSubClassedItems()
{
	VERIFY(m_ChartSpkWnd_Shape.SubclassDlgItem(IDC_DISPLAYSPIKE, this));
	VERIFY(mm_spikenoclass.SubclassDlgItem(IDC_EDIT2, this));
	mm_spikenoclass.ShowScrollBar(SB_VERT);
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	static_cast<CScrollBar*>(GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100, FALSE);
	VERIFY(m_templList.SubclassDlgItem(IDC_LIST2, this));
	VERIFY(m_avgList.SubclassDlgItem(IDC_LIST1, this));
	VERIFY(m_avgAllList.SubclassDlgItem(IDC_LIST3, this));

	VERIFY(mm_t1.SubclassDlgItem(IDC_T1, this));
	VERIFY(mm_t2.SubclassDlgItem(IDC_T2, this));

	VERIFY(mm_hitrate.SubclassDlgItem(IDC_HITRATE, this));
	mm_hitrate.ShowScrollBar(SB_VERT);
	VERIFY(mm_hitratesort.SubclassDlgItem(IDC_HITRATE2, this));
	mm_hitratesort.ShowScrollBar(SB_VERT);
	VERIFY(mm_ktolerance.SubclassDlgItem(IDC_TOLERANCE, this));
	mm_ktolerance.ShowScrollBar(SB_VERT);
	VERIFY(mm_ifirstsortedclass.SubclassDlgItem(IDC_IFIRSTSORTEDCLASS, this));
	mm_ifirstsortedclass.ShowScrollBar(SB_VERT);
}

void CViewSpikeTemplates::defineStretchParameters()
{
	m_stretch.AttachParent(this); // attach formview pointer
	//m_stretch.newProp(IDC_LIST1, XLEQ_XREQ, SZEQ_YTEQ);
	//m_stretch.newProp(IDC_LIST2, XLEQ_XREQ, YTEQ_YBEQ);
	//m_stretch.newProp(IDC_LIST3, XLEQ_XREQ, YTEQ_YBEQ);
	//m_stretch.newProp(IDC_TAB2, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_LIST1, SZEQ_XLEQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_LIST2, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_LIST3, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB2, XLEQ_XREQ, SZEQ_YBEQ);
}

void CViewSpikeTemplates::OnInitialUpdate()
{
	CdbTableView::OnInitialUpdate();
	defineSubClassedItems();
	defineStretchParameters();
	m_binit = TRUE;
	m_autoIncrement = true;
	m_autoDetect = true;

	// load global parameters
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	mdPM = &(p_app->options_viewdata);
	mdMO = &(p_app->options_viewdata_measure);
	m_psC = &(p_app->spkC);
	if (m_psC->ptpl != nullptr)
		m_templList = *static_cast<CTemplateListWnd*>(m_psC->ptpl);

	// set ctrlTab values and extend its size
	CString cs = _T("Create");
	m_tab1Ctrl.InsertItem(0, cs);
	cs = _T("Sort");
	m_tab1Ctrl.InsertItem(1, cs);
	cs = _T("Display");
	m_tab1Ctrl.InsertItem(2, cs);
	CRect rect;
	m_tab1Ctrl.GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	rect.bottom += 200;
	m_tab1Ctrl.MoveWindow(&rect, TRUE);

	m_hitrate = m_psC->hitrate;
	m_hitratesort = m_psC->hitratesort;
	m_ktolerance = m_psC->ktolerance;

	m_ChartSpkWnd_Shape.SetPlotMode(PLOT_ONECLASS, 0);
	m_spkformtagleft = m_ChartSpkWnd_Shape.m_VTtags.AddTag(m_psC->kleft, 0);
	m_spkformtagright = m_ChartSpkWnd_Shape.m_VTtags.AddTag(m_psC->kright, 0);

	updateFileParameters();
	updateCtrlTab1(0);
}

void CViewSpikeTemplates::updateFileParameters()
{
	const BOOL bfirstupdate = (m_pSpkDoc == nullptr);
	updateSpikeFile();
	int icur = m_pSpkDoc->GetSpkList_CurrentIndex();
	selectSpikeList(icur);
}

void CViewSpikeTemplates::updateSpikeFile()
{
	m_pSpkDoc = GetDocument()->OpenCurrentSpikeFile();

	if (nullptr != m_pSpkDoc)
	{
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->GetDB_CurrentSpkFileName(), FALSE);
		int icur = GetDocument()->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(icur);

		// update Tab at the bottom
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);
		m_tabCtrl.SetCurSel(icur);
	}
}

void CViewSpikeTemplates::selectSpikeList(int icur)
{
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(icur);
	m_tabCtrl.SetCurSel(icur);

	if (!m_pSpkList->IsClassListValid())
	{
		m_pSpkList->UpdateClassList();
		m_pSpkDoc->SetModifiedFlag();
	}

	// change pointer to select new spike list & test if one spike is selected
	int spikeno = m_pSpkList->m_selected_spike;
	if (spikeno > m_pSpkList->GetTotalSpikes() - 1 || spikeno < 0)
		spikeno = -1;
	else
	{
		// set source class to the class of the selected spike
		m_spikenoclass = m_pSpkList->GetSpikeClass(spikeno);
		m_psC->sourceclass = m_spikenoclass;
	}

	ASSERT(m_spikenoclass < 32768);
	if (m_spikenoclass > 32768)
		m_spikenoclass = 0;

	// prepare display source spikes
	m_ChartSpkWnd_Shape.SetSourceData(m_pSpkList, GetDocument());
	if (m_psC->kleft == 0 && m_psC->kright == 0)
	{
		m_psC->kleft = m_pSpkList->GetSpikePretrig();
		m_psC->kright = m_psC->kleft + m_pSpkList->GetSpikeRefractory();
	}
	m_t1 = convertSpikeIndexToTime(m_psC->kleft);
	m_t2 = convertSpikeIndexToTime(m_psC->kright);

	if (!m_bDisplaySingleClass)
		m_ChartSpkWnd_Shape.SetPlotMode(PLOT_BLACK, 0);
	else
		m_ChartSpkWnd_Shape.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);

	m_lFirst = 0;
	m_lLast = m_pSpkDoc->GetAcqSize() - 1;
	m_scrollFilePos_infos.nMin = 0;
	m_scrollFilePos_infos.nMax = m_lLast;
	m_ChartSpkWnd_Shape.SetTimeIntervals(m_lFirst, m_lLast);
	m_ChartSpkWnd_Shape.Invalidate();

	selectSpike(spikeno);
	updateLegends();

	displayAvg(FALSE, &m_avgList);
	updateTemplates();
}

void CViewSpikeTemplates::updateTemplates()
{
	auto n_cmd_show = SW_HIDE;
	if (m_templList.GetNtemplates() > 0)
	{
		if (m_templList.GetImageList(LVSIL_NORMAL) != &m_templList.m_imageList)
		{
			CRect rect;
			m_ChartSpkWnd_Shape.GetClientRect(&rect);
			m_templList.m_imageList.Create(rect.right, rect.bottom, ILC_COLOR8, 4, 1);
			m_templList.SetImageList(&m_templList.m_imageList, LVSIL_NORMAL);
		}
		SetDlgItemInt(IDC_NTEMPLATES, m_templList.GetNtemplates());
		int extent = m_ChartSpkWnd_Shape.GetYWExtent();
		int zero = m_ChartSpkWnd_Shape.GetYWOrg();
		m_templList.SetYWExtOrg(extent, zero);
		m_templList.UpdateTemplateLegends("t");
		m_templList.Invalidate();
		n_cmd_show = SW_SHOW;
	}
	GetDlgItem(IDC_NTEMPLS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_NTEMPLATES)->ShowWindow(n_cmd_show);
}

void CViewSpikeTemplates::updateLegends()
{
	if (m_lFirst < 0)
		m_lFirst = 0;
	if (m_lLast <= m_lFirst)
		m_lLast = m_lFirst + 120;
	if (m_lLast >= m_pSpkDoc->GetAcqSize())
		m_lLast = m_pSpkDoc->GetAcqSize() - 1;
	if (m_lFirst > m_lLast)
		m_lFirst = m_lLast - 120;

	m_timefirst = m_lFirst / m_pSpkDoc->GetAcqRate();
	m_timelast = (m_lLast + 1) / m_pSpkDoc->GetAcqRate();

	m_ChartSpkWnd_Shape.SetTimeIntervals(m_lFirst, m_lLast);
	m_ChartSpkWnd_Shape.Invalidate();

	UpdateData(FALSE);
	updateScrollBar();
}

void CViewSpikeTemplates::selectSpike(short spikeno)
{
	m_ChartSpkWnd_Shape.SelectSpikeShape(spikeno);
	m_spikeno = spikeno;
	m_pSpkList->m_selected_spike = spikeno;
}

LRESULT CViewSpikeTemplates::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	short shortValue = LOWORD(lParam);
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		if (shortValue > CURSOR_ZOOM)
			shortValue = 0;
		SetViewMouseCursor(shortValue);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(shortValue, 0));
		break;

	case HINT_HITSPIKE:
		selectSpike(shortValue);
		break;

	case HINT_CHANGEVERTTAG:
		if (shortValue == m_spkformtagleft)
		{
			m_psC->kleft = m_ChartSpkWnd_Shape.m_VTtags.GetValue(m_spkformtagleft);
			m_t1 = convertSpikeIndexToTime(m_psC->kleft);
			mm_t1.m_bEntryDone = TRUE;
			OnEnChangeT1();
		}
		else if (shortValue == m_spkformtagright)
		{
			m_psC->kright = m_ChartSpkWnd_Shape.m_VTtags.GetValue(m_spkformtagright);
			m_t2 = convertSpikeIndexToTime(m_psC->kright);
			mm_t2.m_bEntryDone = TRUE;
			OnEnChangeT2();
		}
		m_templList.SetTemplateLength(0, m_psC->kleft, m_psC->kright);
		m_templList.Invalidate();
		break;

	case HINT_CHANGEHZLIMITS:
	case HINT_CHANGEZOOM:
	case HINT_VIEWSIZECHANGED:
		setExtentZeroAllDisplay(m_ChartSpkWnd_Shape.GetYWExtent(), m_ChartSpkWnd_Shape.GetYWOrg());
		updateLegends();
		break;

	case HINT_RMOUSEBUTTONDOWN:
		editSpikeClass(HIWORD(lParam), shortValue);
		break;
	case HINT_VIEWTABHASCHANGED:
		updateCtrlTab1(shortValue);
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
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_spikenoclass++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_spikenoclass--;
			break;
		default: ;
		}

		mm_spikenoclass.m_bEntryDone = FALSE; // clear flag
		mm_spikenoclass.m_nChar = 0; // empty buffer
		mm_spikenoclass.SetSel(0, -1); // select all text

		if (m_spikenoclass != spikenoclass) // change display if necessary
		{
			m_ChartSpkWnd_Shape.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);
			m_ChartSpkWnd_Shape.Invalidate();
			updateLegends();
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
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_timefirst++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_timefirst--;
			break;
		default: ;
		}

		mm_timefirst.m_bEntryDone = FALSE;
		mm_timefirst.m_nChar = 0;
		mm_timefirst.SetSel(0, -1); //select all text

		const auto l_first = static_cast<long>(m_timefirst * m_pSpkDoc->GetAcqRate());
		if (l_first != m_lFirst)
		{
			m_lFirst = l_first;
			updateLegends();
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
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_timelast++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_timelast--;
			break;
		default: ;
		}

		mm_timelast.m_bEntryDone = FALSE;
		mm_timelast.m_nChar = 0;
		mm_timelast.SetSel(0, -1); //select all text

		const auto l_last = static_cast<long>(m_timelast * m_pSpkDoc->GetAcqRate());
		if (l_last != m_lLast)
		{
			m_lLast = l_last;
			updateLegends();
		}
	}
}

void CViewSpikeTemplates::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CdbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
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
	case SB_LEFT: l_first = 0;
		break; // Scroll to far left.
	case SB_LINELEFT: l_first -= sb_scroll;
		break; // Scroll left.
	case SB_LINERIGHT: l_first += sb_scroll;
		break; // Scroll right
	case SB_PAGELEFT: l_first -= page_scroll;
		break; // Scroll one page left
	case SB_PAGERIGHT: l_first += page_scroll;
		break; // Scroll one page right.
	case SB_RIGHT: l_first = total_scroll - page_scroll + 1;
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
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
		updateLegends();
	}
	else
		updateScrollBar();
}

void CViewSpikeTemplates::updateScrollBar()
{
	if (m_lFirst == 0 && m_lLast >= m_pSpkDoc->GetAcqSize() - 1)
		GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_HIDE);
	else
	{
		GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_SHOW);
		m_scrollFilePos_infos.fMask = SIF_ALL;
		m_scrollFilePos_infos.nPos = m_lFirst;
		m_scrollFilePos_infos.nPage = m_lLast - m_lFirst;
		static_cast<CScrollBar*>(GetDlgItem(IDC_SCROLLBAR1))->SetScrollInfo(&m_scrollFilePos_infos);
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
	m_ChartSpkWnd_Shape.SetXWExtOrg(x_we, x_wo);
	updateLegends();
}

void CViewSpikeTemplates::OnFormatGainadjust()
{
	int maxval, minval;
	GetDocument()->GetAllSpkMaxMin(m_bAllFiles, TRUE, &maxval, &minval);
	const auto extent = MulDiv(maxval - minval + 1, 10, 9);
	const auto zero = (maxval + minval) / 2;

	setExtentZeroAllDisplay(extent, zero);
}

void CViewSpikeTemplates::OnFormatCentercurve()
{
	int maxval, minval;
	GetDocument()->GetAllSpkMaxMin(m_bAllFiles, TRUE, &maxval, &minval);
	const auto extent = m_ChartSpkWnd_Shape.GetYWExtent();
	const auto zero = (maxval + minval) / 2;
	setExtentZeroAllDisplay(extent, zero);
}

void CViewSpikeTemplates::setExtentZeroAllDisplay(int extent, int zero)
{
	m_ChartSpkWnd_Shape.SetYWExtOrg(extent, zero);
	m_templList.SetYWExtOrg(extent, zero);
	m_avgList.SetYWExtOrg(extent, zero);
	m_avgAllList.SetYWExtOrg(extent, zero);

	m_ChartSpkWnd_Shape.Invalidate();
	m_templList.Invalidate();
	m_avgList.Invalidate();
	m_avgAllList.Invalidate();
}

void CViewSpikeTemplates::OnEnChangeHitrate()
{
	if (mm_hitrate.m_bEntryDone)
	{
		switch (mm_hitrate.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_hitrate++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_hitrate--;
			break;
		default: ;
		}

		mm_hitrate.m_bEntryDone = FALSE;
		mm_hitrate.m_nChar = 0;
		mm_hitrate.SetSel(0, -1); //select all text

		if (m_psC->hitrate != m_hitrate)
			m_psC->hitrate = m_hitrate;
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
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_hitratesort++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_hitratesort--;
			break;
		default: ;
		}

		mm_hitratesort.m_bEntryDone = FALSE;
		mm_hitratesort.m_nChar = 0;
		mm_hitratesort.SetSel(0, -1); //select all text

		if (m_psC->hitratesort != m_hitratesort)
			m_psC->hitratesort = m_hitratesort;
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
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_ktolerance += 0.1f;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_ktolerance -= 0.1f;
			break;
		default: ;
		}

		if (m_ktolerance < 0)
			m_ktolerance = -m_ktolerance;
		mm_ktolerance.m_bEntryDone = FALSE;
		mm_ktolerance.m_nChar = 0;
		mm_ktolerance.SetSel(0, -1); //select all text

		if (m_psC->ktolerance != m_ktolerance)
			m_psC->ktolerance = m_ktolerance;
		UpdateData(FALSE);
	}
}

void CViewSpikeTemplates::displayAvg(BOOL ballfiles, CTemplateListWnd* pTPList) //, CImageList* pImList)
{
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();

	// get list of classes
	pTPList->SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);
	int tpllen = m_psC->kright - m_psC->kleft + 1;

	// define and attach to ImageList to CListCtrl; create 1 item by default
	if (pTPList->GetImageList(LVSIL_NORMAL) != &pTPList->m_imageList)
	{
		CRect rect;
		m_ChartSpkWnd_Shape.GetClientRect(&rect);
		pTPList->m_imageList.Create(rect.right, rect.bottom, ILC_COLOR8, 1, 1);
		pTPList->SetImageList(&pTPList->m_imageList, LVSIL_NORMAL);
	}

	// reinit all templates to zero
	pTPList->DeleteAllItems();
	int spikelen = m_pSpkList->GetSpikeLength();
	pTPList->SetTemplateLength(spikelen, 0, spikelen - 1);
	pTPList->SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);

	int zero = m_ChartSpkWnd_Shape.GetYWOrg();
	int extent = m_ChartSpkWnd_Shape.GetYWExtent();
	if (zero == 0 && extent == 0)
	{
		int maxval, minval;
		m_pSpkList->GetTotalMaxMin(TRUE, &maxval, &minval);
		extent = maxval - minval;
		zero = (maxval + minval) / 2;
		m_ChartSpkWnd_Shape.SetYWExtOrg(extent, zero);
	}
	pTPList->SetYWExtOrg(extent, zero);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition(); // index current file
	auto firstfile = currentfile; // index first file in the series
	auto lastfile = currentfile; // index last file in the series
	// make sure we have the correct spike list here
	const auto currentlist = m_tabCtrl.GetCurSel();
	m_pSpkDoc->SetSpkList_AsCurrent(currentlist);

	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	if (ballfiles)
	{
		firstfile = 0; // index first file
		lastfile = p_dbwave_doc->GetDB_NRecords() - 1; // index last file
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
		if (!pSpkList->IsClassListValid()) // if class list not valid:
		{
			pSpkList->UpdateClassList(); // rebuild list of classes
			pSpkDoc->SetModifiedFlag(); // and set modified flag
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
			pTPList->tAdd(j_templ, p_spik); // add spike to template j
			pTPList->tAdd(p_spik); // add spike to template zero
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
	pTPList->UpdateTemplateLegends("cx");
}

void CViewSpikeTemplates::OnBuildTemplates()
{
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();

	// set file indexes
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition();
	const auto currentlist = m_tabCtrl.GetCurSel();
	auto firstfile = currentfile;
	auto lastfile = firstfile;
	if (m_bAllFiles)
	{
		firstfile = 0;
		lastfile = p_dbwave_doc->GetDB_NRecords() - 1;
	}

	// add as many forms as we have classes
	m_templList.DeleteAllItems(); // reinit all templates to zero
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
		if (m_bAllFiles)
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
		if (m_bAllFiles)
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
			const auto ii_time = m_pSpkList->GetSpikeTime(i);
			if (ii_time < m_lFirst || ii_time > m_lLast)
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
			m_templList.tAdd(tplmin, p_spik); // add spike to template j
		}
	}

	// end of loop, select current file again if necessary
	if (m_bAllFiles)
	{
		p_dbwave_doc->SetDB_CurrentRecordPosition(currentfile);
		m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	m_templList.SortTemplatesByNumberofSpikes(TRUE, TRUE, m_ifirstsortedclass);
	updateTemplates();
}

void CViewSpikeTemplates::sortSpikes()
{
	// set tolerance to sort tolerance
	m_templList.SetHitRate_Tolerance(&m_hitratesort, &m_ktolerance);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition(); // index current file
	auto firstfile = currentfile; // index first file in the series
	auto lastfile = firstfile; // index last file in the series
	const auto currentlist = m_tabCtrl.GetCurSel();

	// change indexes if ALL files selected
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	if (m_bAllFiles)
	{
		firstfile = 0; // index first file
		lastfile = p_dbwave_doc->GetDB_NRecords() - 1; // index last file
	}

	// loop CFrameWnd
	const auto ntempl = m_templList.GetNtemplates();
	for (auto ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_bAllFiles)
		{
			p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
			m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->GetDB_CurrentSpkFileName(FALSE);
			p_dbwave_doc->SetTitle(cs);
			m_pSpkDoc->SetModifiedFlag(FALSE);

			m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(currentlist); // load pointer to spike list
			if (!m_pSpkList->IsClassListValid()) // if class list not valid:
			{
				m_pSpkList->UpdateClassList(); // rebuild list of classes
				m_pSpkDoc->SetModifiedFlag(); // and set modified flag
			}
		}

		// spike loop
		const auto nspikes = m_pSpkList->GetTotalSpikes(); // loop over all spikes
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
			const auto ii_time = m_pSpkList->GetSpikeTime(ispike);
			if (ii_time < m_lFirst || ii_time > m_lLast)
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

			GetDocument()->SetDB_n_spikes(m_pSpkList->GetTotalSpikes());
			GetDocument()->SetDB_n_spike_classes(m_pSpkList->GetNbclasses());
		}
	}

	// end of loop, select current file again if necessary
	if (m_bAllFiles)
	{
		p_dbwave_doc->SetDB_CurrentRecordPosition(currentfile);
		m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update display: average and spk form
	displayAvg(FALSE, &m_avgList);
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	m_ChartSpkWnd_Shape.SetSourceData(m_pSpkList, GetDocument());
	m_ChartSpkWnd_Shape.Invalidate();
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

void CViewSpikeTemplates::editSpikeClass(int controlID, int controlItem)
{
	// find which item has been selected
	CTemplateListWnd* pList = nullptr;
	auto b_spikes = TRUE;
	auto b_all_files = m_bAllFiles;
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
	DlgEditSpikeClass dlg;
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
			auto firstfile = currentfile; // index first file in the series
			auto lastfile = firstfile; // index last file in the series
			const auto currentlist = m_tabCtrl.GetCurSel();

			// change indexes if ALL files selected
			CString cscomment;
			CString csfilecomment = _T("Analyze file: ");
			if (b_all_files)
			{
				firstfile = 0; // index first file
				lastfile = p_dbwave_doc->GetDB_NRecords() - 1; // index last file
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
				m_pSpkList->UpdateClassList(); // rebuild list of classes
				m_pSpkList->ChangeSpikeClassID(oldclass, dlg.m_iClass);
				m_pSpkList->UpdateClassList(); // rebuild list of classes
				m_pSpkDoc->SetModifiedFlag(TRUE);

				if (m_pSpkDoc->IsModified())
				{
					m_pSpkDoc->OnSaveDocument(p_dbwave_doc->GetDB_CurrentSpkFileName(FALSE));
					m_pSpkDoc->SetModifiedFlag(FALSE);

					GetDocument()->SetDB_n_spikes(m_pSpkList->GetTotalSpikes());
					GetDocument()->SetDB_n_spike_classes(m_pSpkList->GetNbclasses());
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

void CViewSpikeTemplates::updateCtrlTab1(int iselect)
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
	sortSpikes();
}

void CViewSpikeTemplates::OnBnClickedDisplay()
{
	displayAvg(TRUE, &m_avgAllList); //, &m_ImListAll);
}

void CViewSpikeTemplates::OnEnChangeIfirstsortedclass()
{
	if (mm_ifirstsortedclass.m_bEntryDone)
	{
		switch (mm_ifirstsortedclass.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_ifirstsortedclass++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_ifirstsortedclass--;
			break;
		default: ;
		}

		mm_ifirstsortedclass.m_bEntryDone = FALSE;
		mm_ifirstsortedclass.m_nChar = 0;
		mm_ifirstsortedclass.SetSel(0, -1); //select all text

		// change class of all templates
		//SetTemplateclassID(int item, LPCSTR pszType, int classID)
		m_templList.UpdateTemplateBaseClassID(m_ifirstsortedclass);
		UpdateData(FALSE);
	}
}

void CViewSpikeTemplates::OnTcnSelchangeTab2(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	selectSpikeList(icursel);
	*pResult = 0;
}

void CViewSpikeTemplates::OnNMClickTab2(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	selectSpikeList(icursel);
	*pResult = 0;
}

void CViewSpikeTemplates::OnBnClickedDisplaysingleclass()
{
	UpdateData(TRUE);
	if (m_bDisplaySingleClass)
	{
		GetDlgItem(IDC_EDIT2)->ShowWindow(SW_SHOW);
		m_ChartSpkWnd_Shape.SetPlotMode(PLOT_ONECLASS, m_spikenoclass);
	}
	else
	{
		GetDlgItem(IDC_EDIT2)->ShowWindow(SW_HIDE);
		m_ChartSpkWnd_Shape.SetPlotMode(PLOT_BLACK, m_spikenoclass);
	}
	m_ChartSpkWnd_Shape.Invalidate();
}

void CViewSpikeTemplates::OnEnChangeT1()
{
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();

	if (mm_t1.m_bEntryDone)
	{
		auto t1 = m_t1;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();

		switch (mm_t1.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			t1 = m_t1;
			break;
		case VK_UP:
		case VK_PRIOR:
			t1 += delta;
			break;
		case VK_DOWN:
		case VK_NEXT:
			t1 -= delta;
			break;
		default: ;
		}
		// check boundaries
		if (t1 < 0)
			t1 = 0.0f;
		if (t1 >= m_t2)
			t1 = m_t2 - delta;
		// change display if necessary
		mm_t1.m_bEntryDone = FALSE; // clear flag
		mm_t1.m_nChar = 0; // empty buffer
		mm_t1.SetSel(0, -1); // select all text
		m_t1 = t1;
		const int it1 = convertTimeToSpikeIndex(m_t1);
		if (it1 != m_ChartSpkWnd_Shape.m_VTtags.GetValue(m_spkformtagleft))
		{
			m_psC->kleft = it1;
			m_ChartSpkWnd_Shape.MoveVTtrack(m_spkformtagleft, m_psC->kleft);
			m_pSpkList->m_imaxmin1SL = m_psC->kleft;
		}
		UpdateData(FALSE);
	}
}

void CViewSpikeTemplates::OnEnChangeT2()
{
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();

	if (mm_t2.m_bEntryDone)
	{
		auto t2 = m_t2;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();
		switch (mm_t2.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			t2 = m_t2;
			break;
		case VK_UP:
		case VK_PRIOR:
			t2 += delta;
			break;
		case VK_DOWN:
		case VK_NEXT:
			t2 -= delta;
			break;
		default: ;
		}

		// check boundaries
		if (t2 < m_t1)
			t2 = m_t1 + delta;

		int spikelen = m_pSpkList->GetSpikeLength();
		const auto tmax = convertSpikeIndexToTime(spikelen - 1);
		if (t2 >= tmax)
			t2 = tmax;
		// change display if necessary
		mm_t2.m_bEntryDone = FALSE; // clear flag
		mm_t2.m_nChar = 0; // empty buffer
		mm_t2.SetSel(0, -1); // select all text
		m_t2 = t2;
		const int it2 = convertTimeToSpikeIndex(m_t2);
		if (it2 != m_ChartSpkWnd_Shape.m_VTtags.GetValue(m_spkformtagright))
		{
			m_psC->kright = it2;
			m_ChartSpkWnd_Shape.MoveVTtrack(m_spkformtagright, m_psC->kright);
			m_pSpkList->m_imaxmin2SL = m_psC->kright;
		}
		UpdateData(FALSE);
	}
}
