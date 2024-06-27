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
	m_psC->v_dest_class = m_class_destination;
	m_psC->v_source_class = m_class_source;
	m_psC->b_reset_zoom = m_b_reset_zoom;
	m_psC->f_jitter_ms = m_jitter_ms;
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

	ON_COMMAND(ID_FORMAT_ALLDATA, &ViewSpikes::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &ViewSpikes::OnFormatCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &ViewSpikes::OnFormatGainadjust)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, &ViewSpikes::OnToolsEdittransformspikes)
	ON_COMMAND(ID_EDIT_COPY, &ViewSpikes::OnEditCopy)
	ON_COMMAND(ID_FORMAT_PREVIOUSFRAME, &ViewSpikes::OnFormatPreviousframe)
	ON_COMMAND(ID_FORMAT_NEXTFRAME, &ViewSpikes::OnFormatNextframe)
	ON_COMMAND(ID_RECORD_SHIFTLEFT, &ViewSpikes::OnHScrollLeft)
	ON_COMMAND(ID_RECORD_SHIFTRIGHT, &ViewSpikes::OnHScrollRight)
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

	ON_BN_CLICKED(IDC_BUTTON2, &ViewSpikes::OnZoom)
	ON_BN_CLICKED(IDC_GAIN_button, &ViewSpikes::OnGAINbutton)
	ON_BN_CLICKED(IDC_BIAS_button, &ViewSpikes::OnBIASbutton)
	ON_BN_CLICKED(IDC_ARTEFACT, &ViewSpikes::OnArtefact)

	ON_BN_CLICKED(IDC_SAMECLASS, &ViewSpikes::OnBnClickedSameclass)
END_MESSAGE_MAP()

void ViewSpikes::OnActivateView(BOOL bActivate, CView* pActivateView, CView* p_deactivate_view)
{
	if (bActivate)
	{
		const auto p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		if (this != pActivateView && this == p_deactivate_view)
		{
			saveCurrentSpkFile();
			// save column parameters
			m_psC->col_separator = m_spikeClassListBox.GetColumnsSeparatorWidth();
			m_psC->row_height = m_spikeClassListBox.GetRowHeight();
			m_psC->col_spikes = m_spikeClassListBox.GetColumnsSpikesWidth();
			m_psC->col_text = m_spikeClassListBox.GetColumnsTextWidth();

			if (p_app->m_p_view_spikes_memory_file == nullptr)
			{
				p_app->m_p_view_spikes_memory_file = new CMemFile;
				ASSERT(p_app->m_p_view_spikes_memory_file != NULL);
			}

			CArchive ar(p_app->m_p_view_spikes_memory_file, CArchive::store);
			p_app->m_p_view_spikes_memory_file->SeekToBegin();
			m_ChartDataWnd.Serialize(ar);
			ar.Close();
		}

		p_app->options_view_data.viewdata = *(m_ChartDataWnd.get_scope_parameters());
	}
	dbTableView::OnActivateView(bActivate, pActivateView, p_deactivate_view);
}

BOOL ViewSpikes::OnMove(UINT nIDMoveCommand)
{
	saveCurrentSpkFile();
	return dbTableView::OnMove(nIDMoveCommand);
}

void ViewSpikes::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_b_init)
	{
		switch (LOWORD(lHint))
		{
		case HINT_DOCHASCHANGED:
		case HINT_DOCMOVERECORD:
			update_file_parameters(TRUE);
			break;
		case HINT_CLOSEFILEMODIFIED:
			saveCurrentSpkFile();
			break;
		case HINT_REPLACEVIEW:
		default:
			break;
		}
	}
}

void ViewSpikes::set_add_spikes_mode(int mouse_cursor_type)
{
	// display or hide corresponding controls within this view
	const BOOL set_add_spike_mode = (mouse_cursor_type == CURSOR_CROSS);
	if (m_b_add_spike_mode == set_add_spike_mode)
		return;
	
	m_b_add_spike_mode = set_add_spike_mode;
	const int n_cmd_show = set_add_spike_mode ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_SOURCECLASS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_DESTCLASS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT4)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT5)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_GROUPBOX)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_JITTER)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_JITTERSTATIC)->ShowWindow(n_cmd_show);

	auto hWnd = GetSafeHwnd();
	if (!m_b_add_spike_mode)
		hWnd = nullptr;
	m_ChartDataWnd.reflect_mouse_move_message(hWnd);
	m_spikeClassListBox.ReflectBarsMouseMoveMessg(hWnd);
	m_ChartDataWnd.SetTrackSpike(m_b_add_spike_mode, m_pspkDP->extract_n_points, m_pspkDP->detect_pre_threshold,
	                             m_pspkDP->extract_channel);

	if (m_b_add_spike_mode)
		set_track_rectangle();
}

void ViewSpikes::set_track_rectangle()
{
	CRect rect0, rect1, rect2;
	GetWindowRect(&rect0);
	m_ChartDataWnd.GetWindowRect(&rect1);
	m_spikeClassListBox.GetWindowRect(&rect2);
	m_rectVTtrack.top = rect1.top - rect0.top;
	m_rectVTtrack.bottom = rect2.bottom - rect0.top;
	m_rectVTtrack.left = rect1.left - rect0.left;
	m_rectVTtrack.right = rect1.right - rect0.left;
}

void ViewSpikes::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_b_add_spike_mode)
	{
		if (m_bdummy && m_rectVTtrack.PtInRect(point))
			m_ptVT = point.x - m_rectVTtrack.left;
		else
			m_ptVT = -1;
		m_bdummy = TRUE;
		m_ChartDataWnd.xor_temp_vertical_tag(m_ptVT);
		m_spikeClassListBox.XorTempVTtag(m_ptVT);
	}
	dbTableView::OnMouseMove(nFlags, point);
}

void ViewSpikes::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_rectVTtrack.PtInRect(point))
	{
		ReleaseCapture();
		const int ii_time = m_ChartDataWnd.GetDataOffsetfromPixel(point.x - m_rectVTtrack.left);
		m_jitter = m_jitter_ms;
		auto b_check = TRUE;
		if (nFlags & MK_CONTROL)
			b_check = FALSE;
		add_spike_to_list(ii_time, b_check);
		m_bdummy = FALSE;
	}
	dbTableView::OnLButtonUp(nFlags, point);
}

void ViewSpikes::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_rectVTtrack.PtInRect(point))
		SetCapture();
	dbTableView::OnLButtonDown(nFlags, point);
}

void ViewSpikes::set_mouse_cursor(short param_value)
{
	if (m_ChartDataWnd.get_mouse_cursor_type() != param_value)
		set_add_spikes_mode(param_value);
	m_ChartDataWnd.set_mouse_cursor_type(param_value);
	m_spikeClassListBox.SetMouseCursorType(param_value);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(param_value, 0));
}

void ViewSpikes::change_zoom(LPARAM lParam)
{
	if (HIWORD(lParam) == IDC_DISPLAYDAT)
	{
		m_lFirst = m_ChartDataWnd.GetDataFirstIndex();
		m_lLast = m_ChartDataWnd.GetDataLastIndex();
	}
	else if (HIWORD(lParam) == IDC_LISTCLASSES) //[ne marche pas! HIWORD(lParam)==1]
	{
		m_lFirst = m_spikeClassListBox.GetTimeFirst();
		m_lLast = m_spikeClassListBox.GetTimeLast();
	}
	update_legends(TRUE);
}

