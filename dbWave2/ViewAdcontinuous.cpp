#include "StdAfx.h"
#include <OLERRORS.H>
#include <Olxdaapi.h>

#include "resource.h"
#include "dbTableMain.h"
#include "dbWaveDoc.h"
#include "DlgADIntervals.h"
#include "DlgADExperiment.h"
#include "dtacq32.h"

#include "ChartData.h"
#include "ViewADcontinuous.h"
#include "ChildFrm.h"
#include "dbWave.h"
#include "DlgADInputParms.h"
#include "DlgConfirmSave.h"
#include "DlgDAChannels.h"
#include "DlgDataTranslationBoard.h"
#include "MainFrm.h"
#include "StretchControls.h"
#include "USBPxxS1.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewADcontinuous, ViewDbTable)

ViewADcontinuous::ViewADcontinuous()
	: ViewDbTable(IDD)
{
	m_bEnableActiveAccessibility = FALSE;
	m_ad_y_ruler_bar.attach_scope_wnd(&m_chart_data_ad_, FALSE);
}

ViewADcontinuous::~ViewADcontinuous()
= default;

void ViewADcontinuous::DoDataExchange(CDataExchange * p_dx)
{
	CFormView::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_ANALOGTODIGIT, m_acq32_ad);
	DDX_Control(p_dx, IDC_DIGITTOANALOG, m_acq32_da);
	DDX_Control(p_dx, IDC_COMBOBOARD, m_combo_ad_card);
	DDX_Control(p_dx, IDC_STARTSTOP, m_btn_start_stop_ad);
	DDX_CBIndex(p_dx, IDC_COMBOSTARTOUTPUT, m_b_start_out_put_mode);
	DDX_Control(p_dx, IDC_STARTSTOP2, m_button_start_stop_da);
	DDX_Control(p_dx, IDC_SAMPLINGMODE, m_button_sampling_mode);
	DDX_Control(p_dx, IDC_DAPARAMETERS2, m_button_output_channels);
	DDX_Control(p_dx, IDC_WRITETODISK, m_button_write_to_disk);
	DDX_Control(p_dx, IDC_OSCILLOSCOPE, m_button_oscilloscope);
}

BEGIN_MESSAGE_MAP(ViewADcontinuous, CFormView)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_VSCROLL()

	ON_MESSAGE(WM_MYMESSAGE, &ViewADcontinuous::on_my_message)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &ViewADcontinuous::on_input_channels)
	ON_COMMAND(ID_HARDWARE_AD_INTERVALS, &ViewADcontinuous::on_sampling_mode)
	ON_COMMAND(ID_HARDWARE_DEFINE_EXPERIMENT, &ViewADcontinuous::on_hardware_define_experiment)

	ON_BN_CLICKED(IDC_SAMPLINGMODE, &ViewADcontinuous::on_sampling_mode)
	ON_BN_CLICKED(IDC_INPUTCHANNELS, &ViewADcontinuous::on_input_channels)
	ON_BN_CLICKED(IDC_GAIN_button, &ViewADcontinuous::on_bn_clicked_gain_button)
	ON_BN_CLICKED(IDC_BIAS_button, &ViewADcontinuous::on_bn_clicked_bias_button)
	ON_BN_CLICKED(IDC_DAPARAMETERS2, &ViewADcontinuous::on_bn_clicked_da_parameters2)
	ON_BN_CLICKED(IDC_STARTSTOP, &ViewADcontinuous::on_bn_clicked_start_stop)
	ON_BN_CLICKED(IDC_WRITETODISK, &ViewADcontinuous::on_bn_clicked_write_to_disk)
	ON_BN_CLICKED(IDC_OSCILLOSCOPE, &ViewADcontinuous::on_bn_clicked_oscilloscope)
	ON_BN_CLICKED(IDC_CARDFEATURES, &ViewADcontinuous::on_bn_clicked_card_features)
	ON_BN_CLICKED(IDC_STARTSTOP2, &ViewADcontinuous::on_bn_clicked_start_stop2)
	ON_BN_CLICKED(IDC_UNZOOM, &ViewADcontinuous::on_bn_clicked_un_zoom)

	ON_CBN_SELCHANGE(IDC_COMBOBOARD, &ViewADcontinuous::on_cbn_sel_change_combo_board)
	ON_CBN_SELCHANGE(IDC_COMBOSTARTOUTPUT, &ViewADcontinuous::on_cbn_sel_change_combo_start_output)
END_MESSAGE_MAP()

void ViewADcontinuous::OnDestroy()
{
	if (m_acq32_ad.IsInProgress())
		stop_acquisition();

	m_acq32_da.StopAndLiberateBuffers();

	if (m_b_found_dt_open_layer_dll_)
	{
		// TODO: save data here 
		if (m_acq32_ad.GetHDass() != NULL)
			m_acq32_ad.DeleteBuffers();

		if (m_acq32_da.GetHDass() != NULL)
			m_acq32_da.DeleteBuffers();
	}

	CFormView::OnDestroy();
	delete m_p_background_brush_;
}

HBRUSH ViewADcontinuous::OnCtlColor(CDC * p_dc, CWnd * p_wnd, UINT n_ctl_color)
{
	HBRUSH hbr;
	switch (n_ctl_color)
	{
	case CTLCOLOR_EDIT:
	case CTLCOLOR_MSGBOX:
		p_dc->SetBkColor(m_background_color_);
		hbr = static_cast<HBRUSH>(m_p_background_brush_->GetSafeHandle());
		break;

	default:
		hbr = CFormView::OnCtlColor(p_dc, p_wnd, n_ctl_color);
		break;
	}
	return hbr;
}

BEGIN_EVENTSINK_MAP(ViewADcontinuous, CFormView)

	ON_EVENT(ViewADcontinuous, IDC_ANALOGTODIGIT, 1, ViewADcontinuous::on_buffer_done_adc, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_ANALOGTODIGIT, 2, ViewADcontinuous::on_queue_done_adc, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_ANALOGTODIGIT, 4, ViewADcontinuous::on_trigger_error_adc, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_ANALOGTODIGIT, 5, ViewADcontinuous::on_overrun_error_adc, VTS_NONE)

	ON_EVENT(ViewADcontinuous, IDC_DIGITTOANALOG, 1, ViewADcontinuous::on_buffer_done_dac, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_DIGITTOANALOG, 5, ViewADcontinuous::on_overrun_error_dac, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_DIGITTOANALOG, 2, ViewADcontinuous::on_queue_done_dac, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_DIGITTOANALOG, 4, ViewADcontinuous::on_trigger_error_dac, VTS_NONE)

