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

#include "CNiceUnit.h"
#include "DlgCopyAs.h"
#include "DlgDataSeries.h"
#include "DlgDataSeriesFormat.h"
#include "DlgDataViewAbcissa.h"
#include "DlgDataViewOrdinates.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewData, dbTableView)

BEGIN_MESSAGE_MAP(ViewData, dbTableView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BIAS_button, &ViewData::OnClickedBias)
	ON_BN_CLICKED(IDC_GAIN_button, &ViewData::OnClickedGain)
	ON_WM_VSCROLL()
	ON_COMMAND(ID_FORMAT_XSCALE, &ViewData::OnFormatXscale)
	ON_COMMAND(ID_FORMAT_SETORDINATES, &ViewData::OnFormatYscale)
	ON_COMMAND(ID_EDIT_COPY, &ViewData::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &ViewData::OnUpdateEditCopy)
	ON_COMMAND(ID_TOOLS_DATASERIES, &ViewData::OnToolsDataseries)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &ViewData::ADC_OnHardwareChannelsDlg)
	ON_COMMAND(ID_HARDWARE_ADINTERVALS, &ViewData::ADC_OnHardwareIntervalsDlg)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &ViewData::OnCenterCurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &ViewData::OnGainAdjustCurve)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, &ViewData::OnSplitCurves)
	ON_COMMAND(ID_FORMAT_FIRSTFRAME, &ViewData::OnFirstFrame)
	ON_COMMAND(ID_FORMAT_LASTFRAME, &ViewData::OnLastFrame)
	ON_MESSAGE(WM_MYMESSAGE, &ViewData::OnMyMessage)
	ON_COMMAND(ID_FORMAT_ALLDATA, &ViewData::OnViewAlldata)
	ON_COMMAND(ID_FORMAT_DATASERIESATTRIBUTES, &ViewData::OnFormatDataseriesattributes)
	ON_COMMAND(ID_TOOLS_MEASUREMODE, &ViewData::OnToolsMeasuremode)
	ON_COMMAND(ID_TOOLS_MEASURE, &ViewData::OnToolsMeasure)
	ON_COMMAND(ID_TOOLS_VERTICALTAGS, &ViewData::OnToolsVerticaltags)
	ON_COMMAND(ID_TOOLS_HORIZONTALCURSORS, &ViewData::OnToolsHorizontalcursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_HORIZONTALCURSORS, &ViewData::OnUpdateToolsHorizontalcursors)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_VERTICALTAGS, &ViewData::OnUpdateToolsVerticaltags)
	ON_WM_HSCROLL()
	ON_COMMAND(ID_HARDWARE_DEFINEEXPERIMENT, &ViewData::ADC_OnHardwareDefineexperiment)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewData::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewData::OnEnChangeTimelast)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_FORMAT_SETORDINATES, &ViewData::OnFormatYscale)
	ON_COMMAND(ID_FILE_PRINT, dbTableView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, dbTableView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, dbTableView::OnFilePrintPreview)
	ON_CBN_SELCHANGE(IDC_COMBOCHAN, &ViewData::OnCbnSelchangeCombochan)
END_MESSAGE_MAP()

ViewData::ViewData()
	: dbTableView(IDD)
{
}

ViewData::~ViewData()
{
	m_p_dat_Doc->acq_close_file();
	DeleteObject(m_hBias);
	DeleteObject(m_hZoom);
}

void ViewData::DoDataExchange(CDataExchange* pDX)
{
	// pass values
	dbTableView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, m_first_Hz_cursor);
	DDX_Text(pDX, IDC_EDIT2, m_second_Hz_cursor);
	DDX_Text(pDX, IDC_EDIT3, m_difference_second_minus_first);
	DDX_Text(pDX, IDC_TIMEFIRST, m_time_first_abcissa);
	DDX_Text(pDX, IDC_TIMELAST, m_time_last_abcissa);
	DDX_Control(pDX, IDC_FILESCROLL, m_file_scroll_bar);
	DDX_Control(pDX, IDC_COMBOCHAN, m_comboSelectChan);
}

void ViewData::define_sub_classed_items()
{
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	// bitmap buttons: load icons & set buttons
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hBias)));
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
	                                         reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hZoom)));

	VERIFY(m_ChartDataWnd.SubclassDlgItem(IDC_DISPLAY, this));
	VERIFY(mm_time_first_abcissa.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_time_last_abcissa.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(m_ADC_yRulerBar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_ADC_xRulerBar.SubclassDlgItem(IDC_XSCALE, this));
}

void ViewData::define_stretch_parameters()
{
	// save coordinates and properties of "always visible" controls
	m_stretch_.AttachParent(this); // attach form_view pointer
	m_stretch_.newProp(IDC_DISPLAY, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_COMBOCHAN, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch_.newProp(IDC_SCROLLY_scrollbar, SZEQ_XREQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_SOURCE, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_YSCALE, SZEQ_XLEQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_XSCALE, XLEQ_XREQ, SZEQ_YBEQ);
	m_b_init_ = TRUE;
}

void ViewData::OnInitialUpdate()
{
	define_sub_classed_items();

	m_ADC_yRulerBar.AttachScopeWnd(&m_ChartDataWnd, FALSE);
	m_ADC_xRulerBar.AttachScopeWnd(&m_ChartDataWnd, TRUE);
	m_ChartDataWnd.attach_external_x_ruler(&m_ADC_xRulerBar);
	m_ChartDataWnd.attach_external_y_ruler(&m_ADC_yRulerBar);
	m_ChartDataWnd.b_nice_grid = TRUE;

	define_stretch_parameters();

	// init relation with document, display data, adjust parameters
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	options_view_data = &(p_app->options_view_data);
	mdMO = &(p_app->options_view_data_measure);

	// set data file
	dbTableView::OnInitialUpdate();
	update_file_parameters(TRUE); 

	m_ChartDataWnd.set_scope_parameters(&(options_view_data->viewdata));
	constexpr int legends_options = UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE;
	m_bCommonScale = TRUE;
	m_comboSelectChan.SetCurSel(m_ChartDataWnd.get_channel_list_size());
	UpdateLegends(legends_options);
}

void ViewData::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_b_init_)
		return;

	auto i_update = NULL;
	switch (LOWORD(lHint))
	{
	case HINT_REPLACEVIEW:
		return;
	case HINT_CLOSEFILEMODIFIED: 
		save_modified_file();
		break;
	case HINT_DOCHASCHANGED: 
	case HINT_DOCMOVERECORD:
		m_bInitComment = TRUE;
		update_file_parameters();
		i_update = UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE;
		break;
	default:
		if (m_p_dat_Doc == nullptr)
			return;
		i_update = UPD_ABCISSA | UPD_XSCALE | UPD_ORDINATES | UPD_YSCALE;
		break;
	}
	UpdateLegends(i_update);
	m_ChartDataWnd.Invalidate();
	SetVBarMode(m_VBarMode);
}

void ViewData::UpdateLegends(int legends_options)
{
	if (!m_ChartDataWnd.IsDefined() && !m_b_valid_doc_)
		return;
	if (legends_options & UPD_ABCISSA)
		UpdateFileScroll();
	if (legends_options & CHG_YSCALE)
	{
		UpdateHZtagsVal();
		legends_options |= CHG_YBAR;
	}
	if (legends_options & UPD_YSCALE)
		legends_options |= CHG_YBAR;
	if (legends_options & CHG_YBAR)
		UpdateYZero(m_channel_selected, m_ChartDataWnd.get_channel_list_item(m_channel_selected)->GetYzero());

	UpdateData(FALSE);
}

void ViewData::OnClickedBias()
{
	// set bias down and set gain up CButton
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

void ViewData::OnClickedGain()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS_button))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

