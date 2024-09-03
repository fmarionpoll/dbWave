#include "StdAfx.h"
#include "ViewSpikes.h"

#include <strsafe.h>
#include <cmath>
#include "dbWave.h"
#include "DlgCopyAs.h"
#include "DlgSpikeEdit.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewSpikes, ViewDbTable)

ViewSpikes::ViewSpikes() : ViewDbTable(IDD)
{
	m_bEnableActiveAccessibility = FALSE; 
}

ViewSpikes::~ViewSpikes()
{
	spk_classification_parameters_->v_dest_class = m_class_destination;
	spk_classification_parameters_->v_source_class = m_class_source;
	spk_classification_parameters_->b_reset_zoom = m_b_reset_zoom;
	spk_classification_parameters_->f_jitter_ms = m_jitter_ms;
}

void ViewSpikes::DoDataExchange(CDataExchange* p_dx)
{
	ViewDbTable::DoDataExchange(p_dx);

	DDX_Text(p_dx, IDC_TIMEFIRST, m_time_first);
	DDX_Text(p_dx, IDC_TIMELAST, m_time_last);
	DDX_Text(p_dx, IDC_NSPIKES, m_spike_index);
	DDX_Text(p_dx, IDC_SPIKE_CLASS, m_spike_class);
	DDX_Text(p_dx, IDC_EDIT3, m_zoom);
	DDX_Text(p_dx, IDC_EDIT4, m_class_source);
	DDX_Text(p_dx, IDC_EDIT5, m_class_destination);
	DDX_Check(p_dx, IDC_CHECK1, m_b_reset_zoom);
	DDX_Check(p_dx, IDC_ARTEFACT, m_b_artefact);
	DDX_Text(p_dx, IDC_JITTER, m_jitter_ms);
	DDX_Control(p_dx, IDC_TAB1, spk_list_tab_ctrl);
	DDX_Check(p_dx, IDC_SAMECLASS, m_b_keep_same_class);
	DDX_Control(p_dx, IDC_ZOOM_ON_OFF, set_zoom);
}

BEGIN_MESSAGE_MAP(ViewSpikes, ViewDbTable)

	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()

	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikes::on_my_message)

	ON_COMMAND(ID_FORMAT_VIEW_ALL_DATA_ON_ABSCISSA, &ViewSpikes::on_format_all_data)
	ON_COMMAND(ID_FORMAT_CENTER_CURVE, &ViewSpikes::on_format_center_curve)
	ON_COMMAND(ID_FORMAT_GAIN_ADJUST, &ViewSpikes::on_format_gain_adjust)
	ON_COMMAND(ID_TOOLS_EDIT_SPIKES, &ViewSpikes::on_tools_edit_spikes)
	ON_COMMAND(ID_EDIT_COPY, &ViewSpikes::on_edit_copy)
	ON_COMMAND(ID_FORMAT_PREVIOUS_FRAME, &ViewSpikes::on_format_previous_frame)
	ON_COMMAND(ID_FORMAT_NEXT_FRAME, &ViewSpikes::on_format_next_frame)
	ON_COMMAND(ID_RECORD_SHIFT_LEFT, &ViewSpikes::on_h_scroll_left)
	ON_COMMAND(ID_RECORD_SHIFT_RIGHT, &ViewSpikes::on_h_scroll_right)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_EN_CHANGE(IDC_NSPIKES, &ViewSpikes::on_en_change_no_spike)
	ON_EN_CHANGE(IDC_SPIKE_CLASS, &ViewSpikes::on_en_change_spike_class)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewSpikes::on_en_change_time_first)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewSpikes::on_en_change_time_last)
	ON_EN_CHANGE(IDC_EDIT3, &ViewSpikes::on_en_change_zoom)
	ON_EN_CHANGE(IDC_EDIT4, &ViewSpikes::on_en_change_source_class)
	ON_EN_CHANGE(IDC_EDIT5, &ViewSpikes::on_en_change_dest_class)
	ON_EN_CHANGE(IDC_JITTER, &ViewSpikes::on_en_change_jitter)

	ON_BN_CLICKED(IDC_GAIN_button, &ViewSpikes::on_gain_button)
	ON_BN_CLICKED(IDC_BIAS_button, &ViewSpikes::on_bias_button)
	ON_BN_CLICKED(IDC_ARTEFACT, &ViewSpikes::on_artefact)

	ON_BN_CLICKED(IDC_SAMECLASS, &ViewSpikes::on_bn_clicked_same_class)
	ON_BN_CLICKED(IDC_ZOOM_ON_OFF, &ViewSpikes::on_zoom)
END_MESSAGE_MAP()

void ViewSpikes::OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactivate_view)
{
	if (b_activate)
	{
		const auto p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATE_VIEW, reinterpret_cast<LPARAM>(p_activate_view->GetDocument()));
	}
	else
	{
		const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		if (this != p_activate_view && this == p_deactivate_view)
		{
			save_current_spk_file();
			// save column parameters
			spk_classification_parameters_->col_separator = spike_class_listbox_.get_columns_separator_width();
			spk_classification_parameters_->row_height = spike_class_listbox_.get_row_height();
			spk_classification_parameters_->col_spikes = spike_class_listbox_.get_columns_spikes_width();
			spk_classification_parameters_->col_text = spike_class_listbox_.get_columns_text_width();

			if (p_app->m_p_view_spikes_memory_file == nullptr)
			{
				p_app->m_p_view_spikes_memory_file = new CMemFile;
				ASSERT(p_app->m_p_view_spikes_memory_file != NULL);
			}

			CArchive ar(p_app->m_p_view_spikes_memory_file, CArchive::store);
			p_app->m_p_view_spikes_memory_file->SeekToBegin();
			chart_data_wnd_.Serialize(ar);
			ar.Close();
		}

		p_app->options_view_data.view_data = *(chart_data_wnd_.get_scope_parameters());
	}
	ViewDbTable::OnActivateView(b_activate, p_activate_view, p_deactivate_view);
}

BOOL ViewSpikes::OnMove(UINT n_id_move_command)
{
	save_current_spk_file();
	return ViewDbTable::OnMove(n_id_move_command);
}

void ViewSpikes::OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint)
{
	if (b_init_)
	{
		switch (LOWORD(l_hint))
		{
		case HINT_DOC_HAS_CHANGED:
		case HINT_DOC_MOVE_RECORD:
			update_file_parameters(TRUE);
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

void ViewSpikes::set_add_spikes_mode(int mouse_cursor_type)
{
	// display or hide corresponding controls within this view
	const boolean set_add_spike_mode = (mouse_cursor_type == CURSOR_CROSS);
	if (b_add_spike_mode_ == set_add_spike_mode)
		return;
	
	b_add_spike_mode_ = set_add_spike_mode;
	const int n_cmd_show = set_add_spike_mode ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_SOURCE_CLASS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_DESTCLASS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT4)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT5)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_GROUPBOX)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_JITTER)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_JITTERSTATIC)->ShowWindow(n_cmd_show);

	auto h_wnd = GetSafeHwnd();
	if (!b_add_spike_mode_)
		h_wnd = nullptr;
	chart_data_wnd_.reflect_mouse_move_message(h_wnd);
	spike_class_listbox_.reflect_bar_mouse_move_message(h_wnd);
	chart_data_wnd_.set_track_spike(b_add_spike_mode_, spk_detection_parameters_->extract_n_points, spk_detection_parameters_->detect_pre_threshold,
	                             spk_detection_parameters_->extract_channel);

	if (b_add_spike_mode_)
		set_track_rectangle();
}

void ViewSpikes::set_track_rectangle()
{
	CRect rect0, rect1, rect2;
	GetWindowRect(&rect0);
	chart_data_wnd_.GetWindowRect(&rect1);
	spike_class_listbox_.GetWindowRect(&rect2);
	rect_vt_track_.top = rect1.top - rect0.top;
	rect_vt_track_.bottom = rect2.bottom - rect0.top;
	rect_vt_track_.left = rect1.left - rect0.left;
	rect_vt_track_.right = rect1.right - rect0.left;
}

void ViewSpikes::OnMouseMove(UINT n_flags, CPoint point)
{
	if (b_add_spike_mode_)
	{
		if (b_dummy_ && rect_vt_track_.PtInRect(point))
			pt_vt_ = point.x - rect_vt_track_.left;
		else
			pt_vt_ = -1;
		b_dummy_ = TRUE;
		chart_data_wnd_.xor_temp_vt_tag(pt_vt_);
		spike_class_listbox_.xor_temp_vt_tag(pt_vt_);
	}
	ViewDbTable::OnMouseMove(n_flags, point);
}

void ViewSpikes::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	if (rect_vt_track_.PtInRect(point))
	{
		ReleaseCapture();
		const int ii_time = chart_data_wnd_.get_data_offset_from_pixel(point.x - rect_vt_track_.left);
		jitter_ = m_jitter_ms;
		auto b_check = TRUE;
		if (n_flags & MK_CONTROL)
			b_check = FALSE;
		add_spike_to_list(ii_time, b_check);
		b_dummy_ = FALSE;
	}
	ViewDbTable::OnLButtonUp(n_flags, point);
}

void ViewSpikes::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	if (rect_vt_track_.PtInRect(point))
		SetCapture();
	ViewDbTable::OnLButtonDown(n_flags, point);
}

void ViewSpikes::set_mouse_cursor(const short param_value)
{
	if (chart_data_wnd_.get_mouse_cursor_type() != param_value)
		set_add_spikes_mode(param_value);

	chart_data_wnd_.set_mouse_cursor_type(param_value);
	int old_cursor = spike_class_listbox_.set_mouse_cursor_type(param_value);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(param_value, 0));
}