END_EVENTSINK_MAP()

void ViewADcontinuous::attach_controls()
{
	// attach controls
	VERIFY(m_chart_data_ad_.SubclassDlgItem(IDC_DISPLAYDATA, this));
	VERIFY(m_ad_y_ruler_bar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_ad_x_ruler_bar.SubclassDlgItem(IDC_XSCALE, this));
	VERIFY(m_bias_button.SubclassDlgItem(IDC_BIAS_button, this));
	VERIFY(m_zoom_button.SubclassDlgItem(IDC_GAIN_button, this));
	VERIFY(m_un_zoom_button.SubclassDlgItem(IDC_UNZOOM, this));
	VERIFY(m_combo_start_output.SubclassDlgItem(IDC_COMBOSTARTOUTPUT, this));

	m_ad_y_ruler_bar.attach_scope_wnd(&m_chart_data_ad_, FALSE);
	m_ad_x_ruler_bar.attach_scope_wnd(&m_chart_data_ad_, TRUE);
	m_chart_data_ad_.attach_external_x_ruler(&m_ad_x_ruler_bar);
	m_chart_data_ad_.attach_external_y_ruler(&m_ad_y_ruler_bar);
	m_chart_data_ad_.b_nice_grid = TRUE;

	stretch_.attach_parent(this);
	stretch_.new_prop(IDC_DISPLAYDATA, XLEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_XSCALE, XLEQ_XREQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_YSCALE, SZEQ_XLEQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_SCROLLY_scrollbar, SZEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_UNZOOM, SZEQ_XREQ, SZEQ_YTEQ);

	// bitmap buttons: load icons & set buttons
	m_h_bias_ = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_h_zoom_ = AfxGetApp()->LoadIcon(IDI_ZOOM);
	m_h_un_zoom_ = AfxGetApp()->LoadIcon(IDI_UNZOOM);
	m_bias_button.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_bias_)));
	m_zoom_button.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_zoom_)));
	m_un_zoom_button.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_un_zoom_)));

	const BOOL b32BitIcons = afxGlobalData.m_nBitsPerPixel >= 16;
	m_btn_start_stop_ad.SetImage(b32BitIcons ? IDB_CHECK32 : IDB_CHECK);
	m_btn_start_stop_ad.SetCheckedImage(b32BitIcons ? IDB_CHECKNO32 : IDB_CHECKNO);
	CMFCButton::EnableWindowsTheming(false);
	m_btn_start_stop_ad.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;

	// scrollbar
	VERIFY(m_scroll_y_.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scroll_y_.SetScrollRange(0, 100);
}

void ViewADcontinuous::get_acquisition_parameters_from_data_file() 
{
	const auto pdbDoc = GetDocument();
	const auto pDat = pdbDoc->open_current_data_file();
	if (pDat != nullptr)
	{
		pDat->read_data_infos();
		options_input_data_->wave_format.copy(pDat->get_wave_format());
		options_input_data_->chan_array.chan_array_set_size(options_input_data_->wave_format.scan_count);
		options_input_data_->chan_array.Copy(pDat->get_wave_channels_array());
		options_input_data_->wave_format.b_ad_write_to_file = b_ad_write_to_file;
	}
}

void ViewADcontinuous::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	attach_controls();

	const auto pApp = static_cast<CdbWaveApp*>(AfxGetApp());
	options_input_data_ = &(pApp->options_acq_data);
	options_output_data_ = &(pApp->options_output_data);

	m_b_found_dt_open_layer_dll_ = FALSE;
	b_ad_write_to_file = options_input_data_->wave_format.b_ad_write_to_file;
	m_b_start_out_put_mode = options_output_data_->b_allow_output_data;
	m_combo_start_output.SetCurSel(m_b_start_out_put_mode);

	// if current document, load parameters from current document into the local set of parameters
	get_acquisition_parameters_from_data_file();

	// create data file and copy data acquisition parameters into it
	input_data_file_.OnNewDocument(); 
	input_data_file_.get_wave_format()->copy( &options_input_data_->wave_format);
	options_input_data_->chan_array.chan_array_set_size(options_input_data_->wave_format.scan_count);
	input_data_file_.get_wave_channels_array()->Copy(&options_input_data_->chan_array);
	m_chart_data_ad_.attach_data_file(&input_data_file_);

	pApp->m_ad_card_found = find_dt_open_layers_boards();
	if (pApp->m_ad_card_found)
	{
		init_output_ad();
		initialize_amplifiers(); 
		m_acq32_da.InitSubSystem(options_input_data_);
		m_acq32_da.ClearAllOutputs();
	}
	else
	{
		m_btn_start_stop_ad.ShowWindow(SW_HIDE);
		m_button_sampling_mode.ShowWindow(SW_HIDE);
		m_button_output_channels.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DAGROUP)->ShowWindow(SW_HIDE);
		m_combo_start_output.ShowWindow(SW_HIDE);
		m_button_start_stop_da.ShowWindow(SW_HIDE);
	}

	update_gain_scroll();
	update_radio_buttons();
	GetParent()->PostMessage(WM_MYMESSAGE, NULL, MAKELPARAM(m_cursor_state_, HINT_SET_MOUSE_CURSOR));
}

void ViewADcontinuous::on_cbn_sel_change_combo_board()
{
	const int item_selected = m_combo_ad_card.GetCurSel();
	CString card_name;
	m_combo_ad_card.GetLBText(item_selected, card_name);
	select_dt_open_layers_board(card_name);
}

BOOL ViewADcontinuous::find_dt_open_layers_boards()
{
	m_combo_ad_card.ResetContent();

	// load board name - skip dialog if only one is present
	const short uiNumBoards = m_acq32_ad.GetNumBoards();
	if (uiNumBoards == 0)
	{
		m_combo_ad_card.AddString(_T("No Board"));
		m_combo_ad_card.SetCurSel(0);
		return FALSE;
	}

	for (short i = 0; i < uiNumBoards; i++)
		m_combo_ad_card.AddString(m_acq32_ad.GetBoardList(i));

	short i_sel = 0;
	// if name already defined, check if board present
	if (!(options_input_data_->wave_format).cs_ad_card_name.IsEmpty())
		i_sel = static_cast<short>(m_combo_ad_card.FindString(-1, (options_input_data_->wave_format).cs_ad_card_name));
	if (i_sel < 0)
		i_sel = 0;

	m_combo_ad_card.SetCurSel(i_sel);
	m_boardName = m_acq32_ad.GetBoardList(i_sel);
	select_dt_open_layers_board(m_boardName);
	return TRUE;
}