void ViewData::update_channel(const int channel)
{
	m_channel_selected = channel;
	if (m_channel_selected > m_ChartDataWnd.get_channel_list_size() - 1) 
		m_channel_selected = m_ChartDataWnd.get_channel_list_size() - 1; 
	else if (m_channel_selected < 0) 
		m_channel_selected = 0;

	if (m_channel_selected == channel)
		UpdateData(FALSE);
	else 
	{
		if (m_cursor_state == CURSOR_CROSS && mdMO->wOption == 1
			&& m_ChartDataWnd.horizontal_tags.get_tag_list_size() > 0)
		{
			for (auto i = 0; i < m_ChartDataWnd.horizontal_tags.get_tag_list_size(); i++)
				m_ChartDataWnd.horizontal_tags.set_tag_chan(i, m_channel_selected);
			UpdateHZtagsVal();
			m_ChartDataWnd.Invalidate();
		}
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
	}
}

void ViewData::OnFormatYscale()
{
	DlgDataViewOrdinates dlg;
	dlg.m_pChartDataWnd = &m_ChartDataWnd;
	dlg.m_Channel = m_channel_selected;
	if (IDOK == dlg.DoModal())
		UpdateLegends(UPD_ORDINATES | UPD_YSCALE | CHG_YBAR);

	m_ChartDataWnd.Invalidate();
}

void ViewData::OnToolsDataseries()
{
	// init dialog data
	DlgDataSeries dlg;
	dlg.m_pChartDataWnd = &m_ChartDataWnd;
	dlg.m_pdbDoc = m_p_dat_Doc;
	dlg.m_listindex = m_channel_selected;
	dlg.DoModal();

	m_channel_selected = dlg.m_listindex;
	UpdateLegends(UPD_YSCALE);
}

void ViewData::OnEditCopy()
{
	DlgCopyAs dlg;
	dlg.m_nabcissa = options_view_data->hzResolution;
	dlg.m_nordinates = options_view_data->vtResolution;
	dlg.m_bgraphics = options_view_data->bgraphics;
	dlg.m_ioption = options_view_data->bcontours;
	dlg.m_iunit = options_view_data->bunits;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		options_view_data->bgraphics = dlg.m_bgraphics;
		options_view_data->bcontours = dlg.m_ioption;
		options_view_data->bunits = dlg.m_iunit;
		options_view_data->hzResolution = dlg.m_nabcissa;
		options_view_data->vtResolution = dlg.m_nordinates;

		if (!dlg.m_bgraphics)
			m_ChartDataWnd.copy_as_text(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			CRect old_rect;
			m_ChartDataWnd.GetWindowRect(&old_rect);

			CRect rect(0, 0, options_view_data->hzResolution, options_view_data->vtResolution);
			m_pixels_count_0_ = m_ChartDataWnd.get_rect_width();

			// create metafile
			CMetaFileDC m_dc;
			const auto p_dc_ref = GetDC();
			auto cs_title = _T("dbWave\0") + m_p_dat_Doc->GetTitle();
			cs_title += _T("\0\0");
			const CRect rect_bound(0, 0, 21000, 29700); // dimensions in HIMETRIC units (in .01-millimeter increments)
			const auto hm_dc = m_dc.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
			ASSERT(hm_dc != NULL);

			// Draw document in metafile.
			const CClientDC attrib_dc(this); // Create and attach attribute DC
			m_dc.SetAttribDC(attrib_dc.GetSafeHdc()); // from current screen

			const auto old_scope_struct= new SCOPESTRUCT();
			SCOPESTRUCT* new_scope_struct = m_ChartDataWnd.get_scope_parameters();
			*old_scope_struct = *new_scope_struct;
			new_scope_struct->bDrawframe = options_view_data->bFrameRect;
			new_scope_struct->bClipRect = options_view_data->bClipRect;
			m_ChartDataWnd.Print(&m_dc, &rect);
			*new_scope_struct = *old_scope_struct;

			// print comments : set font
			memset(&m_log_font_, 0, sizeof(LOGFONT));
			GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &m_log_font_);
			m_p_old_font_ = nullptr;
			/*BOOL flag = */
			m_font_print_.CreateFontIndirect(&m_log_font_);
			m_p_old_font_ = m_dc.SelectObject(&m_font_print_);
			const int line_height = m_log_font_.lfHeight + 5;
			auto y_pixels_row = 0;
			constexpr auto x_column = 10;

			CString comments = _T("Abcissa: ");
			CString content;
			content.Format(_T("%g - %g s"), m_time_first_abcissa, m_time_last_abcissa);
			comments += content;
			m_dc.TextOut(x_column, y_pixels_row, comments);
			y_pixels_row += line_height;
			comments.Format(_T("Vertical bar (ch. 0) = %g mV"), m_ChartDataWnd.y_ruler.GetScaleIncrement());
			m_dc.TextOut(x_column, y_pixels_row, comments);
			y_pixels_row += line_height;
			comments.Format(_T("Horizontal bar = %g s"), m_ChartDataWnd.x_ruler.GetScaleIncrement());
			m_dc.TextOut(x_column, y_pixels_row, comments);
			y_pixels_row += line_height;

			// bars
			const auto p_old_brush = static_cast<CBrush*>(m_dc.SelectStockObject(BLACK_BRUSH));
			m_dc.MoveTo(0, y_pixels_row);
			const auto bottom = m_ChartDataWnd.y_ruler.GetScaleUnitPixels(rect.Height());
			m_dc.LineTo(0, y_pixels_row - bottom);
			m_dc.MoveTo(0, y_pixels_row);
			const auto left = m_ChartDataWnd.x_ruler.GetScaleUnitPixels(rect.Width());
			m_dc.LineTo(left, y_pixels_row);

			m_dc.SelectObject(p_old_brush);
			if (m_p_old_font_ != nullptr)
				m_dc.SelectObject(m_p_old_font_);
			m_font_print_.DeleteObject();

			// Close metafile
			ReleaseDC(p_dc_ref);
			const auto h_emf_tmp = m_dc.CloseEnhanced();
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
			m_ChartDataWnd.resize_channels(m_pixels_count_0_, 0);
			m_ChartDataWnd.get_data_from_doc();
			m_ChartDataWnd.Invalidate();
		}
	}
}

void ViewData::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ChartDataWnd.IsDefined() != NULL); // if document has data
}

void ViewData::ADC_OnHardwareChannelsDlg() 
{
	DlgADInputs dlg;
	dlg.m_pwFormat = m_p_dat_Doc->get_waveformat();
	dlg.m_pchArray = m_p_dat_Doc->get_wavechan_array();
	if (IDOK == dlg.DoModal())
	{
		if(m_p_dat_Doc->acq_save_data_descriptors())
			m_p_dat_Doc->SetModifiedFlag(TRUE);
	}
}

void ViewData::ADC_OnHardwareIntervalsDlg() 
{
	DlgADIntervals dlg;
	dlg.m_p_wave_format = m_p_dat_Doc->get_waveformat();
	if (IDOK == dlg.DoModal())
	{
		if (m_p_dat_Doc->acq_save_data_descriptors())
			m_p_dat_Doc->SetModifiedFlag(TRUE);
	}
}

void ViewData::chain_dialog(WORD i_id)
{
	WORD menu_id;
	switch (i_id)
	{
	case IDC_ADINTERVALS:
		menu_id = ID_HARDWARE_ADINTERVALS;
		break;
	case IDC_ADCHANNELS:
		menu_id = ID_HARDWARE_ADCHANNELS;
		break;
	default:
		return;
	}
	PostMessage(WM_COMMAND, menu_id, NULL);
}

void ViewData::OnFirstFrame()
{
	OnFileScroll(SB_LEFT, 1L);
}

void ViewData::OnLastFrame()
{
	OnFileScroll(SB_RIGHT, 1L);
}

