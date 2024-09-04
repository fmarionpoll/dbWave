// TODO
// convert threshold into volts & back to binary (cope with variable gains)
// cf: UpdateFileParameters

#include "StdAfx.h"
#include "dbWave.h"
#include "ViewSpikeDetect.h"

#include "NiceUnit.h"
#include "DlgCopyAs.h"
#include "DlgDataSeries.h"
#include "DlgProgress.h"
#include "DlgSpikeDetect.h"
#include "DlgSpikeEdit.h"
#include "DlgXYParameters.h"
#include "DlgEditStimArray.h"

#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto b_restore = 0;
constexpr auto b_save = 1;

IMPLEMENT_DYNCREATE(ViewSpikeDetection, ViewDbTable)

ViewSpikeDetection::ViewSpikeDetection()
	: ViewDbTable(IDD)
{
	m_bEnableActiveAccessibility = FALSE;
}

ViewSpikeDetection::~ViewSpikeDetection()
{
	if (p_spk_doc != nullptr) 
	{
		save_current_spk_file();
	}
	// save spike detection parameters
	const auto p_array = spike_detection_array_->get_chan_array(scan_count_);
	*p_array = spk_detect_array_;
}

void ViewSpikeDetection::DoDataExchange(CDataExchange* p_dx)
{
	ViewDbTable::DoDataExchange(p_dx);

	DDX_Control(p_dx, IDC_DETECT_WHAT, m_detect_what_combo);
	DDX_Control(p_dx, IDC_SOURCECHAN, m_detect_channel_combo);
	DDX_Control(p_dx, IDC_TRANSFORM, m_transform_combo);
	DDX_Control(p_dx, IDC_TRANSFORM2, m_transform2_combo);
	DDX_Text(p_dx, IDC_TIMEFIRST, m_time_first);
	DDX_Text(p_dx, IDC_TIMELAST, m_time_last);
	DDX_Text(p_dx, IDC_SPIKENO, m_spike_index);
	DDX_Check(p_dx, IDC_ARTEFACT, m_b_artefact);
	DDX_Text(p_dx, IDC_THRESHOLDVAL, m_threshold_val);
	DDX_Text(p_dx, IDC_SPIKEWINDOWAMPLITUDE, m_spk_wnd_amplitude_);
	DDX_Text(p_dx, IDC_SPIKEWINDOWLENGTH, m_spk_wnd_duration_);
	DDX_Text(p_dx, IDC_CHANSELECTED, m_selected_channel);
	DDX_Text(p_dx, IDC_CHANSELECTED2, m_selected_channel2);
	DDX_Control(p_dx, IDC_XSCALE, m_ruler_bar_abscissa);
	DDX_Control(p_dx, IDC_STATICDISPLAYDATA, m_bevel1);
	DDX_Control(p_dx, IDC_STATICDISPLAYDETECT, m_bevel2);
	DDX_Control(p_dx, IDC_STATICDISPLAYBARS, m_bevel3);
	DDX_Control(p_dx, IDC_TAB1, spk_list_tab_ctrl);
}

BEGIN_MESSAGE_MAP(ViewSpikeDetection, ViewDbTable)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikeDetection::on_my_message)

	ON_COMMAND(ID_FORMAT_FIRST_FRAME, &ViewSpikeDetection::on_first_frame)
	ON_COMMAND(ID_FORMAT_LAST_FRAME, &ViewSpikeDetection::on_last_frame)
	ON_COMMAND(ID_FORMAT_SET_ORDINATES, &ViewSpikeDetection::on_format_x_scale)
	ON_COMMAND(ID_FORMAT_VIEW_ALL_DATA_ON_ABSCISSA, &ViewSpikeDetection::on_format_all_data)
	ON_COMMAND(ID_FORMAT_CENTER_CURVE, &ViewSpikeDetection::on_format_y_scale_center_curve)
	ON_COMMAND(ID_FORMAT_GAIN_ADJUST, &ViewSpikeDetection::on_format_y_scale_gain_adjust)
	ON_COMMAND(ID_FORMAT_SPLIT_CURVES, &ViewSpikeDetection::on_format_split_curves)
	ON_COMMAND(ID_TOOLS_DETECT_PARMS, &ViewSpikeDetection::on_tools_detection_parameters)
	ON_COMMAND(ID_TOOLS_DETECT, &ViewSpikeDetection::on_measure_all)
	ON_COMMAND(ID_TOOLS_EDIT_STIMULUS, &ViewSpikeDetection::on_tools_edit_stimulus)
	ON_COMMAND(ID_TOOLS_EDIT_SPIKES, &ViewSpikeDetection::on_tools_edit_spikes)
	ON_COMMAND(ID_TOOLS_DATA_SERIES, &ViewSpikeDetection::on_tools_data_series)
	ON_COMMAND(ID_EDIT_COPY, &ViewSpikeDetection::on_edit_copy)
	ON_COMMAND(ID_FILE_SAVE, &ViewSpikeDetection::on_file_save)
	ON_COMMAND(ID_FORMAT_XSCALE, &ViewSpikeDetection::on_format_x_scale)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_CBN_SELCHANGE(IDC_SOURCECHAN, &ViewSpikeDetection::on_sel_change_detect_chan)
	ON_CBN_SELCHANGE(IDC_TRANSFORM, &ViewSpikeDetection::on_sel_change_transform)
	ON_CBN_SELCHANGE(IDC_DETECT_WHAT, &ViewSpikeDetection::on_sel_change_detect_mode)
	ON_CBN_SELCHANGE(IDC_TRANSFORM2, &ViewSpikeDetection::on_cbn_sel_change_transform_2)

	ON_EN_CHANGE(IDC_THRESHOLDVAL, &ViewSpikeDetection::on_en_change_threshold)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewSpikeDetection::on_en_change_time_first)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewSpikeDetection::on_en_change_time_last)
	ON_EN_CHANGE(IDC_SPIKENO, &ViewSpikeDetection::on_en_change_spike_no)
	ON_EN_CHANGE(IDC_SPIKEWINDOWAMPLITUDE, &ViewSpikeDetection::on_en_change_spk_wnd_amplitude)
	ON_EN_CHANGE(IDC_SPIKEWINDOWLENGTH, &ViewSpikeDetection::on_en_change_spk_wnd_length)
	ON_EN_CHANGE(IDC_CHANSELECTED, &ViewSpikeDetection::on_en_change_chan_selected)
	ON_EN_CHANGE(IDC_CHANSELECTED2, &ViewSpikeDetection::on_en_change_chan_selected_2)

	ON_BN_CLICKED(IDC_MEASUREALL, &ViewSpikeDetection::on_measure_all)
	ON_BN_CLICKED(IDC_CLEAR, &ViewSpikeDetection::on_clear)
	ON_BN_CLICKED(IDC_ARTEFACT, &ViewSpikeDetection::on_artefact)
	ON_BN_CLICKED(IDC_BIAS, &ViewSpikeDetection::on_bn_clicked_bias_button)
	ON_BN_CLICKED(IDC_GAIN, &ViewSpikeDetection::on_bn_clicked_gain_button)
	ON_BN_CLICKED(IDC_LOCATEBTTN, &ViewSpikeDetection::on_bn_clicked_locate_button)
	ON_BN_CLICKED(IDC_CLEARALL, &ViewSpikeDetection::on_bn_clicked_clear_all)
	ON_BN_CLICKED(IDC_MEASURE, &ViewSpikeDetection::on_measure)
	ON_BN_CLICKED(IDC_GAIN2, &ViewSpikeDetection::on_bn_clicked_gain2)
	ON_BN_CLICKED(IDC_BIAS2, &ViewSpikeDetection::on_bn_clicked_bias2)

	//ON_NOTIFY(NM_CLICK, IDC_TAB1,		&dbTableView::OnNMClickTab1)
END_MESSAGE_MAP()