void ViewSpikes::change_zoom(LPARAM l_param)
{
	if (HIWORD(l_param) == IDC_DISPLAY_DAT)
	{
		l_first_ = chart_data_wnd_.get_data_first_index();
		l_last_ = chart_data_wnd_.get_data_last_index();
	}
	else if (HIWORD(l_param) == IDC_LISTCLASSES) //TODO [does not work! HIWORD(lParam)==1]
	{
		l_first_ = spike_class_listbox_.get_time_first();
		l_last_ = spike_class_listbox_.get_time_last();
	}
	update_legends(TRUE);
}

LRESULT ViewSpikes::on_my_message(const WPARAM w_param, const LPARAM l_param)
{
	short param_value = LOWORD(l_param);
	switch (w_param)
	{
	case HINT_SET_MOUSE_CURSOR:
		set_mouse_cursor(param_value);
		break;

	case HINT_SELECT_SPIKES:
		chart_data_wnd_.Invalidate();
		spike_class_listbox_.Invalidate();
		break;

	case HINT_CHANGE_HZ_LIMITS:
	case HINT_CHANGE_ZOOM:
	case HINT_VIEW_SIZE_CHANGED:
		change_zoom(l_param);
		break;

	case HINT_HIT_SPIKE:
		{
			db_spike spike_hit = GetDocument()->get_spike_hit();
			select_spike(spike_hit);
		}
		break;

	case HINT_DBL_CLK_SEL:
		if (param_value < 0)
			param_value = 0;
		m_spike_index = param_value;
		on_tools_edit_spikes();
		break;

	case HINT_DROPPED:
		p_spk_doc->SetModifiedFlag();
		{
			const int old_class = m_spike_class;
			m_spike_class = p_spk_list->get_spike(m_spike_index)->get_class_id();
			if (old_class != m_spike_class)
			{
				chart_data_wnd_.Invalidate();
				spike_class_listbox_.Invalidate();
			}
		}
		UpdateData(FALSE);
		break;

	case HINT_WINDOW_PROPS_CHANGED:
		options_view_data_->spk_view_data = *chart_data_wnd_.get_scope_parameters();
		break;

	case HINT_HIT_SPIKE_SHIFT:
		{
			db_spike spike_hit = GetDocument()->get_spike_hit();
			select_spike(spike_hit);
		}
		break;

	default:
		break;
	}
	return 0L;
}

BOOL ViewSpikes::add_spike_to_list(const long ii_time, const BOOL check_if_spike_nearby)
{
	const int method = p_spk_list->get_detection_parameters()->detect_transform;
	const int doc_channel = p_spk_list->get_detection_parameters()->extract_channel;
	const int pre_threshold = p_spk_list->get_detection_parameters()->detect_pre_threshold;
	const int spike_length = p_spk_list->get_spike_length();
	const int transformation_data_span = AcqDataDoc::get_transformed_data_span(method);
	const auto ii_time0 = ii_time - pre_threshold;
	auto l_read_write_first = ii_time0;
	auto l_read_write_last = ii_time0 + spike_length;
	
	// add a new spike if no spike is found around
	int spike_index = 0;
	auto is_found = FALSE;
	if (check_if_spike_nearby)
	{
		const auto jitter = static_cast<int>((p_spk_doc->get_acq_rate() * jitter_) / 1000);
		is_found = p_spk_list->is_any_spike_around(ii_time0 + pre_threshold, jitter, spike_index, doc_channel);
	}

	if (!is_found)
	{
		if (!p_data_doc_->load_raw_data(&l_read_write_first, &l_read_write_last, transformation_data_span))
			return FALSE;

		p_data_doc_->load_transformed_data(l_read_write_first, l_read_write_last, method, doc_channel);
		const auto p_data_spike_0 = p_data_doc_->get_transformed_data_element(ii_time0 - l_read_write_first);

		spike_index = p_spk_list->add_spike(p_data_spike_0,	//lpSource	= buff pointer to the buffer to copy
		                                  1,				//nb of interleaved channels
		                                  ii_time0 + pre_threshold, //time = file index of first pt of the spk
		                                  doc_channel,		//detect_channel	= data source chan index
		                                  m_class_destination, check_if_spike_nearby);

		p_spk_doc->SetModifiedFlag();
	}
	else if (p_spk_list->get_spike(spike_index)->get_class_id() != m_class_destination)
	{
		p_spk_list->get_spike(spike_index)->set_class_id(m_class_destination);
		p_spk_doc->SetModifiedFlag();
	}

	// save the modified data into the spike file
	if (p_spk_doc->IsModified())
	{
		p_spk_list->update_class_list();
		p_spk_doc->OnSaveDocument(GetDocument()->db_get_current_spk_file_name(FALSE));
		p_spk_doc->SetModifiedFlag(FALSE);
		GetDocument()->set_db_n_spikes(p_spk_list->get_spikes_count());
		GetDocument()->set_db_n_spike_classes(p_spk_list->get_classes_count());
		const auto b_reset_zoom_old = m_b_reset_zoom;
		m_b_reset_zoom = FALSE;
		update_spike_file(TRUE);
		m_b_reset_zoom = b_reset_zoom_old;
	}
	m_spike_index = spike_index;

	update_data_file(TRUE);
	update_legends(TRUE);
	spike_class_listbox_.Invalidate();
	return TRUE;
}

void ViewSpikes::select_spike(db_spike& spike_selected)
{
	if (p_spk_doc == nullptr)
		return;

	if (spike_selected.spike_index >= p_spk_list->get_spikes_count())
		spike_selected.spike_index = -1;
	m_spike_index = spike_selected.spike_index;
	p_spk_list->m_selected_spike = spike_selected.spike_index;
	spike_class_listbox_.select_spike(spike_selected);

	m_spike_class = -1;
	int n_cmd_show;
	if (spike_selected.spike_index >= 0 && spike_selected.spike_index < p_spk_list->get_spikes_count())
	{
		const auto spike = p_spk_list->get_spike(m_spike_index);
		m_spike_class = spike->get_class_id();
		m_b_artefact = (m_spike_class < 0);
		const auto spk_first = spike->get_time() - p_spk_list->get_detection_parameters()->detect_pre_threshold;
		const auto spk_last = spk_first + p_spk_list->get_spike_length();
		n_cmd_show = SW_SHOW;
		if (p_data_doc_ != nullptr)
		{
			highlighted_intervals_.SetAt(3, spk_first);
			highlighted_intervals_.SetAt(4, spk_last);
			chart_data_wnd_.set_highlight_data(&highlighted_intervals_);
			chart_data_wnd_.Invalidate();
		}
	}
	else
	{
		n_cmd_show = SW_HIDE;
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	mm_spike_class_.ShowWindow(n_cmd_show);
	GetDlgItem(IDC_ARTEFACT)->ShowWindow(n_cmd_show);

	UpdateData(FALSE);
}

void ViewSpikes::define_sub_classed_items()
{
	// attach controls
	VERIFY(file_scrollbar_.SubclassDlgItem(IDC_FILESCROLL, this));
	file_scrollbar_.SetScrollRange(0, 100, FALSE);
	VERIFY(mm_spike_index_.SubclassDlgItem(IDC_NSPIKES, this));
	mm_spike_index_.ShowScrollBar(SB_VERT);
	VERIFY(mm_spike_class_.SubclassDlgItem(IDC_SPIKE_CLASS, this));
	mm_spike_class_.ShowScrollBar(SB_VERT);
	VERIFY(spike_class_listbox_.SubclassDlgItem(IDC_LISTCLASSES, this));
	VERIFY(mm_time_first_.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_time_last_.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(chart_data_wnd_.SubclassDlgItem(IDC_DISPLAY_DAT, this));
	VERIFY(mm_zoom_.SubclassDlgItem(IDC_EDIT3, this));
	mm_zoom_.ShowScrollBar(SB_VERT);
	VERIFY(mm_class_source_.SubclassDlgItem(IDC_EDIT4, this));
	mm_class_source_.ShowScrollBar(SB_VERT);
	VERIFY(mm_class_destination_.SubclassDlgItem(IDC_EDIT5, this));
	mm_class_destination_.ShowScrollBar(SB_VERT);
	VERIFY(mm_jitter_ms_.SubclassDlgItem(IDC_JITTER, this));

	// left scrollbar and button
	VERIFY(scrollbar_y_.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	scrollbar_y_.SetScrollRange(0, 100);
	h_bias_ = AfxGetApp()->LoadIcon(IDI_BIAS);
	h_zoom_ = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         (LPARAM)static_cast<HANDLE>(h_bias_));
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         (LPARAM)static_cast<HANDLE>(h_zoom_));
}

void ViewSpikes::define_stretch_parameters()
{
	stretch_.attach_parent(this);
	stretch_.new_prop(IDC_LISTCLASSES, XLEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);

	stretch_.new_prop(IDC_DISPLAY_DAT, XLEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_TIMEINTERVALS, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	stretch_.new_prop(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);

	stretch_.new_prop(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YTEQ);
	stretch_.new_prop(IDC_SCROLLY_scrollbar, SZEQ_XREQ, SZEQ_YTEQ);
}

void ViewSpikes::OnInitialUpdate()
{
	ViewDbTable::OnInitialUpdate();
	define_sub_classed_items();
	define_stretch_parameters();
	b_init_ = TRUE;
	m_auto_increment = true;
	m_auto_detect = true;

	const auto p_btn = static_cast<CButton*>(GetDlgItem(IDC_ZOOM_ON_OFF));
	p_btn->SetCheck(1);

	// load global parameters
	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	options_view_data_ = &(p_app->options_view_data); 
	options_view_data_measure_ = &(p_app->options_view_data_measure); 
	spk_classification_parameters_ = &(p_app->spk_classification); 

	m_class_destination = spk_classification_parameters_->v_dest_class;
	m_class_source = spk_classification_parameters_->v_source_class;
	m_b_reset_zoom = spk_classification_parameters_->b_reset_zoom;
	m_jitter_ms = spk_classification_parameters_->f_jitter_ms;

	// adjust size of the row and cols with text, spikes, and bars
	CRect rect;
	GetDlgItem(IDC_LISTCLASSES)->GetWindowRect(&rect);
	spike_class_listbox_.set_row_height(spk_classification_parameters_->row_height);
	CRect rect2;
	GetDlgItem(IDC_DISPLAY_DAT)->GetWindowRect(&rect2);
	const int left_col_width = rect2.left - rect.left - 2;
	spike_class_listbox_.set_left_column_width(left_col_width);
	if (spk_classification_parameters_->col_text < 0)
	{
		spk_classification_parameters_->col_spikes = spk_classification_parameters_->row_height;
		spk_classification_parameters_->col_text = left_col_width - 2 * spk_classification_parameters_->col_separator - spk_classification_parameters_->col_spikes;
		if (spk_classification_parameters_->col_text < 20)
		{
			const auto half = left_col_width - spk_classification_parameters_->col_separator;
			spk_classification_parameters_->col_spikes = half;
			spk_classification_parameters_->col_text = half;
		}
	}
	spk_classification_parameters_->col_text = left_col_width - spk_classification_parameters_->col_spikes - 2 * spk_classification_parameters_->col_separator;
	spike_class_listbox_.set_columns_width(spk_classification_parameters_->col_spikes, spk_classification_parameters_->col_separator);
	spike_class_listbox_.set_cursor_max_on_dbl_click(3);

	// init relation with document, display data, adjust parameters
	chart_data_wnd_.set_scope_parameters(&(options_view_data_->view_data));
	chart_data_wnd_.set_cursor_max_on_dbl_click(3);

	update_file_parameters(TRUE);
	if (b_add_spike_mode_)
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_CURSOR_MODE_MEASURE, NULL);
		chart_data_wnd_.set_track_spike(b_add_spike_mode_, spk_detection_parameters_->extract_n_points, spk_detection_parameters_->detect_pre_threshold,
		                             spk_detection_parameters_->extract_channel);
	}
}