void ViewData::update_file_parameters(const BOOL b_update_interface)
{
	// load parameters from document file: none yet loaded?
	const BOOL b_first_update = (m_p_dat_Doc == nullptr);
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
	m_p_dat_Doc = dbwave_doc->m_p_dat;
	m_p_dat_Doc->read_data_infos();
	const auto wave_format = m_p_dat_Doc->get_waveformat();

	if (b_first_update)
	{
		m_samplingRate = wave_format->sampling_rate_per_channel; 
		m_time_first_abcissa = 0.0f; 
		m_time_last_abcissa = static_cast<float>(m_p_dat_Doc->get_doc_channel_length()) / m_samplingRate;
	}

	// load parameters from current data file
	m_ChartDataWnd.AttachDataFile(m_p_dat_Doc);
	m_p_dat_Doc->SetModifiedFlag(FALSE);

	// OPTION: display entire file	--	(inactive if multirow)
	long l_first = 0;
	long l_last = m_p_dat_Doc->get_doc_channel_length() - 1;

	if (!options_view_data->bEntireRecord || options_view_data->bMultirowDisplay && !b_first_update)
	{
		l_first = static_cast<long>(m_time_first_abcissa * m_samplingRate);
		l_last = static_cast<long>(m_time_last_abcissa * m_samplingRate);
		if (l_last > m_p_dat_Doc->get_doc_channel_length() - 1) // last OK?
			l_last = m_p_dat_Doc->get_doc_channel_length() - 1; // clip to the end of the file
	}
	m_samplingRate = wave_format->sampling_rate_per_channel; // update sampling rate

	// display all channels
	auto chan_list_size = m_ChartDataWnd.get_channel_list_size();
	
	// display all channels (TRUE) / no : loop through all doc channels & add if necessary
	if (options_view_data->bAllChannels || chan_list_size == 0)
	{
		for (auto doc_channel = 0; doc_channel < wave_format->scan_count; doc_channel++)
		{
			auto b_present = FALSE;
			for (auto j = chan_list_size - 1; j >= 0; j--)
			{
				if ((b_present = (m_ChartDataWnd.get_channel_list_item(j)->GetSourceChan() == doc_channel)))
					break;
			}
			if (!b_present)
			{
				m_ChartDataWnd.add_channel_list_item(doc_channel, 0);
				chan_list_size++;
			}
			m_ChartDataWnd.get_channel_list_item(doc_channel)->SetColor(static_cast<WORD>(doc_channel));
		}
	}

	// load real data from file and update time parameters
	m_ChartDataWnd.get_data_from_doc(l_first, l_last); 
	m_time_first_abcissa = static_cast<float>(m_ChartDataWnd.GetDataFirstIndex()) / m_samplingRate; 
	m_time_last_abcissa = static_cast<float>(m_ChartDataWnd.GetDataLastIndex()) / m_samplingRate; 
	m_channel_selected = 0; // select chan 0

	if (!b_first_update)
		update_channels_display_parameters();

	// fill combo
	m_comboSelectChan.ResetContent();
	for (auto i = 0; i < m_ChartDataWnd.get_channel_list_size(); i++)
	{
		CString cs;
		cs.Format(_T("channel %i - "), i);
		cs = cs + m_ChartDataWnd.get_channel_list_item(i)->GetComment();
		m_comboSelectChan.AddString(cs);
	}
	if (scan_count > 1)
	{
		m_comboSelectChan.AddString(_T("all channels"));
	}
	if (!m_bCommonScale)
		m_comboSelectChan.SetCurSel(0);
	else
		m_comboSelectChan.SetCurSel(m_ChartDataWnd.get_channel_list_size());

	// done
	if (b_update_interface)
	{
		UpdateFileScroll();
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE | CHG_YSCALE);
		m_ChartDataWnd.Invalidate();
	}
}

void ViewData::update_channels_display_parameters()
{
	const auto n_line_view_channels = m_ChartDataWnd.get_channel_list_size();
	int max;
	int min;
	if (!m_bCommonScale)
	{
		for (auto i = 0; i < n_line_view_channels; i++)
		{
			// keep final gain constant even if amplifier gain has changed
			const CChanlistItem* chan_list_item = m_ChartDataWnd.get_channel_list_item(i);
			chan_list_item->GetMaxMin(&max, &min);
			auto y_extent = chan_list_item->GetYextent();
			auto y_zero = chan_list_item->GetYzero();

			if (options_view_data->bMaximizeGain)
				y_extent = MulDiv(max - min + 1, 11, 10);
			// center curve
			if (options_view_data->bCenterCurves)
				y_zero = (max + min) / 2;

			UpdateYExtent(i, y_extent);
			UpdateYZero(i, y_zero);
		}
	}
	else
	{
		constexpr auto chan_0 = 0;
		const CChanlistItem* p_chan0 = m_ChartDataWnd.get_channel_list_item(chan_0);
		auto y_extent = p_chan0->GetYextent();
		auto y_zero = p_chan0->GetYzero();
		if (options_view_data->bMaximizeGain)
		{
			float v_max = 0.;
			float v_min = 0.;
			for (auto i = 0; i < n_line_view_channels; i++)
			{
				// keep final gain constant even if ampli gain changed
				const CChanlistItem* p_chan = m_ChartDataWnd.get_channel_list_item(i);
				p_chan->GetMaxMin(&max, &min);
				const auto max_chan_i = p_chan->ConvertDataBinsToVolts(max);
				const auto min_chan_i = p_chan->ConvertDataBinsToVolts(min);
				if (max_chan_i > v_max)
					v_max = max_chan_i;
				if (min_chan_i < v_min)
					v_min = min_chan_i;
			}
			max = p_chan0->ConvertVoltsToDataBins(v_max);
			min = p_chan0->ConvertVoltsToDataBins(v_min);
			y_extent = MulDiv(max - min + 1, 10, 8);
			y_zero = (max + min) / 2;
		}
		UpdateYExtent(chan_0, y_extent);
		UpdateYZero(chan_0, y_zero);
	}
	m_ChartDataWnd.Invalidate();
}

void ViewData::set_cursor_associated_windows()
{
	auto n_cmd_show = SW_HIDE;
	if (m_cursor_state == CURSOR_CROSS && mdMO->wOption == 1
		&& m_ChartDataWnd.horizontal_tags.get_tag_list_size() > 0)
		n_cmd_show = SW_SHOW;

	// change windows state: edit windows
	GetDlgItem(IDC_STATIC1)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC3)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT1)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT3)->ShowWindow(n_cmd_show);

	// change cursors value
	if (m_cursor_state == CURSOR_CROSS && mdMO->wOption == 1)
		UpdateHZtagsVal();
}

void ViewData::UpdateHZtagsVal()
{
	if (m_ChartDataWnd.horizontal_tags.get_tag_list_size() <= 0)
		return;
	const auto v1 = m_ChartDataWnd.horizontal_tags.get_value(0);
	auto itag = 0;
	if (m_ChartDataWnd.horizontal_tags.get_tag_list_size() > 1)
		itag = 1;
	const auto v2 = m_ChartDataWnd.horizontal_tags.get_value(itag);
	const auto mv_per_bin = m_ChartDataWnd.get_channel_list_item(m_channel_selected)->GetVoltsperDataBin() * 1000.0f;
	m_first_Hz_cursor = static_cast<float>(v1) * mv_per_bin;
	m_second_Hz_cursor = static_cast<float>(v2) * mv_per_bin;
	m_difference_second_minus_first = m_first_Hz_cursor - m_second_Hz_cursor;

	TCHAR sz_buffer[32];
	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_first_Hz_cursor);
	GetDlgItem(IDC_EDIT1)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT1)->Invalidate();

	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_second_Hz_cursor);
	GetDlgItem(IDC_EDIT2)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT2)->Invalidate();

	_stprintf_s(sz_buffer, 32, _T("%.*g"), FLT_DIG, m_difference_second_minus_first);
	GetDlgItem(IDC_EDIT3)->SetWindowText(sz_buffer);
	GetDlgItem(IDC_EDIT3)->Invalidate();
}

