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

IMPLEMENT_DYNCREATE(ViewSpikes, dbTableView)

ViewSpikes::ViewSpikes() : dbTableView(IDD)
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

void ViewSpikes::DoDataExchange(CDataExchange* pDX)
{
	dbTableView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_TIMEFIRST, m_time_first);
	DDX_Text(pDX, IDC_TIMELAST, m_time_last);
	DDX_Text(pDX, IDC_NSPIKES, m_spike_index);
	DDX_Text(pDX, IDC_SPIKE_CLASS, m_spike_class);
	DDX_Text(pDX, IDC_EDIT3, m_zoom);
	DDX_Text(pDX, IDC_EDIT4, m_class_source);
	DDX_Text(pDX, IDC_EDIT5, m_class_destination);
	DDX_Check(pDX, IDC_CHECK1, m_b_reset_zoom);
	DDX_Check(pDX, IDC_ARTEFACT, m_b_artefact);
	DDX_Text(pDX, IDC_JITTER, m_jitter_ms);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
	DDX_Check(pDX, IDC_SAMECLASS, m_b_keep_same_class);
	DDX_Control(pDX, IDC_ZOOM_ON_OFF, set_zoom);
}

BEGIN_MESSAGE_MAP(ViewSpikes, dbTableView)

	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikes::OnMyMessage)

	ON_COMMAND(ID_FORMAT_VIEW_ALL_DATA_ON_ABSCISSA, &ViewSpikes::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTER_CURVE, &ViewSpikes::OnFormatCentercurve)
	ON_COMMAND(ID_FORMAT_GAIN_ADJUST, &ViewSpikes::OnFormatGainadjust)
	ON_COMMAND(ID_TOOLS_EDIT_SPIKES, &ViewSpikes::OnToolsEdittransformspikes)
	ON_COMMAND(ID_EDIT_COPY, &ViewSpikes::OnEditCopy)
	ON_COMMAND(ID_FORMAT_PREVIOUS_FRAME, &ViewSpikes::OnFormatPreviousframe)
	ON_COMMAND(ID_FORMAT_NEXT_FRAME, &ViewSpikes::OnFormatNextframe)
	ON_COMMAND(ID_RECORD_SHIFT_LEFT, &ViewSpikes::OnHScrollLeft)
	ON_COMMAND(ID_RECORD_SHIFT_RIGHT, &ViewSpikes::OnHScrollRight)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_EN_CHANGE(IDC_NSPIKES, &ViewSpikes::OnEnChangeNOspike)
	ON_EN_CHANGE(IDC_SPIKE_CLASS, &ViewSpikes::OnEnChangeSpikenoclass)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewSpikes::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewSpikes::OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_EDIT3, &ViewSpikes::OnEnChangeZoom)
	ON_EN_CHANGE(IDC_EDIT4, &ViewSpikes::OnEnChangeSourceclass)
	ON_EN_CHANGE(IDC_EDIT5, &ViewSpikes::OnEnChangeDestclass)
	ON_EN_CHANGE(IDC_JITTER, &ViewSpikes::OnEnChangeJitter)

	ON_BN_CLICKED(IDC_GAIN_button, &ViewSpikes::OnGAINbutton)
	ON_BN_CLICKED(IDC_BIAS_button, &ViewSpikes::OnBIASbutton)
	ON_BN_CLICKED(IDC_ARTEFACT, &ViewSpikes::OnArtefact)

	ON_BN_CLICKED(IDC_SAMECLASS, &ViewSpikes::OnBnClickedSameclass)
	ON_BN_CLICKED(IDC_ZOOM_ON_OFF, &ViewSpikes::on_zoom)
END_MESSAGE_MAP()

void ViewSpikes::OnActivateView(BOOL bActivate, CView* pActivateView, CView* p_deactivate_view)
{
	if (bActivate)
	{
		const auto p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATE_VIEW, reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		if (this != pActivateView && this == p_deactivate_view)
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

		p_app->options_view_data.viewdata = *(chart_data_wnd_.get_scope_parameters());
	}
	dbTableView::OnActivateView(bActivate, pActivateView, p_deactivate_view);
}

BOOL ViewSpikes::OnMove(UINT nIDMoveCommand)
{
	save_current_spk_file();
	return dbTableView::OnMove(nIDMoveCommand);
}

void ViewSpikes::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_b_init_)
	{
		switch (LOWORD(lHint))
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

void ViewSpikes::OnMouseMove(UINT nFlags, CPoint point)
{
	if (b_add_spike_mode_)
	{
		if (b_dummy_ && rect_vt_track_.PtInRect(point))
			pt_vt_ = point.x - rect_vt_track_.left;
		else
			pt_vt_ = -1;
		b_dummy_ = TRUE;
		chart_data_wnd_.xor_temp_vertical_tag(pt_vt_);
		spike_class_listbox_.xor_temp_vt_tag(pt_vt_);
	}
	dbTableView::OnMouseMove(nFlags, point);
}

void ViewSpikes::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (rect_vt_track_.PtInRect(point))
	{
		ReleaseCapture();
		const int ii_time = chart_data_wnd_.GetDataOffsetfromPixel(point.x - rect_vt_track_.left);
		jitter_ = m_jitter_ms;
		auto b_check = TRUE;
		if (nFlags & MK_CONTROL)
			b_check = FALSE;
		add_spike_to_list(ii_time, b_check);
		b_dummy_ = FALSE;
	}
	dbTableView::OnLButtonUp(nFlags, point);
}

void ViewSpikes::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (rect_vt_track_.PtInRect(point))
		SetCapture();
	dbTableView::OnLButtonDown(nFlags, point);
}

void ViewSpikes::set_mouse_cursor(const short param_value)
{
	if (chart_data_wnd_.get_mouse_cursor_type() != param_value)
		set_add_spikes_mode(param_value);

	chart_data_wnd_.set_mouse_cursor_type(param_value);
	int old_cursor = spike_class_listbox_.set_mouse_cursor_type(param_value);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SET_MOUSE_CURSOR, MAKELPARAM(param_value, 0));
}

