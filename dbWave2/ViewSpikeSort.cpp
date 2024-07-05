#include "StdAfx.h"
#include "ViewSpikeSort.h"

#include "dbWave.h"
#include "DlgProgress.h"
#include "DlgSpikeEdit.h"
#include "MainFrm.h"
#include "ViewSpikeDetect.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO limit size of measure array to nb_spikes within currently selected spike_list

IMPLEMENT_DYNCREATE(ViewSpikeSort, dbTableView)

ViewSpikeSort::ViewSpikeSort()
	: dbTableView(IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

ViewSpikeSort::~ViewSpikeSort()
{
	if (m_pSpkDoc != nullptr)
		save_current_spk_file(); // save file if modified

	spike_classification_parameters_->b_changed = TRUE;
	spike_classification_parameters_->source_class = sort_source_class;
	spike_classification_parameters_->dest_class = sort_destination_class;
	spike_classification_parameters_->mv_max = xy_max_amplitude_mv;
	spike_classification_parameters_->mv_min = xy_min_amplitude_mv;
}

void ViewSpikeSort::DoDataExchange(CDataExchange* pDX)
{
	dbTableView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PARAMETER, m_combo_parameter);
	DDX_Text(pDX, IDC_T1, spike_shape_t1);
	DDX_Text(pDX, IDC_T2, spike_shape_t2);
	DDX_Text(pDX, IDC_LIMITLOWER, histogram_lower_threshold);
	DDX_Text(pDX, IDC_LIMITUPPER, histogram_upper_threshold);
	DDX_Text(pDX, IDC_SOURCECLASS, sort_source_class);
	DDX_Text(pDX, IDC_DESTINATIONCLASS, sort_destination_class);
	DDX_Text(pDX, IDC_EDIT2, display_bars_time_left);
	DDX_Text(pDX, IDC_EDIT3, display_bars_time_last);
	DDX_Text(pDX, IDC_EDIT_MAX_MV, xy_max_amplitude_mv);
	DDX_Text(pDX, IDC_EDIT_MIN_MV, xy_min_amplitude_mv);
	DDX_Text(pDX, IDC_BINMV, histogram_bin_size_mv);
	DDX_Check(pDX, IDC_CHECK1, b_all_files);
	DDX_Text(pDX, IDC_NSPIKES, m_spike_index);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spike_index_class);
	DDX_Text(pDX, IDC_EDITRIGHT2, t_xy_right);
	DDX_Text(pDX, IDC_EDITLEFT2, t_xy_left);

	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}

BEGIN_MESSAGE_MAP(ViewSpikeSort, dbTableView)

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikeSort::on_my_message)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_SOURCECLASS, &ViewSpikeSort::on_en_change_source_spike_class)
	ON_EN_CHANGE(IDC_DESTINATIONCLASS, &ViewSpikeSort::on_en_change_destination_spike_class)
	ON_CBN_SELCHANGE(IDC_PARAMETER, &ViewSpikeSort::on_select_change_parameter)
	ON_EN_CHANGE(IDC_LIMITLOWER, &ViewSpikeSort::on_en_change_lower)
	ON_EN_CHANGE(IDC_LIMITUPPER, &ViewSpikeSort::on_en_change_upper)
	ON_EN_CHANGE(IDC_T1, &ViewSpikeSort::on_en_change_t1)
	ON_EN_CHANGE(IDC_T2, &ViewSpikeSort::on_en_change_t2)
	ON_BN_CLICKED(IDC_EXECUTE, &ViewSpikeSort::on_sort)
	ON_BN_CLICKED(IDC_MEASURE, &ViewSpikeSort::on_measure)
	ON_BN_CLICKED(IDC_CHECK1, &ViewSpikeSort::on_select_all_files)
	ON_COMMAND(ID_FORMAT_ALLDATA, &ViewSpikeSort::on_format_all_data)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &ViewSpikeSort::on_format_center_curve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &ViewSpikeSort::on_format_gain_adjust)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, &ViewSpikeSort::on_format_split_curves)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, &ViewSpikeSort::on_tools_edit_spikes)
	ON_COMMAND(ID_TOOLS_ALIGNSPIKES, &ViewSpikeSort::on_tools_align_spikes)

	ON_EN_CHANGE(IDC_EDIT2, &ViewSpikeSort::on_en_change_time_first)
	ON_EN_CHANGE(IDC_EDIT3, &ViewSpikeSort::on_en_change_time_last)
	ON_EN_CHANGE(IDC_EDIT_MIN_MV, &ViewSpikeSort::on_en_change_mv_min)
	ON_EN_CHANGE(IDC_EDIT_MAX_MV, &ViewSpikeSort::on_en_change_mv_max)
	ON_EN_CHANGE(IDC_EDITLEFT2, &ViewSpikeSort::on_en_change_edit_left2)
	ON_EN_CHANGE(IDC_EDITRIGHT2, &ViewSpikeSort::on_en_change_edit_right2)
	ON_EN_CHANGE(IDC_NSPIKES, &ViewSpikeSort::on_en_change_no_spike)
	ON_BN_DOUBLECLICKED(IDC_DISPLAYPARM, &ViewSpikeSort::on_tools_edit_spikes)
	ON_EN_CHANGE(IDC_SPIKECLASS, &ViewSpikeSort::on_en_change_spike_class)
	ON_EN_CHANGE(IDC_BINMV, &ViewSpikeSort::on_en_change_n_bins)
END_MESSAGE_MAP()

void ViewSpikeSort::define_sub_classed_items()
{
	// subclass some controls
	VERIFY(chart_histogram_.SubclassDlgItem(IDC_HISTOGRAM, this));
	VERIFY(chart_xt_measures_.SubclassDlgItem(IDC_DISPLAYPARM, this));
	VERIFY(chart_spike_shape_.SubclassDlgItem(IDC_DISPLAYSPIKE, this));
	VERIFY(chart_spike_bar_.SubclassDlgItem(IDC_DISPLAYBARS, this));

	VERIFY(mm_t1_.SubclassDlgItem(IDC_T1, this));
	VERIFY(mm_t2_.SubclassDlgItem(IDC_T2, this));
	VERIFY(mm_limit_lower_.SubclassDlgItem(IDC_LIMITLOWER, this));
	VERIFY(mm_limit_upper_.SubclassDlgItem(IDC_LIMITUPPER, this));
	VERIFY(mm_source_class_.SubclassDlgItem(IDC_SOURCECLASS, this));
	VERIFY(mm_destination_class_.SubclassDlgItem(IDC_DESTINATIONCLASS, this));
	VERIFY(mm_time_first_.SubclassDlgItem(IDC_EDIT2, this));
	VERIFY(mm_time_last_.SubclassDlgItem(IDC_EDIT3, this));
	VERIFY(mm_mv_max_.SubclassDlgItem(IDC_EDIT_MAX_MV, this));
	VERIFY(mm_mv_min_.SubclassDlgItem(IDC_EDIT_MIN_MV, this));
	VERIFY(mm_t_xy_right_.SubclassDlgItem(IDC_EDITRIGHT2, this));
	VERIFY(mm_t_xy_left_.SubclassDlgItem(IDC_EDITLEFT2, this));
	VERIFY(mm_mv_bin_.SubclassDlgItem(IDC_BINMV, this));

	VERIFY(mm_spike_index_.SubclassDlgItem(IDC_NSPIKES, this));
	mm_spike_index_.ShowScrollBar(SB_VERT);
	VERIFY(mm_spike_index_class_.SubclassDlgItem(IDC_SPIKECLASS, this));
	mm_spike_index_class_.ShowScrollBar(SB_VERT);

	VERIFY(m_file_scroll_.SubclassDlgItem(IDC_FILESCROLL, this));
	m_file_scroll_.SetScrollRange(0, 100, FALSE);
}

void ViewSpikeSort::define_stretch_parameters()
{
	m_stretch_.AttachParent(this);

	m_stretch_.newProp(IDC_EDIT_MIN_MV, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_DISPLAYPARM, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_DISPLAYBARS, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_EDIT3, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_STATICRIGHT, SZEQ_XREQ, SZEQ_YBEQ);

	m_stretch_.newProp(IDC_STATICLEFT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_STATIC12, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_EDITLEFT2, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_EDITRIGHT2, SZEQ_XLEQ, SZEQ_YBEQ);
}