LRESULT ViewData::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	//int j = HIWORD(lParam);		// control ID of sender
	// code = 0: chan hit 			low_p = channel
	// code = 1: cursor change		low_p = new cursor value
	// code = 2: horizontal cursor hit	low_p = cursor index
	int low_parameter = LOWORD(lParam); // value associated

	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		// save current cursors into document if cursor_state = 3
		if (m_cursor_state == CURSOR_CROSS)
		{
			if (mdMO->wOption == 0) // vertical cursors
			{
				const auto p_tag_list = m_p_dat_Doc->get_vt_tags_list();
				p_tag_list->copy_tag_list(&m_ChartDataWnd.vertical_tags);
				m_ChartDataWnd.vertical_tags.remove_all_tags();
			}
			else if (mdMO->wOption == 1) // horizontal cursors
			{
				const auto p_tag_list = m_p_dat_Doc->get_hz_tags_list();
				p_tag_list->copy_tag_list(&m_ChartDataWnd.horizontal_tags);
				m_ChartDataWnd.horizontal_tags.remove_all_tags();
			}
			else if (mdMO->wOption == 3) // detect stimulus
			{
				mdMO->wStimuluschan = m_ChartDataWnd.horizontal_tags.get_channel(0);
				mdMO->wStimulusthresh = m_ChartDataWnd.horizontal_tags.get_value(0);
				m_ChartDataWnd.horizontal_tags.remove_all_tags();
			}
			m_ChartDataWnd.Invalidate();
		}
	// change cursor value (+1), clip to upper cursor value
		if (low_parameter > CURSOR_CROSS)
			low_parameter = 0;
	// change cursor and tell parent that it has changed
		m_cursor_state = m_ChartDataWnd.set_mouse_cursor_type(low_parameter);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursor_state, 0));

	// recall cursors from document if cursorstate = 2
		if (m_cursor_state == CURSOR_CROSS)
		{
			if (mdMO->wOption == 0)
				m_ChartDataWnd.vertical_tags.copy_tag_list(m_p_dat_Doc->get_vt_tags_list());
			else if (mdMO->wOption == 1)
				m_ChartDataWnd.horizontal_tags.copy_tag_list(m_p_dat_Doc->get_hz_tags_list());
			else if (mdMO->wOption == 3)
				m_ChartDataWnd.horizontal_tags.add_tag(mdMO->wStimulusthresh, mdMO->wStimuluschan);
			m_ChartDataWnd.Invalidate();
		}
		set_cursor_associated_windows();
		break;

	case HINT_HITCHANNEL: // change channel if different
		m_channel_selected = low_parameter;
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
		break;

	case HINT_DEFINEDRECT:
		{
			const auto rect = m_ChartDataWnd.get_defined_rect();
			mdMO->wLimitSup = static_cast<WORD>(rect.top);
			mdMO->wLimitInf = static_cast<WORD>(rect.bottom);
			mdMO->lLimitLeft = m_ChartDataWnd.GetDataOffsetfromPixel(rect.left);
			mdMO->lLimitRight = m_ChartDataWnd.GetDataOffsetfromPixel(rect.right);
		}
	// action according to option
		switch (mdMO->wOption)
		{
		// ......................  vertical tags
		case 0: // if no VTtags, then take those of rectangle, or limits of lineview
			m_ChartDataWnd.vertical_tags.add_l_tag(mdMO->lLimitLeft, 0);
			if (mdMO->lLimitRight != mdMO->lLimitLeft)
				m_ChartDataWnd.vertical_tags.add_l_tag(mdMO->lLimitRight, 0);
		// store new VT tags into document
			m_p_dat_Doc->get_vt_tags_list()->copy_tag_list(&m_ChartDataWnd.vertical_tags);
			break;

		// ......................  horizontal cursors
		case 1: // if no HZcursors, take those of rectangle or limits of lineview
			{
				CChanlistItem* pchan = m_ChartDataWnd.get_channel_list_item(m_channel_selected);
				m_ChartDataWnd.horizontal_tags.add_tag(m_ChartDataWnd.get_channel_list_y_pixels_to_bin(m_channel_selected, mdMO->wLimitSup),
				                               m_channel_selected);
				if (mdMO->wLimitInf != mdMO->wLimitSup)
					m_ChartDataWnd.horizontal_tags.add_tag(
						m_ChartDataWnd.get_channel_list_y_pixels_to_bin(m_channel_selected, mdMO->wLimitInf), m_channel_selected);
				m_p_dat_Doc->get_hz_tags_list()->copy_tag_list(&m_ChartDataWnd.horizontal_tags);
				if (m_ChartDataWnd.horizontal_tags.get_tag_list_size() == 2)
					set_cursor_associated_windows();
				UpdateHZtagsVal();
			}
			break;

		// ......................  rectangle area
		//case 2:				// parameters are already within lineview and mdMO
		//	break;
		// ......................  detect stimulus and then measure
		//case 3:				// if not displayed, plot HZ detection cursor
		//	break;
		default:
			break;
		}
		m_ChartDataWnd.Invalidate();
		break;

	case HINT_CHANGEHZTAG: // horizontal tag has changed 	lowp = tag nb
		if (mdMO->wOption == 3)
			mdMO->wStimulusthresh = m_ChartDataWnd.horizontal_tags.get_value(0);
		else
			UpdateHZtagsVal();
		break;

	case HINT_VIEWSIZECHANGED: // change zoom
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE | UPD_ORDINATES | CHG_YSCALE);
		m_ChartDataWnd.Invalidate();
		SetVBarMode(m_VBarMode);
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_view_data->viewdata = *(m_ChartDataWnd.get_scope_parameters());
		break;
	default:
		break;
	}
	return 0L;
}

void ViewData::OnViewAlldata()
{
	m_ChartDataWnd.get_data_from_doc(0, GetDocument()->db_get_data_len() - 1);
	UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
	UpdateData(FALSE);
	m_ChartDataWnd.Invalidate();
	UpdateFileScroll();
}

void ViewData::OnFormatDataseriesattributes()
{
	DlgDataSeriesFormat dlg;
	dlg.m_pChartDataWnd = &m_ChartDataWnd;
	dlg.m_pdbDoc = m_p_dat_Doc;
	dlg.m_listindex = m_channel_selected;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_channel_selected = dlg.m_listindex;
	}
	UpdateLegends(UPD_YSCALE);
	m_ChartDataWnd.Invalidate();
}

void ViewData::OnToolsVerticaltags()
{
	mdMO->wOption = 0;

	// change cursor and tell parent that it has changed
	m_cursor_state = m_ChartDataWnd.set_mouse_cursor_type(CURSOR_CROSS);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursor_state, 0));
	//MeasureProperties(1);
}

void ViewData::OnToolsHorizontalcursors()
{
	mdMO->wOption = 1;
	// change cursor and tell parent that it has changed
	m_cursor_state = m_ChartDataWnd.set_mouse_cursor_type(CURSOR_CROSS);
	GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursor_state, 0));
	//MeasureProperties(0);
}

void ViewData::OnUpdateToolsHorizontalcursors(CCmdUI* pCmdUI)
{
	// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	//BOOL flag = (mdMO->wOption ==1  && m_cursor_state == 2);
	//pCmdUI->Enable(flag);
	pCmdUI->Enable(true);
}

void ViewData::OnUpdateToolsVerticaltags(CCmdUI* pCmdUI)
{
	// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	//BOOL flag = (mdMO->wOption ==0 && m_cursor_state == 2);
	//pCmdUI->Enable(flag);
	pCmdUI->Enable(true);
}

void ViewData::OnToolsMeasuremode()
{
	MeasureProperties(2);
}

void ViewData::OnToolsMeasure()
{
	MeasureProperties(3);
}

void ViewData::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		dbTableView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// ViewData scroll: vertical scroll bar
	switch (m_VBarMode)
	{
	case BAR_GAIN:
		OnGainScroll(nSBCode, nPos);
		break;
	case BAR_BIAS:
		OnBiasScroll(nSBCode, nPos);
	default:
		break;
	}
}

