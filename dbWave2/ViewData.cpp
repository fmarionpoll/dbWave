#include "StdAfx.h"
#include "dbWave.h"
#include "resource.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "dbWaveDoc.h"
#include "DlgADIntervals.h"
#include "MeasureProperties.h"
#include "RulerBar.h"
#include "DlgdbEditRecord.h"
#include "DlgADInputParms.h"
#include "ViewData.h"

#include "NiceUnit.h"
#include "DlgCopyAs.h"
#include "DlgDataSeries.h"
#include "DlgDataSeriesFormat.h"
#include "DlgDataViewAbscissa.h"
#include "DlgDataViewOrdinates.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewData, ViewDbTable)

BEGIN_MESSAGE_MAP(ViewData, ViewDbTable)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()

	ON_BN_CLICKED(IDC_BIAS_button, &ViewData::on_clicked_bias)
	ON_BN_CLICKED(IDC_GAIN_button, &ViewData::on_clicked_gain)
	ON_COMMAND(ID_FORMAT_XSCALE, &ViewData::on_format_x_scale)
	ON_COMMAND(ID_FORMAT_SET_ORDINATES, &ViewData::on_format_y_scale)
	ON_COMMAND(ID_EDIT_COPY, &ViewData::on_edit_copy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &ViewData::on_update_edit_copy)
	ON_COMMAND(ID_TOOLS_DATA_SERIES, &ViewData::on_tools_data_series)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &ViewData::adc_on_hardware_channels_dlg)
	ON_COMMAND(ID_HARDWARE_AD_INTERVALS, &ViewData::adc_on_hardware_intervals_dlg)
	ON_COMMAND(ID_FORMAT_CENTER_CURVE, &ViewData::on_center_curve)
	ON_COMMAND(ID_FORMAT_GAIN_ADJUST, &ViewData::on_gain_adjust_curve)
	ON_COMMAND(ID_FORMAT_SPLIT_CURVES, &ViewData::on_split_curves)
	ON_COMMAND(ID_FORMAT_FIRST_FRAME, &ViewData::on_first_frame)
	ON_COMMAND(ID_FORMAT_LAST_FRAME, &ViewData::on_last_frame)
	ON_MESSAGE(WM_MYMESSAGE, &ViewData::on_my_message)
	ON_COMMAND(ID_FORMAT_VIEW_ALL_DATA_ON_ABSCISSA, &ViewData::on_view_all_data)
	ON_COMMAND(ID_FORMAT_DATA_SERIES_ATTRIBUTES, &ViewData::on_format_data_series_attributes)
	ON_COMMAND(ID_TOOLS_MEASURE_MODE, &ViewData::on_tools_measure_mode)
	ON_COMMAND(ID_TOOLS_MEASURE, &ViewData::on_tools_measure)
	ON_COMMAND(ID_TOOLS_VERTICAL_TAGS, &ViewData::on_tools_vertical_tags)
	ON_COMMAND(ID_TOOLS_HORIZONTAL_CURSORS, &ViewData::on_tools_horizontal_cursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_HORIZONTAL_CURSORS, &ViewData::on_update_tools_horizontal_cursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_VERTICAL_TAGS, &ViewData::on_update_tools_vertical_tags)
	ON_COMMAND(ID_HARDWARE_DEFINE_EXPERIMENT, &ViewData::adc_on_hardware_define_experiment)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewData::on_en_change_time_first)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewData::on_en_change_time_last)
	ON_COMMAND(ID_FORMAT_SET_ORDINATES, &ViewData::on_format_y_scale)
	ON_COMMAND(ID_FILE_PRINT, ViewDbTable::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, ViewDbTable::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, ViewDbTable::OnFilePrintPreview)
	ON_CBN_SELCHANGE(IDC_COMBOCHAN, &ViewData::on_cbn_sel_change_combo_chan)
END_MESSAGE_MAP()

ViewData::ViewData()
	: ViewDbTable(IDD)
{
}

ViewData::~ViewData()
{
	m_p_dat_->acq_close_file();
	DeleteObject(m_h_bias_);
	DeleteObject(m_h_zoom_);
}

void ViewData::DoDataExchange(CDataExchange* p_dx)
{
	ViewDbTable::DoDataExchange(p_dx);

	DDX_Text(p_dx, IDC_EDIT1, m_first_hz_cursor);
	DDX_Text(p_dx, IDC_EDIT2, m_second_hz_cursor);
	DDX_Text(p_dx, IDC_EDIT3, m_difference_second_minus_first);
	DDX_Text(p_dx, IDC_TIMEFIRST, m_time_first_abscissa);
	DDX_Text(p_dx, IDC_TIMELAST, m_time_last_abscissa);
	DDX_Control(p_dx, IDC_FILESCROLL, file_scroll_bar_);
	DDX_Control(p_dx, IDC_COMBOCHAN, m_combo_select_chan);
}

void ViewData::define_sub_classed_items()
{
	VERIFY(scroll_y_.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	scroll_y_.SetScrollRange(0, 100);

	// bitmap buttons: load icons & set buttons
	m_h_bias_ = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_h_zoom_ = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_bias_)));
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_h_zoom_)));

	VERIFY(chart_data.SubclassDlgItem(IDC_DISPLAY, this));
	VERIFY(mm_time_first_abscissa.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_time_last_abscissa.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(m_adc_y_ruler_bar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_ruler_bar_abscissa.SubclassDlgItem(IDC_XSCALE, this));
}

void ViewData::define_stretch_parameters()
{
	// save coordinates and properties of "always visible" controls
	stretch_.attach_parent(this); // attach form_view pointer
	stretch_.new_prop(IDC_DISPLAY, XLEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_COMBOCHAN, SZEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_SCROLLY_scrollbar, SZEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_SOURCE, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_YSCALE, SZEQ_XLEQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_XSCALE, XLEQ_XREQ, SZEQ_YBEQ);
	b_init_ = TRUE;
}

void ViewData::OnInitialUpdate()
{
	define_sub_classed_items();

	m_adc_y_ruler_bar.attach_scope_wnd(&chart_data, FALSE);
	m_ruler_bar_abscissa.attach_scope_wnd(&chart_data, TRUE);
	chart_data.attach_external_x_ruler(&m_ruler_bar_abscissa);
	chart_data.attach_external_y_ruler(&m_adc_y_ruler_bar);
	chart_data.b_nice_grid = TRUE;

	define_stretch_parameters();

	// init relation with document, display data, adjust parameters
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	options_view_data_ = &(p_app->options_view_data);
	options_data_measures_ = &(p_app->options_view_data_measure);

	// set data file
	ViewDbTable::OnInitialUpdate();
	update_file_parameters(TRUE); 

	chart_data.set_scope_parameters(&(options_view_data_->view_data));
	constexpr int legends_options = UPD_ABSCISSA | CHG_X_SCALE | UPD_ORDINATES | CHG_Y_SCALE;
	b_common_scale_ = TRUE;
	m_combo_select_chan.SetCurSel(chart_data.get_channel_list_size());
	update_legends(legends_options);

	on_split_curves();
}

void ViewData::OnUpdate(CView* p_sender, const LPARAM l_hint, CObject* p_hint)
{
	if (!b_init_)
		return;

	auto i_update = NULL;
	switch (LOWORD(l_hint))
	{
	case HINT_REPLACE_VIEW:
		return;
	case HINT_CLOSE_FILE_MODIFIED: 
		save_modified_file();
		break;
	case HINT_DOC_HAS_CHANGED: 
	case HINT_DOC_MOVE_RECORD:
		m_b_init_comment = TRUE;
		update_file_parameters();
		i_update = UPD_ABSCISSA | CHG_X_SCALE | UPD_ORDINATES | CHG_Y_SCALE;
		break;
	default:
		if (m_p_dat_ == nullptr)
			return;
		i_update = UPD_ABSCISSA | UPD_X_SCALE | UPD_ORDINATES | UPD_Y_SCALE;
		break;
	}
	update_legends(i_update);
	chart_data.Invalidate();
	set_v_bar_mode(m_v_bar_mode_);
}