void ViewSpikeSort::OnInitialUpdate()
{
	dbTableView::OnInitialUpdate();
	define_sub_classed_items();
	define_stretch_parameters();
	m_b_init_ = TRUE;
	m_auto_increment = true;
	m_auto_detect = true;

	// load global parameters
	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	spike_classification_parameters_ = &(p_app->spk_classification);
	options_view_data_ = &(p_app->options_view_data);

	// assign values to controls
	m_combo_parameter.SetCurSel(spike_classification_parameters_->i_parameter);
	xy_max_amplitude_mv = spike_classification_parameters_->mv_max;
	xy_min_amplitude_mv = spike_classification_parameters_->mv_min;

	sort_source_class = spike_classification_parameters_->source_class;
	sort_destination_class = spike_classification_parameters_->dest_class;

	chart_spike_shape_.display_all_files(false, GetDocument());
	chart_spike_shape_.set_plot_mode(PLOT_ONE_COLOR, sort_source_class);
	chart_spike_shape_.set_scope_parameters(&(options_view_data_->spksort1spk));
	m_spk_form_tag_left_ = chart_spike_shape_.vertical_tags.add_tag(spike_classification_parameters_->i_cursor_t1, 0);
	m_spk_form_tag_right_ = chart_spike_shape_.vertical_tags.add_tag(spike_classification_parameters_->i_cursor_t2, 0);

	chart_xt_measures_.display_all_files(false, GetDocument());
	chart_xt_measures_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);
	chart_xt_measures_.set_scope_parameters(&(options_view_data_->spksort1parms));
	m_i_tag_up_ = chart_xt_measures_.horizontal_tags.add_tag(spike_classification_parameters_->upper_threshold, 0);
	m_i_tag_low_ = chart_xt_measures_.horizontal_tags.add_tag(spike_classification_parameters_->lower_threshold, 0);

	chart_spike_bar_.display_all_files(false, GetDocument());
	chart_spike_bar_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);
	chart_spike_bar_.set_scope_parameters(&(options_view_data_->spksort1bars));

	chart_histogram_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);
	chart_histogram_.set_scope_parameters(&(options_view_data_->spksort1hist));

	// display tag lines at proper places
	m_spk_hist_upper_threshold_ = chart_histogram_.vertical_tags.add_tag(spike_classification_parameters_->upper_threshold, 0);
	m_spk_hist_lower_threshold_ = chart_histogram_.vertical_tags.add_tag(spike_classification_parameters_->lower_threshold, 0);

	update_file_parameters();
	if (nullptr != m_pSpkList)
	{
		histogram_lower_threshold = static_cast<float>(spike_classification_parameters_->lower_threshold) * m_delta_mv_;
		histogram_upper_threshold = static_cast<float>(spike_classification_parameters_->upper_threshold) * m_delta_mv_;
		UpdateData(false);
	}
	activate_mode4();
}

void ViewSpikeSort::activate_mode4()
{
	auto n_cmd_show = SW_HIDE;
	if (4 == spike_classification_parameters_->i_parameter)
	{
		n_cmd_show = SW_SHOW;
		if (1 > chart_xt_measures_.vertical_tags.get_tag_list_size())
		{
			m_i_xy_right_ = chart_xt_measures_.vertical_tags.add_tag(spike_classification_parameters_->i_xy_right, 0);
			m_i_xy_left_ = chart_xt_measures_.vertical_tags.add_tag(spike_classification_parameters_->i_xy_left, 0);
			const auto delta = m_pSpkList->get_acq_sampling_rate() / m_time_unit;
			t_xy_right = static_cast<float>(spike_classification_parameters_->i_xy_right) / delta;
			t_xy_left = static_cast<float>(spike_classification_parameters_->i_xy_left) / delta;
		}
		chart_xt_measures_.set_nx_scale_cells(2, 0, 0);
		chart_xt_measures_.get_scope_parameters()->crScopeGrid = RGB(128, 128, 128);

		if (nullptr != m_pSpkList)
		{
			const auto spike_length_ms = (static_cast<double>(static_cast<float>(m_pSpkList->get_spike_length()) * m_time_unit / m_pSpkList->get_acq_sampling_rate()));
			CString cs_dummy;
			cs_dummy.Format(_T("%0.1f ms"), spike_length_ms);
			GetDlgItem(IDC_STATICRIGHT)->SetWindowText(cs_dummy);
			cs_dummy.Format(_T("%0.1f ms"), -spike_length_ms);
			GetDlgItem(IDC_STATICLEFT)->SetWindowText(cs_dummy);
		}
	}
	else
	{
		chart_xt_measures_.vertical_tags.remove_all_tags();
		chart_xt_measures_.set_nx_scale_cells(0, 0, 0);
	}
	GetDlgItem(IDC_STATICRIGHT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATICLEFT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC12)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDITRIGHT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDITLEFT2)->ShowWindow(n_cmd_show);
	chart_xt_measures_.Invalidate();
}

void ViewSpikeSort::OnActivateView(const BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		auto* p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW,
		                         reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		save_current_spk_file();
		const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		if (nullptr == p_app->m_p_sort1_spikes_memory_file)
		{
			p_app->m_p_sort1_spikes_memory_file = new CMemFile;
			ASSERT(p_app->m_p_sort1_spikes_memory_file != NULL);
		}
		CArchive ar(p_app->m_p_sort1_spikes_memory_file, CArchive::store);
		p_app->m_p_sort1_spikes_memory_file->SeekToBegin();
		ar.Close();
	}
	dbTableView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void ViewSpikeSort::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_b_init_)
	{
		switch (LOWORD(lHint))
		{
		case HINT_DOCHASCHANGED: 
		case HINT_DOCMOVERECORD:
		case HINT_REQUERY:
			update_file_parameters();
			break;
		case HINT_CLOSEFILEMODIFIED: 
			save_current_spk_file();
			break;
		case HINT_REPLACEVIEW:
		default:
			break;
		}
	}
}

BOOL ViewSpikeSort::OnMove(const UINT n_id_move_command)
{
	save_current_spk_file();
	return dbTableView::OnMove(n_id_move_command);
}

void ViewSpikeSort::update_spike_file()
{
	m_pSpkDoc = GetDocument()->open_current_spike_file();

	if (nullptr != m_pSpkDoc)
	{
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->db_get_current_spk_file_name(), FALSE);
		const int current_index = GetDocument()->get_current_spike_file()->get_spike_list_current_index();
		m_pSpkList = m_pSpkDoc->set_spike_list_as_current(current_index);

		// update Tab at the bottom
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);
		m_tabCtrl.SetCurSel(current_index);
	}
}

