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
		save_current_spk_file(); 
}

void ViewSpikeTemplates::DoDataExchange(CDataExchange* pDX)
{
	dbTableView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_SHAPE_T1, m_t1);
	DDX_Text(pDX, IDC_SHAPE_T2, m_t2);
	DDX_Text(pDX, IDC_TIMEFIRST, time_first);
	DDX_Text(pDX, IDC_TIMELAST, time_last);
	DDX_Text(pDX, IDC_HITRATE, hit_rate);
	DDX_Text(pDX, IDC_TOLERANCE, k_tolerance);
	DDX_Text(pDX, IDC_EDIT2, spike_no_class);
	DDX_Text(pDX, IDC_HITRATE2, hit_rate_sort);
	DDX_Text(pDX, IDC_IFIRSTSORTEDCLASS, i_first_sorted_class);
	DDX_Check(pDX, IDC_CHECK1, m_b_all_files);
	DDX_Control(pDX, IDC_TAB1, m_tab1_ctrl);
	DDX_Control(pDX, IDC_TAB2, m_tabCtrl);
	DDX_Check(pDX, IDC_DISPLAY_SINGLE_CLASS, b_display_single_class);
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
	ON_COMMAND(ID_FORMAT_VIEW_ALL_DATA_ON_ABSCISSA, &ViewSpikeTemplates::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_GAIN_ADJUST, &ViewSpikeTemplates::OnFormatGainadjust)
	ON_COMMAND(ID_FORMAT_CENTER_CURVE, &ViewSpikeTemplates::OnFormatCentercurve)
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
	ON_BN_CLICKED(IDC_DISPLAY_SINGLE_CLASS, &ViewSpikeTemplates::OnBnClickedDisplaysingleclass)
	ON_EN_CHANGE(IDC_SHAPE_T1, &ViewSpikeTemplates::OnEnChangeT1)
	ON_EN_CHANGE(IDC_SHAPE_T2, &ViewSpikeTemplates::OnEnChangeT2)
END_MESSAGE_MAP()

void ViewSpikeTemplates::OnDestroy()
{
	if (m_template_list_.GetNtemplates() != 0)
	{
		if (spike_classification_parameters_->p_template == nullptr)
			spike_classification_parameters_->create_tpl();
		*static_cast<CTemplateListWnd*>(spike_classification_parameters_->p_template) = m_template_list_;
	}
	dbTableView::OnDestroy();
}

BOOL ViewSpikeTemplates::OnMove(UINT nIDMoveCommand)
{
	save_current_spk_file();
	return dbTableView::OnMove(nIDMoveCommand);
}

void ViewSpikeTemplates::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_b_init_)
	{
		switch (LOWORD(lHint))
		{
		case HINT_DOC_HAS_CHANGED: 
		case HINT_DOC_MOVE_RECORD:
			update_file_parameters();
			break;
		case HINT_CLOSE_FILE_MODIFIED:
			save_current_spk_file();
			break;
		case HINT_REPLACE_VIEW:
		default:
			break;
		}
	}
}

void ViewSpikeTemplates::define_sub_classed_items()
{
	VERIFY(m_chart_spk_wnd_shape_.SubclassDlgItem(IDC_CHART_SHAPE, this));
	VERIFY(mm_spike_no_class_.SubclassDlgItem(IDC_EDIT2, this));
	mm_spike_no_class_.ShowScrollBar(SB_VERT);
	VERIFY(mm_time_first_.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_time_last_.SubclassDlgItem(IDC_TIMELAST, this));
	static_cast<CScrollBar*>(GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100, FALSE);
	VERIFY(m_template_list_.SubclassDlgItem(IDC_LIST2, this));
	VERIFY(m_avg_list_.SubclassDlgItem(IDC_LIST1, this));
	VERIFY(m_avg_all_list_.SubclassDlgItem(IDC_LIST3, this));

	VERIFY(mm_t1_.SubclassDlgItem(IDC_SHAPE_T1, this));
	VERIFY(mm_t2_.SubclassDlgItem(IDC_SHAPE_T2, this));

	VERIFY(mm_hit_rate_.SubclassDlgItem(IDC_HITRATE, this));
	mm_hit_rate_.ShowScrollBar(SB_VERT);
	VERIFY(mm_hit_rate_sort_.SubclassDlgItem(IDC_HITRATE2, this));
	mm_hit_rate_sort_.ShowScrollBar(SB_VERT);
	VERIFY(mm_k_tolerance_.SubclassDlgItem(IDC_TOLERANCE, this));
	mm_k_tolerance_.ShowScrollBar(SB_VERT);
	VERIFY(mm_i_first_sorted_class_.SubclassDlgItem(IDC_IFIRSTSORTEDCLASS, this));
	mm_i_first_sorted_class_.ShowScrollBar(SB_VERT);
}