void ViewData::update_legends(int legends_options)
{
	if (!chart_data.is_defined() && !m_b_valid_doc_)
		return;
	if (legends_options & UPD_ABSCISSA)
		update_file_scroll();
	if (legends_options & CHG_Y_SCALE)
	{
		update_horizontal_tags_value();
		legends_options |= CHG_Y_BAR;
	}
	if (legends_options & UPD_Y_SCALE)
		legends_options |= CHG_Y_BAR;
	if (legends_options & CHG_Y_BAR)
		update_y_zero(m_channel_selected, chart_data.get_channel_list_item(m_channel_selected)->get_y_zero());

	UpdateData(FALSE);
}

void ViewData::on_clicked_bias()
{
	// set bias down and set gain up CButton
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(0);
	set_v_bar_mode(BAR_BIAS);
}

void ViewData::on_clicked_gain()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(1);
	set_v_bar_mode(BAR_GAIN);
}

void ViewData::update_channel(const int channel)
{
	m_channel_selected = channel;
	if (m_channel_selected > chart_data.get_channel_list_size() - 1) 
		m_channel_selected = chart_data.get_channel_list_size() - 1; 
	else if (m_channel_selected < 0) 
		m_channel_selected = 0;

	if (m_channel_selected == channel)
		UpdateData(FALSE);
	else 
	{
		if (m_cursor_state_ == CURSOR_CROSS && options_data_measures_->w_option == MEASURE_HORIZONTAL
			&& chart_data.hz_tags.get_tag_list_size() > 0)
		{
			for (auto i = 0; i < chart_data.hz_tags.get_tag_list_size(); i++)
				chart_data.hz_tags.set_channel(i, m_channel_selected);
			update_horizontal_tags_value();
			chart_data.Invalidate();
		}
		update_legends(UPD_ORDINATES | CHG_Y_SCALE);
	}
}

void ViewData::on_format_y_scale()
{
	DlgDataViewOrdinates dlg;
	dlg.m_p_chart_data_wnd = &chart_data;
	dlg.m_channel = m_channel_selected;
	if (IDOK == dlg.DoModal())
		update_legends(UPD_ORDINATES | UPD_Y_SCALE | CHG_Y_BAR);

	chart_data.Invalidate();
}

void ViewData::on_tools_data_series()
{
	// init dialog data
	DlgDataSeries dlg;
	dlg.m_p_chart_data_wnd = &chart_data;
	dlg.m_pdb_doc = m_p_dat_;
	dlg.m_list_index = m_channel_selected;
	dlg.DoModal();

	m_channel_selected = dlg.m_list_index;
	update_legends(UPD_Y_SCALE);
}

void ViewData::on_edit_copy()
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
			chart_data.copy_as_text(dlg.m_i_option, dlg.m_i_unit, dlg.m_n_abscissa);
		else
		{
			CRect old_rect;
			chart_data.GetWindowRect(&old_rect);

			CRect rect(0, 0, options_view_data_->hz_resolution, options_view_data_->vt_resolution);
			pixels_count_0_ = chart_data.get_rect_width();

			// create metafile
			CMetaFileDC m_dc;
			const auto p_dc_ref = GetDC();
			auto cs_title = _T("dbWave\0") + m_p_dat_->GetTitle();
			cs_title += _T("\0\0");
			const CRect rect_bound(0, 0, 21000, 29700); // dimensions in HIMETRIC units (in .01-millimeter increments)
			const auto hm_dc = m_dc.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
			ASSERT(hm_dc != NULL);

			// Draw document in metafile.
			const CClientDC attrib_dc(this); // Create and attach attribute DC
			m_dc.SetAttribDC(attrib_dc.GetSafeHdc()); // from current screen

			const auto old_scope_struct= new options_scope_struct();
			options_scope_struct* new_scope_struct = chart_data.get_scope_parameters();
			*old_scope_struct = *new_scope_struct;
			new_scope_struct->b_draw_frame = options_view_data_->b_frame_rect;
			new_scope_struct->b_clip_rect = options_view_data_->b_clip_rect;
			chart_data.print(&m_dc, &rect);
			*new_scope_struct = *old_scope_struct;

			// print comments : set font
			memset(&log_font_, 0, sizeof(LOGFONT));
			GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &log_font_);
			p_old_font_ = nullptr;
			/*BOOL flag = */
			font_print_.CreateFontIndirect(&log_font_);
			p_old_font_ = m_dc.SelectObject(&font_print_);
			const int line_height = log_font_.lfHeight + 5;
			auto y_pixels_row = 0;
			constexpr auto x_column = 10;

			CString comments = _T("Abscissa: ");
			CString content;
			content.Format(_T("%g - %g s"), m_time_first_abscissa, m_time_last_abscissa);
			comments += content;
			m_dc.TextOut(x_column, y_pixels_row, comments);
			y_pixels_row += line_height;
			comments.Format(_T("Vertical bar (ch. 0) = %g mV"), chart_data.y_ruler.get_scale_increment());
			m_dc.TextOut(x_column, y_pixels_row, comments);
			y_pixels_row += line_height;
			comments.Format(_T("Horizontal bar = %g s"), chart_data.x_ruler.get_scale_increment());
			m_dc.TextOut(x_column, y_pixels_row, comments);
			y_pixels_row += line_height;

			// bars
			const auto p_old_brush = static_cast<CBrush*>(m_dc.SelectStockObject(BLACK_BRUSH));
			m_dc.MoveTo(0, y_pixels_row);
			const auto bottom = chart_data.y_ruler.get_scale_unit_pixels(rect.Height());
			m_dc.LineTo(0, y_pixels_row - bottom);
			m_dc.MoveTo(0, y_pixels_row);
			const auto left = chart_data.x_ruler.get_scale_unit_pixels(rect.Width());
			m_dc.LineTo(left, y_pixels_row);

			m_dc.SelectObject(p_old_brush);
			if (p_old_font_ != nullptr)
				m_dc.SelectObject(p_old_font_);
			font_print_.DeleteObject();

			// Close metafile
			ReleaseDC(p_dc_ref);
			const auto h_emf_tmp = m_dc.CloseEnhanced();
			ASSERT(h_emf_tmp != NULL);
			if (OpenClipboard())
			{
				EmptyClipboard(); 
				SetClipboardData(CF_ENHMETAFILE, h_emf_tmp); // put data
				CloseClipboard(); 
			}
			else
			{
				// Someone else has the Clipboard open...
				DeleteEnhMetaFile(h_emf_tmp); // delete data
				MessageBeep(0); // tell user something is wrong!
				AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
			}

			// restore initial conditions
			chart_data.resize_channels(pixels_count_0_, 0);
			chart_data.get_data_from_doc();
			chart_data.Invalidate();
		}
	}
}

void ViewData::on_update_edit_copy(CCmdUI* p_cmd_ui) 
{
	p_cmd_ui->Enable(chart_data.is_defined() != NULL); 
}

void ViewData::adc_on_hardware_channels_dlg() 
{
	DlgADInputs dlg;
	dlg.m_pw_format = m_p_dat_->get_wave_format();
	dlg.m_pch_array = m_p_dat_->get_wave_channels_array();
	if (IDOK == dlg.DoModal())
	{
		if(m_p_dat_->acq_save_data_descriptors())
			m_p_dat_->SetModifiedFlag(TRUE);
	}
}

void ViewData::adc_on_hardware_intervals_dlg() 
{
	DlgADIntervals dlg;
	dlg.m_p_wave_format = m_p_dat_->get_wave_format();
	if (IDOK == dlg.DoModal())
	{
		if (m_p_dat_->acq_save_data_descriptors())
			m_p_dat_->SetModifiedFlag(TRUE);
	}
}

void ViewData::chain_dialog(const WORD i_id)
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

void ViewData::on_first_frame()
{
	on_file_scroll(SB_LEFT, 1L);
}