void ViewSpikeSort::update_file_parameters()
{
	// reset parameters ? flag = single file or file list has changed
	if (!b_all_files)
	{
		chart_histogram_.delete_histogram_data();
	}

	const BOOL first_update = (m_pSpkDoc == nullptr);
	update_spike_file();

	if (first_update || options_view_data_->bEntireRecord)
	{
		if (m_pSpkDoc == nullptr)
			return;
		display_bars_time_left = 0.f;
		display_bars_time_last = (static_cast<float>(m_pSpkDoc->get_acq_size()) - 1) / m_pSpkList->get_acq_sampling_rate();
	}
	l_first_ = static_cast<long>(display_bars_time_left * m_pSpkList->get_acq_sampling_rate());
	l_last_ = static_cast<long>(display_bars_time_last * m_pSpkList->get_acq_sampling_rate());
	m_delta_t_ = m_time_unit / m_pSpkList->get_acq_sampling_rate();
	m_delta_mv_ = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;

	// spike and classes
	auto spike_index = m_pSpkList->m_selected_spike;
	if (m_pSpkList->get_spikes_count() < spike_index || 0 > spike_index)
	{
		spike_index = -1;
		sort_source_class = 0;
	}
	else
	{
		sort_source_class = m_pSpkList->get_spike(spike_index)->get_class_id();
		spike_classification_parameters_->source_class = sort_source_class;
	}
	ASSERT(sort_source_class < 32768);

	if (0 == spike_classification_parameters_->i_cursor_t1 && 0 == spike_classification_parameters_->i_cursor_t2)
	{
		spike_classification_parameters_->i_cursor_t1 = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
		spike_classification_parameters_->i_cursor_t2 = spike_classification_parameters_->i_cursor_t1 + m_pSpkList->get_detection_parameters()->detect_refractory_period;
	}
	spike_shape_t1 = static_cast<float>(spike_classification_parameters_->i_cursor_t1) * m_time_unit / m_pSpkList->get_acq_sampling_rate();
	spike_shape_t2 = static_cast<float>(spike_classification_parameters_->i_cursor_t2) * m_time_unit / m_pSpkList->get_acq_sampling_rate();

	chart_spike_bar_.set_source_data(m_pSpkList, GetDocument());
	chart_spike_shape_.set_source_data(m_pSpkList, GetDocument());
	chart_xt_measures_.set_source_data(m_pSpkList, GetDocument());

	chart_spike_bar_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);
	chart_spike_shape_.set_plot_mode(PLOT_ONE_COLOR, sort_source_class);
	chart_xt_measures_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);
	chart_histogram_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);

	chart_spike_shape_.vertical_tags.set_tag_val(m_spk_form_tag_left_, spike_classification_parameters_->i_cursor_t1);
	chart_spike_shape_.vertical_tags.set_tag_val(m_spk_form_tag_right_, spike_classification_parameters_->i_cursor_t2);

	m_file_scroll_infos_.fMask = SIF_ALL;
	m_file_scroll_infos_.nMin = 0;
	m_file_scroll_infos_.nMax = m_pSpkDoc->get_acq_size() - 1;
	m_file_scroll_infos_.nPos = 0;
	m_file_scroll_infos_.nPage = m_pSpkDoc->get_acq_size();
	m_file_scroll_.SetScrollInfo(&m_file_scroll_infos_);

	update_legends();

	// display & compute parameters
	if (!b_all_files || !b_measure_done_)
	{
		if (4 != spike_classification_parameters_->i_parameter)
		{
			chart_xt_measures_.set_time_intervals(l_first_, l_last_);
			if (chart_xt_measures_.vertical_tags.get_tag_list_size() > 0)
			{
				chart_xt_measures_.vertical_tags.remove_all_tags();
				chart_xt_measures_.Invalidate();
			}
		}
		else
		{
			chart_xt_measures_.set_time_intervals(-m_pSpkList->get_spike_length(), m_pSpkList->get_spike_length());
			if (1 > chart_xt_measures_.vertical_tags.get_tag_list_size())
			{
				m_i_xy_right_ = chart_xt_measures_.vertical_tags.add_tag(spike_classification_parameters_->i_xy_right, 0);
				m_i_xy_left_ = chart_xt_measures_.vertical_tags.add_tag(spike_classification_parameters_->i_xy_left, 0);
				const auto delta = m_pSpkList->get_acq_sampling_rate() / m_time_unit;
				t_xy_right = static_cast<float>(spike_classification_parameters_->i_xy_right) / delta;
				t_xy_left = static_cast<float>(spike_classification_parameters_->i_xy_left) / delta;
				chart_xt_measures_.Invalidate();
			}
		}
		// update text , display and compute histogram
		b_measure_done_ = FALSE; 
		on_measure();
	}

	db_spike spike_sel(-1, -1, spike_index);
	select_spike(spike_sel);
}

void ViewSpikeSort::update_legends()
{
	// update text abscissa and horizontal scroll position
	display_bars_time_left = static_cast<float>(l_first_) / m_pSpkList->get_acq_sampling_rate();
	display_bars_time_last = static_cast<float>(l_last_) / m_pSpkList->get_acq_sampling_rate();
	update_file_scroll();

	if (4 != spike_classification_parameters_->i_parameter)
		chart_xt_measures_.set_time_intervals(l_first_, l_last_);
	else
		chart_xt_measures_.set_time_intervals(-m_pSpkList->get_spike_length(), m_pSpkList->get_spike_length());
	chart_spike_shape_.set_time_intervals(l_first_, l_last_);
	chart_spike_bar_.set_time_intervals(l_first_, l_last_);

	all_charts_invalidate();
	UpdateData(FALSE);
}

void ViewSpikeSort::on_sort()
{
	// set file indexes - assume only one file selected
	const auto pdb_doc = GetDocument();
	const int current_file = pdb_doc->db_get_current_record_position();
	auto first_file = current_file;
	auto last_file = first_file;
	const auto n_files = pdb_doc->db_get_n_records();
	const auto current_list = m_pSpkDoc->get_spike_list_current_index();

	// change indexes if ALL files selected
	DlgProgress* dlg_progress = nullptr;
	auto i_step = 0;
	CString cs_comment;
	if (b_all_files)
	{
		first_file = 0; // index first file
		last_file = pdb_doc->db_get_n_records() - 1; // index last file
		dlg_progress = new DlgProgress;
		dlg_progress->Create();
		dlg_progress->SetStep(1);
	}

	for (auto i_file = first_file; i_file <= last_file; i_file++)
	{
		// load spike file
		BOOL flag_changed;
		if (pdb_doc->db_set_current_record_position(i_file)) {
			m_pSpkDoc = pdb_doc->open_current_spike_file();
			if (nullptr == m_pSpkDoc)
				continue;
		}

		// update screen if multi-file requested
		if (b_all_files)
		{
			if (dlg_progress->CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			cs_comment.Format(_T("Processing file [%i / %i]"), i_file + 1, n_files);
			dlg_progress->SetStatus(cs_comment);
			if (MulDiv(i_file, 100, n_files) > i_step)
			{
				dlg_progress->StepIt();
				i_step = MulDiv(i_file, 100, n_files);
			}
		}

		// load spike list
		m_pSpkList = m_pSpkDoc->set_spike_list_as_current(current_list);
		if ((nullptr == m_pSpkList) || (0 == m_pSpkList->get_spike_length()))
			continue;

		// loop over all spikes of the list and compare to a single parameter
		const CSize limits1(spike_classification_parameters_->lower_threshold, spike_classification_parameters_->upper_threshold);
		const CSize from_class_id_to_class_id(sort_source_class, sort_destination_class);
		const CSize time_window(l_first_, l_last_);

		// sort on 1 parameter
		if (4 != spike_classification_parameters_->i_parameter)
		{
			flag_changed = m_pSpkList->sort_spike_with_y1(from_class_id_to_class_id, time_window, limits1);
		}
		// sort on 2 parameters
		else
		{
			const CSize limits2(spike_classification_parameters_->i_xy_left, spike_classification_parameters_->i_xy_right);
			flag_changed = m_pSpkList->sort_spike_with_y1_and_y2(from_class_id_to_class_id, time_window, limits1, limits2);
		}

		if (flag_changed)
		{
			m_pSpkDoc->OnSaveDocument(pdb_doc->db_get_current_spk_file_name(FALSE));
			pdb_doc->set_db_n_spikes(m_pSpkList->get_spikes_count());
		}
	}

	// end of loop, select current file again if necessary
	if (b_all_files)
	{
		delete dlg_progress;
		if (pdb_doc->db_set_current_record_position(current_file)) {
			m_pSpkDoc = pdb_doc->open_current_spike_file();
			m_pSpkList = m_pSpkDoc->get_spike_list_current();
		}
	}

	// refresh data windows
	build_histogram();
	all_charts_invalidate();
	m_pSpkDoc->SetModifiedFlag(TRUE);
}

void ViewSpikeSort::set_mouse_cursor(short short_value)
{
	if (CURSOR_ZOOM < short_value)
		short_value = 0;
	set_view_mouse_cursor(short_value);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(short_value, 0));
}

void ViewSpikeSort::change_hz_limits()
{
	if (4 != spike_classification_parameters_->i_parameter)
	{
		l_first_ = chart_xt_measures_.get_time_first();
		l_last_ = chart_xt_measures_.get_time_last();
	}
	else
	{
		l_first_ = chart_spike_bar_.get_time_first();
		l_last_ = chart_spike_bar_.get_time_last();
	}
	update_legends();
}

void ViewSpikeSort::hit_spike()
{
	if (m_pSpkList->get_spike_flag_array_count() > 0)
		clear_flag_all_spikes();
	db_spike spike_hit = GetDocument()->get_spike_hit();
	select_spike(spike_hit);
}

