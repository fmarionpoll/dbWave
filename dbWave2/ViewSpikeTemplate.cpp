#include "StdAfx.h"
#include "ViewSpikeTemplate.h"
#include "dbWave.h"
#include "DlgEditSpikeClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewSpikeTemplates, dbTableView)

ViewSpikeTemplates::ViewSpikeTemplates()
	: dbTableView(IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

ViewSpikeTemplates::~ViewSpikeTemplates()
{
	if (m_pSpkDoc != nullptr)
		saveCurrentSpkFile(); 
}

void ViewSpikeTemplates::DoDataExchange(CDataExchange* pDX)
{
	dbTableView::DoDataExchange(pDX);

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

BEGIN_MESSAGE_MAP(ViewSpikeTemplates, dbTableView)

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikeTemplates::OnMyMessage)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT2, &ViewSpikeTemplates::OnEnChangeclassno)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewSpikeTemplates::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewSpikeTemplates::OnEnChangeTimelast)
	ON_COMMAND(ID_FORMAT_ALLDATA, &ViewSpikeTemplates::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &ViewSpikeTemplates::OnFormatGainadjust)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &ViewSpikeTemplates::OnFormatCentercurve)
	ON_BN_CLICKED(IDC_BUILD, &ViewSpikeTemplates::OnBuildTemplates)
	ON_EN_CHANGE(IDC_HITRATE, &ViewSpikeTemplates::OnEnChangeHitrate)
	ON_EN_CHANGE(IDC_TOLERANCE, &ViewSpikeTemplates::OnEnChangeTolerance)
	ON_EN_CHANGE(IDC_HITRATE2, &ViewSpikeTemplates::OnEnChangeHitrateSort)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST2, &ViewSpikeTemplates::OnKeydownTemplateList)
	ON_BN_CLICKED(IDC_CHECK1, &ViewSpikeTemplates::OnCheck1)
	ON_BN_CLICKED(IDC_SORT, &ViewSpikeTemplates::OnBnClickedSort)
	ON_BN_CLICKED(IDC_DISPLAY, &ViewSpikeTemplates::OnBnClickedDisplay)
	ON_EN_CHANGE(IDC_IFIRSTSORTEDCLASS, &ViewSpikeTemplates::OnEnChangeIfirstsortedclass)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB2, &ViewSpikeTemplates::OnTcnSelchangeTab2)
	ON_NOTIFY(NM_CLICK, IDC_TAB2, &ViewSpikeTemplates::OnNMClickTab2)
	ON_BN_CLICKED(IDC_DISPLAYSINGLECLASS, &ViewSpikeTemplates::OnBnClickedDisplaysingleclass)
	ON_EN_CHANGE(IDC_T1, &ViewSpikeTemplates::OnEnChangeT1)
	ON_EN_CHANGE(IDC_T2, &ViewSpikeTemplates::OnEnChangeT2)
END_MESSAGE_MAP()

void ViewSpikeTemplates::OnDestroy()
{
	if (m_templList.GetNtemplates() != 0)
	{
		if (m_psC->p_template == nullptr)
			m_psC->CreateTPL();
		*static_cast<CTemplateListWnd*>(m_psC->p_template) = m_templList;
	}
	dbTableView::OnDestroy();
}

BOOL ViewSpikeTemplates::OnMove(UINT nIDMoveCommand)
{
	saveCurrentSpkFile();
	return dbTableView::OnMove(nIDMoveCommand);
}