void ViewSpikes::update_file_parameters(const BOOL b_update_interface)
{
	update_spike_file(b_update_interface);
	update_data_file(b_update_interface);
	update_legends(b_update_interface);
}

void ViewSpikes::update_data_file(const BOOL b_update_interface)
{
	p_data_doc_ = GetDocument()->open_current_data_file();
	if (p_data_doc_ == nullptr)
		return;

	chart_data_wnd_.set_b_use_dib(FALSE);
	chart_data_wnd_.attach_data_file(p_data_doc_);

	const auto detect = p_spk_list->get_detection_parameters();
	int source_data_view = detect->extract_channel;
	if (source_data_view >= p_data_doc_->get_wave_format()->scan_count)
	{
		detect->extract_channel = 0;
		source_data_view = 0;
	}
	if (detect->detect_what == DETECT_STIMULUS)
	{
		source_data_view = detect->detect_channel;
		if (source_data_view >= p_data_doc_->get_wave_format()->scan_count)
		{
			detect->detect_channel = 0;
			source_data_view = 0;
		}
	}

	// set detection channel
	if (chart_data_wnd_.set_channel_list_source_channel(0, source_data_view) < 0)
	{
		chart_data_wnd_.remove_all_channel_list_items();
	}
	else
	{
		chart_data_wnd_.resize_channels(chart_data_wnd_.get_rect_width(), l_last_ - l_first_);
		chart_data_wnd_.get_data_from_doc(l_first_, l_last_);

		if (b_init_source_view_)
		{
			b_init_source_view_ = FALSE;
			int max, min;
			CChanlistItem* chan = chart_data_wnd_.get_channel_list_item(0);
			chan->get_max_min(&max, &min);
			const auto extent = MulDiv(max - min + 1, 11, 10);
			const auto zero = (max + min) / 2;
			chan->set_y_extent(extent);
			chan->set_y_zero(zero);
		}
	}

	if (b_update_interface)
	{
		chart_data_wnd_.Invalidate();

		// adjust scroll bar (size of button and left/right limits)
		file_scroll_infos_.fMask = SIF_ALL;
		file_scroll_infos_.nMin = 0;
		file_scroll_infos_.nMax = p_data_doc_->get_doc_channel_length() - 1;
		file_scroll_infos_.nPos = 0;
		file_scroll_infos_.nPage = chart_data_wnd_.get_data_last_index() - chart_data_wnd_.get_data_first_index() + 1;
		file_scrollbar_.SetScrollInfo(&file_scroll_infos_);
	}

	highlighted_intervals_.SetSize(3 + 2);					// total size
	highlighted_intervals_.SetAt(0, 0);		// source channel
	highlighted_intervals_.SetAt(1, col_red);	
	highlighted_intervals_.SetAt(2, 1);		// pen size
	highlighted_intervals_.SetAt(3, 0);		// pen size
	highlighted_intervals_.SetAt(4, 0);		// pen size
}

void ViewSpikes::update_spike_file(BOOL b_update_interface)
{
	p_spk_doc = GetDocument()->open_current_spike_file();

	if (nullptr == p_spk_doc)
	{
		spike_class_listbox_.set_source_data(nullptr, nullptr);
	}
	else
	{
		p_spk_doc->SetModifiedFlag(FALSE);
		p_spk_doc->SetPathName(GetDocument()->db_get_current_spk_file_name(), FALSE);
		spk_list_tab_ctrl.init_ctrl_tab_from_spike_doc(p_spk_doc);

		const int current_index = GetDocument()->get_current_spike_file()->get_index_current_spike_list();
		p_spk_list = p_spk_doc->set_index_current_spike_list(current_index);
		spk_detection_parameters_ = p_spk_list->get_detection_parameters();

		spike_class_listbox_.set_source_data(p_spk_list, GetDocument());
		if (b_update_interface)
		{
			spk_list_tab_ctrl.SetCurSel(current_index);
			// adjust Y zoom
			ASSERT(l_first_ >= 0);
			if (m_b_reset_zoom)
			{
				spike_class_listbox_.SetRedraw(FALSE);
				zoom_on_preset_interval(0);
				spike_class_listbox_.SetRedraw(TRUE);
			}
			else if (l_last_ > p_spk_doc->get_acq_size() - 1 || l_last_ <= l_first_)
				l_last_ = p_spk_doc->get_acq_size() - 1; 

			spike_class_listbox_.set_time_intervals(l_first_, l_last_);
			adjust_y_zoom_to_max_min(false);
		}
	}

	// select row
	if (b_update_interface)
		spike_class_listbox_.SetCurSel(0);
}

void ViewSpikes::update_legends(const BOOL b_update_interface)
{
	if (!b_update_interface)
		return;

	if (l_first_ < 0)
		l_first_ = 0;
	if (l_last_ <= l_first_)
		l_last_ = l_first_ + 120;
	if (p_spk_doc != nullptr)
	{
		if (l_last_ >= p_spk_doc->get_acq_size())
			l_last_ = p_spk_doc->get_acq_size() - 1;
	}
	if (l_first_ > l_last_)
		l_first_ = l_last_ - 120;

	// set cursor
	auto h_safe_wnd = GetSafeHwnd();
	if (!b_add_spike_mode_)
		h_safe_wnd = nullptr;
	chart_data_wnd_.reflect_mouse_move_message(h_safe_wnd);
	spike_class_listbox_.reflect_bar_mouse_move_message(h_safe_wnd);
	chart_data_wnd_.set_track_spike(b_add_spike_mode_, spk_detection_parameters_->extract_n_points, spk_detection_parameters_->detect_pre_threshold,
	                             spk_detection_parameters_->extract_channel);

	// update spike bars & forms CListBox
	if (l_first_ != spike_class_listbox_.get_time_first()
		|| l_last_ != spike_class_listbox_.get_time_last())
		spike_class_listbox_.set_time_intervals(l_first_, l_last_);

	// update text abscissa and horizontal scroll position
	if (p_spk_doc != nullptr)
	{
		m_time_first = static_cast<float>(l_first_) / p_spk_doc->get_acq_rate();
		m_time_last = static_cast<float>(l_last_ + 1) / p_spk_doc->get_acq_rate();
	}
	chart_data_wnd_.get_data_from_doc(l_first_, l_last_);

	// update scrollbar and select spikes
	db_spike spike_selected(-1, -1, m_spike_index);
	select_spike(spike_selected);
	update_file_scroll();
}

void ViewSpikes::adjust_y_zoom_to_max_min(const BOOL b_force_search_max_min)
{
	if (y_we_ == 1 || b_force_search_max_min)
	{
		int max, min;
		p_spk_list->get_total_max_min(TRUE, &max, &min);
		y_we_ = MulDiv(max - min + 1, 10, 8);
		y_wo_ = (max + min) / 2;
	}
	spike_class_listbox_.set_y_zoom(y_we_, y_wo_);
}