LRESULT ViewSpikes::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	short param_value = LOWORD(lParam);
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		set_mouse_cursor(param_value);
		break;

	case HINT_SELECTSPIKES:
		m_ChartDataWnd.Invalidate();
		m_spikeClassListBox.Invalidate();
		break;

	case HINT_CHANGEHZLIMITS:
	case HINT_CHANGEZOOM:
	case HINT_VIEWSIZECHANGED:
		change_zoom(lParam);
		break;

	case HINT_HITSPIKE:
		db_spike spike_hit = GetDocument()->get_spike_hit();
		select_spike(spike_hit); 
		break;

	case HINT_DBLCLKSEL:
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
				m_ChartDataWnd.Invalidate();
				m_spikeClassListBox.Invalidate();
			}
		}
		UpdateData(FALSE);
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_viewdata->spkviewdata = *m_ChartDataWnd.get_scope_parameters();
		break;

	case HINT_HITSPIKE_SHIFT:
	{
		spike_hit = GetDocument()->get_spike_hit();
		select_spike(spike_hit);
	}
		break;

	default:
		break;
	}
	return 0L;
}

BOOL ViewSpikes::add_spike_to_list(long ii_time, BOOL check_if_spike_nearby)
{
	const int method = m_pSpkList->get_detection_parameters()->detect_transform;
	const int doc_channel = m_pSpkList->get_detection_parameters()->extract_channel;
	const int pre_threshold = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	const int spike_length = m_pSpkList->get_spike_length();
	const int transformation_data_span = p_data_doc_->get_transformed_data_span(method);
	const auto ii_time0 = ii_time - pre_threshold;
	auto l_read_write_first = ii_time0;
	auto l_read_write_last = ii_time0 + spike_length;
	
	// add a new spike if no spike is found around
	int spike_index = 0;
	auto is_found = FALSE;
	if (check_if_spike_nearby)
	{
		const auto jitter = static_cast<int>((m_pSpkDoc->get_acq_rate() * m_jitter) / 1000);
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
	m_spikeClassListBox.Invalidate();
	return TRUE;
}

void ViewSpikes::select_spike(const db_spike& spike_selected)
{
	if (m_pSpkDoc == nullptr)
		return;
	if (spike_no >= m_pSpkList->get_spikes_count())
		spike_no = -1;
	m_spike_index = spike_no;
	m_pSpkList->m_selected_spike = spike_no;
	m_spikeClassListBox.SelectSpike(spike_no);

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
			m_highlighted_intervals.SetAt(3, spk_first);
			m_highlighted_intervals.SetAt(4, spk_last);
			m_ChartDataWnd.SetHighlightData(&m_highlighted_intervals);
			m_ChartDataWnd.Invalidate();
		}
	}
	else
	{
		n_cmd_show = SW_HIDE;
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	mm_spike_class.ShowWindow(n_cmd_show);
	GetDlgItem(IDC_ARTEFACT)->ShowWindow(n_cmd_show);

	UpdateData(FALSE);
}

void ViewSpikes::define_sub_classed_items()
{
	// attach controls
	VERIFY(m_file_scroll.SubclassDlgItem(IDC_FILESCROLL, this));
	m_file_scroll.SetScrollRange(0, 100, FALSE);
	VERIFY(mm_spike_index.SubclassDlgItem(IDC_NSPIKES, this));
	mm_spike_index.ShowScrollBar(SB_VERT);
	VERIFY(mm_spike_class.SubclassDlgItem(IDC_SPIKE_CLASS, this));
	mm_spike_class.ShowScrollBar(SB_VERT);
	VERIFY(m_spikeClassListBox.SubclassDlgItem(IDC_LISTCLASSES, this));
	VERIFY(mm_time_first.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_time_last.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(m_ChartDataWnd.SubclassDlgItem(IDC_DISPLAYDAT, this));
	VERIFY(mm_zoom.SubclassDlgItem(IDC_EDIT3, this));
	mm_zoom.ShowScrollBar(SB_VERT);
	VERIFY(mm_class_source.SubclassDlgItem(IDC_EDIT4, this));
	mm_class_source.ShowScrollBar(SB_VERT);
	VERIFY(mm_class_destination.SubclassDlgItem(IDC_EDIT5, this));
	mm_class_destination.ShowScrollBar(SB_VERT);
	VERIFY(mm_jitter_ms.SubclassDlgItem(IDC_JITTER, this));

	// left scrollbar and button
	VERIFY(m_scroll_y.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scroll_y.SetScrollRange(0, 100);
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         (LPARAM)static_cast<HANDLE>(m_hBias));
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         (LPARAM)static_cast<HANDLE>(m_hZoom));
}

void ViewSpikes::define_stretch_parameters()
{
	m_stretch.AttachParent(this);
	m_stretch.newProp(IDC_LISTCLASSES, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_DISPLAYDAT, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_TIMEINTERVALS, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_SCROLLY_scrollbar, SZEQ_XREQ, SZEQ_YTEQ);
}