void ViewData::SetVBarMode(int bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}

void ViewData::UpdateGainScroll()
{
	m_scrolly.SetScrollPos(
		MulDiv(
			m_ChartDataWnd.get_channel_list_item(m_channel_selected)->GetYextent(),
			100,
			YEXTENT_MAX)
		+ 50,
		TRUE);
}

void ViewData::OnGainScroll(UINT nSBCode, UINT nPos)
{
	int yExtent = m_ChartDataWnd.get_channel_list_item(m_channel_selected)->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: yExtent = YEXTENT_MIN;
		break;
	case SB_LINELEFT: yExtent -= yExtent / 10 + 1;
		break;
	case SB_LINERIGHT: yExtent += yExtent / 10 + 1;
		break;
	case SB_PAGELEFT: yExtent -= yExtent / 2 + 1;
		break;
	case SB_PAGERIGHT: yExtent += yExtent + 1;
		break;
	case SB_RIGHT: yExtent = YEXTENT_MAX;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: yExtent = MulDiv(nPos - 50, YEXTENT_MAX, 100);
		break;
	default: break;
	}

	// change y extent
	if (yExtent > 0) //&& yExtent<=YEXTENT_MAX)
	{
		UpdateYExtent(m_channel_selected, yExtent);
		UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
	}
	// update scrollBar
	m_ChartDataWnd.Invalidate();
	UpdateGainScroll();
}

void ViewData::UpdateBiasScroll()
{
	CChanlistItem* pchan = m_ChartDataWnd.get_channel_list_item(m_channel_selected);
	const auto i_pos = (pchan->GetYzero() - pchan->GetDataBinZero())
		* 100 / static_cast<int>(YZERO_SPAN) + 50;
	m_scrolly.SetScrollPos(i_pos, TRUE);
	UpdateLegends(UPD_ORDINATES | CHG_YSCALE);
}

void ViewData::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	CChanlistItem* pchan = m_ChartDataWnd.get_channel_list_item(m_channel_selected);
	auto l_size = pchan->GetYzero() - pchan->GetDataBinZero();
	const auto yextent = pchan->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: l_size = YZERO_MIN;
		break;
	case SB_LINELEFT: l_size -= yextent / 100 + 1;
		break;
	case SB_LINERIGHT: l_size += yextent / 100 + 1;
		break;
	case SB_PAGELEFT: l_size -= yextent / 10 + 1;
		break;
	case SB_PAGERIGHT: l_size += yextent / 10 + 1;
		break;
	case SB_RIGHT: l_size = YZERO_MAX;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: l_size = (nPos - 50) * (YZERO_SPAN / 100);
		break;
	default: break;
	}

	// try to read data with this new size
	if (l_size > YZERO_MIN && l_size < YZERO_MAX)
	{
		UpdateYZero(m_channel_selected, l_size + pchan->GetDataBinZero());
	}
	// update scrollBar
	m_ChartDataWnd.Invalidate();
	UpdateBiasScroll();
}

void ViewData::OnCenterCurve()
{
	m_ChartDataWnd.center_chan(m_channel_selected);
	m_ChartDataWnd.Invalidate();

	CChanlistItem* pchan = m_ChartDataWnd.get_channel_list_item(m_channel_selected);
	const auto yextent = pchan->GetYextent();
	UpdateYExtent(m_channel_selected, yextent);
	const auto yzero = pchan->GetYzero();
	UpdateYZero(m_channel_selected, yzero);
}

void ViewData::OnGainAdjustCurve()
{
	m_ChartDataWnd.max_gain_chan(m_channel_selected);
	m_ChartDataWnd.Invalidate();

	CChanlistItem* pchan = m_ChartDataWnd.get_channel_list_item(m_channel_selected);
	const auto yextent = pchan->GetYextent();
	UpdateYExtent(m_channel_selected, yextent);
	const auto yzero = pchan->GetYzero();
	UpdateYZero(m_channel_selected, yzero);
	UpdateLegends(CHG_YSCALE);
}

void ViewData::OnSplitCurves()
{
	const auto nchans = m_ChartDataWnd.get_channel_list_size(); // nb of data channels
	const auto pxheight = m_ChartDataWnd.get_rect_height(); // height of the display area
	const auto pxoffset = pxheight / nchans; // height for each channel
	auto pxzero = (pxheight - pxoffset) / 2; // center first curve at

	// split display area
	int max, min;
	for (auto i = 0; i < nchans; i++)
	{
		CChanlistItem* chan = m_ChartDataWnd.get_channel_list_item(i);
		chan->GetMaxMin(&max, &min);
		const auto iextent = MulDiv(max - min + 1, 100 * nchans, 100);
		const auto ibias = MulDiv(pxzero, iextent, pxheight); // convert pixel into bins
		const auto izero = (max + min) / 2 - ibias; // change bias
		chan->SetYextent(iextent);
		chan->SetYzero(izero);
		pxzero -= pxoffset; // update position of next curve
	}
	UpdateLegends(CHG_YSCALE);
	m_ChartDataWnd.Invalidate();
}

void ViewData::OnFileScroll(UINT nSBCode, UINT nPos)
{
	auto b_result = FALSE;
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: // scroll to the start
	case SB_LINELEFT: // scroll one line left
	case SB_LINERIGHT: // scroll one line right
	case SB_PAGELEFT: // scroll one page left
	case SB_PAGERIGHT: // scroll one page right
	case SB_RIGHT: // scroll to end right
		b_result = m_ChartDataWnd.scroll_data_from_doc(nSBCode);
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		b_result = m_ChartDataWnd.get_data_from_doc(
			(nPos * m_p_dat_Doc->get_doc_channel_length()) / 100L);
		break;
	default: // NOP: set position only
		break;
	}

	// adjust display
	if (b_result)
	{
		UpdateLegends(UPD_ABCISSA);
		UpdateData(FALSE); // copy view object to controls
		m_ChartDataWnd.Invalidate();
	}
	UpdateFileScroll();
}

void ViewData::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		dbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// trap messages from ScrollBarEx
	int l_first;
	int l_last;
	CString cs;
	switch (nSBCode)
	{
	case SB_THUMBTRACK:
		m_file_scroll_bar.GetScrollInfo(&m_file_scroll_bar_infos, SIF_ALL);
		l_first = m_file_scroll_bar_infos.nPos;
		l_last = l_first + m_file_scroll_bar_infos.nPage - 1;
		m_time_first_abcissa = static_cast<float>(l_first) / m_samplingRate;
		m_time_last_abcissa = static_cast<float>(l_last) / m_samplingRate;
		m_ChartDataWnd.get_data_from_doc(l_first, l_last);
		m_ChartDataWnd.Invalidate();
		cs.Format(_T("%.3f"), m_time_first_abcissa);
		SetDlgItemText(IDC_TIMEFIRST, cs);
		cs.Format(_T("%.3f"), m_time_last_abcissa);
		SetDlgItemText(IDC_TIMELAST, cs);
		break;

	case SB_THUMBPOSITION:
		m_file_scroll_bar.GetScrollInfo(&m_file_scroll_bar_infos, SIF_ALL);
		l_first = m_file_scroll_bar_infos.nPos;
		l_last = l_first + m_file_scroll_bar_infos.nPage - 1;
		if (m_ChartDataWnd.get_data_from_doc(l_first, l_last))
		{
			UpdateLegends(UPD_ABCISSA);
			UpdateData(FALSE); // copy view object to controls
			m_ChartDataWnd.Invalidate();
		}

		break;
	default:
		OnFileScroll(nSBCode, nPos);
		break;
	}
}