void ViewSpikes::select_spike_list(int current_selection)
{
	p_spk_list = p_spk_doc->set_index_current_spike_list(current_selection);
	ASSERT(p_spk_list != NULL);

	spike_class_listbox_.set_spk_list(p_spk_list);

	spike_class_listbox_.Invalidate();
	spk_detection_parameters_ = p_spk_list->get_detection_parameters();

	// update source data: change data channel and update display
	int extract_channel = p_spk_list->get_detection_parameters()->extract_channel;
	ASSERT(extract_channel == spk_detection_parameters_->extract_channel);
	if (p_spk_list->get_detection_parameters()->detect_what == DETECT_STIMULUS)
		extract_channel = p_spk_list->get_detection_parameters()->detect_channel;

	// no data available
	if (chart_data_wnd_.set_channel_list_source_channel(0, extract_channel) < 0)
	{
		chart_data_wnd_.remove_all_channel_list_items();
	}
	// data are ok
	else
	{
		chart_data_wnd_.resize_channels(chart_data_wnd_.get_rect_width(), l_last_ - l_first_);
		chart_data_wnd_.get_data_from_doc(l_first_, l_last_);
		int max, min;
		CChanlistItem* chan = chart_data_wnd_.get_channel_list_item(0);
		chan->get_max_min(&max, &min);
		const auto extent = MulDiv(max - min + 1, 11, 10);
		const auto zero = (max + min) / 2;
		chan->set_y_extent(extent);
		chan->set_y_zero(zero);
	}
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::on_tools_edit_spikes()
{
	// return if no spike shape
	if (spike_class_listbox_.get_xw_extent() == 0) 
		return;
	// save time frame to restore it on return
	const auto l_first = spike_class_listbox_.get_time_first();
	const auto l_last = spike_class_listbox_.get_time_last();

	DlgSpikeEdit dlg;
	dlg.y_extent = spike_class_listbox_.get_yw_extent(); 
	dlg.y_zero = spike_class_listbox_.get_yw_org(); 
	dlg.x_extent = spike_class_listbox_.get_xw_extent(); 
	dlg.x_zero = spike_class_listbox_.get_xw_org();
	dlg.dlg_spike_index = m_spike_index; 
	dlg.db_wave_doc = GetDocument();
	dlg.m_parent = this;
	dlg.DoModal();

	if (!dlg.b_artefact)
		m_spike_index = dlg.dlg_spike_index;

	if (dlg.b_changed)
	{
		p_spk_doc->SetModifiedFlag(TRUE);
		save_current_spk_file();
		update_spike_file(TRUE);
	}
	l_first_ = l_first;
	l_last_ = l_last;
	update_data_file(TRUE);
	update_legends(TRUE);
	// display data
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::print_compute_page_size()
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

void ViewSpikes::print_file_bottom_page(CDC* p_dc, const CPrintInfo* p_info)
{
	const auto t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"), // %d:%d",
	          p_info->m_nCurPage, p_info->GetMaxPage(),
	          t.GetDay(), t.GetMonth(), t.GetYear());
	const auto ch_date = GetDocument()->db_get_current_spk_file_name();
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_view_data_->horizontal_resolution / 2, options_view_data_->vertical_resolution - 57, ch_date);
}

CString ViewSpikes::print_convert_file_index(const long l_first, const long l_last) const
{
	CString cs_unit = _T(" s");
	int constexpr array_size = 64;
	TCHAR sz_dest[array_size];
	constexpr size_t cb_dest = array_size * sizeof(TCHAR);

	float x_scale_factor;
	auto x = print_change_unit(
		static_cast<float>(l_first) / p_spk_doc->get_acq_rate(), &cs_unit, &x_scale_factor);
	auto fraction = static_cast<int>((x - floorf(x)) * static_cast<float>(1000.));
	HRESULT hr = StringCbPrintf(sz_dest, cb_dest, TEXT("time = %i.%03.3i - "), static_cast<int>(x), fraction);
	CString cs_comment = _T("");
	if (hr == S_OK)
		cs_comment += sz_dest;

	x = static_cast<float>(l_last) / (p_spk_doc->get_acq_rate() * x_scale_factor); 
	fraction = static_cast<int>((x - floorf(x)) * static_cast<float>(1000.));
	hr = StringCbPrintf(sz_dest, cb_dest, _T("%f.%03.3i %s"), floorf(x), fraction, static_cast<LPCTSTR>(cs_unit));
	if (hr == S_OK)
		cs_comment += sz_dest;

	return cs_comment;
}

long ViewSpikes::print_get_file_series_index_from_page(const int page, int* file_number)
{
	auto l_first = l_print_first_;

	const auto max_row = n_rows_per_page_ * page; 
	auto i_file = 0; 
	if (options_view_data_->b_print_selection)
		i_file = file_0_;

	if (GetDocument()->db_set_current_record_position(i_file)) {

		auto very_last = GetDocument()->db_get_data_len() - 1;
		for (auto row = 0; row < max_row; row++)
		{
			l_first += l_print_len_; // end of row
			if (l_first >= very_last) // next file ?
			{
				i_file++; // next file index
				if (i_file > files_count_) // last file ??
				{
					i_file--;
					break;
				}
				// update end-of-file
				if (GetDocument()->db_move_next()) {
					very_last = GetDocument()->db_get_data_len() - 1;
					l_first = l_print_first_;
				}
			}
		}
	}
	*file_number = i_file; // return index / file list
	
	return l_first; // return index first point / data file
}

