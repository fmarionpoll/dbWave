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

IMPLEMENT_DYNCREATE(ViewSpikeSort, ViewDbTable)

ViewSpikeSort::ViewSpikeSort()
	: ViewDbTable(IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

ViewSpikeSort::~ViewSpikeSort()
{
	if (p_spk_doc != nullptr)
		save_current_spk_file(); 

	if (spike_classification_ != nullptr)
	{
		spike_classification_->b_changed = TRUE;
		spike_classification_->source_class = sort_source_class_;
		spike_classification_->dest_class = sort_destination_class_;
		spike_classification_->mv_max = measure_max_mv_;
		spike_classification_->mv_min = measure_min_mv_;
	}
}

void ViewSpikeSort::DoDataExchange(CDataExchange* p_dx)
{
	ViewDbTable::DoDataExchange(p_dx);

	DDX_Control(p_dx, IDC_MEASURE_TYPE, combo_measure_type_);
	DDX_Text(p_dx, IDC_SHAPE_T1_MS, shape_t1_ms_);
	DDX_Text(p_dx, IDC_SHAPE_T2_MS, shape_t2_ms_);
	DDX_Text(p_dx, IDC_LOWER_THRESHOLD_MV, lower_threshold_mv_);
	DDX_Text(p_dx, IDC_UPPER_THRESHOLD_MV, upper_threshold_mv_);
	DDX_Text(p_dx, IDC_SOURCE_CLASS, sort_source_class_);
	DDX_Text(p_dx, IDC_DESTINATION_CLASS, sort_destination_class_);
	DDX_Text(p_dx, IDC_TIME_FIRST, time_first_s_);
	DDX_Text(p_dx, IDC_TIME_LAST, time_last_s_);
	DDX_Text(p_dx, IDC_MEASURE_MAX_MV, measure_max_mv_);
	DDX_Text(p_dx, IDC_MEASURE_MIN_MV, measure_min_mv_);
	DDX_Text(p_dx, IDC_HISTOGRAM_BIN_MS, histogram_bin_mv_);
	DDX_Check(p_dx, IDC_ALL_FILES, b_all_files_);
	DDX_Text(p_dx, IDC_SPIKE_INDEX, spike_index_);
	DDX_Text(p_dx, IDC_SPIKE_CLASS, spike_class_index_);
	DDX_Text(p_dx, IDC_EDIT_RIGHT, t_xy_right_);
	DDX_Text(p_dx, IDC_EDIT_LEFT, t_xy_left_);

	DDX_Control(p_dx, IDC_TAB1, spk_list_tab_ctrl);
	//DDX_Control(p_dx, IDC_MFCPROPERTYGRID1, property_grid);
}

BEGIN_MESSAGE_MAP(ViewSpikeSort, ViewDbTable)

	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_CTLCOLOR()

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikeSort::on_my_message)

	ON_COMMAND(ID_FORMAT_VIEW_ALL_DATA_ON_ABSCISSA, &ViewSpikeSort::on_view_all_data_on_abscissa)
	ON_COMMAND(ID_FORMAT_CENTER_CURVE, &ViewSpikeSort::on_format_center_curve)
	ON_COMMAND(ID_FORMAT_GAIN_ADJUST, &ViewSpikeSort::on_format_gain_adjust)
	ON_COMMAND(ID_FORMAT_SPLIT_CURVES, &ViewSpikeSort::on_format_split_curves)
	ON_COMMAND(ID_TOOLS_EDIT_SPIKES, &ViewSpikeSort::on_tools_edit_spikes)
	ON_COMMAND(ID_TOOLS_ALIGN_SPIKES, &ViewSpikeSort::on_tools_align_spikes)

	ON_BN_CLICKED(IDC_EXECUTE, &ViewSpikeSort::on_sort)
	ON_BN_CLICKED(IDC_MEASURE, &ViewSpikeSort::on_measure_parameters_from_spikes)
	ON_BN_CLICKED(IDC_ALL_FILES, &ViewSpikeSort::on_select_all_files)

	ON_CBN_SELCHANGE(IDC_MEASURE_TYPE, &ViewSpikeSort::on_select_change_measure_type)

	ON_EN_CHANGE(IDC_LOWER_THRESHOLD_MV, &ViewSpikeSort::on_en_change_lower_threshold)
	ON_EN_CHANGE(IDC_UPPER_THRESHOLD_MV, &ViewSpikeSort::on_en_change_upper_threshold)
	ON_EN_CHANGE(IDC_SOURCE_CLASS, &ViewSpikeSort::on_en_change_source_class)
	ON_EN_CHANGE(IDC_DESTINATION_CLASS, &ViewSpikeSort::on_en_change_destination_class)
	ON_EN_CHANGE(IDC_SHAPE_T1_MS, &ViewSpikeSort::on_en_change_shape_t1)
	ON_EN_CHANGE(IDC_SHAPE_T2_MS, &ViewSpikeSort::on_en_change_shape_t2)
	ON_EN_CHANGE(IDC_TIME_FIRST, &ViewSpikeSort::on_en_change_time_first)
	ON_EN_CHANGE(IDC_TIME_LAST, &ViewSpikeSort::on_en_change_time_last)
	ON_EN_CHANGE(IDC_MEASURE_MIN_MV, &ViewSpikeSort::on_en_change_min_mv)
	ON_EN_CHANGE(IDC_MEASURE_MAX_MV, &ViewSpikeSort::on_en_change_max_mv)
	ON_EN_CHANGE(IDC_EDIT_LEFT, &ViewSpikeSort::on_en_change_edit_left2)
	ON_EN_CHANGE(IDC_EDIT_RIGHT, &ViewSpikeSort::on_en_change_edit_right2)
	ON_EN_CHANGE(IDC_SPIKE_INDEX, &ViewSpikeSort::on_en_change_spike_index)
	ON_EN_CHANGE(IDC_LIST_INDEX, &ViewSpikeSort::on_en_change_list_index)
	ON_EN_CHANGE(IDC_FILE_INDEX, &ViewSpikeSort::on_en_change_file_index)
	ON_EN_CHANGE(IDC_SPIKE_CLASS, &ViewSpikeSort::on_en_change_spike_class)
	ON_EN_CHANGE(IDC_HISTOGRAM_BIN_MS, &ViewSpikeSort::on_en_change_hist_bin_ms)

	ON_BN_DOUBLECLICKED(IDC_CHART_MEASURE, &ViewSpikeSort::on_tools_edit_spikes)

	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)

END_MESSAGE_MAP()

void ViewSpikeSort::define_sub_classed_items()
{
	// subclass some controls
	VERIFY(chart_histogram_.SubclassDlgItem(IDC_CHART_HISTOGRAM, this));
	VERIFY(chart_measures_.SubclassDlgItem(IDC_CHART_MEASURE, this));
	VERIFY(chart_shape_.SubclassDlgItem(IDC_CHART_SHAPE, this));
	VERIFY(chart_spike_bar_.SubclassDlgItem(IDC_CHART_BARS, this));
	VERIFY(classes_grid_.SubclassDlgItem(IDC_GRID_PLACEHOLDER, this));

	VERIFY(mm_shape_t1_ms_.SubclassDlgItem(IDC_SHAPE_T1_MS, this));
	VERIFY(mm_shape_t2_ms_.SubclassDlgItem(IDC_SHAPE_T2_MS, this));
	VERIFY(mm_lower_threshold_mv_.SubclassDlgItem(IDC_LOWER_THRESHOLD_MV, this));
	VERIFY(mm_upper_threshold_mv_.SubclassDlgItem(IDC_UPPER_THRESHOLD_MV, this));
	VERIFY(mm_source_class_.SubclassDlgItem(IDC_SOURCE_CLASS, this));
	VERIFY(mm_destination_class_.SubclassDlgItem(IDC_DESTINATION_CLASS, this));
	VERIFY(mm_time_first_s_.SubclassDlgItem(IDC_TIME_FIRST, this));
	VERIFY(mm_time_last_s_.SubclassDlgItem(IDC_TIME_LAST, this));
	VERIFY(mm_measure_max_mv_.SubclassDlgItem(IDC_MEASURE_MAX_MV, this));
	VERIFY(mm_measure_min_mv_.SubclassDlgItem(IDC_MEASURE_MIN_MV, this));
	VERIFY(mm_t_xy_right_.SubclassDlgItem(IDC_EDIT_RIGHT, this));
	VERIFY(mm_t_xy_left_.SubclassDlgItem(IDC_EDIT_LEFT, this));
	VERIFY(mm_histogram_bin_mv_.SubclassDlgItem(IDC_HISTOGRAM_BIN_MS, this));

	VERIFY(mm_file_index_.SubclassDlgItem(IDC_FILE_INDEX, this));
	mm_file_index_.ShowScrollBar(SB_VERT);
	VERIFY(mm_list_index_.SubclassDlgItem(IDC_LIST_INDEX, this));
	mm_list_index_.ShowScrollBar(SB_VERT);
	VERIFY(mm_spike_index_.SubclassDlgItem(IDC_SPIKE_INDEX, this));
	mm_spike_index_.ShowScrollBar(SB_VERT);
	VERIFY(mm_class_index_.SubclassDlgItem(IDC_SPIKE_CLASS, this));
	mm_class_index_.ShowScrollBar(SB_VERT);

	VERIFY(m_file_scroll_.SubclassDlgItem(IDC_FILESCROLL, this));
	m_file_scroll_.SetScrollRange(0, 100, FALSE);
}

