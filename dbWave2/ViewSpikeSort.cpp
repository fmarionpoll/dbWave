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

// TODO limit size of measure array to nbspikes within currently selected spikelist

IMPLEMENT_DYNCREATE(ViewSpikeSort, dbTableView)

ViewSpikeSort::ViewSpikeSort()
	: dbTableView(IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

ViewSpikeSort::~ViewSpikeSort()
{
	// save spkD list i	 changed
	if (m_pSpkDoc != nullptr)
		saveCurrentSpkFile(); // save file if modified
	// save current spike detection parameters
	spike_classification_parameters_->b_changed = TRUE;
	spike_classification_parameters_->source_class = m_source_class;
	spike_classification_parameters_->dest_class = m_destination_class;
	spike_classification_parameters_->mv_max = m_mv_max;
	spike_classification_parameters_->mv_min = m_mv_min;
}

void ViewSpikeSort::DoDataExchange(CDataExchange* pDX)
{
	dbTableView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PARAMETER, m_combo_parameter);
	DDX_Text(pDX, IDC_T1, m_t1);
	DDX_Text(pDX, IDC_T2, m_t2);
	DDX_Text(pDX, IDC_LIMITLOWER, limit_lower_threshold);
	DDX_Text(pDX, IDC_LIMITUPPER, limit_upper_threshold);
	DDX_Text(pDX, IDC_SOURCECLASS, m_source_class);
	DDX_Text(pDX, IDC_DESTINATIONCLASS, m_destination_class);
	DDX_Text(pDX, IDC_EDIT2, m_time_first);
	DDX_Text(pDX, IDC_EDIT3, m_time_last);
	DDX_Text(pDX, IDC_EDIT6, m_mv_max);
	DDX_Text(pDX, IDC_EDIT7, m_mv_min);
	DDX_Text(pDX, IDC_BINMV, m_mv_bin);
	DDX_Check(pDX, IDC_CHECK1, m_b_all_files);
	DDX_Text(pDX, IDC_NSPIKES, m_spike_index);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spike_index_class);
	DDX_Text(pDX, IDC_EDITRIGHT2, m_t_xy_right);
	DDX_Text(pDX, IDC_EDITLEFT2, m_t_xy_left);

	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}

BEGIN_MESSAGE_MAP(ViewSpikeSort, dbTableView)

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikeSort::OnMyMessage)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_SOURCECLASS, &ViewSpikeSort::OnEnChangeSourceSpikeClass)
	ON_EN_CHANGE(IDC_DESTINATIONCLASS, &ViewSpikeSort::OnEnChangeDestinationSpikeClass)
	ON_CBN_SELCHANGE(IDC_PARAMETER, &ViewSpikeSort::OnSelchangeParameter)
	ON_EN_CHANGE(IDC_LIMITLOWER, &ViewSpikeSort::OnEnChangeLower)
	ON_EN_CHANGE(IDC_LIMITUPPER, &ViewSpikeSort::OnEnChangeUpper)
	ON_EN_CHANGE(IDC_T1, &ViewSpikeSort::OnEnChangeT1)
	ON_EN_CHANGE(IDC_T2, &ViewSpikeSort::OnEnChangeT2)
	ON_BN_CLICKED(IDC_EXECUTE, &ViewSpikeSort::OnSort)
	ON_BN_CLICKED(IDC_MEASURE, &ViewSpikeSort::OnMeasure)
	ON_BN_CLICKED(IDC_CHECK1, &ViewSpikeSort::OnSelectAllFiles)
	ON_COMMAND(ID_FORMAT_ALLDATA, &ViewSpikeSort::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &ViewSpikeSort::OnFormatCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &ViewSpikeSort::OnFormatGainadjust)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, &ViewSpikeSort::OnFormatSplitcurves)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, &ViewSpikeSort::OnToolsEdittransformspikes)
	ON_COMMAND(ID_TOOLS_ALIGNSPIKES, &ViewSpikeSort::OnToolsAlignspikes)

	ON_EN_CHANGE(IDC_EDIT2, &ViewSpikeSort::OnEnChangeTimeFirst)
	ON_EN_CHANGE(IDC_EDIT3, &ViewSpikeSort::OnEnChangeTimeLast)
	ON_EN_CHANGE(IDC_EDIT7, &ViewSpikeSort::OnEnChangemVMin)
	ON_EN_CHANGE(IDC_EDIT6, &ViewSpikeSort::OnEnChangemVMax)
	ON_EN_CHANGE(IDC_EDITLEFT2, &ViewSpikeSort::OnEnChangeEditLeft2)
	ON_EN_CHANGE(IDC_EDITRIGHT2, &ViewSpikeSort::OnEnChangeEditRight2)
	ON_EN_CHANGE(IDC_NSPIKES, &ViewSpikeSort::OnEnChangeNOspike)
	ON_BN_DOUBLECLICKED(IDC_DISPLAYPARM, &ViewSpikeSort::OnToolsEdittransformspikes)
	ON_EN_CHANGE(IDC_SPIKECLASS, &ViewSpikeSort::OnEnChangeSpikeClass)
	ON_EN_CHANGE(IDC_BINMV, &ViewSpikeSort::OnEnChangeNBins)
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
	VERIFY(mm_mv_max_.SubclassDlgItem(IDC_EDIT6, this));
	VERIFY(mm_mv_min_.SubclassDlgItem(IDC_EDIT7, this));
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
	m_stretch.AttachParent(this);

	m_stretch.newProp(IDC_EDIT7, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_DISPLAYPARM, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_DISPLAYBARS, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_EDIT3, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_STATICRIGHT, SZEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_STATICLEFT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_STATIC12, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDITLEFT2, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDITRIGHT2, SZEQ_XLEQ, SZEQ_YBEQ);
}