CString ViewSpikes::print_get_file_infos()
{
	CString str_comment; // scratch pad
	const CString tab("    "); // use 4 spaces as tabulation character
	const CString rc("\n"); // next line
	const auto p_wave_format = &p_spk_doc->m_wave_format; // get data description

	// document's name, date and time
	if (options_view_data_->b_doc_name || options_view_data_->b_acq_date_time) // print doc infos?
	{
		if (options_view_data_->b_doc_name) // print file name
			str_comment += GetDocument()->db_get_current_spk_file_name(FALSE) + tab;
		if (options_view_data_->b_acq_date_time) // print data acquisition date & time
		{
			const auto acquisition_time = p_spk_doc->get_acq_time();
			const auto date = acquisition_time.Format(_T("%#d %m %Y %X")); //("%x %X");
			// or more explicitly %d-%b-%Y %H:%M:%S");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_view_data_->b_acq_comment)
	{
		str_comment += p_wave_format->get_comments(_T(" ")); // cs_comment
		str_comment += rc;
	}

	return str_comment;
}

CString ViewSpikes::print_bars(CDC* p_dc, const CRect* rect) const
{
	CString str_comment;
	const CString rc(_T("\n"));
	CString tab(_T("     "));
	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));

	CString cs_unit;
	CRect rect_horizontal_bar;
	CRect rect_vertical_bar;
	const CPoint bar_origin(-10, -10);
	const CSize bar_size(5, 5); 

	///// time abscissa ///////////////////////////
	const int ii_first = spike_class_listbox_.get_time_first();
	const int ii_last = spike_class_listbox_.get_time_last();
	auto cs_comment = print_convert_file_index(ii_first, ii_last);

	///// horizontal time bar ///////////////////////////
	if (options_view_data_->b_time_scale_bar)
	{
		constexpr auto horizontal_bar = 100;
		// print horizontal bar
		rect_horizontal_bar.left = rect->left + bar_origin.x;
		rect_horizontal_bar.right = rect_horizontal_bar.left + horizontal_bar;
		rect_horizontal_bar.top = rect->bottom - bar_origin.y;
		rect_horizontal_bar.bottom = rect_horizontal_bar.top - bar_size.cy;
		p_dc->Rectangle(&rect_horizontal_bar);
		//get time equivalent of bar length
		const auto ii_bar = MulDiv(ii_last - ii_first, rect_horizontal_bar.Width(), rect->Width());
		const auto x_bar = static_cast<float>(ii_bar) / p_spk_doc->get_acq_rate();
		CString cs;
		cs.Format(_T("\nbar = %f s"), x_bar);
		cs_comment += cs;
		str_comment += cs_comment + rc;
	}

	///// vertical voltage bars ///////////////////////////
	if (options_view_data_->b_voltage_scale_bar)
	{
		constexpr auto vertical_bar = 100;
		rect_vertical_bar.left = rect->left + bar_origin.x;
		rect_vertical_bar.right = rect_vertical_bar.left - bar_size.cx;
		rect_vertical_bar.bottom = rect->bottom - bar_origin.y;
		rect_vertical_bar.top = rect_vertical_bar.bottom - vertical_bar;
		p_dc->Rectangle(&rect_vertical_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_view_data_->b_channel_comment || options_view_data_->b_voltage_scale_bar || options_view_data_->b_channel_settings)
	{
		/*
				int imax = m_sourceView.get_channel_list_size();	// number of data channels
				for (int i_chan=0; i_chan< imax; i_chan++)		// loop
				{
					wsprintf(lpszVal, "chan#%i ", i_chan);	// channel number
					cs_comment = lpszVal;
					if (mdPM->bVoltageScaleBar)				// bar scale value
					{
						csUnit = " V";						// provisional unit
						float z= 	(float) m_y_scale_bar.GetRectHeight()* m_sourceView.Get_Chan_list_Volts_per_Pixel(i_chan);
						float x = PrintChangeUnit(z, &csUnit, &x_scale_factor); // convert

						// approximate
						int j = (int) x;					// get int value
						if (x-j > 0.5)						// increment integer if diff > 0.5
							j++;
						int k = (int) NiceUnit(x);			// compare with nice unit abs
						if (j > 750)                        // there is a gap between 500 and 1000
							k=1000;
						if (MulDiv(100, abs(k-j),j) <= 1)	// keep nice unit if difference is less= than 1 %
							j = k;
						if (k >= 1000)
						{
							z = (float) k * x_scale_factor;
							j = (int) PrintChangeUnit(z, &csUnit, &x_scale_factor); // convert
						}
						wsprintf(sz_value, "bar = %i %s ", j, csUnit);	// store value into comment
						cs_comment += sz_value;
					}
					str_comment += cs_comment;

					// print chan comment
					if (mdPM->b_Chans_Comment)
					{
						str_comment += tab;
						str_comment += m_sourceView.Get_Chan_list_Comment(i_chan);
					}
					str_comment += rc;

					// print amplifiers settings (gain & filter), next line
					if (mdPM->bChanSettings)
					{
						CString cs;
						WORD chan_nb = m_sourceView.Get_Chan_list_Source_Chan(i_chan);
						CWaveChan* pChan = m_pDataDoc->m_pDataFile->Get_p_Wave_chan_Array()->get_p_channel(chan_nb);
						wsprintf(lpszVal, "headstage=%s", pChan->head_stage);
						cs += lpszVal;
						wsprintf(lpszVal, " g=%li", (long) (pChan->x_gain));
						cs += lpszVal;
						wsprintf(lpszVal, " LP=%i", pChan->am_low_pass);
						cs += lpszVal;
						cs += " IN+=";
						cs += pChan->am_cs_Input_pos;
						cs += " IN-=";
						cs += pChan->am_cs_Input_neg;
						str_comment += cs;
						str_comment += rc;
					}
				}
		*/
	}
	p_dc->SelectObject(p_old_brush);
	return str_comment;
}

BOOL ViewSpikes::OnPreparePrinting(CPrintInfo* p_info)
{
	if (!DoPreparePrinting(p_info))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, p_info))
		return FALSE;

	// save current state of the windows
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	if (p_app->m_p_view_spikes_memory_file == nullptr)
	{
		p_app->m_p_view_spikes_memory_file = new CMemFile;
		ASSERT(p_app->m_p_view_spikes_memory_file != NULL);
	}

	CArchive ar(p_app->m_p_view_spikes_memory_file, CArchive::store);
	p_app->m_p_view_spikes_memory_file->SeekToBegin();
	chart_data_wnd_.Serialize(ar);
	//spk_bar_wnd_.Serialize(ar);
	//m_spkShapeView.Serialize(ar);
	ar.Close(); // close archive

	// printing margins
	if (options_view_data_->vertical_resolution <= 0 || options_view_data_->horizontal_resolution <= 0
		|| options_view_data_->horizontal_resolution != p_info->m_rectDraw.Width()
		|| options_view_data_->vertical_resolution != p_info->m_rectDraw.Height())
		print_compute_page_size();

	// how many rows per page?
	const auto size_row = options_view_data_->height_doc + options_view_data_->height_separator;
	n_rows_per_page_ = print_rect_.Height() / size_row;
	if (n_rows_per_page_ == 0) // prevent zero pages
		n_rows_per_page_ = 1;

	// compute number of rows according to multiple row flag
	l_print_first_ = spike_class_listbox_.get_time_first();
	l_print_len_ = spike_class_listbox_.get_time_last() - l_print_first_ + 1;

	// make sure the number of classes per file is known
	const auto p_dbwave_doc = GetDocument();
	file_0_ = p_dbwave_doc->db_get_current_record_position();
	ASSERT(file_0_ >= 0);
	print_first_ = file_0_;
	print_last_ = file_0_;
	files_count_ = 1;

	if (!options_view_data_->b_print_selection)
	{
		print_first_ = 0;
		files_count_ = p_dbwave_doc->db_get_records_count();
		print_last_ = files_count_ - 1;
	}

	// update the nb of classes per file selected and add this number
	max_classes_ = 1;
	auto nb_rect = 0; // total nb of rows
	if (p_dbwave_doc->db_set_current_record_position(print_first_))
	{
		auto nn_classes = 0;

		for (auto i = print_first_; i <= print_last_; i++, p_dbwave_doc->db_move_next())
		{
			// get number of classes
			if (p_dbwave_doc->get_db_n_spike_classes() <= 0)
			{
				p_spk_doc = p_dbwave_doc->open_current_spike_file();
				p_spk_list = p_spk_doc->get_spike_list_current();
				if (!p_spk_list->is_class_list_valid()) 
				{
					p_spk_list->update_class_list();
					p_spk_doc->SetModifiedFlag(); 
				}

				int n_classes = 1;
				if (p_spk_list->get_spikes_count() > 0)
					n_classes = p_spk_list->get_classes_count();
				ASSERT(n_classes > 0);
				p_dbwave_doc->set_db_n_spike_classes(n_classes);
				nn_classes += n_classes;
			}

			if (p_dbwave_doc->get_db_n_spike_classes() > max_classes_)
				max_classes_ = p_dbwave_doc->get_db_n_spike_classes();

			if (options_view_data_->b_multiple_rows)
			{
				const auto len = p_dbwave_doc->db_get_data_len() - l_print_first_;
				auto n_rows = len / l_print_len_; 
				if (len > n_rows * l_print_len_) 
					n_rows++;
				nb_rect += static_cast<int>(n_rows); 
			}
		}
	}

	// multiple rows?
	if (!options_view_data_->b_multiple_rows)
		nb_rect = files_count_;

	// n pages
	auto n_pages = nb_rect / n_rows_per_page_;
	if (nb_rect > n_rows_per_page_ * n_pages)
		n_pages++;

	//------------------------------------------------------
	p_info->SetMaxPage(n_pages); 
	p_info->m_nNumPreviewPages = 1; 
	p_info->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; 

	if (options_view_data_->b_print_selection)
		p_info->m_pPD->m_pd.Flags |= PD_SELECTION; 

	// call dialog box
	const auto flag = DoPreparePrinting(p_info);
	// set max nb of pages according to selection
	options_view_data_->b_print_selection = p_info->m_pPD->PrintSelection();
	if (options_view_data_->b_print_selection)
	{
		n_pages = 1;
		files_count_ = 1;
		if (options_view_data_->b_multiple_rows)
		{
			const auto l_first0 = spike_class_listbox_.get_time_first();
			const auto l_last0 = spike_class_listbox_.get_time_last();
			const auto len = p_spk_doc->get_acq_size() - l_first0;
			nb_rect = len / (l_last0 - l_first0);
			if (nb_rect * (l_last0 - l_first0) < len)
				nb_rect++;

			n_pages = nb_rect / n_rows_per_page_;
			if (n_pages * n_rows_per_page_ < nb_rect)
				n_pages++;
		}
		p_info->SetMaxPage(n_pages);
	}

	if (!p_dbwave_doc->db_set_current_record_position(file_0_))
		AfxMessageBox(_T("database error repositioning record\n"), MB_OK);
	return flag;
}