void ViewSpikeSort::define_stretch_parameters()
{
	stretch_.attach_parent(this);

	stretch_.new_prop(IDC_MEASURE_MIN_MV, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_CHART_MEASURE, XLEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_CHART_BARS, XLEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_TIME_LAST, SZEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_STATICRIGHT, SZEQ_XREQ, SZEQ_YBEQ);

	stretch_.new_prop(IDC_STATICLEFT, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_STATIC12, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_EDIT_LEFT, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_EDIT_RIGHT, SZEQ_XLEQ, SZEQ_YBEQ);
}

void ViewSpikeSort::init_charts_from_saved_parameters()
{
	b_init_ = TRUE;
	m_auto_increment = true;
	m_auto_detect = true;

	// load global parameters
	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	spike_classification_ = &(p_app->spk_classification);
	options_view_data_ = &(p_app->options_view_data);

	// assign values to controls
	combo_measure_type_.SetCurSel(spike_classification_->i_parameter);
	measure_max_mv_ = spike_classification_->mv_max;
	measure_min_mv_ = spike_classification_->mv_min;

	sort_source_class_ = spike_classification_->source_class;
	sort_destination_class_ = spike_classification_->dest_class;
	update_destination_class_descriptor();

	chart_shape_.display_all_files(false, GetDocument());
	chart_shape_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class_); // PLOT_ONE_COLOR
	chart_shape_.set_scope_parameters(&(options_view_data_->spk_sort1_spk));
	shape_t1_ = chart_shape_.vt_tags.add_tag(spike_classification_->shape_t1, 0);
	shape_t2_ = chart_shape_.vt_tags.add_tag(spike_classification_->shape_t2, 0);

	chart_measures_.display_all_files(false, GetDocument());
	chart_measures_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class_);
	chart_measures_.set_scope_parameters(&(options_view_data_->spk_sort1_parameters));
	tag_index_measures_up_ = chart_measures_.hz_tags.add_tag(spike_classification_->upper_threshold, 0);
	tag_index_measures_low_ = chart_measures_.hz_tags.add_tag(spike_classification_->lower_threshold, 0);

	chart_spike_bar_.display_all_files(false, GetDocument());
	chart_spike_bar_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class_);
	chart_spike_bar_.set_scope_parameters(&(options_view_data_->spk_sort1_bars));

	chart_histogram_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class_);
	chart_histogram_.set_scope_parameters(&(options_view_data_->spk_sort1_hist));
	tag_index_hist_up_ = chart_histogram_.vt_tags.add_tag(spike_classification_->upper_threshold, 0);
	tag_index_hist_low_ = chart_histogram_.vt_tags.add_tag(spike_classification_->lower_threshold, 0);
}


void ViewSpikeSort::OnInitialUpdate()
{
	define_sub_classed_items();
	define_stretch_parameters();
	ViewDbTable::OnInitialUpdate();

	// Create properties pane
	classes_grid_.create_grid();
	classes_grid_.get_property_list()->init_header();

	init_charts_from_saved_parameters();
	update_file_parameters();
	b_measure_done_ = FALSE;
	on_measure_parameters_from_spikes();
	select_spike();

	if (nullptr != p_spk_list)
	{
		lower_threshold_mv_ = static_cast<float>(spike_classification_->lower_threshold) * delta_mv_;
		upper_threshold_mv_ = static_cast<float>(spike_classification_->upper_threshold) * delta_mv_;
		UpdateData(false);
	}

	activate_mode4();
	gain_adjust_shape_and_bars();


}

void ViewSpikeSort::activate_mode4()
{
	auto n_cmd_show = SW_HIDE;
	if (4 != spike_classification_->i_parameter)
	{
		chart_measures_.vt_tags.remove_all_tags();
		chart_measures_.set_n_x_scale_cells(0, 0, 0);
	}
	else
	{
		n_cmd_show = SW_SHOW;
		if (1 > chart_measures_.vt_tags.get_tag_list_size())
		{
			m_i_xy_right_ = chart_measures_.vt_tags.add_tag(spike_classification_->i_xy_right, 0);
			m_i_xy_left_ = chart_measures_.vt_tags.add_tag(spike_classification_->i_xy_left, 0);
			const auto delta = p_spk_list->get_acq_sampling_rate() / time_unit_;
			t_xy_right_ = static_cast<float>(spike_classification_->i_xy_right) / delta;
			t_xy_left_ = static_cast<float>(spike_classification_->i_xy_left) / delta;
		}
		chart_measures_.set_n_x_scale_cells(2, 0, 0);
		chart_measures_.get_scope_parameters()->cr_scope_grid = col_light_grey;

		if (nullptr != p_spk_list)
		{
			const auto spike_length_ms = (static_cast<double>(static_cast<float>(p_spk_list->get_spike_length()) * time_unit_ / p_spk_list->get_acq_sampling_rate()));
			CString cs_dummy;
			cs_dummy.Format(_T("%0.1f ms"), spike_length_ms);
			GetDlgItem(IDC_STATICRIGHT)->SetWindowText(cs_dummy);
			cs_dummy.Format(_T("%0.1f ms"), -spike_length_ms);
			GetDlgItem(IDC_STATICLEFT)->SetWindowText(cs_dummy);
		}
	}
	

	show_controls_for_mode4(n_cmd_show);
	chart_measures_.Invalidate();
}

void ViewSpikeSort::show_controls_for_mode4(const int n_cmd_show) const
{
	GetDlgItem(IDC_STATICRIGHT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATICLEFT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC12)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT_RIGHT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT_LEFT)->ShowWindow(n_cmd_show);
}

void ViewSpikeSort::OnActivateView(const BOOL b_activate, CView* p_activate_view, CView* p_deactive_view)
{
	if (b_activate)
	{
		auto* p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATE_VIEW,
			reinterpret_cast<LPARAM>(p_activate_view->GetDocument()));
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
	ViewDbTable::OnActivateView(b_activate, p_activate_view, p_deactive_view);
}

BOOL ViewSpikeSort::OnMove(const UINT n_id_move_command)
{
	save_current_spk_file();
	return ViewDbTable::OnMove(n_id_move_command);
}