void ViewSpikeSort::OnInitialUpdate()
{
	dbTableView::OnInitialUpdate();
	define_sub_classed_items();
	define_stretch_parameters();
	m_b_init = TRUE;
	m_autoIncrement = true;
	m_autoDetect = true;

	// load global parameters
	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	spike_classification_parameters_ = &(p_app->spk_classification);
	options_view_data_ = &(p_app->options_view_data);

	// assign values to controls
	m_combo_parameter.SetCurSel(spike_classification_parameters_->i_parameter);
	m_mv_max = spike_classification_parameters_->mv_max;
	m_mv_min = spike_classification_parameters_->mv_min;

	m_source_class = spike_classification_parameters_->source_class;
	m_destination_class = spike_classification_parameters_->dest_class;

	chart_spike_shape_.display_all_files(false, GetDocument());
	chart_spike_shape_.set_plot_mode(PLOT_ONECOLOR, m_source_class);
	chart_spike_shape_.set_scope_parameters(&(options_view_data_->spksort1spk));
	m_spk_form_tag_left_ = chart_spike_shape_.vertical_tags.add_tag(spike_classification_parameters_->i_left, 0);
	m_spk_form_tag_right_ = chart_spike_shape_.vertical_tags.add_tag(spike_classification_parameters_->i_right, 0);

	chart_xt_measures_.display_all_files(false, GetDocument());
	chart_xt_measures_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
	chart_xt_measures_.set_scope_parameters(&(options_view_data_->spksort1parms));
	m_i_tag_up_ = chart_xt_measures_.horizontal_tags.add_tag(spike_classification_parameters_->upper_threshold, 0);
	m_i_tag_low_ = chart_xt_measures_.horizontal_tags.add_tag(spike_classification_parameters_->lower_threshold, 0);

	chart_spike_bar_.display_all_files(false, GetDocument());
	chart_spike_bar_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
	chart_spike_bar_.set_scope_parameters(&(options_view_data_->spksort1bars));

	chart_histogram_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
	chart_histogram_.set_scope_parameters(&(options_view_data_->spksort1hist));

	// display tag lines at proper places
	m_spk_hist_upper_threshold_ = chart_histogram_.vertical_tags.add_tag(spike_classification_parameters_->upper_threshold, 0);
	m_spk_hist_lower_threshold_ = chart_histogram_.vertical_tags.add_tag(spike_classification_parameters_->lower_threshold, 0);

	update_file_parameters();
	if (nullptr != m_pSpkList)
	{
		m_delta_ = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		limit_lower_threshold = static_cast<float>(spike_classification_parameters_->lower_threshold) * m_delta_;
		limit_upper_threshold = static_cast<float>(spike_classification_parameters_->upper_threshold) * m_delta_;
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
			m_t_xy_right = static_cast<float>(spike_classification_parameters_->i_xy_right) / delta;
			m_t_xy_left = static_cast<float>(spike_classification_parameters_->i_xy_left) / delta;
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
		saveCurrentSpkFile();
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
	if (m_b_init)
	{
		switch (LOWORD(lHint))
		{
		case HINT_DOCHASCHANGED: // file has changed?
		case HINT_DOCMOVERECORD:
		case HINT_REQUERY:
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

BOOL ViewSpikeSort::OnMove(UINT nIDMoveCommand)
{
	saveCurrentSpkFile();
	return dbTableView::OnMove(nIDMoveCommand);
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
	if (!m_b_all_files)
	{
		chart_histogram_.delete_histogram_data();
	}

	const BOOL first_update = (m_pSpkDoc == nullptr);
	update_spike_file();

	if (first_update || options_view_data_->bEntireRecord)
	{
		if (m_pSpkDoc == nullptr)
			return;
		m_time_first = 0.f;
		m_time_last = (static_cast<float>(m_pSpkDoc->get_acq_size()) - 1) / m_pSpkList->get_acq_sampling_rate();
	}
	m_l_first_ = static_cast<long>(m_time_first * m_pSpkList->get_acq_sampling_rate());
	m_l_last_ = static_cast<long>(m_time_last * m_pSpkList->get_acq_sampling_rate());

	// spike and classes
	auto spike_index = m_pSpkList->m_selected_spike;
	if (m_pSpkList->get_spikes_count() < spike_index || 0 > spike_index)
	{
		spike_index = -1;
		m_source_class = 0;
	}
	else
	{
		m_source_class = m_pSpkList->get_spike(spike_index)->get_class_id();
		spike_classification_parameters_->source_class = m_source_class;
	}
	ASSERT(m_source_class < 32768);

	if (0 == spike_classification_parameters_->i_left && 0 == spike_classification_parameters_->i_right)
	{
		spike_classification_parameters_->i_left = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
		spike_classification_parameters_->i_right = spike_classification_parameters_->i_left + m_pSpkList->get_detection_parameters()->detect_refractory_period;
	}
	m_t1 = static_cast<float>(spike_classification_parameters_->i_left) * m_time_unit / m_pSpkList->get_acq_sampling_rate();
	m_t2 = static_cast<float>(spike_classification_parameters_->i_right) * m_time_unit / m_pSpkList->get_acq_sampling_rate();

	chart_spike_bar_.set_source_data(m_pSpkList, GetDocument());
	chart_spike_bar_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);

	chart_spike_shape_.set_source_data(m_pSpkList, GetDocument());
	chart_spike_shape_.vertical_tags.set_tag_val(m_spk_form_tag_left_, spike_classification_parameters_->i_left);
	chart_spike_shape_.vertical_tags.set_tag_val(m_spk_form_tag_right_, spike_classification_parameters_->i_right);
	chart_spike_shape_.set_plot_mode(PLOT_ONECOLOR, m_source_class);

	chart_xt_measures_.set_source_data(m_pSpkList, GetDocument());
	chart_xt_measures_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
	chart_histogram_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);

	m_file_scroll_infos_.fMask = SIF_ALL;
	m_file_scroll_infos_.nMin = 0;
	m_file_scroll_infos_.nMax = m_pSpkDoc->get_acq_size() - 1;
	m_file_scroll_infos_.nPos = 0;
	m_file_scroll_infos_.nPage = m_pSpkDoc->get_acq_size();
	m_file_scroll_.SetScrollInfo(&m_file_scroll_infos_);

	update_legends();

	// display & compute parameters
	if (!m_b_all_files || !m_b_measure_done_)
	{
		if (4 != spike_classification_parameters_->i_parameter)
		{
			chart_xt_measures_.set_time_intervals(m_l_first_, m_l_last_);
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
				m_t_xy_right = static_cast<float>(spike_classification_parameters_->i_xy_right) / delta;
				m_t_xy_left = static_cast<float>(spike_classification_parameters_->i_xy_left) / delta;
				chart_xt_measures_.Invalidate();
			}
		}
		// update text , display and compute histogram
		m_b_measure_done_ = FALSE; 
		OnMeasure();
	}

	select_spike_from_current_list(spike_index);
}

void ViewSpikeSort::update_legends()
{
	// update text abscissa and horizontal scroll position
	m_time_first = static_cast<float>(m_l_first_) / m_pSpkList->get_acq_sampling_rate();
	m_time_last = static_cast<float>(m_l_last_) / m_pSpkList->get_acq_sampling_rate();
	update_file_scroll();

	if (4 != spike_classification_parameters_->i_parameter)
		chart_xt_measures_.set_time_intervals(m_l_first_, m_l_last_);
	else
		chart_xt_measures_.set_time_intervals(-m_pSpkList->get_spike_length(), m_pSpkList->get_spike_length());
	chart_xt_measures_.Invalidate();

	chart_spike_shape_.set_time_intervals(m_l_first_, m_l_last_);
	chart_spike_shape_.Invalidate();

	chart_spike_bar_.set_time_intervals(m_l_first_, m_l_last_);
	chart_spike_bar_.Invalidate();

	chart_histogram_.Invalidate();

	UpdateData(FALSE);
}

void ViewSpikeSort::OnSort()
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
	if (m_b_all_files)
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
		pdb_doc->db_set_current_record_position(i_file);
		m_pSpkDoc = pdb_doc->open_current_spike_file();
		if (nullptr == m_pSpkDoc)
			continue;

		// update screen if multi-file requested
		if (m_b_all_files)
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
		const CSize from_class_id_to_class_id(m_source_class, m_destination_class);
		const CSize time_window(m_l_first_, m_l_last_);
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
	if (m_b_all_files)
	{
		delete dlg_progress;
		pdb_doc->db_set_current_record_position(current_file);
		m_pSpkDoc = pdb_doc->open_current_spike_file();
		m_pSpkList = m_pSpkDoc->get_spike_list_current();
	}

	// refresh data windows
	build_histogram();

	chart_xt_measures_.Invalidate();
	chart_spike_shape_.Invalidate();
	chart_spike_bar_.Invalidate();
	chart_histogram_.Invalidate();

	m_pSpkDoc->SetModifiedFlag(TRUE);
}