void ViewSpikeDetection::on_file_save()
{
	CFile f;
	CFileDialog dlg(FALSE,
		_T("spk"),											
		GetDocument()->db_get_current_spk_file_name(),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Awave Spikes (*.spk) | *.spk |All Files (*.*) | *.* ||"));

	if (IDOK == dlg.DoModal())
	{
		p_spk_doc->OnSaveDocument(dlg.GetPathName());
		GetDocument()->set_db_n_spikes(p_spk_doc->get_spike_list_current()->get_spikes_count());
		GetDocument()->set_db_n_spike_classes(1);
		p_spk_doc->SetModifiedFlag(FALSE);
	}
}

BOOL ViewSpikeDetection::OnMove(const UINT n_id_move_command)
{
	save_current_spk_file();
	return ViewDbTable::OnMove(n_id_move_command);
}

void ViewSpikeDetection::OnUpdate(CView* p_sender, const LPARAM l_hint, CObject* p_hint)
{
	if (b_init_)
	{
		switch (LOWORD(l_hint))
		{
		case HINT_CLOSE_FILE_MODIFIED: 
			save_current_spk_file();
			break;
		case HINT_DOC_MOVE_RECORD:
		case HINT_DOC_HAS_CHANGED:
			update_file_parameters(TRUE);
			break;
		case HINT_REPLACE_VIEW:
		default:
			break;
		}
	}
}

void ViewSpikeDetection::OnActivateView(const BOOL activate, CView* activated_view, CView* de_activated_view)
{
	if (activate)
	{
		const auto p_main_frame = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_main_frame->PostMessage(WM_MYMESSAGE, HINT_ACTIVATE_VIEW, reinterpret_cast<LPARAM>(activated_view->GetDocument()));
	}
	else
	{
		serialize_windows_state(b_save);
		const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		p_app->options_view_data.view_data = *(chart_data_.get_scope_parameters());
	}
	ViewDbTable::OnActivateView(activate, activated_view, de_activated_view);
}

void ViewSpikeDetection::update_legends()
{
	const auto l_first = chart_data_.get_data_first_index();
	const auto l_last = chart_data_.get_data_last_index();
	chart_data_filtered_.get_data_from_doc(l_first, l_last);

	// draw charts
	chart_spike_bar_.set_time_intervals(l_first, l_last);
	chart_spike_shape_.set_time_intervals(l_first, l_last);
	update_spike_shape_window_scale(FALSE);

	// update text abscissa and horizontal scroll position
	m_time_first = static_cast<float>(l_first) / m_sampling_rate_;
	m_time_last = static_cast<float>(l_last + 1) / m_sampling_rate_;
	m_spike_index = p_spk_list->m_selected_spike;

	if (m_spike_index > p_spk_list->get_spikes_count())
	{
		p_spk_list->m_selected_spike = -1;
		m_spike_index = p_spk_list->m_selected_spike;
	}

	m_b_artefact = FALSE;
	if (m_spike_index > 0)
	{
		const auto p_s = p_spk_list->get_spike(m_spike_index);
		m_b_artefact = (p_s->get_class_id() < 0);
	}

	update_file_scroll();
	update_combo_box();
	update_vt_tags();
	update_legend_detection_wnd();
	update_number_of_spikes();

	chart_spike_bar_.Invalidate();
	chart_data_filtered_.Invalidate();
	chart_data_.Invalidate();
	chart_spike_shape_.Invalidate();
	UpdateData(FALSE);
}

void ViewSpikeDetection::update_spike_file(const BOOL b_update_interface)
{
	const auto pdb_doc = GetDocument();
	if (pdb_doc->open_current_spike_file() == nullptr)
	{
		// file not found: create new object, and create file
		auto* p_spike = new CSpikeDoc;
		ASSERT(p_spike != NULL);
		pdb_doc->m_p_spk_doc = p_spike;
		p_spk_doc = p_spike;
		p_spk_doc->OnNewDocument();
		p_spk_doc->clear_data();
		if (options_view_data_->b_detect_while_browse)
			on_measure_all();
	}
	else
	{
		p_spk_doc = pdb_doc->m_p_spk_doc;
		p_spk_doc->SetModifiedFlag(FALSE);
		p_spk_doc->SetPathName(pdb_doc->db_get_current_spk_file_name(), FALSE);
	}

	// select a spike list
	p_spk_list = p_spk_doc->get_spike_list_current();
	if (p_spk_list == nullptr && p_spk_doc->get_spike_list_size() > 0)
		p_spk_list = p_spk_doc->set_index_current_spike_list(0);

	// no spikes list available, create one
	if (p_spk_list == nullptr)
	{
		// create new list here
		ASSERT(p_spk_doc->get_spike_list_size() == 0);
		const auto i_size = spk_detect_array_.get_size();
		p_spk_doc->set_spike_list_size(i_size);
		for (auto i = 0; i < i_size; i++)
		{
			auto spike_list_current = p_spk_doc->set_index_current_spike_list(i);
			if (spike_list_current == nullptr)
			{
				p_spk_doc->add_spk_list();
				spike_list_current = p_spk_doc->get_spike_list_current();
			}
			spike_list_current->init_spike_list(pdb_doc->m_p_data_doc, spk_detect_array_.get_item(i));
		}
		p_spk_list = p_spk_doc->set_index_current_spike_list(0);
		ASSERT(p_spk_list != nullptr);
	}

	chart_spike_bar_.set_source_data(p_spk_list, pdb_doc);
	chart_spike_bar_.set_plot_mode(PLOT_BLACK, 0);
	chart_spike_shape_.set_source_data(p_spk_list, pdb_doc);
	chart_spike_shape_.set_plot_mode(PLOT_BLACK, 0);
	update_vt_tags();

	// update interface elements
	m_spike_index = -1;
	if (b_update_interface)
	{
		update_tabs();
		update_detection_controls();
		highlight_spikes_in_chart_data(FALSE);
		update_number_of_spikes();
		chart_spike_bar_.Invalidate();
		chart_spike_shape_.Invalidate();
	}
}

void ViewSpikeDetection::highlight_spikes_in_chart_data(const BOOL flag)
{
	if (!flag || p_spk_list == nullptr || p_spk_list->get_spikes_count() < 1) 
		return;
	
	const auto array_size = p_spk_list->get_spikes_count() * 2 + 3;
	dw_intervals_.SetSize(array_size);
	dw_intervals_.SetAt(0, 0);
	dw_intervals_.SetAt(1, col_red); 
	dw_intervals_.SetAt(2, 1);
	const auto total_spikes = p_spk_list->get_spikes_count();
	auto j_index = 3;
	auto spike_length = p_spk_list->get_spike_length();
	const auto spike_pre_trigger = p_spk_list->get_detection_parameters()->detect_pre_threshold;
	spike_length--;

	for (auto i = 0; i < total_spikes; i++)
	{
		const auto p_s = p_spk_list->get_spike(i);
		const auto l_first = p_s->get_time() - spike_pre_trigger;
		dw_intervals_.SetAt(j_index, l_first);
		j_index++;
		dw_intervals_.SetAt(j_index, l_first + spike_length);
		j_index++;
	}

	// tell source_view to highlight spk
	chart_data_filtered_.set_highlight_data(&dw_intervals_);
	chart_data_filtered_.Invalidate();
	chart_data_.set_highlight_data(&dw_intervals_);
	chart_data_.Invalidate();
	
}

void ViewSpikeDetection::update_file_parameters(const BOOL b_update_interface)
{
	update_data_file(b_update_interface);
	update_spike_file(b_update_interface);
	if (b_update_interface)
		update_legends();
}

BOOL ViewSpikeDetection::check_detection_settings()
{
	auto flag = TRUE;
	ASSERT(m_p_detect_parameters_ != NULL);
	ASSERT_VALID(m_p_detect_parameters_);
	if (nullptr == m_p_detect_parameters_)
	{
		m_i_detect_parameters_ = GetDocument()->get_current_spike_file()->get_index_current_spike_list();
		m_p_detect_parameters_ = spk_detect_array_.get_item(m_i_detect_parameters_);
	}

	// get infos from data file
	const auto data_file = GetDocument()->m_p_data_doc;
	data_file->read_data_infos();
	const auto wave_format = data_file->get_wave_format();

	// check detection and extraction channels
	if (m_p_detect_parameters_->detect_channel < 0
		|| m_p_detect_parameters_->detect_channel >= wave_format->scan_count)
	{
		m_p_detect_parameters_->detect_channel = 0;
		AfxMessageBox(_T("Spike detection parameters: detection channel modified"));
		flag = FALSE;
	}

	if (m_p_detect_parameters_->extract_channel < 0
		|| m_p_detect_parameters_->extract_channel >= wave_format->scan_count)
	{
		m_p_detect_parameters_->extract_channel = 0;
		AfxMessageBox(_T("Spike detection parameters: channel extracted modified"));
		flag = FALSE;
	}
	return flag;
}

boolean ViewSpikeDetection::update_data_file(BOOL b_update_interface)
{
	const auto pdb_doc = GetDocument();
	const auto p_data_file = pdb_doc->open_current_data_file();
	if (p_data_file == nullptr)
		return false;

	p_data_file->read_data_infos();
	const auto wave_format = p_data_file->get_wave_format();

	// if the number of data channels of the data source has changed, load a new set of parameters
	// keep one array of spike detection parameters per data acquisition configuration (ie nb of acquisition channels)
	if (scan_count_ != wave_format->scan_count)
	{
		// save current set of parameters if scan count >= 0 this might not be necessary
		const auto channel_array = spike_detection_array_->get_chan_array(scan_count_);
		if (scan_count_ >= 0)
			*channel_array = spk_detect_array_;
		// Get parameters from the application array
		scan_count_ = wave_format->scan_count;
		spk_detect_array_ = *(spike_detection_array_->get_chan_array(scan_count_));
		// select by default the first set of detection parameters
		m_i_detect_parameters_ = 0;
		m_p_detect_parameters_ = spk_detect_array_.get_item(m_i_detect_parameters_);
	}

	// check if detection parameters are valid
	check_detection_settings();

	// update combo boxes
	if (m_detect_channel_combo.GetCount() != wave_format->scan_count)
	{
		update_combos_detect_and_transforms();
	}
	// change doc attached to line view
	chart_data_filtered_.attach_data_file(p_data_file);
	chart_data_.attach_data_file(p_data_file);

	// update source view display
	if (chart_data_filtered_.get_channel_list_size() < 1)
	{
		chart_data_filtered_.remove_all_channel_list_items();
		chart_data_filtered_.add_channel_list_item(0, 0);
		CChanlistItem* channel_item = chart_data_filtered_.get_channel_list_item(0);
		channel_item->set_color(0);
		chart_data_filtered_.hz_tags.remove_all_tags();
		m_p_detect_parameters_->detect_threshold_bin = channel_item->convert_volts_to_data_bins(m_p_detect_parameters_->detect_threshold_mv / 1000.f);
		chart_data_filtered_.hz_tags.add_tag(m_p_detect_parameters_->detect_threshold_bin, 0);
	}

	//add all channels to detection window
	auto channel_list_size = chart_data_.get_channel_list_size();
	const int n_document_channels = wave_format->scan_count;
	for (auto i = 0; i < n_document_channels; i++)
	{
		// check if present in the list
		auto b_present = FALSE;
		for (auto j = channel_list_size - 1; j >= 0; j--)
		{
			// test if this data chan is present + no transformation
			const CChanlistItem* channel_item = chart_data_.get_channel_list_item(j);
			if (channel_item->get_source_chan() == i
				&& channel_item->get_transform_mode() == 0)
			{
				b_present = TRUE;
				break;
			}
		}
		if (b_present == FALSE) // no display chan contains that doc chan
		{
			chart_data_.add_channel_list_item(i, 0);
			channel_list_size++;
		}
		chart_data_.get_channel_list_item(i)->set_color(static_cast<WORD>(i));
	}

	// if browse through another file ; keep previous display parameters & load data
	auto l_first = chart_data_filtered_.get_data_first_index();
	auto l_last = chart_data_filtered_.get_data_last_index();
	if (options_view_data_->b_complete_record && b_update_interface)
	{
		l_first = 0;
		l_last = p_data_file->get_doc_channel_length() - 1;
	}
	chart_data_filtered_.get_data_from_doc(l_first, l_last);
	chart_data_.get_data_from_doc(l_first, l_last);

	if (b_update_interface)
	{
		chart_data_filtered_.Invalidate();
		chart_data_.Invalidate();
		// adjust scroll bar (size of button and left/right limits)
		file_scrollbar_infos_.fMask = SIF_ALL;
		file_scrollbar_infos_.nMin = 0;
		file_scrollbar_infos_.nMax = chart_data_filtered_.get_data_last_index();
		file_scrollbar_infos_.nPos = 0;
		file_scrollbar_infos_.nPage = chart_data_filtered_.get_data_last_index() - chart_data_filtered_.get_data_first_index() + 1;
		file_scrollbar_.SetScrollInfo(&file_scrollbar_infos_);

		m_data_comments = wave_format->get_comments(_T(" "));
		m_sampling_rate_ = wave_format->sampling_rate_per_channel;
		m_b_valid_threshold_ = FALSE;
	}
	return true;
}

void ViewSpikeDetection::update_combos_detect_and_transforms()
{
	const auto db_document = GetDocument();
	const auto p_data_file = db_document->m_p_data_doc;
	const auto channel_array = p_data_file->get_wave_channels_array();
	const auto wave_format = p_data_file->get_wave_format();

	// load channel names
	CString comment;
	m_detect_channel_combo.ResetContent();
	const int channel_count = wave_format->scan_count;
	for (auto i = 0; i < channel_count; i++)
	{
		comment.Format(_T("%i: "), i);
		comment += channel_array->get_p_channel(i)->am_csComment;
		VERIFY(m_detect_channel_combo.AddString(comment) != CB_ERR);
	}

	// load transforms names
	m_transform_combo.ResetContent();
	m_transform2_combo.ResetContent();
	const int n_transform_types = AcqDataDoc::get_transforms_count();
	for (auto j = 0; j < n_transform_types; j++)
	{
		VERIFY(m_transform_combo.AddString(p_data_file->get_transform_name(j)) != CB_ERR);
		VERIFY(m_transform2_combo.AddString(p_data_file->get_transform_name(j)) != CB_ERR);
	}
	m_detect_channel_combo.SetCurSel(m_p_detect_parameters_->detect_channel);
}

void ViewSpikeDetection::define_stretch_parameters()
{
	stretch_.attach_parent(this);

	// top right ----------------------------------------
	stretch_.new_prop(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);

	stretch_.new_prop(IDC_EMPTYPICTURE, XLEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_slave_prop(IDC_DISPLAYDATA, XLEQ_XREQ, SZPR_YTEQ, IDC_EMPTYPICTURE);
	stretch_.new_slave_prop(IDC_DISPLAYDETECT, XLEQ_XREQ, SZPR_YBEQ, IDC_EMPTYPICTURE);

	stretch_.new_slave_prop(IDC_CHANSELECTED2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	stretch_.new_slave_prop(IDC_GAIN2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	stretch_.new_slave_prop(IDC_BIAS2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	stretch_.new_slave_prop(IDC_SCROLLY2, SZEQ_XREQ, YTEQ_YBEQ, IDC_DISPLAYDATA);
	stretch_.new_slave_prop(IDC_STATICDISPLAYDATA, SZEQ_XLEQ, YTEQ_YBEQ, IDC_DISPLAYDATA);

	stretch_.new_prop(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);

	stretch_.new_slave_prop(IDC_CHANSELECTED, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	stretch_.new_slave_prop(IDC_GAIN, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	stretch_.new_slave_prop(IDC_BIAS, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	stretch_.new_slave_prop(IDC_SCROLLY, SZEQ_XREQ, YTEQ_YBEQ, IDC_DISPLAYDETECT);
	stretch_.new_slave_prop(IDC_STATICDISPLAYDETECT, SZEQ_XLEQ, YTEQ_YBEQ, IDC_DISPLAYDETECT);

	// bottom right  ------------------------------------
	stretch_.new_prop(IDC_CHART_BARS, XLEQ_XREQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_STATICDISPLAYBARS, SZEQ_XLEQ, SZEQ_YBEQ);

	stretch_.new_prop(IDC_DURATIONTEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_SPIKEWINDOWLENGTH, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_MINTEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_SPIKEWINDOWAMPLITUDE, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_NBSPIKES_NB, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_NBSPIKES_TEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_TRANSFORM2, SZEQ_XLEQ, SZEQ_YBEQ);

	stretch_.new_prop(IDC_STATIC3, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_SPIKENO, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_ARTEFACT, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_DISPLAY_SPIKES, SZEQ_XLEQ, SZEQ_YBEQ);

	stretch_.new_prop(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_SOURCE, SZEQ_XREQ, SZEQ_YBEQ);

	stretch_.new_prop(IDC_XSCALE, XLEQ_XREQ, SZEQ_YBEQ);
}

void ViewSpikeDetection::define_sub_classed_items()
{
	// attach controls
	VERIFY(file_scrollbar_.SubclassDlgItem(IDC_FILESCROLL, this));
	VERIFY(m_transform_combo.SubclassDlgItem(IDC_TRANSFORM, this));
	VERIFY(m_transform2_combo.SubclassDlgItem(IDC_TRANSFORM2, this));
	VERIFY(m_detect_channel_combo.SubclassDlgItem(IDC_SOURCECHAN, this));
	VERIFY(m_detect_what_combo.SubclassDlgItem(IDC_DETECT_WHAT, this));

	VERIFY(mm_spike_no_.SubclassDlgItem(IDC_SPIKENO, this));
	mm_spike_no_.ShowScrollBar(SB_VERT);
	VERIFY(mm_threshold_val_.SubclassDlgItem(IDC_THRESHOLDVAL, this));
	mm_threshold_val_.ShowScrollBar(SB_VERT);
	VERIFY(mm_time_first_.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_time_last_.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(mm_spk_wnd_duration_.SubclassDlgItem(IDC_SPIKEWINDOWLENGTH, this));
	VERIFY(mm_spk_wnd_amplitude_.SubclassDlgItem(IDC_SPIKEWINDOWAMPLITUDE, this));
	VERIFY(mm_selected_channel_.SubclassDlgItem(IDC_CHANSELECTED, this));
	VERIFY(mm_selected_channel2_.SubclassDlgItem(IDC_CHANSELECTED2, this));

	// control derived from CChartWnd
	VERIFY(chart_spike_shape_.SubclassDlgItem(IDC_DISPLAY_SPIKES, this));
	VERIFY(chart_spike_bar_.SubclassDlgItem(IDC_CHART_BARS, this));
	VERIFY(chart_data_filtered_.SubclassDlgItem(IDC_DISPLAYDETECT, this));
	VERIFY(chart_data_.SubclassDlgItem(IDC_DISPLAYDATA, this));

	// load left scrollbar and button
	VERIFY(m_scroll_y_.SubclassDlgItem(IDC_SCROLLY, this));
	m_scroll_y_.SetScrollRange(0, 100);
	m_h_bias_ = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_h_zoom_ = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_bias_)));
	GetDlgItem(IDC_GAIN)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_zoom_)));
	VERIFY(m_scroll_y2_.SubclassDlgItem(IDC_SCROLLY2, this));
	m_scroll_y2_.SetScrollRange(0, 100);
	m_h_bias2_ = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_h_zoom2_ = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS2)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_bias2_)));
	GetDlgItem(IDC_GAIN2)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_zoom2_)));

	VERIFY(m_ruler_bar_abscissa.SubclassDlgItem(IDC_XSCALE, this));
}

void ViewSpikeDetection::OnInitialUpdate()
{
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	spike_detection_array_ = &(p_app->spk_detect_array); 
	options_view_data_ = &(p_app->options_view_data);

	m_ruler_bar_abscissa.attach_scope_wnd(&chart_data_filtered_, TRUE);
	chart_data_filtered_.attach_external_x_ruler(&m_ruler_bar_abscissa);

	define_stretch_parameters();
	b_init_ = TRUE;
	m_auto_increment = true;
	m_auto_detect = true;

	define_sub_classed_items();

	ViewDbTable::OnInitialUpdate();

	// load file data
	if (chart_data_filtered_.hz_tags.get_tag_list_size() < 1)
		chart_data_filtered_.hz_tags.add_tag(0, 0);

	update_file_parameters(TRUE);

	chart_data_filtered_.set_scope_parameters(&(options_view_data_->view_data));
	chart_data_filtered_.Invalidate();

	chart_data_.set_scope_parameters(&(options_view_data_->view_data));
	chart_data_.Invalidate();

	on_format_split_curves();
	
}