void ViewSpikeTemplates::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_b_init)
	{
		switch (LOWORD(lHint))
		{
		case HINT_DOCHASCHANGED: // file has changed?
		case HINT_DOCMOVERECORD:
			update_file_parameters();
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

void ViewSpikeTemplates::defineSubClassedItems()
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

void ViewSpikeTemplates::defineStretchParameters()
{
	m_stretch.AttachParent(this); // attach formview pointer
	m_stretch.newProp(IDC_LIST1, SZEQ_XLEQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_LIST2, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_LIST3, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB2, XLEQ_XREQ, SZEQ_YBEQ);
}

void ViewSpikeTemplates::OnInitialUpdate()
{
	dbTableView::OnInitialUpdate();
	defineSubClassedItems();
	defineStretchParameters();
	m_b_init = TRUE;
	m_autoIncrement = true;
	m_autoDetect = true;

	// load global parameters
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	mdPM = &(p_app->options_view_data);
	mdMO = &(p_app->options_view_data_measure);
	m_psC = &(p_app->spk_classification);
	if (m_psC->p_template != nullptr)
		m_templList = *static_cast<CTemplateListWnd*>(m_psC->p_template);

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

	m_hitrate = m_psC->hit_rate;
	m_hitratesort = m_psC->hit_rate_sort;
	m_ktolerance = m_psC->k_tolerance;

	m_ChartSpkWnd_Shape.set_plot_mode(PLOT_ONECLASS, 0);
	m_spkformtagleft = m_ChartSpkWnd_Shape.vertical_tags.add_tag(m_psC->k_left, 0);
	m_spkformtagright = m_ChartSpkWnd_Shape.vertical_tags.add_tag(m_psC->k_right, 0);

	update_file_parameters();
	updateCtrlTab1(0);
}

void ViewSpikeTemplates::update_file_parameters()
{
	update_spike_file();
	const int index_current = m_pSpkDoc->get_spike_list_current_index();
	select_spike_list(index_current);
}

void ViewSpikeTemplates::update_spike_file()
{
	m_pSpkDoc = GetDocument()->open_current_spike_file();

	if (nullptr != m_pSpkDoc)
	{
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->db_get_current_spk_file_name(), FALSE);
		int icur = GetDocument()->get_current_spike_file()->get_spike_list_current_index();
		m_pSpkList = m_pSpkDoc->set_spike_list_as_current(icur);

		// update Tab at the bottom
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);
		m_tabCtrl.SetCurSel(icur);
	}
}

void ViewSpikeTemplates::select_spike_list(const int index_current)
{
	m_pSpkList = m_pSpkDoc->set_spike_list_as_current(index_current);
	m_tabCtrl.SetCurSel(index_current);

	if (!m_pSpkList->is_class_list_valid())
	{
		m_pSpkList->update_class_list();
		m_pSpkDoc->SetModifiedFlag();
	}

	// change pointer to select new spike list & test if one spike is selected
	int spikeno = m_pSpkList->m_selected_spike;
	if (spikeno > m_pSpkList->get_spikes_count() - 1 || spikeno < 0)
		spikeno = -1;
	else
	{
		// set source class to the class of the selected spike
		m_spikenoclass = m_pSpkList->get_spike(spikeno)->get_class_id();
		m_psC->source_class = m_spikenoclass;
	}

	ASSERT(m_spikenoclass < 32768);
	if (m_spikenoclass > 32768)
		m_spikenoclass = 0;

	// prepare display source spikes
	m_ChartSpkWnd_Shape.set_source_data(m_pSpkList, GetDocument());
	if (m_psC->k_left == 0 && m_psC->k_right == 0)
	{
		m_psC->k_left = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
		m_psC->k_right = m_psC->k_left + m_pSpkList->get_detection_parameters()->detect_refractory_period;
	}
	m_t1 = convertSpikeIndexToTime(m_psC->k_left);
	m_t2 = convertSpikeIndexToTime(m_psC->k_right);

	if (!m_bDisplaySingleClass)
		m_ChartSpkWnd_Shape.set_plot_mode(PLOT_BLACK, 0);
	else
		m_ChartSpkWnd_Shape.set_plot_mode(PLOT_ONECLASS, m_spikenoclass);

	m_lFirst = 0;
	m_lLast = m_pSpkDoc->get_acq_size() - 1;
	m_scrollFilePos_infos.nMin = 0;
	m_scrollFilePos_infos.nMax = m_lLast;
	m_ChartSpkWnd_Shape.set_time_intervals(m_lFirst, m_lLast);
	m_ChartSpkWnd_Shape.Invalidate();

	selectSpike(spikeno);
	updateLegends();

	displayAvg(FALSE, &m_avgList);
	updateTemplates();
}