void ViewSpikes::change_zoom(LPARAM lParam)
{
	if (HIWORD(lParam) == IDC_DISPLAY_DAT)
	{
		l_first_ = chart_data_wnd_.GetDataFirstIndex();
		l_last_ = chart_data_wnd_.GetDataLastIndex();
	}
	else if (HIWORD(lParam) == IDC_LISTCLASSES) //TODO [does not work! HIWORD(lParam)==1]
	{
		l_first_ = spike_class_listbox_.get_time_first();
		l_last_ = spike_class_listbox_.get_time_last();
	}
	update_legends(TRUE);
}

LRESULT ViewSpikes::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	short param_value = LOWORD(lParam);
	switch (wParam)
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
		change_zoom(lParam);
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
		OnToolsEdittransformspikes();
		break;

	case HINT_DROPPED:
		m_pSpkDoc->SetModifiedFlag();
		{
			const int old_class = m_spike_class;
			m_spike_class = m_pSpkList->get_spike(m_spike_index)->get_class_id();
			if (old_class != m_spike_class)
			{
				chart_data_wnd_.Invalidate();
				spike_class_listbox_.Invalidate();
			}
		}
		UpdateData(FALSE);
		break;

	case HINT_WINDOW_PROPS_CHANGED:
		options_view_data_->spkviewdata = *chart_data_wnd_.get_scope_parameters();
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
	const int method = m_pSpkList->get_detection_parameters()->detect_transform;
	const int doc_channel = m_pSpkList->get_detection_parameters()->extract_channel;
	const int pre_threshold = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	const int spike_length = m_pSpkList->get_spike_length();
	const int transformation_data_span = AcqDataDoc::get_transformed_data_span(method);
	const auto ii_time0 = ii_time - pre_threshold;
	auto l_read_write_first = ii_time0;
	auto l_read_write_last = ii_time0 + spike_length;
	
	// add a new spike if no spike is found around
	int spike_index = 0;
	auto is_found = FALSE;
	if (check_if_spike_nearby)
	{
		const auto jitter = static_cast<int>((m_pSpkDoc->get_acq_rate() * jitter_) / 1000);
		is_found = m_pSpkList->is_any_spike_around(ii_time0 + pre_threshold, jitter, spike_index, doc_channel);
	}

	if (!is_found)
	{
		if (!p_data_doc_->load_raw_data(&l_read_write_first, &l_read_write_last, transformation_data_span))
			return FALSE;

		p_data_doc_->load_transformed_data(l_read_write_first, l_read_write_last, method, doc_channel);
		const auto p_data_spike_0 = p_data_doc_->get_transformed_data_element(ii_time0 - l_read_write_first);

		spike_index = m_pSpkList->add_spike(p_data_spike_0,	//lpSource	= buff pointer to the buffer to copy
		                                  1,				//nb of interleaved channels
		                                  ii_time0 + pre_threshold, //time = file index of first pt of the spk
		                                  doc_channel,		//detect_channel	= data source chan index
		                                  m_class_destination, check_if_spike_nearby);

		m_pSpkDoc->SetModifiedFlag();
	}
	else if (m_pSpkList->get_spike(spike_index)->get_class_id() != m_class_destination)
	{
		m_pSpkList->get_spike(spike_index)->set_class_id(m_class_destination);
		m_pSpkDoc->SetModifiedFlag();
	}

	// save the modified data into the spike file
	if (m_pSpkDoc->IsModified())
	{
		m_pSpkList->update_class_list();
		m_pSpkDoc->OnSaveDocument(GetDocument()->db_get_current_spk_file_name(FALSE));
		m_pSpkDoc->SetModifiedFlag(FALSE);
		GetDocument()->set_db_n_spikes(m_pSpkList->get_spikes_count());
		GetDocument()->set_db_n_spike_classes(m_pSpkList->get_classes_count());
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
	if (m_pSpkDoc == nullptr)
		return;

	if (spike_selected.spike_index >= m_pSpkList->get_spikes_count())
		spike_selected.spike_index = -1;
	m_spike_index = spike_selected.spike_index;
	m_pSpkList->m_selected_spike = spike_selected.spike_index;
	spike_class_listbox_.select_spike(spike_selected);

	m_spike_class = -1;
	int n_cmd_show;
	if (spike_selected.spike_index >= 0 && spike_selected.spike_index < m_pSpkList->get_spikes_count())
	{
		const auto spike = m_pSpkList->get_spike(m_spike_index);
		m_spike_class = spike->get_class_id();
		m_b_artefact = (m_spike_class < 0);
		const auto spk_first = spike->get_time() - m_pSpkList->get_detection_parameters()->detect_pre_threshold;
		const auto spk_last = spk_first + m_pSpkList->get_spike_length();
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
	m_stretch_.AttachParent(this);
	m_stretch_.newProp(IDC_LISTCLASSES, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch_.newProp(IDC_DISPLAY_DAT, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_TIMEINTERVALS, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch_.newProp(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_SCROLLY_scrollbar, SZEQ_XREQ, SZEQ_YTEQ);
}

void ViewSpikes::OnInitialUpdate()
{
	dbTableView::OnInitialUpdate();
	define_sub_classed_items();
	define_stretch_parameters();
	m_b_init_ = TRUE;
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
	chart_data_wnd_.set_scope_parameters(&(options_view_data_->viewdata));
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
	chart_data_wnd_.AttachDataFile(p_data_doc_);

	const auto detect = m_pSpkList->get_detection_parameters();
	int source_data_view = detect->extract_channel;
	if (source_data_view >= p_data_doc_->get_waveformat()->scan_count)
	{
		detect->extract_channel = 0;
		source_data_view = 0;
	}
	if (detect->detect_what == DETECT_STIMULUS)
	{
		source_data_view = detect->detect_channel;
		if (source_data_view >= p_data_doc_->get_waveformat()->scan_count)
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
			chan->GetMaxMin(&max, &min);
			const auto extent = MulDiv(max - min + 1, 11, 10);
			const auto zero = (max + min) / 2;
			chan->SetYextent(extent);
			chan->SetYzero(zero);
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
		file_scroll_infos_.nPage = chart_data_wnd_.GetDataLastIndex() - chart_data_wnd_.GetDataFirstIndex() + 1;
		file_scrollbar_.SetScrollInfo(&file_scroll_infos_);
	}

	highlighted_intervals_.SetSize(3 + 2);					// total size
	highlighted_intervals_.SetAt(0, 0);		// source channel
	highlighted_intervals_.SetAt(1, RGB(255, 0, 0));	// red color
	highlighted_intervals_.SetAt(2, 1);		// pen size
	highlighted_intervals_.SetAt(3, 0);		// pen size
	highlighted_intervals_.SetAt(4, 0);		// pen size
}

void ViewSpikes::update_spike_file(BOOL b_update_interface)
{
	m_pSpkDoc = GetDocument()->open_current_spike_file();

	if (nullptr == m_pSpkDoc)
	{
		spike_class_listbox_.set_source_data(nullptr, nullptr);
	}
	else
	{
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->db_get_current_spk_file_name(), FALSE);
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);

		const int current_index = GetDocument()->get_current_spike_file()->get_spike_list_current_index();
		m_pSpkList = m_pSpkDoc->set_spike_list_current_index(current_index);
		spk_detection_parameters_ = m_pSpkList->get_detection_parameters();

		spike_class_listbox_.set_source_data(m_pSpkList, GetDocument());
		if (b_update_interface)
		{
			m_tabCtrl.SetCurSel(current_index);
			// adjust Y zoom
			ASSERT(l_first_ >= 0);
			if (m_b_reset_zoom)
			{
				spike_class_listbox_.SetRedraw(FALSE);
				zoom_on_preset_interval(0);
				spike_class_listbox_.SetRedraw(TRUE);
			}
			else if (l_last_ > m_pSpkDoc->get_acq_size() - 1 || l_last_ <= l_first_)
				l_last_ = m_pSpkDoc->get_acq_size() - 1; 

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
	if (m_pSpkDoc != nullptr)
	{
		if (l_last_ >= m_pSpkDoc->get_acq_size())
			l_last_ = m_pSpkDoc->get_acq_size() - 1;
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
	if (m_pSpkDoc != nullptr)
	{
		m_time_first = static_cast<float>(l_first_) / m_pSpkDoc->get_acq_rate();
		m_time_last = static_cast<float>(l_last_ + 1) / m_pSpkDoc->get_acq_rate();
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
		short max, min;
		m_pSpkList->get_total_max_min(TRUE, &max, &min);
		y_we_ = MulDiv(max - min + 1, 10, 8);
		y_wo_ = (max + min) / 2;
	}
	spike_class_listbox_.set_y_zoom(y_we_, y_wo_);
}

void ViewSpikes::select_spike_list(int current_selection)
{
	m_pSpkList = m_pSpkDoc->set_spike_list_current_index(current_selection);
	ASSERT(m_pSpkList != NULL);

	spike_class_listbox_.set_spk_list(m_pSpkList);

	spike_class_listbox_.Invalidate();
	spk_detection_parameters_ = m_pSpkList->get_detection_parameters();

	// update source data: change data channel and update display
	int extract_channel = m_pSpkList->get_detection_parameters()->extract_channel;
	ASSERT(extract_channel == spk_detection_parameters_->extract_channel);
	if (m_pSpkList->get_detection_parameters()->detect_what == DETECT_STIMULUS)
		extract_channel = m_pSpkList->get_detection_parameters()->detect_channel;

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
		chan->GetMaxMin(&max, &min);
		const auto extent = MulDiv(max - min + 1, 11, 10);
		const auto zero = (max + min) / 2;
		chan->SetYextent(extent);
		chan->SetYzero(zero);
	}
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::OnToolsEdittransformspikes()
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
	dlg.spike_index = m_spike_index; 
	dlg.db_wave_doc = GetDocument();
	dlg.m_parent = this;
	dlg.DoModal();

	if (!dlg.b_artefact)
		m_spike_index = dlg.spike_index;

	if (dlg.b_changed)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
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
	options_view_data_->horzRes = dc.GetDeviceCaps(HORZRES);
	options_view_data_->vertRes = dc.GetDeviceCaps(VERTRES);

	// margins (pixels)
	m_print_rect_.right = options_view_data_->horzRes - options_view_data_->rightPageMargin;
	m_print_rect_.bottom = options_view_data_->vertRes - options_view_data_->bottomPageMargin;
	m_print_rect_.left = options_view_data_->leftPageMargin;
	m_print_rect_.top = options_view_data_->topPageMargin;
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
	p_dc->TextOut(options_view_data_->horzRes / 2, options_view_data_->vertRes - 57, ch_date);
}

CString ViewSpikes::print_convert_file_index(const long l_first, const long l_last) const
{
	CString cs_unit = _T(" s");
	int constexpr array_size = 64;
	TCHAR sz_dest[array_size];
	constexpr size_t cb_dest = array_size * sizeof(TCHAR);

	float x_scale_factor;
	auto x = print_change_unit(
		static_cast<float>(l_first) / m_pSpkDoc->get_acq_rate(), &cs_unit, &x_scale_factor);
	auto fraction = static_cast<int>((x - floorf(x)) * static_cast<float>(1000.));
	HRESULT hr = StringCbPrintf(sz_dest, cb_dest, TEXT("time = %i.%03.3i - "), static_cast<int>(x), fraction);
	CString cs_comment = _T("");
	if (hr == S_OK)
		cs_comment += sz_dest;

	x = static_cast<float>(l_last) / (m_pSpkDoc->get_acq_rate() * x_scale_factor); 
	fraction = static_cast<int>((x - floorf(x)) * static_cast<float>(1000.));
	hr = StringCbPrintf(sz_dest, cb_dest, _T("%f.%03.3i %s"), floorf(x), fraction, static_cast<LPCTSTR>(cs_unit));
	if (hr == S_OK)
		cs_comment += sz_dest;

	return cs_comment;
}

long ViewSpikes::print_get_file_series_index_from_page(const int page, int* file_number)
{
	auto l_first = m_l_print_first_;

	const auto max_row = m_nb_rows_per_page_ * page; 
	auto i_file = 0; 
	if (options_view_data_->bPrintSelection)
		i_file = m_file_0_;

	if (GetDocument()->db_set_current_record_position(i_file)) {

		auto very_last = GetDocument()->db_get_data_len() - 1;
		for (auto row = 0; row < max_row; row++)
		{
			l_first += m_l_print_len_; // end of row
			if (l_first >= very_last) // next file ?
			{
				i_file++; // next file index
				if (i_file > m_files_count_) // last file ??
				{
					i_file--;
					break;
				}
				// update end-of-file
				if (GetDocument()->db_move_next()) {
					very_last = GetDocument()->db_get_data_len() - 1;
					l_first = m_l_print_first_;
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
	const auto p_wave_format = &m_pSpkDoc->m_wave_format; // get data description

	// document's name, date and time
	if (options_view_data_->bDocName || options_view_data_->bAcqDateTime) // print doc infos?
	{
		if (options_view_data_->bDocName) // print file name
			str_comment += GetDocument()->db_get_current_spk_file_name(FALSE) + tab;
		if (options_view_data_->bAcqDateTime) // print data acquisition date & time
		{
			const auto acquisition_time = m_pSpkDoc->get_acq_time();
			const auto date = acquisition_time.Format(_T("%#d %m %Y %X")); //("%x %X");
			// or more explicitly %d-%b-%Y %H:%M:%S");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_view_data_->bAcqComment)
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
	if (options_view_data_->bTimeScaleBar)
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
		const auto x_bar = static_cast<float>(ii_bar) / m_pSpkDoc->get_acq_rate();
		CString cs;
		cs.Format(_T("\nbar = %f s"), x_bar);
		cs_comment += cs;
		str_comment += cs_comment + rc;
	}

	///// vertical voltage bars ///////////////////////////
	if (options_view_data_->bVoltageScaleBar)
	{
		constexpr auto vertical_bar = 100;
		rect_vertical_bar.left = rect->left + bar_origin.x;
		rect_vertical_bar.right = rect_vertical_bar.left - bar_size.cx;
		rect_vertical_bar.bottom = rect->bottom - bar_origin.y;
		rect_vertical_bar.top = rect_vertical_bar.bottom - vertical_bar;
		p_dc->Rectangle(&rect_vertical_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_view_data_->bChansComment || options_view_data_->bVoltageScaleBar || options_view_data_->bChanSettings)
	{
		/*
				int imax = m_sourceView.get_channel_list_size();	// number of data channels
				for (int ichan=0; ichan< imax; ichan++)		// loop
				{
					// boucler sur les commentaires de chan n a chan 0...
					wsprintf(lpszVal, "chan#%i ", ichan);	// channel number
					cs_comment = lpszVal;
					if (mdPM->bVoltageScaleBar)				// bar scale value
					{
						csUnit = " V";						// provisional unit
						float z= 	(float) m_yscalebar.GetRectHeight()* m_sourceView.GetChanlistVoltsperPixel(ichan);
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
					if (mdPM->bChansComment)
					{
						str_comment += tab;
						str_comment += m_sourceView.GetChanlistComment(ichan);
					}
					str_comment += rc;

					// print amplifiers settings (gain & filter), next line
					if (mdPM->bChanSettings)
					{
						CString cs;
						WORD channb = m_sourceView.GetChanlistSourceChan(ichan);
						CWaveChan* pChan = m_pDataDoc->m_pDataFile->GetpWavechanArray()->get_p_channel(channb);
						wsprintf(lpszVal, "headstage=%s", pChan->headstage);
						cs += lpszVal;
						wsprintf(lpszVal, " g=%li", (long) (pChan->xgain));
						cs += lpszVal;
						wsprintf(lpszVal, " LP=%i", pChan->am_lowpass);
						cs += lpszVal;
						cs += " IN+=";
						cs += pChan->am_csInputpos;
						cs += " IN-=";
						cs += pChan->am_csInputneg;
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
	if (options_view_data_->vertRes <= 0 || options_view_data_->horzRes <= 0
		|| options_view_data_->horzRes != p_info->m_rectDraw.Width()
		|| options_view_data_->vertRes != p_info->m_rectDraw.Height())
		print_compute_page_size();

	// how many rows per page?
	const auto size_row = options_view_data_->HeightDoc + options_view_data_->heightSeparator;
	m_nb_rows_per_page_ = m_print_rect_.Height() / size_row;
	if (m_nb_rows_per_page_ == 0) // prevent zero pages
		m_nb_rows_per_page_ = 1;

	// compute number of rows according to multiple row flag
	m_l_print_first_ = spike_class_listbox_.get_time_first();
	m_l_print_len_ = spike_class_listbox_.get_time_last() - m_l_print_first_ + 1;

	// make sure the number of classes per file is known
	const auto p_dbwave_doc = GetDocument();
	m_file_0_ = p_dbwave_doc->db_get_current_record_position();
	ASSERT(m_file_0_ >= 0);
	m_print_first_ = m_file_0_;
	m_print_last_ = m_file_0_;
	m_files_count_ = 1;

	if (!options_view_data_->bPrintSelection)
	{
		m_print_first_ = 0;
		m_files_count_ = p_dbwave_doc->db_get_n_records();
		m_print_last_ = m_files_count_ - 1;
	}

	// update the nb of classes per file selected and add this number
	max_classes_ = 1;
	auto nb_rect = 0; // total nb of rows
	if (p_dbwave_doc->db_set_current_record_position(m_print_first_))
	{
		auto nn_classes = 0;

		for (auto i = m_print_first_; i <= m_print_last_; i++, p_dbwave_doc->db_move_next())
		{
			// get number of classes
			if (p_dbwave_doc->get_db_n_spike_classes() <= 0)
			{
				m_pSpkDoc = p_dbwave_doc->open_current_spike_file();
				m_pSpkList = m_pSpkDoc->get_spike_list_current();
				if (!m_pSpkList->is_class_list_valid()) // if class list not valid:
				{
					m_pSpkList->update_class_list(); // rebuild list of classes
					m_pSpkDoc->SetModifiedFlag(); // and set modified flag
				}

				int n_classes = 1;
				if (m_pSpkList->get_spikes_count() > 0)
					n_classes = m_pSpkList->get_classes_count();
				ASSERT(n_classes > 0);
				p_dbwave_doc->set_db_n_spike_classes(n_classes);
				nn_classes += n_classes;
			}

			if (p_dbwave_doc->get_db_n_spike_classes() > max_classes_)
				max_classes_ = p_dbwave_doc->get_db_n_spike_classes();

			if (options_view_data_->bMultirowDisplay)
			{
				const auto len = p_dbwave_doc->db_get_data_len() - m_l_print_first_; // file length
				auto n_rows = len / m_l_print_len_; // how many rows for this file?
				if (len > n_rows * m_l_print_len_) // remainder?
					n_rows++;
				nb_rect += static_cast<int>(n_rows); // update nb of rows
			}
		}
	}

	// multiple rows?
	if (!options_view_data_->bMultirowDisplay)
		nb_rect = m_files_count_;

	// n pages
	auto n_pages = nb_rect / m_nb_rows_per_page_;
	if (nb_rect > m_nb_rows_per_page_ * n_pages)
		n_pages++;

	//------------------------------------------------------
	p_info->SetMaxPage(n_pages); //one-page printing/preview
	p_info->m_nNumPreviewPages = 1; // preview 1 pages at a time
	p_info->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (options_view_data_->bPrintSelection)
		p_info->m_pPD->m_pd.Flags |= PD_SELECTION; // set button to selection

	// call dialog box
	const auto flag = DoPreparePrinting(p_info);
	// set max nb of pages according to selection
	options_view_data_->bPrintSelection = p_info->m_pPD->PrintSelection();
	if (options_view_data_->bPrintSelection)
	{
		n_pages = 1;
		m_files_count_ = 1;
		if (options_view_data_->bMultirowDisplay)
		{
			const auto l_first0 = spike_class_listbox_.get_time_first();
			const auto l_last0 = spike_class_listbox_.get_time_last();
			const auto len = m_pSpkDoc->get_acq_size() - l_first0;
			nb_rect = len / (l_last0 - l_first0);
			if (nb_rect * (l_last0 - l_first0) < len)
				nb_rect++;

			n_pages = nb_rect / m_nb_rows_per_page_;
			if (n_pages * m_nb_rows_per_page_ < nb_rect)
				n_pages++;
		}
		p_info->SetMaxPage(n_pages);
	}

	if (!p_dbwave_doc->db_set_current_record_position(m_file_0_))
		AfxMessageBox(_T("database error repositioning record\n"), MB_OK);
	return flag;
}

void ViewSpikes::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_b_is_printing_ = TRUE;
	m_l_first_0_ = spike_class_listbox_.get_time_first();
	m_l_last0_ = spike_class_listbox_.get_time_last();

	//---------------------init objects-------------------------------------
	memset(&m_log_font_, 0, sizeof(LOGFONT)); 
	lstrcpy(m_log_font_.lfFaceName, _T("Arial"));
	m_log_font_.lfHeight = options_view_data_->fontsize; 
	m_p_old_font_ = nullptr;
	m_font_print_.CreateFontIndirect(&m_log_font_);
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewSpikes::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_p_old_font_ = p_dc->SelectObject(&m_font_print_);
	p_dc->SetMapMode(MM_TEXT); 
	print_file_bottom_page(p_dc, pInfo); 
	const int current_page = static_cast<int>(pInfo->m_nCurPage); 

	// --------------------- load data corresponding to the first row of current page

	// print only current selection - transform current page into file index
	int file_index; 
	auto l_first = print_get_file_series_index_from_page(current_page - 1, &file_index);
	if (GetDocument()->db_set_current_record_position(file_index))
	{
		update_file_parameters(FALSE);
		update_file_scroll();
	}
	auto very_last = m_pSpkDoc->get_acq_size() - 1; 

	CRect r_where(m_print_rect_.left,
	              m_print_rect_.top, 
	              m_print_rect_.left + options_view_data_->WidthDoc,
	              m_print_rect_.top + options_view_data_->HeightDoc);

	// loop through all files	--------------------------------------------------------
	for (int i = 0; i < m_nb_rows_per_page_; i++)
	{
		// save conditions (Save/RestoreDC is mandatory!) --------------------------------

		const auto old_dc = p_dc->SaveDC(); // save DC

		// set first rectangle where data will be printed

		auto comment_rect = r_where; // save RWhere for comments
		p_dc->SetMapMode(MM_TEXT); // 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); // set text align mode
		if (options_view_data_->bFrameRect) // print rectangle if necessary
		{
			p_dc->MoveTo(r_where.left, r_where.top);
			p_dc->LineTo(r_where.right, r_where.top); // top hz
			p_dc->LineTo(r_where.right, r_where.bottom); // right vert
			p_dc->LineTo(r_where.left, r_where.bottom); // bottom hz
			p_dc->LineTo(r_where.left, r_where.top); // left vert
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
		if (m_pSpkDoc->get_spike_list_current()->get_spike_length() > 1)
			rw_spikes.right = rw_spikes.left + r_col;
		else
			rw_spikes.right = rw_spikes.left;
		rw_bars.left = rw_spikes.right + r_separator;

		// bottom of the first rectangle
		rw_bars.bottom = rw2.top + r_height;
		auto l_last = l_first + m_l_print_len_; // compute last pt to load
		if (l_last > very_last) // check end across file length
			l_last = very_last;
		if ((l_last - l_first + 1) < m_l_print_len_) // adjust rect to length of data
		{
			rw_bars.right = MulDiv(rw_bars.Width(), l_last - l_first, m_l_print_len_)
				+ rw_bars.left;
			ASSERT(rw_bars.right > rw_bars.left);
		}

		// ------------------------ print data

		auto extent = spike_class_listbox_.get_yw_extent();
		//auto zero = m_spkClass.GetYWOrg();

		if (p_data_doc_ != nullptr)
		{
			if (options_view_data_->bClipRect)
				p_dc->IntersectClipRect(&rw_bars); 
			chart_data_wnd_.get_data_from_doc(l_first, l_last);
			chart_data_wnd_.center_chan(0);
			chart_data_wnd_.Print(p_dc, &rw_bars); 
			p_dc->SelectClipRgn(nullptr);

			extent = chart_data_wnd_.get_channel_list_item(0)->GetYextent();
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
		short max, min;
		m_pSpkDoc->get_spike_list_current()->get_total_max_min(TRUE, &max, &min);
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

		if (m_pSpkDoc->m_stimulus_intervals.n_items > 0)
		{
			CBrush blue_brush; 
			blue_brush.CreateSolidBrush(RGB(0, 0, 255));
			const auto old_brush = p_dc->SelectObject(&blue_brush);

			CPen blue_pen; 
			blue_pen.CreatePen(PS_SOLID, 0, RGB(0, 0, 255));
			const auto old_pen = p_dc->SelectObject(&blue_pen);

			rw_spikes.bottom = rw2.bottom;
			rw_spikes.top = rw2.bottom - r_height / 10;

			if (rw_spikes.top == rw_spikes.bottom)
				rw_spikes.bottom++;

			for (auto ii = 0; ii < m_pSpkDoc->m_stimulus_intervals.GetSize(); ii++, ii++)
			{
				int ii_first = m_pSpkDoc->m_stimulus_intervals.GetAt(ii);
				if ((ii + 1) >= m_pSpkDoc->m_stimulus_intervals.GetSize())
					continue;
				int ii_last = m_pSpkDoc->m_stimulus_intervals.GetAt(ii + 1);
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
		r_where.OffsetRect(0, options_view_data_->HeightDoc + options_view_data_->heightSeparator);

		// restore DC ------------------------------------------------------

		p_dc->RestoreDC(old_dc); // restore Display context

		// print comments --------------------------------------------------

		p_dc->SetMapMode(MM_TEXT); // 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr); // no more clipping
		p_dc->SetViewportOrg(0, 0); // org = 0,0

		// print comments according to row within file
		CString cs_comment;
		if (l_first == m_l_print_first_) // first row = full comment
		{
			cs_comment += print_get_file_infos();
			cs_comment += print_bars(p_dc, &comment_rect); // bars and bar legends
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_view_data_->textseparator + comment_rect.Width(), 0);
		comment_rect.right = m_print_rect_.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect,
		               DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);
		p_dc->SetTextAlign(ui_flag);

		// update file parameters for next row --------------------------------------------
		l_first += m_l_print_len_;
		// next file?
		// if index next point is past the end of the file
		// OR not entire record and not multiple row display
		if ((l_first >= very_last)
			|| (!options_view_data_->bEntireRecord &&
				!options_view_data_->bMultirowDisplay))
		{
			file_index++; // next index
			if (file_index < m_files_count_) // last file ??
			{
				// NO: select new file
				if (GetDocument()->db_move_next())
				{
					update_file_parameters(FALSE);
					update_file_scroll();
				}
				very_last = m_pSpkDoc->get_acq_size() - 1;
			}
			else
				i = m_nb_rows_per_page_; // YES: break
			l_first = m_l_print_first_;
		}
	} // this is the end of a very long for loop.....................

	if (m_p_old_font_ != nullptr)
		p_dc->SelectObject(m_p_old_font_);
}

void ViewSpikes::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_font_print_.DeleteObject();
	m_b_is_printing_ = FALSE;

	if (GetDocument()->db_set_current_record_position(m_file_0_)) {
		update_file_parameters(TRUE);
		spike_class_listbox_.set_time_intervals(m_l_first_0_, m_l_last0_);
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
	const Spike* spike = m_pSpkList->get_spike(spike_no);
	const long spk_first = spike->get_time() - m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	const long spk_last = spk_first + m_pSpkList->get_spike_length();
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
	const int doc_channel = m_pSpkList->get_detection_parameters()->extract_channel;
	short max_data = p_data_doc_->get_value_from_buffer(doc_channel, spk_first);
	short min_data = max_data;
	for (long i = spk_first; i <= spk_last; i++)
	{
		const short value = p_data_doc_->get_value_from_buffer(doc_channel, i);
		if (value > max_data) max_data = value;
		if (value < min_data) min_data = value;
	}
	chan->SetYzero((max_data + min_data) / 2);

	// display data
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
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
		{
			file_scrollbar_.GetScrollInfo(&file_scroll_infos_, SIF_ALL);
			const long l_first = file_scroll_infos_.nPos;
			const long l_last = l_first + static_cast<long>(file_scroll_infos_.nPage) - 1;
			chart_data_wnd_.get_data_from_doc(l_first, l_last);
		}
		break;

	default:
		scroll_file(nSBCode, nPos);
		break;
	}

	// adjust display
	l_first_ = chart_data_wnd_.GetDataFirstIndex();
	l_last_ = chart_data_wnd_.GetDataLastIndex();
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
		chart_data_wnd_.get_data_from_doc(MulDiv(static_cast<int>(n_pos), chart_data_wnd_.GetDocumentLast(), 100));
		break;
	default:
		break;
	}
}

void ViewSpikes::update_file_scroll()
{
	file_scroll_infos_.fMask = SIF_PAGE | SIF_POS;
	file_scroll_infos_.nPos = chart_data_wnd_.GetDataFirstIndex();
	file_scroll_infos_.nPage = chart_data_wnd_.GetDataLastIndex() - chart_data_wnd_.GetDataFirstIndex() + 1;
	file_scrollbar_.SetScrollInfo(&file_scroll_infos_);
}

void ViewSpikes::OnEditCopy()
{
	DlgCopyAs dlg;
	dlg.m_nabcissa = options_view_data_->hzResolution;
	dlg.m_nordinates = options_view_data_->vtResolution;
	dlg.m_bgraphics = options_view_data_->bgraphics;
	dlg.m_ioption = options_view_data_->bcontours;
	dlg.m_iunit = options_view_data_->bunits;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		options_view_data_->bgraphics = dlg.m_bgraphics;
		options_view_data_->bcontours = dlg.m_ioption;
		options_view_data_->bunits = dlg.m_iunit;
		options_view_data_->hzResolution = dlg.m_nabcissa;
		options_view_data_->vtResolution = dlg.m_nordinates;

		// output rectangle requested by user
		CRect rect(0, 0, options_view_data_->hzResolution, options_view_data_->vtResolution);

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
		CPen black_pen(PS_SOLID, 0, RGB(0, 0, 0));
		const auto old_pen = mDC.SelectObject(&black_pen);
		if (!static_cast<CBrush*>(mDC.SelectStockObject(BLACK_BRUSH)))
			return;
		CClientDC attribute_dc(this); 
		mDC.SetAttribDC(attribute_dc.GetSafeHdc()); 

		// print comments : set font
		m_p_old_font_ = nullptr;
		const auto old_size = options_view_data_->fontsize;
		options_view_data_->fontsize = 10;
		memset(&m_log_font_, 0, sizeof(LOGFONT)); 
		lstrcpy(m_log_font_.lfFaceName, _T("Arial")); 
		m_log_font_.lfHeight = options_view_data_->fontsize; 
		m_p_old_font_ = nullptr;
		m_font_print_.CreateFontIndirect(&m_log_font_);
		mDC.SetBkMode(TRANSPARENT);

		options_view_data_->fontsize = old_size;
		m_p_old_font_ = mDC.SelectObject(&m_font_print_);

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
			chart_data_wnd_.Print(&mDC, &rw_bars);

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

		if (m_p_old_font_ != nullptr)
			mDC.SelectObject(m_p_old_font_);
		m_font_print_.DeleteObject();

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
			ii_start = m_pSpkList->get_spike(m_spike_index)->get_time();
			const int delta = static_cast<int>(m_zoom * m_pSpkDoc->get_acq_rate());
			ii_start -= delta / 2;
		}
		zoom_on_preset_interval(ii_start);
	}
	else
		OnFormatAlldata();
}


void ViewSpikes::zoom_on_preset_interval(int ii_start)
{
	if (ii_start < 0)
		ii_start = 0;
	l_first_ = ii_start;
	const auto acquisition_rate = m_pSpkDoc->get_acq_rate();
	l_last_ = static_cast<long>((static_cast<float>(l_first_) / acquisition_rate + m_zoom) * acquisition_rate);
	update_legends(TRUE);
	// display data
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::OnGAINbutton()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(1);
	set_v_bar_mode(BAR_GAIN);
}

void ViewSpikes::OnBIASbutton()
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
		MulDiv(chart_data_wnd_.get_channel_list_item(0)->GetYextent(),
		       100,
		       Y_EXTENT_MAX)
		+ 50,
		TRUE);
}

void ViewSpikes::scroll_gain(const UINT n_sb_code, const UINT n_pos)
{
	int l_size = chart_data_wnd_.get_channel_list_item(0)->GetYextent();
	// get corresponding data
	switch (n_sb_code)
	{
	// .................scroll to the start
	case SB_LEFT: l_size = Y_EXTENT_MIN;
		break;
	// .................scroll one line left
	case SB_LINELEFT: l_size -= l_size / 10 + 1;
		break;
	// .................scroll one line right
	case SB_LINERIGHT: l_size += l_size / 10 + 1;
		break;
	// .................scroll one page left
	case SB_PAGELEFT: l_size -= l_size / 2 + 1;
		break;
	// .................scroll one page right
	case SB_PAGERIGHT: l_size += l_size + 1;
		break;
	// .................scroll to end right
	case SB_RIGHT: l_size = Y_EXTENT_MAX;
		break;
	// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: l_size = MulDiv(static_cast<int>(n_pos) - 50, Y_EXTENT_MAX, 100);
		break;
	// .................NOP: set position only
	default: break;
	}

	// change y extent
	if (l_size > 0) 
	{
		chart_data_wnd_.get_channel_list_item(0)->SetYextent(l_size);
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
	const auto i_pos = (chan_list_item->GetYzero()
			- chan_list_item->GetDataBinZero())
			* 100 / static_cast<int>(Y_ZERO_SPAN) + 50;
	scrollbar_y_.SetScrollPos(i_pos, TRUE);
}

void ViewSpikes::scroll_bias(UINT nSBCode, UINT nPos)
{
	CChanlistItem* chan = chart_data_wnd_.get_channel_list_item(0);
	auto l_size = chan->GetYzero() - chan->GetDataBinZero();
	const auto y_extent = chan->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: // scroll to the start
		l_size = Y_ZERO_MIN;
		break;
	case SB_LINELEFT: // scroll one line left
		l_size -= y_extent / 100 + 1;
		break;
	case SB_LINERIGHT: // scroll one line right
		l_size += y_extent / 100 + 1;
		break;
	case SB_PAGELEFT: // scroll one page left
		l_size -= y_extent / 10 + 1;
		break;
	case SB_PAGERIGHT: // scroll one page right
		l_size += y_extent / 10 + 1;
		break;
	case SB_RIGHT: // scroll to end right
		l_size = Y_ZERO_MAX;
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		l_size = MulDiv(static_cast<int>(nPos) - 50, Y_ZERO_SPAN, 100);
		break;
	default: // NOP: set position only
		break;
	}

	// try to read data with this new size
	if (l_size > Y_ZERO_MIN && l_size < Y_ZERO_MAX)
	{
		chan->SetYzero(l_size + chan->GetDataBinZero());
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
	dbTableView::OnVScroll(n_sb_code, n_pos, p_scroll_bar);
}

void ViewSpikes::OnArtefact()
{
	UpdateData(TRUE);
	if (m_spike_index < 0)
	{
		m_b_artefact = FALSE; 
	}
	else
	{
		auto spk_class = m_pSpkList->get_spike(m_spike_index)->get_class_id();
		spk_class = -(spk_class + 1);
		//// if artefact: set class to negative value
		//if (m_b_artefact) 
		//	spk_class = -(spk_class + 1);
		//// if not artefact: if spike has negative class, set to positive value
		//else if (spk_class < 0)
		//	spk_class = -(spk_class + 1);
		spike_class_listbox_.change_spike_class(m_spike_index, spk_class);
	}
	CheckDlgButton(IDC_ARTEFACT, m_b_artefact);
	m_pSpkDoc->SetModifiedFlag(TRUE);
	update_legends(TRUE);
	spike_class_listbox_.Invalidate();
}

void ViewSpikes::OnHScrollLeft()
{
	OnHScroll(SB_PAGELEFT, NULL, static_cast<CScrollBar*>(GetDlgItem(IDC_FILESCROLL)));
}

void ViewSpikes::OnHScrollRight()
{
	OnHScroll(SB_PAGERIGHT, NULL, static_cast<CScrollBar*>(GetDlgItem(IDC_FILESCROLL)));
}

void ViewSpikes::OnBnClickedSameclass()
{
	m_b_keep_same_class = static_cast<CButton*>(GetDlgItem(IDC_SAMECLASS))->GetCheck();
}


void ViewSpikes::OnFormatAlldata()
{
	l_first_ = 0;
	l_last_ = m_pSpkDoc->get_acq_size() - 1;
	// spikes: center spikes horizontally and adjust hz size of display
	constexpr int x_wo = 0;
	const auto x_we = m_pSpkList->get_spike_length();
	spike_class_listbox_.set_x_zoom(x_we, x_wo);

	update_legends(TRUE);

	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::OnFormatCentercurve()
{
	// TODO 

	// loop over all spikes of the list
	const int n_spikes = m_pSpkList->get_spikes_count();
	const auto i_t1 = spk_classification_parameters_->i_cursor_t1;
	const auto i_t2 = spk_classification_parameters_->i_cursor_t2;
	for (int i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		Spike* spike = m_pSpkList->get_spike(i_spike);
		spike->center_spike_amplitude(i_t1, i_t2, 1);
	}

	short max, min;
	m_pSpkList->get_total_max_min(TRUE, &max, &min);
	const WORD middle = max / 2 + min / 2;
	spike_class_listbox_.set_y_zoom(spike_class_listbox_.get_yw_extent(), middle);

	if (p_data_doc_ != nullptr)
		chart_data_wnd_.center_chan(0);

	update_legends(TRUE);
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::OnFormatGainadjust()
{
	if (m_pSpkDoc != nullptr)
	{
		adjust_y_zoom_to_max_min(true);
	}
	if (p_data_doc_ != nullptr)
		chart_data_wnd_.max_gain_chan(0);

	update_legends(TRUE);
	spike_class_listbox_.Invalidate();
	chart_data_wnd_.Invalidate();
}

void ViewSpikes::OnFormatPreviousframe()
{
	zoom_on_preset_interval(l_first_ * 2 - l_last_);
}

void ViewSpikes::OnFormatNextframe()
{
	const long len = l_last_ - l_first_;
	auto last = l_last_ + len;
	if (last > m_pSpkDoc->get_acq_size())
		last = l_last_ - len;
	zoom_on_preset_interval(last);
}


void ViewSpikes::OnEnChangeSpikenoclass()
{
	if (!mm_spike_class_.m_bEntryDone)
		return;
	const auto spike_class_old = m_spike_class;
	mm_spike_class_.OnEnChange(this, m_spike_class, 1, -1);

	if (m_spike_class != spike_class_old)
	{
		spike_class_listbox_.change_spike_class(m_spike_index, m_spike_class);
		m_pSpkDoc->SetModifiedFlag(TRUE);
		update_legends(TRUE);
		spike_class_listbox_.Invalidate();
		chart_data_wnd_.Invalidate();
	}
}

void ViewSpikes::OnEnChangeTimefirst()
{
	if (mm_time_first_.m_bEntryDone)
	{
		mm_time_first_.OnEnChange(this, m_time_first, 1.f, -1.f);
		const auto l_first = static_cast<long>(m_time_first * m_pSpkDoc->get_acq_rate());
		if (l_first != l_first_)
		{
			l_first_ = l_first;
			update_legends(TRUE);
			spike_class_listbox_.Invalidate();
			chart_data_wnd_.Invalidate();
		}
	}
}

void ViewSpikes::OnEnChangeTimelast()
{
	if (mm_time_last_.m_bEntryDone)
	{
		mm_time_last_.OnEnChange(this, m_time_last, 1.f, -1.f);
		const auto l_last = static_cast<long>(m_time_last * m_pSpkDoc->get_acq_rate());
		if (l_last != l_last_)
		{
			l_last_ = l_last;
			update_legends(TRUE);
			spike_class_listbox_.Invalidate();
			chart_data_wnd_.Invalidate();
		}
	}
}

void ViewSpikes::OnEnChangeZoom()
{
	if (mm_zoom_.m_bEntryDone)
	{
		const auto zoom = m_zoom;
		mm_zoom_.OnEnChange(this, m_zoom, 1.f, -1.f);

		// check boundaries
		if (m_zoom < 0.0f)
			m_zoom = 1.0f;

		if (m_zoom > zoom || m_zoom < zoom)
			zoom_on_preset_interval(0);

		UpdateData(FALSE);
	}
}

void ViewSpikes::OnEnChangeSourceclass()
{
	if (mm_class_source_.m_bEntryDone)
	{
		mm_class_source_.OnEnChange(this, m_class_source, 1, -1);
		UpdateData(FALSE);
	}
}

void ViewSpikes::OnEnChangeDestclass()
{
	if (mm_class_destination_.m_bEntryDone)
	{
		mm_class_destination_.OnEnChange(this, m_class_destination, 1, -1);
		UpdateData(FALSE);
	}
}

void ViewSpikes::OnEnChangeJitter()
{
	if (mm_jitter_ms_.m_bEntryDone)
	{
		mm_jitter_ms_.OnEnChange(this, m_jitter_ms, 1.f, -1.f);
		UpdateData(FALSE);
	}
}

void ViewSpikes::OnEnChangeNOspike()
{
	if (mm_spike_index_.m_bEntryDone)
	{
		const auto spike_no = m_spike_index;
		const int delta_up = m_pSpkList->get_next_spike(spike_no, 1, m_b_keep_same_class) - m_spike_index;
		const int delta_down = m_spike_index - m_pSpkList->get_next_spike(spike_no, -1, m_b_keep_same_class);
		mm_spike_index_.OnEnChange(this, m_spike_index, delta_up, -delta_down);

		m_spike_index = m_pSpkList->get_valid_spike_number(m_spike_index);
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