BOOL ViewADcontinuous::select_dt_open_layers_board(const CString& card_name)
{
	// get infos
	m_b_found_dt_open_layer_dll_ = TRUE;
	(options_input_data_->wave_format).cs_ad_card_name = card_name;

	// connect A/D subsystem and display/hide buttons
	m_b_start_out_put_mode = 0;
	const BOOL flag_ad = m_acq32_ad.OpenSubSystem(card_name);
	const BOOL flag_da = m_acq32_da.OpenSubSystem(card_name);
	if (flag_da)
		m_b_start_out_put_mode = 0;
	b_simultaneous_start_ = m_start_da_simultaneously_ && m_acq32_ad.IsSimultaneousStart();

	// display additional interface elements
	int show = (flag_ad ? SW_SHOW : SW_HIDE);
	m_button_sampling_mode.ShowWindow(show);
	m_button_output_channels.ShowWindow(show);
	m_combo_start_output.ShowWindow(show);

	show = (flag_da ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DAPARAMETERS2)->ShowWindow(show);
	GetDlgItem(IDC_DAGROUP)->ShowWindow(show);
	m_combo_start_output.ShowWindow(show);
	m_button_start_stop_da.ShowWindow(show);
	if (show == SW_SHOW)
		set_combo_start_output(options_output_data_->b_allow_output_data);

	return TRUE;
}

void ViewADcontinuous::stop_acquisition()
{
	if (!m_acq32_ad.IsInProgress())
		return;

	// special treatment if simultaneous list
	if (b_simultaneous_start_ && m_b_start_out_put_mode == 0)
	{
		HSSLIST h_ss_list;
		CHAR error_string[255];
		ECODE e_code = olDaGetSSList(&h_ss_list);
		olDaGetErrorString(e_code, error_string, 255);
		e_code = olDaReleaseSSList(h_ss_list);
		olDaGetErrorString(e_code, error_string, 255);
	}

	// stop AD, liberate DT_buffers
	m_acq32_ad.StopAndLiberateBuffers();
	m_chart_data_ad_.stop_display();
	b_changed_ = TRUE;

	// stop DA, liberate buffers
	if (m_b_start_out_put_mode == 0)
		m_acq32_da.StopAndLiberateBuffers();

	// close file and update display
	if (b_file_open_)
	{
		save_and_close_file();
		update_view_data_final();
	}
}

void ViewADcontinuous::save_and_close_file()
{
	input_data_file_.AcqDoc_DataAppendStop();
	const CWaveFormat* pWFormat = input_data_file_.get_wave_format();

	// if burst data acquisition mode ------------------------------------
	if (m_b_hide_subsequent_)
	{
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
			cs_name_array_.Add(sz_file_name_);
		else
			input_data_file_.acq_delete_file();
	}

	// normal data acquisition mode --------------------------------------
	else
	{
		int result = IDCANCEL;
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
		{
			DlgConfirmSave dlg;
			dlg.m_cs_file_name = sz_file_name_;
			result = dlg.DoModal();
		}
		// if no data or user answered no, erase the data
		if (IDOK != result)
		{
			input_data_file_.acq_delete_file();
		}
		else
		{
			// -----------------------------------------------------
			// if current document name is the same, it means something happened, and we have erased a previously existing file
			// if so, skip
			// otherwise add data file name to the database
			auto* pdb_doc = GetDocument();
			if (sz_file_name_.CompareNoCase(pdb_doc->db_get_current_dat_file_name(FALSE)) != 0)
			{
				// add document to database
				cs_name_array_.Add(sz_file_name_);
				transfer_files_to_database();
				update_view_data_final();
			}
		}
	}
}

void ViewADcontinuous::update_view_data_final()
{
	// update view data	
	auto* pdb_doc = GetDocument();
	AcqDataDoc* p_doc_dat = pdb_doc->open_current_data_file();
	if (p_doc_dat == nullptr)
	{
		ATLTRACE2(_T("error reading current document "));
		return;
	}
	p_doc_dat->read_data_infos();
	const long length_doc_channel = p_doc_dat->get_doc_channel_length();
	m_chart_data_ad_.attach_data_file(p_doc_dat);
	m_chart_data_ad_.resize_channels(m_chart_data_ad_.get_rect_width(), length_doc_channel);
	m_chart_data_ad_.get_data_from_doc(0, length_doc_channel);
}

void ViewADcontinuous::transfer_files_to_database()
{
	const auto pdb_doc = GetDocument();
	pdb_doc->import_file_list(cs_name_array_); // add file name(s) to the list of records in the database
	cs_name_array_.RemoveAll(); // clear file names

	CdbTableMain* p_set = &(pdb_doc->db_table->m_main_table_set);
	p_set->build_and_sort_id_arrays();
	p_set->refresh_query();
	pdb_doc->db_set_current_record_position(pdb_doc->db_table->get_records_count() - 1);
	pdb_doc->update_all_views_db_wave(nullptr, HINT_DOC_MOVE_RECORD, nullptr);
}

BOOL ViewADcontinuous::init_output_da()
{
	m_da_present = m_acq32_da.InitSubSystem(options_input_data_);
	if (m_b_start_out_put_mode == 0 && m_da_present)
		m_acq32_da.DeclareAndFillBuffers(options_input_data_);
	return m_da_present;
}

BOOL ViewADcontinuous::init_output_ad()
{
	m_ad_present = m_acq32_ad.InitSubSystem(options_input_data_);
	if (m_ad_present)
	{
		m_acq32_ad.DeclareBuffers(options_input_data_);
		init_acquisition_input_file();
		init_acquisition_display();
	}
	return m_ad_present;
}