void ViewData::on_last_frame()
{
	on_file_scroll(SB_RIGHT, 1L);
}

void ViewData::update_file_parameters(const BOOL b_update_interface)
{
	// load parameters from document file: none yet loaded?
	const BOOL b_first_update = (m_p_dat_ == nullptr);
	const auto dbwave_doc = GetDocument();
	const auto cs_dat_file = dbwave_doc->db_get_current_dat_file_name();
	if ((m_b_valid_doc_ = cs_dat_file.IsEmpty()))
		return;

	// open data file
	if (dbwave_doc->open_current_data_file() == nullptr)
	{
		MessageBox(_T("This data file could not be opened"), _T("The file might be missing, or inaccessible..."),
		           MB_OK);
		m_b_valid_doc_ = FALSE;
		return;
	}
	m_p_dat_ = dbwave_doc->m_p_data_doc;
	m_p_dat_->read_data_infos();
	const auto wave_format = m_p_dat_->get_wave_format();

	if (b_first_update)
	{
		m_sampling_rate_ = wave_format->sampling_rate_per_channel; 
		m_time_first_abscissa = 0.0f; 
		m_time_last_abscissa = static_cast<float>(m_p_dat_->get_doc_channel_length()) / m_sampling_rate_;
	}

	// load parameters from current data file
	chart_data.attach_data_file(m_p_dat_);
	m_p_dat_->SetModifiedFlag(FALSE);

	// OPTION: display entire file	--	(inactive if multirow)
	long l_first = 0;
	long l_last = m_p_dat_->get_doc_channel_length() - 1;

	if (!options_view_data_->b_complete_record || options_view_data_->b_multiple_rows && !b_first_update)
	{
		l_first = static_cast<long>(m_time_first_abscissa * m_sampling_rate_);
		l_last = static_cast<long>(m_time_last_abscissa * m_sampling_rate_);
		if (l_last > m_p_dat_->get_doc_channel_length() - 1) // last OK?
			l_last = m_p_dat_->get_doc_channel_length() - 1; // clip to the end of the file
	}
	m_sampling_rate_ = wave_format->sampling_rate_per_channel; // update sampling rate

	// display all channels
	auto chan_list_size = chart_data.get_channel_list_size();
	
	// display all channels (TRUE) / no : loop through all doc channels & add if necessary
	if (options_view_data_->b_all_channels || chan_list_size == 0)
	{
		for (auto doc_channel = 0; doc_channel < wave_format->scan_count; doc_channel++)
		{
			auto b_present = FALSE;
			for (auto j = chan_list_size - 1; j >= 0; j--)
			{
				if ((b_present = (chart_data.get_channel_list_item(j)->get_source_chan() == doc_channel)))
					break;
			}
			if (!b_present)
			{
				chart_data.add_channel_list_item(doc_channel, 0);
				chan_list_size++;
			}
			chart_data.get_channel_list_item(doc_channel)->set_color(static_cast<WORD>(doc_channel));
		}
	}

	// load real data from file and update time parameters
	chart_data.get_data_from_doc(l_first, l_last); 
	m_time_first_abscissa = static_cast<float>(chart_data.get_data_first_index()) / m_sampling_rate_; 
	m_time_last_abscissa = static_cast<float>(chart_data.get_data_last_index()) / m_sampling_rate_; 
	m_channel_selected = 0; // select chan 0

	if (!b_first_update)
		update_channels_display_parameters();

	// fill combo
	m_combo_select_chan.ResetContent();
	for (auto i = 0; i < chart_data.get_channel_list_size(); i++)
	{
		CString cs;
		cs.Format(_T("channel %i - "), i);
		cs = cs + chart_data.get_channel_list_item(i)->get_comment();
		m_combo_select_chan.AddString(cs);
	}
	if (scan_count_ > 1)
	{
		m_combo_select_chan.AddString(_T("all channels"));
	}
	if (!b_common_scale_)
		m_combo_select_chan.SetCurSel(0);
	else
		m_combo_select_chan.SetCurSel(chart_data.get_channel_list_size());

	// done
	if (b_update_interface)
	{
		update_file_scroll();
		update_legends(UPD_ABSCISSA | CHG_X_SCALE | CHG_Y_SCALE);
		chart_data.Invalidate();
	}
}

void ViewData::update_channels_display_parameters()
{
	const auto n_line_view_channels = chart_data.get_channel_list_size();
	int max;
	int min;
	if (!b_common_scale_)
	{
		for (auto i = 0; i < n_line_view_channels; i++)
		{
			// keep final gain constant even if amplifier gain has changed
			const CChanlistItem* chan_list_item = chart_data.get_channel_list_item(i);
			chan_list_item->get_max_min(&max, &min);
			auto y_extent = chan_list_item->get_y_extent();
			auto y_zero = chan_list_item->get_y_zero();

			if (options_view_data_->b_maximize_gain)
				y_extent = MulDiv(max - min + 1, 11, 10);
			// center curve
			if (options_view_data_->b_center_curves)
				y_zero = (max + min) / 2;

			update_y_extent(i, y_extent);
			update_y_zero(i, y_zero);
		}
	}
	else
	{
		constexpr auto chan_0 = 0;
		const CChanlistItem* p_chan0 = chart_data.get_channel_list_item(chan_0);
		auto y_extent = p_chan0->get_y_extent();
		auto y_zero = p_chan0->get_y_zero();
		if (options_view_data_->b_maximize_gain)
		{
			float v_max = 0.;
			float v_min = 0.;
			for (auto i = 0; i < n_line_view_channels; i++)
			{
				// keep final gain constant even if amplifier gain changed
				const CChanlistItem* p_chan = chart_data.get_channel_list_item(i);
				p_chan->get_max_min(&max, &min);
				const auto max_chan_i = p_chan->convert_data_bins_to_volts(max);
				const auto min_chan_i = p_chan->convert_data_bins_to_volts(min);
				if (max_chan_i > v_max)
					v_max = max_chan_i;
				if (min_chan_i < v_min)
					v_min = min_chan_i;
			}
			max = p_chan0->convert_volts_to_data_bins(v_max);
			min = p_chan0->convert_volts_to_data_bins(v_min);
			y_extent = MulDiv(max - min + 1, 10, 8);
			y_zero = (max + min) / 2;
		}
		update_y_extent(chan_0, y_extent);
		update_y_zero(chan_0, y_zero);
	}
	chart_data.Invalidate();
}

void ViewData::update_controls_according_to_cursor_selected()
{
	auto n_cmd_show = SW_HIDE;
	if (m_cursor_state_ == CURSOR_CROSS && options_data_measures_->w_option == MEASURE_HORIZONTAL
		&& chart_data.hz_tags.get_tag_list_size() > 0)
		n_cmd_show = SW_SHOW;

	// change windows state: edit windows
	GetDlgItem(IDC_STATIC1)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC3)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT1)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT3)->ShowWindow(n_cmd_show);

	// change cursors value
	if (m_cursor_state_ == CURSOR_CROSS && options_data_measures_->w_option == MEASURE_HORIZONTAL)
		update_horizontal_tags_value();
}

void ViewData::update_horizontal_tags_value()
{
	if (chart_data.hz_tags.get_tag_list_size() <= 0)
		return;
	const auto v1 = chart_data.hz_tags.get_value_int(0);
	auto i_tag = 0;
	if (chart_data.hz_tags.get_tag_list_size() > 1)
		i_tag = 1;
	const auto v2 = chart_data.hz_tags.get_value_int(i_tag);
	const auto mv_per_bin = chart_data.get_channel_list_item(m_channel_selected)->get_volts_per_bin() * 1000.0f;
	m_first_hz_cursor = static_cast<float>(v1) * mv_per_bin;
	m_second_hz_cursor = static_cast<float>(v2) * mv_per_bin;
	m_difference_second_minus_first = m_first_hz_cursor - m_second_hz_cursor;

	TCHAR sz_buffer[32];
	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_first_hz_cursor);
	GetDlgItem(IDC_EDIT1)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT1)->Invalidate();

	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_second_hz_cursor);
	GetDlgItem(IDC_EDIT2)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT2)->Invalidate();

	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_difference_second_minus_first);
	GetDlgItem(IDC_EDIT3)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT3)->Invalidate();
}