void ViewSpikes::OnInitialUpdate()
{
	dbTableView::OnInitialUpdate();
	define_sub_classed_items();
	define_stretch_parameters();
	m_b_init = TRUE;
	m_autoIncrement = true;
	m_autoDetect = true;

	// load global parameters
	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	options_viewdata = &(p_app->options_view_data); // viewdata options
	mdMO = &(p_app->options_view_data_measure); // measure options
	m_psC = &(p_app->spk_classification); // get address of spike classif parms

	m_class_destination = m_psC->v_dest_class;
	m_class_source = m_psC->v_source_class;
	m_b_reset_zoom = m_psC->b_reset_zoom;
	m_jitter_ms = m_psC->f_jitter_ms;

	// adjust size of the row and cols with text, spikes, and bars
	CRect rect;
	GetDlgItem(IDC_LISTCLASSES)->GetWindowRect(&rect);
	m_spikeClassListBox.SetRowHeight(m_psC->row_height);
	CRect rect2;
	GetDlgItem(IDC_DISPLAYDAT)->GetWindowRect(&rect2);
	const int left_col_width = rect2.left - rect.left - 2;
	m_spikeClassListBox.SetLeftColumnWidth(left_col_width);
	if (m_psC->col_text < 0)
	{
		m_psC->col_spikes = m_psC->row_height;
		m_psC->col_text = left_col_width - 2 * m_psC->col_separator - m_psC->col_spikes;
		if (m_psC->col_text < 20)
		{
			const auto half = left_col_width - m_psC->col_separator;
			m_psC->col_spikes = half;
			m_psC->col_text = half;
		}
	}
	m_psC->col_text = left_col_width - m_psC->col_spikes - 2 * m_psC->col_separator;
	m_spikeClassListBox.SetColumnsWidth(m_psC->col_spikes, m_psC->col_separator);
	m_spikeClassListBox.SetCursorMaxOnDblClick(3);

	// init relation with document, display data, adjust parameters
	m_ChartDataWnd.set_scope_parameters(&(options_viewdata->viewdata));
	m_ChartDataWnd.set_cursor_max_on_dbl_click(3);

	update_file_parameters(TRUE);
	if (m_b_add_spike_mode)
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_CURSORMODE_MEASURE, NULL);
		m_ChartDataWnd.SetTrackSpike(m_b_add_spike_mode, m_pspkDP->extract_n_points, m_pspkDP->detect_pre_threshold,
		                             m_pspkDP->extract_channel);
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

	m_ChartDataWnd.set_b_use_dib(FALSE);
	m_ChartDataWnd.AttachDataFile(p_data_doc_);

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
	if (m_ChartDataWnd.set_channel_list_source_channel(0, source_data_view) < 0)
	{
		m_ChartDataWnd.remove_all_channel_list_items();
	}
	else
	{
		m_ChartDataWnd.ResizeChannels(m_ChartDataWnd.get_rect_width(), m_lLast - m_lFirst);
		m_ChartDataWnd.GetDataFromDoc(m_lFirst, m_lLast);

		if (m_bInitSourceView)
		{
			m_bInitSourceView = FALSE;
			int max, min;
			CChanlistItem* chan = m_ChartDataWnd.get_channel_list_item(0);
			chan->GetMaxMin(&max, &min);
			const auto extent = MulDiv(max - min + 1, 11, 10);
			const auto zero = (max + min) / 2;
			chan->SetYextent(extent);
			chan->SetYzero(zero);
		}
	}

	if (b_update_interface)
	{
		m_ChartDataWnd.Invalidate();

		// adjust scroll bar (size of button and left/right limits)
		m_file_scroll_infos.fMask = SIF_ALL;
		m_file_scroll_infos.nMin = 0;
		m_file_scroll_infos.nMax = p_data_doc_->get_doc_channel_length() - 1;
		m_file_scroll_infos.nPos = 0;
		m_file_scroll_infos.nPage = m_ChartDataWnd.GetDataLastIndex() - m_ChartDataWnd.GetDataFirstIndex() + 1;
		m_file_scroll.SetScrollInfo(&m_file_scroll_infos);
	}

	m_highlighted_intervals.SetSize(3 + 2);					// total size
	m_highlighted_intervals.SetAt(0, 0);		// source channel
	m_highlighted_intervals.SetAt(1, RGB(255, 0, 0));	// red color
	m_highlighted_intervals.SetAt(2, 1);		// pen size
	m_highlighted_intervals.SetAt(3, 0);		// pen size
	m_highlighted_intervals.SetAt(4, 0);		// pen size
}

void ViewSpikes::update_spike_file(BOOL b_update_interface)
{
	m_pSpkDoc = GetDocument()->open_current_spike_file();

	if (nullptr == m_pSpkDoc)
	{
		m_spikeClassListBox.set_source_data(nullptr, nullptr);
	}
	else
	{
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->db_get_current_spk_file_name(), FALSE);
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);

		const int current_index = GetDocument()->get_current_spike_file()->get_spike_list_current_index();
		m_pSpkList = m_pSpkDoc->set_spike_list_as_current(current_index);
		m_pspkDP = m_pSpkList->get_detection_parameters();

		m_spikeClassListBox.set_source_data(m_pSpkList, GetDocument());
		if (b_update_interface)
		{
			m_tabCtrl.SetCurSel(current_index);
			// adjust Y zoom
			ASSERT(m_lFirst >= 0);
			if (m_b_reset_zoom)
			{
				m_spikeClassListBox.SetRedraw(FALSE);
				zoom_on_preset_interval(0);
				m_spikeClassListBox.SetRedraw(TRUE);
			}
			else if (m_lLast > m_pSpkDoc->get_acq_size() - 1 || m_lLast <= m_lFirst)
				m_lLast = m_pSpkDoc->get_acq_size() - 1; 

			m_spikeClassListBox.SetTimeIntervals(m_lFirst, m_lLast);
			adjust_y_zoom_to_max_min(false);
		}
	}

	// select row
	if (b_update_interface)
		m_spikeClassListBox.SetCurSel(0);
}

void ViewSpikes::update_legends(BOOL b_update_interface)
{
	if (!b_update_interface)
		return;

	if (m_lFirst < 0)
		m_lFirst = 0;
	if (m_lLast <= m_lFirst)
		m_lLast = m_lFirst + 120;
	if (m_lLast >= m_pSpkDoc->get_acq_size())
		m_lLast = m_pSpkDoc->get_acq_size() - 1;
	if (m_lFirst > m_lLast)
		m_lFirst = m_lLast - 120;

	// set cursor
	auto hwnd = GetSafeHwnd();
	if (!m_b_add_spike_mode)
		hwnd = nullptr;
	m_ChartDataWnd.reflect_mouse_move_message(hwnd);
	m_spikeClassListBox.ReflectBarsMouseMoveMessg(hwnd);
	m_ChartDataWnd.SetTrackSpike(m_b_add_spike_mode, m_pspkDP->extract_n_points, m_pspkDP->detect_pre_threshold,
	                             m_pspkDP->extract_channel);

	// update spike bars & forms CListBox
	if (m_lFirst != m_spikeClassListBox.GetTimeFirst()
		|| m_lLast != m_spikeClassListBox.GetTimeLast())
		m_spikeClassListBox.SetTimeIntervals(m_lFirst, m_lLast);

	// update text abscissa and horizontal scroll position
	m_time_first = static_cast<float>(m_lFirst) / m_pSpkDoc->get_acq_rate();
	m_time_last = static_cast<float>(m_lLast + 1) / m_pSpkDoc->get_acq_rate();
	m_ChartDataWnd.GetDataFromDoc(m_lFirst, m_lLast);

	// update scrollbar and select spikes
	db_spike spike_selected(-1, -1, m_spike_index);
	select_spike(spike_selected);
	update_file_scroll();
}

void ViewSpikes::adjust_y_zoom_to_max_min(BOOL b_force_search_max_min)
{
	if (m_yWE == 1 || b_force_search_max_min)
	{
		short max, min;
		m_pSpkList->get_total_max_min(TRUE, &max, &min);
		m_yWE = MulDiv(max - min + 1, 10, 8);
		m_yWO = (max + min) / 2;
	}
	m_spikeClassListBox.SetYzoom(m_yWE, m_yWO);
}

void ViewSpikes::select_spike_list(int current_selection)
{
	m_pSpkList = m_pSpkDoc->set_spike_list_as_current(current_selection);
	ASSERT(m_pSpkList != NULL);

	m_spikeClassListBox.SetSpkList(m_pSpkList);

	m_spikeClassListBox.Invalidate();
	m_pspkDP = m_pSpkList->get_detection_parameters();

	// update source data: change data channel and update display
	int extract_channel = m_pSpkList->get_detection_parameters()->extract_channel;
	ASSERT(extract_channel == m_pspkDP->extract_channel);
	if (m_pSpkList->get_detection_parameters()->detect_what == DETECT_STIMULUS)
		extract_channel = m_pSpkList->get_detection_parameters()->detect_channel;

	// no data available
	if (m_ChartDataWnd.set_channel_list_source_channel(0, extract_channel) < 0)
	{
		m_ChartDataWnd.remove_all_channel_list_items();
	}
	// data are ok
	else
	{
		m_ChartDataWnd.ResizeChannels(m_ChartDataWnd.get_rect_width(), m_lLast - m_lFirst);
		m_ChartDataWnd.GetDataFromDoc(m_lFirst, m_lLast);
		int max, min;
		CChanlistItem* chan = m_ChartDataWnd.get_channel_list_item(0);
		chan->GetMaxMin(&max, &min);
		const auto extent = MulDiv(max - min + 1, 11, 10);
		const auto zero = (max + min) / 2;
		chan->SetYextent(extent);
		chan->SetYzero(zero);
	}
	m_ChartDataWnd.Invalidate();
}