void ViewSpikeTemplates::define_stretch_parameters()
{
	m_stretch_.AttachParent(this); // attach form_view pointer
	m_stretch_.newProp(IDC_LIST1, SZEQ_XLEQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_LIST2, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_LIST3, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_TAB2, XLEQ_XREQ, SZEQ_YBEQ);
}

void ViewSpikeTemplates::OnInitialUpdate()
{
	dbTableView::OnInitialUpdate();
	define_sub_classed_items();
	define_stretch_parameters();
	m_b_init_ = TRUE;
	m_auto_increment = true;
	m_auto_detect = true;

	// load global parameters
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	options_view_data_ = &(p_app->options_view_data);
	options_view_data_measure_ = &(p_app->options_view_data_measure);
	spike_classification_parameters_ = &(p_app->spk_classification);
	if (spike_classification_parameters_->p_template != nullptr)
		m_template_list_ = *static_cast<CTemplateListWnd*>(spike_classification_parameters_->p_template);

	// set ctrlTab values and extend its size
	CString cs = _T("Create");
	m_tab1_ctrl.InsertItem(0, cs);
	cs = _T("Sort");
	m_tab1_ctrl.InsertItem(1, cs);
	cs = _T("Display");
	m_tab1_ctrl.InsertItem(2, cs);
	CRect rect;
	m_tab1_ctrl.GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	rect.bottom += 200;
	m_tab1_ctrl.MoveWindow(&rect, TRUE);

	hit_rate = spike_classification_parameters_->hit_rate;
	hit_rate_sort = spike_classification_parameters_->hit_rate_sort;
	k_tolerance = spike_classification_parameters_->k_tolerance;

	m_chart_spk_wnd_shape_.set_plot_mode(PLOT_ONE_CLASS, 0);
	spk_form_tag_left_ = m_chart_spk_wnd_shape_.vt_tags.add_tag(spike_classification_parameters_->k_left, 0);
	spk_form_tag_right_ = m_chart_spk_wnd_shape_.vt_tags.add_tag(spike_classification_parameters_->k_right, 0);

	update_file_parameters();
	update_ctrl_tab1(0);
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
		const int index_current_spike_list = GetDocument()->get_current_spike_file()->get_spike_list_current_index();
		m_pSpkList = m_pSpkDoc->set_spike_list_current_index(index_current_spike_list);

		// update Tab at the bottom
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);
		m_tabCtrl.SetCurSel(index_current_spike_list);
	}
}

void ViewSpikeTemplates::select_spike_list(const int index_current)
{
	m_pSpkList = m_pSpkDoc->set_spike_list_current_index(index_current);
	m_tabCtrl.SetCurSel(index_current);

	if (!m_pSpkList->is_class_list_valid())
	{
		m_pSpkList->update_class_list();
		m_pSpkDoc->SetModifiedFlag();
	}

	// change pointer to select new spike list & test if one spike is selected
	int spike_index = m_pSpkList->m_selected_spike;
	if (spike_index > m_pSpkList->get_spikes_count() - 1 || spike_index < 0)
		spike_index = -1;
	else
	{
		// set source class to the class of the selected spike
		spike_no_class = m_pSpkList->get_spike(spike_index)->get_class_id();
		spike_classification_parameters_->source_class = spike_no_class;
	}

	ASSERT(spike_no_class < 32768);
	if (spike_no_class > 32768)
		spike_no_class = 0;

	// prepare display source spikes
	m_chart_spk_wnd_shape_.set_source_data(m_pSpkList, GetDocument());
	if (spike_classification_parameters_->k_left == 0 && spike_classification_parameters_->k_right == 0)
	{
		spike_classification_parameters_->k_left = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
		spike_classification_parameters_->k_right = spike_classification_parameters_->k_left + m_pSpkList->get_detection_parameters()->detect_refractory_period;
	}
	m_t1 = convert_spike_index_to_time(spike_classification_parameters_->k_left);
	m_t2 = convert_spike_index_to_time(spike_classification_parameters_->k_right);

	if (!b_display_single_class)
		m_chart_spk_wnd_shape_.set_plot_mode(PLOT_BLACK, 0);
	else
		m_chart_spk_wnd_shape_.set_plot_mode(PLOT_ONE_CLASS, spike_no_class);

	l_first_ = 0;
	l_last_ = m_pSpkDoc->get_acq_size() - 1;
	scroll_file_pos_infos_.nMin = 0;
	scroll_file_pos_infos_.nMax = l_last_;
	m_chart_spk_wnd_shape_.set_time_intervals(l_first_, l_last_);
	m_chart_spk_wnd_shape_.Invalidate();

	db_spike spike_sel(-1, -1, spike_index);
	select_spike(spike_sel);
	update_legends();

	display_avg(FALSE, &m_avg_list_);
	update_templates();
}