void ViewData::set_mouse_cursor(int low_parameter)
{
	// save current cursors into document if cursor_state = 3
	if (m_cursor_state_ == CURSOR_CROSS)
	{
		if (options_data_measures_->w_option == MEASURE_VERTICAL) 
		{
			const auto p_tag_list = m_p_dat_->get_vt_tags_list();
			p_tag_list->copy_tag_list(&chart_data.vt_tags);
			chart_data.vt_tags.remove_all_tags();
		}
		else if (options_data_measures_->w_option == MEASURE_HORIZONTAL) // horizontal cursors
		{
			const auto p_tag_list = m_p_dat_->get_hz_tags_list();
			p_tag_list->copy_tag_list(&chart_data.hz_tags);
			chart_data.hz_tags.remove_all_tags();
		}
		else if (options_data_measures_->w_option == MEASURE_STIMULUS) // detect stimulus
		{
			options_data_measures_->w_stimulus_channel = static_cast<WORD>(chart_data.hz_tags.get_channel(0));
			options_data_measures_->w_stimulus_threshold = static_cast<WORD>(chart_data.hz_tags.get_value_int(0));
			chart_data.hz_tags.remove_all_tags();
		}
		chart_data.Invalidate();
	}
	// change cursor value (+1), clip to upper cursor value
	if (low_parameter > CURSOR_CROSS)
		low_parameter = 0;

	// change cursor and tell parent that it has changed
	m_cursor_state_ = chart_data.set_mouse_cursor_type(low_parameter);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(m_cursor_state_, 0));

	// recall cursors from document if cursor_state = 2
	if (m_cursor_state_ == CURSOR_CROSS)
	{
		if (options_data_measures_->w_option == MEASURE_VERTICAL)
			chart_data.vt_tags.copy_tag_list(m_p_dat_->get_vt_tags_list());
		else if (options_data_measures_->w_option == MEASURE_HORIZONTAL)
			chart_data.hz_tags.copy_tag_list(m_p_dat_->get_hz_tags_list());
		else if (options_data_measures_->w_option == MEASURE_STIMULUS)
			chart_data.hz_tags.add_tag(options_data_measures_->w_stimulus_threshold, options_data_measures_->w_stimulus_channel);
		chart_data.Invalidate();
	}
	update_controls_according_to_cursor_selected();
}

void ViewData::add_vertical_cursors_from_defined_rectangle()
{
	// if no VT tags, then take those of rectangle, or limits of line_view
	chart_data.vt_tags.add_l_tag(options_data_measures_->l_limit_left, 0);
	if (options_data_measures_->l_limit_right != options_data_measures_->l_limit_left)
		chart_data.vt_tags.add_l_tag(options_data_measures_->l_limit_right, 0);

	// store new VT tags into document
	m_p_dat_->get_vt_tags_list()->copy_tag_list(&chart_data.vt_tags);
}

void ViewData::add_horizontal_cursors_from_defined_rectangle()
{
	chart_data.hz_tags.add_tag(chart_data.get_channel_list_y_pixels_to_bin(m_channel_selected, options_data_measures_->w_limit_sup),
		m_channel_selected);
	if (options_data_measures_->w_limit_inf != options_data_measures_->w_limit_sup)
		chart_data.hz_tags.add_tag(
			chart_data.get_channel_list_y_pixels_to_bin(m_channel_selected, options_data_measures_->w_limit_inf), m_channel_selected);
	m_p_dat_->get_hz_tags_list()->copy_tag_list(&chart_data.hz_tags);

	if (chart_data.hz_tags.get_tag_list_size() == 2)
		update_controls_according_to_cursor_selected();
	update_horizontal_tags_value();
}

LRESULT ViewData::on_my_message(const WPARAM w_param, const LPARAM l_param)
{
	//int j = HIWORD(lParam);		// control ID of sender
	// code = 0: chan hit 			low_p = channel
	// code = 1: cursor change		low_p = new cursor value
	// code = 2: horizontal cursor hit	low_p = cursor index
	const int low_parameter = LOWORD(l_param); // value associated

	switch (w_param)
	{
	case HINT_SET_MOUSE_CURSOR:
		set_mouse_cursor(low_parameter);
		break;

	case HINT_HIT_CHANNEL: // change channel if different
		m_channel_selected = low_parameter;
		update_legends(UPD_ORDINATES | CHG_Y_SCALE);
		break;

	case HINT_DEFINED_RECT:
		{
			const auto rect = chart_data.get_defined_rect();
			options_data_measures_->w_limit_sup = static_cast<WORD>(rect.top);
			options_data_measures_->w_limit_inf = static_cast<WORD>(rect.bottom);
			options_data_measures_->l_limit_left = chart_data.get_data_offset_from_pixel(rect.left);
			options_data_measures_->l_limit_right = chart_data.get_data_offset_from_pixel(rect.right);
		}
	// action according to option
		switch (options_data_measures_->w_option)
		{
		case 0: 
			add_vertical_cursors_from_defined_rectangle();
			break;

		case 1: 
			add_horizontal_cursors_from_defined_rectangle();
			break;

		// ......................  rectangle area
		//case 2:				// parameters are already within line_view and mdMO
		//	break;
		// ......................  detect stimulus and then measure
		//case 3:				// if not displayed, plot HZ detection cursor
		//	break;
		default:
			break;
		}
		chart_data.Invalidate();
		break;

	case HINT_CHANGE_HZ_TAG: // horizontal tag has changed 	low_p = tag nb
		if (options_data_measures_->w_option == MEASURE_STIMULUS)
			options_data_measures_->w_stimulus_threshold = static_cast<WORD>(chart_data.hz_tags.get_value_int(0));
		else
			update_horizontal_tags_value();
		break;

	case HINT_VIEW_SIZE_CHANGED: // change zoom
		update_legends(UPD_ABSCISSA | CHG_X_SCALE | UPD_ORDINATES | CHG_Y_SCALE);
		chart_data.Invalidate();
		set_v_bar_mode(m_v_bar_mode_);
		break;

	case HINT_WINDOW_PROPS_CHANGED:
		options_view_data_->view_data = *(chart_data.get_scope_parameters());
		break;
	default:
		break;
	}
	return 0L;
}

void ViewData::on_view_all_data()
{
	chart_data.get_data_from_doc(0, GetDocument()->db_get_data_len() - 1);
	update_legends(UPD_ABSCISSA | CHG_X_SCALE);
	UpdateData(FALSE);
	chart_data.Invalidate();
	update_file_scroll();
}

void ViewData::on_format_data_series_attributes()
{
	DlgDataSeriesFormat dlg;
	dlg.m_p_chart_data_wnd = &chart_data;
	dlg.m_pdb_doc = m_p_dat_;
	dlg.m_list_index = m_channel_selected;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_channel_selected = dlg.m_list_index;
	}
	update_legends(UPD_Y_SCALE);
	chart_data.Invalidate();
}

void ViewData::on_tools_vertical_tags()
{
	options_data_measures_->w_option = MEASURE_VERTICAL;
	m_cursor_state_ = chart_data.set_mouse_cursor_type(CURSOR_CROSS);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(m_cursor_state_, 0));
	//MeasureProperties(1);
}

void ViewData::on_tools_horizontal_cursors()
{
	options_data_measures_->w_option = MEASURE_HORIZONTAL;
	m_cursor_state_ = chart_data.set_mouse_cursor_type(CURSOR_CROSS);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(m_cursor_state_, 0));
	//MeasureProperties(0);
}

void ViewData::on_update_tools_horizontal_cursors(CCmdUI* p_cmd_ui)
{
	p_cmd_ui->Enable(true);
}

void ViewData::on_update_tools_vertical_tags(CCmdUI* p_cmd_ui)
{
	p_cmd_ui->Enable(true);
}