LRESULT ViewSpikeDetection::on_my_message(const WPARAM w_param, const LPARAM l_param)
{
	int threshold = LOWORD(l_param);
	const int i_id = HIWORD(l_param);

	// ----------------------------- change mouse cursor (all 3 items)
	switch (w_param)
	{
	case HINT_SET_MOUSE_CURSOR:
		if (threshold > CURSOR_VERTICAL)
			threshold = 0;
		if (threshold == CURSOR_CROSS)
			threshold = CURSOR_VERTICAL;
		m_cursor_state_ = threshold;
		set_view_mouse_cursor(threshold);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(m_cursor_state_, 0));
		break;

		// ----------------------------- move horizontal cursor / source data
	case HINT_MOVE_HZ_TAG:
		m_p_detect_parameters_->detect_threshold_bin = chart_data_filtered_.hz_tags.get_value_int(threshold);
		m_threshold_val = chart_data_filtered_.get_channel_list_item(0)
			->convert_data_bins_to_volts(
				chart_data_filtered_.hz_tags.get_value_int(threshold)) * 1000.f;
		m_p_detect_parameters_->detect_threshold_mv = m_threshold_val;
		mm_threshold_val_.m_b_entry_done = TRUE;
		on_en_change_threshold();
		break;

		// ----------------------------- select bar/display bars or zoom
	case HINT_CHANGE_HZ_LIMITS: 
		chart_data_filtered_.get_data_from_doc(chart_spike_bar_.get_time_first(), chart_spike_bar_.get_time_last());
		chart_data_.get_data_from_doc(chart_spike_bar_.get_time_first(), chart_spike_bar_.get_time_last());
		update_legends();
		break;

	case HINT_HIT_SPIKE:
		{
			db_spike spike_hit = GetDocument()->get_spike_hit();
			select_spike_no(spike_hit, FALSE); 
			update_spike_display();
		}
		break;

	case HINT_DBL_CLK_SEL:
		if (threshold < 0)
			threshold = 0;
		m_spike_index = threshold;
		on_tools_edit_spikes();
		break;

	case HINT_CHANGE_ZOOM:
		update_spike_shape_window_scale(TRUE);
		chart_spike_shape_.Invalidate();
		break;

	case HINT_VIEW_SIZE_CHANGED:
		if (i_id == chart_data_.GetDlgCtrlID())
		{
			const auto l_first = chart_data_.get_data_first_index(); // get source data time range
			const auto l_last = chart_data_.get_data_last_index();
			chart_data_filtered_.get_data_from_doc(l_first, l_last);
		}
		// else if(iID == m_displayDetect.GetDlgCtrlID())
		// UpdateLegends updates data window from m_displayDetect
		update_legends();
		break;

	case HINT_WINDOW_PROPS_CHANGED:
		options_view_data_->view_spk_detect_filtered = *chart_data_filtered_.get_scope_parameters();
		options_view_data_->view_spk_detect_data = *chart_data_.get_scope_parameters();
		options_view_data_->view_spk_detect_spk = *chart_spike_bar_.get_scope_parameters();
		options_view_data_->view_spk_detect_bars = *chart_spike_shape_.get_scope_parameters();
		break;

	case HINT_DEFINED_RECT:
		if (m_cursor_state_ == CURSOR_CROSS)
		{
			const auto rect = chart_data_filtered_.get_defined_rect();
			int l_limit_left = chart_data_filtered_.get_data_offset_from_pixel(rect.left);
			int l_limit_right = chart_data_filtered_.get_data_offset_from_pixel(rect.right);
			if (l_limit_left > l_limit_right)
			{
				const int i = l_limit_right;
				l_limit_right = l_limit_left;
				l_limit_left = i;
			}
			p_spk_doc->m_stimulus_intervals.set_at_grow(p_spk_doc->m_stimulus_intervals.n_items, l_limit_left);
			p_spk_doc->m_stimulus_intervals.n_items++;
			p_spk_doc->m_stimulus_intervals.set_at_grow(p_spk_doc->m_stimulus_intervals.n_items, l_limit_right);
			p_spk_doc->m_stimulus_intervals.n_items++;
			update_vt_tags();

			chart_spike_bar_.Invalidate();
			chart_data_filtered_.Invalidate();
			chart_data_.Invalidate();
			p_spk_doc->SetModifiedFlag(TRUE);
		}
		break;

	case HINT_SELECT_SPIKES:
		update_spike_display();
		break;

	//case HINT_HIT_VERT_TAG:	//11	// vertical tag hit				low_p = tag index
	//case HINT_MOVE_VERT_TAG: //12		// vertical tag has moved 		low_p = new pixel / selected tag
	case HINT_CHANGE_VERT_TAG: //13
		{
			int lvalue = p_spk_doc->m_stimulus_intervals.get_at(threshold);
			if (i_id == chart_data_filtered_.GetDlgCtrlID())
				lvalue = chart_data_filtered_.vt_tags.get_tag_value_long(threshold);
			else if (i_id == chart_data_.GetDlgCtrlID())
				lvalue = chart_data_.vt_tags.get_tag_value_long(threshold);

			p_spk_doc->m_stimulus_intervals.set_at(threshold, lvalue);
			update_vt_tags();

			chart_spike_bar_.Invalidate();
			chart_data_filtered_.Invalidate();
			chart_data_.Invalidate();
			p_spk_doc->SetModifiedFlag(TRUE);
		}
		break;

	case WM_LBUTTONDOWN:
	case HINT_L_MOUSE_BUTTON_DOWN_CTRL:
		{
			const int cx = LOWORD(l_param);
			const int l_limit_left = chart_data_filtered_.get_data_offset_from_pixel(cx);
			p_spk_doc->m_stimulus_intervals.set_at_grow(p_spk_doc->m_stimulus_intervals.n_items, l_limit_left);
			p_spk_doc->m_stimulus_intervals.n_items++;
			update_vt_tags();

			chart_spike_bar_.Invalidate();
			chart_data_filtered_.Invalidate();
			chart_data_.Invalidate();
			p_spk_doc->SetModifiedFlag(TRUE);
		}
		break;

	case HINT_HIT_SPIKE_SHIFT: // spike is selected or deselected
		{
			//db_spike spike_hit = GetDocument()->get_spike_hit();
			long l_first; 
			long l_last;
			p_spk_list->get_range_of_spike_flagged(l_first, l_last);
			const auto l_time = p_spk_list->get_spike(threshold)->get_time();
			if (l_time < l_first)
				l_first = l_time;
			if (l_time > l_last)
				l_last = l_time;
			p_spk_list->flag_range_of_spikes(l_first, l_last, TRUE);
			update_spike_display();
		}
		break;

	case HINT_HIT_SPIKE_CTRL: // add/remove selected spike to/from the group of spikes selected
		{
			db_spike spike_hit = GetDocument()->get_spike_hit();
			select_spike_no(spike_hit, TRUE);
			update_spike_display();
		}
		break;

	case HINT_HIT_CHANNEL: // change channel if different
		if (i_id == chart_data_filtered_.GetDlgCtrlID())
		{
			if (m_selected_channel != threshold)
			{
				m_selected_channel = threshold; // get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED, m_selected_channel);
			}
		}
		else if (i_id == chart_data_.GetDlgCtrlID())
		{
			if (m_selected_channel2 != threshold)
			{
				m_selected_channel2 = threshold; // get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED2, m_selected_channel2);
			}
		}
		break;

	case HINT_VIEW_TAB_CHANGE:
		update_detection_settings(LOWORD(l_param));
		break;

	default:
		break;
	}
	return 0L;
}

void ViewSpikeDetection::on_first_frame()
{
	scroll_file(SB_LEFT, 1L);
}

void ViewSpikeDetection::on_last_frame()
{
	scroll_file(SB_RIGHT, 1L);
}

void ViewSpikeDetection::OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar)
{
	if (p_scroll_bar == nullptr)
	{
		ViewDbTable::OnHScroll(n_sb_code, n_pos, p_scroll_bar);
		return;
	}

	CString cs;
	switch (n_sb_code)
	{
		long l_last;
		long l_first;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		file_scrollbar_.GetScrollInfo(&file_scrollbar_infos_, SIF_ALL);
		l_first = file_scrollbar_infos_.nPos;
		l_last = l_first + static_cast<long>(file_scrollbar_infos_.nPage) - 1;
		chart_data_filtered_.get_data_from_doc(l_first, l_last);
		chart_data_.get_data_from_doc(l_first, l_last);
		update_legends();
		break;

	default:
		scroll_file(n_sb_code, n_pos);
		break;
	}
}

void ViewSpikeDetection::update_file_scroll()
{
	file_scrollbar_infos_.fMask = SIF_PAGE | SIF_POS;
	file_scrollbar_infos_.nPos = chart_data_filtered_.get_data_first_index();
	file_scrollbar_infos_.nPage = chart_data_filtered_.get_data_last_index() - chart_data_filtered_.get_data_first_index() + 1;
	file_scrollbar_.SetScrollInfo(&file_scrollbar_infos_);
}

void ViewSpikeDetection::scroll_file(const UINT n_sb_code, const UINT n_pos)
{
	auto b_result = FALSE;
	// get corresponding data
	switch (n_sb_code)
	{
	case SB_LEFT: 
	case SB_LINELEFT: 
	case SB_LINERIGHT: 
	case SB_PAGELEFT: 
	case SB_PAGERIGHT: 
	case SB_RIGHT: 
		b_result = chart_data_.scroll_data_from_doc(n_sb_code);
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: 
		b_result = chart_data_.get_data_from_doc(
			static_cast<long>(n_pos) * (chart_data_.get_document_last()) / 100L);
		break;
	default: // NOP: set position only
		break;
	}
	// adjust display
	if (b_result)
		update_legends();
}

void ViewSpikeDetection::on_format_y_scale_center_curve()
{
	chart_data_filtered_.center_chan(0);
	chart_data_filtered_.Invalidate();

	for (auto i = 0; i < chart_data_.get_channel_list_size(); i++)
		chart_data_.center_chan(i);
	chart_data_.Invalidate();

	chart_spike_bar_.center_curve();
	chart_spike_bar_.Invalidate();

	chart_spike_shape_.set_yw_ext_org(chart_spike_bar_.get_yw_extent(), chart_spike_bar_.get_yw_org());
	update_spike_shape_window_scale(TRUE);
	chart_spike_shape_.Invalidate();
}

void ViewSpikeDetection::on_format_y_scale_gain_adjust()
{
	chart_data_filtered_.max_gain_chan(0);
	chart_data_filtered_.set_channel_list_volts_extent(-1, nullptr);
	chart_data_filtered_.Invalidate();

	for (int i = 0; i < chart_data_.get_channel_list_size(); i++)
		chart_data_.max_gain_chan(i);
	chart_data_.set_channel_list_volts_extent(-1, nullptr);
	chart_data_.Invalidate();

	chart_spike_bar_.max_center();
	chart_spike_bar_.Invalidate();

	chart_spike_shape_.set_yw_ext_org(chart_spike_bar_.get_yw_extent(), chart_spike_bar_.get_yw_org());
	chart_spike_shape_.set_xw_ext_org(p_spk_list->get_spike_length(), 0);
	update_spike_shape_window_scale(FALSE);
	chart_spike_shape_.Invalidate();
}

void ViewSpikeDetection::on_format_split_curves()
{
	chart_data_.split_channels();
	chart_data_.set_channel_list_volts_extent(-1, nullptr);
	chart_data_.Invalidate();

	chart_data_filtered_.split_channels();
	chart_data_filtered_.set_channel_list_volts_extent(-1, nullptr);
	chart_data_filtered_.Invalidate();

	// center curve and display bar & spikes
	chart_spike_bar_.max_center();
	chart_spike_bar_.Invalidate();

	chart_spike_shape_.set_yw_ext_org(chart_spike_bar_.get_yw_extent(), chart_spike_bar_.get_yw_org());
	chart_spike_shape_.Invalidate();

	update_spike_shape_window_scale(FALSE);
	
}

void ViewSpikeDetection::on_format_all_data()
{
	const auto l_last = GetDocument()->m_p_data_doc->get_doc_channel_length();
	chart_data_filtered_.resize_channels(0, l_last);
	chart_data_filtered_.get_data_from_doc(0, l_last);

	chart_data_.resize_channels(0, l_last);
	chart_data_.get_data_from_doc(0, l_last);

	const auto x_we = p_spk_list->get_spike_length();
	if (x_we != chart_spike_shape_.get_xw_extent() || 0 != chart_spike_shape_.get_xw_org())
		chart_spike_shape_.set_xw_ext_org(x_we, 0);
	chart_spike_bar_.center_curve();
	chart_spike_shape_.set_yw_ext_org(chart_spike_bar_.get_yw_extent(), chart_spike_bar_.get_yw_org());

	update_legends();
}

void ViewSpikeDetection::update_detection_parameters()
{
	// refresh pointer to spike detection array
	const auto detect_array_size = spk_detect_array_.get_size();
	ASSERT(m_i_detect_parameters_ < detect_array_size);
	m_p_detect_parameters_ = spk_detect_array_.get_item(m_i_detect_parameters_);
	update_detection_settings(m_i_detect_parameters_);

	// make sure that tabs are identical to what has been changed
	update_tabs();
	update_combo_box();
	update_legend_detection_wnd();

	// update static controls
	GetDlgItem(IDC_STATIC10)->Invalidate();
	GetDlgItem(IDC_STATIC1)->Invalidate();
	GetDlgItem(IDC_STATIC2)->Invalidate();
	GetDlgItem(IDC_STATIC6)->Invalidate();
	GetDlgItem(IDC_LOCATEBTTN)->Invalidate();

	// update CEditControls
	GetDlgItem(IDC_DETECT_WHAT)->Invalidate();
	GetDlgItem(IDC_SOURCECHAN)->Invalidate();
	GetDlgItem(IDC_TRANSFORM)->Invalidate();
	mm_threshold_val_.Invalidate();
}

void ViewSpikeDetection::on_tools_detection_parameters()
{
	DlgSpikeDetect dlg;
	dlg.m_db_doc = GetDocument()->m_p_data_doc;
	dlg.m_i_detect_parameters_dlg = m_i_detect_parameters_; // index spk detect parameters currently selected / array
	dlg.m_p_detect_settings_array = &spk_detect_array_; // spike detection parameters array
	dlg.md_pm = options_view_data_;
	dlg.m_p_chart_data_detect_wnd = &chart_data_filtered_;
	if (IDOK == dlg.DoModal())
	{
		// copy modified parameters into array
		// update HorizontalCursorList on both sourceView & histogram
		m_i_detect_parameters_ = dlg.m_i_detect_parameters_dlg;
		update_detection_parameters();
	}
}

void ViewSpikeDetection::on_sel_change_detect_chan()
{
	UpdateData(TRUE);
	m_p_detect_parameters_->detect_channel = m_detect_channel_combo.GetCurSel();
	m_p_detect_parameters_->b_changed = TRUE;
	chart_data_filtered_.set_channel_list_y(0, m_p_detect_parameters_->detect_channel, m_p_detect_parameters_->detect_transform);
	const CChanlistItem* channel_list_item = chart_data_filtered_.get_channel_list_item(0);
	m_p_detect_parameters_->detect_threshold_bin = channel_list_item->convert_volts_to_data_bins(m_p_detect_parameters_->detect_threshold_mv / 1000.f);
	chart_data_filtered_.move_hz_tag_to_val(0, m_p_detect_parameters_->detect_threshold_bin);
	chart_data_filtered_.get_data_from_doc();
	chart_data_filtered_.auto_zoom_chan(0);
	chart_data_filtered_.Invalidate();
}

void ViewSpikeDetection::on_sel_change_transform()
{
	UpdateData(TRUE);
	m_p_detect_parameters_->detect_transform = m_transform_combo.GetCurSel();
	m_p_detect_parameters_->b_changed = TRUE;
	chart_data_filtered_.set_channel_list_transform_mode(0, m_p_detect_parameters_->detect_transform);
	chart_data_filtered_.get_data_from_doc();
	chart_data_filtered_.auto_zoom_chan(0);
	chart_data_filtered_.Invalidate();
	update_legend_detection_wnd();
}

void ViewSpikeDetection::on_measure_all()
{
	detect_all(TRUE);
}

void ViewSpikeDetection::on_measure()
{
	detect_all(FALSE);
}

void ViewSpikeDetection::detect_all(const BOOL b_all)
{
	m_b_detected_ = TRUE;
	p_spk_doc->SetModifiedFlag(TRUE);

	const auto db_document = GetDocument();
	const auto data_document = db_document->m_p_data_doc;
	p_spk_doc->set_acq_filename(db_document->db_get_current_dat_file_name());
	p_spk_doc->init_source_doc(data_document);

	p_spk_doc->set_detection_date(CTime::GetCurrentTime());
	auto old_spike_list_index = db_document->get_current_spike_file()->get_index_current_spike_list();
	m_spike_index = -1;

	// check if detection parameters are ok? prevent detection from a channel that does not exist
	const auto p_dat = db_document->m_p_data_doc;
	if (p_dat == nullptr)
		return;
	const auto wave_format = p_dat->get_wave_format();
	const auto chan_max = wave_format->scan_count - 1;
	for (auto i = 0; i < spk_detect_array_.get_size(); i++)
	{
		const auto spike_detect_array = spk_detect_array_.get_item(i);
		if (spike_detect_array->extract_channel > chan_max)
		{
			MessageBox(_T(
				"Check spike detection parameters \n- one of the detection channel requested \nis not available in the source data"));
			return;
		}
	}

	// adjust size of spike list array
	if (spk_detect_array_.get_size() != p_spk_doc->get_spike_list_size())
		p_spk_doc->set_spike_list_size(spk_detect_array_.get_size());

	// detect spikes from all channels marked as such
	for (int i = 0; i < spk_detect_array_.get_size(); i++)
	{
		if (!b_all && m_i_detect_parameters_ != i)
			continue;
		// detect missing data channel
		if (spk_detect_array_.get_item(i)->extract_channel > chan_max)
			continue;

		// select new spike list (list with no spikes for stimulus channel)
		SpikeList* spike_list = p_spk_doc->set_index_current_spike_list(i);
		if (spike_list == nullptr)
		{
			p_spk_doc->add_spk_list();
			spike_list = p_spk_doc->get_spike_list_current();
		}

		p_spk_list = spike_list;
		if (p_spk_list->get_spikes_count() == 0)
		{
			options_detect_spikes* pFC = spk_detect_array_.get_item(i);
			ASSERT_VALID(pFC);
			ASSERT(pFC != NULL);
			p_spk_list->init_spike_list(db_document->m_p_data_doc, pFC);
		}
		if ((spk_detect_array_.get_item(i))->detect_what == DETECT_SPIKES)
		{
			detect_method_1(static_cast<WORD>(i)); 
		}
		else
		{
			detect_stimulus_1(i); 
			update_vt_tags(); 
			chart_spike_bar_.Invalidate();
			chart_data_filtered_.Invalidate();
			chart_data_.Invalidate();
		}
	}

	// save spike file
	save_current_spk_file();

	// display data
	if (old_spike_list_index < 0)
		old_spike_list_index = 0;
	p_spk_list = p_spk_doc->set_index_current_spike_list(old_spike_list_index);

	chart_spike_bar_.set_source_data(p_spk_list, db_document);
	chart_spike_shape_.set_source_data(p_spk_list, db_document);


	// center spikes, change nb spikes and update content of draw buttons
	if (options_view_data_->b_maximize_gain
		|| chart_spike_bar_.get_yw_extent() == 0
		|| chart_spike_bar_.get_yw_org() == 0
		|| chart_spike_shape_.get_yw_extent() == 0
		|| chart_spike_shape_.get_yw_org() == 0)
	{
		chart_spike_bar_.max_center();
		chart_spike_shape_.set_yw_ext_org(chart_spike_bar_.get_yw_extent(), chart_spike_bar_.get_yw_org());
		int spike_length = 60;
		if (p_spk_list != nullptr)
			spike_length = p_spk_list->get_spike_length();
		chart_spike_shape_.set_xw_ext_org(spike_length, 0);
		update_spike_shape_window_scale(FALSE);
	}

	highlight_spikes_in_chart_data(FALSE);
	update_legends();
	update_tabs();
}