void ViewSpikeSort::OnUpdate(CView* p_sender, const LPARAM l_hint, CObject* p_hint)
{
	if (b_init_)
	{
		switch (LOWORD(l_hint))
		{
		case HINT_DOC_HAS_CHANGED:
		case HINT_DOC_MOVE_RECORD:
		case HINT_REQUERY:
			update_file_parameters();
			b_measure_done_ = FALSE;
			on_measure_parameters_from_spikes();
			select_spike();
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

void ViewSpikeSort::load_current_spike_file()
{
	p_spk_doc = GetDocument()->open_current_spike_file();

	if (nullptr != p_spk_doc)
	{
		p_spk_doc->SetModifiedFlag(FALSE);
		p_spk_doc->SetPathName(GetDocument()->db_get_current_spk_file_name(), FALSE);
		const int current_index = GetDocument()->get_current_spike_file()->get_index_current_spike_list();
		p_spk_list = p_spk_doc->set_index_current_spike_list(current_index);
	}
}

void ViewSpikeSort::update_file_parameters()
{
	const BOOL first_update = p_spk_doc == nullptr;
	load_current_spike_file();
	if (p_spk_doc == nullptr)
		return;

	// update Tab at the bottom
	spk_list_tab_ctrl.init_ctrl_tab_from_spike_doc(p_spk_doc);
	spk_list_tab_ctrl.SetCurSel(GetDocument()->get_current_spike_file()->get_index_current_spike_list());

	if (first_update || options_view_data_->b_complete_record)
	{
		if (p_spk_doc != nullptr)
		{
			time_first_s_ = 0.f;
			time_last_s_ = (static_cast<float>(p_spk_doc->get_acq_size()) - 1) / p_spk_doc->get_acq_rate();
		}
	}

	if (p_spk_list != nullptr)
	{
		sampling_rate_ = p_spk_doc->get_acq_rate();
		l_first_ = static_cast<long>(time_first_s_ * sampling_rate_);
		l_last_ = static_cast<long>(time_last_s_ * sampling_rate_);
		delta_ms_ = time_unit_ / sampling_rate_;
		delta_mv_ = p_spk_list->get_acq_volts_per_bin() * mv_unit_;

		if (0 == spike_classification_->shape_t1 && 0 == spike_classification_->shape_t2)
		{
			spike_classification_->shape_t1 = p_spk_list->get_detection_parameters()->detect_pre_threshold;
			spike_classification_->shape_t2 = spike_classification_->shape_t1 + p_spk_list->get_detection_parameters()->detect_refractory_period;
		}
	}

	chart_shape_.set_source_data(p_spk_list, GetDocument());
	chart_shape_.get_extents_current_spk_list();
	shape_t1_ms_ = static_cast<float>(spike_classification_->shape_t1) * delta_ms_;
	shape_t2_ms_ = static_cast<float>(spike_classification_->shape_t2) * delta_ms_;

	chart_spike_bar_.set_source_data(p_spk_list, GetDocument());

	chart_measures_.set_source_data(p_spk_list, GetDocument());

	if (!b_all_files_)
		chart_histogram_.clear_data();

	m_file_scroll_infos_.fMask = SIF_ALL;
	m_file_scroll_infos_.nMin = 0;
	m_file_scroll_infos_.nMax = p_spk_doc->get_acq_size() - 1;
	m_file_scroll_infos_.nPos = 0;
	m_file_scroll_infos_.nPage = p_spk_doc->get_acq_size();
	m_file_scroll_.SetScrollInfo(&m_file_scroll_infos_);

	update_legends();

	// display & compute parameters
	if (!b_all_files_ || !b_measure_done_)
	{
		if (4 != spike_classification_->i_parameter)
		{
			if (chart_measures_.vt_tags.get_tag_list_size() > 0)
			{
				chart_measures_.vt_tags.remove_all_tags();
				chart_measures_.set_time_intervals(l_first_, l_last_);
			}
		}
		else
		{
			chart_measures_.set_time_intervals(-p_spk_list->get_spike_length(), p_spk_list->get_spike_length());
			if (1 > chart_measures_.vt_tags.get_tag_list_size())
			{
				m_i_xy_right_ = chart_measures_.vt_tags.add_tag(spike_classification_->i_xy_right, 0);
				m_i_xy_left_ = chart_measures_.vt_tags.add_tag(spike_classification_->i_xy_left, 0);
				const auto delta = p_spk_list->get_acq_sampling_rate() / time_unit_;
				t_xy_right_ = static_cast<float>(spike_classification_->i_xy_right) / delta;
				t_xy_left_ = static_cast<float>(spike_classification_->i_xy_left) / delta;
			}
		}
	}

}
void ViewSpikeSort::select_spike()
{
	int spike_index = -1;
	if (p_spk_list != nullptr)
	{
		// spike and classes
		spike_index = p_spk_list->m_selected_spike;
		if (p_spk_list->get_spikes_count() < spike_index || 0 > spike_index)
		{
			spike_index = -1;
			sort_source_class_ = 0;
		}
		else
		{
			sort_source_class_ = p_spk_list->get_spike(spike_index)->get_class_id();
			spike_classification_->source_class = sort_source_class_;
		}
		ASSERT(sort_source_class_ < 32768);
	}
	db_spike spike_sel(-1, -1, spike_index);
	select_spike(spike_sel);
}

void ViewSpikeSort::update_legends()
{
	// update text abscissa and horizontal scroll position
	time_first_s_ = static_cast<float>(l_first_) / sampling_rate_;
	time_last_s_ = static_cast<float>(l_last_) / sampling_rate_;
	update_file_scroll();

	if (4 != spike_classification_->i_parameter)
		chart_measures_.set_time_intervals(l_first_, l_last_);
	else
		chart_measures_.set_time_intervals(-p_spk_list->get_spike_length(), p_spk_list->get_spike_length());
	chart_shape_.set_time_intervals(l_first_, l_last_);
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
	const auto n_files = pdb_doc->db_get_records_count();
	auto current_list = 0;
	if (p_spk_doc != nullptr)
		current_list = p_spk_doc->get_index_current_spike_list();

	// change indexes if ALL files selected
	DlgProgress* dlg_progress = nullptr;
	auto i_step = 0;
	CString cs_comment;
	if (b_all_files_)
	{
		first_file = 0; // index first file
		last_file = pdb_doc->db_get_records_count() - 1; // index last file
		dlg_progress = new DlgProgress;
		dlg_progress->Create();
		dlg_progress->set_step(1);
	}

	classes_grid_.get_property_list()->delete_all();

	for (auto i_file = first_file; i_file <= last_file; i_file++)
	{
		// load spike file
		BOOL flag_changed;
		if (pdb_doc->db_set_current_record_position(i_file)) {
			p_spk_doc = pdb_doc->open_current_spike_file();
			if (nullptr == p_spk_doc)
				continue;
		}

		// update screen if multi-file requested
		if (b_all_files_)
		{
			if (dlg_progress->check_cancel_button())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			cs_comment.Format(_T("Processing file [%i / %i]"), i_file + 1, n_files);
			dlg_progress->set_status(cs_comment);
			if (MulDiv(i_file, 100, n_files) > i_step)
			{
				dlg_progress->step_it();
				i_step = MulDiv(i_file, 100, n_files);
			}
		}

		// load spike list
		p_spk_list = p_spk_doc->set_index_current_spike_list(current_list);
		if ((nullptr == p_spk_list) || (0 == p_spk_list->get_spike_length()))
			continue;

		// loop over all spikes of the list and compare to a single parameter
		const CSize limits1(spike_classification_->lower_threshold, spike_classification_->upper_threshold);
		const CSize from_class_id_to_class_id(sort_source_class_, sort_destination_class_);
		const CSize time_window(l_first_, l_last_);

		// sort on 1 parameter
		if (4 != spike_classification_->i_parameter)
		{
			flag_changed = p_spk_list->sort_spike_with_y1(from_class_id_to_class_id, time_window, limits1);
		}
		// sort on 2 parameters
		else
		{
			const CSize limits2(spike_classification_->i_xy_left, spike_classification_->i_xy_right);
			flag_changed = p_spk_list->sort_spike_with_y1_and_y2(from_class_id_to_class_id, time_window, limits1, limits2);
		}

		if (flag_changed || p_spk_doc->IsModified())
		{
			p_spk_doc->OnSaveDocument(pdb_doc->db_get_current_spk_file_name(FALSE));
			pdb_doc->set_db_n_spikes(p_spk_list->get_spikes_count());

		}

		classes_grid_.update_list(p_spk_list);
		p_spk_doc->SetModifiedFlag(TRUE);
	}

	// end of loop, select current file again if necessary
	if (b_all_files_)
	{
		delete dlg_progress;
		if (pdb_doc->db_set_current_record_position(current_file)) 
		{
			p_spk_doc = pdb_doc->open_current_spike_file();
			if (p_spk_doc != nullptr)
				p_spk_list = p_spk_doc->get_spike_list_current();
		}
	}

	// refresh data windows
	update_file_parameters();
	build_histogram();
	all_charts_invalidate();
	
}

void ViewSpikeSort::set_mouse_cursor(int value)
{
	if (CURSOR_ZOOM < value)
		value = 0;
	set_view_mouse_cursor(value);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(value, 0));
}

void ViewSpikeSort::change_file_time_first_and_last()
{
	if (4 != spike_classification_->i_parameter)
	{
		l_first_ = chart_measures_.get_time_first();
		l_last_ = chart_measures_.get_time_last();
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
	if (p_spk_list->get_spike_flag_array_count() > 0)
		clear_flag_all_spikes();
	db_spike spike_hit = GetDocument()->get_spike_hit();
	select_spike(spike_hit);
}

void ViewSpikeSort::change_spikes_measuring_window(const int tag_index)
{
	if (tag_index == shape_t1_)
	{
		spike_classification_->shape_t1 = chart_shape_.vt_tags.get_value_int(shape_t1_);
		shape_t1_ms_ = static_cast<float>(spike_classification_->shape_t1) * delta_ms_;
		mm_shape_t1_ms_.m_b_entry_done = TRUE;
		on_en_change_shape_t1();
	}
	else if (tag_index == shape_t2_)
	{
		spike_classification_->shape_t2 = chart_shape_.vt_tags.get_value_int(shape_t2_);
		shape_t2_ms_ = static_cast<float>(spike_classification_->shape_t2) * delta_ms_;
		mm_shape_t2_ms_.m_b_entry_done = TRUE;
		on_en_change_shape_t2();
	}
}

void ViewSpikeSort::change_chart_measure_vt_tag(const int value)
{
	if (value == m_i_xy_right_)
	{
		spike_classification_->i_xy_right = chart_measures_.vt_tags.get_value_int(m_i_xy_right_);
		t_xy_right_ = static_cast<float>(spike_classification_->i_xy_right) / delta_ms_;
		mm_t_xy_right_.m_b_entry_done = TRUE;
		on_en_change_edit_right2();
	}
	else if (value == m_i_xy_left_)
	{
		spike_classification_->i_xy_left = chart_measures_.vt_tags.get_value_int(m_i_xy_left_);
		t_xy_left_ = static_cast<float>(spike_classification_->i_xy_left) / delta_ms_;
		mm_t_xy_left_.m_b_entry_done = TRUE;
		on_en_change_edit_left2();
	}
}

void ViewSpikeSort::change_chart_histogram_vt_tag(const int tag_index)
{
	if (tag_index == tag_index_hist_low_) 
	{
		lower_threshold_mv_ = static_cast<float>(chart_histogram_.get_vt_tag_value_mv(tag_index_hist_low_));
		update_cursors_from_lower_threshold_mv();
	}
	else if (tag_index == tag_index_hist_up_)
	{
		upper_threshold_mv_ = static_cast<float>(chart_histogram_.get_vt_tag_value_mv(tag_index_hist_up_));
		update_cursors_from_upper_threshold_mv();
	}
	UpdateData(FALSE);
	chart_measures_.Invalidate();
}

void ViewSpikeSort::update_cursors_from_lower_threshold_mv()
{
	spike_classification_->lower_threshold = static_cast<int>(lower_threshold_mv_ / delta_mv_);
	chart_measures_.move_hz_tag(tag_index_measures_low_, spike_classification_->lower_threshold);

	chart_histogram_.move_vt_tag_to_value_mv(tag_index_hist_low_, lower_threshold_mv_);
}

void ViewSpikeSort::update_cursors_from_upper_threshold_mv()
{
	spike_classification_->upper_threshold = static_cast<int>(upper_threshold_mv_ / delta_mv_);
	chart_measures_.move_hz_tag(tag_index_measures_up_, spike_classification_->upper_threshold);

	chart_histogram_.move_vt_tag_to_value_mv(tag_index_hist_up_, upper_threshold_mv_);
}

void ViewSpikeSort::change_chart_measure_hz_tag(const int value)
{
	if (value == tag_index_measures_low_)
	{
		lower_threshold_mv_ = static_cast<float>(chart_measures_.hz_tags.get_value_int(tag_index_measures_low_)) * delta_mv_;
		update_cursors_from_lower_threshold_mv();
	}
	else if (value == tag_index_measures_up_)
	{
		upper_threshold_mv_ = static_cast<float>(chart_measures_.hz_tags.get_value_int(tag_index_measures_up_)) * delta_mv_;
		update_cursors_from_upper_threshold_mv();
	}
	UpdateData(FALSE);
}

void ViewSpikeSort::save_windows_properties_to_options()
{
	options_view_data_->spk_sort1_spk = *chart_shape_.get_scope_parameters();
	options_view_data_->spk_sort1_parameters = *chart_measures_.get_scope_parameters();
	options_view_data_->spk_sort1_hist = *chart_histogram_.get_scope_parameters();
	options_view_data_->spk_sort1_bars = *chart_spike_bar_.get_scope_parameters();
}

LRESULT ViewSpikeSort::on_my_message(const WPARAM code, const LPARAM l_param)
{
	const int value = LOWORD(l_param);
	switch (code)
	{
	case HINT_SET_MOUSE_CURSOR:
		set_mouse_cursor(value);
		break;

	case HINT_CHANGE_HZ_LIMITS:
		change_file_time_first_and_last();
		break;

	case HINT_HIT_SPIKE:
		hit_spike();
		break;

	case HINT_SELECT_SPIKES:
		all_charts_invalidate();
		break;

	case HINT_DBL_CLK_SEL:
		on_tools_edit_spikes();
		break;

	case HINT_CHANGE_VERT_TAG:
		if (HIWORD(l_param) == IDC_CHART_SHAPE)
			change_spikes_measuring_window(value);
		else if (HIWORD(l_param) == IDC_CHART_HISTOGRAM)
			change_chart_histogram_vt_tag(value);
		else if (HIWORD(l_param) == IDC_CHART_MEASURE)
			change_chart_measure_vt_tag(value);
		break;

	case HINT_CHANGE_HZ_TAG:
		if (HIWORD(l_param) == IDC_CHART_MEASURE)
			change_chart_measure_hz_tag(value);
		break;

	case HINT_VIEW_SIZE_CHANGED: 
		update_legends();
		break;

	case HINT_WINDOW_PROPS_CHANGED:
		save_windows_properties_to_options();
		break;

	case HINT_VIEW_TAB_HAS_CHANGED:
		select_spike_list(value);
		break;

	case HINT_SAVE_SPIKEFILE:
		save_current_spk_file();
		break;

	default:
		break;
	}
	return 0L;
}

void ViewSpikeSort::clear_flag_all_spikes()
{
	if (b_all_files_)
	{
		const auto pdb_doc = GetDocument();
		for (auto i_file = 0; i_file < pdb_doc->db_get_records_count(); i_file++)
		{
			if (pdb_doc->db_set_current_record_position(i_file)) {
				p_spk_doc = pdb_doc->open_current_spike_file();

				for (auto j = 0; j < p_spk_doc->get_spike_list_size(); j++)
				{
					p_spk_list = p_spk_doc->set_index_current_spike_list(j);
					p_spk_list->clear_flagged_spikes();
				}
			}
		}
	}
	else
		p_spk_list->clear_flagged_spikes();
}

void ViewSpikeSort::on_measure_parameters_from_spikes()
{
	const auto pdb_doc = GetDocument();
	const int n_files = pdb_doc->db_get_records_count();
	if (p_spk_doc == nullptr)
		return;

	const auto current_spike_list = p_spk_doc->get_index_current_spike_list();
	const int index_current_file = pdb_doc->db_get_current_record_position();
	db_spike spike_sel(-1, -1, -1);
	select_spike(spike_sel);
	classes_grid_.get_property_list()->delete_all();

	const int index_first_file = b_all_files_?  0: index_current_file;
	const int index_last_file = b_all_files_ ? (n_files-1): index_current_file;

	// loop over all selected files (or only one file currently selected)
	for (auto i_file = index_first_file; i_file <= index_last_file; i_file++)
	{
		if (!pdb_doc->db_set_current_record_position(i_file))
			continue;

		p_spk_doc = pdb_doc->open_current_spike_file();
		if (p_spk_doc == nullptr)
			continue;

		p_spk_list = p_spk_doc->set_index_current_spike_list(current_spike_list);
		if (p_spk_list == nullptr)
			continue;

		const auto n_spikes = p_spk_list->get_spikes_count();
		if (n_spikes <= 0 || p_spk_list->get_spike_length() == 0)
			continue;

		switch (spike_classification_->i_parameter)
		{
		case 1: // value at t1
			p_spk_list->measure_amplitude_vt(spike_classification_->shape_t1);
			b_measure_done_ = TRUE;
			break;
		case 2: // value at t2
			p_spk_list->measure_amplitude_vt(spike_classification_->shape_t2);
			b_measure_done_ = TRUE;
			break;

		case 3: // value at t2- value at t1
			p_spk_list->measure_amplitude_vt2_minus_vt1(spike_classification_->shape_t1, spike_classification_->shape_t2);
			b_measure_done_ = TRUE;
			break;

		case 0: // max - min between t1 and t2
		case 4: // max-min vs t_max-t_min
		default:
			p_spk_list->measure_amplitude_min_to_max(spike_classification_->shape_t1, spike_classification_->shape_t2);
			break;
		}

		classes_grid_.update_list(p_spk_list);
		p_spk_doc->SetModifiedFlag(TRUE);
		save_current_spk_file();

		if (b_all_files_)
		{
			l_first_ = 0;
			if (l_last_ < p_spk_doc->get_acq_size() - 1)
				l_last_ = p_spk_doc->get_acq_size() - 1;
			time_first_s_ = static_cast<float>(l_first_) / p_spk_list->get_acq_sampling_rate();
			time_last_s_ = static_cast<float>(l_last_) / p_spk_list->get_acq_sampling_rate();
		}
	}

	chart_shape_.set_source_data(p_spk_list, GetDocument());
	chart_spike_bar_.set_source_data(p_spk_list, GetDocument());
	on_format_gain_adjust();

	UpdateData(FALSE);
}

void ViewSpikeSort::update_gain()
{
	const auto y_we = static_cast<int>((measure_max_mv_ - measure_min_mv_) / delta_mv_);
	const auto y_wo = static_cast<int>((measure_max_mv_ + measure_min_mv_) / 2 / delta_mv_);
	chart_measures_.set_yw_ext_org(y_we, y_wo);

	build_histogram();
	chart_histogram_.set_xw_ext_org(y_we, y_wo);

	// get max min and center accordingly
	int max, min;
	p_spk_list->get_total_max_min(FALSE, &max, &min);
	const auto middle = (max + min) / 2;
	chart_shape_.set_yw_ext_org(y_we, middle);
	chart_spike_bar_.set_yw_ext_org(y_we, middle);

	all_charts_invalidate();
}

void ViewSpikeSort::on_view_all_data_on_abscissa()
{
	// dots: spk file length
	if (l_first_ != 0 || l_last_ != p_spk_doc->get_acq_size() - 1)
	{
		l_first_ = 0;
		l_last_ = p_spk_doc->get_acq_size() - 1;

		if (spike_classification_->i_parameter != 4) // then, we need imax i_min ...
			chart_measures_.set_time_intervals(l_first_, l_last_);
		else
			chart_measures_.set_time_intervals(-p_spk_list->get_spike_length(), p_spk_list->get_spike_length());
		chart_shape_.set_time_intervals(l_first_, l_last_);
		chart_spike_bar_.set_time_intervals(l_first_, l_last_);

		chart_measures_.Invalidate();
		chart_shape_.Invalidate();
		chart_spike_bar_.Invalidate();
		build_histogram();
		chart_histogram_.Invalidate();
	}

	// spikes: center spikes horizontally and adjust hz size of display
	const auto x_we = p_spk_list->get_spike_length();
	if (x_we != chart_shape_.get_xw_extent() || 0 != chart_shape_.get_xw_org())
		chart_shape_.set_xw_ext_org(x_we, 0);

	update_legends();
}

void ViewSpikeSort::build_histogram()
{
	const auto pdb_doc = GetDocument();
	if (pdb_doc == nullptr)
		return;

	chart_histogram_.build_hist_from_document(pdb_doc, b_all_files_, l_first_, l_last_,
		measure_min_mv_, measure_max_mv_, histogram_bin_mv_);
	chart_histogram_.get_extents();

	upper_threshold_mv_ = static_cast<float>(spike_classification_->upper_threshold) * delta_mv_ ;
	lower_threshold_mv_ = static_cast<float>(spike_classification_->lower_threshold) * delta_mv_;
	chart_histogram_.move_vt_tag_to_value_mv(tag_index_hist_low_, lower_threshold_mv_);
	chart_histogram_.move_vt_tag_to_value_mv(tag_index_hist_up_, upper_threshold_mv_);
}

void ViewSpikeSort::on_format_center_curve()
{
	center_curve();
}

void ViewSpikeSort::center_curve()
{
	GetDocument()->center_spike_amplitude_all_spikes_between_t1_and_t2(static_cast<boolean>(b_all_files_), -1, spike_classification_->shape_t1, spike_classification_->shape_t2);
	int value_max, value_min;
	if (!GetDocument()->get_max_min_amplitude_of_all_spikes(b_all_files_, TRUE, value_max, value_min))
		return;

	const auto y_wo = (value_max + value_min) / 2;
	chart_shape_.set_yw_ext_org(chart_shape_.get_yw_extent(), y_wo);
	chart_spike_bar_.set_yw_ext_org(chart_shape_.get_yw_extent(), y_wo);

	update_legends();
}

void ViewSpikeSort::on_format_split_curves()
{
	gain_adjust();
}

void ViewSpikeSort::on_format_gain_adjust()
{
	gain_adjust();
}

void ViewSpikeSort::gain_adjust()
{
	gain_adjust_shape_and_bars();
	gain_adjust_xy_and_histogram();
	update_legends();
}

void ViewSpikeSort::gain_adjust_shape_and_bars()
{
	int value_max, value_min;
	if (!GetDocument()->get_max_min_amplitude_of_all_spikes(b_all_files_, TRUE, value_max, value_min))
		return;

	const auto y_we = MulDiv(value_max - value_min + 1, 10, 9);
	const auto y_wo = (value_max + value_min) / 2;
	chart_shape_.set_yw_ext_org(y_we, y_wo);
	chart_spike_bar_.set_yw_ext_org(y_we, y_wo);
}

void ViewSpikeSort::gain_adjust_xy_and_histogram()
{
	int value_max, value_min;
	if (!GetDocument()->get_max_min_y1_of_all_spikes(static_cast<boolean>(b_all_files_), value_max, value_min))
		return;

	const auto upper2 = static_cast<int>(upper_threshold_mv_ / delta_mv_);
	const auto lower2 = static_cast<int>(lower_threshold_mv_ / delta_mv_);
	if (upper2 > value_max)
		value_max = upper2;
	if (lower2 < value_min)
		value_min = lower2;
	const auto y_we = MulDiv(value_max - value_min + 1, 10, 8);
	const auto y_wo = (value_max + value_min) / 2;
	chart_measures_.set_yw_ext_org(y_we, y_wo);
	
	// adjust histogram
	chart_histogram_.set_xw_ext_org(y_we, y_wo - y_we / 2); // TODO check if valid
	measure_max_mv_ = delta_mv_ * static_cast<float>(value_max);
	measure_min_mv_ = delta_mv_ * static_cast<float>(value_min);
	build_histogram();
	const auto y_max = static_cast<int>(chart_histogram_.get_hist_max_value());
	chart_histogram_.set_yw_ext_org(MulDiv(y_max, 10, 8), 0);
}

void ViewSpikeSort::select_spike(db_spike& spike_sel)
{
	if (spike_sel.record_id < 0 || spike_sel.spike_list_index < 0) {
		const CdbWaveDoc* p_doc = GetDocument();
		spike_sel.record_id = p_doc->db_get_current_record_id();
		if (p_doc->m_p_spk_doc != nullptr)
			spike_sel.spike_list_index = p_doc->m_p_spk_doc->get_index_current_spike_list();
	}
	record_id_ = spike_sel.record_id;
	list_index_ = spike_sel.spike_list_index;
	spike_index_ = spike_sel.spike_index;

	chart_shape_.select_spike(spike_sel);
	chart_spike_bar_.select_spike(spike_sel);
	chart_measures_.select_spike_measure(spike_sel);
	p_spk_list->m_selected_spike = spike_sel.spike_index;

	spike_class_index_ = -1;
	auto n_cmd_show = SW_HIDE;
	if (spike_sel.spike_index >= 0)
	{
		const auto spike = p_spk_list->get_spike(spike_sel.spike_index);
		if (spike != nullptr) {
			spike_class_index_ = spike->get_class_id();
			n_cmd_show = SW_SHOW;
		}
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_SPIKE_CLASS)->ShowWindow(n_cmd_show);
	spike_index_ = spike_sel.spike_index;
	UpdateData(FALSE);
}

boolean ViewSpikeSort::open_dat_and_spk_files_of_selected_spike(const db_spike& spike_coords)
{
	const auto p_doc = GetDocument();
	if (spike_coords.record_id >= 0 
		&& spike_coords.record_id != p_doc->db_get_current_record_id())
	{
		if (p_doc->db_move_to_id(spike_coords.record_id))
		{
			const auto spk_name = p_doc->db_get_current_spk_file_name();
			load_current_spike_file();
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
	const db_spike spike_coords (-1, -1, spike_index_);

	if (!open_dat_and_spk_files_of_selected_spike(spike_coords))
		return;

	DlgSpikeEdit dlg;
	dlg.y_extent = chart_shape_.get_yw_extent();
	dlg.y_zero = chart_shape_.get_yw_org();
	dlg.x_extent = chart_shape_.get_xw_extent();
	dlg.x_zero = chart_shape_.get_xw_org();
	dlg.dlg_spike_index = spike_index_;
	dlg.db_wave_doc = GetDocument();
	dlg.m_parent = this;
	dlg.DoModal();

	if (!dlg.b_artefact)
	{
		db_spike spike_sel(spike_coords.record_id, spike_coords.spike_list_index, spike_index_);
		select_spike(spike_sel);
	}
	if (dlg.b_changed)
		p_spk_doc->SetModifiedFlag(TRUE);

	update_legends();
	UpdateData(FALSE);
}

void ViewSpikeSort::on_select_all_files()
{
	b_all_files_ = static_cast<CButton*>(GetDlgItem(IDC_ALL_FILES))->GetCheck();
	b_measure_done_ = FALSE;
	on_measure_parameters_from_spikes();
	on_select_all_files_display_interface();

	chart_spike_bar_.set_display_all_files(static_cast<boolean>(b_all_files_));
	chart_shape_.set_display_all_files(static_cast<boolean>(b_all_files_));
	chart_measures_.set_display_all_files(static_cast<boolean>(b_all_files_));
}

void ViewSpikeSort::on_select_all_files_display_interface() const
{
	const int n_show = b_all_files_ ? SW_SHOW: SW_HIDE ;
	GetDlgItem(IDC_FILE_STATIC)->ShowWindow(n_show);
	GetDlgItem(IDC_FILE_INDEX)->ShowWindow(n_show);
	GetDlgItem(IDC_LIST_STATIC)->ShowWindow(n_show);
	GetDlgItem(IDC_LIST_INDEX)->ShowWindow(n_show);
}

void ViewSpikeSort::on_tools_align_spikes()
{
	// get source data
	auto b_doc_exist = FALSE;
	auto data_file_name = p_spk_doc->get_acq_filename();
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

	const auto spike_length = p_spk_list->get_spike_length();
	const auto total_spikes = p_spk_list->get_spikes_count();
	const auto p_sum0 = new double[spike_length]; // array with results / SUMy
	const auto p_cxy0 = new double[spike_length]; // temp array to store correlation
	auto* const p_mean0 = new int[spike_length]; // mean (template) / at scale
	const auto p_dummy0 = new int[spike_length]; // results of correlation / at scale

	// init pSUM with zeros
	auto p_sum = p_sum0;
	for (auto i = 0; i < spike_length; i++, p_sum++)
		*p_sum = 0;

	// compute mean
	auto nb_spk_selected_class = 0;
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		if (p_spk_list->get_spike(i_spike)->get_class_id() != sort_source_class_)
			continue;
		nb_spk_selected_class++;
		int* p_spk = p_spk_list->get_spike(i_spike)->get_p_data();
		p_sum = p_sum0;
		for (auto i = 0; i < spike_length; i++, p_spk++, p_sum++)
			*p_sum += *p_spk;
	}

	// build avg and avg autocorrelation, then display
	auto p_mean = p_mean0;
	p_sum = p_sum0;

	for (auto i = 0; i < spike_length; i++, p_mean++, p_sum++)
		*p_mean = static_cast<int>(*p_sum / nb_spk_selected_class);

	chart_shape_.display_ex_data(p_mean0);

	// for each spike, compute correlation and take max value correlation
	const auto k_start = spike_classification_->shape_t1; // start of template match
	const auto k_end = spike_classification_->shape_t2; // end of template match
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
	const auto p_dat_doc = GetDocument()->m_p_data_doc;
	p_dat_doc->open_document(data_file_name);
	const auto doc_chan = p_spk_list->get_detection_parameters()->extract_channel;
	const auto number_channels = static_cast<int>(p_dat_doc->get_wave_format()->scan_count);
	const auto method = p_spk_list->get_detection_parameters()->extract_transform;
	const auto spike_pre_trigger = p_spk_list->get_detection_parameters()->detect_pre_threshold;
	const int offset = (method > 0) ? 1 : number_channels;
	const int span = AcqDataDoc::get_transformed_data_span(method);

	// pre-load data
	auto ii_time0 = p_spk_list->get_spike(0)->get_time();
	auto l_rw_first0 = ii_time0 - spike_length;
	auto l_rw_last0 = ii_time0 + spike_length;
	if (!p_dat_doc->load_raw_data(&l_rw_first0, &l_rw_last0, span))
		return;

	auto p_data = p_dat_doc->load_transformed_data(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto pre_trigger = p_spk_list->get_detection_parameters()->detect_pre_threshold;
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		Spike* spike = p_spk_list->get_spike(i_spike);

		// exclude spikes that do not fall within time limits
		if (spike->get_class_id() != sort_source_class_)
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

			// add cross product for each point: data * mean long ii_time
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
			p_spk_doc->SetModifiedFlag(TRUE);
			spike->set_time(ii_time0 + pre_trigger);
		}

		// now offset spike vertically to align it with the mean
		spike->offset_spike_data_to_average_ex(k_start, k_end);
	}

	// exit : delete resources used locally
	if (p_spk_doc->IsModified())
	{
		chart_shape_.Invalidate();
		chart_shape_.display_ex_data(p_mean0);
	}

	delete[] p_sum0;
	delete[] p_mean0;
	delete[] p_cxy0;
	delete[] p_dummy0;

	on_measure_parameters_from_spikes();
}

void ViewSpikeSort::OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar)
{
	// view scroll: if pointer null
	if (p_scroll_bar == nullptr)
	{
		ViewDbTable::OnHScroll(n_sb_code, n_pos, p_scroll_bar);
		return;
	}
	// trap messages from ScrollBarEx
	CString cs;
	switch (n_sb_code)
	{
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		m_file_scroll_.GetScrollInfo(&m_file_scroll_infos_, SIF_ALL);
		l_first_ = m_file_scroll_infos_.nPos;
		l_last_ = l_first_ + static_cast<long>(m_file_scroll_infos_.nPage) - 1;
		break;

	default:
		scroll_file(n_sb_code, n_pos);
		break;
	}
	update_legends();
}

void ViewSpikeSort::scroll_file(const UINT n_sb_code, const UINT n_pos)
{
	// get corresponding data
	const auto total_scroll = p_spk_doc->get_acq_size();
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
	case SB_RIGHT: l_first = total_scroll - page_scroll + 1; break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		l_first = static_cast<int>(n_pos); break;
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

void ViewSpikeSort::select_spike_list(const int current_index)
{
	p_spk_list = p_spk_doc->set_index_current_spike_list(current_index);
	ASSERT(p_spk_list != NULL);
	on_measure_parameters_from_spikes();

	all_charts_set_spike_list(p_spk_list);
	all_charts_invalidate();
}

void ViewSpikeSort::on_en_change_edit_left2()
{
	if (mm_t_xy_left_.m_b_entry_done)
	{
		mm_t_xy_left_.on_en_change(this, t_xy_left_, delta_ms_, -delta_ms_);
		// check boundaries
		if (t_xy_left_ >= t_xy_right_)
			t_xy_left_ = t_xy_right_ - delta_ms_;

		// change display if necessary
		const auto left = t_xy_left_ / delta_ms_;
		const auto it_left = static_cast<int>(left);
		if (it_left != chart_measures_.vt_tags.get_value_int(m_i_xy_left_))
		{
			spike_classification_->i_xy_left = it_left;
			chart_measures_.move_vt_tag(m_i_xy_left_, it_left);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_edit_right2()
{
	if (mm_t_xy_right_.m_b_entry_done)
	{
		mm_t_xy_right_.on_en_change(this, t_xy_right_, delta_ms_, -delta_ms_);

		// check boundaries
		if (t_xy_right_ <= t_xy_left_)
			t_xy_right_ = t_xy_left_ + delta_ms_;

		// change display if necessary
		const auto right = t_xy_right_ / delta_ms_;
		const auto i_right = static_cast<int>(right);
		if (i_right != chart_measures_.vt_tags.get_value_int(m_i_xy_right_))
		{
			spike_classification_->i_xy_right = i_right;
			chart_measures_.move_vt_tag(m_i_xy_right_, i_right);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::all_charts_invalidate()
{
	chart_shape_.Invalidate();
	chart_spike_bar_.Invalidate();
	chart_measures_.Invalidate();
	chart_histogram_.Invalidate();
}

void  ViewSpikeSort::all_charts_set_spike_list(SpikeList* spk_list)
{
	chart_histogram_.set_spike_list(spk_list);
	chart_shape_.set_spike_list(spk_list);
	chart_spike_bar_.set_spike_list(spk_list);
	chart_measures_.set_spike_list(spk_list);
}

void ViewSpikeSort::on_en_change_source_class()
{
	if (mm_source_class_.m_b_entry_done)
	{
		const auto source_class = sort_source_class_;
		mm_source_class_.on_en_change(this, sort_source_class_, 1, -1);
		if (sort_source_class_ != source_class)
		{
			chart_measures_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class_);
			chart_histogram_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class_);
			chart_spike_bar_.set_plot_mode(PLOT_CLASS_COLORS, sort_source_class_);
		}
		all_charts_invalidate();

		auto db_sel = db_spike(-1, -1, -1);
		select_spike(db_sel);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::update_destination_class_descriptor() const
{
	CString cs = _T("undefined category");
	if (sort_destination_class_ >= 0 && sort_destination_class_ < SpikeClassProperties::nb_descriptors)
		cs = SpikeClassProperties::class_descriptor[sort_destination_class_];
	GetDlgItem(IDC_CLASS_DESCRIPTOR)->SetWindowText(cs);
}

void ViewSpikeSort::on_en_change_destination_class()
{
	if (mm_destination_class_.m_b_entry_done)
	{
		mm_destination_class_.on_en_change(this, sort_destination_class_, 1, -1);
		update_destination_class_descriptor();

		auto db_sel = db_spike(-1, -1, -1);
		select_spike(db_sel);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_select_change_measure_type()
{
	const auto i_parameter = combo_measure_type_.GetCurSel();
	if (i_parameter != spike_classification_->i_parameter)
	{
		spike_classification_->i_parameter = i_parameter;
		activate_mode4();
		on_measure_parameters_from_spikes();
		on_format_center_curve();
	}

	//    STATIC3 lower STATIC4 upper STATIC5 T1 STATIC6 T2
	// 0  mV      vis    mV      vis    vis     vis  vis   vis
	// 1  mV      vis    mV      vis    vis     vis  NOT   NOT
	// 2  mS      vis    mS      vis    vis     vis  vis   vis
}

void ViewSpikeSort::check_valid_threshold_limits()
{
	if (lower_threshold_mv_ >= upper_threshold_mv_ || lower_threshold_mv_ < 0)
		lower_threshold_mv_ = 0;

	if (upper_threshold_mv_ <= lower_threshold_mv_)
		upper_threshold_mv_ = lower_threshold_mv_ + 0.5f; // m_delta_ * 10.f;
}

void ViewSpikeSort::on_en_change_lower_threshold()
{
	if (mm_lower_threshold_mv_.m_b_entry_done)
	{
		const auto old_threshold = lower_threshold_mv_;
		mm_lower_threshold_mv_.on_en_change(this, lower_threshold_mv_, delta_mv_, -delta_mv_);
		check_valid_threshold_limits();

		if (old_threshold > lower_threshold_mv_ || old_threshold < lower_threshold_mv_)
		{
			UpdateData(FALSE);
			update_cursors_from_lower_threshold_mv();
			chart_measures_.Invalidate();
		}
	}
}

void ViewSpikeSort::on_en_change_upper_threshold()
{
	if (mm_upper_threshold_mv_.m_b_entry_done)
	{
		const auto old_threshold = upper_threshold_mv_;
		mm_upper_threshold_mv_.on_en_change(this, upper_threshold_mv_, delta_mv_, -delta_mv_);
		check_valid_threshold_limits();
		if (old_threshold > upper_threshold_mv_ || old_threshold < upper_threshold_mv_)
		{
			UpdateData(FALSE);
			update_cursors_from_upper_threshold_mv();
			chart_measures_.Invalidate();
		}
	}
}

void ViewSpikeSort::on_en_change_shape_t1()
{
	if (mm_shape_t1_ms_.m_b_entry_done)
	{
		mm_shape_t1_ms_.on_en_change(this, shape_t1_ms_, delta_ms_, -delta_ms_);
		// check boundaries
		if (shape_t1_ms_ < 0)
			shape_t1_ms_ = 0.0f;
		if (shape_t1_ms_ >= shape_t2_ms_)
			shape_t1_ms_ = shape_t2_ms_ - delta_ms_;

		const auto t1 = static_cast<int>(shape_t1_ms_ / delta_ms_);
		if (t1 != chart_shape_.vt_tags.get_value_int(shape_t1_))
		{
			spike_classification_->shape_t1 = t1;
			chart_shape_.move_vt_track(shape_t1_, spike_classification_->shape_t1);
			p_spk_list->shape_t1 = spike_classification_->shape_t1;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_shape_t2()
{
	if (mm_shape_t2_ms_.m_b_entry_done)
	{
		mm_shape_t2_ms_.on_en_change(this, shape_t2_ms_, delta_ms_, -delta_ms_);

		// check boundaries
		if (shape_t2_ms_ < shape_t1_ms_)
			shape_t2_ms_ = shape_t1_ms_ + delta_ms_;
		const auto t_max = (static_cast<float>(p_spk_list->get_spike_length()) - 1.f) * delta_ms_;
		if (shape_t2_ms_ >= t_max)
			shape_t2_ms_ = t_max;

		const auto it2 = static_cast<int>(shape_t2_ms_ / delta_ms_);
		if (it2 != chart_shape_.vt_tags.get_value_int(shape_t2_))
		{
			spike_classification_->shape_t2 = it2;
			chart_shape_.move_vt_track(shape_t2_, spike_classification_->shape_t2);
			p_spk_list->shape_t2 = spike_classification_->shape_t2;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::on_en_change_time_first()
{
	if (mm_time_first_s_.m_b_entry_done)
	{
		mm_time_first_s_.on_en_change(this, time_first_s_, 1.f, -1.f);

		// check boundaries
		if (time_first_s_ < 0.f)
			time_first_s_ = 0.f;
		if (time_first_s_ >= time_last_s_)
			time_first_s_ = 0.f;

		l_first_ = static_cast<long>(time_first_s_ * p_spk_list->get_acq_sampling_rate());
		update_legends();
	}
}

void ViewSpikeSort::on_en_change_time_last()
{
	if (mm_time_last_s_.m_b_entry_done)
	{
		mm_time_last_s_.on_en_change(this, time_last_s_, 1.f, -1.f);

		// check boundaries
		if (time_last_s_ <= time_first_s_)
			l_last_ = static_cast<long>(static_cast<float>(p_spk_doc->get_acq_size() - 1) / p_spk_list->get_acq_sampling_rate());

		l_last_ = static_cast<long>(time_last_s_ * p_spk_list->get_acq_sampling_rate());
		update_legends();
	}
}

void ViewSpikeSort::on_en_change_min_mv()
{
	if (mm_measure_min_mv_.m_b_entry_done)
	{
		mm_measure_min_mv_.on_en_change(this, measure_min_mv_, 1.f, -1.f);

		// check boundaries
		if (measure_min_mv_ >= measure_max_mv_)
			measure_min_mv_ = measure_max_mv_ - 1.f;

		// change display if necessary
		update_gain();
		update_legends();
	}
}

void ViewSpikeSort::on_en_change_max_mv()
{
	if (mm_measure_max_mv_.m_b_entry_done)
	{
		mm_measure_max_mv_.on_en_change(this, measure_max_mv_, 1.f, -1.f);
		if (measure_max_mv_ <= measure_min_mv_)
			measure_max_mv_ = measure_min_mv_ + 1.f;

		update_gain();
		update_legends();
	}
}

void ViewSpikeSort::on_en_change_file_index()
{
	if (mm_file_index_.m_b_entry_done)
	{
		const auto file_index = record_id_;
		mm_file_index_.on_en_change(this, record_id_, 1, -1);

		//if (file_index_ != file_index)
		//{
			//p_spk_doc->SetModifiedFlag(TRUE);
			//const auto current_list = spk_list_tab_ctrl.GetCurSel();
			//auto* spike_list = p_spk_doc->set_spike_list_current_index(current_list);
			//spike_list->get_spike(spike_index_)->set_class_id(class_index_);
		//	
		//}
	}
	update_legends();
}


void ViewSpikeSort::on_en_change_list_index()
{
	if (mm_list_index_.m_b_entry_done)
	{
		const auto list_index = list_index_;
		mm_file_index_.on_en_change(this, list_index_, 1, -1);

		//if (list_index_ != list_index)
		//{
			//p_spk_doc->SetModifiedFlag(TRUE);
			//const auto current_list = spk_list_tab_ctrl.GetCurSel();
			//auto* spike_list = p_spk_doc->set_spike_list_current_index(current_list);
			//spike_list->get_spike(spike_index_)->set_class_id(class_index_);
			//update_legends();
		//}
	}
	update_legends();
}

void ViewSpikeSort::on_en_change_spike_index()
{
	if (mm_spike_index_.m_b_entry_done)
	{
		if (b_all_files_)
			change_spike_index_in_all_files_mode();
		else
			change_spike_index_in_single_file_mode();
	}
}

// TODO check this routine for all_files
void ViewSpikeSort::change_spike_index_in_single_file_mode()
{
	const int spike_index = spike_index_;
	mm_spike_index_.on_en_change(this, spike_index_, 1, -1);

	// check boundaries
	if (spike_index_ < 0)
		spike_index_ = -1;
	if (spike_index_ >= p_spk_list->get_spikes_count())
		spike_index_ = p_spk_list->get_spikes_count() - 1;

	if (spike_index_ != spike_index)
	{
		if (spike_index_ >= 0)
		{
			// test if spike visible in the current time interval
			const auto spike_element = p_spk_list->get_spike(spike_index_);
			const auto spk_first = spike_element->get_time() - p_spk_list->get_detection_parameters()->detect_pre_threshold;
			const auto spk_last = spk_first + p_spk_list->get_spike_length();

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
	auto db_sel = db_spike(-1, -1, spike_index_);
	select_spike(db_sel);

}

// TODO make it work properly
void ViewSpikeSort::change_spike_index_in_all_files_mode()
{
		const int spike_index = spike_index_;
		mm_spike_index_.on_en_change(this, spike_index_, 1, -1);

		// check boundaries
		if (spike_index_ < 0)
			spike_index_ = -1;
		if (spike_index_ >= p_spk_list->get_spikes_count())
			spike_index_ = p_spk_list->get_spikes_count() - 1;

		if (spike_index_ != spike_index)
		{
			if (spike_index_ >= 0)
			{
				// test if spike visible in the current time interval
				const auto spike_element = p_spk_list->get_spike(spike_index_);
				const auto spk_first = spike_element->get_time() - p_spk_list->get_detection_parameters()->detect_pre_threshold;
				const auto spk_last = spk_first + p_spk_list->get_spike_length();

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
		auto db_sel = db_spike(-1, -1, spike_index_);
		select_spike(db_sel);
}

void ViewSpikeSort::on_en_change_spike_class()
{
	if (mm_class_index_.m_b_entry_done)
	{
		const auto spike_index_class = spike_class_index_;
		mm_class_index_.on_en_change(this, spike_class_index_, 1, -1);

		if (spike_class_index_ != spike_index_class)
		{
			p_spk_doc->SetModifiedFlag(TRUE);
			const auto current_list = spk_list_tab_ctrl.GetCurSel();
			auto* spike_list = p_spk_doc->set_index_current_spike_list(current_list);
			spike_list->get_spike(spike_index_)->set_class_id(spike_class_index_);

			classes_grid_.update_list(spike_list);
			update_legends();
		}
	}
}

void ViewSpikeSort::on_en_change_hist_bin_ms()
{
	if (mm_histogram_bin_mv_.m_b_entry_done)
	{
		const auto old_histogram_bin_mv = histogram_bin_mv_;
		const auto delta = (measure_max_mv_ - measure_min_mv_) / 10.f;
		mm_histogram_bin_mv_.on_en_change(this, histogram_bin_mv_, delta, -delta);

		if (histogram_bin_mv_ > old_histogram_bin_mv || histogram_bin_mv_ < old_histogram_bin_mv)
		{
			build_histogram();
			update_legends();
		}
	}
}

HBRUSH ViewSpikeSort::set_brush_and_text_color_according_to_class(CDC* p_dc, const int spike_class)
{
	const int color_index = spike_class % ChartWnd::nb_colors;
	const COLORREF color = ChartWnd::color_spike_class[color_index];
	p_dc->SetBkColor(color);
	
	const COLORREF color_text = ChartWnd::color_spike_class_text[color_index];
	p_dc->SetTextColor(color_text);

	return CreateSolidBrush(color);
}

HBRUSH ViewSpikeSort::OnCtlColor(CDC* p_dc, CWnd* p_wnd, const UINT n_ctl_color)
{
	HBRUSH hbr = ViewDbTable::OnCtlColor(p_dc, p_wnd, n_ctl_color);

	const int control_id = p_wnd->GetDlgCtrlID();
	switch (control_id)
	{
	case IDC_SOURCE_CLASS:
		hbr = set_brush_and_text_color_according_to_class(p_dc, sort_source_class_);
		break;
	case IDC_DESTINATION_CLASS:
		hbr = set_brush_and_text_color_according_to_class(p_dc, sort_destination_class_);
		break;
	case IDC_SPIKE_CLASS:
		hbr = set_brush_and_text_color_according_to_class(p_dc, spike_class_index_);
		break;
	default:
		break;
	}

	return hbr;
}

LRESULT ViewSpikeSort::OnPropertyChanged(WPARAM w_param, LPARAM l_param)
{
	if (!l_param)
		return 0;
	auto prop = reinterpret_cast<CMFCPropertyGridProperty*>(l_param);
	/*if (prop != m_pProp)
	{*/
	const int class_id = static_cast<int>(prop->GetData());
	const auto str_variant = prop->GetValue();
	CString str;
	if (str_variant.vt == VT_BSTR)
		str = CString(str_variant.bstrVal);

	SpikeClassProperties* p_desc = p_spk_list->get_class_descriptor_from_id(class_id);
	p_desc->set_class_text(str);

	p_spk_doc->SetModifiedFlag(TRUE);
	save_current_spk_file();
	//}
	return 0;
}