void ViewData::on_tools_measure_mode()
{
	measure_properties(2);
}

void ViewData::on_tools_measure()
{
	measure_properties(3);
}

void ViewData::OnVScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar* p_scroll_bar)
{
	// form_view scroll: if pointer null
	if (p_scroll_bar == nullptr)
	{
		ViewDbTable::OnVScroll(n_sb_code, n_pos, p_scroll_bar);
		return;
	}

	// ViewData scroll: vertical scroll bar
	switch (m_v_bar_mode_)
	{
	case BAR_GAIN:
		on_gain_scroll(n_sb_code, n_pos);
		break;
	case BAR_BIAS:
		on_bias_scroll(n_sb_code, n_pos);
	default:
		break;
	}
}

void ViewData::set_v_bar_mode(const int b_mode)
{
	if (b_mode == BAR_BIAS)
		m_v_bar_mode_ = b_mode;
	else
		m_v_bar_mode_ = BAR_GAIN;
	update_bias_scroll();
}

void ViewData::update_gain_scroll()
{
	scroll_y_.SetScrollPos(
		MulDiv(
			chart_data.get_channel_list_item(m_channel_selected)->get_y_extent(),
			100,
			Y_EXTENT_MAX)
		+ 50,
		TRUE);
}

void ViewData::on_gain_scroll(const UINT n_sb_code, const UINT n_pos)
{
	int y_extent = chart_data.get_channel_list_item(m_channel_selected)->get_y_extent();
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
	default: break;
	}

	// change y extent
	if (y_extent > 0) //&& yExtent<=Y_EXTENT_MAX)
	{
		update_y_extent(m_channel_selected, y_extent);
		update_legends(UPD_ORDINATES | CHG_Y_SCALE);
	}
	// update scrollBar
	chart_data.Invalidate();
	update_gain_scroll();
}

void ViewData::update_bias_scroll()
{
	const CChanlistItem* p_chan_list_item = chart_data.get_channel_list_item(m_channel_selected);
	const auto i_pos = (p_chan_list_item->get_y_zero() - p_chan_list_item->get_data_bin_zero())
		* 100 / static_cast<int>(Y_ZERO_SPAN) + 50;
	scroll_y_.SetScrollPos(i_pos, TRUE);
	update_legends(UPD_ORDINATES | CHG_Y_SCALE);
}

void ViewData::on_bias_scroll(const UINT n_sb_code, const UINT n_pos)
{
	const CChanlistItem* p_chan_list_item = chart_data.get_channel_list_item(m_channel_selected);
	auto l_size = p_chan_list_item->get_y_zero() - p_chan_list_item->get_data_bin_zero();
	const auto y_extent = p_chan_list_item->get_y_extent();
	// get corresponding data
	switch (n_sb_code)
	{
	case SB_LEFT:		l_size = Y_ZERO_MIN; break;
	case SB_LINELEFT:	l_size -= y_extent / 100 + 1; break;
	case SB_LINERIGHT:	l_size += y_extent / 100 + 1; break;
	case SB_PAGELEFT:	l_size -= y_extent / 10 + 1; break;
	case SB_PAGERIGHT:	l_size += y_extent / 10 + 1; break;
	case SB_RIGHT:		l_size = Y_ZERO_MAX; break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: l_size = (static_cast<int>(n_pos) - 50) * (Y_ZERO_SPAN / 100); break;
	default: break;
	}

	// try to read data with this new size
	if (l_size > Y_ZERO_MIN && l_size < Y_ZERO_MAX)
	{
		update_y_zero(m_channel_selected, l_size + p_chan_list_item->get_data_bin_zero());
	}
	// update scrollBar
	chart_data.Invalidate();
	update_bias_scroll();
}

void ViewData::on_center_curve()
{
	chart_data.center_chan(m_channel_selected);
	chart_data.Invalidate();

	const CChanlistItem* p_chan_list_item = chart_data.get_channel_list_item(m_channel_selected);
	const auto y_extent = p_chan_list_item->get_y_extent();
	update_y_extent(m_channel_selected, y_extent);
	const auto y_zero = p_chan_list_item->get_y_zero();
	update_y_zero(m_channel_selected, y_zero);
}

void ViewData::on_gain_adjust_curve()
{
	chart_data.max_gain_chan(m_channel_selected);
	chart_data.Invalidate();

	const CChanlistItem* p_chan_list_item = chart_data.get_channel_list_item(m_channel_selected);
	const auto y_extent = p_chan_list_item->get_y_extent();
	update_y_extent(m_channel_selected, y_extent);
	const auto y_zero = p_chan_list_item->get_y_zero();
	update_y_zero(m_channel_selected, y_zero);
	update_legends(CHG_Y_SCALE);
}

void ViewData::on_split_curves()
{
	const auto n_channels = chart_data.get_channel_list_size(); // nb of data channels
	const auto height = chart_data.get_rect_height(); // height of the display area
	const auto offset = height / n_channels; // height for each channel
	auto zero = (height - offset) / 2; // center first curve at

	// split display area
	int max, min;
	for (auto i = 0; i < n_channels; i++)
	{
		CChanlistItem* chan = chart_data.get_channel_list_item(i);
		chan->get_max_min(&max, &min);
		const auto extent = MulDiv(max - min + 1, 100 * n_channels, 100);
		const auto bias = MulDiv(zero, extent, height); // convert pixel into bins
		const auto i_zero = (max + min) / 2 - bias; // change bias
		chan->set_y_extent(extent);
		chan->set_y_zero(i_zero);
		zero -= offset; // update position of next curve
	}
	update_legends(CHG_Y_SCALE);
	chart_data.Invalidate();
}

void ViewData::on_file_scroll(const UINT n_sb_code, const UINT n_pos)
{
	auto b_result = FALSE;
	// get corresponding data
	switch (n_sb_code)
	{
	case SB_LEFT: // scroll to the start
	case SB_LINELEFT: // scroll one line left
	case SB_LINERIGHT: // scroll one line right
	case SB_PAGELEFT: // scroll one page left
	case SB_PAGERIGHT: // scroll one page right
	case SB_RIGHT: // scroll to end right
		b_result = chart_data.scroll_data_from_doc(static_cast<WORD>(n_sb_code));
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		b_result = chart_data.get_data_from_doc(
			(static_cast<long>(n_pos) * m_p_dat_->get_doc_channel_length()) / 100L);
		break;
	default: // NOP: set position only
		break;
	}

	// adjust display
	if (b_result)
	{
		update_legends(UPD_ABSCISSA);
		UpdateData(FALSE); 
		chart_data.Invalidate();
	}
	update_file_scroll();
}

void ViewData::OnHScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar* p_scroll_bar)
{
	// form_view scroll: if pointer null
	if (p_scroll_bar == nullptr)
	{
		ViewDbTable::OnHScroll(n_sb_code, n_pos, p_scroll_bar);
		return;
	}

	// trap messages from ScrollBarEx
	int l_first;
	int l_last;
	CString cs;
	switch (n_sb_code)
	{
	case SB_THUMBTRACK:
		file_scroll_bar_.GetScrollInfo(&file_scroll_bar_infos_, SIF_ALL);
		l_first = file_scroll_bar_infos_.nPos;
		l_last = l_first + static_cast<int>(file_scroll_bar_infos_.nPage) - 1;
		m_time_first_abscissa = static_cast<float>(l_first) / m_sampling_rate_;
		m_time_last_abscissa = static_cast<float>(l_last) / m_sampling_rate_;
		chart_data.get_data_from_doc(l_first, l_last);
		chart_data.Invalidate();
		cs.Format(_T("%.3f"), m_time_first_abscissa);
		SetDlgItemText(IDC_TIMEFIRST, cs);
		cs.Format(_T("%.3f"), m_time_last_abscissa);
		SetDlgItemText(IDC_TIMELAST, cs);
		break;

	case SB_THUMBPOSITION:
		file_scroll_bar_.GetScrollInfo(&file_scroll_bar_infos_, SIF_ALL);
		l_first = file_scroll_bar_infos_.nPos;
		l_last =  l_first + static_cast<int>(file_scroll_bar_infos_.nPage) - 1;
		if (chart_data.get_data_from_doc(l_first, l_last))
		{
			update_legends(UPD_ABSCISSA);
			UpdateData(FALSE); // copy view object to controls
			chart_data.Invalidate();
		}

		break;
	default:
		on_file_scroll(n_sb_code, n_pos);
		break;
	}
}