int ViewSpikeDetection::detect_stimulus_1(const int channel_index)
{
	const auto detect_parameters = spk_detect_array_.get_item(channel_index);
	const auto threshold = detect_parameters->detect_threshold_bin;
	const auto detect_transform = detect_parameters->detect_transform; 
	const auto source_channel = detect_parameters->detect_channel;
	const auto data_document = GetDocument()->m_p_data_doc;
	const auto detect_transform_span = AcqDataDoc::get_transformed_data_span(detect_transform); 

	// detect mode: 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);
	auto b_cross_upw = TRUE;
	if (detect_parameters->detect_mode == MODE_OFF_ON || detect_parameters->detect_mode == MODE_OFF_OFF)
		b_cross_upw = FALSE;
	auto b_mode = TRUE;
	if (detect_parameters->detect_mode == MODE_ON_ON || detect_parameters->detect_mode == MODE_OFF_OFF)
		b_mode = FALSE;
	auto b_save_on = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = chart_data_filtered_.get_data_first_index();
	const auto l_data_last = chart_data_filtered_.get_data_last_index();

	// plot progress dialog box
	DlgProgress dlg;
	dlg.Create();
	dlg.set_range(0, 100);
	const auto l_data_len = l_data_last - l_data_first;
	const auto l_data_first0 = l_data_first;
	dlg.SetWindowText(_T("Detect trigger events..."));

	// loop through data defined in the line view window
	while (l_data_first < l_data_last)
	{
		auto l_rw_first = l_data_first; 
		auto l_rw_last = l_data_last; 
		if (!data_document->load_raw_data(&l_rw_first, &l_rw_last, detect_transform_span))
			break; 
		if (!data_document->build_transformed_data(detect_transform, source_channel))
			break;

		// compute initial offset (address of first point)
		auto l_last = l_rw_last;
		if (l_last > l_data_last)
			l_last = l_data_last;
		const int i_buf_first = l_data_first - data_document->get_doc_channel_index_first();
		const auto p_data_first = data_document->get_transformed_data_element(i_buf_first);

		// DETECT STIMULUS ---------------------------------------------------------------
		// detect event if value above threshold
		for (long cx = l_data_first; cx <= l_last; cx++)
		{
			const auto p_data = p_data_first + (cx - l_data_first);
			const int val = *p_data;
			if (b_cross_upw)
			{
				if (val <= threshold)
					continue; 

				b_cross_upw = FALSE;
				b_save_on = ~b_save_on;
				if (!b_mode && !b_save_on)
					continue;
			}
			else
			{
				if (val >= threshold) 
					continue; 

				// add element
				b_cross_upw = TRUE;
				b_save_on = ~b_save_on;
				if (!b_mode && !b_save_on)
					continue;
			}

			const int pct_achieved = MulDiv(cx - l_data_first0, 100, l_data_len);
			dlg.set_pos(pct_achieved);
			CString comment;
			comment.Format(_T("Processing stimulus event: %i"), p_spk_doc->m_stimulus_intervals.n_items + 1);
			dlg.set_status(comment);

			if (dlg.check_cancel_button())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				{
					l_last = l_data_last;
					const auto stimulus_intervals = &(p_spk_doc->m_stimulus_intervals);
					stimulus_intervals->remove_all();
					p_spk_doc->m_stimulus_intervals.n_items = 0;
					break;
				}

			// check if already present and insert it at the proper place
			const auto stimulus_intervals = &(p_spk_doc->m_stimulus_intervals);
			auto flag = TRUE;
			int i2;
			for (i2 = 0; i2 < stimulus_intervals->get_size(); i2++)
			{
				constexpr auto jitter = 2;
				const auto l_value = stimulus_intervals->get_at(i2);
				if (cx <= (l_value + jitter) && cx >= (l_value - jitter))
				{
					flag = FALSE; 
					break;
				}
				if (cx < l_value - jitter)
				{
					flag = TRUE; 
					break;
				}
			}
			if (flag)
			{
				stimulus_intervals->insert_at(i2, cx);
				p_spk_doc->m_stimulus_intervals.n_items++;
			}
		}
		l_data_first = l_last + 1; // update for next loop
	}

	return p_spk_doc->m_stimulus_intervals.n_items;
}

int ViewSpikeDetection::detect_method_1(const WORD channel_index)
{
	const options_detect_spikes* spike_detection_parameters = spk_detect_array_.get_item(channel_index);
	if (spike_detection_parameters->extract_transform != spike_detection_parameters->detect_transform &&
		spike_detection_parameters->extract_transform != 0)
	{
		AfxMessageBox(
			_T("Options not implemented yet!\ndetected chan = extracted chan or !extracted chan=raw data\nChange detection parameters"));
		return p_spk_list->get_spikes_count();
	}

	// set parameters (copy array into local parameters)
	const auto threshold = spike_detection_parameters->detect_threshold_bin; 
	const auto detect_transform = spike_detection_parameters->detect_transform;
	const auto source_channel = spike_detection_parameters->detect_channel; 
	const auto pre_threshold = spike_detection_parameters->detect_pre_threshold; 
	const auto refractory = spike_detection_parameters->detect_refractory_period; 
	const auto post_threshold = spike_detection_parameters->extract_n_points - pre_threshold;

	// get parameters from document
	const auto p_dat = GetDocument()->m_p_data_doc;
	const int n_channels = p_dat->get_scan_count();
	const auto p_buf = p_dat->get_raw_data_buffer();
	const auto span = AcqDataDoc::get_transformed_data_span(detect_transform);

	// adjust detection method: if threshold lower than data zero detect lower crossing
	auto b_cross_upw = TRUE;
	if (threshold < 0)
		b_cross_upw = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = chart_data_filtered_.get_data_first_index(); // index first pt to test
	auto l_data_last = chart_data_filtered_.get_data_last_index(); // index last pt to test
	if (l_data_first < pre_threshold + span)
		l_data_first = static_cast<long>(pre_threshold) + span;
	if (l_data_last > p_dat->get_doc_channel_length() - post_threshold - span)
		l_data_last = p_dat->get_doc_channel_length() - post_threshold - span;

	// loop through data defined in the line_view window
	while (l_data_first < l_data_last)
	{
		auto l_rw_first = l_data_first - pre_threshold;
		auto l_rw_last = l_data_last; 
		if (!p_dat->load_raw_data(&l_rw_first, &l_rw_last, span)) 
			break; 
		if (!p_dat->build_transformed_data(detect_transform, source_channel)) 
			break;

		// load a chunk of data and see if any spikes are detected within it
		// compute initial offset (address of first point
		auto l_last = l_rw_last - post_threshold;
		if (l_last > l_data_last)
			l_last = l_data_last;
		const int i_buf_first = l_data_first - p_dat->get_doc_channel_index_first();
		const auto p_data_first = p_dat->get_transformed_data_element(i_buf_first);

		// DETECT SPIKES ---------------------------------------------------------------
		// detect event if value above threshold
		long ii_time;
		long cx;
		for (cx = l_data_first; cx <= l_last; cx++)
		{
			// ........................................ SPIKE NOT DETECTED
			auto p_data = p_data_first + (cx - l_data_first);

			// detect > threshold ......... if found, search for max
			if (b_cross_upw)
			{
				if (*p_data < threshold)
					continue;

				// search max and threshold crossing
				auto max = *p_data;
				auto p_data1 = p_data; 
				ii_time = cx; 
				// loop to search max
				for (auto i = cx; i < cx + refractory; i++, p_data1++)
				{
					if (max < *p_data1) 
					{
						max = *p_data1;
						p_data = p_data1;
						ii_time = i;
					}
				}
			}
			// detect < threshold ......... if found, search for min
			else
			{
				if (*p_data > threshold) // test if a spike is present
					continue; // no: loop to next point

				// search min and threshold crossing
				auto min = *p_data;
				auto p_data1 = p_data;
				ii_time = cx;
				for (auto i = cx; i < cx + refractory; i++, p_data1++)
				{
					if (min > *p_data1)
					{
						min = *p_data1;
						p_data = p_data1; // p_data = "center" of spike
						ii_time = i;
					}
				}
			}

			// ........................................ SPIKE DETECTED
			if (spike_detection_parameters->extract_transform == spike_detection_parameters->detect_transform)
			{
				const auto p_m = p_data - pre_threshold;
				p_spk_list->add_spike(p_m, 1, ii_time, source_channel, 0, TRUE);
			}
			else // extract from raw data
			{
				const auto pM = p_buf
					+ n_channels * (ii_time - pre_threshold - l_rw_first + span)
					+ spike_detection_parameters->extract_channel;
				p_spk_list->add_spike(pM, n_channels, ii_time, source_channel, 0, TRUE);
			}
			cx = ii_time + refractory;
		}
		l_data_first = cx + 1; // update for next loop
	}

	return p_spk_list->get_spikes_count();
}

void ViewSpikeDetection::on_tools_edit_spikes()
{
	DlgSpikeEdit dlg; 
	dlg.y_extent = chart_spike_shape_.get_yw_extent();
	dlg.y_zero = chart_spike_shape_.get_yw_org(); 
	dlg.x_extent = chart_spike_shape_.get_xw_extent(); 
	dlg.x_zero = chart_spike_shape_.get_xw_org(); 
	dlg.dlg_spike_index = m_spike_index; 
	//p_spk_list->remove_all_spike_flags();
	dlg.db_wave_doc = GetDocument();
	dlg.m_parent = this;
	dlg.DoModal();

	m_spike_index = dlg.dlg_spike_index;
	db_spike spike_sel(-1, -1, m_spike_index);
	select_spike_no(spike_sel, FALSE);
	update_spike_display();
	if (dlg.b_changed)
		p_spk_doc->SetModifiedFlag(TRUE);

	update_legends();
}

void ViewSpikeDetection::on_format_x_scale()
{
	DlgXYParameters dlg;
	CWnd* pFocus = GetFocus();

	if (pFocus != nullptr && chart_data_filtered_.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_x_param = FALSE;
		const CChanlistItem* p_chan = chart_data_filtered_.get_channel_list_item(m_p_detect_parameters_->detect_channel);
		dlg.m_y_zero = p_chan->get_y_zero();
		dlg.m_y_extent = p_chan->get_y_extent();
		dlg.m_b_display_source = TRUE;
	}
	else if (pFocus != nullptr && chart_spike_bar_.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_x_param = FALSE;
		dlg.m_y_zero = chart_spike_bar_.get_yw_org();
		dlg.m_y_extent = chart_spike_bar_.get_yw_extent();
		dlg.m_b_display_bars = TRUE;
	}
	else
	{
		dlg.m_x_zero = chart_spike_shape_.get_xw_org();
		dlg.m_x_extent = chart_spike_shape_.get_xw_extent();
		dlg.m_y_zero = chart_spike_shape_.get_yw_org();
		dlg.m_y_extent = chart_spike_shape_.get_yw_extent();
		dlg.b_display_spikes = TRUE;
	}

	if (IDOK == dlg.DoModal())
	{
		if (dlg.m_b_display_source)
		{
			CChanlistItem* chan = chart_data_filtered_.get_channel_list_item(0);
			chan->set_y_zero(dlg.m_y_zero);
			chan->set_y_extent(dlg.m_y_extent);
			chart_data_filtered_.Invalidate();
		}
		if (dlg.m_b_display_bars)
		{
			chart_spike_bar_.set_yw_ext_org(dlg.m_y_extent, dlg.m_y_zero);
			chart_spike_bar_.Invalidate();
		}
		if (dlg.b_display_spikes)
		{
			chart_spike_shape_.set_yw_ext_org(dlg.m_y_extent, dlg.m_y_zero);
			chart_spike_shape_.set_xw_ext_org(dlg.m_x_extent, dlg.m_x_zero);
			chart_spike_shape_.Invalidate();
		}
	}
}

void ViewSpikeDetection::on_bn_clicked_clear_all()
{
	m_spike_index = -1;
	db_spike spike_sel(-1, -1, -1);
	chart_spike_bar_.select_spike(spike_sel);
	chart_spike_shape_.select_spike(spike_sel);

	// update spike list
	for (int i = 0; i < p_spk_doc->get_spike_list_size(); i++)
	{
		SpikeList* p_spk_list = p_spk_doc->set_index_current_spike_list(i);
		p_spk_list->init_spike_list(GetDocument()->m_p_data_doc, nullptr);
	}
	p_spk_list = p_spk_doc->get_spike_list_current();
	ASSERT(p_spk_list != NULL);

	highlight_spikes_in_chart_data(FALSE); // remove display of spikes
	chart_spike_shape_.set_source_data(p_spk_list, GetDocument());
	p_spk_doc->m_stimulus_intervals.n_items = 0; // zero stimuli
	p_spk_doc->m_stimulus_intervals.remove_all();

	update_detection_parameters();
	update_vt_tags(); // update display of vertical tags
	update_legends(); // change legends
	p_spk_doc->SetModifiedFlag(TRUE); // mark spike document as changed
}

void ViewSpikeDetection::on_clear()
{
	m_spike_index = -1;
	db_spike spike_sel(-1, -1, -1);
	chart_spike_bar_.select_spike(spike_sel);
	chart_spike_shape_.select_spike(spike_sel);

	p_spk_list = p_spk_doc->get_spike_list_current();
	p_spk_list->init_spike_list(GetDocument()->m_p_data_doc, nullptr);
	highlight_spikes_in_chart_data(FALSE);

	if (p_spk_list->get_detection_parameters()->detect_what == DETECT_STIMULUS)
	{
		p_spk_doc->m_stimulus_intervals.n_items = 0;
		p_spk_doc->m_stimulus_intervals.remove_all();
		update_vt_tags();
	}

	update_legends();
	p_spk_doc->SetModifiedFlag(TRUE);
}

void ViewSpikeDetection::on_en_change_spike_no()
{
	if (mm_spike_no_.m_b_entry_done)
	{
		mm_spike_no_.on_en_change(this, m_spike_index, 1, -1);
		// check boundaries
		if (m_spike_index < -1)
			m_spike_index = -1;
		if (m_spike_index >= p_spk_list->get_spikes_count())
			m_spike_index = p_spk_list->get_spikes_count() - 1;

		// update spike num and center display on the selected spike
		db_spike spike_sel(-1, -1, m_spike_index);
		select_spike_no(spike_sel, FALSE);
		update_spike_display();
	}
}