void ViewSpikeTemplates::update_templates()
{
	auto n_cmd_show = SW_HIDE;
	if (m_template_list_.GetNtemplates() > 0)
	{
		if (m_template_list_.GetImageList(LVSIL_NORMAL) != &m_template_list_.m_imageList)
		{
			CRect rect;
			m_chart_spk_wnd_shape_.GetClientRect(&rect);
			m_template_list_.m_imageList.Create(rect.right, rect.bottom, ILC_COLOR8, 4, 1);
			m_template_list_.SetImageList(&m_template_list_.m_imageList, LVSIL_NORMAL);
		}
		SetDlgItemInt(IDC_NTEMPLATES, m_template_list_.GetNtemplates());
		const int extent = m_chart_spk_wnd_shape_.get_yw_extent();
		const int zero = m_chart_spk_wnd_shape_.get_yw_org();
		m_template_list_.SetYWExtOrg(extent, zero);
		m_template_list_.UpdateTemplateLegends("t");
		m_template_list_.Invalidate();
		n_cmd_show = SW_SHOW;
	}
	GetDlgItem(IDC_NTEMPLS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_NTEMPLATES)->ShowWindow(n_cmd_show);
}

void ViewSpikeTemplates::update_legends()
{
	if (l_first_ < 0)
		l_first_ = 0;
	if (l_last_ <= l_first_)
		l_last_ = l_first_ + 120;
	if (l_last_ >= m_pSpkDoc->get_acq_size())
		l_last_ = m_pSpkDoc->get_acq_size() - 1;
	if (l_first_ > l_last_)
		l_first_ = l_last_ - 120;

	time_first = static_cast<float>(l_first_) / m_pSpkDoc->get_acq_rate();
	time_last = static_cast<float>(l_last_ + 1) / m_pSpkDoc->get_acq_rate();

	m_chart_spk_wnd_shape_.set_time_intervals(l_first_, l_last_);
	m_chart_spk_wnd_shape_.Invalidate();

	UpdateData(FALSE);
	update_scrollbar();
}

void ViewSpikeTemplates::select_spike(db_spike& spike_sel)
{
	const CdbWaveDoc* p_doc = m_chart_spk_wnd_shape_.get_db_wave_doc();
	spike_sel.database_position = p_doc->db_get_current_record_position();
	spike_sel.spike_list_index = p_doc->m_p_spk->get_spike_list_current_index();

	m_chart_spk_wnd_shape_.select_spike(spike_sel);
	spike_no_ = spike_sel.spike_index;
	m_pSpkList->m_selected_spike = spike_no_;
}

LRESULT ViewSpikeTemplates::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	short shortValue = LOWORD(lParam);
	switch (wParam)
	{
	case HINT_SET_MOUSE_CURSOR:
		if (shortValue > CURSOR_ZOOM)
			shortValue = 0;
		set_view_mouse_cursor(shortValue);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(shortValue, 0));
		break;

	case HINT_HIT_SPIKE:
		{
			db_spike spike_hit = GetDocument()->get_spike_hit();
			select_spike(spike_hit); 
		}
		break;

	case HINT_CHANGE_VERT_TAG:
		if (shortValue == spk_form_tag_left_)
		{
			spike_classification_parameters_->k_left = m_chart_spk_wnd_shape_.vt_tags.get_value_int(spk_form_tag_left_);
			m_t1 = convert_spike_index_to_time(spike_classification_parameters_->k_left);
			mm_t1_.m_bEntryDone = TRUE;
			OnEnChangeT1();
		}
		else if (shortValue == spk_form_tag_right_)
		{
			spike_classification_parameters_->k_right = m_chart_spk_wnd_shape_.vt_tags.get_value_int(spk_form_tag_right_);
			m_t2 = convert_spike_index_to_time(spike_classification_parameters_->k_right);
			mm_t2_.m_bEntryDone = TRUE;
			OnEnChangeT2();
		}
		m_template_list_.SetTemplateLength(0, spike_classification_parameters_->k_left, spike_classification_parameters_->k_right);
		m_template_list_.Invalidate();
		break;

	case HINT_CHANGE_HZ_LIMITS:
	case HINT_CHANGE_ZOOM:
	case HINT_VIEW_SIZE_CHANGED:
		set_extent_zero_all_display(m_chart_spk_wnd_shape_.get_yw_extent(), m_chart_spk_wnd_shape_.get_yw_org());
		update_legends();
		break;

	case HINT_R_MOUSE_BUTTON_DOWN:
		edit_spike_class(HIWORD(lParam), shortValue);
		break;
	case HINT_VIEW_TAB_HAS_CHANGED:
		update_ctrl_tab1(shortValue);
		break;
	default:
		break;
	}
	return 0L;
}