void ViewData::MeasureProperties(int item)
{
	// make sure that cursor is ok
	if (m_cursor_state != CURSOR_CROSS)
		OnMyMessage(NULL, MAKELPARAM(CURSOR_CROSS, HINT_SETMOUSECURSOR));

	// save current data into data document
	switch (mdMO->wOption)
	{
	case 0:
		m_p_dat_Doc->get_vt_tags_list()->copy_tag_list(&m_ChartDataWnd.vertical_tags);
		break;
	case 1:
		m_p_dat_Doc->get_hz_tags_list()->copy_tag_list(&m_ChartDataWnd.horizontal_tags);
		break;
	case 3:
		mdMO->wStimuluschan = m_ChartDataWnd.horizontal_tags.get_channel(0);
		mdMO->wStimulusthresh = m_ChartDataWnd.horizontal_tags.get_value(0);
		break;
	default: break;
	}

	// call routine and pass parameters
	CMeasureProperties dlg(nullptr, item);
	dlg.m_pChartDataWnd = &m_ChartDataWnd;
	dlg.m_samplingrate = m_samplingRate;
	dlg.m_pdatDoc = m_p_dat_Doc;
	dlg.m_pdbDoc = GetDocument();
	dlg.m_pMO = mdMO;
	dlg.m_currentchan = m_channel_selected;

	dlg.DoModal();
	m_ChartDataWnd.Invalidate();
	set_cursor_associated_windows();
}

void ViewData::save_modified_file()
{
	// save previous file if anything has changed
	if (m_p_dat_Doc == nullptr)
		return;

	if (m_p_dat_Doc->IsModified())
	{
		CString docname = GetDocument()->db_get_current_dat_file_name();
		m_p_dat_Doc->save_document(docname);
	}
	m_p_dat_Doc->SetModifiedFlag(FALSE);
}

void ViewData::ADC_OnHardwareDefineexperiment()
{
	DlgdbEditRecord dlg;
	dlg.m_pdbDoc = GetDocument();

	if (IDOK == dlg.DoModal())
	{
		auto p_dbwave_doc = GetDocument();
		const auto record_id = p_dbwave_doc->db_get_current_record_id();
		GetDocument()->update_all_views_db_wave(nullptr, HINT_DOCHASCHANGED, nullptr);
		p_dbwave_doc->db_move_to_id(record_id);
		p_dbwave_doc->update_all_views_db_wave(nullptr, HINT_DOCMOVERECORD, nullptr);
	}
}

void ViewData::OnFormatXscale()
{
	// init dialog data
	DlgDataViewAbcissa dlg;
	dlg.m_firstAbcissa = m_time_first_abcissa;
	dlg.m_lastAbcissa = m_time_last_abcissa;
	dlg.m_veryLastAbcissa = m_ChartDataWnd.GetDocumentLast() / m_samplingRate;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_time_first_abcissa = dlg.m_firstAbcissa * dlg.m_abcissaScale;
		m_time_last_abcissa = dlg.m_lastAbcissa * dlg.m_abcissaScale;
		m_ChartDataWnd.get_data_from_doc(static_cast<long>(m_time_first_abcissa * m_samplingRate),
		                              static_cast<long>(m_time_last_abcissa * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | UPD_XSCALE | CHG_XBAR);
	}
}

void ViewData::ComputePrinterPageSize()
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
	options_view_data->horzRes = dc.GetDeviceCaps(HORZRES);
	options_view_data->vertRes = dc.GetDeviceCaps(VERTRES);

	// margins (pixels)
	m_print_rect_.right = options_view_data->horzRes - options_view_data->rightPageMargin;
	m_print_rect_.bottom = options_view_data->vertRes - options_view_data->bottomPageMargin;
	m_print_rect_.left = options_view_data->leftPageMargin;
	m_print_rect_.top = options_view_data->topPageMargin;
}

void ViewData::PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo)
{
	auto t = CTime::GetCurrentTime();
	TCHAR ch[256];
	wsprintf(ch, _T("  page %d:%d %d-%d-%d"), // %d:%d",
	         pInfo->m_nCurPage, pInfo->GetMaxPage(),
	         t.GetDay(), t.GetMonth(), t.GetYear());

	auto cs_dat_file = GetDocument()->db_get_current_dat_file_name();
	const auto icount = cs_dat_file.ReverseFind(_T('\\'));
	auto ch_date = cs_dat_file.Left(icount);
	ch_date = ch_date.Left(ch_date.GetLength() - 1) + ch;
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_view_data->horzRes / 2, options_view_data->vertRes - 57, ch_date);
}

CString ViewData::convert_file_index(long l_first, long l_last)
{
	CString csUnit = _T(" s"); // get time,  prepare time unit

	TCHAR sz_value[64]; // buffer to receive ascii represent of values
	const auto psz_value = sz_value;
	float x_scale_factor; // scale factor returned by changeunit
	auto x = CNiceUnit::change_unit(static_cast<float>(l_first) / m_samplingRate, &csUnit, &x_scale_factor);
	auto fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.)); // separate fractional part
	wsprintf(psz_value, _T("time = %i.%03.3i - "), static_cast<int>(x), fraction); // print value
	CString cs_comment = psz_value; // save ascii to string

	x = l_last / (m_samplingRate * x_scale_factor); // same operations for last interval
	fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.));
	wsprintf(psz_value, _T("%i.%03.3i %s"), static_cast<int>(x), fraction, static_cast<LPCTSTR>(csUnit));
	cs_comment += psz_value;
	return cs_comment;
}