void ViewSpikes::OnBeginPrinting(CDC* p_dc, CPrintInfo* p_info)
{
	is_printing_ = TRUE;
	l_first_0_ = spike_class_listbox_.get_time_first();
	l_last0_ = spike_class_listbox_.get_time_last();

	//---------------------init objects-------------------------------------
	memset(&log_font_, 0, sizeof(LOGFONT)); 
	lstrcpy(log_font_.lfFaceName, _T("Arial"));
	log_font_.lfHeight = options_view_data_->font_size; 
	p_old_font_ = nullptr;
	font_print_.CreateFontIndirect(&log_font_);
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewSpikes::OnPrint(CDC* p_dc, CPrintInfo* p_info)
{
	p_old_font_ = p_dc->SelectObject(&font_print_);
	p_dc->SetMapMode(MM_TEXT); 
	print_file_bottom_page(p_dc, p_info); 
	const int current_page = static_cast<int>(p_info->m_nCurPage); 

	// --------------------- load data corresponding to the first row of current page

	// print only current selection - transform current page into file index
	int file_index; 
	auto l_first = print_get_file_series_index_from_page(current_page - 1, &file_index);
	if (GetDocument()->db_set_current_record_position(file_index))
	{
		update_file_parameters(FALSE);
		update_file_scroll();
	}
	auto very_last = p_spk_doc->get_acq_size() - 1; 

	CRect r_where(print_rect_.left,
	              print_rect_.top, 
	              print_rect_.left + options_view_data_->width_doc,
	              print_rect_.top + options_view_data_->height_doc);

	// loop through all files	--------------------------------------------------------
	for (int i = 0; i < n_rows_per_page_; i++)
	{
		// save conditions (Save/RestoreDC is mandatory!) --------------------------------

		const auto old_dc = p_dc->SaveDC(); // save DC

		// set first rectangle where data will be printed

		auto comment_rect = r_where;
		p_dc->SetMapMode(MM_TEXT);
		p_dc->SetTextAlign(TA_LEFT); 
		if (options_view_data_->b_frame_rect) 
		{
			p_dc->MoveTo(r_where.left, r_where.top);
			p_dc->LineTo(r_where.right, r_where.top); 
			p_dc->LineTo(r_where.right, r_where.bottom); 
			p_dc->LineTo(r_where.left, r_where.bottom); 
			p_dc->LineTo(r_where.left, r_where.top); 
		}
		p_dc->SetViewportOrg(r_where.left, r_where.top);

		// load data and adjust display rectangle ----------------------------------------
		// reduce width to the size of the data

		auto rw2 = r_where; // printing rectangle - constant
		rw2.OffsetRect(-r_where.left, -r_where.top); // set RW2 origin = 0,0
		auto r_height = rw2.Height() / max_classes_; // n_count;
		if (p_data_doc_ != nullptr)
			r_height = rw2.Height() / (max_classes_ + 1);
		const auto r_separator = r_height / 8;
		const auto r_col = rw2.Width() / 8;

		// rectangles for the 3 categories of data
		auto rw_text = rw2;
		auto rw_spikes = rw2;
		auto rw_bars = rw2;

		// horizontal size and position of the 3 rectangles

		rw_text.right = rw_text.left + r_col;
		rw_spikes.left = rw_text.right + r_separator;
		//auto n = m_pSpkDoc->GetSpkListCurrent()->get_spike_length();
		if (p_spk_doc->get_spike_list_current()->get_spike_length() > 1)
			rw_spikes.right = rw_spikes.left + r_col;
		else
			rw_spikes.right = rw_spikes.left;
		rw_bars.left = rw_spikes.right + r_separator;

		// bottom of the first rectangle
		rw_bars.bottom = rw2.top + r_height;
		auto l_last = l_first + l_print_len_; 
		if (l_last > very_last) 
			l_last = very_last;
		if ((l_last - l_first + 1) < l_print_len_) 
		{
			rw_bars.right = MulDiv(rw_bars.Width(), l_last - l_first, l_print_len_)
				+ rw_bars.left;
			ASSERT(rw_bars.right > rw_bars.left);
		}

		// ------------------------ print data

		auto extent = spike_class_listbox_.get_yw_extent();
		//auto zero = m_spkClass.GetYWOrg();

		if (p_data_doc_ != nullptr)
		{
			if (options_view_data_->b_clip_rect)
				p_dc->IntersectClipRect(&rw_bars); 
			chart_data_wnd_.get_data_from_doc(l_first, l_last);
			chart_data_wnd_.center_chan(0);
			chart_data_wnd_.print(p_dc, &rw_bars); 
			p_dc->SelectClipRgn(nullptr);

			extent = chart_data_wnd_.get_channel_list_item(0)->get_y_extent();
			rw_bars.top = rw_bars.bottom;
		}

		// ------------------------ print spikes

		rw_bars.bottom = rw_bars.top + r_height; 
		rw_spikes.top = rw_bars.top;
		rw_spikes.bottom = rw_bars.bottom;
		rw_text.top = rw_bars.top;
		rw_text.bottom = rw_bars.bottom;

		//m_pSpkList = m_pSpkDoc->GetSpkList_Current();
		//AdjustYZoomToMaxMin(true); 
		int max, min;
		p_spk_doc->get_spike_list_current()->get_total_max_min(TRUE, &max, &min);
		const int middle = (static_cast<int>(max) + static_cast<int>(min)) / 2;
		spike_class_listbox_.set_y_zoom(extent, middle);
		const auto n_count = spike_class_listbox_.GetCount(); 

		for (auto i_count = 0; i_count < n_count; i_count++)
		{
			spike_class_listbox_.set_time_intervals(l_first, l_last);
			spike_class_listbox_.print_item(p_dc, &rw_text, &rw_spikes, &rw_bars, i_count);
			rw_text.OffsetRect(0, r_height);
			rw_spikes.OffsetRect(0, r_height);
			rw_bars.OffsetRect(0, r_height);
		}

		// ------------------------ print stimulus

		if (p_spk_doc->m_stimulus_intervals.n_items > 0)
		{
			CBrush blue_brush; 
			blue_brush.CreateSolidBrush(col_blue);
			const auto old_brush = p_dc->SelectObject(&blue_brush);

			CPen blue_pen; 
			blue_pen.CreatePen(PS_SOLID, 0, col_blue);
			const auto old_pen = p_dc->SelectObject(&blue_pen);

			rw_spikes.bottom = rw2.bottom;
			rw_spikes.top = rw2.bottom - r_height / 10;

			if (rw_spikes.top == rw_spikes.bottom)
				rw_spikes.bottom++;

			for (auto ii = 0; ii < p_spk_doc->m_stimulus_intervals.get_size(); ii++, ii++)
			{
				int ii_first = p_spk_doc->m_stimulus_intervals.get_at(ii);
				if ((ii + 1) >= p_spk_doc->m_stimulus_intervals.get_size())
					continue;
				int ii_last = p_spk_doc->m_stimulus_intervals.get_at(ii + 1);
				if (ii_first > l_last || ii_last < l_first)
					continue;
				if (ii_first < l_first)
					ii_first = l_first;
				if (ii_last > l_last)
					ii_last = l_last;

				rw_spikes.left = MulDiv(ii_first - l_first, rw_bars.Width(), l_last - l_first) + rw_bars.left;
				rw_spikes.right = MulDiv(ii_last - l_first, rw_bars.Width(), l_last - l_first) + rw_bars.left;
				if (rw_spikes.right <= rw_spikes.left)
					rw_spikes.right = rw_spikes.left + 1;
				p_dc->Rectangle(rw_spikes);
			}

			p_dc->SelectObject(old_brush);
			p_dc->SelectObject(old_pen);
		}

		// ------------------------ print stimulus

		// update display rectangle for next row
		r_where.OffsetRect(0, options_view_data_->height_doc + options_view_data_->height_separator);

		// restore DC ------------------------------------------------------

		p_dc->RestoreDC(old_dc); // restore Display context

		// print comments --------------------------------------------------

		p_dc->SetMapMode(MM_TEXT); // 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr); // no more clipping
		p_dc->SetViewportOrg(0, 0); // org = 0,0

		// print comments according to row within file
		CString cs_comment;
		if (l_first == l_print_first_) // first row = full comment
		{
			cs_comment += print_get_file_infos();
			cs_comment += print_bars(p_dc, &comment_rect); // bars and bar legends
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_view_data_->text_separator + comment_rect.Width(), 0);
		comment_rect.right = print_rect_.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect,
		               DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);
		p_dc->SetTextAlign(ui_flag);

		// update file parameters for next row --------------------------------------------
		l_first += l_print_len_;
		// next file?
		// if index next point is past the end of the file
		// OR not entire record and not multiple row display
		if ((l_first >= very_last)
			|| (!options_view_data_->b_complete_record &&
				!options_view_data_->b_multiple_rows))
		{
			file_index++; // next index
			if (file_index < files_count_) // last file ??
			{
				// NO: select new file
				if (GetDocument()->db_move_next())
				{
					update_file_parameters(FALSE);
					update_file_scroll();
				}
				very_last = p_spk_doc->get_acq_size() - 1;
			}
			else
				i = n_rows_per_page_; // YES: break
			l_first = l_print_first_;
		}
	} // this is the end of a very long for loop.....................

	if (p_old_font_ != nullptr)
		p_dc->SelectObject(p_old_font_);
}

void ViewSpikes::OnEndPrinting(CDC* p_dc, CPrintInfo* p_info)
{
	font_print_.DeleteObject();
	is_printing_ = FALSE;

	if (GetDocument()->db_set_current_record_position(file_0_)) {
		update_file_parameters(TRUE);
		spike_class_listbox_.set_time_intervals(l_first_0_, l_last0_);
		spike_class_listbox_.Invalidate();
	}

	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	if (p_app->m_p_view_spikes_memory_file != nullptr)
	{
		CArchive ar(p_app->m_p_view_spikes_memory_file, CArchive::load);
		p_app->m_p_view_spikes_memory_file->SeekToBegin();
		chart_data_wnd_.Serialize(ar);
		ar.Close(); // close archive
	}
}

char vs_units[] = {"GM  mµpf  "};  
int vs_units_power[] = {9, 6, 0, 0, -3, -6, -9, -12, 0};
constexpr int vs_max_index = 8; 

float ViewSpikes::print_change_unit(float x_val, CString* x_unit, float* x_scale_factor)
{
	if (x_val == 0.f) 
	{
		*x_scale_factor = 1.0f;
		return 0.0f;
	}

	short i;
	short i_sign = 1;
	if (x_val < 0)
	{
		i_sign = -1;
		x_val = -x_val;
	}
	const auto ip_rec = static_cast<short> (floor(std::log10(x_val))); 
	if (ip_rec <= 0 && x_val < 1.) 
		i = static_cast<short>(4 - ip_rec / 3); 
	else
		i = static_cast<short>(3 - ip_rec / 3);

	if (i > vs_max_index) 
		i = vs_max_index;
	else if (i < 0) 
		i = 0;
	*x_scale_factor = static_cast<float>(pow(10.0f, vs_units_power[i])); 
	x_unit->SetAt(0, vs_units[i]); 
	return x_val * static_cast<float>(i_sign) / *x_scale_factor; 
}