void ViewSpikeTemplates::OnEnChangeclassno()
{
	if (mm_spike_no_class_.m_bEntryDone)
	{
		const auto spike_class = spike_no_class;
		mm_spike_no_class_.OnEnChange(this, spike_no_class, 1, -1);

		if (spike_no_class != spike_class) // change display if necessary
		{
			m_chart_spk_wnd_shape_.set_plot_mode(PLOT_ONE_CLASS, spike_no_class);
			m_chart_spk_wnd_shape_.Invalidate();
			update_legends();
		}
	}
}

void ViewSpikeTemplates::OnEnChangeTimefirst()
{
	if (mm_time_first_.m_bEntryDone)
	{
		mm_time_first_.OnEnChange(this, time_first, 1.f, -1.f);

		const auto l_first = static_cast<long>(time_first * m_pSpkDoc->get_acq_rate());
		if (l_first != l_first_)
		{
			l_first_ = l_first;
			update_legends();
		}
	}
}

void ViewSpikeTemplates::OnEnChangeTimelast()
{
	if (mm_time_last_.m_bEntryDone)
	{
		mm_time_last_.OnEnChange(this, time_last, 1.f, -1.f);

		const auto l_last = static_cast<long>(time_last * m_pSpkDoc->get_acq_rate());
		if (l_last != l_last_)
		{
			l_last_ = l_last;
			update_legends();
		}
	}
}

void ViewSpikeTemplates::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// form_view scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		dbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// get corresponding data
	const auto total_scroll = m_pSpkDoc->get_acq_size();
	const long page_scroll = (l_last_ - l_first_);
	auto sb_scroll = page_scroll / 10;
	if (sb_scroll == 0)
		sb_scroll = 1;
	long l_first = l_first_;
	switch (nSBCode)
	{
	case SB_LEFT: l_first = 0; break; 
	case SB_LINELEFT: l_first -= sb_scroll; break;
	case SB_LINERIGHT: l_first += sb_scroll; break; 
	case SB_PAGELEFT: l_first -= page_scroll; break; 
	case SB_PAGERIGHT: l_first += page_scroll; break; 
	case SB_RIGHT: l_first = total_scroll - page_scroll + 1; break;
	case SB_THUMBPOSITION: 
	case SB_THUMBTRACK: 
		l_first = static_cast<int>(nPos); break;
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
	if (l_first != l_first_)
	{
		l_first_ = l_first;
		l_last_ = l_last;
		update_legends();
	}
	else
		update_scrollbar();
}

void ViewSpikeTemplates::update_scrollbar()
{
	if (l_first_ == 0 && l_last_ >= m_pSpkDoc->get_acq_size() - 1)
		GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_HIDE);
	else
	{
		GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_SHOW);
		scroll_file_pos_infos_.fMask = SIF_ALL;
		scroll_file_pos_infos_.nPos = l_first_;
		scroll_file_pos_infos_.nPage = l_last_ - l_first_;
		static_cast<CScrollBar*>(GetDlgItem(IDC_SCROLLBAR1))->SetScrollInfo(&scroll_file_pos_infos_);
	}
}

void ViewSpikeTemplates::OnFormatAlldata()
{
	// dots: spk file length
	l_first_ = 0;
	l_last_ = m_pSpkDoc->get_acq_size() - 1;
	// spikes: center spikes horizontally and adjust hz size of display
	constexpr short x_wo = 0;
	const short x_we = static_cast<short>(m_pSpkList->get_spike_length());
	m_chart_spk_wnd_shape_.set_xw_ext_org(x_we, x_wo);
	update_legends();
}

void ViewSpikeTemplates::OnFormatGainadjust()
{
	short maxvalue, minvalue;
	GetDocument()->get_max_min_amplitude_of_all_spikes(m_b_all_files, TRUE, maxvalue, minvalue);
	const auto extent = MulDiv(maxvalue - minvalue + 1, 10, 9);
	const auto zero = (maxvalue + minvalue) / 2;

	set_extent_zero_all_display(extent, zero);
}