void ViewSpikes::OnToolsEdittransformspikes()
{
	// return if no spike shape
	if (m_spikeClassListBox.GetXWExtent() == 0) 
		return;
	// save time frame to restore it on return
	const auto l_first = m_spikeClassListBox.GetTimeFirst();
	const auto l_last = m_spikeClassListBox.GetTimeLast();

	DlgSpikeEdit dlg;
	dlg.m_yextent = m_spikeClassListBox.GetYWExtent(); 
	dlg.m_yzero = m_spikeClassListBox.GetYWOrg(); 
	dlg.m_xextent = m_spikeClassListBox.GetXWExtent(); 
	dlg.m_xzero = m_spikeClassListBox.GetXWOrg();
	dlg.m_spike_index = m_spike_index; 
	dlg.m_parent = this;
	dlg.m_pdbWaveDoc = GetDocument();

	// open dialog box and wait for response
	dlg.DoModal();
	if (!dlg.m_bartefact)
		m_spike_index = dlg.m_spike_index;

	if (dlg.m_bchanged)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
		saveCurrentSpkFile();
		update_spike_file(TRUE);
	}
	m_lFirst = l_first;
	m_lLast = l_last;
	update_data_file(TRUE);
	update_legends(TRUE);
	// display data
	m_spikeClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

void ViewSpikes::PrintComputePageSize()
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
	options_viewdata->horzRes = dc.GetDeviceCaps(HORZRES);
	options_viewdata->vertRes = dc.GetDeviceCaps(VERTRES);

	// margins (pixels)
	m_printRect.right = options_viewdata->horzRes - options_viewdata->rightPageMargin;
	m_printRect.bottom = options_viewdata->vertRes - options_viewdata->bottomPageMargin;
	m_printRect.left = options_viewdata->leftPageMargin;
	m_printRect.top = options_viewdata->topPageMargin;
}

void ViewSpikes::PrintFileBottomPage(CDC* p_dc, const CPrintInfo* pInfo)
{
	const auto t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"), // %d:%d",
	          pInfo->m_nCurPage, pInfo->GetMaxPage(),
	          t.GetDay(), t.GetMonth(), t.GetYear());
	const auto ch_date = GetDocument()->db_get_current_spk_file_name();
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_viewdata->horzRes / 2, options_viewdata->vertRes - 57, ch_date);
}

CString ViewSpikes::PrintConvertFileIndex(long l_first, long l_last)
{
	CString cs_unit = _T(" s");
	int constexpr array_size = 64;
	TCHAR sz_dest[array_size];
	constexpr size_t cbDest = array_size * sizeof(TCHAR);

	float x_scale_factor;
	auto x = PrintChangeUnit(
		static_cast<float>(l_first) / m_pSpkDoc->get_acq_rate(), &cs_unit, &x_scale_factor);
	auto fraction = static_cast<int>((x - floorf(x)) * static_cast<float>(1000.));
	StringCbPrintf(sz_dest, cbDest, TEXT("time = %i.%03.3i - "), static_cast<int>(x), fraction);
	CString cs_comment = sz_dest;

	x = static_cast<float>(l_last) / (m_pSpkDoc->get_acq_rate() * x_scale_factor); 
	fraction = static_cast<int>((x - floorf(x)) * static_cast<float>(1000.));
	StringCbPrintf(sz_dest, cbDest, _T("%f.%03.3i %s"), floorf(x), fraction, static_cast<LPCTSTR>(cs_unit));
	StringCbPrintf(sz_dest, cbDest, _T("%f.%03.3i %s"), floorf(x), fraction, static_cast<LPCTSTR>(cs_unit));
	cs_comment += sz_dest;
	return cs_comment;
}

long ViewSpikes::PrintGetFileSeriesIndexFromPage(const int page, int* file_number)
{
	auto l_first = m_lprintFirst;

	const auto max_row = m_nbrowsperpage * page; 
	auto i_file = 0; 
	if (options_viewdata->bPrintSelection)
		i_file = m_file0;
	const auto current = GetDocument()->db_get_current_record_position();
	GetDocument()->db_set_current_record_position(i_file);
	auto very_last = GetDocument()->db_get_data_len() - 1;
	for (auto row = 0; row < max_row; row++)
	{
		l_first += m_lprintLen; // end of row
		if (l_first >= very_last) // next file ?
		{
			i_file++; // next file index
			if (i_file > m_nfiles) // last file ??
			{
				i_file--;
				break;
			}
			// update end-of-file
			GetDocument()->db_move_next();
			very_last = GetDocument()->db_get_data_len() - 1;
			l_first = m_lprintFirst;
		}
	}
	*file_number = i_file; // return index / file list
	GetDocument()->db_set_current_record_position(current);
	return l_first; // return index first point / data file
}