void ViewSpikeDetection::on_artefact()
{
	UpdateData(TRUE); 
	const auto n_spikes = p_spk_list->get_spike_flag_array_count();
	if (n_spikes < 1)
	{
		m_b_artefact = FALSE; // no action if spike index < 0
	}
	else
	{
		// load old class nb
		ASSERT(n_spikes >= 0);
		for (auto i = 0; i < n_spikes; i++)
		{
			const auto spike_no = p_spk_list->get_spike_index_of_flag(i);
			Spike* spike = p_spk_list->get_spike(spike_no);
			auto spike_class = spike->get_class_id();

			// if artefact: set class to negative value
			if ((m_b_artefact && spike_class >= 0) || (spike_class < 0))
				spike_class = -(spike_class + 1);
			spike->set_class_id(spike_class);
		}

		p_spk_doc->SetModifiedFlag(TRUE);
		save_current_spk_file();
	}
	m_spike_index = -1;

	const auto i_sel_parameters = spk_list_tab_ctrl.GetCurSel();
	m_p_detect_parameters_ = spk_detect_array_.get_item(i_sel_parameters);
	p_spk_list = p_spk_doc->set_index_current_spike_list(i_sel_parameters);

	db_spike spike_sel(-1, -1, m_spike_index);
	select_spike_no(spike_sel, FALSE);
	update_spike_display();
	update_number_of_spikes();
}

void ViewSpikeDetection::update_number_of_spikes()
{
	const int total_spikes = p_spk_list->get_spikes_count();
	if (total_spikes != static_cast<int>(GetDlgItemInt(IDC_NBSPIKES_NB)))
		SetDlgItemInt(IDC_NBSPIKES_NB, total_spikes);
}

void ViewSpikeDetection::align_display_to_current_spike()
{
	if (m_spike_index < 0)
		return;

	const auto l_spike_time = p_spk_list->get_spike(m_spike_index)->get_time();
	if (l_spike_time < chart_data_filtered_.get_data_first_index()
		|| l_spike_time > chart_data_filtered_.get_data_last_index())
	{
		const auto l_size = chart_data_filtered_.get_data_last_index() - chart_data_filtered_.get_data_first_index();
		auto l_first = l_spike_time - l_size / 2;
		if (l_first < 0)
			l_first = 0;
		auto l_last = l_first + l_size - 1;
		if (l_last > chart_data_filtered_.get_document_last())
		{
			l_last = chart_data_filtered_.get_document_last();
			l_first = l_last - l_size + 1;
		}
		chart_data_filtered_.get_data_from_doc(l_first, l_last);
		chart_data_.get_data_from_doc(l_first, l_last);

		update_legends();
	}
}

void ViewSpikeDetection::update_spike_shape_window_scale(const BOOL b_set_from_controls)
{
	// get current values
	int ix_we;
	auto iy_we = 0;

	// if set from controls, get value from the controls
	if (b_set_from_controls && p_spk_list->get_spikes_count() > 0)
	{
		// set timescale
		CString cs;
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->GetWindowText(cs);
		const auto x = static_cast<float>(_ttof(cs)) / 1000.0f;
		ix_we = static_cast<int>(p_spk_list->get_acq_sampling_rate() * x);
		if (ix_we == 0)
			ix_we = p_spk_list->get_detection_parameters()->extract_n_points;
		ASSERT(ix_we != 0);
		chart_spike_shape_.set_xw_ext_org(ix_we, chart_spike_shape_.get_xw_org());
		
		// set amplitude
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->GetWindowText(cs);
		if (!cs.IsEmpty())
		{
			const auto y = static_cast<float>(_ttof(cs)) / 1000.0f;
			iy_we = static_cast<int>(y / p_spk_list->get_acq_volts_per_bin());
		}
		if (iy_we == 0)
			iy_we = chart_spike_shape_.get_yw_extent();
		chart_spike_shape_.set_yw_ext_org(iy_we, chart_spike_shape_.get_yw_org());
	}
	else
	{
		ix_we = chart_spike_shape_.get_xw_extent();
		iy_we = chart_spike_shape_.get_yw_extent();
	}

	if (ix_we != 0 && iy_we != 0)
	{
		const float x = chart_spike_shape_.get_extent_ms() / static_cast<float>(chart_spike_shape_.get_nx_scale_cells());
		chart_spike_shape_.set_x_scale_unit_value(x);

		const float y = chart_spike_shape_.get_extent_mv() / static_cast<float>(chart_spike_shape_.get_ny_scale_cells());
		chart_spike_shape_.set_y_scale_unit_value(y);
	}
	
	CString dummy1;
	dummy1.Format(_T("%.3lf"), chart_spike_shape_.get_extent_mv());
	SetDlgItemText(IDC_SPIKEWINDOWAMPLITUDE, dummy1);

	CString dummy2;
	dummy2.Format(_T("%.3lf"), chart_spike_shape_.get_extent_ms());
	SetDlgItemText(IDC_SPIKEWINDOWLENGTH, dummy2);
}

void ViewSpikeDetection::select_spike_no(db_spike& spike_sel, const BOOL b_multiple_selection)
{
	if (spike_sel.spike_index >= 0)
	{
		p_spk_list = p_spk_doc->get_spike_list_current();
		const auto p_spike_element = p_spk_list->get_spike(spike_sel.spike_index);
		m_b_artefact = (p_spike_element->get_class_id() < 0);
		if (b_multiple_selection)
		{
			const int count_flagged_spikes = p_spk_list->toggle_spike_flag(spike_sel.spike_index);
			if (count_flagged_spikes < 1)
				spike_sel.spike_index = -1;
			if (m_spike_index == spike_sel.spike_index)
				spike_sel.spike_index = 0;
		}
		else
		{
			p_spk_list->set_spike_flag(spike_sel.spike_index, true);
			p_spk_list->m_selected_spike = m_spike_index;
		}
		m_spike_index = spike_sel.spike_index;
		align_display_to_current_spike();
	}
	else
	{
		p_spk_list->clear_flagged_spikes();
		m_b_artefact = FALSE;
	}
}

void ViewSpikeDetection::update_spike_display()
{
	// update spike display windows
	chart_spike_bar_.Invalidate(TRUE);
	chart_spike_shape_.Invalidate(TRUE);

	// update Dlg interface
	GetDlgItem(IDC_SPIKENO)->EnableWindow(p_spk_list->get_spike_flag_array_count() <= 1);
	SetDlgItemInt(IDC_SPIKENO, m_spike_index, TRUE);
	CheckDlgButton(IDC_ARTEFACT, m_b_artefact);
}

void ViewSpikeDetection::on_en_change_threshold()
{
	if (mm_threshold_val_.m_b_entry_done)
	{
		const auto threshold_value = m_threshold_val;
		mm_threshold_val_.on_en_change(this, m_threshold_val, 1.f, -1.f);

		// change display if necessary
		if (m_threshold_val < threshold_value || m_threshold_val > threshold_value)
		{
			m_threshold_val = threshold_value;
			m_p_detect_parameters_->detect_threshold_mv = threshold_value;
			const CChanlistItem* channel_item = chart_data_filtered_.get_channel_list_item(0);
			m_p_detect_parameters_->detect_threshold_bin = channel_item->convert_volts_to_data_bins(m_threshold_val / 1000.f);
			chart_data_filtered_.move_hz_tag_to_val(0, m_p_detect_parameters_->detect_threshold_bin);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeDetection::on_en_change_time_first()
{
	if (mm_time_first_.m_b_entry_done)
	{
		mm_time_first_.on_en_change(this, m_time_first, 1.f, -1.f);
		chart_data_filtered_.get_data_from_doc(static_cast<long>(m_time_first * m_sampling_rate_),
			static_cast<long>(m_time_last * m_sampling_rate_));
		chart_data_.get_data_from_doc(static_cast<long>(m_time_first * m_sampling_rate_),
			static_cast<long>(m_time_last * m_sampling_rate_));
		update_legends();
	}
}

void ViewSpikeDetection::on_en_change_time_last()
{
	if (mm_time_last_.m_b_entry_done)
	{
		mm_time_last_.on_en_change(this, m_time_last, 1.f, -1.f);

		chart_data_filtered_.get_data_from_doc(static_cast<long>(m_time_first * m_sampling_rate_),
			static_cast<long>(m_time_last * m_sampling_rate_));
		chart_data_.get_data_from_doc(static_cast<long>(m_time_first * m_sampling_rate_),
			static_cast<long>(m_time_last * m_sampling_rate_));
		update_legends();
	}
}

void ViewSpikeDetection::on_tools_data_series()
{
	// init dialog data
	DlgDataSeries dlg;
	dlg.m_p_chart_data_wnd = &chart_data_filtered_;
	dlg.m_pdb_doc = GetDocument()->m_p_data_doc;
	dlg.m_list_index = 0;

	// invoke dialog box
	dlg.DoModal();
	if (chart_data_filtered_.get_channel_list_size() < 1)
	{
		chart_data_filtered_.remove_all_channel_list_items();
		chart_data_filtered_.add_channel_list_item(m_p_detect_parameters_->detect_channel, m_p_detect_parameters_->detect_transform);
	}
	update_legends();
}

void ViewSpikeDetection::print_data_cartridge(CDC* p_dc, ChartData* p_data_chart_wnd, const CRect* p_rect)
{
	options_scope_struct* p_struct = p_data_chart_wnd->get_scope_parameters();
	const auto b_draw_f = p_struct->b_draw_frame;
	p_struct->b_draw_frame = TRUE;
	p_data_chart_wnd->print(p_dc, p_rect, (options_view_data_->b_contours == 1));
	p_struct->b_draw_frame = b_draw_f;

	// data vertical and horizontal bars
	const auto comments = print_data_bars(p_dc, p_data_chart_wnd, p_rect);

	const int left = p_rect->left;
	const int top = p_rect->top;
	p_dc->TextOut(left, top, comments);

	p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
}

void ViewSpikeDetection::on_edit_copy()
{
	DlgCopyAs dlg;
	dlg.m_n_abscissa = options_view_data_->hz_resolution;
	dlg.m_n_ordinates = options_view_data_->vt_resolution;
	dlg.b_graphics = options_view_data_->b_graphics;
	dlg.m_i_option = options_view_data_->b_contours;
	dlg.m_i_unit = options_view_data_->b_units;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		options_view_data_->b_graphics = dlg.b_graphics;
		options_view_data_->b_contours = dlg.m_i_option;
		options_view_data_->b_units = dlg.m_i_unit;
		options_view_data_->hz_resolution = dlg.m_n_abscissa;
		options_view_data_->vt_resolution = dlg.m_n_ordinates;

		if (!dlg.b_graphics)
			chart_data_filtered_.copy_as_text(dlg.m_i_option, dlg.m_i_unit, dlg.m_n_abscissa);
		else
		{
			serialize_windows_state(b_save);

			CRect old_rect1; // save size of line view windows
			chart_data_filtered_.GetWindowRect(&old_rect1);
			CRect old_rect2;
			chart_data_.GetWindowRect(&old_rect2);

			const CRect rect(0, 0, options_view_data_->hz_resolution, options_view_data_->vt_resolution);
			pixels_count_0_ = chart_data_filtered_.get_rect_width();

			// create meta file
			CMetaFileDC m_dc;
			auto rect_bound = rect;
			rect_bound.right *= 32;
			rect_bound.bottom *= 30;
			const auto p_dc_ref = GetDC();
			auto cs_title = _T("dbWave\0") + GetDocument()->m_p_data_doc->GetTitle();
			cs_title += _T("\0\0");
			const auto hm_dc = m_dc.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
			ASSERT(hm_dc != NULL);

			// Draw document in meta file.
			CPen black_pen(PS_SOLID, 0, col_black);
			const auto old_pen = m_dc.SelectObject(&black_pen);
			const auto p_old_brush = static_cast<CBrush*>(m_dc.SelectStockObject(BLACK_BRUSH));
			CClientDC attribute_dc(this); 
			m_dc.SetAttribDC(attribute_dc.GetSafeHdc()); // from current screen

			// print comments : set font
			p_old_font_ = nullptr;
			const auto old_font_size = options_view_data_->font_size;
			options_view_data_->font_size = 10;
			print_create_font();
			m_dc.SetBkMode(TRANSPARENT);
			options_view_data_->font_size = old_font_size;
			p_old_font_ = m_dc.SelectObject(&font_print_);
			const int line_height = log_font_.lfHeight + 5;
			auto row = 0;
			constexpr auto column = 10;

			// comment and descriptors
			auto comments = GetDocument()->export_database_data(1);
			m_dc.TextOut(column, row, comments);
			row += line_height;

			// abscissa
			comments = _T("Abscissa: ");
			CString content;
			GetDlgItem(IDC_TIMEFIRST)->GetWindowText(content);
			comments += content;
			comments += _T(" - ");
			GetDlgItem(IDC_TIMELAST)->GetWindowText(content);
			comments += content;
			m_dc.TextOut(column, row, comments);

			// define display sizes - data_view & data_detect are same, spk_shape & spk_bar = as on screen
			auto data_rect = rect;
			data_rect.top -= -3 * line_height;
			const auto rect_spike_width = MulDiv(chart_spike_shape_.get_rect_width(), data_rect.Width(),
				chart_spike_shape_.get_rect_width() + chart_data_filtered_.get_rect_width());
			const auto rect_data_height = MulDiv(chart_data_filtered_.get_rect_height(), data_rect.Height(),
				chart_data_filtered_.get_rect_height() * 2 + chart_spike_bar_.
				get_rect_height());
			const auto separator = rect_spike_width / 10;

			// display curves : data
			data_rect.bottom = rect.top + rect_data_height - separator / 2;
			data_rect.left = rect.left + rect_spike_width + separator;
			print_data_cartridge(&m_dc, &chart_data_, &data_rect);

			// display curves: detect channel
			data_rect.top = data_rect.bottom + separator;
			data_rect.bottom = data_rect.top + rect_data_height;
			print_data_cartridge(&m_dc, &chart_data_filtered_, &data_rect);

			// display spike bars
			auto rect_bars = data_rect;
			rect_bars.top = data_rect.bottom + separator;
			rect_bars.bottom = rect.bottom - 2 * line_height;
			chart_spike_bar_.print(&m_dc, &rect_bars);

			// display spike shapes
			auto rect_spikes = rect; // compute output rectangle
			rect_spikes.left += separator;
			rect_spikes.right = rect.left + rect_spike_width;
			rect_spikes.bottom = rect.bottom - 2 * line_height;
			rect_spikes.top = rect_spikes.bottom - rect_bars.Height();
			chart_spike_shape_.print(&m_dc, &rect_spikes);
			comments = print_spk_shape_bars(&m_dc, &rect_spikes, TRUE);

			auto rect_comment = rect;
			rect_comment.right = data_rect.left;
			rect_comment.top = rect_spikes.bottom;
			constexpr UINT n_format = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
			m_dc.DrawText(comments, comments.GetLength(), rect_comment, n_format);
			m_dc.SelectObject(p_old_brush);

			if (p_old_font_ != nullptr)
				m_dc.SelectObject(p_old_font_);
			font_print_.DeleteObject();

			// restore old pen
			m_dc.SelectObject(old_pen);
			ReleaseDC(p_dc_ref);

			// Close metafile
			const auto h_emf_tmp = m_dc.CloseEnhanced();
			ASSERT(h_emf_tmp != NULL);
			if (OpenClipboard())
			{
				EmptyClipboard(); 
				SetClipboardData(CF_ENHMETAFILE, h_emf_tmp); 
				CloseClipboard(); 
			}
			else
			{
				// someone has the Clipboard open...
				DeleteEnhMetaFile(h_emf_tmp); 
				MessageBeep(0); 
				AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
			}
			// restore initial conditions
			serialize_windows_state(b_restore);
		}
	}
}

void ViewSpikeDetection::on_sel_change_detect_mode()
{
	UpdateData(TRUE);
	m_p_detect_parameters_->detect_what = m_detect_what_combo.GetCurSel();
	update_combo_box();
	update_legend_detection_wnd();
	chart_data_filtered_.get_data_from_doc();
	chart_data_filtered_.auto_zoom_chan(0);
	chart_data_filtered_.Invalidate();
}

void ViewSpikeDetection::update_combo_box()
{
	m_detect_channel_combo.SetCurSel(m_p_detect_parameters_->detect_channel);
	m_transform_combo.SetCurSel(m_p_detect_parameters_->detect_transform);
	chart_data_filtered_.set_channel_list_y(0, m_p_detect_parameters_->detect_channel, m_p_detect_parameters_->detect_transform);
	m_p_detect_parameters_->detect_threshold_bin = chart_data_filtered_.get_channel_list_item(0)->convert_volts_to_data_bins(
		m_threshold_val / 1000.f);
	chart_data_filtered_.hz_tags.set_channel(0, 0);
	chart_data_filtered_.hz_tags.set_value_int(0, m_p_detect_parameters_->detect_threshold_bin);
	m_p_detect_parameters_->detect_threshold_mv = m_threshold_val;
}

void ViewSpikeDetection::update_legend_detection_wnd()
{
	CString text;
	GetDlgItem(IDC_TRANSFORM)->GetWindowTextW(text);
	CString text0;
	m_bevel2.GetWindowTextW(text0);
	if (text0 != text)
	{
		m_bevel2.SetWindowTextW(text);
		m_bevel2.Invalidate();
	}
}

void ViewSpikeDetection::update_vt_tags()
{
	chart_spike_bar_.vt_tags.remove_all_tags();
	chart_data_filtered_.vt_tags.remove_all_tags();
	chart_data_.vt_tags.remove_all_tags();
	if (p_spk_doc->m_stimulus_intervals.n_items == 0)
		return;

	for (auto i = 0; i < p_spk_doc->m_stimulus_intervals.get_size(); i++)
	{
		const int cx = p_spk_doc->m_stimulus_intervals.get_at(i);
		chart_spike_bar_.vt_tags.add_l_tag(cx, 0);
		chart_data_filtered_.vt_tags.add_l_tag(cx, 0);
		chart_data_.vt_tags.add_l_tag(cx, 0);
	}
}

void ViewSpikeDetection::print_compute_page_size()
{
	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlg(FALSE);
	VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	const auto h_dc = dlg.CreatePrinterDC();
	ASSERT(h_dc != NULL);
	dc.Attach(h_dc);

	// Get the size of the page in pixels
	options_view_data_->horizontal_resolution = dc.GetDeviceCaps(HORZRES);
	options_view_data_->vertical_resolution = dc.GetDeviceCaps(VERTRES);

	// margins (pixels)
	print_rect_.right = options_view_data_->horizontal_resolution - options_view_data_->right_page_margin;
	print_rect_.bottom = options_view_data_->vertical_resolution - options_view_data_->bottom_page_margin;
	print_rect_.left = options_view_data_->left_page_margin;
	print_rect_.top = options_view_data_->top_page_margin;
}

void ViewSpikeDetection::print_file_bottom_page(CDC* p_dc, const CPrintInfo* p_info)
{
	const auto t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"),
		p_info->m_nCurPage, p_info->GetMaxPage(),
		t.GetDay(), t.GetMonth(), t.GetYear());
	const auto ch_date = GetDocument()->db_get_current_spk_file_name();
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_view_data_->horizontal_resolution / 2, options_view_data_->vertical_resolution - 57, ch_date);
}