void ViewData::measure_properties(const int item)
{
	// make sure that cursor is ok
	if (m_cursor_state_ != CURSOR_CROSS)
		on_my_message(NULL, MAKELPARAM(CURSOR_CROSS, HINT_SET_MOUSE_CURSOR));

	// save current data into data document
	switch (options_data_measures_->w_option)
	{
	case 0:
		m_p_dat_->get_vt_tags_list()->copy_tag_list(&chart_data.vt_tags);
		break;
	case 1:
		m_p_dat_->get_hz_tags_list()->copy_tag_list(&chart_data.hz_tags);
		break;
	case 3:
		options_data_measures_->w_stimulus_channel = static_cast<WORD>(chart_data.hz_tags.get_channel(0));
		options_data_measures_->w_stimulus_threshold = static_cast<WORD>(chart_data.hz_tags.get_value_int(0));
		break;
	default: break;
	}

	// call routine and pass parameters
	CMeasureProperties dlg(nullptr, item);
	dlg.m_pChartDataWnd = &chart_data;
	dlg.m_samplingrate = m_sampling_rate_;
	dlg.m_pdatDoc = m_p_dat_;
	dlg.m_pdbDoc = GetDocument();
	dlg.m_pMO = options_data_measures_;
	dlg.m_currentchan = m_channel_selected;

	dlg.DoModal();
	chart_data.Invalidate();
	update_controls_according_to_cursor_selected();
}

void ViewData::save_modified_file()
{
	// save previous file if anything has changed
	if (m_p_dat_ == nullptr)
		return;

	if (m_p_dat_->IsModified())
	{
		CString doc_name = GetDocument()->db_get_current_dat_file_name();
		m_p_dat_->save_document(doc_name);
	}
	m_p_dat_->SetModifiedFlag(FALSE);
}

void ViewData::adc_on_hardware_define_experiment()
{
	DlgdbEditRecord dlg;
	dlg.m_pdb_doc = GetDocument();

	if (IDOK == dlg.DoModal())
	{
		const auto p_dbwave_doc = GetDocument();
		const auto record_id = p_dbwave_doc->db_get_current_record_id();
		GetDocument()->update_all_views_db_wave(nullptr, HINT_DOC_HAS_CHANGED, nullptr);
		if(p_dbwave_doc->db_move_to_id(record_id))
		 p_dbwave_doc->update_all_views_db_wave(nullptr, HINT_DOC_MOVE_RECORD, nullptr);
	}
}

void ViewData::on_format_x_scale()
{
	// init dialog data
	DlgDataViewAbscissa dlg;
	dlg.m_first_abscissa = m_time_first_abscissa;
	dlg.m_last_abscissa = m_time_last_abscissa;
	dlg.m_very_last_abscissa = static_cast<float>(chart_data.get_document_last()) / m_sampling_rate_;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_time_first_abscissa = dlg.m_first_abscissa * dlg.m_abscissa_scale;
		m_time_last_abscissa = dlg.m_last_abscissa * dlg.m_abscissa_scale;
		chart_data.get_data_from_doc(static_cast<long>(m_time_first_abscissa * m_sampling_rate_),
		                              static_cast<long>(m_time_last_abscissa * m_sampling_rate_));
		update_legends(UPD_ABSCISSA | UPD_X_SCALE | CHG_X_BAR);
	}
}

void ViewData::compute_printer_page_size()
{
	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlg(FALSE);
	VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	const auto h_dc = dlg.CreatePrinterDC(); // to delete at the end -- see doc!
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

void ViewData::print_file_bottom_page(CDC* p_dc, const CPrintInfo* p_info)
{
	auto t = CTime::GetCurrentTime();
	TCHAR ch[256];
	wsprintf(ch, _T("  page %d:%d %d-%d-%d"), // %d:%d",
	         p_info->m_nCurPage, p_info->GetMaxPage(),
	         t.GetDay(), t.GetMonth(), t.GetYear());

	const auto cs_dat_file = GetDocument()->db_get_current_dat_file_name();
	const auto i_count = cs_dat_file.ReverseFind(_T('\\'));
	auto ch_date = cs_dat_file.Left(i_count);
	ch_date = ch_date.Left(ch_date.GetLength() - 1) + ch;
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_view_data_->horizontal_resolution / 2, options_view_data_->vertical_resolution - 57, ch_date);
}

CString ViewData::convert_file_index(const long l_first, const long l_last) const
{
	CString cs_unit = _T(" s"); 

	TCHAR sz_value[64]; 
	const auto psz_value = sz_value;
	float x_scale_factor; 
	auto x = NiceUnit::change_unit(static_cast<float>(l_first) / m_sampling_rate_, &cs_unit, &x_scale_factor);
	auto fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.)); 
	wsprintf(psz_value, _T("time = %i.%03.3i - "), static_cast<int>(x), fraction); 
	CString cs_comment = psz_value;

	x = static_cast<float>(l_last) / (m_sampling_rate_ * x_scale_factor);
	fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.));
	wsprintf(psz_value, _T("%i.%03.3i %s"), static_cast<int>(x), fraction, static_cast<LPCTSTR>(cs_unit));
	cs_comment += psz_value;
	return cs_comment;
}

BOOL ViewData::get_file_series_index_from_page(int page, int& file_number, long& l_first)
{
	// loop until we get all rows
	const auto total_rows = n_rows_per_page_ * (page - 1);
	l_first = l_print_first_;
	file_number = 0; // file list index
	if (options_view_data_->b_print_selection) // current file if selection only
		file_number = file_0_;
	else
		BOOL success = GetDocument()->db_move_first();

	auto very_last = l_print_first_ + l_print_len_;
	if (options_view_data_->b_complete_record)
		very_last = m_p_dat_->get_doc_channel_length() - 1;

	for (auto row = 0; row < total_rows; row++)
	{
		if (!print_get_next_row(file_number, l_first, very_last))
			break;
	}

	return TRUE;
}