CString ViewSpikes::PrintGetFileInfos()
{
	CString str_comment; // scratch pad
	const CString tab("    "); // use 4 spaces as tabulation character
	const CString rc("\n"); // next line
	const auto p_wave_format = &m_pSpkDoc->m_wave_format; // get data description

	// document's name, date and time
	if (options_viewdata->bDocName || options_viewdata->bAcqDateTime) // print doc infos?
	{
		if (options_viewdata->bDocName) // print file name
			str_comment += GetDocument()->db_get_current_spk_file_name(FALSE) + tab;
		if (options_viewdata->bAcqDateTime) // print data acquisition date & time
		{
			const auto acquisition_time = m_pSpkDoc->get_acq_time();
			const auto date = acquisition_time.Format(_T("%#d %m %Y %X")); //("%x %X");
			// or more explicitly %d-%b-%Y %H:%M:%S");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_viewdata->bAcqComment)
	{
		str_comment += p_wave_format->get_comments(_T(" ")); // cs_comment
		str_comment += rc;
	}

	return str_comment;
}

CString ViewSpikes::PrintBars(CDC* p_dc, const CRect* rect)
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
	const int ii_first = m_spikeClassListBox.GetTimeFirst();
	const int ii_last = m_spikeClassListBox.GetTimeLast();
	auto cs_comment = PrintConvertFileIndex(ii_first, ii_last);

	///// horizontal time bar ///////////////////////////
	if (options_viewdata->bTimeScaleBar)
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
	if (options_viewdata->bVoltageScaleBar)
	{
		constexpr auto vertical_bar = 100;
		rect_vertical_bar.left = rect->left + bar_origin.x;
		rect_vertical_bar.right = rect_vertical_bar.left - bar_size.cx;
		rect_vertical_bar.bottom = rect->bottom - bar_origin.y;
		rect_vertical_bar.top = rect_vertical_bar.bottom - vertical_bar;
		p_dc->Rectangle(&rect_vertical_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_viewdata->bChansComment || options_viewdata->bVoltageScaleBar || options_viewdata->bChanSettings)
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

BOOL ViewSpikes::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
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
	m_ChartDataWnd.Serialize(ar);
	//spk_bar_wnd_.Serialize(ar);
	//m_spkShapeView.Serialize(ar);
	ar.Close(); // close archive

	// printing margins
	if (options_viewdata->vertRes <= 0 || options_viewdata->horzRes <= 0
		|| options_viewdata->horzRes != pInfo->m_rectDraw.Width()
		|| options_viewdata->vertRes != pInfo->m_rectDraw.Height())
		PrintComputePageSize();

	// how many rows per page?
	const auto size_row = options_viewdata->HeightDoc + options_viewdata->heightSeparator;
	m_nbrowsperpage = m_printRect.Height() / size_row;
	if (m_nbrowsperpage == 0) // prevent zero pages
		m_nbrowsperpage = 1;

	// compute number of rows according to multiple row flag
	m_lprintFirst = m_spikeClassListBox.GetTimeFirst();
	m_lprintLen = m_spikeClassListBox.GetTimeLast() - m_lprintFirst + 1;

	// make sure the number of classes per file is known
	auto nn_classes = 0; // store sum (n classes from file (i=i_file0, i_file1))
	const auto p_dbwave_doc = GetDocument();
	m_file0 = p_dbwave_doc->db_get_current_record_position();
	ASSERT(m_file0 >= 0);
	m_printFirst = m_file0;
	m_printLast = m_file0;
	m_nfiles = 1;

	if (!options_viewdata->bPrintSelection)
	{
		m_printFirst = 0;
		m_nfiles = p_dbwave_doc->db_get_n_records();
		m_printLast = m_nfiles - 1;
	}

	// update the nb of classes per file selected and add this number
	m_max_classes = 1;
	p_dbwave_doc->db_set_current_record_position(m_printFirst);
	auto nb_rect = 0; // total nb of rows
	for (auto i = m_printFirst; i <= m_printLast; i++, p_dbwave_doc->db_move_next())
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

		if (p_dbwave_doc->get_db_n_spike_classes() > m_max_classes)
			m_max_classes = p_dbwave_doc->get_db_n_spike_classes();

		if (options_viewdata->bMultirowDisplay)
		{
			const auto len = p_dbwave_doc->db_get_data_len() - m_lprintFirst; // file length
			auto n_rows = len / m_lprintLen; // how many rows for this file?
			if (len > n_rows * m_lprintLen) // remainder?
				n_rows++;
			nb_rect += static_cast<int>(n_rows); // update nb of rows
		}
	}

	// multiple rows?
	if (!options_viewdata->bMultirowDisplay)
		nb_rect = m_nfiles;

	// n pages
	auto n_pages = nb_rect / m_nbrowsperpage;
	if (nb_rect > m_nbrowsperpage * n_pages)
		n_pages++;

	//------------------------------------------------------
	pInfo->SetMaxPage(n_pages); //one page printing/preview
	pInfo->m_nNumPreviewPages = 1; // preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (options_viewdata->bPrintSelection)
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION; // set button to selection

	// call dialog box
	const auto flag = DoPreparePrinting(pInfo);
	// set max nb of pages according to selection
	options_viewdata->bPrintSelection = pInfo->m_pPD->PrintSelection();
	if (options_viewdata->bPrintSelection)
	{
		n_pages = 1;
		m_nfiles = 1;
		if (options_viewdata->bMultirowDisplay)
		{
			const auto l_first0 = m_spikeClassListBox.GetTimeFirst();
			const auto l_last0 = m_spikeClassListBox.GetTimeLast();
			const auto len = m_pSpkDoc->get_acq_size() - l_first0;
			nb_rect = len / (l_last0 - l_first0);
			if (nb_rect * (l_last0 - l_first0) < len)
				nb_rect++;

			n_pages = nb_rect / m_nbrowsperpage;
			if (n_pages * m_nbrowsperpage < nb_rect)
				n_pages++;
		}
		pInfo->SetMaxPage(n_pages);
	}

	p_dbwave_doc->db_set_current_record_position(m_file0);
	return flag;
}

void ViewSpikes::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_spikeClassListBox.GetTimeFirst();
	m_lLast0 = m_spikeClassListBox.GetTimeLast();

	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT)); 
	lstrcpy(m_logFont.lfFaceName, _T("Arial"));
	m_logFont.lfHeight = options_viewdata->fontsize; 
	m_pOldFont = nullptr;
	m_fontPrint.CreateFontIndirect(&m_logFont);
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewSpikes::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_pOldFont = p_dc->SelectObject(&m_fontPrint);
	p_dc->SetMapMode(MM_TEXT); 
	PrintFileBottomPage(p_dc, pInfo); 
	const int current_page = static_cast<int>(pInfo->m_nCurPage); 

	// --------------------- load data corresponding to the first row of current page

	// print only current selection - transform current page into file index
	int file_index; 
	auto l_first = PrintGetFileSeriesIndexFromPage(current_page - 1, &file_index);
	GetDocument()->db_set_current_record_position(file_index);
	update_file_parameters(FALSE);
	update_file_scroll();
	auto very_last = m_pSpkDoc->get_acq_size() - 1; 

	CRect r_where(m_printRect.left,
	              m_printRect.top, 
	              m_printRect.left + options_viewdata->WidthDoc,
	              m_printRect.top + options_viewdata->HeightDoc);

	// loop through all files	--------------------------------------------------------
	for (int i = 0; i < m_nbrowsperpage; i++)
	{
		// save conditions (Save/RestoreDC is mandatory!) --------------------------------

		const auto old_dc = p_dc->SaveDC(); // save DC

		// set first rectangle where data will be printed

		auto comment_rect = r_where; // save RWhere for comments
		p_dc->SetMapMode(MM_TEXT); // 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); // set text align mode
		if (options_viewdata->bFrameRect) // print rectangle if necessary
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
		auto r_height = rw2.Height() / m_max_classes; // ncount;
		if (p_data_doc_ != nullptr)
			r_height = rw2.Height() / (m_max_classes + 1);
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
		auto l_last = l_first + m_lprintLen; // compute last pt to load
		if (l_last > very_last) // check end across file length
			l_last = very_last;
		if ((l_last - l_first + 1) < m_lprintLen) // adjust rect to length of data
		{
			rw_bars.right = MulDiv(rw_bars.Width(), l_last - l_first, m_lprintLen)
				+ rw_bars.left;
			ASSERT(rw_bars.right > rw_bars.left);
		}

		// ------------------------ print data

		auto extent = m_spikeClassListBox.GetYWExtent(); // get current extents
		//auto zero = m_spkClass.GetYWOrg();

		if (p_data_doc_ != nullptr)
		{
			if (options_viewdata->bClipRect) // clip curve display ?
				p_dc->IntersectClipRect(&rw_bars); // yes
			m_ChartDataWnd.GetDataFromDoc(l_first, l_last); // load data from file
			m_ChartDataWnd.CenterChan(0);
			m_ChartDataWnd.Print(p_dc, &rw_bars); // print data
			p_dc->SelectClipRgn(nullptr);

			extent = m_ChartDataWnd.get_channel_list_item(0)->GetYextent();
			rw_bars.top = rw_bars.bottom;
		}

		// ------------------------ print spikes

		rw_bars.bottom = rw_bars.top + r_height; // adjust the rect position
		rw_spikes.top = rw_bars.top;
		rw_spikes.bottom = rw_bars.bottom;
		rw_text.top = rw_bars.top;
		rw_text.bottom = rw_bars.bottom;

		//m_pSpkList = m_pSpkDoc->GetSpkList_Current();
		//AdjustYZoomToMaxMin(true); 
		short max, min;
		m_pSpkDoc->get_spike_list_current()->get_total_max_min(TRUE, &max, &min);
		const int middle = (static_cast<int>(max) + static_cast<int>(min)) / 2;
		m_spikeClassListBox.SetYzoom(extent, middle);
		const auto n_count = m_spikeClassListBox.GetCount(); 

		for (auto i_count = 0; i_count < n_count; i_count++)
		{
			m_spikeClassListBox.SetTimeIntervals(l_first, l_last);
			m_spikeClassListBox.PrintItem(p_dc, &rw_text, &rw_spikes, &rw_bars, i_count);
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
		r_where.OffsetRect(0, options_viewdata->HeightDoc + options_viewdata->heightSeparator);

		// restore DC ------------------------------------------------------

		p_dc->RestoreDC(old_dc); // restore Display context

		// print comments --------------------------------------------------

		p_dc->SetMapMode(MM_TEXT); // 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr); // no more clipping
		p_dc->SetViewportOrg(0, 0); // org = 0,0

		// print comments according to row within file
		CString cs_comment;
		if (l_first == m_lprintFirst) // first row = full comment
		{
			cs_comment += PrintGetFileInfos();
			cs_comment += PrintBars(p_dc, &comment_rect); // bars and bar legends
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_viewdata->textseparator + comment_rect.Width(), 0);
		comment_rect.right = m_printRect.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect,
		               DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);
		p_dc->SetTextAlign(ui_flag);

		// update file parameters for next row --------------------------------------------
		l_first += m_lprintLen;
		// next file?
		// if index next point is past the end of the file
		// OR not entire record and not multiple row display
		if ((l_first >= very_last)
			|| (!options_viewdata->bEntireRecord &&
				!options_viewdata->bMultirowDisplay))
		{
			file_index++; // next index
			if (file_index < m_nfiles) // last file ??
			{
				// NO: select new file
				GetDocument()->db_move_next();
				update_file_parameters(FALSE);
				update_file_scroll();
				very_last = m_pSpkDoc->get_acq_size() - 1;
			}
			else
				i = m_nbrowsperpage; // YES: break
			l_first = m_lprintFirst;
		}
	} // this is the end of a very long for loop.....................

	if (m_pOldFont != nullptr)
		p_dc->SelectObject(m_pOldFont);
}