LRESULT ViewSpikeSort::OnMyMessage(WPARAM code, LPARAM lParam)
{
	short shortValue = LOWORD(lParam);
	switch (code)
	{
	case HINT_SETMOUSECURSOR: // ------------- change mouse cursor (all 3 items)
		if (CURSOR_ZOOM < shortValue)
			shortValue = 0;
		SetViewMouseCursor(shortValue);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(shortValue, 0));
		break;

	case HINT_CHANGEHZLIMITS: // -------------  abscissa have changed
		if (4 != spike_classification_parameters_->i_parameter)
		{
			m_l_first_ = chart_xt_measures_.get_time_first();
			m_l_last_ = chart_xt_measures_.get_time_last();
		}
		else
		{
			m_l_first_ = chart_spike_bar_.get_time_first();
			m_l_last_ = chart_spike_bar_.get_time_last();
		}
		update_legends();
		break;

	case HINT_HITSPIKE: // -------------  spike is selected or deselected
		{
			if (m_pSpkList->get_spike_flag_array_count() > 0)
				clear_flag_all_spikes();
			auto spike_index = 0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spike_index = chart_spike_shape_.get_hit_spike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spike_index = chart_spike_bar_.get_hit_spike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				spike_index = chart_xt_measures_.get_hit_spike();

			select_spike_from_current_list(spike_index);
		}
		break;

	case HINT_SELECTSPIKES:
		chart_xt_measures_.Invalidate();
		chart_spike_shape_.Invalidate();
		chart_spike_bar_.Invalidate();
		break;

	case HINT_DBLCLKSEL:
		{
			auto spike_index = 0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spike_index = chart_spike_shape_.get_hit_spike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spike_index = chart_spike_bar_.get_hit_spike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				spike_index = chart_xt_measures_.get_hit_spike();
			// if m_bAllFiles, spike_index is global, otherwise it comes from a single file...
			select_spike_from_current_list(spike_index);
			OnToolsEdittransformspikes();
		}
		break;

	//case HINT_MOVEVERTTAG: // -------------  vertical tag has moved lowp = tag index
	case HINT_CHANGEVERTTAG: // -------------  vertical tag value has changed
		if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
		{
			if (shortValue == m_spk_form_tag_left_) // first tag
			{
				spike_classification_parameters_->i_left = chart_spike_shape_.vertical_tags.get_value(m_spk_form_tag_left_);
				m_t1 = static_cast<float>(spike_classification_parameters_->i_left) * m_time_unit / m_pSpkList->get_acq_sampling_rate();
				mm_t1_.m_bEntryDone = TRUE;
				OnEnChangeT1();
			}
			else if (shortValue == m_spk_form_tag_right_) // second tag
			{
				spike_classification_parameters_->i_right = chart_spike_shape_.vertical_tags.get_value(m_spk_form_tag_right_);
				m_t2 = static_cast<float>(spike_classification_parameters_->i_right) * m_time_unit / m_pSpkList->get_acq_sampling_rate();
				mm_t2_.m_bEntryDone = TRUE;
				OnEnChangeT2();
			}
		}
		else if (HIWORD(lParam) == IDC_HISTOGRAM)
		{
			if (shortValue == m_spk_hist_lower_threshold_) // first tag
			{
				spike_classification_parameters_->lower_threshold = chart_histogram_.vertical_tags.get_value(m_spk_hist_lower_threshold_);
				limit_lower_threshold = static_cast<float>(spike_classification_parameters_->lower_threshold) * m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
				UpdateData(false);
			}
			else if (shortValue == m_spk_hist_upper_threshold_) // second tag
			{
				spike_classification_parameters_->upper_threshold = chart_histogram_.vertical_tags.get_value(m_spk_hist_upper_threshold_); // load new value
				limit_upper_threshold = static_cast<float>(spike_classification_parameters_->upper_threshold) * m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
				UpdateData(false);
			}
		}
		else if (HIWORD(lParam) == IDC_DISPLAYPARM)
		{
			if (shortValue == m_i_xy_right_)
			{
				const auto delta = m_pSpkList->get_acq_sampling_rate() / m_time_unit;
				spike_classification_parameters_->i_xy_right = chart_xt_measures_.vertical_tags.get_value(m_i_xy_right_);
				m_t_xy_right = static_cast<float>(spike_classification_parameters_->i_xy_right) / delta;
				mm_t_xy_right_.m_bEntryDone = TRUE;
				OnEnChangeEditRight2();
			}
			else if (shortValue == m_i_xy_left_)
			{
				const auto delta = m_pSpkList->get_acq_sampling_rate() / m_time_unit;
				spike_classification_parameters_->i_xy_left = chart_xt_measures_.vertical_tags.get_value(m_i_xy_left_);
				m_t_xy_left = static_cast<float>(spike_classification_parameters_->i_xy_left) / delta;
				mm_t_xy_left_.m_bEntryDone = TRUE;
				OnEnChangeEditLeft2();
			}
		}
		break;

	case HINT_CHANGEHZTAG: // ------------- change horizontal tag value
		//case HINT_MOVEHZTAG:	// ------------- move horizontal tag
		if (HIWORD(lParam) == IDC_DISPLAYPARM)
		{
			if (shortValue == m_i_tag_low_) // first tag
			{
				spike_classification_parameters_->lower_threshold = chart_xt_measures_.horizontal_tags.get_value(m_i_tag_low_);
				limit_lower_threshold = static_cast<float>(spike_classification_parameters_->lower_threshold) * m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
				mm_limit_lower_.m_bEntryDone = TRUE;
				OnEnChangeLower();
			}
			else if (shortValue == m_i_tag_up_) // second tag
			{
				spike_classification_parameters_->upper_threshold = chart_xt_measures_.horizontal_tags.get_value(m_i_tag_up_); 
				limit_upper_threshold = static_cast<float>(spike_classification_parameters_->upper_threshold) * m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
				mm_limit_upper_.m_bEntryDone = TRUE;
				OnEnChangeUpper();
			}
		}
		break;

	case HINT_VIEWSIZECHANGED: // ------------- change zoom
		update_legends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_view_data_->spksort1spk = *chart_spike_shape_.get_scope_parameters();
		options_view_data_->spksort1parms = *chart_xt_measures_.get_scope_parameters();
		options_view_data_->spksort1hist = *chart_histogram_.get_scope_parameters();
		options_view_data_->spksort1bars = *chart_spike_bar_.get_scope_parameters();
		break;

	case HINT_VIEWTABHASCHANGED:
		select_spike_list(shortValue);
		break;

	default:
		break;
	}
	return 0L;
}