void ViewSpikes::center_data_display_on_spike(const int spike_no)
{
	// test if spike visible in the current time interval
	const Spike* spike = p_spk_list->get_spike(spike_no);
	const long spk_first = spike->get_time() - p_spk_list->get_detection_parameters()->detect_pre_threshold;
	const long spk_last = spk_first + p_spk_list->get_spike_length();
	const long spk_center = (spk_last + spk_first) / 2;
	if (spk_first < l_first_ || spk_last > l_last_)
	{
		const long span = (l_last_ - l_first_) / 2;
		l_first_ = spk_center - span;
		l_last_ = spk_center + span;
		update_legends(TRUE);
	}

	// center curve vertically
	CChanlistItem* chan = chart_data_wnd_.get_channel_list_item(0);
	const int doc_channel = p_spk_list->get_detection_parameters()->extract_channel;
	short max_data = p_data_doc_->get_value_from_buffer(doc_channel, spk_first);
	short min_data = max_data;
	for (long i = spk_first; i <= spk_last; i++)
	{
		const short value = p_data_doc_->get_value_from_buffer(doc_channel, i);
		if (value > max_data) max_data = value;
		if (value < min_data) min_data = value;
	}
	chan->set_y_zero((max_data + min_data) / 2);

	// display data
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::OnHScroll(UINT n_sb_code, UINT n_pos, CScrollBar* p_scroll_bar)
{
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
		{
			file_scrollbar_.GetScrollInfo(&file_scroll_infos_, SIF_ALL);
			const long l_first = file_scroll_infos_.nPos;
			const long l_last = l_first + static_cast<long>(file_scroll_infos_.nPage) - 1;
			chart_data_wnd_.get_data_from_doc(l_first, l_last);
		}
		break;

	default:
		scroll_file(n_sb_code, n_pos);
		break;
	}

	// adjust display
	l_first_ = chart_data_wnd_.get_data_first_index();
	l_last_ = chart_data_wnd_.get_data_last_index();
	update_legends(TRUE);
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();

	if (p_data_doc_ != nullptr)
		chart_data_wnd_.center_chan(0);
}

void ViewSpikes::scroll_file(const UINT n_sb_code, const UINT n_pos)
{
	// get corresponding data
	switch (n_sb_code)
	{
	case SB_LEFT: 
	case SB_LINELEFT: 
	case SB_LINERIGHT: 
	case SB_PAGELEFT: 
	case SB_PAGERIGHT: 
	case SB_RIGHT: 
		chart_data_wnd_.scroll_data_from_doc(static_cast<WORD>(n_sb_code));
		break;
	case SB_THUMBPOSITION: 
	case SB_THUMBTRACK: 
		chart_data_wnd_.get_data_from_doc(MulDiv(static_cast<int>(n_pos), chart_data_wnd_.get_document_last(), 100));
		break;
	default:
		break;
	}
}

void ViewSpikes::update_file_scroll()
{
	file_scroll_infos_.fMask = SIF_PAGE | SIF_POS;
	file_scroll_infos_.nPos = chart_data_wnd_.get_data_first_index();
	file_scroll_infos_.nPage = chart_data_wnd_.get_data_last_index() - chart_data_wnd_.get_data_first_index() + 1;
	file_scrollbar_.SetScrollInfo(&file_scroll_infos_);
}

void ViewSpikes::on_edit_copy()
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

		// output rectangle requested by user
		CRect rect(0, 0, options_view_data_->hz_resolution, options_view_data_->vt_resolution);

		// create metafile
		CMetaFileDC mDC;
		auto rect_bound = rect;
		rect_bound.right *= 32;
		rect_bound.bottom *= 30;
		auto p_dc_ref = GetDC();
		const auto old_dc = p_dc_ref->SaveDC(); // save DC

		auto cs_title = _T("dbWave\0") + GetDocument()->GetTitle();
		cs_title += _T("\0\0");
		mDC.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);

		// Draw document in metafile.
		CPen black_pen(PS_SOLID, 0, col_black);
		const auto old_pen = mDC.SelectObject(&black_pen);
		if (!static_cast<CBrush*>(mDC.SelectStockObject(BLACK_BRUSH)))
			return;
		CClientDC attribute_dc(this); 
		mDC.SetAttribDC(attribute_dc.GetSafeHdc()); 

		// print comments : set font
		p_old_font_ = nullptr;
		const auto old_size = options_view_data_->font_size;
		options_view_data_->font_size = 10;
		memset(&log_font_, 0, sizeof(LOGFONT)); 
		lstrcpy(log_font_.lfFaceName, _T("Arial")); 
		log_font_.lfHeight = options_view_data_->font_size; 
		p_old_font_ = nullptr;
		font_print_.CreateFontIndirect(&log_font_);
		mDC.SetBkMode(TRANSPARENT);

		options_view_data_->font_size = old_size;
		p_old_font_ = mDC.SelectObject(&font_print_);

		CString comments;
		// display data: source data and spikes
		//auto extent = m_ChartSpikesListBox.GetYWExtent(); 
		const auto r_height = MulDiv(spike_class_listbox_.get_row_height(), rect.Width(),
		                            spike_class_listbox_.get_columns_time_width());
		auto rw_spikes = rect;
		rw_spikes.bottom = r_height; 
		auto rw_text = rw_spikes;
		auto rw_bars = rw_spikes;
		// horizontal size and position of the 3 rectangles
		const auto r_separator = r_height / 5;
		rw_text.right = rw_text.left + r_height;
		rw_spikes.left = rw_text.right + r_separator;
		rw_spikes.right = rw_spikes.left + r_height;
		rw_bars.left = rw_spikes.right + r_separator;

		// display data	if data file was found
		if (p_data_doc_ != nullptr)
		{
			chart_data_wnd_.center_chan(0);
			chart_data_wnd_.print(&mDC, &rw_bars);

			//auto extent = m_ChartDataWnd.get_channel_list_item(0)->Get_Y_extent();
			rw_spikes.OffsetRect(0, r_height);
			rw_bars.OffsetRect(0, r_height);
			rw_text.OffsetRect(0, r_height);
		}

		// display spikes and bars
		adjust_y_zoom_to_max_min(true);
		const auto n_count = spike_class_listbox_.GetCount();

		for (int i_count = 0; i_count < n_count; i_count++)
		{
			spike_class_listbox_.print_item(&mDC, &rw_text, &rw_spikes, &rw_bars, i_count);
			rw_spikes.OffsetRect(0, r_height);
			rw_bars.OffsetRect(0, r_height);
			rw_text.OffsetRect(0, r_height);
		}

		if (p_old_font_ != nullptr)
			mDC.SelectObject(p_old_font_);
		font_print_.DeleteObject();

		// restore old_pen
		mDC.SelectObject(old_pen);
		ReleaseDC(p_dc_ref);

		// Close metafile
		const auto h_emf_tmp = mDC.CloseEnhanced();
		ASSERT(h_emf_tmp != NULL);
		if (OpenClipboard())
		{
			EmptyClipboard(); 
			SetClipboardData(CF_ENHMETAFILE, h_emf_tmp); 
			CloseClipboard();
		}
		else
		{
			// Someone else has the Clipboard open...
			DeleteEnhMetaFile(h_emf_tmp); 
			MessageBeep(0); 
			AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
		}
		p_dc_ref->RestoreDC(old_dc); 
	}

	// restore screen in previous state
	update_spike_file(TRUE);
	update_file_scroll();
	spike_class_listbox_.Invalidate();
	if (p_data_doc_ != nullptr)
	{
		chart_data_wnd_.get_data_from_doc(l_first_, l_last_);
		chart_data_wnd_.resize_channels(chart_data_wnd_.get_rect_width(), l_last_ - l_first_);
		chart_data_wnd_.Invalidate();
	}
}


void ViewSpikes::on_zoom()
{
	if (set_zoom.GetCheck())
	{
		int ii_start = 0;
		if (m_spike_index != -1)
		{
			ii_start = p_spk_list->get_spike(m_spike_index)->get_time();
			const int delta = static_cast<int>(m_zoom * p_spk_doc->get_acq_rate());
			ii_start -= delta / 2;
		}
		zoom_on_preset_interval(ii_start);
	}
	else
		on_format_all_data();
}


void ViewSpikes::zoom_on_preset_interval(int ii_start)
{
	if (ii_start < 0)
		ii_start = 0;
	l_first_ = ii_start;
	const auto acquisition_rate = p_spk_doc->get_acq_rate();
	l_last_ = static_cast<long>((static_cast<float>(l_first_) / acquisition_rate + m_zoom) * acquisition_rate);
	update_legends(TRUE);
	// display data
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::on_gain_button()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(1);
	set_v_bar_mode(BAR_GAIN);
}

void ViewSpikes::on_bias_button()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(0);
	set_v_bar_mode(BAR_BIAS);
}

void ViewSpikes::set_v_bar_mode(const short b_mode)
{
	if (b_mode == BAR_BIAS)
		v_bar_mode_ = b_mode;
	else
		v_bar_mode_ = BAR_GAIN;
	update_bias_scroll();
}

void ViewSpikes::update_gain_scroll()
{
	scrollbar_y_.SetScrollPos(
		MulDiv(chart_data_wnd_.get_channel_list_item(0)->get_y_extent(),
		       100,
		       Y_EXTENT_MAX)
		+ 50,
		TRUE);
}

void ViewSpikes::scroll_gain(const UINT n_sb_code, const UINT n_pos)
{
	int l_size = chart_data_wnd_.get_channel_list_item(0)->get_y_extent();
	// get corresponding data
	switch (n_sb_code)
	{
	case SB_LEFT: 
		l_size = Y_EXTENT_MIN;
		break;
	case SB_LINELEFT: 
		l_size -= l_size / 10 + 1;
		break;
	case SB_LINERIGHT: 
		l_size += l_size / 10 + 1;
		break;
	case SB_PAGELEFT: 
		l_size -= l_size / 2 + 1;
		break;
	case SB_PAGERIGHT: 
		l_size += l_size + 1;
		break;
	case SB_RIGHT: 
		l_size = Y_EXTENT_MAX;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		l_size = MulDiv(static_cast<int>(n_pos) - 50, Y_EXTENT_MAX, 100);
		break;
	default: break;
	}

	// change y extent
	if (l_size > 0) 
	{
		chart_data_wnd_.get_channel_list_item(0)->set_y_extent(l_size);
		update_legends(TRUE);
		chart_data_wnd_.Invalidate();
	}
	// update scrollBar
	if (v_bar_mode_ == BAR_GAIN)
		update_gain_scroll();
}