void ViewSpikes::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	m_bIsPrinting = FALSE;

	GetDocument()->db_set_current_record_position(m_file0);
	update_file_parameters(TRUE);
	m_spikeClassListBox.SetTimeIntervals(m_lFirst0, m_lLast0);
	m_spikeClassListBox.Invalidate();

	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	if (p_app->m_p_view_spikes_memory_file != nullptr)
	{
		CArchive ar(p_app->m_p_view_spikes_memory_file, CArchive::load);
		p_app->m_p_view_spikes_memory_file->SeekToBegin();
		m_ChartDataWnd.Serialize(ar);
		ar.Close(); // close archive
	}
}

static char vs_units[] = {"GM  mµpf  "};  
static int vs_units_power[] = {9, 6, 0, 0, -3, -6, -9, -12, 0};
static int vsmax_index = 8; 

float ViewSpikes::PrintChangeUnit(float xVal, CString* xUnit, float* xScalefactor)
{
	if (xVal == 0.f) 
	{
		*xScalefactor = 1.0f;
		return 0.0f;
	}

	short i;
	short i_sign = 1;
	if (xVal < 0)
	{
		i_sign = -1;
		xVal = -xVal;
	}
	const auto ip_rec = static_cast<short> (floor(std::log10(xVal))); 
	if (ip_rec <= 0 && xVal < 1.) 
		i = 4 - ip_rec / 3; 
	else
		i = 3 - ip_rec / 3;

	if (i > vsmax_index) 
		i = vsmax_index;
	else if (i < 0) 
		i = 0;
	*xScalefactor = static_cast<float>(pow(10.0f, vs_units_power[i])); // convert & store
	xUnit->SetAt(0, vs_units[i]); 
	return xVal * static_cast<float>(i_sign) / *xScalefactor; 
}

void ViewSpikes::center_data_display_on_spike(const int spike_no)
{
	// test if spike visible in the current time interval
	const Spike* spike = m_pSpkList->get_spike(spike_no);
	const long spk_first = spike->get_time() - m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	const long spk_last = spk_first + m_pSpkList->get_spike_length();
	const long spk_center = (spk_last + spk_first) / 2;
	if (spk_first < m_lFirst || spk_last > m_lLast)
	{
		const long span = (m_lLast - m_lFirst) / 2;
		m_lFirst = spk_center - span;
		m_lLast = spk_center + span;
		update_legends(TRUE);
	}

	// center curve vertically
	CChanlistItem* chan = m_ChartDataWnd.get_channel_list_item(0);
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
	m_spikeClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
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
			m_file_scroll.GetScrollInfo(&m_file_scroll_infos, SIF_ALL);
			const long l_first = m_file_scroll_infos.nPos;
			const long l_last = l_first + static_cast<long>(m_file_scroll_infos.nPage) - 1;
			m_ChartDataWnd.GetDataFromDoc(l_first, l_last);
		}
		break;

	default:
		scroll_file(nSBCode, nPos);
		break;
	}

	// adjust display
	m_lFirst = m_ChartDataWnd.GetDataFirstIndex();
	m_lLast = m_ChartDataWnd.GetDataLastIndex();
	update_legends(TRUE);
	m_spikeClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();

	if (p_data_doc_ != nullptr)
		m_ChartDataWnd.CenterChan(0);
}

void ViewSpikes::scroll_file(UINT nSBCode, UINT nPos)
{
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: // scroll to the start
	case SB_LINELEFT: // scroll one line left
	case SB_LINERIGHT: // scroll one line right
	case SB_PAGELEFT: // scroll one page left
	case SB_PAGERIGHT: // scroll one page right
	case SB_RIGHT: // scroll to end right
		m_ChartDataWnd.ScrollDataFromDoc(nSBCode);
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		m_ChartDataWnd.GetDataFromDoc(MulDiv(nPos, m_ChartDataWnd.GetDocumentLast(), 100));
		break;
	default: // NOP: set position only
		break;
	}
}

void ViewSpikes::update_file_scroll()
{
	m_file_scroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_file_scroll_infos.nPos = m_ChartDataWnd.GetDataFirstIndex();
	m_file_scroll_infos.nPage = m_ChartDataWnd.GetDataLastIndex() - m_ChartDataWnd.GetDataFirstIndex() + 1;
	m_file_scroll.SetScrollInfo(&m_file_scroll_infos);
}