void ViewSpikeSort::change_vertical_tag_spike_shape(const short short_value)
{
	if (short_value == m_spk_form_tag_left_) // first tag
	{
		spike_classification_parameters_->i_cursor_t1 = chart_spike_shape_.vertical_tags.get_value(m_spk_form_tag_left_);
		spike_shape_t1 = static_cast<float>(spike_classification_parameters_->i_cursor_t1) * m_time_unit / m_pSpkList->get_acq_sampling_rate();
		mm_t1_.m_bEntryDone = TRUE;
		on_en_change_t1();
	}
	else if (short_value == m_spk_form_tag_right_) // second tag
	{
		spike_classification_parameters_->i_cursor_t2 = chart_spike_shape_.vertical_tags.get_value(m_spk_form_tag_right_);
		spike_shape_t2 = static_cast<float>(spike_classification_parameters_->i_cursor_t2) * m_time_unit / m_pSpkList->get_acq_sampling_rate();
		mm_t2_.m_bEntryDone = TRUE;
		on_en_change_t2();
	}
}

void ViewSpikeSort::change_vertical_tag_histogram(const short short_value)
{
	if (short_value == m_spk_hist_lower_threshold_) // first tag
	{
		spike_classification_parameters_->lower_threshold = chart_histogram_.vertical_tags.get_value(m_spk_hist_lower_threshold_);
		histogram_lower_threshold = static_cast<float>(spike_classification_parameters_->lower_threshold) * m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		UpdateData(false);
	}
	else if (short_value == m_spk_hist_upper_threshold_) // second tag
	{
		spike_classification_parameters_->upper_threshold = chart_histogram_.vertical_tags.get_value(m_spk_hist_upper_threshold_); // load new value
		histogram_upper_threshold = static_cast<float>(spike_classification_parameters_->upper_threshold) * m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		UpdateData(false);
	}
}

void ViewSpikeSort::change_vertical_tag_xy_chart(const short short_value)
{
	if (short_value == m_i_xy_right_)
	{
		const auto delta = m_pSpkList->get_acq_sampling_rate() / m_time_unit;
		spike_classification_parameters_->i_xy_right = chart_xt_measures_.vertical_tags.get_value(m_i_xy_right_);
		t_xy_right = static_cast<float>(spike_classification_parameters_->i_xy_right) / delta;
		mm_t_xy_right_.m_bEntryDone = TRUE;
		on_en_change_edit_right2();
	}
	else if (short_value == m_i_xy_left_)
	{
		const auto delta = m_pSpkList->get_acq_sampling_rate() / m_time_unit;
		spike_classification_parameters_->i_xy_left = chart_xt_measures_.vertical_tags.get_value(m_i_xy_left_);
		t_xy_left = static_cast<float>(spike_classification_parameters_->i_xy_left) / delta;
		mm_t_xy_left_.m_bEntryDone = TRUE;
		on_en_change_edit_left2();
	}
}

void ViewSpikeSort::change_horizontal_tag_xy_chart(const short short_value)
{
	if (short_value == m_i_tag_low_)
	{
		spike_classification_parameters_->lower_threshold = chart_xt_measures_.horizontal_tags.get_value(m_i_tag_low_);
		histogram_lower_threshold = static_cast<float>(spike_classification_parameters_->lower_threshold) * m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		mm_limit_lower_.m_bEntryDone = TRUE;
		on_en_change_lower();
	}
	else if (short_value == m_i_tag_up_)
	{
		spike_classification_parameters_->upper_threshold = chart_xt_measures_.horizontal_tags.get_value(m_i_tag_up_);
		histogram_upper_threshold = static_cast<float>(spike_classification_parameters_->upper_threshold) * m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		mm_limit_upper_.m_bEntryDone = TRUE;
		on_en_change_upper();
	}
}

void ViewSpikeSort::save_windows_properties_to_options()
{
	options_view_data_->spksort1spk = *chart_spike_shape_.get_scope_parameters();
	options_view_data_->spksort1parms = *chart_xt_measures_.get_scope_parameters();
	options_view_data_->spksort1hist = *chart_histogram_.get_scope_parameters();
	options_view_data_->spksort1bars = *chart_spike_bar_.get_scope_parameters();
}

LRESULT ViewSpikeSort::on_my_message(WPARAM code, LPARAM lParam)
{
	const short short_value = LOWORD(lParam);
	switch (code)
	{
	case HINT_SETMOUSECURSOR: // ------------- change mouse cursor (all 3 items)
		set_mouse_cursor(short_value);
		break;

	case HINT_CHANGEHZLIMITS: // -------------  abscissa have changed
		change_hz_limits();
		break;

	case HINT_HITSPIKE:
		hit_spike();
		break;

	case HINT_SELECTSPIKES:
		all_charts_invalidate();
		break;

	case HINT_DBLCLKSEL:
		on_tools_edit_spikes();
		break;

	case HINT_CHANGEVERTTAG: // -------------  vertical tag value has changed
		if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
			change_vertical_tag_spike_shape(short_value);
		else if (HIWORD(lParam) == IDC_HISTOGRAM)
			change_vertical_tag_histogram(short_value);
		else if (HIWORD(lParam) == IDC_DISPLAYPARM)
			change_vertical_tag_xy_chart(short_value);
		break;

	case HINT_CHANGEHZTAG: // ------------- change horizontal tag value
		if (HIWORD(lParam) == IDC_DISPLAYPARM)
			change_horizontal_tag_xy_chart(short_value);
		break;

	case HINT_VIEWSIZECHANGED: // ------------- change zoom
		update_legends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		save_windows_properties_to_options();
		break;

	case HINT_VIEWTABHASCHANGED:
		select_spike_list(short_value);
		break;

	default:
		break;
	}
	return 0L;
}

void ViewSpikeSort::clear_flag_all_spikes()
{
	if (b_all_files)
	{
		const auto pdb_doc = GetDocument();
		for (auto i_file = 0; i_file < pdb_doc->db_get_n_records(); i_file++)
		{
			if (pdb_doc->db_set_current_record_position(i_file)) {
				m_pSpkDoc = pdb_doc->open_current_spike_file();

				for (auto j = 0; j < m_pSpkDoc->get_spike_list_size(); j++)
				{
					m_pSpkList = m_pSpkDoc->set_spike_list_as_current(j);
					m_pSpkList->remove_all_spike_flags();
				}
			}
		}
	}
	else
		m_pSpkList->remove_all_spike_flags();

	//chart_xt_measures_.Invalidate();
	//chart_spike_shape_.Invalidate();
	//chart_spike_bar_.Invalidate();
}