void ViewSpikes::update_bias_scroll()
{
	const CChanlistItem* chan_list_item = chart_data_wnd_.get_channel_list_item(0);
	const auto i_pos = (chan_list_item->get_y_zero()
			- chan_list_item->get_data_bin_zero())
			* 100 / static_cast<int>(Y_ZERO_SPAN) + 50;
	scrollbar_y_.SetScrollPos(i_pos, TRUE);
}

void ViewSpikes::scroll_bias(const UINT n_sb_code, const UINT n_pos)
{
	CChanlistItem* chan = chart_data_wnd_.get_channel_list_item(0);
	auto l_size = chan->get_y_zero() - chan->get_data_bin_zero();
	const auto y_extent = chan->get_y_extent();
	// get corresponding data
	switch (n_sb_code)
	{
	case SB_LEFT: 
		l_size = Y_ZERO_MIN;
		break;
	case SB_LINELEFT: 
		l_size -= y_extent / 100 + 1;
		break;
	case SB_LINERIGHT:
		l_size += y_extent / 100 + 1;
		break;
	case SB_PAGELEFT: 
		l_size -= y_extent / 10 + 1;
		break;
	case SB_PAGERIGHT: 
		l_size += y_extent / 10 + 1;
		break;
	case SB_RIGHT:
		l_size = Y_ZERO_MAX;
		break;
	case SB_THUMBPOSITION: 
	case SB_THUMBTRACK: 
		l_size = MulDiv(static_cast<int>(n_pos) - 50, Y_ZERO_SPAN, 100);
		break;
	default: 
		break;
	}

	// try to read data with this new size
	if (l_size > Y_ZERO_MIN && l_size < Y_ZERO_MAX)
	{
		chan->set_y_zero(l_size + chan->get_data_bin_zero());
		chart_data_wnd_.Invalidate();
	}
	// update scrollBar
	if (v_bar_mode_ == BAR_BIAS)
		update_bias_scroll();
}

void ViewSpikes::OnVScroll(const UINT n_sb_code, const UINT n_pos, CScrollBar* p_scroll_bar)
{
	if (p_scroll_bar != nullptr)
	{
		// CViewData scroll: vertical scroll bar
		switch (v_bar_mode_)
		{
		case BAR_GAIN:
			scroll_gain(n_sb_code, n_pos);
			break;
		case BAR_BIAS:
			scroll_bias(n_sb_code, n_pos);
		default:
			break;
		}
	}
	ViewDbTable::OnVScroll(n_sb_code, n_pos, p_scroll_bar);
}

void ViewSpikes::on_artefact()
{
	UpdateData(TRUE);
	if (m_spike_index < 0)
	{
		m_b_artefact = FALSE; 
	}
	else
	{
		const auto spike = p_spk_list->get_spike(m_spike_index);
		auto spk_class = spike->get_class_id();
		spk_class = -(spk_class + 1);
		spike_class_listbox_.change_spike_class(m_spike_index, spk_class);
	}
	CheckDlgButton(IDC_ARTEFACT, m_b_artefact);
	p_spk_doc->SetModifiedFlag(TRUE);
	update_legends(TRUE);
	spike_class_listbox_.Invalidate();
}

void ViewSpikes::on_h_scroll_left()
{
	OnHScroll(SB_PAGELEFT, NULL, static_cast<CScrollBar*>(GetDlgItem(IDC_FILESCROLL)));
}

void ViewSpikes::on_h_scroll_right()
{
	OnHScroll(SB_PAGERIGHT, NULL, static_cast<CScrollBar*>(GetDlgItem(IDC_FILESCROLL)));
}

void ViewSpikes::on_bn_clicked_same_class()
{
	m_b_keep_same_class = static_cast<CButton*>(GetDlgItem(IDC_SAMECLASS))->GetCheck();
}


void ViewSpikes::on_format_all_data()
{
	l_first_ = 0;
	l_last_ = p_spk_doc->get_acq_size() - 1;

	constexpr int x_wo = 0;
	const auto x_we = p_spk_list->get_spike_length();
	spike_class_listbox_.set_x_zoom(x_we, x_wo);

	update_legends(TRUE);

	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::on_format_center_curve()
{
	// loop over all spikes of the list
	const int n_spikes = p_spk_list->get_spikes_count();
	const auto i_t1 = spk_classification_parameters_->shape_t1;
	const auto i_t2 = spk_classification_parameters_->shape_t2;
	for (int i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		Spike* spike = p_spk_list->get_spike(i_spike);
		spike->center_spike_amplitude(i_t1, i_t2, 1);
	}

	int max, min;
	p_spk_list->get_total_max_min(TRUE, &max, &min);
	const int middle = max / 2 + min / 2;
	spike_class_listbox_.set_y_zoom(spike_class_listbox_.get_yw_extent(), middle);

	if (p_data_doc_ != nullptr)
		chart_data_wnd_.center_chan(0);

	update_legends(TRUE);
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::on_format_gain_adjust()
{
	if (p_spk_doc != nullptr)
	{
		adjust_y_zoom_to_max_min(true);
	}
	if (p_data_doc_ != nullptr)
		chart_data_wnd_.max_gain_chan(0);

	update_legends(TRUE);
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::on_format_previous_frame()
{
	zoom_on_preset_interval(l_first_ * 2 - l_last_);
}

void ViewSpikes::on_format_next_frame()
{
	const long len = l_last_ - l_first_;
	auto last = l_last_ + len;
	if (last > p_spk_doc->get_acq_size())
		last = l_last_ - len;
	zoom_on_preset_interval(last);
}


void ViewSpikes::on_en_change_spike_class()
{
	if (!mm_spike_class_.m_b_entry_done)
		return;
	const auto spike_class_old = m_spike_class;
	mm_spike_class_.on_en_change(this, m_spike_class, 1, -1);

	if (m_spike_class != spike_class_old)
	{
		spike_class_listbox_.change_spike_class(m_spike_index, m_spike_class);
		p_spk_doc->SetModifiedFlag(TRUE);
		update_legends(TRUE);
		spike_class_listbox_.Invalidate();
		chart_data_wnd_.Invalidate();
	}
}

void ViewSpikes::on_en_change_time_first()
{
	if (mm_time_first_.m_b_entry_done)
	{
		mm_time_first_.on_en_change(this, m_time_first, 1.f, -1.f);
		const auto l_first = static_cast<long>(m_time_first * p_spk_doc->get_acq_rate());
		if (l_first != l_first_)
		{
			l_first_ = l_first;
			update_legends(TRUE);
			spike_class_listbox_.Invalidate();
			chart_data_wnd_.Invalidate();
		}
	}
}

void ViewSpikes::on_en_change_time_last()
{
	if (mm_time_last_.m_b_entry_done)
	{
		mm_time_last_.on_en_change(this, m_time_last, 1.f, -1.f);
		const auto l_last = static_cast<long>(m_time_last * p_spk_doc->get_acq_rate());
		if (l_last != l_last_)
		{
			l_last_ = l_last;
			update_legends(TRUE);
			spike_class_listbox_.Invalidate();
			chart_data_wnd_.Invalidate();
		}
	}
}

void ViewSpikes::on_en_change_zoom()
{
	if (mm_zoom_.m_b_entry_done)
	{
		const auto zoom = m_zoom;
		mm_zoom_.on_en_change(this, m_zoom, 1.f, -1.f);

		// check boundaries
		if (m_zoom < 0.0f)
			m_zoom = 1.0f;

		if (m_zoom > zoom || m_zoom < zoom)
			zoom_on_preset_interval(0);

		UpdateData(FALSE);
	}
}

void ViewSpikes::on_en_change_source_class()
{
	if (mm_class_source_.m_b_entry_done)
	{
		mm_class_source_.on_en_change(this, m_class_source, 1, -1);
		UpdateData(FALSE);
	}
}

void ViewSpikes::on_en_change_dest_class()
{
	if (mm_class_destination_.m_b_entry_done)
	{
		mm_class_destination_.on_en_change(this, m_class_destination, 1, -1);
		UpdateData(FALSE);
	}
}

void ViewSpikes::on_en_change_jitter()
{
	if (mm_jitter_ms_.m_b_entry_done)
	{
		mm_jitter_ms_.on_en_change(this, m_jitter_ms, 1.f, -1.f);
		UpdateData(FALSE);
	}
}

void ViewSpikes::on_en_change_no_spike()
{
	if (mm_spike_index_.m_b_entry_done)
	{
		const auto spike_no = m_spike_index;
		const int delta_up = p_spk_list->get_next_spike(spike_no, 1, m_b_keep_same_class) - m_spike_index;
		const int delta_down = m_spike_index - p_spk_list->get_next_spike(spike_no, -1, m_b_keep_same_class);
		mm_spike_index_.on_en_change(this, m_spike_index, delta_up, -delta_down);

		m_spike_index = p_spk_list->get_valid_spike_number(m_spike_index);
		if (m_spike_index != spike_no)
		{
			db_spike spike_selected(-1, -1, m_spike_index);
			select_spike(spike_selected);
			if (m_spike_index >= 0)
				center_data_display_on_spike(m_spike_index);
		}
		else
			UpdateData(FALSE);
	}
}