void ViewSpikeTemplates::updateTemplates()
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
		int extent = m_ChartSpkWnd_Shape.get_yw_extent();
		int zero = m_ChartSpkWnd_Shape.get_yw_org();
		m_templList.SetYWExtOrg(extent, zero);
		m_templList.UpdateTemplateLegends("t");
		m_templList.Invalidate();
		n_cmd_show = SW_SHOW;
	}
	GetDlgItem(IDC_NTEMPLS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_NTEMPLATES)->ShowWindow(n_cmd_show);
}

void ViewSpikeTemplates::updateLegends()
{
	if (m_lFirst < 0)
		m_lFirst = 0;
	if (m_lLast <= m_lFirst)
		m_lLast = m_lFirst + 120;
	if (m_lLast >= m_pSpkDoc->get_acq_size())
		m_lLast = m_pSpkDoc->get_acq_size() - 1;
	if (m_lFirst > m_lLast)
		m_lFirst = m_lLast - 120;

	m_timefirst = m_lFirst / m_pSpkDoc->get_acq_rate();
	m_timelast = (m_lLast + 1) / m_pSpkDoc->get_acq_rate();

	m_ChartSpkWnd_Shape.set_time_intervals(m_lFirst, m_lLast);
	m_ChartSpkWnd_Shape.Invalidate();

	UpdateData(FALSE);
	updateScrollBar();
}

void ViewSpikeTemplates::selectSpike(short spikeno)
{
	CdbWaveDoc* pDoc = m_ChartSpkWnd_Shape.get_db_wave_doc();
	dbSpike spike_sel(pDoc->db_get_current_record_position(),
		pDoc->m_p_spk->get_spike_list_current_index(),
		spikeno);
	m_ChartSpkWnd_Shape.select_spike(spike_sel);
	m_spikeno = spikeno;
	m_pSpkList->m_selected_spike = spikeno;
}

LRESULT ViewSpikeTemplates::OnMyMessage(WPARAM wParam, LPARAM lParam)
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
			m_psC->k_left = m_ChartSpkWnd_Shape.vertical_tags.get_value(m_spkformtagleft);
			m_t1 = convertSpikeIndexToTime(m_psC->k_left);
			mm_t1.m_bEntryDone = TRUE;
			OnEnChangeT1();
		}
		else if (shortValue == m_spkformtagright)
		{
			m_psC->k_right = m_ChartSpkWnd_Shape.vertical_tags.get_value(m_spkformtagright);
			m_t2 = convertSpikeIndexToTime(m_psC->k_right);
			mm_t2.m_bEntryDone = TRUE;
			OnEnChangeT2();
		}
		m_templList.SetTemplateLength(0, m_psC->k_left, m_psC->k_right);
		m_templList.Invalidate();
		break;

	case HINT_CHANGEHZLIMITS:
	case HINT_CHANGEZOOM:
	case HINT_VIEWSIZECHANGED:
		setExtentZeroAllDisplay(m_ChartSpkWnd_Shape.get_yw_extent(), m_ChartSpkWnd_Shape.get_yw_org());
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

void ViewSpikeTemplates::OnEnChangeclassno()
{
	if (mm_spikenoclass.m_bEntryDone)
	{
		const auto spike_no_class = m_spikenoclass;
		mm_spikenoclass.OnEnChange(this, m_spikenoclass, 1, -1);

		if (m_spikenoclass != spike_no_class) // change display if necessary
		{
			m_ChartSpkWnd_Shape.set_plot_mode(PLOT_ONECLASS, m_spikenoclass);
			m_ChartSpkWnd_Shape.Invalidate();
			updateLegends();
		}
	}
}