void ViewSpikes::OnEditCopy()
{
	DlgCopyAs dlg;
	dlg.m_nabcissa = options_viewdata->hzResolution;
	dlg.m_nordinates = options_viewdata->vtResolution;
	dlg.m_bgraphics = options_viewdata->bgraphics;
	dlg.m_ioption = options_viewdata->bcontours;
	dlg.m_iunit = options_viewdata->bunits;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		options_viewdata->bgraphics = dlg.m_bgraphics;
		options_viewdata->bcontours = dlg.m_ioption;
		options_viewdata->bunits = dlg.m_iunit;
		options_viewdata->hzResolution = dlg.m_nabcissa;
		options_viewdata->vtResolution = dlg.m_nordinates;

		// output rectangle requested by user
		CRect rect(0, 0, options_viewdata->hzResolution, options_viewdata->vtResolution);

		// create metafile
		CMetaFileDC mDC;
		auto rect_bound = rect;
		rect_bound.right *= 32;
		rect_bound.bottom *= 30;
		auto p_dc_ref = GetDC();
		const auto old_dc = p_dc_ref->SaveDC(); // save DC

		auto cs_title = _T("dbWave\0") + GetDocument()->GetTitle();
		cs_title += _T("\0\0");
		const auto hm_dc = mDC.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
		ASSERT(hm_dc != NULL);

		// Draw document in metafile.
		CPen black_pen(PS_SOLID, 0, RGB(0, 0, 0));
		const auto old_pen = mDC.SelectObject(&black_pen);
		if (!static_cast<CBrush*>(mDC.SelectStockObject(BLACK_BRUSH)))
			return;
		CClientDC attribute_dc(this); 
		mDC.SetAttribDC(attribute_dc.GetSafeHdc()); 

		// print comments : set font
		m_pOldFont = nullptr;
		const auto oldsize = options_viewdata->fontsize;
		options_viewdata->fontsize = 10;
		memset(&m_logFont, 0, sizeof(LOGFONT)); 
		lstrcpy(m_logFont.lfFaceName, _T("Arial")); 
		m_logFont.lfHeight = options_viewdata->fontsize; 
		m_pOldFont = nullptr;
		m_fontPrint.CreateFontIndirect(&m_logFont);
		mDC.SetBkMode(TRANSPARENT);

		options_viewdata->fontsize = oldsize;
		m_pOldFont = mDC.SelectObject(&m_fontPrint);

		CString comments;
		// display data: source data and spikes
		//auto extent = m_ChartSpikesListBox.GetYWExtent(); 
		const auto r_height = MulDiv(m_spikeClassListBox.GetRowHeight(), rect.Width(),
		                            m_spikeClassListBox.GetColumnsTimeWidth());
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
			m_ChartDataWnd.CenterChan(0);
			m_ChartDataWnd.Print(&mDC, &rw_bars);

			//auto extent = m_ChartDataWnd.get_channel_list_item(0)->GetYextent();
			rw_spikes.OffsetRect(0, r_height);
			rw_bars.OffsetRect(0, r_height);
			rw_text.OffsetRect(0, r_height);
		}

		// display spikes and bars
		adjust_y_zoom_to_max_min(true);
		const auto n_count = m_spikeClassListBox.GetCount(); // get nb of items in this file

		for (int i_count = 0; i_count < n_count; i_count++)
		{
			m_spikeClassListBox.PrintItem(&mDC, &rw_text, &rw_spikes, &rw_bars, i_count);
			rw_spikes.OffsetRect(0, r_height);
			rw_bars.OffsetRect(0, r_height);
			rw_text.OffsetRect(0, r_height);
		}

		if (m_pOldFont != nullptr)
			mDC.SelectObject(m_pOldFont);
		m_fontPrint.DeleteObject();

		// restore oldpen
		mDC.SelectObject(old_pen);
		ReleaseDC(p_dc_ref);

		// Close metafile
		const auto h_emf_tmp = mDC.CloseEnhanced();
		ASSERT(h_emf_tmp != NULL);
		if (OpenClipboard())
		{
			EmptyClipboard(); // prepare clipboard
			SetClipboardData(CF_ENHMETAFILE, h_emf_tmp); // put data
			CloseClipboard(); // close clipboard
		}
		else
		{
			// Someone else has the Clipboard open...
			DeleteEnhMetaFile(h_emf_tmp); // delete data
			MessageBeep(0); // tell user something is wrong!
			AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
		}

		// restore initial conditions
		p_dc_ref->RestoreDC(old_dc); // restore Display context
	}

	// restore screen in previous state
	update_spike_file(TRUE);
	update_file_scroll();
	m_spikeClassListBox.Invalidate();
	if (p_data_doc_ != nullptr)
	{
		m_ChartDataWnd.GetDataFromDoc(m_lFirst, m_lLast);
		m_ChartDataWnd.ResizeChannels(m_ChartDataWnd.get_rect_width(), m_lLast - m_lFirst);
		m_ChartDataWnd.Invalidate();
	}
}

void ViewSpikes::OnZoom()
{
	int ii_start = 0;
	if (m_spike_index != -1)
	{
		ii_start = m_pSpkList->get_spike(m_spike_index)->get_time();
		const int delta = static_cast<int>(m_zoom * m_pSpkDoc->get_acq_rate());
		ii_start -= delta/2;
	}
	zoom_on_preset_interval(ii_start);
}

void ViewSpikes::zoom_on_preset_interval(int ii_start)
{
	if (ii_start < 0)
		ii_start = 0;
	m_lFirst = ii_start;
	const auto acqrate = m_pSpkDoc->get_acq_rate();
	m_lLast = static_cast<long>((m_lFirst / acqrate + m_zoom) * acqrate);
	update_legends(TRUE);
	// display data
	m_spikeClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
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

void ViewSpikes::set_v_bar_mode(short bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	update_bias_scroll();
}

void ViewSpikes::update_gain_scroll()
{
	m_scroll_y.SetScrollPos(
		MulDiv(m_ChartDataWnd.get_channel_list_item(0)->GetYextent(),
		       100,
		       YEXTENT_MAX)
		+ 50,
		TRUE);
}

void ViewSpikes::scroll_gain(UINT nSBCode, UINT nPos)
{
	int lSize = m_ChartDataWnd.get_channel_list_item(0)->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	// .................scroll to the start
	case SB_LEFT: lSize = YEXTENT_MIN;
		break;
	// .................scroll one line left
	case SB_LINELEFT: lSize -= lSize / 10 + 1;
		break;
	// .................scroll one line right
	case SB_LINERIGHT: lSize += lSize / 10 + 1;
		break;
	// .................scroll one page left
	case SB_PAGELEFT: lSize -= lSize / 2 + 1;
		break;
	// .................scroll one page right
	case SB_PAGERIGHT: lSize += lSize + 1;
		break;
	// .................scroll to end right
	case SB_RIGHT: lSize = YEXTENT_MAX;
		break;
	// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: lSize = MulDiv(static_cast<int>(nPos) - 50, YEXTENT_MAX, 100);
		break;
	// .................NOP: set position only
	default: break;
	}

	// change y extent
	if (lSize > 0) //&& lSize<=YEXTENT_MAX)
	{
		m_ChartDataWnd.get_channel_list_item(0)->SetYextent(lSize);
		update_legends(TRUE);
		m_ChartDataWnd.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		update_gain_scroll();
}

void ViewSpikes::update_bias_scroll()
{
	CChanlistItem* pchan = m_ChartDataWnd.get_channel_list_item(0);
	const auto i_pos = (pchan->GetYzero()
			- pchan->GetDataBinZero())
		* 100 / static_cast<int>(YZERO_SPAN) + 50;
	m_scroll_y.SetScrollPos(i_pos, TRUE);
}

void ViewSpikes::scroll_bias(UINT nSBCode, UINT nPos)
{
	CChanlistItem* chan = m_ChartDataWnd.get_channel_list_item(0);
	auto l_size = chan->GetYzero() - chan->GetDataBinZero();
	const auto y_extent = chan->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: // scroll to the start
		l_size = YZERO_MIN;
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
		l_size = YZERO_MAX;
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		l_size = MulDiv(static_cast<int>(nPos) - 50, YZERO_SPAN, 100);
		break;
	default: // NOP: set position only
		break;
	}

	// try to read data with this new size
	if (l_size > YZERO_MIN && l_size < YZERO_MAX)
	{
		chan->SetYzero(l_size + chan->GetDataBinZero());
		m_ChartDataWnd.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		update_bias_scroll();
}

void ViewSpikes::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != nullptr)
	{
		// CViewData scroll: vertical scroll bar
		switch (m_VBarMode)
		{
		case BAR_GAIN:
			scroll_gain(nSBCode, nPos);
			break;
		case BAR_BIAS:
			scroll_bias(nSBCode, nPos);
		default:
			break;
		}
	}
	dbTableView::OnVScroll(nSBCode, nPos, pScrollBar);
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
		// if artefact: set class to negative value
		if (m_b_artefact && spk_class >= 0)
			spk_class = -(spk_class + 1);
		// if not artefact: if spike has negative class, set to positive value
		else if (spk_class < 0)
			spk_class = -(spk_class + 1);
		//m_pSpkList->get_spike(m_spike_index)->set_class_id(spk_class);
		m_spikeClassListBox.ChangeSpikeClass(m_spike_index, spk_class);
	}
	CheckDlgButton(IDC_ARTEFACT, m_b_artefact);
	m_pSpkDoc->SetModifiedFlag(TRUE);
	update_legends(TRUE);
	m_spikeClassListBox.Invalidate();
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
	m_lFirst = 0;
	m_lLast = m_pSpkDoc->get_acq_size() - 1;
	// spikes: center spikes horizontally and adjust hz size of display
	constexpr int x_wo = 0;
	const auto x_we = m_pSpkList->get_spike_length();
	m_spikeClassListBox.SetXzoom(x_we, x_wo);

	update_legends(TRUE);

	m_spikeClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