void ViewADcontinuous::init_acquisition_display()
{
	const CWaveFormat* pWFormat = &(options_input_data_->wave_format);
	m_chart_data_ad_.attach_data_file(&input_data_file_);
	m_chart_data_ad_.resize_channels(0, m_channel_sweep_length);
	if (m_chart_data_ad_.get_channel_list_size() != pWFormat->scan_count)
	{
		m_chart_data_ad_.remove_all_channel_list_items();
		for (int j = 0; j < pWFormat->scan_count; j++)
		{
			m_chart_data_ad_.add_channel_list_item(j, 0);
		}
	}

	// adapt source view 
	int i_extent = MulDiv(pWFormat->bin_span, 12, 10);
	if (options_input_data_->i_zoom_cur_sel != 0)
		i_extent = options_input_data_->i_zoom_cur_sel;

	for (int i = 0; i < pWFormat->scan_count; i++)
	{
		constexpr int i_offset = 0;
		CChanlistItem* p_d = m_chart_data_ad_.get_channel_list_item(i);
		p_d->set_y_zero(i_offset);
		p_d->set_y_extent(i_extent);
		p_d->set_color(static_cast<WORD>(i));
		float doc_volts_per_bin;
		input_data_file_.get_volts_per_bin(i, &doc_volts_per_bin);
		p_d->set_data_bin_format(pWFormat->bin_zero, pWFormat->bin_span);
		p_d->set_data_volts_format(doc_volts_per_bin, pWFormat->full_scale_volts);
	}

	update_gain_scroll();
	m_chart_data_ad_.Invalidate();
}

BOOL ViewADcontinuous::start_acquisition()
{
	// set display
	if (b_ad_write_to_file && !define_experiment())
	{
		stop_acquisition();
		update_start_stop(m_acq32_ad.IsInProgress());
		return FALSE;
	}

	init_output_da();
	init_output_ad();

	// start AD display
	m_channel_sweep_start_ = 0;
	m_channel_sweep_end_ = -1;
	m_chart_data_ad_.start_display(m_channel_sweep_length);
	CWaveFormat* p_w_format = input_data_file_.get_wave_format();
	p_w_format->sample_count = 0; 
	p_w_format->sampling_rate_per_channel = p_w_format->sampling_rate_per_channel / static_cast<float>(options_input_data_->i_under_sample);
	m_clock_rate_ = p_w_format->sampling_rate_per_channel * static_cast<float>(p_w_format->scan_count);
	p_w_format->acquisition_time = CTime::GetCurrentTime();

	// data format
	p_w_format->bin_span = (options_input_data_->wave_format).bin_span;
	p_w_format->full_scale_volts = (options_input_data_->wave_format).full_scale_volts;
	// trick: if OLx_ENC_BINARY, it is changed on the fly within AD_Transfer function 
	p_w_format->mode_encoding = OLx_ENC_2SCOMP;
	p_w_format->bin_zero = 0;

	// start acquisition and save data to file?
	if (b_ad_write_to_file && (p_w_format->trig_mode == OLx_TRG_EXTRA + 1))
	{
		if (AfxMessageBox(_T("Start data acquisition"), MB_OKCANCEL) != IDOK)
		{
			stop_acquisition();
			update_start_stop(m_acq32_ad.IsInProgress());
			return FALSE;
		}
	}

	// start
	if (!b_simultaneous_start_ || m_b_start_out_put_mode != 0)
	{
		m_acq32_ad.ConfigAndStart();
		if (m_b_start_out_put_mode == 0 && m_da_present)
			m_acq32_da.ConfigAndStart();
	}
	else
	{
		start_simultaneous_list();
	}

	return TRUE;
}

ECODE ViewADcontinuous::start_simultaneous_list()
{
	ECODE code_returned;
	HSSLIST h_ss_list;
	CHAR error_string[255];

	// create simultaneous starting list
	ECODE e_code = olDaGetSSList(&h_ss_list);
	olDaGetErrorString(e_code, error_string, 255);

	// DA system
	m_acq32_da.Config();
	e_code = olDaPutDassToSSList(h_ss_list, (HDASS)m_acq32_da.GetHDass());
	if (e_code != OLNOERROR)
	{
		code_returned = olDaReleaseSSList(h_ss_list);
		TRACE("error %i \n", code_returned);
		return e_code;
	}

	// AD system
	m_acq32_ad.Config();
	e_code = olDaPutDassToSSList(h_ss_list, (HDASS)m_acq32_ad.GetHDass());
	if (e_code != OLNOERROR)
	{
		code_returned = olDaReleaseSSList(h_ss_list);
		TRACE("error %i \n", code_returned);
		return e_code;
	}

	// pre-start
	e_code = olDaSimultaneousPrestart(h_ss_list);
	if (e_code != OLNOERROR)
	{
		olDaGetErrorString(e_code, error_string, 255);
		display_ol_da_error_message(error_string);
	}

	// start simultaneously
	e_code = olDaSimultaneousStart(h_ss_list);
	if (e_code != OLNOERROR)
	{
		olDaGetErrorString(e_code, error_string, 255);
		display_ol_da_error_message(error_string);
	}

	m_acq32_ad.SetInProgress();
	m_acq32_da.SetInProgress();
	return e_code;
}

void ViewADcontinuous::display_ol_da_error_message(const CHAR * error_string)
{
	CString cs_error;
	const CStringA string_a(error_string);
	cs_error = string_a;
	AfxMessageBox(cs_error);
}

#ifdef _DEBUG
void ViewADcontinuous::AssertValid() const
{
	CFormView::AssertValid();
}

void ViewADcontinuous::Dump(CDumpContext & dc) const
{
	CFormView::Dump(dc);
}

#endif //_DEBUG

CdbTableMain* ViewADcontinuous::OnGetRecordset()
{
	return m_p_table_set;
}

void ViewADcontinuous::OnUpdate(CView * p_sender, LPARAM l_hint, CObject * p_hint)
{
	// update sent from within this class
	if (p_sender == this)
	{
		ASSERT(GetDocument() != NULL);
		m_chart_data_ad_.Invalidate(); // display data
	}
}

void ViewADcontinuous::OnActivateView(BOOL b_activate, CView * p_activate_view, CView * p_deactive_view)
{
	const auto pmF = static_cast<CMainFrame*>(AfxGetMainWnd());
	if (b_activate)
	{
		pmF->ActivatePropertyPane(FALSE);
		static_cast<CChildFrame*>(pmF->MDIGetActive())->m_cursor_state = 0;
	}
	CFormView::OnActivateView(b_activate, p_activate_view, p_deactive_view);
}

void ViewADcontinuous::OnSize(const UINT n_type, const int cx, const int cy)
{
	switch (n_type)
	{
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
		if (m_acq32_ad.IsInProgress())
		{
			stop_acquisition();
			update_start_stop(false);
		}

		if (cx <= 0 || cy <= 0)
			break;
		stretch_.resize_controls(n_type, cx, cy);
		break;
	default:
		break;
	}
	CFormView::OnSize(n_type, cx, cy);
}