BOOL ViewData::get_file_series_index_from_page(int page, int& file_number, long& l_first)
{
	// loop until we get all rows
	const auto totalrows = m_nb_rows_per_page_ * (page - 1);
	l_first = m_l_print_first_;
	file_number = 0; // file list index
	if (options_view_data->bPrintSelection) // current file if selection only
		file_number = m_file_0_;
	else
		GetDocument()->db_move_first();

	auto very_last = m_l_print_first_ + m_l_print_len_;
	if (options_view_data->bEntireRecord)
		very_last = m_p_dat_Doc->get_doc_channel_length() - 1;

	for (auto row = 0; row < totalrows; row++)
	{
		if (!PrintGetNextRow(file_number, l_first, very_last))
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
	const auto pwave_format = m_p_dat_Doc->get_waveformat();
	if (options_view_data->bDocName || options_view_data->bAcqDateTime) // print doc infos?
	{
		if (options_view_data->bDocName) // print file name
		{
			str_comment += GetDocument()->db_get_current_dat_file_name() + tab;
		}
		if (options_view_data->bAcqDateTime) // print data acquisition date & time
		{
			const auto date = pwave_format->acqtime.Format(_T("%#d %B %Y %X")); //("%c");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_view_data->bAcqComment)
		str_comment += pwave_format->get_comments(_T(" ")) + rc;

	return str_comment;
}

CString ViewData::PrintBars(CDC* p_dc, CRect* prect)
{
	CString str_comment;
	const CString rc(_T("\n"));
	const CString tab(_T("     "));

	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
	TCHAR sz_value[64];
	const auto lpsz_val = sz_value;
	CString cs_unit;
	float x_scale_factor;
	CPoint bar_origin(-10, -10); // origine barre à 10,10 pts de coin inf gauche rectangle
	bar_origin.x += prect->left;
	bar_origin.y += prect->bottom;
	auto xbar_end = bar_origin;
	auto ybar_end = bar_origin;

	// same len ratio as displayed on viewdata
	const auto horz_bar = m_ChartDataWnd.x_ruler.GetScaleUnitPixels(m_ChartDataWnd.get_rect_width());
	ASSERT(horz_bar > 0);
	const auto vert_bar = m_ChartDataWnd.y_ruler.GetScaleUnitPixels(m_ChartDataWnd.get_rect_height());
	ASSERT(vert_bar > 0);

	auto cs_comment = convert_file_index(m_ChartDataWnd.GetDataFirstIndex(), m_ChartDataWnd.GetDataLastIndex());
	if (options_view_data->bTimeScaleBar)
	{
		// print horizontal bar
		xbar_end.x += horz_bar;
		p_dc->MoveTo(bar_origin);
		p_dc->LineTo(xbar_end);

		// read text from control edit
		CString cs;
		cs.Format(_T(" bar= %g"), m_ChartDataWnd.x_ruler.GetScaleIncrement());
		cs_comment += cs;
		str_comment += cs_comment + rc;
	}

	if (options_view_data->bVoltageScaleBar)
	{
		ybar_end.y -= vert_bar;
		p_dc->MoveTo(bar_origin);
		p_dc->LineTo(ybar_end);
	}

	// comments, bar value and chan settings for each channel
	if (options_view_data->bChansComment || options_view_data->bVoltageScaleBar || options_view_data->bChanSettings)
	{
		const auto imax = m_ChartDataWnd.get_channel_list_size(); // number of data channels
		for (auto ichan = 0; ichan < imax; ichan++) // loop
		{
			// boucler sur les commentaires de chan n a chan 0...
			wsprintf(lpsz_val, _T("chan#%i "), ichan); // channel number
			cs_comment = lpsz_val;
			if (options_view_data->bVoltageScaleBar) // bar scale value
			{
				cs_unit = _T(" V"); // provisional unit
				auto z = static_cast<float>(m_ChartDataWnd.get_rect_height()) / 5
					* m_ChartDataWnd.get_channel_list_volts_per_pixel(ichan);
				auto x = CNiceUnit::change_unit(z, &cs_unit, &x_scale_factor); // convert

				// approximate
				auto j = static_cast<int>(x); // get int value
				if ((static_cast<double>(x) - j) > 0.5) // increment integer if diff > 0.5
					j++;
				auto k = CNiceUnit::nice_unit(x); // compare with nice unit abs
				if (j > 750) // there is a gap between 500 and 1000
					k = 1000;
				if (MulDiv(100, abs(k - j), j) <= 1) // keep nice unit if difference is less= than 1 %
					j = k;
				if (k >= 1000)
				{
					z = static_cast<float>(k) * x_scale_factor;
					j = static_cast<int>(CNiceUnit::change_unit(z, &cs_unit, &x_scale_factor)); // convert
				}
				wsprintf(sz_value, _T("bar = %i %s "), j, static_cast<LPCTSTR>(cs_unit)); // store value into comment
				cs_comment += sz_value;
			}
			str_comment += cs_comment;

			// print chan comment
			if (options_view_data->bChansComment)
			{
				str_comment += tab;
				str_comment += m_ChartDataWnd.get_channel_list_item(ichan)->GetComment();
			}
			str_comment += rc;

			// print amplifiers settings (gain & filter), next line
			if (options_view_data->bChanSettings)
			{
				CString cs;
				const WORD channb = m_ChartDataWnd.get_channel_list_item(ichan)->GetSourceChan();
				const auto pchanArray = m_p_dat_Doc->get_wavechan_array();
				const auto pChan = pchanArray->get_p_channel(channb);
				cs.Format(_T("headstage=%s gain=%.0f  filter= %s - %i Hz"), (LPCTSTR)pChan->am_csheadstage,
				          pChan->am_gaintotal, (LPCTSTR)pChan->am_csInputpos, pChan->am_lowpass);
				str_comment += cs;
				str_comment += rc;
			}
		}
	}
	p_dc->SelectObject(p_old_brush);
	return str_comment;
}

BOOL ViewData::OnPreparePrinting(CPrintInfo* pInfo)
{
	// printing margins
	if (options_view_data->vertRes <= 0 // vertical resolution defined ?
		|| options_view_data->horzRes <= 0 // horizontal resolution defined?
		|| options_view_data->horzRes != pInfo->m_rectDraw.Width() // same as infos provided
		|| options_view_data->vertRes != pInfo->m_rectDraw.Height()) // by caller?
		ComputePrinterPageSize();

	auto npages = print_get_n_pages();
	pInfo->SetMaxPage(npages); //one page printing/preview
	pInfo->m_nNumPreviewPages = 1; // preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (options_view_data->bPrintSelection)
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION; // set button to selection

	if (!DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	if (options_view_data->bPrintSelection != pInfo->m_pPD->PrintSelection())
	{
		options_view_data->bPrintSelection = pInfo->m_pPD->PrintSelection();
		npages = print_get_n_pages();
		pInfo->SetMaxPage(npages);
	}

	return TRUE;
}

int ViewData::print_get_n_pages()
{
	// how many rows per page?
	const auto size_row = options_view_data->HeightDoc + options_view_data->heightSeparator;
	m_nb_rows_per_page_ = m_print_rect_.Height() / size_row;
	if (m_nb_rows_per_page_ == 0) // prevent zero pages
		m_nb_rows_per_page_ = 1;

	int ntotal_rows; // number of rectangles -- or nb of rows
	auto p_dbwave_doc = GetDocument();

	// compute number of rows according to bmultirow & bentirerecord flag
	m_l_print_first_ = m_ChartDataWnd.GetDataFirstIndex();
	m_l_print_len_ = m_ChartDataWnd.GetDataLastIndex() - m_l_print_first_ + 1;
	m_file_0_ = GetDocument()->db_get_current_record_position();
	ASSERT(m_file_0_ >= 0);
	m_files_count_ = 1;
	auto ifile0 = m_file_0_;
	auto ifile1 = m_file_0_;
	if (!options_view_data->bPrintSelection)
	{
		ifile0 = 0;
		m_files_count_ = p_dbwave_doc->db_get_n_records();
		ifile1 = m_files_count_;
	}

	// only one row per file
	if (!options_view_data->bMultirowDisplay || !options_view_data->bEntireRecord)
		ntotal_rows = m_files_count_;

	// multirows per file
	else
	{
		ntotal_rows = 0;
		p_dbwave_doc->db_set_current_record_position(ifile0);
		for (auto i = ifile0; i < ifile1; i++, p_dbwave_doc->db_move_next())
		{
			// get size of document for all files
			auto len = p_dbwave_doc->db_get_data_len();
			if (len <= 0)
			{
				p_dbwave_doc->open_current_data_file();
				len = m_p_dat_Doc->get_doc_channel_length();
				p_dbwave_doc->db_set_data_len(len);
			}
			len -= m_l_print_first_;
			auto nrows = len / m_l_print_len_; // how many rows for this file?
			if (len > nrows * m_l_print_len_) // remainder?
				nrows++;
			ntotal_rows += static_cast<int>(nrows); // update nb of rows
		}
	}

	if (m_file_0_ >= 0)
	{
		p_dbwave_doc->db_set_current_record_position(m_file_0_);
		p_dbwave_doc->open_current_data_file();
	}

	// npages
	int npages = ntotal_rows / m_nb_rows_per_page_;
	if (ntotal_rows > m_nb_rows_per_page_ * npages)
		npages++;

	return npages;
}

void ViewData::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_b_is_printing_ = TRUE;
	m_l_first_0_ = m_ChartDataWnd.GetDataFirstIndex();
	m_l_last0_ = m_ChartDataWnd.GetDataLastIndex();
	m_pixels_count_0_ = m_ChartDataWnd.get_rect_width();

	//---------------------init objects-------------------------------------
	memset(&m_log_font_, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_log_font_.lfFaceName, _T("Arial")); // Arial font
	m_log_font_.lfHeight = options_view_data->fontsize; // font height
	m_p_old_font_ = nullptr;
	/*BOOL flag = */
	m_font_print_.CreateFontIndirect(&m_log_font_);
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewData::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_p_old_font_ = p_dc->SelectObject(&m_font_print_);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	const auto r_width = options_view_data->WidthDoc; // margins
	const auto r_height = options_view_data->HeightDoc; // margins
	CRect r_where(m_print_rect_.left, // printing rectangle for data
	              m_print_rect_.top,
	              m_print_rect_.left + r_width,
	              m_print_rect_.top + r_height);
	//CRect RW2 = RWhere;									// printing rectangle - constant
	//RW2.OffsetRect(-RWhere.left, -RWhere.top);			// set RW2 origin = 0,0

	p_dc->SetMapMode(MM_TEXT); // change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(p_dc, pInfo); // print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int filenumber; // file number and file index
	long l_first; // index first data point / first file
	auto very_last = m_l_print_first_ + m_l_print_len_; // index last data point / current file
	const int curpage = pInfo->m_nCurPage; // get current page number
	get_file_series_index_from_page(curpage, filenumber, l_first);
	if (l_first < GetDocument()->db_get_data_len() - 1)
		update_file_parameters();
	if (options_view_data->bEntireRecord)
		very_last = GetDocument()->db_get_data_len() - 1;

	SCOPESTRUCT oldparms;
	SCOPESTRUCT* p_newparms = m_ChartDataWnd.get_scope_parameters();
	oldparms = *p_newparms;
	p_newparms->bDrawframe = options_view_data->bFrameRect;
	p_newparms->bClipRect = options_view_data->bClipRect;

	// loop through all files	--------------------------------------------------------
	const int old_dc = p_dc->SaveDC(); // save DC
	for (auto i = 0; i < m_nb_rows_per_page_; i++)
	{
		// first : set rectangle where data will be printed
		auto comment_rect = r_where; // save RWhere for comments
		p_dc->SetMapMode(MM_TEXT); // 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); // set text align mode

		// load data and adjust display rectangle ----------------------------------------
		// reduce width to the size of the data
		auto l_last = l_first + m_l_print_len_; // compute last pt to load
		if (l_first < GetDocument()->db_get_data_len() - 1)
		{
			if (l_last > very_last) // check end across file length
				l_last = very_last;
			m_ChartDataWnd.get_data_from_doc(l_first, l_last); // load data from file
			update_channels_display_parameters();
			m_ChartDataWnd.Print(p_dc, &r_where); // print data
		}

		// update display rectangle for next row
		r_where.OffsetRect(0, r_height + options_view_data->heightSeparator);

		// restore DC and print comments --------------------------------------------------
		p_dc->SetMapMode(MM_TEXT); // 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr); // no more clipping
		p_dc->SetViewportOrg(0, 0); // org = 0,0

		// print comments according to row within file
		CString cs_comment;
		if (l_first == m_l_print_first_) // first row = full comment
		{
			cs_comment += get_file_infos();
			cs_comment += PrintBars(p_dc, &comment_rect); // bars and bar legends
		}
		else // other rows: time intervals only
			cs_comment = convert_file_index(m_ChartDataWnd.GetDataFirstIndex(), m_ChartDataWnd.GetDataLastIndex());

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_view_data->textseparator + comment_rect.Width(), 0);
		comment_rect.right = m_print_rect_.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect,
		               DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
		p_dc->SetTextAlign(ui_flag);

		// update file parameters for next row --------------------------------------------
		const auto ifile = filenumber;
		if (!PrintGetNextRow(filenumber, l_first, very_last))
		{
			i = m_nb_rows_per_page_;
			break;
		}
		if (ifile != filenumber)
			update_file_parameters(FALSE);
	}
	p_dc->RestoreDC(old_dc); // restore Display context

	// end of file loop : restore initial conditions
	if (m_p_old_font_ != nullptr)
		p_dc->SelectObject(m_p_old_font_);
	*p_newparms = oldparms;
}

BOOL ViewData::PrintGetNextRow(int& file_number, long& l_first, long& very_last)
{
	if (!options_view_data->bMultirowDisplay || !options_view_data->bEntireRecord)
	{
		file_number++;
		if (file_number >= m_files_count_)
			return FALSE;

		GetDocument()->db_move_next();
		if (l_first < GetDocument()->db_get_data_len() - 1)
		{
			if (options_view_data->bEntireRecord)
				very_last = GetDocument()->db_get_data_len() - 1;
		}
	}
	else
	{
		l_first += m_l_print_len_;
		if (l_first >= very_last)
		{
			file_number++; // next index
			if (file_number >= m_files_count_) // last file ??
				return FALSE;

			GetDocument()->db_move_next();
			very_last = GetDocument()->db_get_data_len() - 1;
			l_first = m_l_print_first_;
		}
	}
	return TRUE;
}

void ViewData::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_font_print_.DeleteObject();
	m_b_is_printing_ = FALSE;
	GetDocument()->db_set_current_record_position(m_file_0_);
	m_ChartDataWnd.resize_channels(m_pixels_count_0_, 0);
	m_ChartDataWnd.get_data_from_doc(m_l_first_0_, m_l_last0_);
	update_file_parameters();
}

void ViewData::OnEnChangeTimefirst()
{
	if (mm_time_first_abcissa.m_bEntryDone)
	{
		mm_time_first_abcissa.OnEnChange(this, m_time_first_abcissa, 1.f, -1.f);
		m_ChartDataWnd.get_data_from_doc(static_cast<long>(m_time_first_abcissa * m_samplingRate),
		                              static_cast<long>(m_time_last_abcissa * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
		m_ChartDataWnd.Invalidate();
	}
}

void ViewData::OnEnChangeTimelast()
{
	if (mm_time_last_abcissa.m_bEntryDone)
	{
		mm_time_last_abcissa.OnEnChange(this, m_time_last_abcissa, 1.f, -1.f);
		m_ChartDataWnd.get_data_from_doc(static_cast<long>(m_time_first_abcissa * m_samplingRate),
		                              static_cast<long>(m_time_last_abcissa * m_samplingRate));
		UpdateLegends(UPD_ABCISSA | CHG_XSCALE);
		m_ChartDataWnd.Invalidate();
	}
}

void ViewData::UpdateFileScroll()
{
	m_file_scroll_bar_infos.fMask = SIF_ALL | SIF_PAGE | SIF_POS;
	m_file_scroll_bar_infos.nMin = 0;
	m_file_scroll_bar_infos.nMax = GetDocument()->db_get_data_len();
	m_file_scroll_bar_infos.nPos = m_ChartDataWnd.GetDataFirstIndex();
	m_file_scroll_bar_infos.nPage = m_ChartDataWnd.GetDataLastIndex() - m_ChartDataWnd.GetDataFirstIndex() + 1;
	m_file_scroll_bar.SetScrollInfo(&m_file_scroll_bar_infos);
}

void ViewData::OnCbnSelchangeCombochan()
{
	const auto ichan = m_comboSelectChan.GetCurSel();
	if (ichan < m_ChartDataWnd.get_channel_list_size())
	{
		m_bCommonScale = FALSE;
		update_channel(ichan);
	}
	else
	{
		m_bCommonScale = TRUE;
		m_channel_selected = 0;
		CChanlistItem* pchan = m_ChartDataWnd.get_channel_list_item(0);
		const auto yextent = pchan->GetYextent();
		UpdateYExtent(0, yextent);
		const auto yzero = pchan->GetYzero();
		UpdateYZero(0, yzero);
	}
}

void ViewData::UpdateYExtent(int ichan, int yextent)
{
	CChanlistItem* pchan = m_ChartDataWnd.get_channel_list_item(ichan);
	pchan->SetYextent(yextent);
	if (m_comboSelectChan.GetCurSel() == m_ChartDataWnd.get_channel_list_size())
	{
		const auto yVoltsextent = pchan->GetVoltsperDataBin() * yextent;
		m_ChartDataWnd.set_channel_list_volts_extent(-1, &yVoltsextent);
	}
}

void ViewData::UpdateYZero(int ichan, int ybias)
{
	CChanlistItem* chan = m_ChartDataWnd.get_channel_list_item(ichan);
	chan->SetYzero(ybias);
	if (m_comboSelectChan.GetCurSel() == m_ChartDataWnd.get_channel_list_size())
	{
		const auto yVoltsextent = chan->GetVoltsperDataBin() * ybias;
		m_ChartDataWnd.set_channel_list_volts_zero(-1, &yVoltsextent);
	}
}