void ViewSpikeTemplates::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone)
	{
		mm_timefirst.OnEnChange(this, m_timefirst, 1.f, -1.f);

		const auto l_first = static_cast<long>(m_timefirst * m_pSpkDoc->get_acq_rate());
		if (l_first != m_lFirst)
		{
			m_lFirst = l_first;
			updateLegends();
		}
	}
}

void ViewSpikeTemplates::OnEnChangeTimelast()
{
	if (mm_timelast.m_bEntryDone)
	{
		mm_timelast.OnEnChange(this, m_timelast, 1.f, -1.f);

		const auto l_last = static_cast<long>(m_timelast * m_pSpkDoc->get_acq_rate());
		if (l_last != m_lLast)
		{
			m_lLast = l_last;
			updateLegends();
		}
	}
}

void ViewSpikeTemplates::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		dbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// get corresponding data
	const auto total_scroll = m_pSpkDoc->get_acq_size();
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

void ViewSpikeTemplates::updateScrollBar()
{
	if (m_lFirst == 0 && m_lLast >= m_pSpkDoc->get_acq_size() - 1)
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

void ViewSpikeTemplates::OnFormatAlldata()
{
	// dots: spk file length
	m_lFirst = 0;
	m_lLast = m_pSpkDoc->get_acq_size() - 1;
	// spikes: center spikes horizontally and adjust hz size of display
	const short x_wo = 0;
	const short x_we = m_pSpkList->get_spike_length();
	m_ChartSpkWnd_Shape.set_xw_ext_org(x_we, x_wo);
	updateLegends();
}

void ViewSpikeTemplates::OnFormatGainadjust()
{
	short maxval, minval;
	GetDocument()->get_max_min_of_all_spikes(m_bAllFiles, TRUE, &maxval, &minval);
	const auto extent = MulDiv(maxval - minval + 1, 10, 9);
	const auto zero = (maxval + minval) / 2;

	setExtentZeroAllDisplay(extent, zero);
}

void ViewSpikeTemplates::OnFormatCentercurve()
{
	short maxval, minval;
	GetDocument()->get_max_min_of_all_spikes(m_bAllFiles, TRUE, &maxval, &minval);
	const auto extent = m_ChartSpkWnd_Shape.get_yw_extent();
	const auto zero = (maxval + minval) / 2;
	setExtentZeroAllDisplay(extent, zero);
}

void ViewSpikeTemplates::setExtentZeroAllDisplay(int extent, int zero)
{
	m_ChartSpkWnd_Shape.set_yw_ext_org(extent, zero);
	m_templList.SetYWExtOrg(extent, zero);
	m_avgList.SetYWExtOrg(extent, zero);
	m_avgAllList.SetYWExtOrg(extent, zero);

	m_ChartSpkWnd_Shape.Invalidate();
	m_templList.Invalidate();
	m_avgList.Invalidate();
	m_avgAllList.Invalidate();
}

void ViewSpikeTemplates::OnEnChangeHitrate()
{
	if (mm_hitrate.m_bEntryDone)
	{
		mm_hitrate.OnEnChange(this, m_hitrate, 1, -1);

		if (m_psC->hit_rate != m_hitrate)
			m_psC->hit_rate = m_hitrate;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::OnEnChangeHitrateSort()
{
	if (mm_hitratesort.m_bEntryDone)
	{
		mm_hitratesort.OnEnChange(this, m_hitratesort, 1, -1);

		if (m_psC->hit_rate_sort != m_hitratesort)
			m_psC->hit_rate_sort = m_hitratesort;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::OnEnChangeTolerance()
{
	if (mm_ktolerance.m_bEntryDone)
	{
		mm_ktolerance.OnEnChange(this, m_ktolerance, 1.f, -1.f);
		if (m_ktolerance < 0)
			m_ktolerance = -m_ktolerance;
		if (m_psC->k_tolerance != m_ktolerance)
			m_psC->k_tolerance = m_ktolerance;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::displayAvg(BOOL ballfiles, CTemplateListWnd* pTPList) //, CImageList* pImList)
{
	m_pSpkList = m_pSpkDoc->get_spike_list_current();

	// get list of classes
	pTPList->SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);
	int tpllen = m_psC->k_right - m_psC->k_left + 1;

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
	int spikelen = m_pSpkList->get_spike_length();
	pTPList->SetTemplateLength(spikelen, 0, spikelen - 1);
	pTPList->SetHitRate_Tolerance(&m_hitrate, &m_ktolerance);

	int zero = m_ChartSpkWnd_Shape.get_yw_org();
	int extent = m_ChartSpkWnd_Shape.get_yw_extent();
	if (zero == 0 && extent == 0)
	{
		short valuemax, valuemin;
		m_pSpkList->get_total_max_min(TRUE, &valuemax, &valuemin);
		extent = valuemax - valuemin;
		zero = (valuemax + valuemin) / 2;
		m_ChartSpkWnd_Shape.set_yw_ext_org(extent, zero);
	}
	pTPList->SetYWExtOrg(extent, zero);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int current_file = p_dbwave_doc->db_get_current_record_position(); // index current file
	auto first_file = current_file; // index first file in the series
	auto last_file = current_file; // index last file in the series
	// make sure we have the correct spike list here
	const auto current_list = m_tabCtrl.GetCurSel();
	m_pSpkDoc->set_spike_list_as_current(current_list);

	CString cs_comment;
	CString cs_file_comment = _T("Analyze file: ");
	if (ballfiles)
	{
		first_file = 0; // index first file
		last_file = p_dbwave_doc->db_get_n_records() - 1; // index last file
	}
	// loop over files
	for (auto i_file = first_file; i_file <= last_file; i_file++)
	{
		// load file
		p_dbwave_doc->db_set_current_record_position(i_file);
		auto pSpkDoc = p_dbwave_doc->open_current_spike_file();
		if (pSpkDoc == nullptr)
			continue;
		CString cs;
		cs.Format(_T("%i/%i - "), i_file, last_file);
		cs += p_dbwave_doc->db_get_current_spk_file_name(FALSE);
		p_dbwave_doc->SetTitle(cs);
		pSpkDoc->SetModifiedFlag(FALSE);

		auto pSpkList = pSpkDoc->set_spike_list_as_current(current_list); // load pointer to spike list
		if (!pSpkList->is_class_list_valid()) // if class list not valid:
		{
			pSpkList->update_class_list(); // rebuild list of classes
			pSpkDoc->SetModifiedFlag(); // and set modified flag
		}
		const auto nspikes = pSpkList->get_spikes_count();

		// add spikes to templates - create templates on the fly
		int j_templ;
		for (auto i = 0; i < nspikes; i++)
		{
			const auto cla = pSpkList->get_spike(i)->get_class_id();
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
			const auto p_spik = pSpkList->get_spike(i)->get_p_data();
			pTPList->tAdd(j_templ, p_spik); // add spike to template j
			pTPList->tAdd(p_spik); // add spike to template zero
		}
	}

	// end of loop, select current file again if necessary
	if (ballfiles)
	{
		p_dbwave_doc->db_set_current_record_position(current_file);
		m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update average
	pTPList->TransferTemplateData();
	pTPList->tGlobalstats();
	pTPList->UpdateTemplateLegends("cx");
}

void ViewSpikeTemplates::OnBuildTemplates()
{
	m_pSpkList = m_pSpkDoc->get_spike_list_current();

	// set file indexes
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->db_get_current_record_position();
	const auto currentlist = m_tabCtrl.GetCurSel();
	auto firstfile = currentfile;
	auto lastfile = firstfile;
	if (m_bAllFiles)
	{
		firstfile = 0;
		lastfile = p_dbwave_doc->db_get_n_records() - 1;
	}

	// add as many forms as we have classes
	m_templList.DeleteAllItems(); // reinit all templates to zero
	m_templList.SetTemplateLength(m_pSpkList->get_spike_length(), m_psC->k_left, m_psC->k_right);
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
			p_dbwave_doc->db_set_current_record_position(ifile);
			m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
		}

		const auto spike_list = m_pSpkDoc->set_spike_list_as_current(currentlist);
		nspikes = spike_list->get_spikes_count();
		for (auto i = 0; i < nspikes; i++)
			m_templList.tAdd(m_pSpkList->get_spike(i)->get_p_data());
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
			p_dbwave_doc->db_set_current_record_position(ifile);
			p_dbwave_doc->open_current_spike_file();
			m_pSpkDoc = p_dbwave_doc->m_p_spk;
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->db_get_current_spk_file_name(FALSE);
			p_dbwave_doc->SetTitle(cs);
		}

		auto spike_list = m_pSpkDoc->set_spike_list_as_current(currentlist);
		nspikes = spike_list->get_spikes_count();

		// create template CListCtrl
		for (auto i = 0; i < nspikes; i++)
		{
			// filter out undesirable spikes
			if (m_bDisplaySingleClass)
			{
				if (m_pSpkList->get_spike(i)->get_class_id() != m_spikenoclass)
					continue;
			}
			const auto ii_time = m_pSpkList->get_spike(i)->get_time();
			if (ii_time < m_lFirst || ii_time > m_lLast)
				continue;

			// get pointer to spike data and search if any template is suitable
			auto* p_spik = m_pSpkList->get_spike(i)->get_p_data();
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
		p_dbwave_doc->db_set_current_record_position(currentfile);
		m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	m_templList.SortTemplatesByNumberofSpikes(TRUE, TRUE, m_ifirstsortedclass);
	updateTemplates();
}

void ViewSpikeTemplates::sortSpikes()
{
	// set tolerance to sort tolerance
	m_templList.SetHitRate_Tolerance(&m_hitratesort, &m_ktolerance);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->db_get_current_record_position(); // index current file
	auto firstfile = currentfile; // index first file in the series
	auto lastfile = firstfile; // index last file in the series
	const auto currentlist = m_tabCtrl.GetCurSel();

	// change indexes if ALL files selected
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	if (m_bAllFiles)
	{
		firstfile = 0; // index first file
		lastfile = p_dbwave_doc->db_get_n_records() - 1; // index last file
	}

	// loop CFrameWnd
	const auto ntempl = m_templList.GetNtemplates();
	for (auto ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_bAllFiles)
		{
			p_dbwave_doc->db_set_current_record_position(ifile);
			m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->db_get_current_spk_file_name(FALSE);
			p_dbwave_doc->SetTitle(cs);
			m_pSpkDoc->SetModifiedFlag(FALSE);

			m_pSpkList = m_pSpkDoc->set_spike_list_as_current(currentlist); // load pointer to spike list
			if (!m_pSpkList->is_class_list_valid()) // if class list not valid:
			{
				m_pSpkList->update_class_list(); // rebuild list of classes
				m_pSpkDoc->SetModifiedFlag(); // and set modified flag
			}
		}

		// spike loop
		const auto nspikes = m_pSpkList->get_spikes_count(); // loop over all spikes
		for (auto ispike = 0; ispike < nspikes; ispike++)
		{
			// filter out undesirable spikes - i.e. not relevant to the sort
			if (m_bDisplaySingleClass)
			{
				// skip spikes that do not belong to selected class
				if (m_pSpkList->get_spike(ispike)->get_class_id() != m_spikenoclass)
					continue;
			}

			// skip spikes that do not fit into time interval selected
			const auto ii_time = m_pSpkList->get_spike(ispike)->get_time();
			if (ii_time < m_lFirst || ii_time > m_lLast)
				continue;

			// get pointer to spike data and search if any template is suitable
			const auto p_spik = m_pSpkList->get_spike(ispike)->get_p_data();
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
				if (m_pSpkList->get_spike(ispike)->get_class_id() != class_id)
				{
					m_pSpkList->get_spike(ispike)->set_class_id(class_id);
					m_pSpkDoc->SetModifiedFlag(TRUE);
				}
			}
		}
		if (m_pSpkDoc->IsModified())
		{
			m_pSpkDoc->OnSaveDocument(p_dbwave_doc->db_get_current_spk_file_name(FALSE));
			m_pSpkDoc->SetModifiedFlag(FALSE);

			GetDocument()->set_db_n_spikes(m_pSpkList->get_spikes_count());
			GetDocument()->set_db_n_spike_classes(m_pSpkList->get_classes_count());
		}
	}

	// end of loop, select current file again if necessary
	if (m_bAllFiles)
	{
		p_dbwave_doc->db_set_current_record_position(currentfile);
		m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update display: average and spk form
	displayAvg(FALSE, &m_avgList);
	m_pSpkList = m_pSpkDoc->get_spike_list_current();
	m_ChartSpkWnd_Shape.set_source_data(m_pSpkList, GetDocument());
	m_ChartSpkWnd_Shape.Invalidate();
}

void ViewSpikeTemplates::OnKeydownTemplateList(NMHDR* pNMHDR, LRESULT* pResult)
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

void ViewSpikeTemplates::OnCheck1()
{
	UpdateData(TRUE);
}

void ViewSpikeTemplates::editSpikeClass(int controlID, int controlItem)
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
			const auto dbwave_doc = GetDocument();
			const int current_file_index = dbwave_doc->db_get_current_record_position(); 
			auto index_first_file = current_file_index; // index first file in the series
			auto index_last_file = index_first_file; // index last file in the series
			const auto current_list = m_tabCtrl.GetCurSel();

			// change indexes if ALL files selected
			CString cs_comment;
			CString cs_file_comment = _T("Analyze file: ");
			if (b_all_files)
			{
				index_first_file = 0; // index first file
				index_last_file = dbwave_doc->db_get_n_records() - 1; // index last file
			}

			// loop CFrameWnd
			for (auto index_file = index_first_file; index_file <= index_last_file; index_file++)
			{
				// store nb of spikes within array
				if (b_all_files)
				{
					dbwave_doc->db_set_current_record_position(index_file);
					m_pSpkDoc = dbwave_doc->open_current_spike_file();
					CString cs;
					cs.Format(_T("%i/%i - "), index_file, index_last_file);
					cs += dbwave_doc->db_get_current_spk_file_name(FALSE);
					dbwave_doc->SetTitle(cs);
					m_pSpkDoc->SetModifiedFlag(FALSE);
					m_pSpkList = m_pSpkDoc->set_spike_list_as_current(current_list);
				}

				// TODO: this should not work - changing SpikeClassID does not change the spike class because UpdateClassList reset classes array to zero
				m_pSpkList->update_class_list(); // rebuild list of classes
				m_pSpkList->change_all_spike_from_class_id_to_new_class_id(oldclass, dlg.m_iClass);
				m_pSpkList->update_class_list(); // rebuild list of classes
				m_pSpkDoc->SetModifiedFlag(TRUE);

				if (m_pSpkDoc->IsModified())
				{
					m_pSpkDoc->OnSaveDocument(dbwave_doc->db_get_current_spk_file_name(FALSE));
					m_pSpkDoc->SetModifiedFlag(FALSE);

					GetDocument()->set_db_n_spikes(m_pSpkList->get_spikes_count());
					GetDocument()->set_db_n_spike_classes(m_pSpkList->get_classes_count());
				}
			}

			// end of loop, select current file again if necessary
			if (b_all_files)
			{
				dbwave_doc->db_set_current_record_position(current_file_index);
				m_pSpkDoc = dbwave_doc->open_current_spike_file();
				dbwave_doc->SetTitle(dbwave_doc->GetPathName());
			}
		}
	}
}

void ViewSpikeTemplates::updateCtrlTab1(int iselect)
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

void ViewSpikeTemplates::OnBnClickedSort()
{
	sortSpikes();
}

void ViewSpikeTemplates::OnBnClickedDisplay()
{
	displayAvg(TRUE, &m_avgAllList); //, &m_ImListAll);
}

void ViewSpikeTemplates::OnEnChangeIfirstsortedclass()
{
	if (mm_ifirstsortedclass.m_bEntryDone)
	{
		mm_ifirstsortedclass.OnEnChange(this, m_ifirstsortedclass, 1, -1);

		// change class of all templates
		//SetTemplateclassID(int item, LPCSTR pszType, int classID)
		m_templList.UpdateTemplateBaseClassID(m_ifirstsortedclass);
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::OnTcnSelchangeTab2(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	select_spike_list(icursel);
	*pResult = 0;
}

void ViewSpikeTemplates::OnNMClickTab2(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	select_spike_list(icursel);
	*pResult = 0;
}

void ViewSpikeTemplates::OnBnClickedDisplaysingleclass()
{
	UpdateData(TRUE);
	if (m_bDisplaySingleClass)
	{
		GetDlgItem(IDC_EDIT2)->ShowWindow(SW_SHOW);
		m_ChartSpkWnd_Shape.set_plot_mode(PLOT_ONECLASS, m_spikenoclass);
	}
	else
	{
		GetDlgItem(IDC_EDIT2)->ShowWindow(SW_HIDE);
		m_ChartSpkWnd_Shape.set_plot_mode(PLOT_BLACK, m_spikenoclass);
	}
	m_ChartSpkWnd_Shape.Invalidate();
}

void ViewSpikeTemplates::OnEnChangeT1()
{
	m_pSpkList = m_pSpkDoc->get_spike_list_current();

	if (mm_t1.m_bEntryDone)
	{
		const auto delta = m_tunit / m_pSpkList->get_acq_sampling_rate();
		
		mm_t1.OnEnChange(this, m_t1, delta, -delta);
		// check boundaries
		if (m_t1 < 0)
			m_t1 = 0.0f;
		if (m_t1 >= m_t2)
			m_t1 = m_t2 - delta;

		const int it1 = convertTimeToSpikeIndex(m_t1);
		if (it1 != m_ChartSpkWnd_Shape.vertical_tags.get_value(m_spkformtagleft))
		{
			m_psC->k_left = it1;
			m_ChartSpkWnd_Shape.move_vt_track(m_spkformtagleft, m_psC->k_left);
			m_pSpkList->m_imaxmin1SL = m_psC->k_left;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::OnEnChangeT2()
{
	m_pSpkList = m_pSpkDoc->get_spike_list_current();

	if (mm_t2.m_bEntryDone)
	{
		const auto delta = m_tunit / m_pSpkList->get_acq_sampling_rate();
		mm_t2.OnEnChange(this, m_t2, delta, -delta);

		// check boundaries
		if (m_t2 < m_t1)
			m_t2 = m_t1 + delta;

		const int spike_length = m_pSpkList->get_spike_length();
		const auto t_max = convertSpikeIndexToTime(spike_length - 1);
		if (m_t2 >= t_max)
			m_t2 = t_max;
		// change display if necessary
		const int it2 = convertTimeToSpikeIndex(m_t2);
		if (it2 != m_ChartSpkWnd_Shape.vertical_tags.get_value(m_spkformtagright))
		{
			m_psC->k_right = it2;
			m_ChartSpkWnd_Shape.move_vt_track(m_spkformtagright, m_psC->k_right);
			m_pSpkList->m_imaxmin2SL = m_psC->k_right;
		}
		UpdateData(FALSE);
	}
}