void ViewSpikes::OnFormatCentercurve()
{
	// TODO 

	// loop over all spikes of the list
	const int n_spikes = m_pSpkList->get_spikes_count();
	const auto i_t1 = m_psC->i_left;
	const auto i_t2 = m_psC->i_right;
	for (int i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		Spike* spike = m_pSpkList->get_spike(i_spike);
		spike->center_spike_amplitude(i_t1, i_t2, 1);
	}

	short max, min;
	m_pSpkList->get_total_max_min(TRUE, &max, &min);
	const WORD middle = max / 2 + min / 2;
	m_spikeClassListBox.SetYzoom(m_spikeClassListBox.GetYWExtent(), middle);

	if (p_data_doc_ != nullptr)
		m_ChartDataWnd.CenterChan(0);

	update_legends(TRUE);
	m_spikeClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

void ViewSpikes::OnFormatGainadjust()
{
	if (m_pSpkDoc != nullptr)
	{
		adjust_y_zoom_to_max_min(true);
	}
	if (p_data_doc_ != nullptr)
		m_ChartDataWnd.MaxgainChan(0);

	update_legends(TRUE);
	m_spikeClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

void ViewSpikes::OnFormatPreviousframe()
{
	zoom_on_preset_interval(m_lFirst * 2 - m_lLast);
}

void ViewSpikes::OnFormatNextframe()
{
	const long len = m_lLast - m_lFirst;
	auto last = m_lLast + len;
	if (last > m_pSpkDoc->get_acq_size())
		last = m_lLast - len;
	zoom_on_preset_interval(last);
}


void ViewSpikes::OnEnChangeSpikenoclass()
{
	if (!mm_spike_class.m_bEntryDone)
		return;
	const auto spike_class_old = m_spike_class;
	mm_spike_class.OnEnChange(this, m_spike_class, 1, -1);

	if (m_spike_class != spike_class_old)
	{
		m_spikeClassListBox.ChangeSpikeClass(m_spike_index, m_spike_class);
		m_pSpkDoc->SetModifiedFlag(TRUE);
		update_legends(TRUE);
		m_spikeClassListBox.Invalidate();
		m_ChartDataWnd.Invalidate();
	}
}

void ViewSpikes::OnEnChangeTimefirst()
{
	if (mm_time_first.m_bEntryDone)
	{
		mm_time_first.OnEnChange(this, m_time_first, 1.f, -1.f);
		const auto l_first = static_cast<long>(m_time_first * m_pSpkDoc->get_acq_rate());
		if (l_first != m_lFirst)
		{
			m_lFirst = l_first;
			update_legends(TRUE);
			m_spikeClassListBox.Invalidate();
			m_ChartDataWnd.Invalidate();
		}
	}
}

void ViewSpikes::OnEnChangeTimelast()
{
	if (mm_time_last.m_bEntryDone)
	{
		mm_time_last.OnEnChange(this, m_time_last, 1.f, -1.f);
		const auto l_last = static_cast<long>(m_time_last * m_pSpkDoc->get_acq_rate());
		if (l_last != m_lLast)
		{
			m_lLast = l_last;
			update_legends(TRUE);
			m_spikeClassListBox.Invalidate();
			m_ChartDataWnd.Invalidate();
		}
	}
}

void ViewSpikes::OnEnChangeZoom()
{
	if (mm_zoom.m_bEntryDone)
	{
		const auto zoom = m_zoom;
		mm_zoom.OnEnChange(this, m_zoom, 1.f, -1.f);

		// check boundaries
		if (m_zoom < 0.0f)
			m_zoom = 1.0f;

		if (m_zoom != zoom)
			zoom_on_preset_interval(0);
		else
			UpdateData(FALSE);
	}
}

void ViewSpikes::OnEnChangeSourceclass()
{
	if (mm_class_source.m_bEntryDone)
	{
		mm_class_source.OnEnChange(this, m_class_source, 1, -1);
		UpdateData(FALSE);
	}
}

void ViewSpikes::OnEnChangeDestclass()
{
	if (mm_class_destination.m_bEntryDone)
	{
		mm_class_destination.OnEnChange(this, m_class_destination, 1, -1);
		UpdateData(FALSE);
	}
}

void ViewSpikes::OnEnChangeJitter()
{
	if (mm_jitter_ms.m_bEntryDone)
	{
		mm_jitter_ms.OnEnChange(this, m_jitter_ms, 1.f, -1.f);
		UpdateData(FALSE);
	}
}

void ViewSpikes::OnEnChangeNOspike()
{
	if (mm_spike_index.m_bEntryDone)
	{
		const auto spike_no = m_spike_index;
		const int delta_up = m_pSpkList->get_next_spike(spike_no, 1, m_b_keep_same_class) - m_spike_index;
		const int delta_down = m_spike_index - m_pSpkList->get_next_spike(spike_no, -1, m_b_keep_same_class);
		mm_spike_index.OnEnChange(this, m_spike_index, delta_up, -delta_down);

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