CString ViewData::get_file_infos()
{
	CString str_comment; // scratch pad
	const CString tab(_T("    ")); // use 4 spaces as tabulation character
	const CString rc(_T("\n")); // next line

	// document's name, date and time
	const auto wave_format = m_p_dat_->get_wave_format();
	if (options_view_data_->b_doc_name || options_view_data_->b_acq_date_time) // print doc infos?
	{
		if (options_view_data_->b_doc_name) // print file name
		{
			str_comment += GetDocument()->db_get_current_dat_file_name() + tab;
		}
		if (options_view_data_->b_acq_date_time) // print data acquisition date & time
		{
			const auto date = wave_format->acquisition_time.Format(_T("%#d %B %Y %X")); //("%c");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_view_data_->b_acq_comment)
		str_comment += wave_format->get_comments(_T(" ")) + rc;

	return str_comment;
}

CString ViewData::print_bars(CDC* p_dc, const CRect* rect) const
{
	CString str_comment;
	const CString rc(_T("\n"));
	const CString tab(_T("     "));

	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
	TCHAR sz_value[64];
	const auto lpsz_val = sz_value;
	CString cs_unit;
	float x_scale_factor;
	CPoint bar_origin(-10, -10); // bar origin is at 10,10 pts of left bottom rectangle
	bar_origin.x += rect->left;
	bar_origin.y += rect->bottom;
	auto x_bar_end = bar_origin;
	auto y_bar_end = bar_origin;

	// same len ratio as displayed on view_data
	const auto horizontal_bar = chart_data.x_ruler.get_scale_unit_pixels(chart_data.get_rect_width());
	ASSERT(horizontal_bar > 0);
	const auto vert_bar = chart_data.y_ruler.get_scale_unit_pixels(chart_data.get_rect_height());
	ASSERT(vert_bar > 0);

	auto cs_comment = convert_file_index(chart_data.get_data_first_index(), chart_data.get_data_last_index());
	if (options_view_data_->b_time_scale_bar)
	{
		// print horizontal bar
		x_bar_end.x += horizontal_bar;
		p_dc->MoveTo(bar_origin);
		p_dc->LineTo(x_bar_end);

		// read text from control edit
		CString cs;
		cs.Format(_T(" bar= %g"), chart_data.x_ruler.get_scale_increment());
		cs_comment += cs;
		str_comment += cs_comment + rc;
	}

	if (options_view_data_->b_voltage_scale_bar)
	{
		y_bar_end.y -= vert_bar;
		p_dc->MoveTo(bar_origin);
		p_dc->LineTo(y_bar_end);
	}

	// comments, bar value and chan settings for each channel
	if (options_view_data_->b_channel_comment || options_view_data_->b_voltage_scale_bar || options_view_data_->b_channel_settings)
	{
		const auto channels_list_size = chart_data.get_channel_list_size(); 
		for (auto i_chan = 0; i_chan < channels_list_size; i_chan++) // loop
		{
			wsprintf(lpsz_val, _T("chan#%i "), i_chan); 
			cs_comment = lpsz_val;
			if (options_view_data_->b_voltage_scale_bar) 
			{
				cs_unit = _T(" V"); 
				auto z = static_cast<float>(chart_data.get_rect_height()) / 5
					* chart_data.get_channel_list_volts_per_pixel(i_chan);
				const auto x = NiceUnit::change_unit(z, &cs_unit, &x_scale_factor); 

				// approximate
				auto j = static_cast<int>(x);
				if ((static_cast<double>(x) - j) > 0.5) // increment integer if diff > 0.5
					j++;
				auto k = NiceUnit::nice_unit(x); // compare with nice unit abs
				if (j > 750) // there is a gap between 500 and 1000
					k = 1000;
				if (MulDiv(100, abs(k - j), j) <= 1) // keep nice unit if difference is less= than 1 %
					j = k;
				if (k >= 1000)
				{
					z = static_cast<float>(k) * x_scale_factor;
					j = static_cast<int>(NiceUnit::change_unit(z, &cs_unit, &x_scale_factor)); // convert
				}
				wsprintf(sz_value, _T("bar = %i %s "), j, static_cast<LPCTSTR>(cs_unit)); // store value into comment
				cs_comment += sz_value;
			}
			str_comment += cs_comment;

			// print chan comment
			if (options_view_data_->b_channel_comment)
			{
				str_comment += tab;
				str_comment += chart_data.get_channel_list_item(i_chan)->get_comment();
			}
			str_comment += rc;

			// print amplifiers settings (gain & filter), next line
			if (options_view_data_->b_channel_settings)
			{
				CString cs;
				const WORD chan_count = static_cast<WORD>(chart_data.get_channel_list_item(i_chan)->get_source_chan());
				const auto channels_array = m_p_dat_->get_wave_channels_array();
				const auto p_chan = channels_array->get_p_channel(chan_count);
				cs.Format(_T("headstage=%s gain=%.0f  filter= %s - %i Hz"), (LPCTSTR)p_chan->am_csheadstage,
				          p_chan->am_gaintotal, (LPCTSTR)p_chan->am_csInputpos, p_chan->am_lowpass);
				str_comment += cs;
				str_comment += rc;
			}
		}
	}
	p_dc->SelectObject(p_old_brush);
	return str_comment;
}

BOOL ViewData::OnPreparePrinting(CPrintInfo* p_info)
{
	// printing margins
	if (options_view_data_->vertical_resolution <= 0 // vertical resolution defined ?
		|| options_view_data_->horizontal_resolution <= 0 // horizontal resolution defined?
		|| options_view_data_->horizontal_resolution != p_info->m_rectDraw.Width() // same as infos provided
		|| options_view_data_->vertical_resolution != p_info->m_rectDraw.Height()) // by caller?
		compute_printer_page_size();

	auto pages_count = print_get_n_pages();
	p_info->SetMaxPage(pages_count); //one-page printing/preview
	p_info->m_nNumPreviewPages = 1; // preview 1 pages at a time
	p_info->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (options_view_data_->b_print_selection)
		p_info->m_pPD->m_pd.Flags |= PD_SELECTION; // set button to selection

	if (!DoPreparePrinting(p_info))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, p_info))
		return FALSE;

	if (options_view_data_->b_print_selection != p_info->m_pPD->PrintSelection())
	{
		options_view_data_->b_print_selection = p_info->m_pPD->PrintSelection();
		pages_count = print_get_n_pages();
		p_info->SetMaxPage(pages_count);
	}

	return TRUE;
}

int ViewData::print_get_n_pages()
{
	// how many rows per page?
	const auto size_row = options_view_data_->height_doc + options_view_data_->height_separator;
	n_rows_per_page_ = print_rect_.Height() / size_row;
	if (n_rows_per_page_ == 0) // prevent zero pages
		n_rows_per_page_ = 1;

	int total_rows; // number of rectangles -- or nb of rows
	const auto p_dbwave_doc = GetDocument();

	// compute number of rows according to b_multi_row & b_entire_record flag
	l_print_first_ = chart_data.get_data_first_index();
	l_print_len_ = chart_data.get_data_last_index() - l_print_first_ + 1;
	file_0_ = GetDocument()->db_get_current_record_position();
	ASSERT(file_0_ >= 0);
	files_count_ = 1;
	auto file0 = file_0_;
	auto file1 = file_0_;
	if (!options_view_data_->b_print_selection)
	{
		file0 = 0;
		files_count_ = p_dbwave_doc->db_get_records_count();
		file1 = files_count_;
	}

	// one row per file
	if (!options_view_data_->b_multiple_rows || !options_view_data_->b_complete_record)
		total_rows = files_count_;

	// multiple rows per file
	else
	{
		total_rows = 0;
		for (auto i = file0; i < file1; i++, p_dbwave_doc->db_move_next())
		{
			const BOOL success = p_dbwave_doc->db_set_current_record_position(i);
			if (!success)
				continue;
			auto len = p_dbwave_doc->db_get_data_len();
			if (len <= 0)
			{
				p_dbwave_doc->open_current_data_file();
				len = m_p_dat_->get_doc_channel_length();
				p_dbwave_doc->db_set_data_len(len);
			}
			len -= l_print_first_;
			auto row_count = len / l_print_len_; // how many rows for this file?
			if (len > row_count * l_print_len_) 
				row_count++;
			total_rows += static_cast<int>(row_count);
		}
	}

	if (file_0_ >= 0)
	{
		const BOOL success = p_dbwave_doc->db_set_current_record_position(file_0_);
		if (success) 
			p_dbwave_doc->open_current_data_file();
	}

	int pages_count = total_rows / n_rows_per_page_;
	if (total_rows > n_rows_per_page_ * pages_count)
		pages_count++;

	return pages_count;
}