LRESULT ViewADcontinuous::on_my_message(WPARAM w_param, LPARAM l_param)
{
	// message emitted by IDC_DISPLAY_AREA_button	
	//if (j == IDC_DISPLAY_AREA_button)		// always true here...
	//int j = wParam;				// control ID of sender

	// parameters
	const int code = HIWORD(l_param); // code parameter
	// code = 0: chan hit 			low_p = channel
	// code = 1: cursor change		low_p = new cursor value
	// code = 2: horizontal cursor hit	low_p = cursor index	
	int low_l_param = LOWORD(l_param); // value associated with code

	if (code == HINT_SET_MOUSE_CURSOR) {
		if (low_l_param > CURSOR_ZOOM)
			low_l_param = 0;
		m_cursor_state_ = m_chart_data_ad_.set_mouse_cursor_type(low_l_param);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(m_cursor_state_, 0));
	}
	else {
		if (low_l_param == 0)
			MessageBeep(MB_ICONEXCLAMATION);
	}
	return 0L;
}

void ViewADcontinuous::on_bn_clicked_start_stop()
{
	if (m_btn_start_stop_ad.IsChecked())
	{
		if (start_acquisition())
		{
			if ((input_data_file_.get_wave_format())->trig_mode == OLx_TRG_EXTERN)
				on_buffer_done_adc();
		}
		else
		{
			stop_acquisition();
		}
	}
	else
	{
		stop_acquisition();
		if (m_b_hide_subsequent_)
		{
			transfer_files_to_database();
			update_view_data_final();
		}
		else if (b_changed_ && b_ask_erase_)
			if (AfxMessageBox(IDS_ACQDATA_SAVEYESNO, MB_YESNO) == IDYES)
				b_changed_ = FALSE;
	}
	update_start_stop(m_acq32_ad.IsInProgress());
}

void ViewADcontinuous::update_start_stop(const BOOL b_start)
{
	if (b_start)
	{
		m_btn_start_stop_ad.SetWindowText(_T("STOP"));
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);
	}
	else
	{
		m_btn_start_stop_ad.SetWindowText(_T("START"));
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
		if (options_input_data_->b_audible_sound)
			Beep(500, 400);
		ASSERT(m_acq32_ad.IsInProgress() == FALSE);
	}
	m_btn_start_stop_ad.SetCheck(b_start);
	m_chart_data_ad_.Invalidate();
}

void ViewADcontinuous::on_hardware_define_experiment()
{
	define_experiment();
}

BOOL ViewADcontinuous::define_experiment()
{
	CString file_name;
	b_file_open_ = FALSE;
	if (!m_b_hide_subsequent_)
	{
		DlgADExperiment dlg;
		dlg.m_b_filename = true;
		dlg.options_input = options_input_data_;
		dlg.p_db_doc = GetDocument();
		dlg.m_b_hide_subsequent = m_b_hide_subsequent_;
		if (IDOK != dlg.DoModal())
			return FALSE;
		m_b_hide_subsequent_ = dlg.m_b_hide_subsequent;
		file_name = dlg.m_sz_file_name;
	}

	// hide define experiment dialog
	else
	{
		// build file name
		CString cs_buf_temp;
		options_input_data_->experiment_number++;
		cs_buf_temp.Format(_T("%06.6lu"), options_input_data_->experiment_number);
		file_name = options_input_data_->cs_pathname + options_input_data_->cs_basename + cs_buf_temp + _T(".dat");

		// check if this file is already present, exit if not...
		CFileStatus status;
		int i_id_response = IDYES; 
		if (CFile::GetStatus(file_name, status))
			i_id_response = AfxMessageBox(IDS_FILEOVERWRITE, MB_YESNO | MB_ICONWARNING);
		// no? find first available number
		while (IDNO == i_id_response)
		{
			BOOL flag = TRUE;
			while (flag)
			{
				options_input_data_->experiment_number++;
				cs_buf_temp.Format(_T("%06.6lu"), options_input_data_->experiment_number);
				file_name = options_input_data_->cs_pathname + options_input_data_->cs_basename + cs_buf_temp + _T(".dat");
				flag = CFile::GetStatus(file_name, status);
			}
			const CString cs = _T("Next available file name: ") + file_name;
			i_id_response = AfxMessageBox(cs, MB_YESNO | MB_ICONWARNING);
		}
	}
	// close current file and open new file to prepare it for adding chunks of data
	input_data_file_.acq_close_file();
	if (!input_data_file_.acq_create_file(file_name))
		return FALSE;
	sz_file_name_ = file_name;
	input_data_file_.AcqDoc_DataAppendStart();
	b_file_open_ = TRUE;
	return TRUE;
}

void ViewADcontinuous::on_input_channels()
{
	UpdateData(TRUE);

	DlgADInputs dlg;
	dlg.m_pw_format = &(options_input_data_->wave_format);
	dlg.m_pch_array = &(options_input_data_->chan_array);
	dlg.n_channels_max_di = m_acq32_ad.GetSSCaps(OLSSC_MAXDICHANS);
	dlg.n_channels_max_se = m_acq32_ad.GetSSCaps(OLSSC_MAXSECHANS);
	dlg.channel_type = options_input_data_->b_channel_type;
	dlg.b_chain_dialog = TRUE;
	dlg.b_command_amplifier = TRUE;

	const auto p_alligator = new CUSBPxxS1();
	dlg.m_alligator_amplifier = p_alligator;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		options_input_data_->b_channel_type = dlg.channel_type;
		const boolean is_acquisition_running = m_acq32_ad.IsInProgress();
		if (dlg.is_ad_changed) 
		{
			if (is_acquisition_running)
				stop_acquisition();
			init_output_ad();
		}
		UpdateData(FALSE);
		update_gain_scroll();
		if (is_acquisition_running && dlg.is_ad_changed)
			start_acquisition();
	}
	delete p_alligator;

	if (dlg.m_postmessage != NULL)
		chain_dialog(dlg.m_postmessage);
}