CString ViewSpikeDetection::print_convert_file_index(const long l_first, const long l_last) const
{
	CString cs_unit = _T(" s");
	CString cs_comment;
	float x_scale_factor;
	const auto x1 = NiceUnit::change_unit(static_cast<float>(l_first) /									m_sampling_rate_, &cs_unit, &x_scale_factor);
	double d;
	const auto fraction1 = modf(x1, &d) * 1000.f;
		//static_cast<int>((x1 - static_cast<int>(x1)) * static_cast<float>(1000.));
	// separate fractional part
	const auto x2 = static_cast<float>(l_last) / (m_sampling_rate_ * x_scale_factor);
	const auto fraction2 = modf(x2, &d) * 1000.;
		//static_cast<int>((x2 - static_cast<int>(x2)) * static_cast<float>(1000.));
	cs_comment.Format(_T("time = %i.%03.3i - %i.%03.3i"), 
		static_cast<int>(x1), static_cast<int>(fraction1), 
		static_cast<int>(x2), static_cast<int>(fraction2));
	cs_comment += cs_unit;
	return cs_comment;
}

BOOL ViewSpikeDetection::print_get_file_series_index_from_page(const int page, int& file_number, long& l_first)
{
	// loop until we get all rows
	const auto total_rows = n_rows_per_page_ * (page - 1);
	l_first = l_print_first_;
	file_number = 0; 
	if (options_view_data_->b_print_selection) 
		file_number = file_0_;
	else
		BOOL success = GetDocument()->db_move_first();

	auto very_last = l_print_first_ + l_print_len_;
	if (options_view_data_->b_complete_record)
		very_last = GetDocument()->db_get_data_len() - 1;

	for (auto row = 0; row < total_rows; row++)
	{
		if (!print_get_next_row(file_number, l_first, very_last))
			break;
	}

	return TRUE;
}

BOOL ViewSpikeDetection::print_get_next_row(int& file_index, long& l_first, long& very_last)
{
	if (!options_view_data_->b_multiple_rows || !options_view_data_->b_complete_record)
	{
		file_index++;
		if (file_index >= files_count_)
			return FALSE;

		BOOL success = GetDocument()->db_move_next();
		if (l_first < GetDocument()->db_get_data_len() - 1)
		{
			if (options_view_data_->b_complete_record)
				very_last = GetDocument()->db_get_data_len() - 1;
		}
	}
	else
	{
		l_first += l_print_len_;
		if (l_first >= very_last)
		{
			file_index++; // next index
			if (file_index >= files_count_) // last file ??
				return FALSE;

			BOOL success = GetDocument()->db_move_next();
			very_last = GetDocument()->db_get_data_len() - 1;
			l_first = l_print_first_;
		}
	}
	return TRUE;
}

CString ViewSpikeDetection::print_get_file_infos()
{
	CString str_comment; 
	const CString tab(_T("    ")); 
	const CString rc(_T("\n")); 

	// document's name, date and time
	const auto p_data_file = GetDocument()->m_p_data_doc;
	const auto wave_format = p_data_file->get_wave_format();
	if (options_view_data_->b_doc_name || options_view_data_->b_acq_date_time)
	{
		if (options_view_data_->b_doc_name) 
			str_comment += GetDocument()->db_get_current_dat_file_name() + tab;

		if (options_view_data_->b_acq_date_time) 
		{
			const auto date = (wave_format->acquisition_time).Format(_T("%#d %B %Y %X")); 
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_view_data_->b_acq_comment)
		str_comment += GetDocument()->export_database_data(); 

	return str_comment;
}

CString ViewSpikeDetection::print_data_bars(CDC* p_dc, const ChartData* p_data_chart_wnd, const CRect* p_rect)
{
	CString cs;
	const CString rc(_T("\r"));
	const CString tab(_T("     "));
	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
	CString cs_unit; 
	const CPoint bar_origin(-10, -10); // bar origin at 10,10 pts on the left lower corner of the rectangle
	auto i_horizontal_bar = p_data_chart_wnd->get_rect_width() / 10; // initial horizontal bar length 1/10th of display rect
	auto i_vertical_bar = p_data_chart_wnd->get_rect_height() / 3; // initial vertical bar height 1/3rd  of display rect

	auto str_comment = print_convert_file_index(p_data_chart_wnd->get_data_first_index(), p_data_chart_wnd->get_data_last_index());

	///// horizontal time bar ///////////////////////////
	if (options_view_data_->b_time_scale_bar)
	{
		// convert bar size into time units and back into pixels
		cs_unit = _T(" s"); 
		const auto time_per_pixel = p_data_chart_wnd->get_time_per_pixel();
		const auto z = time_per_pixel * static_cast<float>(i_horizontal_bar); // convert 1/10 of the length of the data displayed into time
		float x_scale_factor;
		const auto x = NiceUnit::change_unit(z, &cs_unit, &x_scale_factor); // convert time into a scaled time
		const auto k = NiceUnit::nice_unit(x); // convert the (scaled) time value into time expressed as an integral
		i_horizontal_bar = static_cast<int>((static_cast<float>(k) * x_scale_factor) / time_per_pixel);
		// compute how many pixels it makes
		// print out the scale and units
		cs.Format(_T("horz bar = %i %s"), k, (LPCTSTR)cs_unit);
		str_comment += cs + rc;
		// draw horizontal line
		i_horizontal_bar = MulDiv(i_horizontal_bar, p_rect->Width(), p_data_chart_wnd->get_rect_width());
		p_dc->MoveTo(p_rect->left + bar_origin.x, p_rect->bottom - bar_origin.y);
		p_dc->LineTo(p_rect->left + bar_origin.x + i_horizontal_bar, p_rect->bottom - bar_origin.y);
	}

	///// vertical voltage bars ///////////////////////////
	float y_scale_factor; // compute a good unit for channel 0
	cs_unit = _T(" V"); 

	// convert bar size into voltage units and back into pixels
	const auto volts_per_pixel = p_data_chart_wnd->get_channel_list_volts_per_pixel(0);
	const auto z_volts = volts_per_pixel * static_cast<float>(i_vertical_bar); // convert 1/3 of the height into voltage
	const auto z_scale = NiceUnit::change_unit(z_volts, &cs_unit, &y_scale_factor);
	const auto z_nice = static_cast<float>(NiceUnit::nice_unit(z_scale));
	i_vertical_bar = static_cast<int>(z_nice * y_scale_factor / volts_per_pixel); // compute how many pixels it makes

	if (options_view_data_->b_voltage_scale_bar)
	{
		i_vertical_bar = MulDiv(i_vertical_bar, p_rect->Height(), p_data_chart_wnd->get_rect_height());
		p_dc->MoveTo(p_rect->left + bar_origin.x, p_rect->bottom - bar_origin.y);
		p_dc->LineTo(p_rect->left + bar_origin.x, p_rect->bottom - bar_origin.y - i_vertical_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_view_data_->b_channel_comment || options_view_data_->b_voltage_scale_bar || options_view_data_->b_channel_settings)
	{
		const auto channel_list_size = p_data_chart_wnd->get_channel_list_size();
		for (auto channel_index = 0; channel_index < channel_list_size; channel_index++) // loop
		{
			CChanlistItem* channel_item = p_data_chart_wnd->get_channel_list_item(channel_index);
			if (!channel_item->is_print_visible())
				continue;

			cs.Format(_T("chan#%i "), channel_index); 
			str_comment += cs;
			if (options_view_data_->b_voltage_scale_bar) 
			{
				const auto z = static_cast<float>(i_vertical_bar) * p_data_chart_wnd->get_channel_list_volts_per_pixel(channel_index);
				const auto x = z / y_scale_factor;
				const auto j = NiceUnit::nice_unit(x);
				cs.Format(_T("vert bar = %i %s "), j, (LPCTSTR)cs_unit); 
				str_comment += cs;
			}
			// print chan comment
			if (options_view_data_->b_channel_comment)
			{
				str_comment += tab;
				str_comment += channel_item->get_comment();
			}
			str_comment += rc;
			// print amplifiers settings (gain & filter), next line
			if (options_view_data_->b_channel_settings)
			{
				const auto source_channel = channel_item->get_source_chan();
				const auto wave_chan_array = GetDocument()->m_p_data_doc->get_wave_channels_array();
				const auto p_chan = wave_chan_array->get_p_channel(source_channel);
				cs.Format(_T("headstage=%s  g=%li LP=%i  IN+=%s  IN-=%s"),
					(LPCTSTR)p_chan->am_csheadstage, static_cast<long>(p_chan->am_gaintotal), p_chan->am_lowpass,
					(LPCTSTR)p_chan->am_csInputpos, (LPCTSTR)p_chan->am_csInputneg);
				str_comment += cs;
				str_comment += rc;
			}
		}
	}
	p_dc->SelectObject(p_old_brush);
	return str_comment;
}

CString ViewSpikeDetection::print_spk_shape_bars(CDC* p_dc, const CRect* p_rect, const BOOL b_all)
{
	const CString rc("\n");
	CString str_comment;
	float z;
	int k;

	///// vertical voltage bars ///////////////////////////
	if (options_view_data_->b_voltage_scale_bar && p_spk_list->get_spikes_count() > 0)
	{
		// the following assume that spikes are higher than 1 mV...
		const CString cs_unit = _T("mV");
		z = chart_spike_shape_.get_extent_mv() / 2.0f; 
		k = static_cast<int>(z); 
		if ((static_cast<double>(z) - k) > 0.5)
			k++;
		if (b_all)
		{
			CString dummy;
			if (k > 0)
				dummy.Format( _T("Vbar=%i %s"), k, static_cast<LPCTSTR>(cs_unit));
			else
				dummy.Format( _T("Vbar=%f.3 %s"), z, static_cast<LPCTSTR>(cs_unit));
			str_comment = dummy + rc;
		}

		// display bar
		const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
		if (k > 0)
			z = static_cast<float>(k) / z;
		const auto vertical_bar = static_cast<int>((static_cast<float>(p_rect->Height()) * z) / 2.f);

		// compute coordinates of the rect
		CRect rect_vertical_bar; 
		const auto bar_width = CSize(5, 5);
		rect_vertical_bar.left = p_rect->left - options_view_data_->text_separator;
		rect_vertical_bar.right = rect_vertical_bar.left + bar_width.cx;
		rect_vertical_bar.top = p_rect->top + (p_rect->Height() - vertical_bar) / 2;
		rect_vertical_bar.bottom = rect_vertical_bar.top + vertical_bar;
		p_dc->Rectangle(&rect_vertical_bar);
		p_dc->SelectObject(p_old_brush);
	}

	// spike duration
	if (p_spk_list->get_spikes_count() > 0 && b_all)
	{
		z = chart_spike_shape_.get_extent_ms();
		const CString cs_unit = _T(" ms");
		k = static_cast<int>(z);
		CString dummy3;
		dummy3.Format(_T("Horz=%i."), k);
		str_comment += dummy3;

		k = static_cast<int>(1000.0f * (z - static_cast<float>(k)));
		CString dummy4;
		dummy4.Format( _T("%i %s"), k, static_cast<LPCTSTR>(cs_unit));
		str_comment += dummy4;
		str_comment += rc;
	}

	// number of spikes
	k = p_spk_list->get_spikes_count();
	CString dummy5;
	dummy5.Format( _T("n spk= %i"), k);
	str_comment += dummy5;
	str_comment += rc;

	return str_comment;
}

void ViewSpikeDetection::serialize_windows_state(const BOOL save, int tab_index)
{
	const auto p_dbWave_app = static_cast<CdbWaveApp*>(AfxGetApp()); 
	if (tab_index < 0 || tab_index >= spk_list_tab_ctrl.GetItemCount())
	{
		int tab_selected = spk_list_tab_ctrl.GetCurSel(); 
		if (tab_selected < 0)
			tab_selected = 0;
		tab_index = tab_selected;
	}

	// adjust size of the array
	if (p_dbWave_app->view_spikes_memory_file_ptr_array.GetSize() == 0)
		p_dbWave_app->view_spikes_memory_file_ptr_array.SetSize(1);

	if (p_dbWave_app->view_spikes_memory_file_ptr_array.GetSize() < spk_list_tab_ctrl.GetItemCount())
		p_dbWave_app->view_spikes_memory_file_ptr_array.SetSize(spk_list_tab_ctrl.GetItemCount());
	CMemFile* p_mem_file = nullptr;
	if (p_dbWave_app->view_spikes_memory_file_ptr_array.GetSize() > 0 && p_dbWave_app->view_spikes_memory_file_ptr_array.GetSize() > tab_index)
		p_mem_file = p_dbWave_app->view_spikes_memory_file_ptr_array.GetAt(tab_index);

	// save display parameters
	if (save)
	{
		if (p_mem_file == nullptr)
		{
			p_mem_file = new CMemFile;
			ASSERT(p_mem_file != NULL);
			p_dbWave_app->view_spikes_memory_file_ptr_array.SetAt(tab_index, p_mem_file);
		}
		// save data into archive
		CArchive ar(p_mem_file, CArchive::store);
		p_mem_file->SeekToBegin();
		chart_data_.Serialize(ar);
		chart_data_filtered_.Serialize(ar);
		chart_spike_bar_.Serialize(ar);
		chart_spike_shape_.Serialize(ar);
		ar.Close();
	}

	// restore display parameters
	else
	{
		if (p_mem_file != nullptr)
		{
			CArchive ar(p_mem_file, CArchive::load);
			p_mem_file->SeekToBegin();
			chart_data_.Serialize(ar);
			chart_data_filtered_.Serialize(ar);
			chart_spike_bar_.Serialize(ar);
			chart_spike_shape_.Serialize(ar);
			ar.Close(); 
		}
		else
		{
			*chart_data_.get_scope_parameters() = options_view_data_->view_spk_detect_data;
			*chart_data_filtered_.get_scope_parameters() = options_view_data_->view_spk_detect_filtered;
			*chart_spike_bar_.get_scope_parameters() = options_view_data_->view_spk_detect_spk;
			*chart_spike_shape_.get_scope_parameters() = options_view_data_->view_spk_detect_bars;
		}
	}
}

BOOL ViewSpikeDetection::OnPreparePrinting(CPrintInfo* p_info)
{
	// save current state of the windows
	serialize_windows_state(b_save);

	// printing margins
	if (options_view_data_->vertical_resolution <= 0 || options_view_data_->horizontal_resolution <= 0
		|| options_view_data_->horizontal_resolution != p_info->m_rectDraw.Width()
		|| options_view_data_->vertical_resolution != p_info->m_rectDraw.Height())
		print_compute_page_size();

	// nb print pages?
	int n_pages = print_get_n_pages();
	p_info->SetMaxPage(n_pages); 
	p_info->m_nNumPreviewPages = 1; 
	p_info->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; 

	if (!DoPreparePrinting(p_info))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, p_info))
		return FALSE;

	if (options_view_data_->b_print_selection != p_info->m_pPD->PrintSelection())
	{
		options_view_data_->b_print_selection = p_info->m_pPD->PrintSelection();
		n_pages = print_get_n_pages();
		p_info->SetMaxPage(n_pages);
	}
	return TRUE;
}