void ViewSpikeSort::clear_flag_all_spikes()
{
	if (m_b_all_files)
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

	chart_xt_measures_.Invalidate();
	chart_spike_shape_.Invalidate();
	chart_spike_bar_.Invalidate();
}

void ViewSpikeSort::OnMeasure()
{
	// set file indexes - assume only one file selected
	const auto pdb_doc = GetDocument();
	const int n_files = pdb_doc->db_get_n_records();
	const auto current_spike_list = m_pSpkDoc->get_spike_list_current_index();
	int index_current_file = pdb_doc->db_get_current_record_position(); // index current file

	// change size of arrays and prepare temporary dialog
	select_spike_from_current_list(-1);
	int first_file = index_current_file;
	int last_file = index_current_file;
	if (m_b_all_files)
	{
		first_file = 0; // index first file
		last_file = n_files - 1; // index last file
	}

	// loop over all selected files (or only one file currently selected)
	for (auto i_file = first_file; i_file <= last_file; i_file++)
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
			m_pSpkList->measure_case1_amplitude_at_t(spike_classification_parameters_->i_left);
			m_b_measure_done_ = TRUE;
			break;
		case 2: // value at t2
			m_pSpkList->measure_case1_amplitude_at_t(spike_classification_parameters_->i_right);
			m_b_measure_done_ = TRUE;
			break;

		case 3: // value at t2- value at t1
			m_pSpkList->measure_case2_amplitude_at_t2_minus_at_t1(spike_classification_parameters_->i_left, spike_classification_parameters_->i_right);
			m_b_measure_done_ = TRUE;
			break;

		case 0: // max - min between t1 and t2
		case 4: // max-min vs tmax-tmin
		default:
			m_pSpkList->measure_case0_amplitude_min_to_max(spike_classification_parameters_->i_left, spike_classification_parameters_->i_right);
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
	const auto delta = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
	const auto y_we = static_cast<int>((m_mv_max - m_mv_min) / delta);
	const auto y_wo = static_cast<int>((m_mv_max + m_mv_min) / 2 /delta);

	chart_xt_measures_.set_yw_ext_org(y_we, y_wo);
	chart_histogram_.set_xw_ext_org(y_we, y_wo);// -y_we / 2);

	// get max min and center accordingly
	short max, min;
	m_pSpkList->get_total_max_min(FALSE, &max, &min);
	const auto middle = (max + min) / 2;
	chart_spike_shape_.set_yw_ext_org(y_we, middle);
	chart_spike_bar_.set_yw_ext_org(y_we, middle);

	chart_xt_measures_.Invalidate();
	chart_histogram_.Invalidate();
	chart_spike_shape_.Invalidate();
	chart_spike_bar_.Invalidate();
}