void ViewADcontinuous::on_sampling_mode()
{
	DlgADIntervals dlg;
	CWaveFormat* p_w_format = &(options_input_data_->wave_format);
	dlg.m_p_wave_format = p_w_format;
	dlg.m_rate_minimum = 1.0f;
	dlg.m_rate_maximum = static_cast<float>(m_acq32_ad.GetMaximumFrequency() / p_w_format->scan_count);
	dlg.m_buffer_w_size_maximum = static_cast<UINT>(65536) * 4;
	dlg.m_under_sample_factor = options_input_data_->i_under_sample;
	dlg.m_b_audible_sound = options_input_data_->b_audible_sound;
	dlg.m_acquisition_duration = options_input_data_->duration_to_acquire;
	dlg.m_sweep_duration = m_sweep_duration_;
	dlg.m_b_chain_dialog = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		if (m_acq32_ad.IsInProgress()) {
			stop_acquisition();
			update_start_stop(m_acq32_ad.IsInProgress());
		}

		options_input_data_->i_under_sample = static_cast<int>(dlg.m_under_sample_factor);
		options_input_data_->b_audible_sound = dlg.m_b_audible_sound;
		options_input_data_->duration_to_acquire = dlg.m_acquisition_duration;
		m_sweep_duration_ = dlg.m_sweep_duration;
		options_input_data_->sweep_duration = m_sweep_duration_;
		init_output_ad();
		UpdateData(FALSE);
	}

	if (dlg.m_postmessage != NULL)
		chain_dialog(dlg.m_postmessage);
}

void ViewADcontinuous::chain_dialog(const WORD i_id)
{
	WORD menu_id;
	switch (i_id)
	{
	case IDC_ADINTERVALS:
		menu_id = ID_HARDWARE_AD_INTERVALS;
		break;
	case IDC_ADCHANNELS:
		menu_id = ID_HARDWARE_ADCHANNELS;
		break;
	default:
		return;
	}
	PostMessage(WM_COMMAND, menu_id, NULL);
}

void ViewADcontinuous::on_trigger_error_adc()
{
	stop_acquisition();
	update_start_stop(m_acq32_ad.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void ViewADcontinuous::on_queue_done_adc()
{
	stop_acquisition();
	update_start_stop(m_acq32_ad.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_TOOFAST);
}

void ViewADcontinuous::on_overrun_error_adc()
{
	stop_acquisition();
	update_start_stop(m_acq32_ad.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_OVERRUN);
}

void ViewADcontinuous::on_overrun_error_dac()
{
	m_acq32_da.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_OVERRUN);
}

void ViewADcontinuous::on_queue_done_dac()
{
	m_acq32_da.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TOOFAST);
}

void ViewADcontinuous::on_trigger_error_dac()
{
	m_acq32_da.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void ViewADcontinuous::on_buffer_done_adc()
{
	short* p_buffer_done = m_acq32_ad.OnBufferDone();
	if (p_buffer_done == nullptr)
		return;

	CWaveFormat* wave_format = input_data_file_.get_wave_format();
	short* p_raw_data_buf = adc_transfer(p_buffer_done, wave_format);
	adc_transfer_to_file(wave_format);
	adc_transfer_to_chart(p_raw_data_buf, wave_format);
}

short* ViewADcontinuous::adc_transfer(short* source_data, const CWaveFormat * p_w_format)
{
	short* p_raw_data_buf = input_data_file_.get_raw_data_buffer();

	m_channel_sweep_start_ = m_channel_sweep_end_ + 1;
	if (m_channel_sweep_start_ >= m_channel_sweep_length)
		m_channel_sweep_start_ = 0;
	m_channel_sweep_end_ = m_channel_sweep_start_ + m_acq32_ad.Getchbuflen() - 1;
	m_channel_sweep_refresh_ = m_channel_sweep_end_ - m_channel_sweep_start_ + 1;
	p_raw_data_buf += (m_channel_sweep_start_ * p_w_format->scan_count);

	// if offset binary (unsigned words), transform data into signed integers (two's complement)
	if ((options_input_data_->wave_format).bin_zero != NULL)
	{
		const auto bin_zero_value = static_cast<short>(options_input_data_->wave_format.bin_zero);
		short* p_data_acquisition_value = source_data;
		for (int j = 0; j < m_acq32_ad.Getbuflen(); j++, p_data_acquisition_value++)
		{
			*p_data_acquisition_value -= static_cast<short>(bin_zero_value);
		}
	}

	if (options_input_data_->i_under_sample <= 1)
		memcpy(p_raw_data_buf, source_data, m_acq32_ad.Getbuflen() * sizeof(short));
	else
		under_sample_buffer(p_raw_data_buf, source_data, p_w_format, options_input_data_->i_under_sample);

	m_byte_sweep_refresh_ = m_channel_sweep_refresh_ * static_cast<int>(sizeof(short)) * static_cast<int>(p_w_format->scan_count);

	return p_raw_data_buf;
}

void ViewADcontinuous::under_sample_buffer(short* p_raw_data_buf, short* p_dt_buf0, const CWaveFormat* p_w_format, const int under_sample_factor)
{
	short* pdata_buf2 = p_raw_data_buf;
	short* p_dt_buf = p_dt_buf0;
	m_channel_sweep_refresh_ = m_channel_sweep_refresh_ / under_sample_factor;
	// loop and compute average between consecutive points
	for (int j = 0; j < p_w_format->scan_count; j++, pdata_buf2++, p_dt_buf++)
	{
		const short* p_source = p_dt_buf;
		short* p_dest = pdata_buf2;
		for (int i = 0; i < m_acq32_ad.Getchbuflen(); i += under_sample_factor)
		{
			long sum = 0;
			for (int k = 0; k < under_sample_factor; k++)
			{
				sum += *p_source;
				p_source += p_w_format->scan_count;
			}
			*p_dest = static_cast<short>(sum / under_sample_factor);
			p_dest += p_w_format->scan_count;
		}
	}
}

void ViewADcontinuous::adc_transfer_to_chart(short* p_data_buf, const CWaveFormat * p_w_format)
{
	if (p_w_format->b_online_display)
		m_chart_data_ad_.display_buffer(p_data_buf, m_channel_sweep_refresh_);
	
	const double duration = static_cast<double>(p_w_format->sample_count) / static_cast<double>(m_clock_rate_);
	CString cs;
	cs.Format(_T("%.3lf"), duration);
	SetDlgItemText(IDC_STATIC1, cs);
}

void ViewADcontinuous::adc_transfer_to_file(CWaveFormat * p_w_format)
{
	p_w_format->sample_count += m_byte_sweep_refresh_ / 2;
	const float duration = static_cast<float>(p_w_format->sample_count) / m_clock_rate_;

	short* pdata_buf = input_data_file_.get_raw_data_buffer();
	pdata_buf += (m_channel_sweep_start_ * p_w_format->scan_count);

	if (p_w_format->b_ad_write_to_file)
	{
		const BOOL flag = input_data_file_.AcqDoc_DataAppend(pdata_buf, m_byte_sweep_refresh_);
		ASSERT(flag);
		// end of acquisition
		if (duration >= options_input_data_->duration_to_acquire)
		{
			stop_acquisition();
			if (m_b_hide_subsequent_)
			{
				if (!start_acquisition())
					stop_acquisition();
				else if ((input_data_file_.get_wave_format())->trig_mode == OLx_TRG_EXTERN)
					on_buffer_done_adc();
				return;
			}
			update_start_stop(FALSE);
			return;
		}
	}
	m_acq32_ad.ReleaseLastBufferToQueue();
}

void ViewADcontinuous::on_buffer_done_dac()
{
	m_acq32_da.OnBufferDone();
}

void ViewADcontinuous::initialize_amplifiers() const
{
	init_cyber_amp(); // TODO init other amplifiers and look at p_wave_format?
}

BOOL ViewADcontinuous::init_cyber_amp() const
{
	CyberAmp cyber_amp;
	BOOL cyber_amp_is_present = FALSE;
	const int n_ad_channels = (options_input_data_->chan_array).chan_array_get_size();

	// test if Cyber_amp320 selected
	for (int i = 0; i < n_ad_channels; i++)
	{
		const CWaveChan* p_wave_channel = (options_input_data_->chan_array).get_p_channel(i);

		const int a = p_wave_channel->am_csamplifier.Find(_T("CyberAmp"));
		const int b = p_wave_channel->am_csamplifier.Find(_T("Axon Instrument"));
		if (a == 0 || b == 0)
		{
			if (!cyber_amp_is_present)
				cyber_amp_is_present = (cyber_amp.Initialize() == C300_SUCCESS);
			if (!cyber_amp_is_present)
			{
				AfxMessageBox(_T("CyberAmp not found"), MB_OK);
				continue;
			}

			// chan, gain, filter + low-pass, notch	
			cyber_amp.SetHPFilter(p_wave_channel->am_amplifierchan, C300_POSINPUT, p_wave_channel->am_csInputpos);
			cyber_amp.SetmVOffset(p_wave_channel->am_amplifierchan, p_wave_channel->am_offset);

			cyber_amp.SetNotchFilter(p_wave_channel->am_amplifierchan, p_wave_channel->am_notchfilt);
			cyber_amp.SetGain(
				p_wave_channel->am_amplifierchan,
				static_cast<int>(p_wave_channel->am_gaintotal / (static_cast<double>(p_wave_channel->am_gainheadstage) * static_cast<double>(p_wave_channel->
					am_gainAD))));
			cyber_amp.SetLPFilter(p_wave_channel->am_amplifierchan, static_cast<int>(p_wave_channel->am_lowpass));
			cyber_amp.C300_FlushCommandsAndAwaitResponse();
		}
	}
	return cyber_amp_is_present;
}

void ViewADcontinuous::on_bn_clicked_gain_button()
{
	set_v_bar_mode(BAR_GAIN);
}

void ViewADcontinuous::on_bn_clicked_bias_button()
{
	set_v_bar_mode(BAR_BIAS);
}

void ViewADcontinuous::OnVScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar * p_scroll_bar)
{
	if (p_scroll_bar == nullptr)
	{
		CFormView::OnVScroll(n_sb_code, n_pos, p_scroll_bar);
		return;
	}

	if(m_v_bar_mode_ == BAR_GAIN)
		on_gain_scroll(n_sb_code, n_pos);
	else
		on_bias_scroll(n_sb_code, n_pos);

}