void ViewSpikeSort::on_measure()
{
	// set file indexes - assume only one file selected
	const auto pdb_doc = GetDocument();
	const int n_files = pdb_doc->db_get_n_records();
	const auto current_spike_list = m_pSpkDoc->get_spike_list_current_index();
	const int index_current_file = pdb_doc->db_get_current_record_position(); 

	// change size of arrays and prepare temporary dialog
	db_spike spike_sel(-1, -1, -1);
	select_spike(spike_sel);
	int index_first_file = index_current_file;
	int index_last_file = index_current_file;
	if (b_all_files)
	{
		index_first_file = 0; 
		index_last_file = n_files - 1; 
	}

	// loop over all selected files (or only one file currently selected)
	for (auto i_file = index_first_file; i_file <= index_last_file; i_file++)
	{
		if (!pdb_doc->db_set_current_record_position(i_file))
			continue;

		m_pSpkDoc = pdb_doc->open_current_spike_file();
		if (m_pSpkDoc == nullptr)
			continue;

		m_pSpkList = m_pSpkDoc->set_spike_list_as_current(current_spike_list);
		if (m_pSpkList == nullptr)
			continue;

		const auto n_spikes = m_pSpkList->get_spikes_count();
		if (n_spikes <= 0 || m_pSpkList->get_spike_length() == 0)
			continue;

		switch (spike_classification_parameters_->i_parameter)
		{
		case 1: // value at t1
			m_pSpkList->measure_case1_amplitude_at_t(spike_classification_parameters_->i_cursor_t1);
			b_measure_done_ = TRUE;
			break;
		case 2: // value at t2
			m_pSpkList->measure_case1_amplitude_at_t(spike_classification_parameters_->i_cursor_t2);
			b_measure_done_ = TRUE;
			break;

		case 3: // value at t2- value at t1
			m_pSpkList->measure_case2_amplitude_at_t2_minus_at_t1(spike_classification_parameters_->i_cursor_t1, spike_classification_parameters_->i_cursor_t2);
			b_measure_done_ = TRUE;
			break;

		case 0: // max - min between t1 and t2
		case 4: // max-min vs t_max-t_min
		default:
			m_pSpkList->measure_case0_amplitude_min_to_max(spike_classification_parameters_->i_cursor_t1, spike_classification_parameters_->i_cursor_t2);
			break;
		}

		//save only if changed?
		m_pSpkDoc->OnSaveDocument(pdb_doc->db_get_current_spk_file_name(FALSE));
	}

	//if (m_b_all_files)
	//{
	//	index_current_file = pdb_doc->db_get_current_record_position();
	//	if (pdb_doc->db_set_current_record_position(index_current_file)) {
	//		m_pSpkDoc = pdb_doc->open_current_spike_file();
	//		m_pSpkList = m_pSpkDoc->get_spike_list_current();
	//	}
	//}

	chart_spike_shape_.set_source_data(m_pSpkList, GetDocument());
	chart_spike_bar_.set_source_data(m_pSpkList, GetDocument());

	chart_xt_measures_.horizontal_tags.set_tag_val(m_i_tag_low_, spike_classification_parameters_->lower_threshold);
	chart_xt_measures_.horizontal_tags.set_tag_val(m_i_tag_up_, spike_classification_parameters_->upper_threshold);

	build_histogram();
	chart_histogram_.vertical_tags.set_tag_val(m_i_tag_low_, spike_classification_parameters_->lower_threshold);
	chart_histogram_.vertical_tags.set_tag_val(m_i_tag_up_, spike_classification_parameters_->upper_threshold);

	update_gain();
	UpdateData(FALSE);
}

void ViewSpikeSort::update_gain()
{
	const auto y_we = static_cast<int>((xy_max_amplitude_mv - xy_min_amplitude_mv) / m_delta_mv_);
	const auto y_wo = static_cast<int>((xy_max_amplitude_mv + xy_min_amplitude_mv) / 2 / m_delta_mv_);

	chart_xt_measures_.set_yw_ext_org(y_we, y_wo);
	chart_histogram_.set_xw_ext_org(y_we, y_wo);// -y_we / 2);

	// get max min and center accordingly
	short max, min;
	m_pSpkList->get_total_max_min(FALSE, &max, &min);
	const auto middle = (max + min) / 2;
	chart_spike_shape_.set_yw_ext_org(y_we, middle);
	chart_spike_bar_.set_yw_ext_org(y_we, middle);

	all_charts_invalidate();
}

void ViewSpikeSort::on_format_all_data()
{
	// build new histogram only if necessary
	auto calculate_histogram = FALSE;

	// dots: spk file length
	if (l_first_ != 0 || l_last_ != m_pSpkDoc->get_acq_size() - 1)
	{
		l_first_ = 0;
		l_last_ = m_pSpkDoc->get_acq_size() - 1;

		if (spike_classification_parameters_->i_parameter != 4) // then, we need imax imin ...
			chart_xt_measures_.set_time_intervals(l_first_, l_last_);
		else
			chart_xt_measures_.set_time_intervals(-m_pSpkList->get_spike_length(), m_pSpkList->get_spike_length());
		chart_spike_shape_.set_time_intervals(l_first_, l_last_);
		chart_spike_bar_.set_time_intervals(l_first_, l_last_);

		chart_xt_measures_.Invalidate();
		chart_spike_shape_.Invalidate();
		chart_spike_bar_.Invalidate();
		calculate_histogram = TRUE;
	}

	// spikes: center spikes horizontally and adjust hz size of display

	const auto x_we = m_pSpkList->get_spike_length();
	if (x_we != chart_spike_shape_.get_xw_extent() || 0 != chart_spike_shape_.get_xw_org())
		chart_spike_shape_.set_xw_ext_org(x_we, 0);

	// change spk_hist_wnd_
	if (calculate_histogram)
		build_histogram();
	update_legends();
}

void ViewSpikeSort::build_histogram()
{
	const auto pdb_doc = GetDocument();
	if (pdb_doc == nullptr)
		return;

	const auto delta = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
	m_measure_y1_max_ = static_cast<int>(xy_max_amplitude_mv / delta);
	m_measure_y1_min_ = static_cast<int>(xy_min_amplitude_mv / delta);
	const int n_bins = static_cast<int>((xy_max_amplitude_mv - xy_min_amplitude_mv) / histogram_bin_size_mv);
	if (n_bins <= 0)
		return;
	chart_histogram_.build_hist_from_document(pdb_doc, b_all_files, l_first_, l_last_, m_measure_y1_max_, m_measure_y1_min_, n_bins, TRUE);
}

void ViewSpikeSort::on_format_center_curve()
{
	const auto n_spikes = m_pSpkList->get_spikes_count();
	for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		const auto spike = m_pSpkList->get_spike(i_spike);
		spike->set_spike_length(m_pSpkList->get_spike_length());
		spike->center_spike_amplitude(spike_classification_parameters_->i_cursor_t1, spike_classification_parameters_->i_cursor_t2, 1);
	}

	short max, min;
	m_pSpkList->get_total_max_min(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	chart_spike_shape_.set_yw_ext_org(chart_spike_shape_.get_yw_extent(), middle);
	chart_spike_bar_.set_yw_ext_org(chart_spike_shape_.get_yw_extent(), middle);

	update_legends();
}

void ViewSpikeSort::on_format_split_curves()
{
	on_format_gain_adjust();
}

void ViewSpikeSort::on_format_gain_adjust()
{
	// adjust gain
	short maxvalue, minvalue;
	GetDocument()->get_max_min_of_all_spikes(b_all_files, TRUE, &maxvalue, &minvalue);

	auto y_we = MulDiv(maxvalue - minvalue + 1, 10, 9);
	auto y_wo = (maxvalue + minvalue) / 2;
	chart_spike_shape_.set_yw_ext_org(y_we, y_wo);
	chart_spike_bar_.set_yw_ext_org(y_we, y_wo);
	//chart_spike_bar_.MaxCenter();

	// adjust gain for spk_hist_wnd_ and XYp: data = computed values
	// search max min of parameter values
	const CSize measure = GetDocument()->get_max_min_of_single_spike(b_all_files);
	maxvalue = static_cast<short>(measure.cx);
	minvalue = static_cast<short>(measure.cy);

	const auto delta = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
	const auto upper2 = static_cast<short>(histogram_upper_threshold / delta);
	const auto lower2 = static_cast<short>(histogram_lower_threshold / delta);
	if (upper2 > maxvalue)
		maxvalue = upper2;
	if (lower2 < minvalue)
		minvalue = lower2;
	y_we = MulDiv(maxvalue - minvalue + 1, 10, 8);
	y_wo = (maxvalue + minvalue) / 2;

	// update display
	chart_xt_measures_.set_yw_ext_org(y_we, y_wo);
	const auto y_max = static_cast<int>(chart_histogram_.GetHistMax());
	chart_histogram_.set_xw_ext_org(y_we, y_wo - y_we / 2);
	chart_histogram_.set_yw_ext_org(MulDiv(y_max, 10, 8), 0);

	// update edit controls
	xy_max_amplitude_mv = static_cast<float>(maxvalue) * delta;
	xy_min_amplitude_mv = static_cast<float>(minvalue) * delta;
	build_histogram();
	update_legends();
}

void ViewSpikeSort::select_spike(db_spike& spike_sel)
{
	if (spike_sel.database_position >= 0 || spike_sel.spike_list_index < 0) {
		const CdbWaveDoc* p_doc = chart_spike_shape_.get_db_wave_doc();
		spike_sel.database_position = p_doc->db_get_current_record_position();
		spike_sel.spike_list_index = p_doc->m_p_spk->get_spike_list_current_index();
	}
	chart_spike_shape_.select_spike(spike_sel);
	chart_spike_bar_.select_spike(spike_sel);
	chart_xt_measures_.select_spike(spike_sel);
	m_pSpkList->m_selected_spike = spike_sel.spike_index;

	m_spike_index_class = -1;
	auto n_cmd_show = SW_HIDE;
	if (spike_sel.spike_index >= 0)
	{
		const auto spike = m_pSpkList->get_spike(spike_sel.spike_index);
		if (spike != nullptr) {
			m_spike_index_class = spike->get_class_id();
			n_cmd_show = SW_SHOW;
		}
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_SPIKECLASS)->ShowWindow(n_cmd_show);
	m_spike_index = spike_sel.spike_index;
	UpdateData(FALSE);
}