void ViewSpikeSort::OnFormatAlldata()
{
	// build new histogram only if necessary
	auto calculate_histogram = FALSE;

	// dots: spk file length
	if (m_l_first_ != 0 || m_l_last_ != m_pSpkDoc->get_acq_size() - 1)
	{
		m_l_first_ = 0;
		m_l_last_ = m_pSpkDoc->get_acq_size() - 1;

		if (spike_classification_parameters_->i_parameter != 4) // then, we need imax imin ...
			chart_xt_measures_.set_time_intervals(m_l_first_, m_l_last_);
		else
			chart_xt_measures_.set_time_intervals(-m_pSpkList->get_spike_length(), m_pSpkList->get_spike_length());
		chart_xt_measures_.Invalidate();

		chart_spike_shape_.set_time_intervals(m_l_first_, m_l_last_);
		chart_spike_shape_.Invalidate();

		chart_spike_bar_.set_time_intervals(m_l_first_, m_l_last_);
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
	m_measure_y1_max_ = static_cast<int>(m_mv_max / delta);
	m_measure_y1_min_ = static_cast<int>(m_mv_min / delta);
	const int n_bins = static_cast<int>((m_mv_max - m_mv_min) / m_mv_bin);
	if (n_bins <= 0)
		return;
	chart_histogram_.build_hist_from_document(pdb_doc, m_b_all_files, m_l_first_, m_l_last_, m_measure_y1_max_, m_measure_y1_min_, n_bins, TRUE);
}

void ViewSpikeSort::OnFormatCentercurve()
{
	const auto n_spikes = m_pSpkList->get_spikes_count();
	for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		const auto spike = m_pSpkList->get_spike(i_spike);
		spike->set_spike_length(m_pSpkList->get_spike_length());
		spike->center_spike_amplitude(spike_classification_parameters_->i_left, spike_classification_parameters_->i_right, 1);
	}

	short max, min;
	m_pSpkList->get_total_max_min(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	chart_spike_shape_.set_yw_ext_org(chart_spike_shape_.get_yw_extent(), middle);
	chart_spike_bar_.set_yw_ext_org(chart_spike_shape_.get_yw_extent(), middle);

	update_legends();
}

void ViewSpikeSort::OnFormatSplitcurves()
{
	OnFormatGainadjust();
}

void ViewSpikeSort::OnFormatGainadjust()
{
	// adjust gain
	short maxvalue, minvalue;
	GetDocument()->get_max_min_of_all_spikes(m_b_all_files, TRUE, &maxvalue, &minvalue);

	auto y_we = MulDiv(maxvalue - minvalue + 1, 10, 9);
	auto y_wo = (maxvalue + minvalue) / 2;
	chart_spike_shape_.set_yw_ext_org(y_we, y_wo);
	chart_spike_bar_.set_yw_ext_org(y_we, y_wo);
	//chart_spike_bar_.MaxCenter();

	// adjust gain for spk_hist_wnd_ and XYp: data = computed values
	// search max min of parameter values
	const CSize measure = GetDocument()->get_max_min_of_single_spike(m_b_all_files);
	maxvalue = static_cast<short>(measure.cx);
	minvalue = static_cast<short>(measure.cy);

	const auto delta = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
	const auto max2 = static_cast<short>(limit_upper_threshold / delta);
	const auto min2 = static_cast<short>(limit_lower_threshold / delta);
	if (max2 > maxvalue)
		maxvalue = max2;
	if (min2 < minvalue)
		minvalue = min2;
	y_we = MulDiv(maxvalue - minvalue + 1, 10, 8);
	y_wo = (maxvalue + minvalue) / 2;

	// update display
	chart_xt_measures_.set_yw_ext_org(y_we, y_wo);
	const auto y_max = static_cast<int>(chart_histogram_.GetHistMax());
	chart_histogram_.set_xw_ext_org(y_we, y_wo - y_we / 2);
	chart_histogram_.set_yw_ext_org(MulDiv(y_max, 10, 8), 0);

	// update edit controls
	m_mv_max = static_cast<float>(maxvalue) * delta;
	m_mv_min = static_cast<float>(minvalue) * delta;
	build_histogram();
	update_legends();
}

void ViewSpikeSort::select_spike_from_current_list(const int spike_index)
{
	CdbWaveDoc* pDoc = chart_spike_shape_.get_db_wave_doc();
	dbSpike spike_sel(pDoc->db_get_current_record_position(),
		pDoc->m_p_spk->get_spike_list_current_index(),
		spike_index);
	chart_spike_shape_.select_spike(spike_sel);
	chart_spike_bar_.select_spike(spike_sel);
	chart_xt_measures_.select_spike(spike_sel);
	m_pSpkList->m_selected_spike = spike_index;

	m_spike_index_class = -1;
	auto n_cmd_show = SW_HIDE;
	if (spike_index >= 0)
	{
		const auto spike = m_pSpkList->get_spike(spike_index);
		if (spike != nullptr) {
			m_spike_index_class = spike->get_class_id();
			n_cmd_show = SW_SHOW;
		}
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_SPIKECLASS)->ShowWindow(n_cmd_show);
	m_spike_index = spike_index;
	UpdateData(FALSE);
}

void ViewSpikeSort::OnToolsEdittransformspikes()
{
	DlgSpikeEdit dlg;
	dlg.m_yextent = chart_spike_shape_.get_yw_extent();
	dlg.m_yzero = chart_spike_shape_.get_yw_org();
	dlg.m_xextent = chart_spike_shape_.get_xw_extent();
	dlg.m_xzero = chart_spike_shape_.get_xw_org();
	dlg.m_spike_index = m_spike_index;
	dlg.m_parent = this;
	dlg.m_pdbWaveDoc = GetDocument();

	// refresh pointer to data file because it not used elsewhere in the view
	const auto dat_name = GetDocument()->db_get_current_dat_file_name();
	auto b_doc_exists = FALSE;
	if (!dat_name.IsEmpty())
	{
		CFileStatus status;
		b_doc_exists = CFile::GetStatus(dat_name, status);
	}
	if (b_doc_exists)
	{
		const auto flag = (GetDocument()->open_current_data_file() != nullptr);
		ASSERT(flag);
	}

	// run dialog box
	dlg.DoModal();
	if (dlg.m_bchanged)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
		const auto current_list = m_tabCtrl.GetCurSel();
		m_pSpkDoc->set_spike_list_as_current(current_list);
	}

	if (!dlg.m_bartefact && m_spike_index != dlg.m_spike_index)
		select_spike_from_current_list(dlg.m_spike_index);

	update_legends();
}

