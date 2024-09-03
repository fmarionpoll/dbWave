#include "StdAfx.h"
#include "ViewSpikeTemplate.h"
#include "dbWave.h"
#include "DlgEditSpikeClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewSpikeTemplates, ViewDbTable)

ViewSpikeTemplates::ViewSpikeTemplates()
	: ViewDbTable(IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

ViewSpikeTemplates::~ViewSpikeTemplates()
{
	if (p_spk_doc != nullptr)
		save_current_spk_file(); 
}

void ViewSpikeTemplates::OnDestroy()
{
	if (m_template_list_.get_n_templates() != 0)
	{
		if (spike_classification_parameters_->p_template == nullptr)
			spike_classification_parameters_->create_tpl();
		*static_cast<CTemplateListWnd*>(spike_classification_parameters_->p_template) = m_template_list_;
	}
	ViewDbTable::OnDestroy();
}

void ViewSpikeTemplates::DoDataExchange(CDataExchange* pDX)
{
	ViewDbTable::DoDataExchange(pDX);

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
	DDX_Control(pDX, IDC_TAB2, spk_list_tab_ctrl);
	DDX_Check(pDX, IDC_DISPLAY_SINGLE_CLASS, b_display_single_class);
}

BEGIN_MESSAGE_MAP(ViewSpikeTemplates, ViewDbTable)

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikeTemplates::on_my_message)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_HSCROLL()

	ON_EN_CHANGE(IDC_EDIT2, &ViewSpikeTemplates::on_en_change_class)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewSpikeTemplates::on_en_change_time_first)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewSpikeTemplates::on_en_change_time_last)
	ON_COMMAND(ID_FORMAT_VIEW_ALL_DATA_ON_ABSCISSA, &ViewSpikeTemplates::on_format_all_data)
	ON_COMMAND(ID_FORMAT_GAIN_ADJUST, &ViewSpikeTemplates::on_format_gain_adjust)
	ON_COMMAND(ID_FORMAT_CENTER_CURVE, &ViewSpikeTemplates::on_format_center_curve)
	ON_BN_CLICKED(IDC_BUILD, &ViewSpikeTemplates::on_build_templates)
	ON_EN_CHANGE(IDC_HITRATE, &ViewSpikeTemplates::on_en_change_hit_rate)
	ON_EN_CHANGE(IDC_TOLERANCE, &ViewSpikeTemplates::on_en_change_tolerance)
	ON_EN_CHANGE(IDC_HITRATE2, &ViewSpikeTemplates::on_en_change_hit_rate_sort)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST2, &ViewSpikeTemplates::on_keydown_template_list)
	ON_BN_CLICKED(IDC_CHECK1, &ViewSpikeTemplates::on_check1)
	ON_BN_CLICKED(IDC_SORT, &ViewSpikeTemplates::on_bn_clicked_sort)
	ON_BN_CLICKED(IDC_DISPLAY, &ViewSpikeTemplates::on_bn_clicked_display)
	ON_EN_CHANGE(IDC_IFIRSTSORTEDCLASS, &ViewSpikeTemplates::on_en_change_i_first_sorted_class)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB2, &ViewSpikeTemplates::on_tcn_sel_change_tab2)
	ON_NOTIFY(NM_CLICK, IDC_TAB2, &ViewSpikeTemplates::on_nm_click_tab2)
	ON_BN_CLICKED(IDC_DISPLAY_SINGLE_CLASS, &ViewSpikeTemplates::on_bn_clicked_display_single_class)
	ON_EN_CHANGE(IDC_SHAPE_T1, &ViewSpikeTemplates::on_en_change_t1)
	ON_EN_CHANGE(IDC_SHAPE_T2, &ViewSpikeTemplates::on_en_change_t2)
END_MESSAGE_MAP()

BOOL ViewSpikeTemplates::OnMove(UINT n_id_move_command)
{
	save_current_spk_file();
	return ViewDbTable::OnMove(n_id_move_command);
}