int ViewSpikeDetection::print_get_n_pages()
{
	// how many rows per page?
	const auto size_row = options_view_data_->height_doc + options_view_data_->height_separator;
	n_rows_per_page_ = print_rect_.Height() / size_row;
	if (n_rows_per_page_ == 0)
		n_rows_per_page_ = 1;

	int n_total_rows;
	const auto p_document = GetDocument();

	// compute number of rows according to b_multi_row & b_entire_record flag
	l_print_first_ = chart_data_filtered_.get_data_first_index();
	l_print_len_ = chart_data_filtered_.get_data_last_index() - l_print_first_ + 1;
	file_0_ = GetDocument()->db_get_current_record_position();
	ASSERT(file_0_ >= 0);
	files_count_ = 1;
	auto i_file_0 = file_0_;
	auto i_file_1 = file_0_;
	if (!options_view_data_->b_print_selection)
	{
		i_file_0 = 0;
		files_count_ = p_document->db_get_records_count();
		i_file_1 = files_count_;
	}

	// only one row per file
	if (!options_view_data_->b_multiple_rows || !options_view_data_->b_complete_record)
		n_total_rows = files_count_;

	// multiple rows per file
	else
	{
		n_total_rows = 0;
		
		for (auto i = i_file_0; i < i_file_1; i++)
		{
			if (!p_document->db_set_current_record_position(i)) 
				continue;
			// get size of document for all files
			auto len = p_document->db_get_data_len();
			if (len <= 0)
			{
				p_document->open_current_data_file();
				len = p_document->m_p_data_doc->get_doc_channel_length();
				const auto len1 = GetDocument()->db_get_data_len() - 1;
				ASSERT(len == len1);
				p_document->db_set_data_len(len);
			}
			len -= l_print_first_;
			auto n_rows = len / l_print_len_; 
			if (len > n_rows * l_print_len_)
				n_rows++;
			n_total_rows += static_cast<int>(n_rows);
		}
	}

	if (file_0_ >= 0)
	{
		if (p_document->db_set_current_record_position(file_0_))
			p_document->open_current_data_file();
	}

	auto n_pages = n_total_rows / n_rows_per_page_;
	if (n_total_rows > n_rows_per_page_ * n_pages)
		n_pages++;

	return n_pages;
}

void ViewSpikeDetection::OnBeginPrinting(CDC* p_dc, CPrintInfo* p_info)
{
	is_printing_ = TRUE;
	l_first_0_ = chart_data_filtered_.get_data_first_index();
	l_last0_ = chart_data_filtered_.get_data_last_index();
	pixels_count_0_ = chart_data_filtered_.get_rect_width();
	print_create_font();
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewSpikeDetection::print_create_font()
{
	//---------------------init objects-------------------------------------
	memset(&log_font_, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(log_font_.lfFaceName, _T("Arial")); // Arial font
	log_font_.lfHeight = options_view_data_->font_size; // font height
	p_old_font_ = nullptr;
	font_print_.CreateFontIndirect(&log_font_);
}

void ViewSpikeDetection::OnPrint(CDC* p_dc, CPrintInfo* p_info)
{
	p_old_font_ = p_dc->SelectObject(&font_print_);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	const auto r_width = options_view_data_->width_doc; // page margins
	const auto r_height = options_view_data_->height_doc; // page margins
	CRect r_where(print_rect_.left, // printing rectangle for one line of data
		print_rect_.top,
		print_rect_.left + r_width,
		print_rect_.top + r_height);
	auto rw2 = r_where; // printing rectangle - constant
	rw2.OffsetRect(-r_where.left, -r_where.top); // set RW2 origin = 0,0

	// define spike shape area (rect): same height as data area
	auto r_spk_height = options_view_data_->spike_height;
	auto r_spk_width = options_view_data_->spike_width;
	if (r_spk_height == 0)
	{
		r_spk_height = r_height - options_view_data_->font_size * 4;
		r_spk_width = r_spk_height / 2;
		if (r_spk_width < MulDiv(r_where.Width(), 10, 100))
			r_spk_width = MulDiv(r_where.Width(), 10, 100);
		options_view_data_->spike_height = r_spk_height;
		options_view_data_->spike_width = r_spk_width;
	}

	// save current draw mode (it will be modified to print only one channel)

	if (!options_view_data_->b_filter_data_source)
		chart_data_filtered_.set_channel_list_transform_mode(0, 0);

	p_dc->SetMapMode(MM_TEXT); // change map mode to text (1 pixel = 1 logical point)
	print_file_bottom_page(p_dc, p_info); // print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int file_index; 
	long index_first_data_point; 
	auto index_last_data_point = l_print_first_ + l_print_len_; 
	const auto current_page_number = static_cast<int>(p_info->m_nCurPage);
	print_get_file_series_index_from_page(current_page_number, file_index, index_first_data_point);
	if (index_first_data_point < GetDocument()->db_get_data_len() - 1)
		update_file_parameters(FALSE);
	if (options_view_data_->b_complete_record)
		index_last_data_point = GetDocument()->db_get_data_len() - 1;

	// loop through all files	--------------------------------------------------------
	for (auto i = 0; i < n_rows_per_page_; i++)
	{
		constexpr WORD chan_0_draw_mode = 1;
		const auto old_dc = p_dc->SaveDC(); // save DC

		// first : set rectangle where data will be printed
		auto comment_rect = r_where; 
		p_dc->SetMapMode(MM_TEXT); 
		p_dc->SetTextAlign(TA_LEFT); 

		// load data and adjust display rectangle ----------------------------------------
		// set data rectangle to half height to the row height
		rect_data_ = r_where;
		if (options_view_data_->b_print_spk_bars)
			rect_data_.bottom = rect_data_.top + r_where.Height() / 2;
		rect_data_.left += (r_spk_width + options_view_data_->text_separator);
		const auto old_size = rect_data_.Width(); 

		// make sure enough data fit into this rectangle, otherwise clip rect
		auto l_last = index_first_data_point + l_print_len_; 
		if (l_last > index_last_data_point) 
			l_last = index_last_data_point;
		if ((l_last - index_first_data_point + 1) < l_print_len_) 
			rect_data_.right = (old_size * (l_last - index_first_data_point)) / l_print_len_ + rect_data_.left;
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// if option requested, clip output to rect
		if (options_view_data_->b_clip_rect) // clip curve display
			p_dc->IntersectClipRect(&rect_data_); // (eventually)

		// print detected channel only data
		chart_data_filtered_.get_channel_list_item(0)->set_flag_print_visible(chan_0_draw_mode);
		chart_data_filtered_.resize_channels(rect_data_.Width(), 0);
		chart_data_filtered_.get_data_from_doc(index_first_data_point, l_last);
		chart_data_filtered_.print(p_dc, &rect_data_);
		p_dc->SelectClipRgn(nullptr);

		// print spike bars 
		if (options_view_data_->b_print_spk_bars)
		{
			CRect bars_rect = r_where; 
			bars_rect.top = rect_data_.bottom;
			bars_rect.left = rect_data_.left;
			bars_rect.right = rect_data_.right;

			chart_spike_bar_.set_time_intervals(index_first_data_point, l_last);
			chart_spike_bar_.print(p_dc, &bars_rect);
		}

		// print spike shape within a square (same width as height) 
		rect_spike_ = r_where; 
		rect_spike_.right = rect_spike_.left + r_spk_width;
		rect_spike_.left += options_view_data_->text_separator;
		rect_spike_.bottom = rect_spike_.top + r_spk_height; 

		chart_spike_shape_.set_time_intervals(index_first_data_point, l_last);
		chart_spike_shape_.print(p_dc, &rect_spike_);

		// restore DC and print comments 
		p_dc->RestoreDC(old_dc); 
		p_dc->SetMapMode(MM_TEXT);
		p_dc->SelectClipRgn(nullptr); 
		p_dc->SetViewportOrg(0, 0);

		// print data Bars & get comments according to row within file
		CString cs_comment;
		const BOOL b_all = (index_first_data_point == l_print_first_);
		if (b_all) 
		{
			cs_comment += print_get_file_infos();
			cs_comment += print_data_bars(p_dc, &chart_data_filtered_, &rect_data_);
		}
		else
		{
			// other rows: time intervals only
			cs_comment = print_convert_file_index(chart_data_filtered_.get_data_first_index(),
				chart_data_filtered_.get_data_last_index());
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_view_data_->text_separator + comment_rect.Width(), 0);
		comment_rect.right = print_rect_.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		constexpr UINT format_parameters = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect, format_parameters);

		// print comments & bar / spike shape
		cs_comment.Empty();
		rect_spike_.right = rect_spike_.left + r_spk_height;
		cs_comment = print_spk_shape_bars(p_dc, &rect_spike_, b_all);
		rect_spike_.right = rect_spike_.left + r_spk_width;
		rect_spike_.left -= options_view_data_->text_separator;
		rect_spike_.top = rect_spike_.bottom;
		rect_spike_.bottom += log_font_.lfHeight * 3;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), rect_spike_, format_parameters);
		p_dc->SetTextAlign(ui_flag);
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// update file parameters for next row --------------------------------------------

		r_where.OffsetRect(0, r_height + options_view_data_->height_separator);
		const auto i_file = file_index;
		if (!print_get_next_row(file_index, index_first_data_point, index_last_data_point))
		{
			//i = m_n_rows_per_page;
			break;
		}
		if (i_file != file_index)
		{
			update_file_parameters(FALSE);
			chart_data_filtered_.get_channel_list_item(0)->set_flag_print_visible(0); // cancel printing channel zero
		}
	}

	// end of file loop : restore initial conditions
	chart_data_filtered_.get_channel_list_item(0)->set_flag_print_visible(1);
	if (!options_view_data_->b_filter_data_source)
		chart_data_filtered_.set_channel_list_transform_mode(0, m_p_detect_parameters_->detect_transform);

	if (p_old_font_ != nullptr)
		p_dc->SelectObject(p_old_font_);
}

void ViewSpikeDetection::OnEndPrinting(CDC* p_dc, CPrintInfo* p_info)
{
	font_print_.DeleteObject();
	// restore file from index and display parameters
	if (GetDocument()->db_set_current_record_position(file_0_))
	{
		chart_data_filtered_.resize_channels(pixels_count_0_, 0);
		chart_data_filtered_.get_data_from_doc(l_first_0_, l_last0_);
		chart_spike_shape_.set_time_intervals(l_first_0_, l_last0_);
	}
	update_file_parameters(TRUE);

	is_printing_ = FALSE;
	serialize_windows_state(b_restore);
}

void ViewSpikeDetection::on_bn_clicked_bias_button()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN))->SetState(0);
	set_v_bar_mode(BAR_BIAS, IDC_SCROLLY);
}

void ViewSpikeDetection::on_bn_clicked_bias2()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS2))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN2))->SetState(0);
	set_v_bar_mode(BAR_BIAS, IDC_SCROLLY2);
}

void ViewSpikeDetection::on_bn_clicked_gain_button()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN))->SetState(1);
	set_v_bar_mode(BAR_GAIN, IDC_SCROLLY);
}

void ViewSpikeDetection::on_bn_clicked_gain2()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS2))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN2))->SetState(1);
	set_v_bar_mode(BAR_GAIN, IDC_SCROLLY2);
}

void ViewSpikeDetection::OnVScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar* p_scroll_bar)
{
	if (p_scroll_bar != nullptr)
	{
		const auto i_id = p_scroll_bar->GetDlgCtrlID();
		if ((i_id == IDC_SCROLLY) || (i_id == IDC_SCROLLY2))
		{
			// CViewData scroll: vertical scroll bar
			switch (m_v_bar_mode_)
			{
			case BAR_GAIN:
				on_gain_scroll(n_sb_code, n_pos, i_id);
				break;
			case BAR_BIAS:
				on_bias_scroll(n_sb_code, n_pos, i_id);
			default:
				break;
			}
		}
	}
	else
		ViewDbTable::OnVScroll(n_sb_code, n_pos, p_scroll_bar);
}

void ViewSpikeDetection::set_v_bar_mode(const int b_mode, const int i_id)
{
	if (b_mode == BAR_BIAS)
		m_v_bar_mode_ = b_mode;
	else
		m_v_bar_mode_ = BAR_GAIN;
	update_bias_scroll(i_id);
}

void ViewSpikeDetection::update_gain_scroll(const int i_id)
{
	if (i_id == IDC_SCROLLY)
		m_scroll_y_.SetScrollPos(
			MulDiv(chart_data_filtered_.get_channel_list_item(m_selected_channel)->get_y_extent(), 100, Y_EXTENT_MAX) + 50, TRUE);
	else
		m_scroll_y2_.SetScrollPos(
			MulDiv(chart_data_filtered_.get_channel_list_item(m_selected_channel2)->get_y_extent(), 100, Y_EXTENT_MAX) + 50, TRUE);
}