boolean ViewSpikeSort::open_dat_and_spk_files_of_selected_spike(const db_spike& spike_coords)
{
	if (spike_coords.database_position >= 0)
	{
		if (GetDocument()->db_set_current_record_position(spike_coords.database_position))
		{
			const auto spk_name = GetDocument()->db_get_current_dat_file_name();
			GetDocument()->open_current_spike_file();
		}
		else
		{
			AfxMessageBox(_T("Unexpected error: Recording not found in the data base\n"), MB_OK);
			return false;
		}
	}

	const auto dat_name = GetDocument()->db_get_current_dat_file_name();
	auto flag = false;
	if (!dat_name.IsEmpty())
	{
		CFileStatus status;
		flag = CFile::GetStatus(dat_name, status);
	}
	if (flag)
		flag = (GetDocument()->open_current_data_file() != nullptr);

	return flag;
}

void ViewSpikeSort::on_tools_edit_spikes()
{
	db_spike spike_coords = GetDocument()->get_spike_hit();
	ASSERT(m_spike_index == spike_coords.spike_index);
	select_spike(spike_coords);

	if (!open_dat_and_spk_files_of_selected_spike(spike_coords))
		return;

	DlgSpikeEdit dlg;
	dlg.y_extent = chart_spike_shape_.get_yw_extent();
	dlg.y_zero = chart_spike_shape_.get_yw_org();
	dlg.x_extent = chart_spike_shape_.get_xw_extent();
	dlg.x_zero = chart_spike_shape_.get_xw_org();
	dlg.spike_index = m_spike_index;
	dlg.db_wave_doc = GetDocument();
	dlg.m_parent = this;
	dlg.DoModal();

	if (!dlg.b_artefact)
	{
		m_spike_index = dlg.spike_index;
		db_spike spike_sel(-1, -1, m_spike_index);
		select_spike(spike_sel);
	}

	if (dlg.b_changed)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
	}

	//update_legends();
	all_charts_invalidate();
	UpdateData(FALSE);
}

void ViewSpikeSort::on_select_all_files()
{
	b_all_files = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	b_measure_done_ = FALSE;
	on_measure();

	chart_spike_bar_.display_all_files(b_all_files, GetDocument());
	chart_spike_shape_.display_all_files(b_all_files, GetDocument());
	chart_xt_measures_.display_all_files(b_all_files, GetDocument());
}

void ViewSpikeSort::on_tools_align_spikes()
{
	// get source data
	auto b_doc_exist = FALSE;
	auto data_file_name = m_pSpkDoc->get_acq_filename();
	if (!data_file_name.IsEmpty())
	{
		CFileStatus status;
		b_doc_exist = CFile::GetStatus(data_file_name, status);
	}
	if (!b_doc_exist)
	{
		AfxMessageBox(_T("Source data not found - operation aborted"));
		return;
	}

	// first prepare array with SUM

	const auto spike_length = m_pSpkList->get_spike_length(); 
	const auto total_spikes = m_pSpkList->get_spikes_count(); 
	const auto p_sum0 = new double[spike_length]; // array with results / SUMy
	const auto p_cxy0 = new double[spike_length]; // temp array to store correlation
	auto* const p_mean0 = new short[spike_length]; // mean (template) / at scale
	const auto p_dummy0 = new short[spike_length]; // results of correlation / at scale

	// init pSUM with zeros
	auto p_sum = p_sum0;
	for (auto i = 0; i < spike_length; i++, p_sum++)
		*p_sum = 0;

	// compute mean
	auto nb_spk_selected_class = 0;
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		if (m_pSpkList->get_spike(i_spike)->get_class_id() != sort_source_class)
			continue;
		nb_spk_selected_class++;
		short* p_spk = m_pSpkList->get_spike(i_spike)->get_p_data();
		p_sum = p_sum0;
		for (auto i = 0; i < spike_length; i++, p_spk++, p_sum++)
			*p_sum += *p_spk;
	}

	// build avg and avg autocorrelation, then display
	auto p_mean = p_mean0;
	p_sum = p_sum0;

	for (auto i = 0; i < spike_length; i++, p_mean++, p_sum++)
		*p_mean = static_cast<short>(*p_sum / nb_spk_selected_class);

	chart_spike_shape_.display_ex_data(p_mean0);

	// for each spike, compute correlation and take max value correlation
	const auto k_start = spike_classification_parameters_->i_cursor_t1; // start of template match
	const auto k_end = spike_classification_parameters_->i_cursor_t2; // end of template match
	if (k_end <= k_start)
		return;
	const auto j0 = k_start - (k_end - k_start) / 2; // start time lag
	const auto j1 = k_end - (k_end - k_start) / 2 + 1; // last lag

	// compute autocorrelation for mean;
	double cxx_mean = 0;
	p_mean = p_mean0 + k_start;
	for (auto i = k_start; i < k_end; i++, p_mean++)
	{
		const auto val = static_cast<double>(*p_mean);
		cxx_mean += val * val;
	}

	// get parameters from document
	const auto p_dat_doc = GetDocument()->m_p_dat;
	p_dat_doc->open_document(data_file_name);
	const auto doc_chan = m_pSpkList->get_detection_parameters()->extract_channel; 
	const auto number_channels = static_cast<int>(p_dat_doc->get_waveformat()->scan_count); 
	const auto method = m_pSpkList->get_detection_parameters()->extract_transform;
	const auto spike_pre_trigger = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	const int offset = (method > 0) ? 1 : number_channels; 
	const int span = p_dat_doc->get_transformed_data_span(method); 

	// pre-load data
	auto ii_time0 = m_pSpkList->get_spike(0)->get_time(); 
	auto l_rw_first0 = ii_time0 - spike_length;
	auto l_rw_last0 = ii_time0 + spike_length;
	if (!p_dat_doc->load_raw_data(&l_rw_first0, &l_rw_last0, span))
		return;

	auto p_data = p_dat_doc->load_transformed_data(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto pre_trigger = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		Spike* spike = m_pSpkList->get_spike(i_spike);

		// exclude spikes that do not fall within time limits
		if (spike->get_class_id() != sort_source_class)
			continue;

		ii_time0 = spike->get_time();
		ii_time0 -= spike_pre_trigger;

		// make sure that source data are loaded and get pointer to it (p_data)
		auto l_rw_first = ii_time0 - spike_length; 
		auto l_rw_last = ii_time0 + spike_length; 
		if (ii_time0 > l_last_ || ii_time0 < l_first_)
			continue;
		if (!p_dat_doc->load_raw_data(&l_rw_first, &l_rw_last, span))
			break; 

		// load data only if necessary
		if (l_rw_first != l_rw_first0 || l_rw_last != l_rw_last0)
		{
			p_data = p_dat_doc->load_transformed_data(l_rw_first, l_rw_last, method, doc_chan);
			l_rw_last0 = l_rw_last; 
			l_rw_first0 = l_rw_first;
		}

		// pointer to first point of spike
		auto p_data_spike0 = p_data + (ii_time0 - l_rw_first) * offset;

		// for spike i_spk: loop over spike_len time lags centered over interval center

		// compute autocorrelation & cross correlation at first time lag
		auto p_cxy_lag = p_cxy0; // pointer to array with correlation coefficients
		*p_cxy_lag = 0;
		auto p_data_k0 = p_data_spike0 + j0 * offset; // source data start

		// loop over all time lag requested
		for (auto j = j0; j < j1; j++, p_cxy_lag++, p_data_k0 += offset)
		{
			*p_cxy_lag = 0;

			// add cross product for each point: data * meanlong ii_time
			auto p_mean_k = p_mean0 + k_start; // first point / template
			short* p_data_k = p_data_k0; // first data point
			double cxx_spike = 0; // autocorrelation

			// loop over all points of source data and mean
			for (auto k = k_start; k < k_end; k++, p_mean_k++, p_data_k += offset)
			{
				const auto val = static_cast<double>(*p_data_k);
				*p_cxy_lag += static_cast<double>(*p_mean_k) * val;
				cxx_spike += val * val;
			}

			*p_cxy_lag /= (static_cast<double>(k_end) - k_start + 1);
			*p_cxy_lag = *p_cxy_lag / sqrt(cxx_mean * cxx_spike);
		}

		// get max and min of this correlation
		auto p_cxy = p_cxy0;
		auto cxy_max = *p_cxy; // correlation max value
		auto i_cxy_max = 0; // correlation max index
		for (auto i = 0; i < k_end - k_start; p_cxy++, i++)
		{
			if (cxy_max < *p_cxy) // get max and max position
			{
				cxy_max = *p_cxy;
				i_cxy_max = i;
			}
		}

		// offset spike so that max is at spike_len/2
		const auto j_decal = i_cxy_max - (k_end - k_start) / 2;
		if (j_decal != 0)
		{
			p_data_spike0 = p_data + static_cast<WORD>(ii_time0 + j_decal - l_rw_first) * offset + doc_chan;
			spike->transfer_data_to_spike_buffer(p_data_spike0, number_channels, spike_length);
			m_pSpkDoc->SetModifiedFlag(TRUE);
			spike->set_time(ii_time0 + pre_trigger);
		}

		// now offset spike vertically to align it with the mean
		spike->offset_spike_data_to_average_ex(k_start, k_end);
	}

	// exit : delete resources used locally
	if (m_pSpkDoc->IsModified())
	{
		chart_spike_shape_.Invalidate();
		chart_spike_shape_.display_ex_data(p_mean0);
	}

	delete[] p_sum0;
	delete[] p_mean0;
	delete[] p_cxy0;
	delete[] p_dummy0;

	on_measure();
}