void ViewSpikeTemplates::OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint)
{
	if (b_init_)
	{
		switch (LOWORD(l_hint))
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
	stretch_.attach_parent(this); // attach form_view pointer
	stretch_.new_prop(IDC_LIST1, SZEQ_XLEQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_LIST2, XLEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_LIST3, XLEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_TAB2, XLEQ_XREQ, SZEQ_YBEQ);
}

void ViewSpikeTemplates::OnInitialUpdate()
{
	ViewDbTable::OnInitialUpdate();
	define_sub_classed_items();
	define_stretch_parameters();
	b_init_ = TRUE;
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
	const int index_current = p_spk_doc->get_index_current_spike_list();
	select_spike_list(index_current);
}

void ViewSpikeTemplates::update_spike_file()
{
	p_spk_doc = GetDocument()->open_current_spike_file();

	if (nullptr != p_spk_doc)
	{
		p_spk_doc->SetModifiedFlag(FALSE);
		p_spk_doc->SetPathName(GetDocument()->db_get_current_spk_file_name(), FALSE);
		const int index_current_spike_list = GetDocument()->get_current_spike_file()->get_index_current_spike_list();
		p_spk_list = p_spk_doc->set_index_current_spike_list(index_current_spike_list);

		// update Tab at the bottom
		spk_list_tab_ctrl.init_ctrl_tab_from_spike_doc(p_spk_doc);
		spk_list_tab_ctrl.SetCurSel(index_current_spike_list);
	}
}

void ViewSpikeTemplates::select_spike_list(const int index_current)
{
	p_spk_list = p_spk_doc->set_index_current_spike_list(index_current);
	spk_list_tab_ctrl.SetCurSel(index_current);

	if (!p_spk_list->is_class_list_valid())
	{
		p_spk_list->update_class_list();
		p_spk_doc->SetModifiedFlag();
	}

	// change pointer to select new spike list & test if one spike is selected
	int spike_index = p_spk_list->m_selected_spike;
	if (spike_index > p_spk_list->get_spikes_count() - 1 || spike_index < 0)
		spike_index = -1;
	else
	{
		// set source class to the class of the selected spike
		spike_no_class = p_spk_list->get_spike(spike_index)->get_class_id();
		spike_classification_parameters_->source_class = spike_no_class;
	}

	ASSERT(spike_no_class < 32768);
	if (spike_no_class > 32768)
		spike_no_class = 0;

	// prepare display source spikes
	m_chart_spk_wnd_shape_.set_source_data(p_spk_list, GetDocument());
	if (spike_classification_parameters_->k_left == 0 && spike_classification_parameters_->k_right == 0)
	{
		spike_classification_parameters_->k_left = p_spk_list->get_detection_parameters()->detect_pre_threshold;
		spike_classification_parameters_->k_right = spike_classification_parameters_->k_left + p_spk_list->get_detection_parameters()->detect_refractory_period;
	}
	m_t1 = convert_spike_index_to_time(spike_classification_parameters_->k_left);
	m_t2 = convert_spike_index_to_time(spike_classification_parameters_->k_right);

	if (!b_display_single_class)
		m_chart_spk_wnd_shape_.set_plot_mode(PLOT_BLACK, 0);
	else
		m_chart_spk_wnd_shape_.set_plot_mode(PLOT_ONE_CLASS, spike_no_class);

	l_first_ = 0;
	l_last_ = p_spk_doc->get_acq_size() - 1;
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
	if (m_template_list_.get_n_templates() > 0)
	{
		if (m_template_list_.GetImageList(LVSIL_NORMAL) != &m_template_list_.m_image_list)
		{
			CRect rect;
			m_chart_spk_wnd_shape_.GetClientRect(&rect);
			m_template_list_.m_image_list.Create(rect.right, rect.bottom, ILC_COLOR8, 4, 1);
			m_template_list_.SetImageList(&m_template_list_.m_image_list, LVSIL_NORMAL);
		}
		SetDlgItemInt(IDC_NTEMPLATES, m_template_list_.get_n_templates());
		const int extent = m_chart_spk_wnd_shape_.get_yw_extent();
		const int zero = m_chart_spk_wnd_shape_.get_yw_org();
		m_template_list_.set_y_w_ext_org(extent, zero);
		m_template_list_.update_template_legends("t");
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
	if (l_last_ >= p_spk_doc->get_acq_size())
		l_last_ = p_spk_doc->get_acq_size() - 1;
	if (l_first_ > l_last_)
		l_first_ = l_last_ - 120;

	time_first = static_cast<float>(l_first_) / p_spk_doc->get_acq_rate();
	time_last = static_cast<float>(l_last_ + 1) / p_spk_doc->get_acq_rate();

	m_chart_spk_wnd_shape_.set_time_intervals(l_first_, l_last_);
	m_chart_spk_wnd_shape_.Invalidate();

	UpdateData(FALSE);
	update_scrollbar();
}

void ViewSpikeTemplates::select_spike(db_spike& spike_sel)
{
	const CdbWaveDoc* p_doc = GetDocument();
	if (spike_sel.record_id < 0) 
	{
		spike_sel.record_id = p_doc->db_get_current_record_id();
		spike_sel.spike_list_index = p_doc->m_p_spk_doc->get_index_current_spike_list();
	}
	else if (spike_sel.record_id != p_doc->db_get_current_record_id())
	{
		if (p_doc->db_move_to_id(spike_sel.record_id))
		{
			;
		}
	}

	m_chart_spk_wnd_shape_.select_spike(spike_sel);
	spike_no_ = spike_sel.spike_index;
	p_spk_list->m_selected_spike = spike_no_;
}

LRESULT ViewSpikeTemplates::on_my_message(const WPARAM w_param, const LPARAM l_param)
{
	short short_value = LOWORD(l_param);
	switch (w_param)
	{
	case HINT_SET_MOUSE_CURSOR:
		if (short_value > CURSOR_ZOOM)
			short_value = 0;
		set_view_mouse_cursor(short_value);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(short_value, 0));
		break;

	case HINT_HIT_SPIKE:
		{
			db_spike spike_hit = GetDocument()->get_spike_hit();
			select_spike(spike_hit); 
		}
		break;

	case HINT_CHANGE_VERT_TAG:
		if (short_value == spk_form_tag_left_)
		{
			spike_classification_parameters_->k_left = m_chart_spk_wnd_shape_.vt_tags.get_value_int(spk_form_tag_left_);
			m_t1 = convert_spike_index_to_time(spike_classification_parameters_->k_left);
			mm_t1_.m_b_entry_done = TRUE;
			on_en_change_t1();
		}
		else if (short_value == spk_form_tag_right_)
		{
			spike_classification_parameters_->k_right = m_chart_spk_wnd_shape_.vt_tags.get_value_int(spk_form_tag_right_);
			m_t2 = convert_spike_index_to_time(spike_classification_parameters_->k_right);
			mm_t2_.m_b_entry_done = TRUE;
			on_en_change_t2();
		}
		m_template_list_.set_template_length(0, spike_classification_parameters_->k_left, spike_classification_parameters_->k_right);
		m_template_list_.Invalidate();
		break;

	case HINT_CHANGE_HZ_LIMITS:
	case HINT_CHANGE_ZOOM:
	case HINT_VIEW_SIZE_CHANGED:
		set_extent_zero_all_display(m_chart_spk_wnd_shape_.get_yw_extent(), m_chart_spk_wnd_shape_.get_yw_org());
		update_legends();
		break;

	case HINT_R_MOUSE_BUTTON_DOWN:
		edit_spike_class(HIWORD(l_param), short_value);
		break;
	case HINT_VIEW_TAB_HAS_CHANGED:
		update_ctrl_tab1(short_value);
		break;
	default:
		break;
	}
	return 0L;
}

void ViewSpikeTemplates::on_en_change_class()
{
	if (mm_spike_no_class_.m_b_entry_done)
	{
		const auto spike_class = spike_no_class;
		mm_spike_no_class_.on_en_change(this, spike_no_class, 1, -1);

		if (spike_no_class != spike_class) // change display if necessary
		{
			m_chart_spk_wnd_shape_.set_plot_mode(PLOT_ONE_CLASS, spike_no_class);
			m_chart_spk_wnd_shape_.Invalidate();
			update_legends();
		}
	}
}

void ViewSpikeTemplates::on_en_change_time_first()
{
	if (mm_time_first_.m_b_entry_done)
	{
		mm_time_first_.on_en_change(this, time_first, 1.f, -1.f);

		const auto l_first = static_cast<long>(time_first * p_spk_doc->get_acq_rate());
		if (l_first != l_first_)
		{
			l_first_ = l_first;
			update_legends();
		}
	}
}

void ViewSpikeTemplates::on_en_change_time_last()
{
	if (mm_time_last_.m_b_entry_done)
	{
		mm_time_last_.on_en_change(this, time_last, 1.f, -1.f);

		const auto l_last = static_cast<long>(time_last * p_spk_doc->get_acq_rate());
		if (l_last != l_last_)
		{
			l_last_ = l_last;
			update_legends();
		}
	}
}

void ViewSpikeTemplates::OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar)
{
	// form_view scroll: if pointer null
	if (p_scroll_bar == nullptr)
	{
		ViewDbTable::OnHScroll(n_sb_code, n_pos, p_scroll_bar);
		return;
	}

	// get corresponding data
	const auto total_scroll = p_spk_doc->get_acq_size();
	const long page_scroll = (l_last_ - l_first_);
	auto sb_scroll = page_scroll / 10;
	if (sb_scroll == 0)
		sb_scroll = 1;
	long l_first = l_first_;
	switch (n_sb_code)
	{
	case SB_LEFT: l_first = 0; break; 
	case SB_LINELEFT: l_first -= sb_scroll; break;
	case SB_LINERIGHT: l_first += sb_scroll; break; 
	case SB_PAGELEFT: l_first -= page_scroll; break; 
	case SB_PAGERIGHT: l_first += page_scroll; break; 
	case SB_RIGHT: l_first = total_scroll - page_scroll + 1; break;
	case SB_THUMBPOSITION: 
	case SB_THUMBTRACK: 
		l_first = static_cast<int>(n_pos); break;
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
	if (l_first_ == 0 && l_last_ >= p_spk_doc->get_acq_size() - 1)
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

void ViewSpikeTemplates::on_format_all_data()
{
	// dots: spk file length
	l_first_ = 0;
	l_last_ = p_spk_doc->get_acq_size() - 1;
	// spikes: center spikes horizontally and adjust hz size of display
	constexpr short x_wo = 0;
	const short x_we = static_cast<short>(p_spk_list->get_spike_length());
	m_chart_spk_wnd_shape_.set_xw_ext_org(x_we, x_wo);
	update_legends();
}

void ViewSpikeTemplates::on_format_gain_adjust()
{
	int maxvalue, minvalue;
	GetDocument()->get_max_min_amplitude_of_all_spikes(m_b_all_files, TRUE, maxvalue, minvalue);
	const auto extent = MulDiv(maxvalue - minvalue + 1, 10, 9);
	const auto zero = (maxvalue + minvalue) / 2;

	set_extent_zero_all_display(extent, zero);
}

void ViewSpikeTemplates::on_format_center_curve()
{
	int maxvalue, minvalue;
	GetDocument()->get_max_min_amplitude_of_all_spikes(m_b_all_files, TRUE, maxvalue, minvalue);
	const auto extent = m_chart_spk_wnd_shape_.get_yw_extent();
	const auto zero = (maxvalue + minvalue) / 2;
	set_extent_zero_all_display(extent, zero);
}

void ViewSpikeTemplates::set_extent_zero_all_display(const int extent, const int zero)
{
	m_chart_spk_wnd_shape_.set_yw_ext_org(extent, zero);
	m_template_list_.set_y_w_ext_org(extent, zero);
	m_avg_list_.set_y_w_ext_org(extent, zero);
	m_avg_all_list_.set_y_w_ext_org(extent, zero);

	m_chart_spk_wnd_shape_.Invalidate();
	m_template_list_.Invalidate();
	m_avg_list_.Invalidate();
	m_avg_all_list_.Invalidate();
}

void ViewSpikeTemplates::on_en_change_hit_rate()
{
	if (mm_hit_rate_.m_b_entry_done)
	{
		mm_hit_rate_.on_en_change(this, hit_rate, 1, -1);

		if (spike_classification_parameters_->hit_rate != hit_rate)
			spike_classification_parameters_->hit_rate = hit_rate;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::on_en_change_hit_rate_sort()
{
	if (mm_hit_rate_sort_.m_b_entry_done)
	{
		mm_hit_rate_sort_.on_en_change(this, hit_rate_sort, 1, -1);

		if (spike_classification_parameters_->hit_rate_sort != hit_rate_sort)
			spike_classification_parameters_->hit_rate_sort = hit_rate_sort;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::on_en_change_tolerance()
{
	if (mm_k_tolerance_.m_b_entry_done)
	{
		mm_k_tolerance_.on_en_change(this, k_tolerance, 1.f, -1.f);
		if (k_tolerance < 0)
			k_tolerance = -k_tolerance;
		if (spike_classification_parameters_->k_tolerance != k_tolerance)
			spike_classification_parameters_->k_tolerance = k_tolerance;
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::display_avg(const boolean b_all_files, CTemplateListWnd* template_list) //, CImageList* pImList)
{
	p_spk_list = p_spk_doc->get_spike_list_current();

	// get list of classes
	template_list->set_hit_rate_tolerance(&hit_rate, &k_tolerance);
	int tpllen = spike_classification_parameters_->k_right - spike_classification_parameters_->k_left + 1;

	// define and attach to ImageList to CListCtrl; create 1 item by default
	if (template_list->GetImageList(LVSIL_NORMAL) != &template_list->m_image_list)
	{
		CRect rect;
		m_chart_spk_wnd_shape_.GetClientRect(&rect);
		template_list->m_image_list.Create(rect.right, rect.bottom, ILC_COLOR8, 1, 1);
		template_list->SetImageList(&template_list->m_image_list, LVSIL_NORMAL);
	}

	// reinit all templates to zero
	template_list->DeleteAllItems();
	const int spike_len = p_spk_list->get_spike_length();
	template_list->set_template_length(spike_len, 0, spike_len - 1);
	template_list->set_hit_rate_tolerance(&hit_rate, &k_tolerance);

	int zero = m_chart_spk_wnd_shape_.get_yw_org();
	int extent = m_chart_spk_wnd_shape_.get_yw_extent();
	if (zero == 0 && extent == 0)
	{
		int valuemax, valuemin;
		p_spk_list->get_total_max_min(TRUE, &valuemax, &valuemin);
		extent = valuemax - valuemin;
		zero = (valuemax + valuemin) / 2;
		m_chart_spk_wnd_shape_.set_yw_ext_org(extent, zero);
	}
	template_list->set_y_w_ext_org(extent, zero);

	// set file indexes - assume only one file selected
	auto p_dbwave_doc = GetDocument();
	const int current_file = p_dbwave_doc->db_get_current_record_position(); // index current file
	auto first_file = current_file; // index first file in the series
	auto last_file = current_file; // index last file in the series
	// make sure we have the correct spike list here
	const auto current_list = spk_list_tab_ctrl.GetCurSel();
	p_spk_doc->set_index_current_spike_list(current_list);

	CString cs_comment;
	CString cs_file_comment = _T("Analyze file: ");
	if (b_all_files)
	{
		first_file = 0; // index first file
		last_file = p_dbwave_doc->db_get_records_count() - 1; // index last file
	}
	// loop over files
	for (auto i_file = first_file; i_file <= last_file; i_file++)
	{
		// load file
		p_dbwave_doc->db_set_current_record_position(i_file);
		const auto p_spk_doc = p_dbwave_doc->open_current_spike_file();
		if (p_spk_doc == nullptr)
			continue;
		CString cs;
		cs.Format(_T("%i/%i - "), i_file, last_file);
		cs += p_dbwave_doc->db_get_current_spk_file_name(FALSE);
		p_dbwave_doc->SetTitle(cs);
		p_spk_doc->SetModifiedFlag(FALSE);

		auto pSpkList = p_spk_doc->set_index_current_spike_list(current_list); // load pointer to spike list
		if (!pSpkList->is_class_list_valid()) // if class list not valid:
		{
			pSpkList->update_class_list(); // rebuild list of classes
			p_spk_doc->SetModifiedFlag(); // and set modified flag
		}
		const auto n_spikes = pSpkList->get_spikes_count();

		// add spikes to templates - create templates on the fly
		int j_template;
		for (auto i = 0; i < n_spikes; i++)
		{
			const auto cla = pSpkList->get_spike(i)->get_class_id();
			auto b_found = FALSE;
			for (j_template = 0; j_template < template_list->get_template_data_size(); j_template++)
			{
				if (cla == template_list->get_template_class_id(j_template))
				{
					b_found = TRUE;
					break;
				}
				if (cla < template_list->get_template_class_id(j_template))
					break;
			}
			// add template if not found - insert it at the proper place
			if (!b_found) // add item if not found
			{
				if (j_template < 0)
					j_template = 0;
				j_template = template_list->insert_template_data(j_template, cla);
			}

			// get data and add spike
			const auto p_spik = pSpkList->get_spike(i)->get_p_data();
			template_list->t_add(j_template, p_spik); // add spike to template j
			template_list->t_add(p_spik); // add spike to template zero
		}
	}

	// end of loop, select current file again if necessary
	if (b_all_files)
	{
		if(p_dbwave_doc->db_set_current_record_position(current_file))
			p_spk_doc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update average
	template_list->transfer_template_data();
	template_list->t_global_stats();
	template_list->update_template_legends("cx");
}

void ViewSpikeTemplates::on_build_templates()
{
	p_spk_list = p_spk_doc->get_spike_list_current();

	// set file indexes
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->db_get_current_record_position();
	const auto currentlist = spk_list_tab_ctrl.GetCurSel();
	auto firstfile = currentfile;
	auto lastfile = firstfile;
	if (m_b_all_files)
	{
		firstfile = 0;
		lastfile = p_dbwave_doc->db_get_records_count() - 1;
	}

	// add as many forms as we have classes
	m_template_list_.DeleteAllItems(); // reinit all templates to zero
	m_template_list_.set_template_length(p_spk_list->get_spike_length(), spike_classification_parameters_->k_left, spike_classification_parameters_->k_right);
	m_template_list_.set_hit_rate_tolerance(&hit_rate, &k_tolerance);

	// compute global std
	// loop over all selected files (or only one file currently selected)
	int n_spikes;
	int i_file;
	CString cs_comment;

	for (i_file = firstfile; i_file <= lastfile; i_file++)
	{
		// store nb of spikes within array
		if (m_b_all_files)
		{
			if (p_dbwave_doc->db_set_current_record_position(i_file))
				p_spk_doc = p_dbwave_doc->open_current_spike_file();
			else
				continue;
		}

		const auto spike_list = p_spk_doc->set_index_current_spike_list(currentlist);
		n_spikes = spike_list->get_spikes_count();
		for (auto i = 0; i < n_spikes; i++)
			m_template_list_.t_add(p_spk_list->get_spike(i)->get_p_data());
	}
	m_template_list_.t_global_stats();

	// now scan all spikes to build templates
	auto n_templates = 0;
	double dist_min;
	int offset_min;
	int tpl_min;
	CString cs_file_comment = _T("Second pass - analyze file: ");

	for (i_file = firstfile; i_file <= lastfile; i_file++)
	{
		// store nb of spikes within array
		if (m_b_all_files)
		{
			if (p_dbwave_doc->db_set_current_record_position(i_file))
				p_dbwave_doc->open_current_spike_file();
			else
				continue;
			p_spk_doc = p_dbwave_doc->m_p_spk_doc;
			CString cs;
			cs.Format(_T("%i/%i - "), i_file, lastfile);
			cs += p_dbwave_doc->db_get_current_spk_file_name(FALSE);
			p_dbwave_doc->SetTitle(cs);
		}

		auto spike_list = p_spk_doc->set_index_current_spike_list(currentlist);
		n_spikes = spike_list->get_spikes_count();

		// create template CListCtrl
		for (auto i = 0; i < n_spikes; i++)
		{
			// filter out undesirable spikes
			if (b_display_single_class)
			{
				if (p_spk_list->get_spike(i)->get_class_id() != spike_no_class)
					continue;
			}
			const auto ii_time = p_spk_list->get_spike(i)->get_time();
			if (ii_time < l_first_ || ii_time > l_last_)
				continue;

			// get pointer to spike data and search if any template is suitable
			auto* p_spik = p_spk_list->get_spike(i)->get_p_data();
			auto b_within = FALSE;
			int itpl;
			for (itpl = 0; itpl < n_templates; itpl++)
			{
				// exit loop if spike is within template
				b_within = m_template_list_.t_within(itpl, p_spik);
				if (b_within)
					break;
				// OR exit loop if spike dist is less distant
				m_template_list_.t_min_dist(itpl, p_spik, &offset_min, &dist_min);
				b_within = (dist_min <= m_template_list_.m_global_dist);
				if (b_within)
					break;
			}

			// if a template is suitable, find the most likely
			if (b_within)
			{
				tpl_min = itpl;
				dist_min = m_template_list_.m_global_dist;
				double x;
				int offset;
				for (auto i_tpl2 = 0; i_tpl2 < n_templates; i_tpl2++)
				{
					m_template_list_.t_min_dist(i_tpl2, p_spik, &offset, &x);
					if (x < dist_min)
					{
						offset_min = offset;
						dist_min = x;
						tpl_min = i_tpl2;
					}
				}
			}
			// else (no suitable template), create a new one
			else
			{
				m_template_list_.insert_template(n_templates, n_templates + i_first_sorted_class);
				tpl_min = n_templates;
				n_templates++;
			}

			// add spike to the corresponding template
			m_template_list_.t_add(tpl_min, p_spik); // add spike to template j
		}
	}

	// end of loop, select current file again if necessary
	if (m_b_all_files)
	{
		p_dbwave_doc->db_set_current_record_position(currentfile);
		p_spk_doc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	m_template_list_.sort_templates_by_number_of_spikes(TRUE, TRUE, i_first_sorted_class);
	update_templates();
}

void ViewSpikeTemplates::sort_spikes()
{
	// set tolerance to sort tolerance
	m_template_list_.set_hit_rate_tolerance(&hit_rate_sort, &k_tolerance);

	// set file indexes - assume only one file selected
	const auto p_dbwave_doc = GetDocument();
	const int current_file = p_dbwave_doc->db_get_current_record_position(); // index current file
	auto first_file = current_file; // index first file in the series
	auto last_file = first_file; // index last file in the series
	const auto current_list = spk_list_tab_ctrl.GetCurSel();

	// change indexes if ALL files selected
	CString cs_comment;
	CString cs_file_comment = _T("Analyze file: ");
	if (m_b_all_files)
	{
		first_file = 0; // index first file
		last_file = p_dbwave_doc->db_get_records_count() - 1; // index last file
	}

	// loop CFrameWnd
	const auto n_templates = m_template_list_.get_n_templates();
	for (auto i_file = first_file; i_file <= last_file; i_file++)
	{
		// store nb of spikes within array
		if (m_b_all_files)
		{
			if (p_dbwave_doc->db_set_current_record_position(i_file))
				p_spk_doc = p_dbwave_doc->open_current_spike_file();
			else
				continue;
			CString cs;
			cs.Format(_T("%i/%i - "), i_file, last_file);
			cs += p_dbwave_doc->db_get_current_spk_file_name(FALSE);
			p_dbwave_doc->SetTitle(cs);
			p_spk_doc->SetModifiedFlag(FALSE);

			p_spk_list = p_spk_doc->set_index_current_spike_list(current_list); // load pointer to spike list
			if (!p_spk_list->is_class_list_valid()) // if class list not valid:
			{
				p_spk_list->update_class_list(); // rebuild list of classes
				p_spk_doc->SetModifiedFlag(); // and set modified flag
			}
		}

		// spike loop
		const auto n_spikes = p_spk_list->get_spikes_count(); // loop over all spikes
		for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
		{
			// filter out undesirable spikes - i.e. not relevant to the sort
			if (b_display_single_class)
			{
				// skip spikes that do not belong to selected class
				if (p_spk_list->get_spike(i_spike)->get_class_id() != spike_no_class)
					continue;
			}

			// skip spikes that do not fit into time interval selected
			const auto ii_time = p_spk_list->get_spike(i_spike)->get_time();
			if (ii_time < l_first_ || ii_time > l_last_)
				continue;

			// get pointer to spike data and search if any template is suitable
			const auto p_spike = p_spk_list->get_spike(i_spike)->get_p_data();
			auto b_within = FALSE;
			double dist_min;
			int offset_min;

			// search first template that meet criteria
			int tpl_min;
			for (tpl_min = 0; tpl_min < n_templates; tpl_min++)
			{
				// exit loop if spike is within template
				b_within = m_template_list_.t_within(tpl_min, p_spike);
				m_template_list_.t_min_dist(tpl_min, p_spike, &offset_min, &dist_min);
				if (b_within)
					break;

				// OR exit loop if spike dist is less distant
				b_within = (dist_min <= m_template_list_.m_global_dist);
				if (b_within)
					break;
			}

			// if a template is suitable, find the most likely
			if (b_within)
			{
				for (auto i_tpl = tpl_min + 1; i_tpl < n_templates; i_tpl++)
				{
					double x;
					int offset;
					m_template_list_.t_min_dist(i_tpl, p_spike, &offset, &x);
					if (x < dist_min)
					{
						offset_min = offset;
						dist_min = x;
						tpl_min = i_tpl;
					}
				}

				// change spike class ID
				const auto class_id = (m_template_list_.get_template_wnd(tpl_min))->m_class_id;
				if (p_spk_list->get_spike(i_spike)->get_class_id() != class_id)
				{
					p_spk_list->get_spike(i_spike)->set_class_id(class_id);
					p_spk_doc->SetModifiedFlag(TRUE);
				}
			}
		}
		if (p_spk_doc->IsModified())
		{
			p_spk_doc->OnSaveDocument(p_dbwave_doc->db_get_current_spk_file_name(FALSE));
			p_spk_doc->SetModifiedFlag(FALSE);

			GetDocument()->set_db_n_spikes(p_spk_list->get_spikes_count());
			GetDocument()->set_db_n_spike_classes(p_spk_list->get_classes_count());
		}
	}

	// end of loop, select current file again if necessary
	if (m_b_all_files)
	{
		if(p_dbwave_doc->db_set_current_record_position(current_file))
			p_spk_doc = p_dbwave_doc->open_current_spike_file();
		p_dbwave_doc->SetTitle(p_dbwave_doc->GetPathName());
	}

	// update display: average and spk form
	display_avg(FALSE, &m_avg_list_);
	p_spk_list = p_spk_doc->get_spike_list_current();
	m_chart_spk_wnd_shape_.set_source_data(p_spk_list, GetDocument());
	m_chart_spk_wnd_shape_.Invalidate();
}

void ViewSpikeTemplates::on_keydown_template_list(NMHDR* p_nmhdr, LRESULT* p_result)
{
	const auto* p_lv_key_dow = reinterpret_cast<LV_KEYDOWN*>(p_nmhdr);

	// delete selected template
	if (p_lv_key_dow->wVKey == VK_DELETE && m_template_list_.GetSelectedCount() > 0)
	{
		auto flag = FALSE;
		const auto i_sup = m_template_list_.GetItemCount();
		auto cla = i_first_sorted_class;
		for (auto i = 0; i < i_sup; i++)
		{
			const auto state = m_template_list_.GetItemState(i, LVIS_SELECTED);
			if (state > 0)
			{
				cla = m_template_list_.get_template_class_id(i);
				m_template_list_.delete_item(i);
				flag = TRUE;
				continue;
			}
			if (flag)
			{
				m_template_list_.set_template_class_id(i - 1, _T("t"), cla);
				cla++;
			}
		}
	}
	SetDlgItemInt(IDC_NTEMPLATES, m_template_list_.GetItemCount());
	m_template_list_.Invalidate();
	*p_result = 0;
}

void ViewSpikeTemplates::on_check1()
{
	UpdateData(TRUE);
}

void ViewSpikeTemplates::edit_spike_class(const int control_id, const int control_item)
{
	// find which item has been selected
	CTemplateListWnd* p_list = nullptr;
	auto b_spikes = TRUE;
	auto b_all_files = m_b_all_files;
	if (m_avg_list_.GetDlgCtrlID() == control_id)
		p_list = &m_avg_list_;
	else if (m_template_list_.GetDlgCtrlID() == control_id)
	{
		p_list = &m_template_list_;
		b_spikes = FALSE;
	}
	else if (m_avg_all_list_.GetDlgCtrlID() == control_id)
	{
		p_list = &m_avg_all_list_;
		b_all_files = TRUE;
	}
	if (p_list == nullptr)
		return;

	// find which icon has been selected and get the key
	const auto old_class = p_list->get_template_class_id(control_item);

	// launch edit dlg
	DlgEditSpikeClass dlg;
	dlg.m_i_class = old_class;
	if (IDOK == dlg.DoModal() && old_class != dlg.m_i_class)
	{
		// templates
		if (!b_spikes)
			p_list->set_template_class_id(control_item, _T("t"), dlg.m_i_class);
		// spikes
		else
		{
			p_list->set_template_class_id(control_item, _T("c"), dlg.m_i_class);

			// set file indexes - assume only one file selected
			const auto dbwave_doc = GetDocument();
			const int current_file_index = dbwave_doc->db_get_current_record_position(); 
			auto index_first_file = current_file_index; // index first file in the series
			auto index_last_file = index_first_file; // index last file in the series
			const auto current_list = spk_list_tab_ctrl.GetCurSel();

			// change indexes if ALL files selected
			CString cs_comment;
			CString cs_file_comment = _T("Analyze file: ");
			if (b_all_files)
			{
				index_first_file = 0; // index first file
				index_last_file = dbwave_doc->db_get_records_count() - 1; // index last file
			}

			// loop CFrameWnd
			for (auto index_file = index_first_file; index_file <= index_last_file; index_file++)
			{
				// store nb of spikes within array
				if (b_all_files)
				{
					if (dbwave_doc->db_set_current_record_position(index_file))
						p_spk_doc = dbwave_doc->open_current_spike_file();
					else
						continue;
					CString cs;
					cs.Format(_T("%i/%i - "), index_file, index_last_file);
					cs += dbwave_doc->db_get_current_spk_file_name(FALSE);
					dbwave_doc->SetTitle(cs);
					p_spk_doc->SetModifiedFlag(FALSE);
					p_spk_list = p_spk_doc->set_index_current_spike_list(current_list);
				}

				// TODO: this should not work - changing SpikeClassID does not change the spike class because UpdateClassList reset classes array to zero
				p_spk_list->update_class_list(); // rebuild list of classes
				p_spk_list->change_all_spike_from_class_id_to_new_class_id(old_class, dlg.m_i_class);
				p_spk_list->update_class_list(); // rebuild list of classes
				p_spk_doc->SetModifiedFlag(TRUE);

				if (p_spk_doc->IsModified())
				{
					p_spk_doc->OnSaveDocument(dbwave_doc->db_get_current_spk_file_name(FALSE));
					p_spk_doc->SetModifiedFlag(FALSE);

					GetDocument()->set_db_n_spikes(p_spk_list->get_spikes_count());
					GetDocument()->set_db_n_spike_classes(p_spk_list->get_classes_count());
				}
			}

			// end of loop, select current file again if necessary
			if (b_all_files)
			{
				if (dbwave_doc->db_set_current_record_position(current_file_index))
					p_spk_doc = dbwave_doc->open_current_spike_file();
				dbwave_doc->SetTitle(dbwave_doc->GetPathName());
			}
		}
	}
}

void ViewSpikeTemplates::update_ctrl_tab1(const int i_select)
{
	WORD i_template = SW_SHOW;
	WORD i_avg = SW_HIDE;
	WORD i_sort = SW_HIDE;
	switch (i_select)
	{
	case 0:
		m_avg_all_list_.ShowWindow(SW_HIDE);
		m_template_list_.ShowWindow(SW_SHOW);
		break;
	case 1:
		i_template = SW_HIDE;
		i_sort = SW_SHOW;
		i_avg = SW_HIDE;
		m_avg_all_list_.ShowWindow(SW_HIDE);
		m_template_list_.ShowWindow(SW_SHOW);
		break;
	case 2:
		i_template = SW_HIDE;
		i_sort = SW_HIDE;
		i_avg = SW_SHOW;
		m_avg_all_list_.ShowWindow(SW_SHOW);
		m_template_list_.ShowWindow(SW_HIDE);
		break;
	default:
		break;
	}

	// build templates
	GetDlgItem(IDC_STATIC3)->ShowWindow(i_template);
	GetDlgItem(IDC_HITRATE)->ShowWindow(i_template);
	GetDlgItem(IDC_STATIC4)->ShowWindow(i_template);
	GetDlgItem(IDC_TOLERANCE)->ShowWindow(i_template);
	GetDlgItem(IDC_BUILD)->ShowWindow(i_template);
	GetDlgItem(IDC_LOAD_SAVE)->ShowWindow(i_template);
	GetDlgItem(IDC_CHECK1)->ShowWindow(i_template);
	GetDlgItem(IDC_NTEMPLS)->ShowWindow(i_template);
	GetDlgItem(IDC_NTEMPLATES)->ShowWindow(i_template);
	GetDlgItem(IDC_TFIRSTSORTEDCLASS)->ShowWindow(i_template);
	GetDlgItem(IDC_IFIRSTSORTEDCLASS)->ShowWindow(i_template);

	// sort spikes using templates
	GetDlgItem(IDC_STATIC6)->ShowWindow(i_sort);
	GetDlgItem(IDC_HITRATE2)->ShowWindow(i_sort);
	GetDlgItem(IDC_SORT)->ShowWindow(i_sort);
	GetDlgItem(IDC_CHECK2)->ShowWindow(i_sort);

	// display average (total)
	GetDlgItem(IDC_DISPLAY)->ShowWindow(i_avg);
}

void ViewSpikeTemplates::on_bn_clicked_sort()
{
	sort_spikes();
}

void ViewSpikeTemplates::on_bn_clicked_display()
{
	display_avg(TRUE, &m_avg_all_list_); //, &m_ImListAll);
}

void ViewSpikeTemplates::on_en_change_i_first_sorted_class()
{
	if (mm_i_first_sorted_class_.m_b_entry_done)
	{
		mm_i_first_sorted_class_.on_en_change(this, i_first_sorted_class, 1, -1);

		// change class of all templates
		//SetTemplateclassID(int item, LPCSTR pszType, int classID)
		m_template_list_.update_template_base_class_id(i_first_sorted_class);
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::on_tcn_sel_change_tab2(NMHDR* p_nmhdr, LRESULT* p_result)
{
	const auto icursel = spk_list_tab_ctrl.GetCurSel();
	select_spike_list(icursel);
	*p_result = 0;
}

void ViewSpikeTemplates::on_nm_click_tab2(NMHDR* p_nmhdr, LRESULT* p_result)
{
	const auto icursel = spk_list_tab_ctrl.GetCurSel();
	select_spike_list(icursel);
	*p_result = 0;
}

void ViewSpikeTemplates::on_bn_clicked_display_single_class()
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

void ViewSpikeTemplates::on_en_change_t1()
{
	p_spk_list = p_spk_doc->get_spike_list_current();

	if (mm_t1_.m_b_entry_done)
	{
		const auto delta = t_unit / p_spk_list->get_acq_sampling_rate();
		
		mm_t1_.on_en_change(this, m_t1, delta, -delta);
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
			p_spk_list->shape_t1 = spike_classification_parameters_->k_left;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeTemplates::on_en_change_t2()
{
	p_spk_list = p_spk_doc->get_spike_list_current();

	if (mm_t2_.m_b_entry_done)
	{
		const auto delta = t_unit / p_spk_list->get_acq_sampling_rate();
		mm_t2_.on_en_change(this, m_t2, delta, -delta);

		// check boundaries
		if (m_t2 < m_t1)
			m_t2 = m_t1 + delta;

		const int spike_length = p_spk_list->get_spike_length();
		const auto t_max = convert_spike_index_to_time(spike_length - 1);
		if (m_t2 >= t_max)
			m_t2 = t_max;
		// change display if necessary
		const int it2 = convert_time_to_spike_index(m_t2);
		if (it2 != m_chart_spk_wnd_shape_.vt_tags.get_value_int(spk_form_tag_right_))
		{
			spike_classification_parameters_->k_right = it2;
			m_chart_spk_wnd_shape_.move_vt_track(spk_form_tag_right_, spike_classification_parameters_->k_right);
			p_spk_list->shape_t2 = spike_classification_parameters_->k_right;
		}
		UpdateData(FALSE);
	}
}