void ViewADcontinuous::set_v_bar_mode(short b_mode)
{
	m_v_bar_mode_ = b_mode;
	m_bias_button.SetState(b_mode == BAR_BIAS);
	m_zoom_button.SetState(b_mode == BAR_GAIN);
	if (b_mode == BAR_GAIN)
		update_gain_scroll();
	else 
		update_bias_scroll();
}

void ViewADcontinuous::on_gain_scroll(const UINT n_sb_code, const UINT n_pos)
{
	const CChanlistItem* p_chan = m_chart_data_ad_.get_channel_list_item(0);
	int y_extent = p_chan->get_y_extent();
	const int span = p_chan->get_data_bin_span();

	switch (n_sb_code)
	{
		case SB_LEFT: y_extent = Y_EXTENT_MIN;
			break;
		case SB_LINELEFT: y_extent -= static_cast<int>(static_cast<float>(span) / 100.f) + 1;
			break;
		case SB_LINERIGHT: y_extent += static_cast<int>(static_cast<float>(span) / 100.f) + 1;
			break;
		case SB_PAGELEFT: y_extent -= static_cast<int>(static_cast<float>(span) / 10.f) + 1;
			break;
		case SB_PAGERIGHT: y_extent += static_cast<int>(static_cast<float>(span) / 10.f) + 1;
			break;
		case SB_RIGHT: y_extent = Y_EXTENT_MAX;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK: y_extent = MulDiv(static_cast<int>(n_pos - 50), p_chan->get_data_bin_span(), 100);
			break;
		default: break;
	}

	if (y_extent > 0) 
	{
		const CWaveFormat* pWFormat = &(options_input_data_->wave_format);
		const int last_channel = pWFormat->scan_count - 1;
		for (int channel = 0; channel <= last_channel; channel++)
		{
			CChanlistItem* ppChan = m_chart_data_ad_.get_channel_list_item(channel);
			ppChan->set_y_extent(y_extent);
		}
		m_chart_data_ad_.Invalidate();
		options_input_data_->i_zoom_cur_sel = y_extent;
	}

	update_gain_scroll();
	UpdateData(false);
}

void ViewADcontinuous::on_bias_scroll(const UINT n_sb_code, const UINT n_pos)
{
	const CChanlistItem* p_chan = m_chart_data_ad_.get_channel_list_item(0);
	int y_zero = p_chan->get_y_zero();
	const int span = p_chan->get_data_bin_span() / 2;
	const int initial = y_zero;
	switch (n_sb_code)
	{
		case SB_LEFT: 
			y_zero = Y_ZERO_MIN;
			break;
		case SB_LINELEFT: 
			y_zero -= static_cast<int>(static_cast<float>(span) / 100.f) + 1;
			break;
		case SB_LINERIGHT: 
			y_zero += static_cast<int>(static_cast<float>(span) / 100.f) + 1;
			break;
		case SB_PAGELEFT:
			y_zero -= static_cast<int>(static_cast<float>(span) / 10.f) + 1;
			break;
		case SB_PAGERIGHT:
			y_zero += static_cast<int>(static_cast<float>(span) / 10.f) + 1;
			break;
		case SB_RIGHT: 
			y_zero = Y_ZERO_MAX;
			break;
		case SB_THUMBPOSITION: 		
		case SB_THUMBTRACK:
			y_zero = static_cast<int>(n_pos - 50) * (Y_ZERO_SPAN / 100);
			break;
		default: // NOP: set position only
			break;
	}
	
	const CWaveFormat* p_w_format = &(options_input_data_->wave_format);
	constexpr int first_channel = 0;
	const int last_channel = p_w_format->scan_count - 1;
	for (int i = first_channel; i <= last_channel; i++)
	{
		CChanlistItem* p_chan_i = m_chart_data_ad_.get_channel_list_item(i);
		p_chan_i->set_y_zero(y_zero);
	}
	m_chart_data_ad_.Invalidate();

	update_bias_scroll();
	UpdateData(false);
}