void ViewSpikeSort::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// view scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		dbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	// trap messages from ScrollBarEx
	CString cs;
	switch (nSBCode)
	{
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		m_file_scroll_.GetScrollInfo(&m_file_scroll_infos_, SIF_ALL);
		l_first_ = m_file_scroll_infos_.nPos;
		l_last_ = l_first_ + static_cast<long>(m_file_scroll_infos_.nPage) - 1;
		break;

	default:
		scroll_file(nSBCode, nPos);
		break;
	}
	update_legends();
}

void ViewSpikeSort::scroll_file(UINT n_sb_code, UINT n_pos)
{
	// get corresponding data
	const auto total_scroll = m_pSpkDoc->get_acq_size();
	const auto page_scroll = (l_last_ - l_first_);
	auto sb_scroll = MulDiv(page_scroll, 10, 100);
	if (sb_scroll == 0)
		sb_scroll = 1;
	auto l_first = l_first_;
	switch (n_sb_code)
	{
	case SB_LEFT: l_first = 0; break; // Scroll to far left.
	case SB_LINELEFT: l_first -= sb_scroll; break; // Scroll left.
	case SB_LINERIGHT: l_first += sb_scroll; break; // Scroll right
	case SB_PAGELEFT: l_first -= page_scroll; break; // Scroll one page left
	case SB_PAGERIGHT: l_first += page_scroll; break; // Scroll one page right.
	case SB_RIGHT: l_first = total_scroll - page_scroll + 1;break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		l_first = static_cast<int>(n_pos);break;
	default:return;
	}

	if (l_first < 0)
		l_first = 0;
	auto l_last = l_first + page_scroll;

	if (l_last >= total_scroll)
	{
		l_last = total_scroll - 1;
		l_first = l_last - page_scroll;
	}

	l_first_ = l_first;
	l_last_ = l_last;
}

void ViewSpikeSort::update_file_scroll()
{
	m_file_scroll_infos_.fMask = SIF_PAGE | SIF_POS;
	m_file_scroll_infos_.nPos = l_first_;
	m_file_scroll_infos_.nPage = l_last_ - l_first_ + 1;
	m_file_scroll_.SetScrollInfo(&m_file_scroll_infos_);
}

void ViewSpikeSort::select_spike_list(int current_index)
{
	m_pSpkList = m_pSpkDoc->set_spike_list_as_current(current_index);
	ASSERT(m_pSpkList != NULL);
	on_measure();

	all_charts_set_spike_list(m_pSpkList);
	all_charts_invalidate();
}