void ViewSpikeDetection::on_gain_scroll(const UINT n_sb_code, const UINT n_pos, const int i_id)
{
	const ChartData* p_view_data_filtered = &chart_data_filtered_;
	int selected_channel = m_selected_channel;
	if (i_id == IDC_SCROLLY2)
	{
		p_view_data_filtered = &chart_data_;
		selected_channel = m_selected_channel2;
	}
	int y_extent = p_view_data_filtered->get_channel_list_item(selected_channel)->get_y_extent();

	// get corresponding data
	switch (n_sb_code)
	{
	case SB_LEFT: y_extent = Y_EXTENT_MIN;
		break; 
	case SB_LINELEFT: y_extent -= y_extent / 10 + 1;
		break; 
	case SB_LINERIGHT: y_extent += y_extent / 10 + 1;
		break; 
	case SB_PAGELEFT: y_extent -= y_extent / 2 + 1;
		break; 
	case SB_PAGERIGHT: y_extent += y_extent + 1;
		break; 
	case SB_RIGHT: y_extent = Y_EXTENT_MAX;
		break; 
	case SB_THUMBPOSITION: 
	case SB_THUMBTRACK: y_extent = MulDiv(static_cast<int>(n_pos) - 50, Y_EXTENT_MAX, 100);
		break;
	default: 
		break;
	}
	// change y extent
	if (y_extent > 0) 
	{
		p_view_data_filtered->get_channel_list_item(selected_channel)->set_y_extent(y_extent);
		update_legends();
	}
	// update scrollBar
	if (m_v_bar_mode_ == BAR_GAIN)
		update_gain_scroll(i_id);
}

void ViewSpikeDetection::update_bias_scroll(const int i_id)
{
	if (i_id == IDC_SCROLLY)
	{
		const CChanlistItem* channel_item = chart_data_filtered_.get_channel_list_item(m_selected_channel);
		const auto i_pos = static_cast<int>((channel_item->get_y_zero() - channel_item->get_data_bin_zero())
			* 100 / static_cast<int>(Y_ZERO_SPAN)) + static_cast<int>(50);
		m_scroll_y_.SetScrollPos(i_pos, TRUE);
	}
	else
	{
		const CChanlistItem* channel_item = chart_data_filtered_.get_channel_list_item(m_selected_channel2);
		const auto i_pos = static_cast<int>((channel_item->get_y_zero() - channel_item->get_data_bin_zero())
			* 100 / static_cast<int>(Y_ZERO_SPAN)) + static_cast<int>(50);
		m_scroll_y2_.SetScrollPos(i_pos, TRUE);
	}
}

void ViewSpikeDetection::on_bias_scroll(const UINT n_sb_code, const UINT n_pos, const int i_id)
{
	auto p_view = &chart_data_filtered_;
	auto selected_channel_index = m_selected_channel;
	if (i_id == IDC_SCROLLY2)
	{
		p_view = &chart_data_;
		selected_channel_index = m_selected_channel2;
	}

	const CChanlistItem* channel_item = chart_data_filtered_.get_channel_list_item(selected_channel_index);
	auto l_size = channel_item->get_y_zero() - channel_item->get_data_bin_zero();
	const auto y_extent = channel_item->get_y_extent();
	// get corresponding data
	switch (n_sb_code)
	{
	case SB_LEFT: l_size = Y_ZERO_MIN;
		break;
	case SB_LINELEFT: l_size -= y_extent / 100 + 1;
		break;
	case SB_LINERIGHT: l_size += y_extent / 100 + 1;
		break; 
	case SB_PAGELEFT: l_size -= y_extent / 10 + 1;
		break;
	case SB_PAGERIGHT: l_size += y_extent / 10 + 1;
		break; 
	case SB_RIGHT: l_size = Y_ZERO_MAX;
		break;
	case SB_THUMBPOSITION: 
	case SB_THUMBTRACK: l_size = (static_cast<int>(n_pos) - 50) * (Y_ZERO_SPAN / 100);
		break;
	default: 
		break; 
	}

	// try to read data with this new size
	if (l_size > Y_ZERO_MIN && l_size < Y_ZERO_MAX)
	{
		CChanlistItem* chan = p_view->get_channel_list_item(selected_channel_index);
		chan->set_y_zero(l_size + chan->get_data_bin_zero());
		p_view->Invalidate();
	}
	// update scrollBar
	if (m_v_bar_mode_ == BAR_BIAS)
		update_bias_scroll(i_id);
}

void ViewSpikeDetection::on_en_change_spk_wnd_amplitude()
{
	if (mm_spk_wnd_amplitude_.m_b_entry_done)
	{
		m_spk_wnd_amplitude_ = chart_spike_shape_.get_extent_mv();
		const auto y_old = m_spk_wnd_amplitude_;
		CString cs;
		mm_spk_wnd_amplitude_.on_en_change(this, m_spk_wnd_amplitude_, 1.f, -1.f);

		// compute new extent and change the display
		if (m_spk_wnd_amplitude_ <= 0)
		{
			m_spk_wnd_amplitude_ = y_old;
			MessageBeep(-1);
		}
		const auto y_we = static_cast<int>(static_cast<float>(chart_spike_shape_.get_yw_extent()) * m_spk_wnd_amplitude_ / y_old);
		chart_spike_shape_.set_yw_ext_org(y_we, chart_spike_shape_.get_yw_org());
		chart_spike_shape_.set_y_scale_unit_value(m_spk_wnd_amplitude_);
		chart_spike_shape_.Invalidate();

		// update the dialog control
		
		cs.Format(_T("%.3f"), m_spk_wnd_amplitude_);
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->SetWindowText(cs);
	}
}

void ViewSpikeDetection::on_en_change_spk_wnd_length()
{
	if (mm_spk_wnd_duration_.m_b_entry_done)
	{
		m_spk_wnd_duration_ = chart_spike_shape_.get_extent_ms();
		const auto x_old = m_spk_wnd_duration_;
		CString cs;
		mm_spk_wnd_duration_.on_en_change(this, m_spk_wnd_duration_, 1.f, -1.f);

		// compute new extent and change the display
		if (m_spk_wnd_duration_ <= 0)
		{
			MessageBeep(-1);
			m_spk_wnd_duration_ = x_old;
		}
		const auto x_we = static_cast<int>(static_cast<float>(chart_spike_shape_.get_xw_extent()) * m_spk_wnd_duration_ / x_old);
		chart_spike_shape_.set_xw_ext_org(x_we, chart_spike_shape_.get_xw_org());
		chart_spike_shape_.set_x_scale_unit_value(m_spk_wnd_duration_);
		chart_spike_shape_.Invalidate();

		// update the dialog control
		cs.Format(_T("%.3f"), m_spk_wnd_duration_);
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->SetWindowText(cs);
	}
}

void ViewSpikeDetection::on_bn_clicked_locate_button()
{
	int max, min;
	const CChanlistItem* channel_item = chart_data_filtered_.get_channel_list_item(0);
	channel_item->get_max_min(&max, &min);

	// modify value
	m_p_detect_parameters_->detect_threshold_bin = (max + min) / 2;
	m_threshold_val = channel_item->convert_data_bins_to_volts(m_p_detect_parameters_->detect_threshold_bin) * 1000.f;
	m_p_detect_parameters_->detect_threshold_mv = m_threshold_val;
	// update user-interface: edit control and threshold bar in source_view
	CString cs;
	cs.Format(_T("%.3f"), m_threshold_val);
	GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
	chart_data_filtered_.move_hz_tag_to_val(0, m_p_detect_parameters_->detect_threshold_bin);
	chart_data_filtered_.Invalidate();
}

void ViewSpikeDetection::update_detection_settings(const int i_sel_parameters)
{
	// check size of spike detection parameters
	if (i_sel_parameters >= spk_detect_array_.get_size())
	{
		// load new set of parameters from spike list
		const auto spike_list_size = p_spk_doc->get_spike_list_size();
		spk_detect_array_.set_size(spike_list_size);
		for (int i = 0; i < spike_list_size; i++)
		{
			// select new spike list (list with no spikes for stimulus channel)
			const auto spike_list_current = p_spk_doc->set_index_current_spike_list(i);
			ASSERT(spike_list_current != NULL);
			const auto ps_d = spike_list_current->get_detection_parameters();
			spk_detect_array_.set_item(i, ps_d); 
		}
	}

	// loop over each spike detection set to update spike detection parameters
	for (auto i = 0; i < spk_detect_array_.get_size(); i++)
	{
		// select new spike list (list with no spikes for stimulus channel)
		auto spike_list_current = p_spk_doc->set_index_current_spike_list(i);
		const auto p_sd = spk_detect_array_.get_item(i);
		if (spike_list_current == nullptr)
		{
			p_spk_doc->add_spk_list();
			spike_list_current = p_spk_doc->get_spike_list_current();
			spike_list_current->init_spike_list(GetDocument()->m_p_data_doc, p_sd);
		}
		else
			spike_list_current->set_detection_parameters(p_sd);
	}

	// set new parameters
	p_spk_list->m_selected_spike = m_spike_index; 
	m_i_detect_parameters_ = i_sel_parameters;
	m_p_detect_parameters_ = spk_detect_array_.get_item(i_sel_parameters);
	p_spk_list = p_spk_doc->set_index_current_spike_list(i_sel_parameters);
	if (p_spk_list != nullptr)
		highlight_spikes_in_chart_data(TRUE);

	update_detection_controls();
	update_legends();
}

void ViewSpikeDetection::update_detection_controls()
{
	const options_detect_spikes* detect_parameters = p_spk_list->get_detection_parameters();
	m_detect_what_combo.SetCurSel(detect_parameters->detect_what);
	m_detect_channel_combo.SetCurSel(detect_parameters->detect_channel);
	m_transform_combo.SetCurSel(detect_parameters->detect_transform);

	// check that spike detection parameters are compatible with current data doc
	auto detection_channel = detect_parameters->detect_channel;
	const auto p_dat = GetDocument()->m_p_data_doc;
	const int scan_count = p_dat->get_wave_format()->scan_count;
	if (detection_channel >= scan_count)
		detection_channel = 0;

	chart_data_filtered_.set_channel_list_y(0, detection_channel, detect_parameters->detect_transform);
	m_transform2_combo.SetCurSel(p_spk_list->get_detection_parameters()->extract_transform);

	CChanlistItem* channel_item = chart_data_filtered_.get_channel_list_item(0);
	channel_item->set_color(static_cast<WORD>(detection_channel));

	chart_data_filtered_.get_data_from_doc(); 

	const auto detect_threshold = detect_parameters->detect_threshold_bin;
	m_threshold_val = channel_item->convert_data_bins_to_volts(detect_threshold) * 1000.f;
	if (chart_data_filtered_.hz_tags.get_tag_list_size() < 1)
		chart_data_filtered_.hz_tags.add_tag(detect_threshold, 0);
	else
		chart_data_filtered_.hz_tags.set_value_int(0, detect_threshold);

	// update spike channel displayed
	chart_spike_bar_.set_spike_list(p_spk_list);
	chart_spike_shape_.set_spike_list(p_spk_list);
}

void ViewSpikeDetection::on_sel_change_tab(NMHDR* p_nmhdr, LRESULT* p_result)
{
	serialize_windows_state(b_save, m_i_detect_parameters_);
	const auto selected_tab = spk_list_tab_ctrl.GetCurSel();
	serialize_windows_state(b_restore, selected_tab);
	update_detection_settings(selected_tab);
	*p_result = 0;
}

void ViewSpikeDetection::on_tools_edit_stimulus()
{
	p_spk_doc->sort_stimulus_array();

	DlgEditStimArray dlg;
	dlg.intervals = p_spk_doc->m_stimulus_intervals;
	dlg.m_sampling_rate = m_sampling_rate_;
	dlg.intervals_saved = GetDocument()->stimulus_saved;
	
	if (IDOK == dlg.DoModal())
	{
		p_spk_doc->m_stimulus_intervals = dlg.intervals;
		update_vt_tags();
		chart_spike_bar_.Invalidate();
		chart_data_filtered_.Invalidate();
		chart_data_.Invalidate();
		p_spk_doc->SetModifiedFlag(TRUE);
	}
}
void ViewSpikeDetection::on_en_change_chan_selected()
{
	if (mm_selected_channel_.m_b_entry_done)
	{
		mm_selected_channel_.on_en_change(this, m_selected_channel, 1, -1);
		SetDlgItemInt(IDC_CHANSELECTED, m_selected_channel);
	}
}

void ViewSpikeDetection::on_en_change_chan_selected_2()
{
	if (mm_selected_channel2_.m_b_entry_done)
	{
		mm_selected_channel2_.on_en_change(this, m_selected_channel2, 1, -1);
		SetDlgItemInt(IDC_CHANSELECTED2, m_selected_channel2);
	}
}

void ViewSpikeDetection::on_cbn_sel_change_transform_2()
{
	const auto method = m_transform2_combo.GetCurSel(); 
	const auto data_document = GetDocument()->m_p_data_doc;

	const auto detect_parameters = p_spk_list->get_detection_parameters();
	const auto doc_chan = detect_parameters->extract_channel;
	const auto span = AcqDataDoc::get_transformed_data_span(method); 
	detect_parameters->extract_transform = method; 

	// pre-load data
	const auto spike_length = p_spk_list->get_spike_length();
	const auto spike_pre_threshold = detect_parameters->detect_pre_threshold;
	auto ii_time = p_spk_list->get_spike(0)->get_time() - spike_pre_threshold;
	auto l_rw_first0 = ii_time - spike_length;
	auto l_rw_last0 = ii_time + spike_length;
	if (!data_document->load_raw_data(&l_rw_first0, &l_rw_last0, span))
		return;

	// loop over all spikes now
	const auto total_spikes = p_spk_list->get_spikes_count();
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		constexpr auto offset = 1;
		Spike* p_spike = p_spk_list->get_spike(i_spike);
		// make sure that source data are loaded and get pointer to it (p_data)
		ii_time = p_spike->get_time();
		auto l_rw_first = ii_time - spike_pre_threshold; 
		auto l_rw_last = l_rw_first + spike_length; 
		if (!data_document->load_raw_data(&l_rw_first, &l_rw_last, span))
			break;

		const auto p_data = data_document->load_transformed_data(l_rw_first, l_rw_last, method, doc_chan);
		const auto p_data_spike0 = p_data + (ii_time - spike_pre_threshold - l_rw_first) * offset;
		p_spike->transfer_data_to_spike_buffer(p_data_spike0, offset, p_spk_list->get_spike_length());

		// n channels should be 1 if they come from the transform buffer as data are not interleaved...
		p_spike->center_spike_amplitude(0, spike_length, 1); // 1=center average
	}
	p_spk_doc->SetModifiedFlag(TRUE);

	int max, min;
	p_spk_list->get_total_max_min(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	chart_spike_shape_.set_yw_ext_org(chart_spike_shape_.get_yw_extent(), middle);
	chart_spike_bar_.set_yw_ext_org(chart_spike_shape_.get_yw_extent(), middle);
	update_spike_shape_window_scale(FALSE);

	highlight_spikes_in_chart_data(TRUE);
	update_legends();
	update_tabs();
}

void ViewSpikeDetection::update_tabs()
{
	// load initial data
	const BOOL b_replace = (spk_list_tab_ctrl.GetItemCount() == p_spk_doc->get_spike_list_size());
	if (!b_replace)
		spk_list_tab_ctrl.DeleteAllItems();

	// load list of detection parameters
	const auto current_spike_list_index = p_spk_doc->get_index_current_spike_list();
	for (auto i = 0; i < p_spk_doc->get_spike_list_size(); i++)
	{
		CString cs;
		const auto current_spike_list = p_spk_doc->set_index_current_spike_list(i);
		cs.Format(_T("#%i %s"), i, (LPCTSTR)current_spike_list->get_detection_parameters()->comment);
		if (!b_replace)
			spk_list_tab_ctrl.InsertItem(i, cs);
		else
		{
			TCITEM tab_ctrl_item;
			tab_ctrl_item.mask = TCIF_TEXT;
			const auto p_cs_buffer = cs.GetBuffer(0);
			tab_ctrl_item.pszText = p_cs_buffer;
			spk_list_tab_ctrl.SetItem(i, &tab_ctrl_item);
			cs.ReleaseBuffer();
		}
	}
	p_spk_doc->set_index_current_spike_list(current_spike_list_index);

	m_i_detect_parameters_ = p_spk_doc->get_index_current_spike_list();
	spk_list_tab_ctrl.SetCurSel(m_i_detect_parameters_);
}