void ViewData::OnBeginPrinting(CDC* p_dc, CPrintInfo* p_info)
{
	is_printing_ = TRUE;
	l_first_0_ = chart_data.get_data_first_index();
	l_last0_ = chart_data.get_data_last_index();
	pixels_count_0_ = chart_data.get_rect_width();

	//---------------------init objects-------------------------------------
	memset(&log_font_, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(log_font_.lfFaceName, _T("Arial")); // Arial font
	log_font_.lfHeight = options_view_data_->font_size; // font height
	p_old_font_ = nullptr;
	/*BOOL flag = */
	font_print_.CreateFontIndirect(&log_font_);
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewData::OnPrint(CDC* p_dc, CPrintInfo* p_info)
{
	p_old_font_ = p_dc->SelectObject(&font_print_);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	const auto r_width = options_view_data_->width_doc; // margins
	const auto r_height = options_view_data_->height_doc; // margins
	CRect r_where(print_rect_.left, // printing rectangle for data
	              print_rect_.top,
	              print_rect_.left + r_width,
	              print_rect_.top + r_height);
	//CRect RW2 = RWhere;									// printing rectangle - constant
	//RW2.OffsetRect(-RWhere.left, -RWhere.top);			// set RW2 origin = 0,0

	p_dc->SetMapMode(MM_TEXT); // change map mode to text (1 pixel = 1 logical point)
	print_file_bottom_page(p_dc, p_info); // print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int file_number; // file number and file index
	long l_first; // index first data point / first file
	auto very_last = l_print_first_ + l_print_len_; // index last data point / current file
	const int current_page = static_cast<int>(p_info->m_nCurPage); // get current page number
	get_file_series_index_from_page(current_page, file_number, l_first);
	if (l_first < GetDocument()->db_get_data_len() - 1)
		update_file_parameters();
	if (options_view_data_->b_complete_record)
		very_last = GetDocument()->db_get_data_len() - 1;

	options_scope_struct old_scope_structure;
	options_scope_struct* scope_structure = chart_data.get_scope_parameters();
	old_scope_structure = *scope_structure;
	scope_structure->b_draw_frame = options_view_data_->b_frame_rect;
	scope_structure->b_clip_rect = options_view_data_->b_clip_rect;

	// loop through all files	--------------------------------------------------------
	const int old_dc = p_dc->SaveDC(); // save DC
	for (auto i = 0; i < n_rows_per_page_; i++)
	{
		// first : set rectangle where data will be printed
		auto comment_rect = r_where; // save RWhere for comments
		p_dc->SetMapMode(MM_TEXT); // 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); // set text align mode

		// load data and adjust display rectangle ----------------------------------------
		// reduce width to the size of the data
		auto l_last = l_first + l_print_len_; // compute last pt to load
		if (l_first < GetDocument()->db_get_data_len() - 1)
		{
			if (l_last > very_last) // check end across file length
				l_last = very_last;
			chart_data.get_data_from_doc(l_first, l_last); // load data from file
			update_channels_display_parameters();
			chart_data.print(p_dc, &r_where); // print data
		}

		// update display rectangle for next row
		r_where.OffsetRect(0, r_height + options_view_data_->height_separator);

		// restore DC and print comments --------------------------------------------------
		p_dc->SetMapMode(MM_TEXT); // 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr); // no more clipping
		p_dc->SetViewportOrg(0, 0); // org = 0,0

		// print comments according to row within file
		CString cs_comment;
		if (l_first == l_print_first_) // first row = full comment
		{
			cs_comment += get_file_infos();
			cs_comment += print_bars(p_dc, &comment_rect); // bars and bar legends
		}
		else // other rows: time intervals only
			cs_comment = convert_file_index(chart_data.get_data_first_index(), chart_data.get_data_last_index());

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_view_data_->text_separator + comment_rect.Width(), 0);
		comment_rect.right = print_rect_.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect,
		               DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
		p_dc->SetTextAlign(ui_flag);

		// update file parameters for next row --------------------------------------------
		const auto i_file = file_number;
		if (!print_get_next_row(file_number, l_first, very_last))
		{
			//i = m_nb_rows_per_page_;
			break;
		}
		if (i_file != file_number)
			update_file_parameters(FALSE);
	}
	p_dc->RestoreDC(old_dc); // restore Display context

	// end of file loop : restore initial conditions
	if (p_old_font_ != nullptr)
		p_dc->SelectObject(p_old_font_);
	*scope_structure = old_scope_structure;
}

BOOL ViewData::print_get_next_row(int& file_number, long& l_first, long& very_last)
{
	if (!options_view_data_->b_multiple_rows || !options_view_data_->b_complete_record)
	{
		file_number++;
		if (file_number >= files_count_)
			return FALSE;

		const BOOL success = GetDocument()->db_move_next();
		if (success && l_first < GetDocument()->db_get_data_len() - 1)
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
			file_number++; // next index
			if (file_number >= files_count_) // last file ??
				return FALSE;

			const BOOL success = GetDocument()->db_move_next();
			if (success)
			{
				very_last = GetDocument()->db_get_data_len() - 1;
				l_first = l_print_first_;
			}
		}
	}
	return TRUE;
}

void ViewData::OnEndPrinting(CDC* p_dc, CPrintInfo* p_info)
{
	font_print_.DeleteObject();
	is_printing_ = FALSE;
	if (GetDocument()->db_set_current_record_position(file_0_))
	{
		chart_data.resize_channels(pixels_count_0_, 0);
		chart_data.get_data_from_doc(l_first_0_, l_last0_);
		update_file_parameters();
	}
}

void ViewData::on_en_change_time_first()
{
	if (mm_time_first_abscissa.m_b_entry_done)
	{
		mm_time_first_abscissa.on_en_change(this, m_time_first_abscissa, 1.f, -1.f);
		chart_data.get_data_from_doc(static_cast<long>(m_time_first_abscissa * m_sampling_rate_),
		                              static_cast<long>(m_time_last_abscissa * m_sampling_rate_));
		update_legends(UPD_ABSCISSA | CHG_X_SCALE);
		chart_data.Invalidate();
	}
}

void ViewData::on_en_change_time_last()
{
	if (mm_time_last_abscissa.m_b_entry_done)
	{
		mm_time_last_abscissa.on_en_change(this, m_time_last_abscissa, 1.f, -1.f);
		chart_data.get_data_from_doc(static_cast<long>(m_time_first_abscissa * m_sampling_rate_),
		                              static_cast<long>(m_time_last_abscissa * m_sampling_rate_));
		update_legends(UPD_ABSCISSA | CHG_X_SCALE);
		chart_data.Invalidate();
	}
}

void ViewData::update_file_scroll()
{
	file_scroll_bar_infos_.fMask = SIF_ALL; // | SIF_PAGE | SIF_POS;
	file_scroll_bar_infos_.nMin = 0;
	file_scroll_bar_infos_.nMax = GetDocument()->db_get_data_len();
	file_scroll_bar_infos_.nPos = chart_data.get_data_first_index();
	file_scroll_bar_infos_.nPage = chart_data.get_data_last_index() - chart_data.get_data_first_index() + 1;
	file_scroll_bar_.SetScrollInfo(&file_scroll_bar_infos_);
}

void ViewData::on_cbn_sel_change_combo_chan()
{
	const auto i_chan = m_combo_select_chan.GetCurSel();
	if (i_chan < chart_data.get_channel_list_size())
	{
		b_common_scale_ = FALSE;
		update_channel(i_chan);
	}
	else
	{
		b_common_scale_ = TRUE;
		m_channel_selected = 0;
		const CChanlistItem* p_chan = chart_data.get_channel_list_item(0);
		const auto y_extent = p_chan->get_y_extent();
		update_y_extent(0, y_extent);
		const auto y_zero = p_chan->get_y_zero();
		update_y_zero(0, y_zero);
	}
}

void ViewData::update_y_extent(const int i_chan, const int y_extent)
{
	CChanlistItem* p_chan = chart_data.get_channel_list_item(i_chan);
	p_chan->set_y_extent(y_extent);
	if (m_combo_select_chan.GetCurSel() == chart_data.get_channel_list_size())
	{
		const auto y_volts_extent = p_chan->get_volts_per_bin() * static_cast<float>(y_extent);
		chart_data.set_channel_list_volts_extent(-1, &y_volts_extent);
	}
}

void ViewData::update_y_zero(const int i_chan, const int y_bias)
{
	CChanlistItem* chan = chart_data.get_channel_list_item(i_chan);
	chan->set_y_zero(y_bias);
	if (m_combo_select_chan.GetCurSel() == chart_data.get_channel_list_size())
	{
		const auto y_volts_extent = chan->get_volts_per_bin() * static_cast<float>(y_bias);
		chart_data.set_channel_list_volts_zero(-1, &y_volts_extent);
	}
}