void ViewADcontinuous::update_bias_scroll()
{
	const CChanlistItem* p_chan = m_chart_data_ad_.get_channel_list_item(0);
	m_chart_data_ad_.update_y_ruler();
	const int i_pos = MulDiv(p_chan->get_y_zero(), 100, p_chan->get_data_bin_span())+50;
	m_scroll_y_.SetScrollPos(i_pos, TRUE);
}

void ViewADcontinuous::update_gain_scroll()
{
	const CChanlistItem* p_chan = m_chart_data_ad_.get_channel_list_item(0);
	m_chart_data_ad_.update_y_ruler();
	const int i_pos = MulDiv(p_chan->get_y_extent(), 100, p_chan->get_data_bin_span()) + 50;
	m_scroll_y_.SetScrollPos(i_pos, TRUE);
}

void ViewADcontinuous::on_cbn_sel_change_combo_start_output()
{
	m_b_start_out_put_mode = m_combo_start_output.GetCurSel();
	options_output_data_->b_allow_output_data = m_b_start_out_put_mode;
	m_button_start_stop_da.EnableWindow(m_b_start_out_put_mode != 0);
}

void ViewADcontinuous::set_combo_start_output(int option)
{
	m_combo_start_output.SetCurSel(option);
	option = m_combo_start_output.GetCurSel();
	m_b_start_out_put_mode = option;
	options_output_data_->b_allow_output_data = option;
	m_button_start_stop_da.EnableWindow(m_b_start_out_put_mode != 0);
}

void ViewADcontinuous::on_bn_clicked_da_parameters2()
{
	DlgDAChannels dlg;
	const auto i_size = options_output_data_->output_parameters_array.GetSize();
	if (i_size < 10)
		options_output_data_->output_parameters_array.SetSize(10);
	dlg.output_params_array.SetSize(10);
	for (auto i = 0; i < 10; i++)
		dlg.output_params_array[i] = options_output_data_->output_parameters_array[i];
	const CWaveFormat* wave_format = &(options_input_data_->wave_format);
	dlg.m_sampling_rate = wave_format->sampling_rate_per_channel;

	if (IDOK == dlg.DoModal())
	{
		for (int i = 0; i < 10; i++)
			options_output_data_->output_parameters_array[i] = dlg.output_params_array[i];
		m_acq32_da.SetChannelList();
		m_button_start_stop_da.EnableWindow(m_acq32_da.GetDigitalChannel() > 0);
	}
}

void ViewADcontinuous::on_bn_clicked_write_to_disk()
{
	b_ad_write_to_file = TRUE;
	options_input_data_->wave_format.b_ad_write_to_file = b_ad_write_to_file;
	input_data_file_.get_wave_format()->b_ad_write_to_file = b_ad_write_to_file;
}

void ViewADcontinuous::on_bn_clicked_oscilloscope()
{
	b_ad_write_to_file = FALSE;
	options_input_data_->wave_format.b_ad_write_to_file = b_ad_write_to_file;
	input_data_file_.get_wave_format()->b_ad_write_to_file = b_ad_write_to_file;
}

void ViewADcontinuous::update_radio_buttons()
{
	if (b_ad_write_to_file)
		m_button_write_to_disk.SetCheck(BST_CHECKED);
	else
		m_button_oscilloscope.SetCheck(BST_CHECKED);
	UpdateData(TRUE);
}

void ViewADcontinuous::on_bn_clicked_card_features()
{
	DlgDataTranslationBoard dlg;
	dlg.m_p_analog_in = &m_acq32_ad;
	dlg.m_p_analog_out = &m_acq32_da;
	dlg.DoModal();
}

void ViewADcontinuous::on_bn_clicked_start_stop2()
{
	CString cs;
	if (m_acq32_da.IsInProgress())
	{
		cs = _T("Start");
		stop_output();
	}
	else
	{
		cs = _T("Stop");
		start_output();
	}
	m_button_start_stop_da.SetWindowTextW(cs);
}

BOOL ViewADcontinuous::start_output()
{
	if (!m_acq32_da.InitSubSystem(options_input_data_))
		return FALSE;
	m_acq32_da.DeclareAndFillBuffers(options_input_data_);
	m_acq32_ad.DeclareBuffers(options_input_data_);
	m_acq32_da.ConfigAndStart();
	return TRUE;
}

void ViewADcontinuous::stop_output()
{
	m_acq32_da.StopAndLiberateBuffers();
}

void ViewADcontinuous::init_acquisition_input_file()
{
	const CWaveFormat* pWFormat = &(options_input_data_->wave_format);

	m_channel_sweep_length = static_cast<long>(m_sweep_duration_ * pWFormat->sampling_rate_per_channel / static_cast<float>(options_input_data_->i_under_sample));
	// AD system is changed:  update AD buffers & change encoding: it is changed on-the-fly in the transfer loop
	input_data_file_.get_wave_channels_array()->Copy(&options_input_data_->chan_array);
	input_data_file_.get_wave_format()->copy( &options_input_data_->wave_format);

	// set sweep length to the nb of data buffers
	input_data_file_.get_wave_format()->sample_count = m_channel_sweep_length * static_cast<long>(pWFormat->scan_count);
	input_data_file_.adjust_buffer(m_channel_sweep_length);
}

void ViewADcontinuous::on_bn_clicked_un_zoom()
{
	const CWaveFormat* p_w_format = &(options_input_data_->wave_format);
	const int i_extent = p_w_format->bin_span;

	for (int i = 0; i < p_w_format->scan_count; i++)
	{
		constexpr int i_offset = 0;
		CChanlistItem* p_d = m_chart_data_ad_.get_channel_list_item(i);
		p_d->set_y_zero(i_offset);
		p_d->set_y_extent(i_extent);
	}

	set_v_bar_mode(BAR_GAIN);
}