void ViewSpikeTemplates::OnFormatCentercurve()
{
	short maxvalue, minvalue;
	GetDocument()->get_max_min_amplitude_of_all_spikes(m_b_all_files, TRUE, maxvalue, minvalue);
	const auto extent = m_chart_spk_wnd_shape_.get_yw_extent();
	const auto zero = (maxvalue + minvalue) / 2;
	set_extent_zero_all_display(extent, zero);
}

void ViewSpikeTemplates::set_extent_zero_all_display(const int extent, const int zero)
{
	m_chart_spk_wnd_shape_.set_yw_ext_org(extent, zero);
	m_template_list_.SetYWExtOrg(extent, zero);
	m_avg_list_.SetYWExtOrg(extent, zero);
	m_avg_all_list_.SetYWExtOrg(extent, zero);

	m_chart_spk_wnd_shape_.Invalidate();
	m_template_list_.Invalidate();
	m_avg_list_.Invalidate();
	m_avg_all_list_.Invalidate();
}

void ViewSpikeTemplates::OnEnChangeHitrate()
{
	if (mm_hit_rate_.m_bEntryDone)
	{
		mm_hit_rate_.OnEnChange(this, hit_rate, 1, -1);

		if (spike_classification_parameters_->hit_rate != hit_rate)
			spike_classification_parameters_->hit_rate = hit_rate;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::OnEnChangeHitrateSort()
{
	if (mm_hit_rate_sort_.m_bEntryDone)
	{
		mm_hit_rate_sort_.OnEnChange(this, hit_rate_sort, 1, -1);

		if (spike_classification_parameters_->hit_rate_sort != hit_rate_sort)
			spike_classification_parameters_->hit_rate_sort = hit_rate_sort;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::OnEnChangeTolerance()
{
	if (mm_k_tolerance_.m_bEntryDone)
	{
		mm_k_tolerance_.OnEnChange(this, k_tolerance, 1.f, -1.f);
		if (k_tolerance < 0)
			k_tolerance = -k_tolerance;
		if (spike_classification_parameters_->k_tolerance != k_tolerance)
			spike_classification_parameters_->k_tolerance = k_tolerance;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::display_avg(const boolean b_all_files, CTemplateListWnd* template_list) //, CImageList* pImList)
{
	m_pSpkList = m_pSpkDoc->get_spike_list_current();

	// get list of classes
	template_list->SetHitRate_Tolerance(&hit_rate, &k_tolerance);
	int tpllen = spike_classification_parameters_->k_right - spike_classification_parameters_->k_left + 1;

	// define and attach to ImageList to CListCtrl; create 1 item by default
	if (template_list->GetImageList(LVSIL_NORMAL) != &template_list->m_imageList)
	{
		CRect rect;
		m_chart_spk_wnd_shape_.GetClientRect(&rect);
		template_list->m_imageList.Create(rect.right, rect.bottom, ILC_COLOR8, 1, 1);
		template_list->SetImageList(&template_list->m_imageList, LVSIL_NORMAL);
	}

	// reinit all templates to zero
	template_list->DeleteAllItems();
	int spikelen = m_pSpkList->get_spike_length();
	template_list->SetTemplateLength(spikelen, 0, spikelen - 1);
	template_list->SetHitRate_Tolerance(&hit_rate, &k_tolerance);

	int zero = m_chart_spk_wnd_shape_.get_yw_org();
	int extent = m_chart_spk_wnd_shape_.get_yw_extent();
	if (zero == 0 && extent == 0)
	{
		short valuemax, valuemin;
		m_pSpkList->get_total_max_min(TRUE, &valuemax, &valuemin);
		extent = valuemax - valuemin;
		zero = (valuemax + valuemin) / 2;
		m_chart_spk_wnd_shape_.set_yw_ext_org(extent, zero);
	}
	template_list->SetYWExtOrg(extent, zero);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int current_file = p_dbwave_doc->db_get_current_record_position(); // index current file
	auto first_file = current_file; // index first file in the series
	auto last_file = current_file; // index last file in the series
	// make sure we have the correct spike list here
	const auto current_list = m_tabCtrl.GetCurSel();
	m_pSpkDoc->set_spike_list_current_index(current_list);

	CString cs_comment;
	CString cs_file_comment = _T("Analyze file: ");
	if (b_all_files)
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

		auto pSpkList = pSpkDoc->set_spike_list_current_index(current_list); // load pointer to spike list
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
			for (j_templ = 0; j_templ < template_list->GetTemplateDataSize(); j_templ++)
			{
				if (cla == template_list->GetTemplateclassID(j_templ))
				{
					b_found = TRUE;
					break;
				}
				if (cla < template_list->GetTemplateclassID(j_templ))
					break;
			}
			// add template if not found - insert it at the proper place
			if (!b_found) // add item if not found
			{
				if (j_templ < 0)
					j_templ = 0;
				j_templ = template_list->InsertTemplateData(j_templ, cla);
			}

			// get data and add spike
			const auto p_spik = pSpkList->get_spike(i)->get_p_data();
			template_list->tAdd(j_templ, p_spik); // add spike to template j
			template_list->tAdd(p_spik); // add spike to template zero
		}
	}

	// end of loop, select current file again if necessary
	if (b_all_files)
	{
		p_dbwave_doc->db_set_current_record_position(current_file);
		m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update average
	template_list->TransferTemplateData();
	template_list->tGlobalstats();
	template_list->UpdateTemplateLegends("cx");
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
	if (m_b_all_files)
	{
		firstfile = 0;
		lastfile = p_dbwave_doc->db_get_n_records() - 1;
	}

	// add as many forms as we have classes
	m_template_list_.DeleteAllItems(); // reinit all templates to zero
	m_template_list_.SetTemplateLength(m_pSpkList->get_spike_length(), spike_classification_parameters_->k_left, spike_classification_parameters_->k_right);
	m_template_list_.SetHitRate_Tolerance(&hit_rate, &k_tolerance);

	// compute global std
	// loop over all selected files (or only one file currently selected)
	int nspikes;
	int ifile;
	CString cscomment;

	for (ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_b_all_files)
		{
			p_dbwave_doc->db_set_current_record_position(ifile);
			m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
		}

		const auto spike_list = m_pSpkDoc->set_spike_list_current_index(currentlist);
		nspikes = spike_list->get_spikes_count();
		for (auto i = 0; i < nspikes; i++)
			m_template_list_.tAdd(m_pSpkList->get_spike(i)->get_p_data());
	}
	m_template_list_.tGlobalstats();

	// now scan all spikes to build templates
	auto ntempl = 0;
	double distmin;
	int offsetmin;
	int tplmin;
	CString csfilecomment = _T("Second pass - analyze file: ");

	for (ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_b_all_files)
		{
			p_dbwave_doc->db_set_current_record_position(ifile);
			p_dbwave_doc->open_current_spike_file();
			m_pSpkDoc = p_dbwave_doc->m_p_spk;
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->db_get_current_spk_file_name(FALSE);
			p_dbwave_doc->SetTitle(cs);
		}

		auto spike_list = m_pSpkDoc->set_spike_list_current_index(currentlist);
		nspikes = spike_list->get_spikes_count();

		// create template CListCtrl
		for (auto i = 0; i < nspikes; i++)
		{
			// filter out undesirable spikes
			if (b_display_single_class)
			{
				if (m_pSpkList->get_spike(i)->get_class_id() != spike_no_class)
					continue;
			}
			const auto ii_time = m_pSpkList->get_spike(i)->get_time();
			if (ii_time < l_first_ || ii_time > l_last_)
				continue;

			// get pointer to spike data and search if any template is suitable
			auto* p_spik = m_pSpkList->get_spike(i)->get_p_data();
			auto b_within = FALSE;
			int itpl;
			for (itpl = 0; itpl < ntempl; itpl++)
			{
				// exit loop if spike is within template
				b_within = m_template_list_.tWithin(itpl, p_spik);
				if (b_within)
					break;
				// OR exit loop if spike dist is less distant
				m_template_list_.tMinDist(itpl, p_spik, &offsetmin, &distmin);
				b_within = (distmin <= m_template_list_.m_globaldist);
				if (b_within)
					break;
			}

			// if a template is suitable, find the most likely
			if (b_within)
			{
				tplmin = itpl;
				distmin = m_template_list_.m_globaldist;
				double x;
				int offset;
				for (auto itpl2 = 0; itpl2 < ntempl; itpl2++)
				{
					m_template_list_.tMinDist(itpl2, p_spik, &offset, &x);
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
				m_template_list_.InsertTemplate(ntempl, ntempl + i_first_sorted_class);
				tplmin = ntempl;
				ntempl++;
			}

			// add spike to the corresp template
			m_template_list_.tAdd(tplmin, p_spik); // add spike to template j
		}
	}

	// end of loop, select current file again if necessary
	if (m_b_all_files)
	{
		p_dbwave_doc->db_set_current_record_position(currentfile);
		m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	m_template_list_.SortTemplatesByNumberofSpikes(TRUE, TRUE, i_first_sorted_class);
	update_templates();
}

void ViewSpikeTemplates::sort_spikes()
{
	// set tolerance to sort tolerance
	m_template_list_.SetHitRate_Tolerance(&hit_rate_sort, &k_tolerance);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->db_get_current_record_position(); // index current file
	auto firstfile = currentfile; // index first file in the series
	auto lastfile = firstfile; // index last file in the series
	const auto currentlist = m_tabCtrl.GetCurSel();

	// change indexes if ALL files selected
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");
	if (m_b_all_files)
	{
		firstfile = 0; // index first file
		lastfile = p_dbwave_doc->db_get_n_records() - 1; // index last file
	}

	// loop CFrameWnd
	const auto ntempl = m_template_list_.GetNtemplates();
	for (auto ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		if (m_b_all_files)
		{
			p_dbwave_doc->db_set_current_record_position(ifile);
			m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
			CString cs;
			cs.Format(_T("%i/%i - "), ifile, lastfile);
			cs += p_dbwave_doc->db_get_current_spk_file_name(FALSE);
			p_dbwave_doc->SetTitle(cs);
			m_pSpkDoc->SetModifiedFlag(FALSE);

			m_pSpkList = m_pSpkDoc->set_spike_list_current_index(currentlist); // load pointer to spike list
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
			if (b_display_single_class)
			{
				// skip spikes that do not belong to selected class
				if (m_pSpkList->get_spike(ispike)->get_class_id() != spike_no_class)
					continue;
			}

			// skip spikes that do not fit into time interval selected
			const auto ii_time = m_pSpkList->get_spike(ispike)->get_time();
			if (ii_time < l_first_ || ii_time > l_last_)
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
				b_within = m_template_list_.tWithin(tplmin, p_spik);
				m_template_list_.tMinDist(tplmin, p_spik, &offsetmin, &distmin);
				if (b_within)
					break;

				// OR exit loop if spike dist is less distant
				b_within = (distmin <= m_template_list_.m_globaldist);
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
					m_template_list_.tMinDist(itpl, p_spik, &offset, &x);
					if (x < distmin)
					{
						offsetmin = offset;
						distmin = x;
						tplmin = itpl;
					}
				}

				// change spike class ID
				const auto class_id = (m_template_list_.GetTemplateWnd(tplmin))->m_classID;
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
	if (m_b_all_files)
	{
		p_dbwave_doc->db_set_current_record_position(currentfile);
		m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update display: average and spk form
	display_avg(FALSE, &m_avg_list_);
	m_pSpkList = m_pSpkDoc->get_spike_list_current();
	m_chart_spk_wnd_shape_.set_source_data(m_pSpkList, GetDocument());
	m_chart_spk_wnd_shape_.Invalidate();
}

void ViewSpikeTemplates::OnKeydownTemplateList(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto* p_lv_key_dow = reinterpret_cast<LV_KEYDOWN*>(pNMHDR);

	// delete selected template
	if (p_lv_key_dow->wVKey == VK_DELETE && m_template_list_.GetSelectedCount() > 0)
	{
		auto flag = FALSE;
		const auto isup = m_template_list_.GetItemCount();
		auto cla = i_first_sorted_class;
		for (auto i = 0; i < isup; i++)
		{
			const auto state = m_template_list_.GetItemState(i, LVIS_SELECTED);
			if (state > 0)
			{
				cla = m_template_list_.GetTemplateclassID(i);
				m_template_list_.DeleteItem(i);
				flag = TRUE;
				continue;
			}
			if (flag)
			{
				m_template_list_.SetTemplateclassID(i - 1, _T("t"), cla);
				cla++;
			}
		}
	}
	SetDlgItemInt(IDC_NTEMPLATES, m_template_list_.GetItemCount());
	m_template_list_.Invalidate();
	*pResult = 0;
}

void ViewSpikeTemplates::OnCheck1()
{
	UpdateData(TRUE);
}

void ViewSpikeTemplates::edit_spike_class(int control_id, int control_item)
{
	// find which item has been selected
	CTemplateListWnd* pList = nullptr;
	auto b_spikes = TRUE;
	auto b_all_files = m_b_all_files;
	if (m_avg_list_.GetDlgCtrlID() == control_id)
		pList = &m_avg_list_;
	else if (m_template_list_.GetDlgCtrlID() == control_id)
	{
		pList = &m_template_list_;
		b_spikes = FALSE;
	}
	else if (m_avg_all_list_.GetDlgCtrlID() == control_id)
	{
		pList = &m_avg_all_list_;
		b_all_files = TRUE;
	}
	if (pList == nullptr)
		return;

	// find which icon has been selected and get the key
	const auto oldclass = pList->GetTemplateclassID(control_item);

	// launch edit dlg
	DlgEditSpikeClass dlg;
	dlg.m_iClass = oldclass;
	if (IDOK == dlg.DoModal() && oldclass != dlg.m_iClass)
	{
		// templates
		if (!b_spikes)
			pList->SetTemplateclassID(control_item, _T("t"), dlg.m_iClass);
		// spikes
		else
		{
			pList->SetTemplateclassID(control_item, _T("c"), dlg.m_iClass);

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
					m_pSpkList = m_pSpkDoc->set_spike_list_current_index(current_list);
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

void ViewSpikeTemplates::update_ctrl_tab1(int i_select)
{
	WORD i_templ = SW_SHOW;
	WORD i_avg = SW_HIDE;
	WORD i_sort = SW_HIDE;
	switch (i_select)
	{
	case 0:
		m_avg_all_list_.ShowWindow(SW_HIDE);
		m_template_list_.ShowWindow(SW_SHOW);
		break;
	case 1:
		i_templ = SW_HIDE;
		i_sort = SW_SHOW;
		i_avg = SW_HIDE;
		m_avg_all_list_.ShowWindow(SW_HIDE);
		m_template_list_.ShowWindow(SW_SHOW);
		break;
	case 2:
		i_templ = SW_HIDE;
		i_sort = SW_HIDE;
		i_avg = SW_SHOW;
		m_avg_all_list_.ShowWindow(SW_SHOW);
		m_template_list_.ShowWindow(SW_HIDE);
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
	sort_spikes();
}

void ViewSpikeTemplates::OnBnClickedDisplay()
{
	display_avg(TRUE, &m_avg_all_list_); //, &m_ImListAll);
}

void ViewSpikeTemplates::OnEnChangeIfirstsortedclass()
{
	if (mm_i_first_sorted_class_.m_bEntryDone)
	{
		mm_i_first_sorted_class_.OnEnChange(this, i_first_sorted_class, 1, -1);

		// change class of all templates
		//SetTemplateclassID(int item, LPCSTR pszType, int classID)
		m_template_list_.UpdateTemplateBaseClassID(i_first_sorted_class);
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
	if (b_display_single_class)
	{
		GetDlgItem(IDC_EDIT2)->ShowWindow(SW_SHOW);
		m_chart_spk_wnd_shape_.set_plot_mode(PLOT_ONE_CLASS, spike_no_class);
	}
	else
	{
		GetDlgItem(IDC_EDIT2)->ShowWindow(SW_HIDE);
		m_chart_spk_wnd_shape_.set_plot_mode(PLOT_BLACK, spike_no_class);
	}
	m_chart_spk_wnd_shape_.Invalidate();
}

void ViewSpikeTemplates::OnEnChangeT1()
{
	m_pSpkList = m_pSpkDoc->get_spike_list_current();

	if (mm_t1_.m_bEntryDone)
	{
		const auto delta = t_unit / m_pSpkList->get_acq_sampling_rate();
		
		mm_t1_.OnEnChange(this, m_t1, delta, -delta);
		// check boundaries
		if (m_t1 < 0)
			m_t1 = 0.0f;
		if (m_t1 >= m_t2)
			m_t1 = m_t2 - delta;

		const int it1 = convert_time_to_spike_index(m_t1);
		if (it1 != m_chart_spk_wnd_shape_.vt_tags.get_value_int(spk_form_tag_left_))
		{
			spike_classification_parameters_->k_left = it1;
			m_chart_spk_wnd_shape_.move_vt_track(spk_form_tag_left_, spike_classification_parameters_->k_left);
			m_pSpkList->shape_t1 = spike_classification_parameters_->k_left;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::OnEnChangeT2()
{
	m_pSpkList = m_pSpkDoc->get_spike_list_current();

	if (mm_t2_.m_bEntryDone)
	{
		const auto delta = t_unit / m_pSpkList->get_acq_sampling_rate();
		mm_t2_.OnEnChange(this, m_t2, delta, -delta);

		// check boundaries
		if (m_t2 < m_t1)
			m_t2 = m_t1 + delta;

		const int spike_length = m_pSpkList->get_spike_length();
		const auto t_max = convert_spike_index_to_time(spike_length - 1);
		if (m_t2 >= t_max)
			m_t2 = t_max;
		// change display if necessary
		const int it2 = convert_time_to_spike_index(m_t2);
		if (it2 != m_chart_spk_wnd_shape_.vt_tags.get_value_int(spk_form_tag_right_))
		{
			spike_classification_parameters_->k_right = it2;
			m_chart_spk_wnd_shape_.move_vt_track(spk_form_tag_right_, spike_classification_parameters_->k_right);
			m_pSpkList->shape_t2 = spike_classification_parameters_->k_right;
		}
		UpdateData(FALSE);
	}
}