void ViewSpikeSort::on_en_change_edit_left2()
{
	if (mm_t_xy_left_.m_bEntryDone)
	{
		const auto delta = m_time_unit / m_pSpkList->get_acq_sampling_rate();
		mm_t_xy_left_.OnEnChange(this, t_xy_left, delta, -delta);
		// check boundaries
		if (t_xy_left >= t_xy_right)
			t_xy_left = t_xy_right - delta;

		// change display if necessary
		const auto left = t_xy_left / delta;
		const auto it_left = static_cast<int>(left);
		if (it_left != chart_xt_measures_.vertical_tags.get_value(m_i_xy_left_))
		{
			spike_classification_parameters_->i_xy_left = it_left;
			chart_xt_measures_.move_vt_tag(m_i_xy_left_, it_left);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_edit_right2()
{
	if (mm_t_xy_right_.m_bEntryDone)
	{
		const auto delta = m_time_unit / m_pSpkList->get_acq_sampling_rate();
		mm_t_xy_right_.OnEnChange(this, t_xy_right, delta, -delta);

		// check boundaries
		if (t_xy_right <= t_xy_left)
			t_xy_right = t_xy_left + delta;

		// change display if necessary
		const auto right = t_xy_right / delta;
		const auto i_right = static_cast<int>(right);
		if (i_right != chart_xt_measures_.vertical_tags.get_value(m_i_xy_right_))
		{
			spike_classification_parameters_->i_xy_right = i_right;
			chart_xt_measures_.move_vt_tag(m_i_xy_right_, i_right);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::all_charts_invalidate()
{
	chart_spike_shape_.Invalidate();
	chart_spike_bar_.Invalidate();
	chart_xt_measures_.Invalidate();
	chart_histogram_.Invalidate();
}

void  ViewSpikeSort::all_charts_set_spike_list(SpikeList* spk_list)
{
	chart_histogram_.set_spike_list(spk_list);
	chart_spike_shape_.set_spike_list(spk_list);
	chart_spike_bar_.set_spike_list(spk_list);
	chart_xt_measures_.set_spike_list(spk_list);
}

void ViewSpikeSort::on_en_change_source_spike_class()
{
	if (mm_source_class_.m_bEntryDone)
	{
		const auto source_class = sort_source_class;
		mm_source_class_.OnEnChange(this, sort_source_class, 1, -1);
		if (sort_source_class != source_class)
		{
			chart_spike_shape_.set_plot_mode(PLOT_ONE_COLOR, sort_source_class);
			chart_xt_measures_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);
			chart_histogram_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);
			chart_spike_bar_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class);
		}
		all_charts_invalidate();

		auto db_sel = db_spike(-1, -1, -1);
		select_spike(db_sel);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_destination_spike_class()
{
	if (mm_destination_class_.m_bEntryDone)
	{
		mm_destination_class_.OnEnChange(this, sort_destination_class, 1, -1);
		auto db_sel = db_spike(-1, -1, -1);
		select_spike(db_sel);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_select_change_parameter()
{
	const auto i_parameter = m_combo_parameter.GetCurSel();
	if (i_parameter != spike_classification_parameters_->i_parameter)
	{
		spike_classification_parameters_->i_parameter = i_parameter;
		activate_mode4();
		on_measure();
		on_format_center_curve();
	}

	//    STATIC3 lower STATIC4 upper STATIC5 T1 STATIC6 T2
	// 0  mV      vis    mV      vis    vis     vis  vis   vis
	// 1  mV      vis    mV      vis    vis     vis  NOT   NOT
	// 2  mS      vis    mS      vis    vis     vis  vis   vis
}

void ViewSpikeSort::check_valid_threshold_limits()
{
	if (histogram_lower_threshold >= histogram_upper_threshold|| histogram_lower_threshold < 0) 
		histogram_lower_threshold = 0;

	if (histogram_upper_threshold <= histogram_lower_threshold)
		histogram_upper_threshold = histogram_lower_threshold + 0.5f; // m_delta_ * 10.f;
}

void ViewSpikeSort::on_en_change_lower()
{
	if (mm_limit_lower_.m_bEntryDone)
	{
		//m_delta_ = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		mm_limit_lower_.OnEnChange(this, histogram_lower_threshold, m_delta_mv_, -m_delta_mv_);
		check_valid_threshold_limits();

		spike_classification_parameters_->lower_threshold = static_cast<int>(histogram_lower_threshold / m_delta_mv_);
		if (spike_classification_parameters_->lower_threshold != chart_histogram_.vertical_tags.get_value(m_spk_hist_lower_threshold_))
		{
			chart_histogram_.move_vt_tag_to_val(m_spk_hist_lower_threshold_, spike_classification_parameters_->lower_threshold);
			chart_xt_measures_.move_hz_tag(m_i_tag_low_, spike_classification_parameters_->lower_threshold);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_upper()
{
	if (mm_limit_upper_.m_bEntryDone)
	{
		//m_delta_ = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		mm_limit_upper_.OnEnChange(this, histogram_upper_threshold,m_delta_mv_, -m_delta_mv_);
		// check boundaries
		check_valid_threshold_limits();

		spike_classification_parameters_->upper_threshold = static_cast<int>(histogram_upper_threshold / m_delta_mv_);
		if (spike_classification_parameters_->lower_threshold != chart_histogram_.vertical_tags.get_value(m_spk_hist_upper_threshold_))
		{
			chart_histogram_.move_vt_tag_to_val(m_spk_hist_upper_threshold_, spike_classification_parameters_->upper_threshold);
			chart_xt_measures_.move_hz_tag(m_i_tag_up_, spike_classification_parameters_->upper_threshold);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_t1()
{
	if (mm_t1_.m_bEntryDone)
	{
		mm_t1_.OnEnChange(this, spike_shape_t1, m_delta_t_, -m_delta_t_);
		// check boundaries
		if (spike_shape_t1 < 0)
			spike_shape_t1 = 0.0f;
		if (spike_shape_t1 >= spike_shape_t2)
			spike_shape_t1 = spike_shape_t2 - m_delta_t_;
	
		const auto it1 = static_cast<int>(spike_shape_t1 / m_delta_t_);
		if (it1 != chart_spike_shape_.vertical_tags.get_value(m_spk_form_tag_left_))
		{
			spike_classification_parameters_->i_cursor_t1 = it1;
			chart_spike_shape_.move_vt_track(m_spk_form_tag_left_, spike_classification_parameters_->i_cursor_t1);
			m_pSpkList->m_i_max_min_1_sl = spike_classification_parameters_->i_cursor_t1;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_t2()
{
	if (mm_t2_.m_bEntryDone)
	{
		mm_t2_.OnEnChange(this, spike_shape_t2, m_delta_t_, -m_delta_t_);

		// check boundaries
		if (spike_shape_t2 < spike_shape_t1)
			spike_shape_t2 = spike_shape_t1 + m_delta_t_;
		const auto t_max = (static_cast<float>(m_pSpkList->get_spike_length()) - 1.f) * m_delta_t_;
		if (spike_shape_t2 >= t_max)
			spike_shape_t2 = t_max;

		const auto it2 = static_cast<int>(spike_shape_t2 / m_delta_t_);
		if (it2 != chart_spike_shape_.vertical_tags.get_value(m_spk_form_tag_right_))
		{
			spike_classification_parameters_->i_cursor_t2 = it2;
			chart_spike_shape_.move_vt_track(m_spk_form_tag_right_, spike_classification_parameters_->i_cursor_t2);
			m_pSpkList->m_i_max_min_2_sl = spike_classification_parameters_->i_cursor_t2;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_time_first()
{
	if (mm_time_first_.m_bEntryDone)
	{
		mm_time_first_.OnEnChange(this, display_bars_time_left, 1.f, -1.f);

		// check boundaries
		if (display_bars_time_left < 0.f)
			display_bars_time_left = 0.f;
		if (display_bars_time_left >= display_bars_time_last)
			display_bars_time_left = 0.f;

		l_first_ = static_cast<long>(display_bars_time_left * m_pSpkList->get_acq_sampling_rate());
		update_legends();
	}
}

void ViewSpikeSort::on_en_change_time_last()
{
	if (mm_time_last_.m_bEntryDone)
	{
		mm_time_last_.OnEnChange(this, display_bars_time_last, 1.f, -1.f);

		// check boundaries
		if (display_bars_time_last <= display_bars_time_left)
			l_last_ = static_cast<long>(static_cast<float>(m_pSpkDoc->get_acq_size() - 1) / m_pSpkList->get_acq_sampling_rate());

		l_last_ = static_cast<long>(display_bars_time_last * m_pSpkList->get_acq_sampling_rate());
		update_legends();
	}
}

void ViewSpikeSort::on_en_change_mv_min()
{
	if (mm_mv_min_.m_bEntryDone)
	{
		mm_mv_min_.OnEnChange(this, xy_min_amplitude_mv, 1.f, -1.f);

		// check boundaries
		if (xy_min_amplitude_mv >= xy_max_amplitude_mv)
			xy_min_amplitude_mv = xy_max_amplitude_mv - 1.f;

		// change display if necessary
		update_gain();
		update_legends();
	}
}

void ViewSpikeSort::on_en_change_mv_max()
{
	if (mm_mv_max_.m_bEntryDone)
	{
		mm_mv_max_.OnEnChange(this, xy_max_amplitude_mv, 1.f, -1.f);
		if (xy_max_amplitude_mv <= xy_min_amplitude_mv)
			xy_max_amplitude_mv = xy_min_amplitude_mv + 1.f;

		update_gain();
		update_legends();
	}
}

void ViewSpikeSort::on_en_change_no_spike()
{
	if (mm_spike_index_.m_bEntryDone)
	{
		const int spike_index = m_spike_index;
		mm_spike_index_.OnEnChange(this, m_spike_index, 1, -1);

		// check boundaries
		if (m_spike_index < 0)
			m_spike_index = -1;
		if (m_spike_index >= m_pSpkList->get_spikes_count())
			m_spike_index = m_pSpkList->get_spikes_count() - 1;

		if (m_spike_index != spike_index)
		{
			if (m_spike_index >= 0)
			{
				// test if spike visible in the current time interval
				const auto spike_element = m_pSpkList->get_spike(m_spike_index);
				const auto spk_first = spike_element->get_time() - m_pSpkList->get_detection_parameters()->detect_pre_threshold;
				const auto spk_last = spk_first + m_pSpkList->get_spike_length();

				if (spk_first < l_first_ || spk_last > l_last_)
				{
					const auto l_span = (l_last_ - l_first_) / 2;
					const auto l_center = (spk_last + spk_first) / 2;
					l_first_ = l_center - l_span;
					l_last_ = l_center + l_span;
					update_legends();
				}
			}
		}
		auto db_sel = db_spike(-1, -1, m_spike_index);
		select_spike(db_sel);
	}
}

void ViewSpikeSort::on_en_change_spike_class()
{
	if (mm_spike_index_class_.m_bEntryDone)
	{
		const auto spike_index_class = m_spike_index_class;
		mm_spike_index_class_.OnEnChange(this, m_spike_index_class, 1, -1);

		if (m_spike_index_class != spike_index_class)
		{
			m_pSpkDoc->SetModifiedFlag(TRUE);
			const auto current_list = m_tabCtrl.GetCurSel();
			auto* spike_list = m_pSpkDoc->set_spike_list_as_current(current_list);
			spike_list->get_spike(m_spike_index)->set_class_id(m_spike_index_class);
			update_legends();
		}
	}
}

void ViewSpikeSort::on_en_change_n_bins()
{
	if (mm_mv_bin_.m_bEntryDone)
	{
		const auto bin_mv = histogram_bin_size_mv;
		const auto delta = (xy_max_amplitude_mv - xy_min_amplitude_mv) / 10.f;
		mm_mv_bin_.OnEnChange(this, histogram_bin_size_mv, delta, -delta);

		if (histogram_bin_size_mv != bin_mv)
		{
			build_histogram();
			update_legends();
		}
	}
}