void ViewSpikeSort::OnSelectAllFiles()
{
	m_b_all_files = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	chart_spike_bar_.display_all_files(m_b_all_files, GetDocument());
	chart_spike_shape_.display_all_files(m_b_all_files, GetDocument());
	chart_xt_measures_.display_all_files(m_b_all_files, GetDocument());

	m_b_measure_done_ = FALSE;
	OnMeasure();
}

void ViewSpikeSort::OnToolsAlignspikes()
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
		if (m_pSpkList->get_spike(i_spike)->get_class_id() != m_source_class)
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
	const auto k_start = spike_classification_parameters_->i_left; // start of template match
	const auto k_end = spike_classification_parameters_->i_right; // end of template match
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
	p_dat_doc->OnOpenDocument(data_file_name);
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
		if (spike->get_class_id() != m_source_class)
			continue;

		ii_time0 = spike->get_time();
		ii_time0 -= spike_pre_trigger;

		// make sure that source data are loaded and get pointer to it (p_data)
		auto l_rw_first = ii_time0 - spike_length; 
		auto l_rw_last = ii_time0 + spike_length; 
		if (ii_time0 > m_l_last_ || ii_time0 < m_l_first_)
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

	OnMeasure();
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
		m_l_first_ = m_file_scroll_infos_.nPos;
		m_l_last_ = m_l_first_ + long(m_file_scroll_infos_.nPage) - 1;
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
	const auto page_scroll = (m_l_last_ - m_l_first_);
	auto sb_scroll = MulDiv(page_scroll, 10, 100);
	if (sb_scroll == 0)
		sb_scroll = 1;
	auto l_first = m_l_first_;
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

	m_l_first_ = l_first;
	m_l_last_ = l_last;
}

void ViewSpikeSort::update_file_scroll()
{
	m_file_scroll_infos_.fMask = SIF_PAGE | SIF_POS;
	m_file_scroll_infos_.nPos = m_l_first_;
	m_file_scroll_infos_.nPage = m_l_last_ - m_l_first_ + 1;
	m_file_scroll_.SetScrollInfo(&m_file_scroll_infos_);
}

void ViewSpikeSort::select_spike_list(int current_index)
{
	m_pSpkList = m_pSpkDoc->set_spike_list_as_current(current_index);
	//GetDocument()->GetCurrent_Spk_Document()->SetSpkList_CurrentIndex(current_index);
	ASSERT(m_pSpkList != NULL);
	OnMeasure();

	// update source data: change data channel and update display
	chart_histogram_.set_spike_list(m_pSpkList);
	chart_spike_shape_.set_spike_list(m_pSpkList);
	chart_spike_bar_.set_spike_list(m_pSpkList);
	chart_xt_measures_.set_spike_list(m_pSpkList);

	chart_histogram_.Invalidate();
	chart_xt_measures_.Invalidate();
	chart_spike_shape_.Invalidate();
	chart_spike_bar_.Invalidate();
}

void ViewSpikeSort::OnEnChangeEditLeft2()
{
	if (mm_t_xy_left_.m_bEntryDone)
	{
		auto left = m_t_xy_left;
		const auto delta = m_time_unit / m_pSpkList->get_acq_sampling_rate();
		mm_t_xy_left_.OnEnChange(this, m_t_xy_left, delta, -delta);
		// check boundaries
		if (m_t_xy_left >= m_t_xy_right)
			m_t_xy_left = m_t_xy_right - delta;

		// change display if necessary
		left = m_t_xy_left / delta;
		const auto it_left = static_cast<int>(left);
		if (it_left != chart_xt_measures_.vertical_tags.get_value(m_i_xy_left_))
		{
			spike_classification_parameters_->i_xy_left = it_left;
			chart_xt_measures_.move_vt_tag(m_i_xy_left_, it_left);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeEditRight2()
{
	if (mm_t_xy_right_.m_bEntryDone)
	{
		auto right = m_t_xy_right;
		const auto delta = m_time_unit / m_pSpkList->get_acq_sampling_rate();
		mm_t_xy_right_.OnEnChange(this, m_t_xy_right, delta, -delta);

		// check boundaries
		if (m_t_xy_right <= m_t_xy_left)
			m_t_xy_right = m_t_xy_left + delta;

		// change display if necessary
		right = m_t_xy_right / delta;
		const auto i_right = static_cast<int>(right);
		if (i_right != chart_xt_measures_.vertical_tags.get_value(m_i_xy_right_))
		{
			spike_classification_parameters_->i_xy_right = i_right;
			chart_xt_measures_.move_vt_tag(m_i_xy_right_, i_right);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeSourceSpikeClass()
{
	if (mm_source_class_.m_bEntryDone)
	{
		const auto source_class = m_source_class;
		mm_source_class_.OnEnChange(this, m_source_class, 1, -1);
		if (source_class != m_source_class)
		{
			chart_spike_shape_.set_plot_mode(PLOT_ONECOLOR, m_source_class);
			chart_xt_measures_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
			chart_histogram_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
			chart_spike_bar_.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
		}
		// change histogram accordingly
		chart_spike_shape_.Invalidate();
		chart_spike_bar_.Invalidate();
		chart_xt_measures_.Invalidate();
		chart_histogram_.Invalidate();

		select_spike_from_current_list(-1);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeDestinationSpikeClass()
{
	if (mm_destination_class_.m_bEntryDone)
	{
		mm_destination_class_.OnEnChange(this, m_destination_class, 1, -1);
		select_spike_from_current_list(-1);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnSelchangeParameter()
{
	const auto i_parameter = m_combo_parameter.GetCurSel();
	if (i_parameter != spike_classification_parameters_->i_parameter)
	{
		spike_classification_parameters_->i_parameter = i_parameter;
		activate_mode4();
		OnMeasure();
		OnFormatCentercurve();
	}

	//    STATIC3 lower STATIC4 upper STATIC5 T1 STATIC6 T2
	// 0  mV      vis    mV      vis    vis     vis  vis   vis
	// 1  mV      vis    mV      vis    vis     vis  NOT   NOT
	// 2  mS      vis    mS      vis    vis     vis  vis   vis
}

void ViewSpikeSort::check_valid_threshold_limits()
{
	
	if (limit_lower_threshold >= limit_upper_threshold)
		limit_lower_threshold = limit_upper_threshold - m_delta_ * 10.f;

	if (limit_lower_threshold < 0) {
		limit_lower_threshold = 0;
	}

	if (limit_upper_threshold <= limit_lower_threshold)
		limit_upper_threshold = limit_lower_threshold + m_delta_ * 10.f;
}

void ViewSpikeSort::OnEnChangeLower()
{
	if (mm_limit_lower_.m_bEntryDone)
	{
		m_delta_ = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		mm_limit_lower_.OnEnChange(this, limit_lower_threshold, m_delta_, -m_delta_);
		check_valid_threshold_limits();

		spike_classification_parameters_->lower_threshold = static_cast<int>(limit_lower_threshold / m_delta_);
		if (spike_classification_parameters_->lower_threshold != chart_xt_measures_.horizontal_tags.get_value(m_i_tag_low_))
			chart_xt_measures_.move_hz_tag(m_i_tag_low_, spike_classification_parameters_->lower_threshold);
		if (spike_classification_parameters_->lower_threshold != chart_histogram_.vertical_tags.get_value(m_spk_hist_lower_threshold_))
			chart_histogram_.MoveVTtagtoVal(m_spk_hist_lower_threshold_, spike_classification_parameters_->lower_threshold);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeUpper()
{
	if (mm_limit_upper_.m_bEntryDone)
	{
		m_delta_ = m_pSpkList->get_acq_volts_per_bin() * mv_unit_;
		mm_limit_upper_.OnEnChange(this, limit_upper_threshold,m_delta_, -m_delta_);
		// check boundaries
		check_valid_threshold_limits();

		spike_classification_parameters_->upper_threshold = static_cast<int>(limit_upper_threshold / m_delta_);
		if (spike_classification_parameters_->upper_threshold != chart_xt_measures_.horizontal_tags.get_value(m_i_tag_up_))
			chart_xt_measures_.move_hz_tag(m_i_tag_up_, spike_classification_parameters_->upper_threshold);
		if (spike_classification_parameters_->lower_threshold != chart_histogram_.vertical_tags.get_value(m_spk_hist_upper_threshold_))
			chart_histogram_.MoveVTtagtoVal(m_spk_hist_upper_threshold_, spike_classification_parameters_->upper_threshold);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeT1()
{
	if (mm_t1_.m_bEntryDone)
	{
		const auto delta = m_time_unit / m_pSpkList->get_acq_sampling_rate();
		
		mm_t1_.OnEnChange(this, m_t1, delta, -delta);
		// check boundaries
		if (m_t1 < 0)
			m_t1 = 0.0f;
		if (m_t1 >= m_t2)
			m_t1 = m_t2 - delta;
	;
		const auto it1 = static_cast<int>(m_t1 / delta);
		if (it1 != chart_spike_shape_.vertical_tags.get_value(m_spk_form_tag_left_))
		{
			spike_classification_parameters_->i_left = it1;
			chart_spike_shape_.move_vt_track(m_spk_form_tag_left_, spike_classification_parameters_->i_left);
			m_pSpkList->m_imaxmin1SL = spike_classification_parameters_->i_left;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeT2()
{
	if (mm_t2_.m_bEntryDone)
	{
		const auto delta = m_time_unit / m_pSpkList->get_acq_sampling_rate();
		mm_t2_.OnEnChange(this, m_t2, delta, -delta);

		// check boundaries
		if (m_t2 < m_t1)
			m_t2 = m_t1 + delta;
		const auto t_max = (static_cast<float>(m_pSpkList->get_spike_length()) - 1.f) * delta;
		if (m_t2 >= t_max)
			m_t2 = t_max;

		const auto it2 = static_cast<int>(m_t2 / delta);
		if (it2 != chart_spike_shape_.vertical_tags.get_value(m_spk_form_tag_right_))
		{
			spike_classification_parameters_->i_right = it2;
			chart_spike_shape_.move_vt_track(m_spk_form_tag_right_, spike_classification_parameters_->i_right);
			m_pSpkList->m_imaxmin2SL = spike_classification_parameters_->i_right;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeTimeFirst()
{
	if (mm_time_first_.m_bEntryDone)
	{
		mm_time_first_.OnEnChange(this, m_time_first, 1.f, -1.f);

		// check boundaries
		if (m_time_first < 0.f)
			m_time_first = 0.f;
		if (m_time_first >= m_time_last)
			m_time_first = 0.f;

		m_l_first_ = static_cast<long>(m_time_first * m_pSpkList->get_acq_sampling_rate());
		update_legends();
	}
}

void ViewSpikeSort::OnEnChangeTimeLast()
{
	if (mm_time_last_.m_bEntryDone)
	{
		mm_time_last_.OnEnChange(this, m_time_last, 1.f, -1.f);

		// check boundaries
		if (m_time_last <= m_time_first)
			m_l_last_ = static_cast<long>(static_cast<float>(m_pSpkDoc->get_acq_size() - 1) / m_pSpkList->get_acq_sampling_rate());

		m_l_last_ = static_cast<long>(m_time_last * m_pSpkList->get_acq_sampling_rate());
		update_legends();
	}
}

void ViewSpikeSort::OnEnChangemVMin()
{
	if (mm_mv_min_.m_bEntryDone)
	{
		mm_mv_min_.OnEnChange(this, m_mv_min, 1.f, -1.f);

		// check boundaries
		if (m_mv_min >= m_mv_max)
			m_mv_min = m_mv_max - 1.f;

		// change display if necessary
		update_gain();
		update_legends();
	}
}

void ViewSpikeSort::OnEnChangemVMax()
{
	if (mm_mv_max_.m_bEntryDone)
	{
		mm_mv_max_.OnEnChange(this, m_mv_max, 1.f, -1.f);
		if (m_mv_max <= m_mv_min)
			m_mv_max = m_mv_min + 1.f;

		update_gain();
		update_legends();
	}
}

void ViewSpikeSort::OnEnChangeNOspike()
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

				if (spk_first < m_l_first_ || spk_last > m_l_last_)
				{
					const auto l_span = (m_l_last_ - m_l_first_) / 2;
					const auto l_center = (spk_last + spk_first) / 2;
					m_l_first_ = l_center - l_span;
					m_l_last_ = l_center + l_span;
					update_legends();
				}
			}
		}
		select_spike_from_current_list(m_spike_index);
	}
}

void ViewSpikeSort::OnEnChangeSpikeClass()
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

void ViewSpikeSort::OnEnChangeNBins()
{
	if (mm_mv_bin_.m_bEntryDone)
	{
		const auto mV_bin = m_mv_bin;
		const auto delta = (m_mv_max - m_mv_min) / 10.f;
		mm_mv_bin_.OnEnChange(this, m_mv_bin, delta, -delta);

		if (m_mv_bin != mV_bin)
		{
			build_histogram();
			update_legends();
		}
	}
}
