// TODO
// convert threshold into volts & back to binary (cope with variable gains)
// cf: UpdateFileParameters

#include "StdAfx.h"
#include "dbWave.h"
#include "ViewSpikeDetect.h"

#include "CNiceUnit.h"
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

IMPLEMENT_DYNCREATE(ViewSpikeDetection, dbTableView)

ViewSpikeDetection::ViewSpikeDetection()
	: dbTableView(IDD)
{
	m_bEnableActiveAccessibility = FALSE;
}

ViewSpikeDetection::~ViewSpikeDetection()
{
	if (m_pSpkDoc != nullptr) {
		save_current_spk_file();
	}
	// save spike detection parameters
	const auto p_array = m_pArrayFromApp->GetChanArray(m_scan_count_doc);
	*p_array = m_spk_detect_array_current;
}

void ViewSpikeDetection::DoDataExchange(CDataExchange* pDX)
{
	dbTableView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO1, m_CBdetectWhat);
	DDX_Control(pDX, IDC_SOURCECHAN, m_CBdetectChan);
	DDX_Control(pDX, IDC_TRANSFORM, m_CBtransform);
	DDX_Control(pDX, IDC_TRANSFORM2, m_CBtransform2);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_SPIKENO, m_spike_index);
	DDX_Check(pDX, IDC_ARTEFACT, m_bartefact);
	DDX_Text(pDX, IDC_THRESHOLDVAL, m_thresholdval);
	DDX_Text(pDX, IDC_SPIKEWINDOWAMPLITUDE, m_spkWndAmplitude);
	DDX_Text(pDX, IDC_SPIKEWINDOWLENGTH, m_spkWndDuration);
	DDX_Text(pDX, IDC_CHANSELECTED, m_ichanselected);
	DDX_Text(pDX, IDC_CHANSELECTED2, m_ichanselected2);
	DDX_Control(pDX, IDC_XSCALE, m_xspkdscale);
	DDX_Control(pDX, IDC_STATICDISPLAYDATA, m_bevel1);
	DDX_Control(pDX, IDC_STATICDISPLAYDETECT, m_bevel2);
	DDX_Control(pDX, IDC_STATICDISPLAYBARS, m_bevel3);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}

BEGIN_MESSAGE_MAP(ViewSpikeDetection, dbTableView)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikeDetection::OnMyMessage)

	ON_COMMAND(ID_FORMAT_FIRSTFRAME, &ViewSpikeDetection::OnFirstFrame)
	ON_COMMAND(ID_FORMAT_LASTFRAME, &ViewSpikeDetection::OnLastFrame)
	ON_COMMAND(ID_FORMAT_SETORDINATES, &ViewSpikeDetection::OnFormatXscale)
	ON_COMMAND(ID_FORMAT_ALLDATA, &ViewSpikeDetection::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &ViewSpikeDetection::OnFormatYscaleCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &ViewSpikeDetection::OnFormatYscaleGainadjust)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, &ViewSpikeDetection::OnFormatSplitcurves)
	ON_COMMAND(ID_TOOLS_DETECT_PARMS, &ViewSpikeDetection::OnToolsDetectionparameters)
	ON_COMMAND(ID_TOOLS_DETECT, &ViewSpikeDetection::OnMeasureAll)
	ON_COMMAND(ID_TOOLS_EDITSTIMULUS, &ViewSpikeDetection::OnToolsEditstimulus)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, &ViewSpikeDetection::OnToolsEdittransformspikes)
	ON_COMMAND(ID_TOOLS_DATASERIES, &ViewSpikeDetection::OnToolsDataseries)
	ON_COMMAND(ID_EDIT_COPY, &ViewSpikeDetection::OnEditCopy)
	ON_COMMAND(ID_FILE_SAVE, &ViewSpikeDetection::OnFileSave)
	ON_COMMAND(ID_FORMAT_XSCALE, &ViewSpikeDetection::OnFormatXscale)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_CBN_SELCHANGE(IDC_SOURCECHAN, &ViewSpikeDetection::OnSelchangeDetectchan)
	ON_CBN_SELCHANGE(IDC_TRANSFORM, &ViewSpikeDetection::OnSelchangeTransform)
	ON_CBN_SELCHANGE(IDC_COMBO1, &ViewSpikeDetection::OnSelchangeDetectMode)
	ON_CBN_SELCHANGE(IDC_TRANSFORM2, &ViewSpikeDetection::OnCbnSelchangeTransform2)

	ON_EN_CHANGE(IDC_THRESHOLDVAL, &ViewSpikeDetection::OnEnChangeThresholdval)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewSpikeDetection::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewSpikeDetection::OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_SPIKENO, &ViewSpikeDetection::OnEnChangeSpikeno)
	ON_EN_CHANGE(IDC_SPIKEWINDOWAMPLITUDE, &ViewSpikeDetection::OnEnChangeSpkWndAmplitude)
	ON_EN_CHANGE(IDC_SPIKEWINDOWLENGTH, &ViewSpikeDetection::OnEnChangeSpkWndLength)
	ON_EN_CHANGE(IDC_CHANSELECTED, &ViewSpikeDetection::OnEnChangeChanselected)
	ON_EN_CHANGE(IDC_CHANSELECTED2, &ViewSpikeDetection::OnEnChangeChanselected2)

	ON_BN_CLICKED(IDC_MEASUREALL, &ViewSpikeDetection::OnMeasureAll)
	ON_BN_CLICKED(IDC_CLEAR, &ViewSpikeDetection::OnClear)
	ON_BN_CLICKED(IDC_ARTEFACT, &ViewSpikeDetection::OnArtefact)
	ON_BN_CLICKED(IDC_BIAS, &ViewSpikeDetection::OnBnClickedBiasbutton)
	ON_BN_CLICKED(IDC_GAIN, &ViewSpikeDetection::OnBnClickedGainbutton)
	ON_BN_CLICKED(IDC_LOCATEBTTN, &ViewSpikeDetection::OnBnClickedLocatebttn)
	ON_BN_CLICKED(IDC_CLEARALL, &ViewSpikeDetection::OnBnClickedClearAll)
	ON_BN_CLICKED(IDC_MEASURE, &ViewSpikeDetection::OnMeasure)
	ON_BN_CLICKED(IDC_GAIN2, &ViewSpikeDetection::OnBnClickedGain2)
	ON_BN_CLICKED(IDC_BIAS2, &ViewSpikeDetection::OnBnClickedBias2)

	//ON_NOTIFY(NM_CLICK, IDC_TAB1,		&dbTableView::OnNMClickTab1)
END_MESSAGE_MAP()

void ViewSpikeDetection::OnFileSave()
{
	CFile f;
	CFileDialog dlg(FALSE,
		_T("spk"),										// default filename extension
		GetDocument()->db_get_current_spk_file_name(),	// initial file name
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Awave Spikes (*.spk) | *.spk |All Files (*.*) | *.* ||"));

	if (IDOK == dlg.DoModal())
	{
		m_pSpkDoc->OnSaveDocument(dlg.GetPathName());
		GetDocument()->set_db_n_spikes(m_pSpkDoc->get_spike_list_current()->get_spikes_count());
		GetDocument()->set_db_n_spike_classes(1);
		m_pSpkDoc->SetModifiedFlag(FALSE);
	}
}

BOOL ViewSpikeDetection::OnMove(UINT nIDMoveCommand)
{
	save_current_spk_file();
	return dbTableView::OnMove(nIDMoveCommand);
}

void ViewSpikeDetection::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_b_init_)
	{
		switch (LOWORD(lHint))
		{
		case HINT_CLOSEFILEMODIFIED: // close modified file: save
			save_current_spk_file();
			break;
		case HINT_DOCMOVERECORD:
		case HINT_DOCHASCHANGED: // file has changed?
			update_file_parameters(TRUE);
			break;
		case HINT_REPLACEVIEW:
		default:
			break;
		}
	}
}

void ViewSpikeDetection::OnActivateView(BOOL activate, CView* activated_view, CView* de_activated_view)
{
	if (activate)
	{
		const auto p_main_frame = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_main_frame->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, reinterpret_cast<LPARAM>(activated_view->GetDocument()));
	}
	else
	{
		serialize_windows_state(b_save);
		const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		p_app->options_view_data.viewdata = *(m_chart_data_source.get_scope_parameters());
	}
	dbTableView::OnActivateView(activate, activated_view, de_activated_view);
}

void ViewSpikeDetection::update_legends()
{
	const auto l_first = m_chart_data_source.GetDataFirstIndex();
	const auto l_last = m_chart_data_source.GetDataLastIndex();
	m_chart_data_filtered.get_data_from_doc(l_first, l_last);

	// draw charts
	m_chart_spike_bar.set_time_intervals(l_first, l_last);
	m_chart_spike_shape.set_time_intervals(l_first, l_last);
	update_spike_shape_window_scale(FALSE);

	// update text abscissa and horizontal scroll position
	m_timefirst = static_cast<float>(l_first) / m_samplingRate;
	m_timelast = static_cast<float>(l_last + 1) / m_samplingRate;
	m_spike_index = m_pSpkList->m_selected_spike;

	if (m_spike_index > m_pSpkList->get_spikes_count())
	{
		m_pSpkList->m_selected_spike = -1;
		m_spike_index = m_pSpkList->m_selected_spike;
	}

	m_bartefact = FALSE;
	if (m_spike_index > 0)
	{
		const auto p_s = m_pSpkList->get_spike(m_spike_index);
		m_bartefact = (p_s->get_class_id() < 0);
	}

	update_file_scroll();
	update_combo_box();
	update_VT_tags();
	update_legend_detection_wnd();
	update_number_of_spikes();

	m_chart_spike_bar.Invalidate();
	m_chart_data_filtered.Invalidate();
	m_chart_data_source.Invalidate();
	m_chart_spike_shape.Invalidate();
	UpdateData(FALSE);
}

void ViewSpikeDetection::update_spike_file(BOOL bUpdateInterface)
{
	const auto pdb_doc = GetDocument();
	if (pdb_doc->open_current_spike_file() == nullptr)
	{
		// file not found: create new object, and create file
		auto* p_spike = new CSpikeDoc;
		ASSERT(p_spike != NULL);
		pdb_doc->m_p_spk = p_spike;
		m_pSpkDoc = p_spike;
		m_pSpkDoc->OnNewDocument();
		m_pSpkDoc->clear_data();
		if (options_view_data->bDetectWhileBrowse)
			OnMeasureAll();
	}
	else
	{
		m_pSpkDoc = pdb_doc->m_p_spk;
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(pdb_doc->db_get_current_spk_file_name(), FALSE);
	}

	// select a spike list
	m_pSpkList = m_pSpkDoc->get_spike_list_current();
	if (m_pSpkList == nullptr && m_pSpkDoc->get_spike_list_size() > 0)
		m_pSpkList = m_pSpkDoc->set_spike_list_current_index(0);

	// no spikes list available, create one
	if (m_pSpkList == nullptr)
	{
		// create new list here
		ASSERT(m_pSpkDoc->get_spike_list_size() == 0);
		const auto i_size = m_spk_detect_array_current.GetSize();
		m_pSpkDoc->set_spike_list_size(i_size);
		for (auto i = 0; i < i_size; i++)
		{
			auto spike_list_current = m_pSpkDoc->set_spike_list_current_index(i);
			if (spike_list_current == nullptr)
			{
				m_pSpkDoc->add_spk_list();
				spike_list_current = m_pSpkDoc->get_spike_list_current();
			}
			spike_list_current->init_spike_list(pdb_doc->m_p_dat, m_spk_detect_array_current.GetItem(i));
		}
		m_pSpkList = m_pSpkDoc->set_spike_list_current_index(0);
		ASSERT(m_pSpkList != nullptr);
	}

	m_chart_spike_bar.set_source_data(m_pSpkList, pdb_doc);
	m_chart_spike_bar.set_plot_mode(PLOT_BLACK, 0);
	m_chart_spike_shape.set_source_data(m_pSpkList, pdb_doc);
	m_chart_spike_shape.set_plot_mode(PLOT_BLACK, 0);
	update_VT_tags();

	// update interface elements
	m_spike_index = -1;
	if (bUpdateInterface)
	{
		update_tabs();
		update_detection_controls();
		highlight_spikes(FALSE);
		update_number_of_spikes();
		m_chart_spike_bar.Invalidate();
		m_chart_spike_shape.Invalidate();
	}
}

void ViewSpikeDetection::highlight_spikes(BOOL flag)
{
	if (!flag || m_pSpkList == nullptr || m_pSpkList->get_spikes_count() < 1) 
		return;
	
	const auto array_size = m_pSpkList->get_spikes_count() * 2 + 3;
	m_DWintervals.SetSize(array_size);
	m_DWintervals.SetAt(0, 0);
	m_DWintervals.SetAt(1, static_cast<DWORD>(RGB(255, 0, 0))); // red 
	m_DWintervals.SetAt(2, 1);
	const auto total_spikes = m_pSpkList->get_spikes_count();
	auto j_index = 3;
	auto spike_length = m_pSpkList->get_spike_length();
	const auto spike_pre_trigger = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	spike_length--;

	for (auto i = 0; i < total_spikes; i++)
	{
		const auto p_s = m_pSpkList->get_spike(i);
		const auto l_first = p_s->get_time() - spike_pre_trigger;
		m_DWintervals.SetAt(j_index, l_first);
		j_index++;
		m_DWintervals.SetAt(j_index, l_first + spike_length);
		j_index++;
	}

	// tell source_view to highlight spk
	m_chart_data_filtered.set_highlight_data(&m_DWintervals);
	m_chart_data_filtered.Invalidate();
	m_chart_data_source.set_highlight_data(&m_DWintervals);
	m_chart_data_source.Invalidate();
	
}

void ViewSpikeDetection::update_file_parameters(BOOL bUpdateInterface)
{
	update_data_file(bUpdateInterface);
	update_spike_file(bUpdateInterface);
	if (bUpdateInterface)
		update_legends();
}

BOOL ViewSpikeDetection::check_detection_settings()
{
	auto flag = TRUE;
	ASSERT(m_p_detect_parameters != NULL);
	ASSERT_VALID(m_p_detect_parameters);
	if (nullptr == m_p_detect_parameters)
	{
		m_i_detect_parameters = GetDocument()->get_current_spike_file()->get_spike_list_current_index();
		m_p_detect_parameters = m_spk_detect_array_current.GetItem(m_i_detect_parameters);
	}

	// get infos from data file
	const auto data_file = GetDocument()->m_p_dat;
	data_file->read_data_infos();
	const auto wave_format = data_file->get_waveformat();

	// check detection and extraction channels
	if (m_p_detect_parameters->detect_channel < 0
		|| m_p_detect_parameters->detect_channel >= wave_format->scan_count)
	{
		m_p_detect_parameters->detect_channel = 0;
		AfxMessageBox(_T("Spike detection parameters: detection channel modified"));
		flag = FALSE;
	}

	if (m_p_detect_parameters->extract_channel < 0
		|| m_p_detect_parameters->extract_channel >= wave_format->scan_count)
	{
		m_p_detect_parameters->extract_channel = 0;
		AfxMessageBox(_T("Spike detection parameters: channel extracted modified"));
		flag = FALSE;
	}
	return flag;
}

boolean ViewSpikeDetection::update_data_file(BOOL bUpdateInterface)
{
	const auto pdb_doc = GetDocument();
	const auto p_data_file = pdb_doc->open_current_data_file();
	if (p_data_file == nullptr)
		return false;

	p_data_file->read_data_infos();
	const auto wave_format = p_data_file->get_waveformat();

	// if the number of data channels of the data source has changed, load a new set of parameters
	// keep one array of spike detection parameters per data acquisition configuration (ie nb of acquisition channels)
	if (m_scan_count_doc != wave_format->scan_count)
	{
		// save current set of parameters if scan count >= 0 this might not be necessary
		const auto channel_array = m_pArrayFromApp->GetChanArray(m_scan_count_doc);
		if (m_scan_count_doc >= 0)
			*channel_array = m_spk_detect_array_current;
		// Get parameters from the application array
		m_scan_count_doc = wave_format->scan_count;
		m_spk_detect_array_current = *(m_pArrayFromApp->GetChanArray(m_scan_count_doc));
		// select by default the first set of detection parameters
		m_i_detect_parameters = 0;
		m_p_detect_parameters = m_spk_detect_array_current.GetItem(m_i_detect_parameters);
	}

	// check if detection parameters are valid
	check_detection_settings();

	// update combo boxes
	if (m_CBdetectChan.GetCount() != wave_format->scan_count)
	{
		update_combos_detect_and_transforms();
	}
	// change doc attached to line view
	m_chart_data_filtered.AttachDataFile(p_data_file);
	m_chart_data_source.AttachDataFile(p_data_file);

	// update source view display
	if (m_chart_data_filtered.get_channel_list_size() < 1)
	{
		m_chart_data_filtered.remove_all_channel_list_items();
		m_chart_data_filtered.add_channel_list_item(0, 0);
		CChanlistItem* channel_item = m_chart_data_filtered.get_channel_list_item(0);
		channel_item->SetColor(0);
		m_chart_data_filtered.horizontal_tags.remove_all_tags();
		m_p_detect_parameters->detect_threshold_bin = channel_item->ConvertVoltsToDataBins(m_p_detect_parameters->detect_threshold_mv / 1000.f);
		m_chart_data_filtered.horizontal_tags.add_tag(m_p_detect_parameters->detect_threshold_bin, 0);
	}

	//add all channels to detection window
	auto channel_list_size = m_chart_data_source.get_channel_list_size();
	const int n_document_channels = wave_format->scan_count;
	for (auto i = 0; i < n_document_channels; i++)
	{
		// check if present in the list
		auto b_present = FALSE;
		for (auto j = channel_list_size - 1; j >= 0; j--)
		{
			// test if this data chan is present + no transformation
			const CChanlistItem* channel_item = m_chart_data_source.get_channel_list_item(j);
			if (channel_item->GetSourceChan() == i
				&& channel_item->GetTransformMode() == 0)
			{
				b_present = TRUE;
				break;
			}
		}
		if (b_present == FALSE) // no display chan contains that doc chan
		{
			m_chart_data_source.add_channel_list_item(i, 0);
			channel_list_size++;
		}
		m_chart_data_source.get_channel_list_item(i)->SetColor(static_cast<WORD>(i));
	}

	// if browse through another file ; keep previous display parameters & load data
	auto l_first = m_chart_data_filtered.GetDataFirstIndex();
	auto l_last = m_chart_data_filtered.GetDataLastIndex();
	if (options_view_data->bEntireRecord && bUpdateInterface)
	{
		l_first = 0;
		l_last = p_data_file->get_doc_channel_length() - 1;
	}
	m_chart_data_filtered.get_data_from_doc(l_first, l_last);
	m_chart_data_source.get_data_from_doc(l_first, l_last);

	if (bUpdateInterface)
	{
		m_chart_data_filtered.Invalidate();
		m_chart_data_source.Invalidate();
		// adjust scroll bar (size of button and left/right limits)
		m_filescroll_infos.fMask = SIF_ALL;
		m_filescroll_infos.nMin = 0;
		m_filescroll_infos.nMax = m_chart_data_filtered.GetDataLastIndex();
		m_filescroll_infos.nPos = 0;
		m_filescroll_infos.nPage = m_chart_data_filtered.GetDataLastIndex() - m_chart_data_filtered.GetDataFirstIndex() + 1;
		m_filescroll.SetScrollInfo(&m_filescroll_infos);

		m_datacomments = wave_format->get_comments(_T(" "));
		m_samplingRate = wave_format->sampling_rate_per_channel;
		m_bValidThreshold = FALSE;
	}
	return true;
}

void ViewSpikeDetection::update_combos_detect_and_transforms()
{
	const auto db_document = GetDocument();
	const auto p_data_file = db_document->m_p_dat;
	const auto channel_array = p_data_file->get_wavechan_array();
	const auto wave_format = p_data_file->get_waveformat();

	// load channel names
	CString comment;
	m_CBdetectChan.ResetContent();
	const int channel_count = wave_format->scan_count;
	for (auto i = 0; i < channel_count; i++)
	{
		comment.Format(_T("%i: "), i);
		comment += channel_array->get_p_channel(i)->am_csComment;
		VERIFY(m_CBdetectChan.AddString(comment) != CB_ERR);
	}

	// load transforms names
	m_CBtransform.ResetContent();
	m_CBtransform2.ResetContent();
	const int n_transform_types = p_data_file->get_transforms_count();
	for (auto j = 0; j < n_transform_types; j++)
	{
		VERIFY(m_CBtransform.AddString(p_data_file->get_transform_name(j)) != CB_ERR);
		VERIFY(m_CBtransform2.AddString(p_data_file->get_transform_name(j)) != CB_ERR);
	}
	m_CBdetectChan.SetCurSel(m_p_detect_parameters->detect_channel);
}

void ViewSpikeDetection::define_stretch_parameters()
{
	m_stretch_.AttachParent(this);

	// top right ----------------------------------------
	m_stretch_.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch_.newProp(IDC_EMPTYPICTURE, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch_.newSlaveProp(IDC_DISPLAYDATA, XLEQ_XREQ, SZPR_YTEQ, IDC_EMPTYPICTURE);
	m_stretch_.newSlaveProp(IDC_DISPLAYDETECT, XLEQ_XREQ, SZPR_YBEQ, IDC_EMPTYPICTURE);

	m_stretch_.newSlaveProp(IDC_CHANSELECTED2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	m_stretch_.newSlaveProp(IDC_GAIN2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	m_stretch_.newSlaveProp(IDC_BIAS2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	m_stretch_.newSlaveProp(IDC_SCROLLY2, SZEQ_XREQ, YTEQ_YBEQ, IDC_DISPLAYDATA);
	m_stretch_.newSlaveProp(IDC_STATICDISPLAYDATA, SZEQ_XLEQ, YTEQ_YBEQ, IDC_DISPLAYDATA);

	m_stretch_.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch_.newSlaveProp(IDC_CHANSELECTED, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	m_stretch_.newSlaveProp(IDC_GAIN, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	m_stretch_.newSlaveProp(IDC_BIAS, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	m_stretch_.newSlaveProp(IDC_SCROLLY, SZEQ_XREQ, YTEQ_YBEQ, IDC_DISPLAYDETECT);
	m_stretch_.newSlaveProp(IDC_STATICDISPLAYDETECT, SZEQ_XLEQ, YTEQ_YBEQ, IDC_DISPLAYDETECT);

	// bottom right  ------------------------------------
	m_stretch_.newProp(IDC_DISPLAYBARS, XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_STATICDISPLAYBARS, SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch_.newProp(IDC_DURATIONTEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_SPIKEWINDOWLENGTH, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_MINTEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_SPIKEWINDOWAMPLITUDE, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_NBSPIKES_NB, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_NBSPIKES_TEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_TRANSFORM2, SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch_.newProp(IDC_STATIC3, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_SPIKENO, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_ARTEFACT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_DISPLAYSPIKES, SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch_.newProp(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch_.newProp(IDC_SOURCE, SZEQ_XREQ, SZEQ_YBEQ);

	m_stretch_.newProp(IDC_XSCALE, XLEQ_XREQ, SZEQ_YBEQ);
}

void ViewSpikeDetection::define_sub_classed_items()
{
	// attach controls
	VERIFY(m_filescroll.SubclassDlgItem(IDC_FILESCROLL, this));
	VERIFY(m_CBtransform.SubclassDlgItem(IDC_TRANSFORM, this));
	VERIFY(m_CBtransform2.SubclassDlgItem(IDC_TRANSFORM2, this));
	VERIFY(m_CBdetectChan.SubclassDlgItem(IDC_SOURCECHAN, this));
	VERIFY(m_CBdetectWhat.SubclassDlgItem(IDC_COMBO1, this));

	VERIFY(mm_spikeno.SubclassDlgItem(IDC_SPIKENO, this));
	mm_spikeno.ShowScrollBar(SB_VERT);
	VERIFY(mm_thresholdval.SubclassDlgItem(IDC_THRESHOLDVAL, this));
	mm_thresholdval.ShowScrollBar(SB_VERT);
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(mm_spkWndDuration.SubclassDlgItem(IDC_SPIKEWINDOWLENGTH, this));
	VERIFY(mm_spkWndAmplitude.SubclassDlgItem(IDC_SPIKEWINDOWAMPLITUDE, this));
	VERIFY(mm_ichanselected.SubclassDlgItem(IDC_CHANSELECTED, this));
	VERIFY(mm_ichanselected2.SubclassDlgItem(IDC_CHANSELECTED2, this));

	// control derived from CChartWnd
	VERIFY(m_chart_spike_shape.SubclassDlgItem(IDC_DISPLAYSPIKES, this));
	VERIFY(m_chart_spike_bar.SubclassDlgItem(IDC_DISPLAYBARS, this));
	VERIFY(m_chart_data_filtered.SubclassDlgItem(IDC_DISPLAYDETECT, this));
	VERIFY(m_chart_data_source.SubclassDlgItem(IDC_DISPLAYDATA, this));

	// load left scrollbar and button
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY, this));
	m_scrolly.SetScrollRange(0, 100);
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hBias)));
	GetDlgItem(IDC_GAIN)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hZoom)));
	VERIFY(m_scrolly2.SubclassDlgItem(IDC_SCROLLY2, this));
	m_scrolly2.SetScrollRange(0, 100);
	m_hBias2 = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom2 = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS2)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hBias2)));
	GetDlgItem(IDC_GAIN2)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hZoom2)));
}

void ViewSpikeDetection::OnInitialUpdate()
{
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	m_pArrayFromApp = &(p_app->spk_detect_array); 
	options_view_data = &(p_app->options_view_data); 

	define_stretch_parameters();
	m_b_init_ = TRUE;
	m_auto_increment = true;
	m_auto_detect = true;

	define_sub_classed_items();

	dbTableView::OnInitialUpdate();

	// load file data
	if (m_chart_data_filtered.horizontal_tags.get_tag_list_size() < 1)
		m_chart_data_filtered.horizontal_tags.add_tag(0, 0);

	update_file_parameters(TRUE);
	m_chart_data_filtered.set_scope_parameters(&(options_view_data->viewdata));
	m_chart_data_filtered.Invalidate();
	m_chart_data_source.set_scope_parameters(&(options_view_data->viewdata));
	m_chart_data_source.Invalidate();
}

LRESULT ViewSpikeDetection::OnMyMessage(const WPARAM wParam, const LPARAM lParam)
{
	int threshold = LOWORD(lParam);
	const int i_id = HIWORD(lParam);

	// ----------------------------- change mouse cursor (all 3 items)
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		if (threshold > CURSOR_VERTICAL)
			threshold = 0;
		if (threshold == CURSOR_CROSS)
			threshold = CURSOR_VERTICAL;
		m_cursor_state = threshold;
		SetViewMouseCursor(threshold);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursor_state, 0));
		break;

		// ----------------------------- move horizontal cursor / source data
	case HINT_MOVEHZTAG:
		m_p_detect_parameters->detect_threshold_bin = m_chart_data_filtered.horizontal_tags.get_value(threshold);
		m_thresholdval = m_chart_data_filtered.get_channel_list_item(0)
			->ConvertDataBinsToVolts(
				m_chart_data_filtered.horizontal_tags.get_value(threshold)) * 1000.f;
		m_p_detect_parameters->detect_threshold_mv = m_thresholdval;
		mm_thresholdval.m_bEntryDone = TRUE;
		OnEnChangeThresholdval();
		break;

		// ----------------------------- select bar/display bars or zoom
	case HINT_CHANGEHZLIMITS: 
		m_chart_data_filtered.get_data_from_doc(m_chart_spike_bar.get_time_first(), m_chart_spike_bar.get_time_last());
		m_chart_data_source.get_data_from_doc(m_chart_spike_bar.get_time_first(), m_chart_spike_bar.get_time_last());
		update_legends();
		break;

	case HINT_HITSPIKE:
		{
			db_spike spike_hit = GetDocument()->get_spike_hit();
			select_spike_no(spike_hit, FALSE); 
			update_spike_display();
		}
		break;

	case HINT_DBLCLKSEL:
		if (threshold < 0)
			threshold = 0;
		m_spike_index = threshold;
		OnToolsEdittransformspikes();
		break;

	case HINT_CHANGEZOOM:
		update_spike_shape_window_scale(TRUE);
		m_chart_spike_shape.Invalidate();
		break;

	case HINT_VIEWSIZECHANGED:
		if (i_id == m_chart_data_source.GetDlgCtrlID())
		{
			const auto l_first = m_chart_data_source.GetDataFirstIndex(); // get source data time range
			const auto l_last = m_chart_data_source.GetDataLastIndex();
			m_chart_data_filtered.get_data_from_doc(l_first, l_last);
		}
		// else if(iID == m_displayDetect.GetDlgCtrlID())
		// UpdateLegends updates data window from m_displayDetect
		update_legends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_view_data->viewspkdetectfiltered = *m_chart_data_filtered.get_scope_parameters();
		options_view_data->viewspkdetectdata = *m_chart_data_source.get_scope_parameters();
		options_view_data->viewspkdetectspk = *m_chart_spike_bar.get_scope_parameters();
		options_view_data->viewspkdetectbars = *m_chart_spike_shape.get_scope_parameters();
		break;

	case HINT_DEFINEDRECT:
		if (m_cursor_state == CURSOR_CROSS)
		{
			const auto rect = m_chart_data_filtered.get_defined_rect();
			int l_limit_left = m_chart_data_filtered.GetDataOffsetfromPixel(rect.left);
			int l_limit_right = m_chart_data_filtered.GetDataOffsetfromPixel(rect.right);
			if (l_limit_left > l_limit_right)
			{
				const int i = l_limit_right;
				l_limit_right = l_limit_left;
				l_limit_left = i;
			}
			m_pSpkDoc->m_stimulus_intervals.SetAtGrow(m_pSpkDoc->m_stimulus_intervals.n_items, l_limit_left);
			m_pSpkDoc->m_stimulus_intervals.n_items++;
			m_pSpkDoc->m_stimulus_intervals.SetAtGrow(m_pSpkDoc->m_stimulus_intervals.n_items, l_limit_right);
			m_pSpkDoc->m_stimulus_intervals.n_items++;
			update_VT_tags();

			m_chart_spike_bar.Invalidate();
			m_chart_data_filtered.Invalidate();
			m_chart_data_source.Invalidate();
			m_pSpkDoc->SetModifiedFlag(TRUE);
		}
		break;

	case HINT_SELECTSPIKES:
		update_spike_display();
		break;

		//case HINT_HITVERTTAG:	 //11	// vertical tag hit				lowp = tag index
		//case HINT_MOVEVERTTAG: //12	// vertical tag has moved 		lowp = new pixel / selected tag
	case HINT_CHANGEVERTTAG: //13
		{
			int lvalue = m_pSpkDoc->m_stimulus_intervals.GetAt(threshold);
			if (i_id == m_chart_data_filtered.GetDlgCtrlID())
				lvalue = m_chart_data_filtered.vertical_tags.get_tag_l_val(threshold);
			else if (i_id == m_chart_data_source.GetDlgCtrlID())
				lvalue = m_chart_data_source.vertical_tags.get_tag_l_val(threshold);

			m_pSpkDoc->m_stimulus_intervals.SetAt(threshold, lvalue);
			update_VT_tags();

			m_chart_spike_bar.Invalidate();
			m_chart_data_filtered.Invalidate();
			m_chart_data_source.Invalidate();
			m_pSpkDoc->SetModifiedFlag(TRUE);
		}
		break;

	case WM_LBUTTONDOWN:
	case HINT_LMOUSEBUTTONDOW_CTRL:
		{
			const int cx = LOWORD(lParam);
			const int l_limit_left = m_chart_data_filtered.GetDataOffsetfromPixel(cx);
			m_pSpkDoc->m_stimulus_intervals.SetAtGrow(m_pSpkDoc->m_stimulus_intervals.n_items, l_limit_left);
			m_pSpkDoc->m_stimulus_intervals.n_items++;
			update_VT_tags();

			m_chart_spike_bar.Invalidate();
			m_chart_data_filtered.Invalidate();
			m_chart_data_source.Invalidate();
			m_pSpkDoc->SetModifiedFlag(TRUE);
		}
		break;

	case HINT_HITSPIKE_SHIFT: // spike is selected or deselected
		{
			//db_spike spike_hit = GetDocument()->get_spike_hit();
			long l_first; 
			long l_last;
			m_pSpkList->get_range_of_spike_flagged(l_first, l_last);
			const auto l_time = m_pSpkList->get_spike(threshold)->get_time();
			if (l_time < l_first)
				l_first = l_time;
			if (l_time > l_last)
				l_last = l_time;
			m_pSpkList->flag_range_of_spikes(l_first, l_last, TRUE);
			update_spike_display();
		}
		break;

	case HINT_HITSPIKE_CTRL: // add/remove selected spike to/from the group of spikes selected
		{
			db_spike spike_hit = GetDocument()->get_spike_hit();
			select_spike_no(spike_hit, TRUE);
			update_spike_display();
		}
		break;

	case HINT_HITCHANNEL: // change channel if different
		if (i_id == m_chart_data_filtered.GetDlgCtrlID())
		{
			if (m_ichanselected != threshold)
			{
				m_ichanselected = threshold; // get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED, m_ichanselected);
			}
		}
		else if (i_id == m_chart_data_source.GetDlgCtrlID())
		{
			if (m_ichanselected2 != threshold)
			{
				m_ichanselected2 = threshold; // get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED2, m_ichanselected2);
			}
		}
		break;

	case HINT_VIEWTABCHANGE:
		update_detection_settings(LOWORD(lParam));
		break;

	default:
		break;
	}
	return 0L;
}

void ViewSpikeDetection::OnFirstFrame()
{
	scroll_file(SB_LEFT, 1L);
}

void ViewSpikeDetection::OnLastFrame()
{
	scroll_file(SB_RIGHT, 1L);
}

void ViewSpikeDetection::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == nullptr)
	{
		dbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	CString cs;
	switch (nSBCode)
	{
		long l_last;
		long l_first;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL);
		l_first = m_filescroll_infos.nPos;
		l_last = l_first + static_cast<long>(m_filescroll_infos.nPage) - 1;
		m_chart_data_filtered.get_data_from_doc(l_first, l_last);
		m_chart_data_source.get_data_from_doc(l_first, l_last);
		update_legends();
		break;

	default:
		scroll_file(nSBCode, nPos);
		break;
	}
}

void ViewSpikeDetection::update_file_scroll()
{
	m_filescroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_filescroll_infos.nPos = m_chart_data_filtered.GetDataFirstIndex();
	m_filescroll_infos.nPage = m_chart_data_filtered.GetDataLastIndex() - m_chart_data_filtered.GetDataFirstIndex() + 1;
	m_filescroll.SetScrollInfo(&m_filescroll_infos);
}

void ViewSpikeDetection::scroll_file(UINT nSBCode, UINT nPos)
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
		b_result = m_chart_data_source.scroll_data_from_doc(nSBCode);
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		b_result = m_chart_data_source.get_data_from_doc(
			static_cast<long>(nPos) * (m_chart_data_source.GetDocumentLast()) / 100L);
		break;
	default: // NOP: set position only
		break;
	}
	// adjust display
	if (b_result)
		update_legends();
}

void ViewSpikeDetection::OnFormatYscaleCentercurve()
{
	m_chart_data_filtered.center_chan(0);
	m_chart_data_filtered.Invalidate();

	for (auto i = 0; i < m_chart_data_source.get_channel_list_size(); i++)
		m_chart_data_source.center_chan(i);
	m_chart_data_source.Invalidate();

	m_chart_spike_bar.center_curve();
	m_chart_spike_bar.Invalidate();

	m_chart_spike_shape.set_yw_ext_org(m_chart_spike_bar.get_yw_extent(), m_chart_spike_bar.get_yw_org());
	update_spike_shape_window_scale(TRUE);
	m_chart_spike_shape.Invalidate();
}

void ViewSpikeDetection::OnFormatYscaleGainadjust()
{
	m_chart_data_filtered.max_gain_chan(0);
	m_chart_data_filtered.set_channel_list_volts_extent(-1, nullptr);
	m_chart_data_filtered.Invalidate();

	for (int i = 0; i < m_chart_data_source.get_channel_list_size(); i++)
		m_chart_data_source.max_gain_chan(i);
	m_chart_data_source.set_channel_list_volts_extent(-1, nullptr);
	m_chart_data_source.Invalidate();

	m_chart_spike_bar.max_center();
	m_chart_spike_bar.Invalidate();

	m_chart_spike_shape.set_yw_ext_org(m_chart_spike_bar.get_yw_extent(), m_chart_spike_bar.get_yw_org());
	m_chart_spike_shape.set_xw_ext_org(m_pSpkList->get_spike_length(), 0);
	update_spike_shape_window_scale(FALSE);
	m_chart_spike_shape.Invalidate();
}

void ViewSpikeDetection::OnFormatSplitcurves()
{
	m_chart_data_filtered.split_channels();
	m_chart_data_filtered.set_channel_list_volts_extent(-1, nullptr);
	m_chart_data_filtered.Invalidate();

	m_chart_data_source.split_channels();
	m_chart_data_source.set_channel_list_volts_extent(-1, nullptr);
	m_chart_data_source.Invalidate();

	// center curve and display bar & spikes
	m_chart_spike_bar.max_center();
	m_chart_spike_bar.Invalidate();

	m_chart_spike_shape.set_yw_ext_org(m_chart_spike_bar.get_yw_extent(), m_chart_spike_bar.get_yw_org());
	m_chart_spike_shape.Invalidate();

	update_spike_shape_window_scale(FALSE);
	
}

void ViewSpikeDetection::OnFormatAlldata()
{
	const auto l_last = GetDocument()->m_p_dat->get_doc_channel_length();
	m_chart_data_filtered.resize_channels(0, l_last);
	m_chart_data_filtered.get_data_from_doc(0, l_last);

	m_chart_data_source.resize_channels(0, l_last);
	m_chart_data_source.get_data_from_doc(0, l_last);

	const auto x_we = m_pSpkList->get_spike_length();
	if (x_we != m_chart_spike_shape.get_xw_extent() || 0 != m_chart_spike_shape.get_xw_org())
		m_chart_spike_shape.set_xw_ext_org(x_we, 0);
	m_chart_spike_bar.center_curve();
	m_chart_spike_shape.set_yw_ext_org(m_chart_spike_bar.get_yw_extent(), m_chart_spike_bar.get_yw_org());

	update_legends();
}

void ViewSpikeDetection::update_detection_parameters()
{
	// refresh pointer to spike detection array
	const auto detect_array_size = m_spk_detect_array_current.GetSize();
	ASSERT(m_i_detect_parameters < detect_array_size);
	m_p_detect_parameters = m_spk_detect_array_current.GetItem(m_i_detect_parameters);
	update_detection_settings(m_i_detect_parameters);

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
	GetDlgItem(IDC_COMBO1)->Invalidate();
	GetDlgItem(IDC_SOURCECHAN)->Invalidate();
	GetDlgItem(IDC_TRANSFORM)->Invalidate();
	mm_thresholdval.Invalidate();
}

void ViewSpikeDetection::OnToolsDetectionparameters()
{
	DlgSpikeDetect dlg;
	dlg.m_dbDoc = GetDocument()->m_p_dat;
	dlg.m_iDetectParmsDlg = m_i_detect_parameters; // index spk detect parameters currently selected / array
	dlg.m_pDetectSettingsArray = &m_spk_detect_array_current; // spike detection parameters array
	dlg.mdPM = options_view_data;
	dlg.m_pChartDataDetectWnd = &m_chart_data_filtered;
	if (IDOK == dlg.DoModal())
	{
		// copy modified parameters into array
		// update HorizontalCursorList on both sourceView & histogram
		m_i_detect_parameters = dlg.m_iDetectParmsDlg;
		update_detection_parameters();
	}
}

void ViewSpikeDetection::OnSelchangeDetectchan()
{
	UpdateData(TRUE);
	m_p_detect_parameters->detect_channel = m_CBdetectChan.GetCurSel();
	m_p_detect_parameters->b_changed = TRUE;
	m_chart_data_filtered.set_channel_list_y(0, m_p_detect_parameters->detect_channel, m_p_detect_parameters->detect_transform);
	const CChanlistItem* channel_list_item = m_chart_data_filtered.get_channel_list_item(0);
	m_p_detect_parameters->detect_threshold_bin = channel_list_item->ConvertVoltsToDataBins(m_p_detect_parameters->detect_threshold_mv / 1000.f);
	m_chart_data_filtered.move_hz_tag_to_val(0, m_p_detect_parameters->detect_threshold_bin);
	m_chart_data_filtered.get_data_from_doc();
	m_chart_data_filtered.AutoZoomChan(0);
	m_chart_data_filtered.Invalidate();
}

void ViewSpikeDetection::OnSelchangeTransform()
{
	UpdateData(TRUE);
	m_p_detect_parameters->detect_transform = m_CBtransform.GetCurSel();
	m_p_detect_parameters->b_changed = TRUE;
	m_chart_data_filtered.set_channel_list_transform_mode(0, m_p_detect_parameters->detect_transform);
	m_chart_data_filtered.get_data_from_doc();
	m_chart_data_filtered.AutoZoomChan(0);
	m_chart_data_filtered.Invalidate();
	update_legend_detection_wnd();
}

void ViewSpikeDetection::OnMeasureAll()
{
	detect_all(TRUE);
}

void ViewSpikeDetection::OnMeasure()
{
	detect_all(FALSE);
}

void ViewSpikeDetection::detect_all(BOOL bAll)
{
	m_bDetected = TRUE;
	m_pSpkDoc->SetModifiedFlag(TRUE);

	const auto db_document = GetDocument();
	const auto data_document = db_document->m_p_dat;
	m_pSpkDoc->set_acq_filename(db_document->db_get_current_dat_file_name());
	m_pSpkDoc->init_source_doc(data_document);

	m_pSpkDoc->set_detection_date(CTime::GetCurrentTime());
	auto old_spike_list_index = db_document->get_current_spike_file()->get_spike_list_current_index();
	m_spike_index = -1;

	// check if detection parameters are ok? prevent detection from a channel that does not exist
	const auto p_dat = db_document->m_p_dat;
	if (p_dat == nullptr)
		return;
	const auto wave_format = p_dat->get_waveformat();
	const auto chan_max = wave_format->scan_count - 1;
	for (auto i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		const auto spike_detect_array = m_spk_detect_array_current.GetItem(i);
		if (spike_detect_array->extract_channel > chan_max)
		{
			MessageBox(_T(
				"Check spike detection parameters \n- one of the detection channel requested \nis not available in the source data"));
			return;
		}
	}

	// adjust size of spike list array
	if (m_spk_detect_array_current.GetSize() != m_pSpkDoc->get_spike_list_size())
		m_pSpkDoc->set_spike_list_size(m_spk_detect_array_current.GetSize());

	// detect spikes from all channels marked as such
	for (int i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		if (!bAll && m_i_detect_parameters != i)
			continue;
		// detect missing data channel
		if (m_spk_detect_array_current.GetItem(i)->extract_channel > chan_max)
			continue;

		// select new spike list (list with no spikes for stimulus channel)
		SpikeList* spike_list = m_pSpkDoc->set_spike_list_current_index(i);
		if (spike_list == nullptr)
		{
			m_pSpkDoc->add_spk_list();
			spike_list = m_pSpkDoc->get_spike_list_current();
		}

		m_pSpkList = spike_list;
		if (m_pSpkList->get_spikes_count() == 0)
		{
			SPKDETECTPARM* pFC = m_spk_detect_array_current.GetItem(i);
			ASSERT_VALID(pFC);
			ASSERT(pFC != NULL);
			m_pSpkList->init_spike_list(db_document->m_p_dat, pFC);
		}
		if ((m_spk_detect_array_current.GetItem(i))->detect_what == DETECT_SPIKES)
		{
			detect_method_1(static_cast<WORD>(i)); 
		}
		else
		{
			detect_stimulus_1(i); 
			update_VT_tags(); 
			m_chart_spike_bar.Invalidate();
			m_chart_data_filtered.Invalidate();
			m_chart_data_source.Invalidate();
		}
	}

	// save spike file
	save_current_spk_file();

	// display data
	if (old_spike_list_index < 0)
		old_spike_list_index = 0;
	m_pSpkList = m_pSpkDoc->set_spike_list_current_index(old_spike_list_index);

	m_chart_spike_bar.set_source_data(m_pSpkList, db_document);
	m_chart_spike_shape.set_source_data(m_pSpkList, db_document);


	// center spikes, change nb spikes and update content of draw buttons
	if (options_view_data->bMaximizeGain
		|| m_chart_spike_bar.get_yw_extent() == 0
		|| m_chart_spike_bar.get_yw_org() == 0
		|| m_chart_spike_shape.get_yw_extent() == 0
		|| m_chart_spike_shape.get_yw_org() == 0)
	{
		m_chart_spike_bar.max_center();
		m_chart_spike_shape.set_yw_ext_org(m_chart_spike_bar.get_yw_extent(), m_chart_spike_bar.get_yw_org());
		int spike_length = 60;
		if (m_pSpkList != nullptr)
			spike_length = m_pSpkList->get_spike_length();
		m_chart_spike_shape.set_xw_ext_org(spike_length, 0);
		update_spike_shape_window_scale(FALSE);
	}

	highlight_spikes(FALSE);
	update_legends();
	update_tabs();
}

int ViewSpikeDetection::detect_stimulus_1(int channel_index)
{
	const auto detect_parameters = m_spk_detect_array_current.GetItem(channel_index);
	const auto threshold = detect_parameters->detect_threshold_bin;
	const auto detect_transform = detect_parameters->detect_transform; 
	const auto source_channel = detect_parameters->detect_channel;
	const auto data_document = GetDocument()->m_p_dat;
	const auto detect_transform_span = data_document->get_transformed_data_span(detect_transform); 

	// detect mode: 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);
	auto b_cross_upw = TRUE;
	if (detect_parameters->detect_mode == MODE_OFF_ON || detect_parameters->detect_mode == MODE_OFF_OFF)
		b_cross_upw = FALSE;
	auto b_mode = TRUE;
	if (detect_parameters->detect_mode == MODE_ON_ON || detect_parameters->detect_mode == MODE_OFF_OFF)
		b_mode = FALSE;
	auto b_save_on = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = m_chart_data_filtered.GetDataFirstIndex();
	const auto l_data_last = m_chart_data_filtered.GetDataLastIndex();

	// plot progress dialog box
	DlgProgress dlg;
	dlg.Create();
	dlg.SetRange(0, 100);
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
			dlg.SetPos(pct_achieved);
			CString comment;
			comment.Format(_T("Processing stimulus event: %i"), m_pSpkDoc->m_stimulus_intervals.n_items + 1);
			dlg.SetStatus(comment);

			if (dlg.CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				{
					l_last = l_data_last;
					const auto stimulus_intervals = &(m_pSpkDoc->m_stimulus_intervals);
					stimulus_intervals->RemoveAll();
					m_pSpkDoc->m_stimulus_intervals.n_items = 0;
					break;
				}

			// check if already present and insert it at the proper place
			const auto stimulus_intervals = &(m_pSpkDoc->m_stimulus_intervals);
			auto flag = TRUE;
			int i2;
			for (i2 = 0; i2 < stimulus_intervals->GetSize(); i2++)
			{
				constexpr auto jitter = 2;
				const auto l_value = stimulus_intervals->GetAt(i2);
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
				stimulus_intervals->InsertAt(i2, cx);
				m_pSpkDoc->m_stimulus_intervals.n_items++;
			}
		}
		l_data_first = l_last + 1; // update for next loop
	}

	return m_pSpkDoc->m_stimulus_intervals.n_items;
}

int ViewSpikeDetection::detect_method_1(WORD channel_index)
{
	const SPKDETECTPARM* spike_detection_parameters = m_spk_detect_array_current.GetItem(channel_index);
	if (spike_detection_parameters->extract_transform != spike_detection_parameters->detect_transform &&
		spike_detection_parameters->extract_transform != 0)
	{
		AfxMessageBox(
			_T("Options not implemented yet!\nd chan == extr chan or !extr chan=0\nChange detection parameters"));
		return m_pSpkList->get_spikes_count();
	}

	// set parameters (copy array into local parms)
	const auto threshold = static_cast<short>(spike_detection_parameters->detect_threshold_bin); 
	const auto detect_transform = spike_detection_parameters->detect_transform;
	const auto source_channel = spike_detection_parameters->detect_channel; 
	const auto pre_threshold = spike_detection_parameters->detect_pre_threshold; 
	const auto refractory = spike_detection_parameters->detect_refractory_period; 
	const auto post_threshold = spike_detection_parameters->extract_n_points - pre_threshold;

	// get parameters from document
	const auto p_dat = GetDocument()->m_p_dat;
	int n_channels = p_dat->get_scan_count();
	const auto p_buf = p_dat->get_raw_data_buffer();
	const auto span = p_dat->get_transformed_data_span(detect_transform);

	// adjust detection method: if threshold lower than data zero detect lower crossing
	auto b_cross_upw = TRUE;
	if (threshold < 0)
		b_cross_upw = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = m_chart_data_filtered.GetDataFirstIndex(); // index first pt to test
	auto l_data_last = m_chart_data_filtered.GetDataLastIndex(); // index last pt to test
	if (l_data_first < pre_threshold + span)
		l_data_first = static_cast<long>(pre_threshold) + span;
	if (l_data_last > p_dat->get_doc_channel_length() - post_threshold - span)
		l_data_last = p_dat->get_doc_channel_length() - post_threshold - span;

	// loop through data defined in the lineview window
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
				m_pSpkList->add_spike(p_m, 1, ii_time, source_channel, 0, TRUE);
			}
			else // extract from raw data
			{
				const auto pM = p_buf
					+ n_channels * (ii_time - pre_threshold - l_rw_first + span)
					+ spike_detection_parameters->extract_channel;
				m_pSpkList->add_spike(pM, n_channels, ii_time, source_channel, 0, TRUE);
			}

			// update loop parameters
			cx = ii_time + refractory;
		}

		///////////////////////////////////////////////////////////////
		l_data_first = cx + 1; // update for next loop
	}

	return m_pSpkList->get_spikes_count();
}

void ViewSpikeDetection::OnToolsEdittransformspikes()
{
	DlgSpikeEdit dlg; // dialog box
	dlg.y_extent = m_chart_spike_shape.get_yw_extent();
	dlg.y_zero = m_chart_spike_shape.get_yw_org(); 
	dlg.x_extent = m_chart_spike_shape.get_xw_extent(); 
	dlg.x_zero = m_chart_spike_shape.get_xw_org(); 
	dlg.spike_index = m_spike_index; 
	m_pSpkList->remove_all_spike_flags();
	dlg.db_wave_doc = GetDocument();
	dlg.m_parent = this;

	// open dialog box and wait for response
	dlg.DoModal();

	m_spike_index = dlg.spike_index;
	db_spike spike_sel(-1, -1, m_spike_index);
	select_spike_no(spike_sel, FALSE);
	update_spike_display();
	if (dlg.b_changed)
		m_pSpkDoc->SetModifiedFlag(TRUE);

	update_legends();
}

void ViewSpikeDetection::OnFormatXscale()
{
	DlgXYParameters dlg;
	CWnd* pFocus = GetFocus();

	if (pFocus != nullptr && m_chart_data_filtered.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		CChanlistItem* pchan = m_chart_data_filtered.get_channel_list_item(m_p_detect_parameters->detect_channel);
		dlg.m_yzero = pchan->GetYzero();
		dlg.m_yextent = pchan->GetYextent();
		dlg.m_bDisplaysource = TRUE;
	}
	else if (pFocus != nullptr && m_chart_spike_bar.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		dlg.m_yzero = m_chart_spike_bar.get_yw_org();
		dlg.m_yextent = m_chart_spike_bar.get_yw_extent();
		dlg.m_bDisplaybars = TRUE;
	}
	else
	{
		dlg.m_xzero = m_chart_spike_shape.get_xw_org();
		dlg.m_xextent = m_chart_spike_shape.get_xw_extent();
		dlg.m_yzero = m_chart_spike_shape.get_yw_org();
		dlg.m_yextent = m_chart_spike_shape.get_yw_extent();
		dlg.m_bDisplayspikes = TRUE;
	}

	if (IDOK == dlg.DoModal())
	{
		if (dlg.m_bDisplaysource)
		{
			CChanlistItem* chan = m_chart_data_filtered.get_channel_list_item(0);
			chan->SetYzero(dlg.m_yzero);
			chan->SetYextent(dlg.m_yextent);
			m_chart_data_filtered.Invalidate();
		}
		if (dlg.m_bDisplaybars)
		{
			m_chart_spike_bar.set_yw_ext_org(dlg.m_yextent, dlg.m_yzero);
			m_chart_spike_bar.Invalidate();
		}
		if (dlg.m_bDisplayspikes)
		{
			m_chart_spike_shape.set_yw_ext_org(dlg.m_yextent, dlg.m_yzero);
			m_chart_spike_shape.set_xw_ext_org(dlg.m_xextent, dlg.m_xzero);
			m_chart_spike_shape.Invalidate();
		}
	}
}

void ViewSpikeDetection::OnBnClickedClearAll()
{
	m_spike_index = -1;
	db_spike spike_sel(-1, -1, -1);
	m_chart_spike_bar.select_spike(spike_sel);
	m_chart_spike_shape.select_spike(spike_sel);

	// update spike list
	for (int i = 0; i < m_pSpkDoc->get_spike_list_size(); i++)
	{
		SpikeList* p_spk_list = m_pSpkDoc->set_spike_list_current_index(i);
		p_spk_list->init_spike_list(GetDocument()->m_p_dat, nullptr);
	}
	m_pSpkList = m_pSpkDoc->get_spike_list_current();
	ASSERT(m_pSpkList != NULL);

	highlight_spikes(FALSE); // remove display of spikes
	m_chart_spike_shape.set_source_data(m_pSpkList, GetDocument());
	m_pSpkDoc->m_stimulus_intervals.n_items = 0; // zero stimuli
	m_pSpkDoc->m_stimulus_intervals.RemoveAll();

	update_detection_parameters();
	update_VT_tags(); // update display of vertical tags
	update_legends(); // change legends
	m_pSpkDoc->SetModifiedFlag(TRUE); // mark spike document as changed
}

void ViewSpikeDetection::OnClear()
{
	m_spike_index = -1;
	db_spike spike_sel(-1, -1, -1);
	m_chart_spike_bar.select_spike(spike_sel);
	m_chart_spike_shape.select_spike(spike_sel);

	m_pSpkList = m_pSpkDoc->get_spike_list_current();
	m_pSpkList->init_spike_list(GetDocument()->m_p_dat, nullptr);
	highlight_spikes(FALSE);

	if (m_pSpkList->get_detection_parameters()->detect_what == DETECT_STIMULUS)
	{
		m_pSpkDoc->m_stimulus_intervals.n_items = 0;
		m_pSpkDoc->m_stimulus_intervals.RemoveAll();
		update_VT_tags();
	}

	update_legends();
	m_pSpkDoc->SetModifiedFlag(TRUE);
	// TODO : upate database?
}

void ViewSpikeDetection::OnEnChangeSpikeno()
{
	if (mm_spikeno.m_bEntryDone)
	{
		mm_spikeno.OnEnChange(this, m_spike_index, 1, -1);
		// check boundaries
		if (m_spike_index < -1)
			m_spike_index = -1;
		if (m_spike_index >= m_pSpkList->get_spikes_count())
			m_spike_index = m_pSpkList->get_spikes_count() - 1;

		// update spike num and center display on the selected spike
		db_spike spike_sel(-1, -1, m_spike_index);
		select_spike_no(spike_sel, FALSE);
		update_spike_display();
	}
}

void ViewSpikeDetection::OnArtefact()
{
	UpdateData(TRUE); 
	const auto n_spikes = m_pSpkList->get_spike_flag_array_count();
	if (n_spikes < 1)
	{
		m_bartefact = FALSE; // no action if spike index < 0
	}
	else
	{
		// load old class nb
		ASSERT(n_spikes >= 0);
		for (auto i = 0; i < n_spikes; i++)
		{
			const auto spike_no = m_pSpkList->get_spike_flag_array_at(i);
			Spike* spike = m_pSpkList->get_spike(spike_no);
			auto spike_class = spike->get_class_id();

			// if artefact: set class to negative value
			if ((m_bartefact && spike_class >= 0) || (spike_class < 0))
				spike_class = -(spike_class + 1);
			spike->set_class_id(spike_class);
		}

		m_pSpkDoc->SetModifiedFlag(TRUE);
		save_current_spk_file();
	}
	m_spike_index = -1;

	const auto i_sel_parameters = m_tabCtrl.GetCurSel();
	m_p_detect_parameters = m_spk_detect_array_current.GetItem(i_sel_parameters);
	m_pSpkList = m_pSpkDoc->set_spike_list_current_index(i_sel_parameters);

	db_spike spike_sel(-1, -1, m_spike_index);
	select_spike_no(spike_sel, FALSE);
	update_spike_display();
	update_number_of_spikes();
}

void ViewSpikeDetection::update_number_of_spikes()
{
	const int total_spikes = m_pSpkList->get_spikes_count();
	if (total_spikes != static_cast<int>(GetDlgItemInt(IDC_NBSPIKES_NB)))
		SetDlgItemInt(IDC_NBSPIKES_NB, total_spikes);
}

void ViewSpikeDetection::align_display_to_current_spike()
{
	if (m_spike_index < 0)
		return;

	const auto l_spike_time = m_pSpkList->get_spike(m_spike_index)->get_time();
	if (l_spike_time < m_chart_data_filtered.GetDataFirstIndex()
		|| l_spike_time > m_chart_data_filtered.GetDataLastIndex())
	{
		const auto l_size = m_chart_data_filtered.GetDataLastIndex() - m_chart_data_filtered.GetDataFirstIndex();
		auto l_first = l_spike_time - l_size / 2;
		if (l_first < 0)
			l_first = 0;
		auto l_last = l_first + l_size - 1;
		if (l_last > m_chart_data_filtered.GetDocumentLast())
		{
			l_last = m_chart_data_filtered.GetDocumentLast();
			l_first = l_last - l_size + 1;
		}
		m_chart_data_filtered.get_data_from_doc(l_first, l_last);
		m_chart_data_source.get_data_from_doc(l_first, l_last);

		update_legends();
	}
}

void ViewSpikeDetection::update_spike_shape_window_scale(const BOOL b_set_from_controls)
{
	// get current values
	int ix_we;
	auto iy_we = 0;

	// if set from controls, get value from the controls
	if (b_set_from_controls && m_pSpkList->get_spikes_count() > 0)
	{
		// set time scale
		CString cs;
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->GetWindowText(cs);
		const auto x = static_cast<float>(_ttof(cs)) / 1000.0f;
		ix_we = static_cast<int>(m_pSpkList->get_acq_sampling_rate() * x);
		if (ix_we == 0)
			ix_we = m_pSpkList->get_detection_parameters()->extract_n_points;
		ASSERT(ix_we != 0);
		m_chart_spike_shape.set_xw_ext_org(ix_we, m_chart_spike_shape.get_xw_org());
		
		// set amplitude
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->GetWindowText(cs);
		if (!cs.IsEmpty())
		{
			const auto y = static_cast<float>(_ttof(cs)) / 1000.0f;
			iy_we = static_cast<int>(y / m_pSpkList->get_acq_volts_per_bin());
		}
		if (iy_we == 0)
			iy_we = m_chart_spike_shape.get_yw_extent();
		m_chart_spike_shape.set_yw_ext_org(iy_we, m_chart_spike_shape.get_yw_org());
	}
	else
	{
		ix_we = m_chart_spike_shape.get_xw_extent();
		iy_we = m_chart_spike_shape.get_yw_extent();
	}

	if (ix_we != NULL && iy_we != NULL)
	{
		const float x = m_chart_spike_shape.get_extent_ms() / static_cast<float>(m_chart_spike_shape.get_nx_scale_cells());
		m_chart_spike_shape.set_x_scale_unit_value(x);

		const float y = m_chart_spike_shape.get_extent_m_v() / static_cast<float>(m_chart_spike_shape.get_ny_scale_cells());
		m_chart_spike_shape.set_y_scale_unit_value(y);
		
	}
	
	CString dummy1;
	dummy1.Format(_T("%.3lf"), m_chart_spike_shape.get_extent_m_v());
	SetDlgItemText(IDC_SPIKEWINDOWAMPLITUDE, dummy1);

	CString dummy2;
	dummy2.Format(_T("%.3lf"), m_chart_spike_shape.get_extent_ms());
	SetDlgItemText(IDC_SPIKEWINDOWLENGTH, dummy2);
}

void ViewSpikeDetection::select_spike_no(db_spike& spike_sel, BOOL bMultipleSelection)
{
	if (spike_sel.spike_index >= 0)
	{
		m_pSpkList = m_pSpkDoc->get_spike_list_current();
		const auto p_spike_element = m_pSpkList->get_spike(spike_sel.spike_index);
		m_bartefact = (p_spike_element->get_class_id() < 0);
		if (bMultipleSelection)
		{
			m_pSpkList->toggle_spike_flag(spike_sel.spike_index);
			if (m_pSpkList->get_spike_flag_array_count() < 1)
				spike_sel.spike_index = -1;
			if (m_spike_index == spike_sel.spike_index)
				spike_sel.spike_index = 0;
		}
		else
		{
			m_pSpkList->set_single_spike_flag(spike_sel.spike_index);
			m_pSpkList->m_selected_spike = m_spike_index;
		}
		m_spike_index = spike_sel.spike_index;
		align_display_to_current_spike();
	}
	else
	{
		m_pSpkList->remove_all_spike_flags();
		m_bartefact = FALSE;
	}
}

void ViewSpikeDetection::update_spike_display()
{
	// update spike display windows
	m_chart_spike_bar.Invalidate(TRUE);
	m_chart_spike_shape.Invalidate(TRUE);

	// update Dlg interface
	GetDlgItem(IDC_SPIKENO)->EnableWindow(!(m_pSpkList->get_spike_flag_array_count() > 1));
	SetDlgItemInt(IDC_SPIKENO, m_spike_index, TRUE);
	CheckDlgButton(IDC_ARTEFACT, m_bartefact);
}

void ViewSpikeDetection::OnEnChangeThresholdval()
{
	if (mm_thresholdval.m_bEntryDone)
	{
		auto threshold_value = m_thresholdval;
		mm_thresholdval.OnEnChange(this, m_thresholdval, 1.f, -1.f);

		// change display if necessary
		if (m_thresholdval != threshold_value)
		{
			m_thresholdval = threshold_value;
			m_p_detect_parameters->detect_threshold_mv = threshold_value;
			const CChanlistItem* channel_item = m_chart_data_filtered.get_channel_list_item(0);
			m_p_detect_parameters->detect_threshold_bin = channel_item->ConvertVoltsToDataBins(m_thresholdval / 1000.f);
			m_chart_data_filtered.move_hz_tag_to_val(0, m_p_detect_parameters->detect_threshold_bin);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeDetection::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone)
	{
		mm_timefirst.OnEnChange(this, m_timefirst, 1.f, -1.f);
		m_chart_data_filtered.get_data_from_doc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		m_chart_data_source.get_data_from_doc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		update_legends();
	}
}

void ViewSpikeDetection::OnEnChangeTimelast()
{
	if (mm_timelast.m_bEntryDone)
	{
		mm_timelast.OnEnChange(this, m_timelast, 1.f, -1.f);

		m_chart_data_filtered.get_data_from_doc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		m_chart_data_source.get_data_from_doc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		update_legends();
	}
}

void ViewSpikeDetection::OnToolsDataseries()
{
	// init dialog data
	DlgDataSeries dlg;
	dlg.m_pChartDataWnd = &m_chart_data_filtered;
	dlg.m_pdbDoc = GetDocument()->m_p_dat;
	dlg.m_listindex = 0;

	// invoke dialog box
	dlg.DoModal();
	if (m_chart_data_filtered.get_channel_list_size() < 1)
	{
		m_chart_data_filtered.remove_all_channel_list_items();
		m_chart_data_filtered.add_channel_list_item(m_p_detect_parameters->detect_channel, m_p_detect_parameters->detect_transform);
	}
	update_legends();
}

void ViewSpikeDetection::PrintDataCartridge(CDC* p_dc, ChartData* pDataChartWnd, CRect* prect, BOOL bComments)
{
	SCOPESTRUCT* p_struct = pDataChartWnd->get_scope_parameters();
	const auto b_draw_f = p_struct->bDrawframe;
	p_struct->bDrawframe = TRUE;
	pDataChartWnd->Print(p_dc, prect, (options_view_data->bcontours == 1));
	p_struct->bDrawframe = b_draw_f;

	// data vertical and horizontal bars
	const auto comments = PrintDataBars(p_dc, pDataChartWnd, prect);

	const int left = prect->left;
	const int top = prect->top;
	p_dc->TextOut(left, top, comments);

	p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
}

void ViewSpikeDetection::OnEditCopy()
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
			m_chart_data_filtered.copy_as_text(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			serialize_windows_state(b_save);

			CRect old_rect1; // save size of line view windows
			m_chart_data_filtered.GetWindowRect(&old_rect1);
			CRect old_rect2;
			m_chart_data_source.GetWindowRect(&old_rect2);

			const CRect rect(0, 0, options_view_data->hzResolution, options_view_data->vtResolution);
			m_pixels_count_0_ = m_chart_data_filtered.get_rect_width();

			// create meta file
			CMetaFileDC m_dc;
			auto rect_bound = rect;
			rect_bound.right *= 32;
			rect_bound.bottom *= 30;
			const auto p_dc_ref = GetDC();
			auto cs_title = _T("dbWave\0") + GetDocument()->m_p_dat->GetTitle();
			cs_title += _T("\0\0");
			const auto hm_dc = m_dc.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
			ASSERT(hm_dc != NULL);

			// Draw document in meta file.
			CPen black_pen(PS_SOLID, 0, RGB(0, 0, 0));
			const auto old_pen = m_dc.SelectObject(&black_pen);
			const auto p_old_brush = static_cast<CBrush*>(m_dc.SelectStockObject(BLACK_BRUSH));
			CClientDC attribute_dc(this); 
			m_dc.SetAttribDC(attribute_dc.GetSafeHdc()); // from current screen

			// print comments : set font
			m_p_old_font_ = nullptr;
			const auto old_font_size = options_view_data->fontsize;
			options_view_data->fontsize = 10;
			PrintCreateFont();
			m_dc.SetBkMode(TRANSPARENT);
			options_view_data->fontsize = old_font_size;
			m_p_old_font_ = m_dc.SelectObject(&m_font_print_);
			const int line_height = m_log_font_.lfHeight + 5;
			auto row = 0;
			const auto column = 10;

			// comment and descriptors
			auto comments = GetDocument()->export_database_data(1);
			m_dc.TextOut(column, row, comments);
			row += line_height;

			// abscissa
			comments = _T("Abcissa: ");
			CString content;
			GetDlgItem(IDC_TIMEFIRST)->GetWindowText(content);
			comments += content;
			comments += _T(" - ");
			GetDlgItem(IDC_TIMELAST)->GetWindowText(content);
			comments += content;
			m_dc.TextOut(column, row, comments);

			// define display sizes - dataview & datadetect are same, spkshape & spkbar = as on screen
			auto data_rect = rect;
			data_rect.top -= -3 * line_height;
			const auto rect_spike_width = MulDiv(m_chart_spike_shape.get_rect_width(), data_rect.Width(),
				m_chart_spike_shape.get_rect_width() + m_chart_data_filtered.get_rect_width());
			const auto rect_data_height = MulDiv(m_chart_data_filtered.get_rect_height(), data_rect.Height(),
				m_chart_data_filtered.get_rect_height() * 2 + m_chart_spike_bar.
				get_rect_height());
			const auto separator = rect_spike_width / 10;

			// display curves : data
			data_rect.bottom = rect.top + rect_data_height - separator / 2;
			data_rect.left = rect.left + rect_spike_width + separator;
			PrintDataCartridge(&m_dc, &m_chart_data_source, &data_rect, TRUE);

			// display curves: detect channel
			data_rect.top = data_rect.bottom + separator;
			data_rect.bottom = data_rect.top + rect_data_height;
			PrintDataCartridge(&m_dc, &m_chart_data_filtered, &data_rect, TRUE);

			// display spike bars
			auto rect_bars = data_rect;
			rect_bars.top = data_rect.bottom + separator;
			rect_bars.bottom = rect.bottom - 2 * line_height;
			m_chart_spike_bar.print(&m_dc, &rect_bars);

			// display spike shapes
			auto rect_spikes = rect; // compute output rectangle
			rect_spikes.left += separator;
			rect_spikes.right = rect.left + rect_spike_width;
			rect_spikes.bottom = rect.bottom - 2 * line_height;
			rect_spikes.top = rect_spikes.bottom - rect_bars.Height();
			m_chart_spike_shape.print(&m_dc, &rect_spikes);
			comments = PrintSpkShapeBars(&m_dc, &rect_spikes, TRUE);

			auto rect_comment = rect;
			rect_comment.right = data_rect.left;
			rect_comment.top = rect_spikes.bottom;
			constexpr UINT n_format = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
			m_dc.DrawText(comments, comments.GetLength(), rect_comment, n_format);
			m_dc.SelectObject(p_old_brush);

			if (m_p_old_font_ != nullptr)
				m_dc.SelectObject(m_p_old_font_);
			m_font_print_.DeleteObject();

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

void ViewSpikeDetection::OnSelchangeDetectMode()
{
	UpdateData(TRUE);
	m_p_detect_parameters->detect_what = m_CBdetectWhat.GetCurSel();
	update_combo_box();
	update_legend_detection_wnd();
	m_chart_data_filtered.get_data_from_doc();
	m_chart_data_filtered.AutoZoomChan(0);
	m_chart_data_filtered.Invalidate();
}

void ViewSpikeDetection::update_combo_box()
{
	m_CBdetectChan.SetCurSel(m_p_detect_parameters->detect_channel);
	m_CBtransform.SetCurSel(m_p_detect_parameters->detect_transform);
	m_chart_data_filtered.set_channel_list_y(0, m_p_detect_parameters->detect_channel, m_p_detect_parameters->detect_transform);
	m_p_detect_parameters->detect_threshold_bin = m_chart_data_filtered.get_channel_list_item(0)->ConvertVoltsToDataBins(
		m_thresholdval / 1000.f);
	m_chart_data_filtered.horizontal_tags.set_tag_chan(0, 0);
	m_chart_data_filtered.horizontal_tags.set_tag_val(0, m_p_detect_parameters->detect_threshold_bin);
	m_p_detect_parameters->detect_threshold_mv = m_thresholdval;
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

void ViewSpikeDetection::update_VT_tags()
{
	m_chart_spike_bar.vertical_tags.remove_all_tags();
	m_chart_data_filtered.vertical_tags.remove_all_tags();
	m_chart_data_source.vertical_tags.remove_all_tags();
	if (m_pSpkDoc->m_stimulus_intervals.n_items == 0)
		return;

	for (auto i = 0; i < m_pSpkDoc->m_stimulus_intervals.GetSize(); i++)
	{
		const int cx = m_pSpkDoc->m_stimulus_intervals.GetAt(i);
		m_chart_spike_bar.vertical_tags.add_l_tag(cx, 0);
		m_chart_data_filtered.vertical_tags.add_l_tag(cx, 0);
		m_chart_data_source.vertical_tags.add_l_tag(cx, 0);
	}
}

void ViewSpikeDetection::PrintComputePageSize()
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
	options_view_data->horzRes = dc.GetDeviceCaps(HORZRES);
	options_view_data->vertRes = dc.GetDeviceCaps(VERTRES);

	// margins (pixels)
	m_print_rect_.right = options_view_data->horzRes - options_view_data->rightPageMargin;
	m_print_rect_.bottom = options_view_data->vertRes - options_view_data->bottomPageMargin;
	m_print_rect_.left = options_view_data->leftPageMargin;
	m_print_rect_.top = options_view_data->topPageMargin;
}

void ViewSpikeDetection::PrintFileBottomPage(CDC* p_dc, const CPrintInfo* p_info)
{
	auto t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"),
		p_info->m_nCurPage, p_info->GetMaxPage(),
		t.GetDay(), t.GetMonth(), t.GetYear());
	const auto ch_date = GetDocument()->db_get_current_spk_file_name();
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_view_data->horzRes / 2, options_view_data->vertRes - 57, ch_date);
}

CString ViewSpikeDetection::PrintConvertFileIndex(const long l_first, const long l_last)
{
	CString cs_unit = _T(" s");
	CString cs_comment;
	float x_scale_factor;
	const auto x1 = CNiceUnit::change_unit(static_cast<float>(l_first) / m_samplingRate, &cs_unit,
		&x_scale_factor);
	const auto fraction1 = static_cast<int>((x1 - static_cast<int>(x1)) * static_cast<float>(1000.));
	// separate fractional part
	const auto x2 = static_cast<float>(l_last) / (m_samplingRate * x_scale_factor);
	const auto fraction2 = static_cast<int>((x2 - static_cast<int>(x2)) * static_cast<float>(1000.));
	cs_comment.Format(_T("time = %i.%03.3i - %i.%03.3i"), 
		static_cast<int>(x1), 
		fraction1, static_cast<int>(x2),
		fraction2);
	cs_comment += cs_unit;
	return cs_comment;
}

BOOL ViewSpikeDetection::PrintGetFileSeriesIndexFromPage(int page, int& file_number, long& l_first)
{
	// loop until we get all rows
	const auto total_rows = m_nb_rows_per_page_ * (page - 1);
	l_first = m_l_print_first_;
	file_number = 0; 
	if (options_view_data->bPrintSelection) 
		file_number = m_file_0_;
	else
		GetDocument()->db_move_first();

	auto very_last = m_l_print_first_ + m_l_print_len_;
	if (options_view_data->bEntireRecord)
		very_last = GetDocument()->db_get_data_len() - 1;

	for (auto row = 0; row < total_rows; row++)
	{
		if (!PrintGetNextRow(file_number, l_first, very_last))
			break;
	}

	return TRUE;
}

BOOL ViewSpikeDetection::PrintGetNextRow(int& file_index, long& l_first, long& very_last)
{
	if (!options_view_data->bMultirowDisplay || !options_view_data->bEntireRecord)
	{
		file_index++;
		if (file_index >= m_files_count_)
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
			file_index++; // next index
			if (file_index >= m_files_count_) // last file ??
				return FALSE;

			GetDocument()->db_move_next();
			very_last = GetDocument()->db_get_data_len() - 1;
			l_first = m_l_print_first_;
		}
	}
	return TRUE;
}

CString ViewSpikeDetection::PrintGetFileInfos()
{
	CString str_comment; 
	const CString tab(_T("    ")); 
	const CString rc(_T("\n")); 

	// document's name, date and time
	const auto p_data_file = GetDocument()->m_p_dat;
	const auto wave_format = p_data_file->get_waveformat();
	if (options_view_data->bDocName || options_view_data->bAcqDateTime)
	{
		if (options_view_data->bDocName) 
			str_comment += GetDocument()->db_get_current_dat_file_name() + tab;

		if (options_view_data->bAcqDateTime) 
		{
			const auto date = (wave_format->acquisition_time).Format(_T("%#d %B %Y %X")); 
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_view_data->bAcqComment)
		str_comment += GetDocument()->export_database_data(); 

	return str_comment;
}

CString ViewSpikeDetection::PrintDataBars(CDC* p_dc, ChartData* pDataChartWnd, const CRect* rect)
{
	CString cs;
	const CString rc(_T("\r"));
	const CString tab(_T("     "));
	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
	CString cs_unit; 
	const CPoint bar_origin(-10, -10); // bar origin at 10,10 pts on the left lower corner of the rectangle
	auto i_horizontal_bar = pDataChartWnd->get_rect_width() / 10; // initial horizontal bar length 1/10th of display rect
	auto i_vertical_bar = pDataChartWnd->get_rect_height() / 3; // initial vertical bar height 1/3rd  of display rect

	auto str_comment = PrintConvertFileIndex(pDataChartWnd->GetDataFirstIndex(), pDataChartWnd->GetDataLastIndex());

	///// horizontal time bar ///////////////////////////
	if (options_view_data->bTimeScaleBar)
	{
		// convert bar size into time units and back into pixels
		cs_unit = _T(" s"); 
		const auto time_per_pixel = pDataChartWnd->get_time_per_pixel();
		const auto z = time_per_pixel * static_cast<float>(i_horizontal_bar); // convert 1/10 of the length of the data displayed into time
		float x_scale_factor;
		const auto x = CNiceUnit::change_unit(z, &cs_unit, &x_scale_factor); // convert time into a scaled time
		const auto k = CNiceUnit::nice_unit(x); // convert the (scaled) time value into time expressed as an integral
		i_horizontal_bar = static_cast<int>((static_cast<float>(k) * x_scale_factor) / time_per_pixel);
		// compute how much pixels it makes
		// print out the scale and units
		cs.Format(_T("horz bar = %i %s"), k, (LPCTSTR)cs_unit);
		str_comment += cs + rc;
		// draw horizontal line
		i_horizontal_bar = MulDiv(i_horizontal_bar, rect->Width(), pDataChartWnd->get_rect_width());
		p_dc->MoveTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y);
		p_dc->LineTo(rect->left + bar_origin.x + i_horizontal_bar, rect->bottom - bar_origin.y);
	}

	///// vertical voltage bars ///////////////////////////
	float y_scale_factor; // compute a good unit for channel 0
	cs_unit = _T(" V"); 

	// convert bar size into voltage units and back into pixels
	const auto volts_per_pixel = pDataChartWnd->get_channel_list_volts_per_pixel(0);
	const auto z_volts = volts_per_pixel * static_cast<float>(i_vertical_bar); // convert 1/3 of the height into voltage
	const auto z_scale = CNiceUnit::change_unit(z_volts, &cs_unit, &y_scale_factor);
	const auto z_nice = static_cast<float>(CNiceUnit::nice_unit(z_scale));
	i_vertical_bar = static_cast<int>(z_nice * y_scale_factor / volts_per_pixel); // compute how much pixels it makes

	if (options_view_data->bVoltageScaleBar)
	{
		i_vertical_bar = MulDiv(i_vertical_bar, rect->Height(), pDataChartWnd->get_rect_height());
		p_dc->MoveTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y);
		p_dc->LineTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y - i_vertical_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_view_data->bChansComment || options_view_data->bVoltageScaleBar || options_view_data->bChanSettings)
	{
		const auto channel_list_size = pDataChartWnd->get_channel_list_size();
		for (auto channel_index = 0; channel_index < channel_list_size; channel_index++) // loop
		{
			CChanlistItem* channel_item = pDataChartWnd->get_channel_list_item(channel_index);
			if (!channel_item->GetflagPrintVisible())
				continue;

			cs.Format(_T("chan#%i "), channel_index); 
			str_comment += cs;
			if (options_view_data->bVoltageScaleBar) 
			{
				const auto z = static_cast<float>(i_vertical_bar) * pDataChartWnd->get_channel_list_volts_per_pixel(channel_index);
				const auto x = z / y_scale_factor;
				const auto j = CNiceUnit::nice_unit(x);
				cs.Format(_T("vert bar = %i %s "), j, (LPCTSTR)cs_unit); 
				str_comment += cs;
			}
			// print chan comment
			if (options_view_data->bChansComment)
			{
				str_comment += tab;
				str_comment += channel_item->GetComment();
			}
			str_comment += rc;
			// print amplifiers settings (gain & filter), next line
			if (options_view_data->bChanSettings)
			{
				const auto source_channel = channel_item->GetSourceChan();
				const auto wave_chan_array = GetDocument()->m_p_dat->get_wavechan_array();
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

CString ViewSpikeDetection::PrintSpkShapeBars(CDC* p_dc, const CRect* rect, BOOL bAll)
{
	const CString rc("\n");
	CString str_comment;
	float z;
	int k;

	///// vertical voltage bars ///////////////////////////
	if (options_view_data->bVoltageScaleBar && m_pSpkList->get_spikes_count() > 0)
	{
		// the following assume that spikes are higher than 1 mV...
		const CString cs_unit = _T("mV");
		z = m_chart_spike_shape.get_extent_m_v() / 2.0f; 
		k = static_cast<int>(z); 
		if ((static_cast<double>(z) - k) > 0.5)
			k++;
		if (bAll)
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
		const auto vertical_bar = static_cast<int>(rect->Height() * z) / 2;

		// compute coordinates of the rect
		CRect rect_vertical_bar; 
		const auto bar_width = CSize(5, 5);
		rect_vertical_bar.left = rect->left - options_view_data->textseparator;
		rect_vertical_bar.right = rect_vertical_bar.left + bar_width.cx;
		rect_vertical_bar.top = rect->top + (rect->Height() - vertical_bar) / 2;
		rect_vertical_bar.bottom = rect_vertical_bar.top + vertical_bar;
		p_dc->Rectangle(&rect_vertical_bar);
		p_dc->SelectObject(p_old_brush);
	}

	// spike duration
	if (m_pSpkList->get_spikes_count() > 0 && bAll)
	{
		z = m_chart_spike_shape.get_extent_ms();
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
	k = m_pSpkList->get_spikes_count();
	CString dummy5;
	dummy5.Format( _T("n spk= %i"), k);
	str_comment += dummy5;
	str_comment += rc;

	return str_comment;
}

void ViewSpikeDetection::serialize_windows_state(const BOOL save, int tab_index)
{
	const auto p_dbWave_app = static_cast<CdbWaveApp*>(AfxGetApp()); 
	if (tab_index < 0 || tab_index >= m_tabCtrl.GetItemCount())
	{
		int tab_selected = m_tabCtrl.GetCurSel(); 
		if (tab_selected < 0)
			tab_selected = 0;
		tab_index = tab_selected;
	}

	// adjust size of the array
	if (p_dbWave_app->view_spikes_memory_file_ptr_array.GetSize() == 0)
		p_dbWave_app->view_spikes_memory_file_ptr_array.SetSize(1);

	if (p_dbWave_app->view_spikes_memory_file_ptr_array.GetSize() < m_tabCtrl.GetItemCount())
		p_dbWave_app->view_spikes_memory_file_ptr_array.SetSize(m_tabCtrl.GetItemCount());
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
		m_chart_data_source.Serialize(ar);
		m_chart_data_filtered.Serialize(ar);
		m_chart_spike_bar.Serialize(ar);
		m_chart_spike_shape.Serialize(ar);
		ar.Close();
	}

	// restore display parameters
	else
	{
		if (p_mem_file != nullptr)
		{
			CArchive ar(p_mem_file, CArchive::load);
			p_mem_file->SeekToBegin();
			m_chart_data_source.Serialize(ar);
			m_chart_data_filtered.Serialize(ar);
			m_chart_spike_bar.Serialize(ar);
			m_chart_spike_shape.Serialize(ar);
			ar.Close(); 
		}
		else
		{
			*m_chart_data_source.get_scope_parameters() = options_view_data->viewspkdetectdata;
			*m_chart_data_filtered.get_scope_parameters() = options_view_data->viewspkdetectfiltered;
			*m_chart_spike_bar.get_scope_parameters() = options_view_data->viewspkdetectspk;
			*m_chart_spike_shape.get_scope_parameters() = options_view_data->viewspkdetectbars;
		}
	}
}

BOOL ViewSpikeDetection::OnPreparePrinting(CPrintInfo* pInfo)
{
	// save current state of the windows
	serialize_windows_state(b_save);

	// printing margins
	if (options_view_data->vertRes <= 0 || options_view_data->horzRes <= 0
		|| options_view_data->horzRes != pInfo->m_rectDraw.Width()
		|| options_view_data->vertRes != pInfo->m_rectDraw.Height())
		PrintComputePageSize();

	// nb print pages?
	int n_pages = PrintGetNPages();
	pInfo->SetMaxPage(n_pages); 
	pInfo->m_nNumPreviewPages = 1; 
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; 

	if (!DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	if (options_view_data->bPrintSelection != pInfo->m_pPD->PrintSelection())
	{
		options_view_data->bPrintSelection = pInfo->m_pPD->PrintSelection();
		n_pages = PrintGetNPages();
		pInfo->SetMaxPage(n_pages);
	}
	return TRUE;
}

int ViewSpikeDetection::PrintGetNPages()
{
	// how many rows per page?
	const auto size_row = options_view_data->HeightDoc + options_view_data->heightSeparator;
	m_nb_rows_per_page_ = m_print_rect_.Height() / size_row;
	if (m_nb_rows_per_page_ == 0)
		m_nb_rows_per_page_ = 1;

	int n_total_rows;
	const auto p_document = GetDocument();

	// compute number of rows according to b_multi_row & b_entire_record flag
	m_l_print_first_ = m_chart_data_filtered.GetDataFirstIndex();
	m_l_print_len_ = m_chart_data_filtered.GetDataLastIndex() - m_l_print_first_ + 1;
	m_file_0_ = GetDocument()->db_get_current_record_position();
	ASSERT(m_file_0_ >= 0);
	m_files_count_ = 1;
	auto i_file_0 = m_file_0_;
	auto i_file_1 = m_file_0_;
	if (!options_view_data->bPrintSelection)
	{
		i_file_0 = 0;
		m_files_count_ = p_document->db_get_n_records();
		i_file_1 = m_files_count_;
	}

	// only one row per file
	if (!options_view_data->bMultirowDisplay || !options_view_data->bEntireRecord)
		n_total_rows = m_files_count_;

	// multiple rows per file
	else
	{
		n_total_rows = 0;
		p_document->db_set_current_record_position(i_file_0);
		for (auto i = i_file_0; i < i_file_1; i++, p_document->db_move_next())
		{
			// get size of document for all files
			auto len = p_document->db_get_data_len();
			if (len <= 0)
			{
				p_document->open_current_data_file();
				len = p_document->m_p_dat->get_doc_channel_length();
				const auto len1 = GetDocument()->db_get_data_len() - 1;
				ASSERT(len == len1);
				p_document->db_set_data_len(len);
			}
			len -= m_l_print_first_;
			auto n_rows = len / m_l_print_len_; 
			if (len > n_rows * m_l_print_len_)
				n_rows++;
			n_total_rows += static_cast<int>(n_rows);
		}
	}

	if (m_file_0_ >= 0)
	{
		p_document->db_set_current_record_position(m_file_0_);
		p_document->open_current_data_file();
	}

	auto n_pages = n_total_rows / m_nb_rows_per_page_;
	if (n_total_rows > m_nb_rows_per_page_ * n_pages)
		n_pages++;

	return n_pages;
}

void ViewSpikeDetection::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_b_is_printing_ = TRUE;
	m_l_first_0_ = m_chart_data_filtered.GetDataFirstIndex();
	m_l_last0_ = m_chart_data_filtered.GetDataLastIndex();
	m_pixels_count_0_ = m_chart_data_filtered.get_rect_width();
	PrintCreateFont();
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewSpikeDetection::PrintCreateFont()
{
	//---------------------init objects-------------------------------------
	memset(&m_log_font_, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_log_font_.lfFaceName, _T("Arial")); // Arial font
	m_log_font_.lfHeight = options_view_data->fontsize; // font height
	m_p_old_font_ = nullptr;
	m_font_print_.CreateFontIndirect(&m_log_font_);
}

void ViewSpikeDetection::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_p_old_font_ = p_dc->SelectObject(&m_font_print_);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	const auto r_width = options_view_data->WidthDoc; // page margins
	const auto r_height = options_view_data->HeightDoc; // page margins
	CRect r_where(m_print_rect_.left, // printing rectangle for one line of data
		m_print_rect_.top,
		m_print_rect_.left + r_width,
		m_print_rect_.top + r_height);
	auto rw2 = r_where; // printing rectangle - constant
	rw2.OffsetRect(-r_where.left, -r_where.top); // set RW2 origin = 0,0

	// define spike shape area (rect): same height as data area
	auto r_sp_kheight = options_view_data->spkheight;
	auto r_sp_kwidth = options_view_data->spkwidth;
	if (r_sp_kheight == 0)
	{
		r_sp_kheight = r_height - options_view_data->fontsize * 4;
		r_sp_kwidth = r_sp_kheight / 2;
		if (r_sp_kwidth < MulDiv(r_where.Width(), 10, 100))
			r_sp_kwidth = MulDiv(r_where.Width(), 10, 100);
		options_view_data->spkheight = r_sp_kheight;
		options_view_data->spkwidth = r_sp_kwidth;
	}

	// save current draw mode (it will be modified to print only one channel)
	const WORD chan0Drawmode = 1;
	//WORD chan1Drawmode = 0;
	if (!options_view_data->bFilterDataSource)
		m_chart_data_filtered.set_channel_list_transform_mode(0, 0);

	p_dc->SetMapMode(MM_TEXT); // change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(p_dc, pInfo); // print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int file_index; 
	long index_first_data_point; 
	auto index_last_data_point = m_l_print_first_ + m_l_print_len_; 
	const auto current_page_number = pInfo->m_nCurPage;
	PrintGetFileSeriesIndexFromPage(current_page_number, file_index, index_first_data_point);
	if (index_first_data_point < GetDocument()->db_get_data_len() - 1)
		update_file_parameters(FALSE);
	if (options_view_data->bEntireRecord)
		index_last_data_point = GetDocument()->db_get_data_len() - 1;

	// loop through all files	--------------------------------------------------------
	for (auto i = 0; i < m_nb_rows_per_page_; i++)
	{
		const auto old_dc = p_dc->SaveDC(); // save DC

		// first : set rectangle where data will be printed
		auto comment_rect = r_where; 
		p_dc->SetMapMode(MM_TEXT); 
		p_dc->SetTextAlign(TA_LEFT); 

		// load data and adjust display rectangle ----------------------------------------
		// set data rectangle to half height to the row height
		rect_data_ = r_where;
		if (options_view_data->bPrintSpkBars)
			rect_data_.bottom = rect_data_.top + r_where.Height() / 2;
		rect_data_.left += (r_sp_kwidth + options_view_data->textseparator);
		const auto old_size = rect_data_.Width(); 

		// make sure enough data fit into this rectangle, otherwise clip rect
		auto l_last = index_first_data_point + m_l_print_len_; 
		if (l_last > index_last_data_point) 
			l_last = index_last_data_point;
		if ((l_last - index_first_data_point + 1) < m_l_print_len_) 
			rect_data_.right = (old_size * (l_last - index_first_data_point)) / m_l_print_len_ + rect_data_.left;
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// if option requested, clip output to rect
		if (options_view_data->bClipRect) // clip curve display
			p_dc->IntersectClipRect(&rect_data_); // (eventually)

		// print detected channel only data
		m_chart_data_filtered.get_channel_list_item(0)->SetflagPrintVisible(chan0Drawmode);
		m_chart_data_filtered.resize_channels(rect_data_.Width(), 0);
		m_chart_data_filtered.get_data_from_doc(index_first_data_point, l_last);
		m_chart_data_filtered.Print(p_dc, &rect_data_);
		p_dc->SelectClipRgn(nullptr);

		// print spike bars 
		if (options_view_data->bPrintSpkBars)
		{
			CRect BarsRect = r_where; 
			BarsRect.top = rect_data_.bottom;
			BarsRect.left = rect_data_.left;
			BarsRect.right = rect_data_.right;

			m_chart_spike_bar.set_time_intervals(index_first_data_point, l_last);
			m_chart_spike_bar.print(p_dc, &BarsRect);
		}

		// print spike shape within a square (same width as height) 
		rect_spike_ = r_where; 
		rect_spike_.right = rect_spike_.left + r_sp_kwidth;
		rect_spike_.left += options_view_data->textseparator;
		rect_spike_.bottom = rect_spike_.top + r_sp_kheight; 

		m_chart_spike_shape.set_time_intervals(index_first_data_point, l_last);
		m_chart_spike_shape.print(p_dc, &rect_spike_);

		// restore DC and print comments 
		p_dc->RestoreDC(old_dc); 
		p_dc->SetMapMode(MM_TEXT);
		p_dc->SelectClipRgn(nullptr); 
		p_dc->SetViewportOrg(0, 0);

		// print data Bars & get comments according to row within file
		CString cs_comment;
		const BOOL b_all = (index_first_data_point == m_l_print_first_);
		if (b_all) 
		{
			cs_comment += PrintGetFileInfos();
			cs_comment += PrintDataBars(p_dc, &m_chart_data_filtered, &rect_data_);
		}
		else
		{
			// other rows: time intervals only
			cs_comment = PrintConvertFileIndex(m_chart_data_filtered.GetDataFirstIndex(),
				m_chart_data_filtered.GetDataLastIndex());
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_view_data->textseparator + comment_rect.Width(), 0);
		comment_rect.right = m_print_rect_.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		constexpr UINT format_parameters = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect, format_parameters);

		// print comments & bar / spike shape
		cs_comment.Empty();
		rect_spike_.right = rect_spike_.left + r_sp_kheight;
		cs_comment = PrintSpkShapeBars(p_dc, &rect_spike_, b_all);
		rect_spike_.right = rect_spike_.left + r_sp_kwidth;
		rect_spike_.left -= options_view_data->textseparator;
		rect_spike_.top = rect_spike_.bottom;
		rect_spike_.bottom += m_log_font_.lfHeight * 3;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), rect_spike_, format_parameters);
		p_dc->SetTextAlign(ui_flag);
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// update file parameters for next row --------------------------------------------

		r_where.OffsetRect(0, r_height + options_view_data->heightSeparator);
		const auto i_file = file_index;
		if (!PrintGetNextRow(file_index, index_first_data_point, index_last_data_point))
		{
			//i = m_n_rows_per_page;
			break;
		}
		if (i_file != file_index)
		{
			update_file_parameters(FALSE);
			m_chart_data_filtered.get_channel_list_item(0)->SetflagPrintVisible(0); // cancel printing channel zero
		}
	}

	// end of file loop : restore initial conditions
	m_chart_data_filtered.get_channel_list_item(0)->SetflagPrintVisible(1);
	if (!options_view_data->bFilterDataSource)
		m_chart_data_filtered.set_channel_list_transform_mode(0, m_p_detect_parameters->detect_transform);

	if (m_p_old_font_ != nullptr)
		p_dc->SelectObject(m_p_old_font_);
}

void ViewSpikeDetection::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_font_print_.DeleteObject();
	// restore file from index and display parameters
	GetDocument()->db_set_current_record_position(m_file_0_);

	m_chart_data_filtered.resize_channels(m_pixels_count_0_, 0);
	m_chart_data_filtered.get_data_from_doc(m_l_first_0_, m_l_last0_);
	m_chart_spike_shape.set_time_intervals(m_l_first_0_, m_l_last0_);
	update_file_parameters(TRUE);

	m_b_is_printing_ = FALSE;
	serialize_windows_state(b_restore);
}

void ViewSpikeDetection::OnBnClickedBiasbutton()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN))->SetState(0);
	SetVBarMode(BAR_BIAS, IDC_SCROLLY);
}

void ViewSpikeDetection::OnBnClickedBias2()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS2))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN2))->SetState(0);
	SetVBarMode(BAR_BIAS, IDC_SCROLLY2);
}

void ViewSpikeDetection::OnBnClickedGainbutton()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN))->SetState(1);
	SetVBarMode(BAR_GAIN, IDC_SCROLLY);
}

void ViewSpikeDetection::OnBnClickedGain2()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS2))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN2))->SetState(1);
	SetVBarMode(BAR_GAIN, IDC_SCROLLY2);
}

void ViewSpikeDetection::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != nullptr)
	{
		const auto i_id = pScrollBar->GetDlgCtrlID();
		if ((i_id == IDC_SCROLLY) || (i_id == IDC_SCROLLY2))
		{
			// CViewData scroll: vertical scroll bar
			switch (m_VBarMode)
			{
			case BAR_GAIN:
				OnGainScroll(nSBCode, nPos, i_id);
				break;
			case BAR_BIAS:
				OnBiasScroll(nSBCode, nPos, i_id);
			default:
				break;
			}
		}
	}
	else
		dbTableView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void ViewSpikeDetection::SetVBarMode(short bMode, int iID)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll(iID);
}

void ViewSpikeDetection::UpdateGainScroll(int iID)
{
	if (iID == IDC_SCROLLY)
		m_scrolly.SetScrollPos(
			MulDiv(m_chart_data_filtered.get_channel_list_item(m_ichanselected)->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
	else
		m_scrolly2.SetScrollPos(
			MulDiv(m_chart_data_filtered.get_channel_list_item(m_ichanselected2)->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
}

void ViewSpikeDetection::OnGainScroll(UINT nSBCode, UINT nPos, int iID)
{
	const ChartData* p_view_data_filtered = &m_chart_data_filtered;
	int selected_channel = m_ichanselected;
	if (iID == IDC_SCROLLY2)
	{
		p_view_data_filtered = &m_chart_data_source;
		selected_channel = m_ichanselected2;
	}
	int y_extent = p_view_data_filtered->get_channel_list_item(selected_channel)->GetYextent();

	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: y_extent = YEXTENT_MIN;
		break; // .................scroll to the start
	case SB_LINELEFT: y_extent -= y_extent / 10 + 1;
		break; // .................scroll one line left
	case SB_LINERIGHT: y_extent += y_extent / 10 + 1;
		break; // .................scroll one line right
	case SB_PAGELEFT: y_extent -= y_extent / 2 + 1;
		break; // .................scroll one page left
	case SB_PAGERIGHT: y_extent += y_extent + 1;
		break; // .................scroll one page right
	case SB_RIGHT: y_extent = YEXTENT_MAX;
		break; // .................scroll to end right
	case SB_THUMBPOSITION: // .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBTRACK: y_extent = MulDiv(nPos - 50, YEXTENT_MAX, 100);
		break;
	default: break; // .................NOP: set position only
	}
	// change y extent
	if (y_extent > 0) 
	{
		p_view_data_filtered->get_channel_list_item(selected_channel)->SetYextent(y_extent);
		update_legends();
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		UpdateGainScroll(iID);
}

void ViewSpikeDetection::UpdateBiasScroll(int iID)
{
	if (iID == IDC_SCROLLY)
	{
		const CChanlistItem* channel_item = m_chart_data_filtered.get_channel_list_item(m_ichanselected);
		const auto i_pos = static_cast<int>((channel_item->GetYzero() - channel_item->GetDataBinZero())
			* 100 / static_cast<int>(YZERO_SPAN)) + static_cast<int>(50);
		m_scrolly.SetScrollPos(i_pos, TRUE);
	}
	else
	{
		const CChanlistItem* channel_item = m_chart_data_filtered.get_channel_list_item(m_ichanselected2);
		const auto i_pos = static_cast<int>((channel_item->GetYzero() - channel_item->GetDataBinZero())
			* 100 / int(YZERO_SPAN)) + static_cast<int>(50);
		m_scrolly2.SetScrollPos(i_pos, TRUE);
	}
}

void ViewSpikeDetection::OnBiasScroll(UINT nSBCode, UINT nPos, int iID)
{
	auto p_view = &m_chart_data_filtered;
	auto selected_channel_index = m_ichanselected;
	if (iID == IDC_SCROLLY2)
	{
		p_view = &m_chart_data_source;
		selected_channel_index = m_ichanselected2;
	}

	const CChanlistItem* channel_item = m_chart_data_filtered.get_channel_list_item(selected_channel_index);
	auto l_size = channel_item->GetYzero() - channel_item->GetDataBinZero();
	const auto y_extent = channel_item->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: l_size = YZERO_MIN;
		break; // scroll to the start
	case SB_LINELEFT: l_size -= y_extent / 100 + 1;
		break; // scroll one line left
	case SB_LINERIGHT: l_size += y_extent / 100 + 1;
		break; // scroll one line right
	case SB_PAGELEFT: l_size -= y_extent / 10 + 1;
		break; // scroll one page left
	case SB_PAGERIGHT: l_size += y_extent / 10 + 1;
		break; // scroll one page right
	case SB_RIGHT: l_size = YZERO_MAX;
		break; // scroll to end right
	case SB_THUMBPOSITION: // scroll to pos = nPos	// drag scroll box -- pos = nPos
	case SB_THUMBTRACK: l_size = (nPos - 50) * (YZERO_SPAN / 100);
		break;
	default: break; // NOP: set position only
	}

	// try to read data with this new size
	if (l_size > YZERO_MIN && l_size < YZERO_MAX)
	{
		CChanlistItem* chan = p_view->get_channel_list_item(selected_channel_index);
		chan->SetYzero(l_size + chan->GetDataBinZero());
		p_view->Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		UpdateBiasScroll(iID);
}

void ViewSpikeDetection::OnEnChangeSpkWndAmplitude()
{
	if (mm_spkWndAmplitude.m_bEntryDone)
	{
		m_spkWndAmplitude = m_chart_spike_shape.get_extent_m_v();
		const auto y_old = m_spkWndAmplitude;
		CString cs;
		mm_spkWndAmplitude.OnEnChange(this, m_spkWndAmplitude, 1.f, -1.f);

		// compute new extent and change the display
		if (m_spkWndAmplitude <= 0)
		{
			m_spkWndAmplitude = y_old;
			MessageBeep(-1);
		}
		const auto y_we = static_cast<int>(static_cast<float>(m_chart_spike_shape.get_yw_extent()) * m_spkWndAmplitude / y_old);
		m_chart_spike_shape.set_yw_ext_org(y_we, m_chart_spike_shape.get_yw_org());
		m_chart_spike_shape.set_y_scale_unit_value(m_spkWndAmplitude);
		m_chart_spike_shape.Invalidate();

		// update the dialog control
		
		cs.Format(_T("%.3f"), m_spkWndAmplitude);
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->SetWindowText(cs);
	}
}

void ViewSpikeDetection::OnEnChangeSpkWndLength()
{
	if (mm_spkWndDuration.m_bEntryDone)
	{
		m_spkWndDuration = m_chart_spike_shape.get_extent_ms();
		const auto xold = m_spkWndDuration;
		CString cs;
		mm_spkWndDuration.OnEnChange(this, m_spkWndDuration, 1.f, -1.f);

		// compute new extent and change the display
		if (m_spkWndDuration <= 0)
		{
			MessageBeep(-1);
			m_spkWndDuration = xold;
		}
		const auto x_we = static_cast<int>(static_cast<float>(m_chart_spike_shape.get_xw_extent()) * m_spkWndDuration / xold);
		m_chart_spike_shape.set_xw_ext_org(x_we, m_chart_spike_shape.get_xw_org());
		m_chart_spike_shape.set_x_scale_unit_value(m_spkWndDuration);
		m_chart_spike_shape.Invalidate();

		// update the dialog control
		cs.Format(_T("%.3f"), m_spkWndDuration);
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->SetWindowText(cs);
	}
}

void ViewSpikeDetection::OnBnClickedLocatebttn()
{
	int max, min;
	const CChanlistItem* channel_item = m_chart_data_filtered.get_channel_list_item(0);
	channel_item->GetMaxMin(&max, &min);

	// modify value
	m_p_detect_parameters->detect_threshold_bin = (max + min) / 2;
	m_thresholdval = channel_item->ConvertDataBinsToVolts(m_p_detect_parameters->detect_threshold_bin) * 1000.f;
	m_p_detect_parameters->detect_threshold_mv = m_thresholdval;
	// update user-interface: edit control and threshold bar in sourceview
	CString cs;
	cs.Format(_T("%.3f"), m_thresholdval);
	GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
	m_chart_data_filtered.move_hz_tag_to_val(0, m_p_detect_parameters->detect_threshold_bin);
	m_chart_data_filtered.Invalidate();
}

void ViewSpikeDetection::update_detection_settings(int iSelParms)
{
	// check size of spike detection parameters
	if (iSelParms >= m_spk_detect_array_current.GetSize())
	{
		// load new set of parameters from spike list
		const auto spike_list_size = m_pSpkDoc->get_spike_list_size();
		m_spk_detect_array_current.SetSize(spike_list_size);
		for (int i = 0; i < spike_list_size; i++)
		{
			// select new spike list (list with no spikes for stimulus channel)
			const auto spike_list_current = m_pSpkDoc->set_spike_list_current_index(i);
			ASSERT(spike_list_current != NULL);
			const auto ps_d = spike_list_current->get_detection_parameters();
			m_spk_detect_array_current.SetItem(i, ps_d); 
		}
	}

	// loop over each spike detection set to update spike detection parameters
	for (auto i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		// select new spike list (list with no spikes for stimulus channel)
		auto spike_list_current = m_pSpkDoc->set_spike_list_current_index(i);
		const auto p_sd = m_spk_detect_array_current.GetItem(i);
		if (spike_list_current == nullptr)
		{
			m_pSpkDoc->add_spk_list();
			spike_list_current = m_pSpkDoc->get_spike_list_current();
			spike_list_current->init_spike_list(GetDocument()->m_p_dat, p_sd);
		}
		else
			spike_list_current->set_detection_parameters(p_sd);
	}

	// set new parameters
	m_pSpkList->m_selected_spike = m_spike_index; 
	m_i_detect_parameters = iSelParms;
	m_p_detect_parameters = m_spk_detect_array_current.GetItem(iSelParms);
	m_pSpkList = m_pSpkDoc->set_spike_list_current_index(iSelParms);
	if (m_pSpkList != nullptr)
		highlight_spikes(TRUE);

	update_detection_controls();
	update_legends();
}

void ViewSpikeDetection::update_detection_controls()
{
	const SPKDETECTPARM* detect_parameters = m_pSpkList->get_detection_parameters();
	m_CBdetectWhat.SetCurSel(detect_parameters->detect_what);
	m_CBdetectChan.SetCurSel(detect_parameters->detect_channel);
	m_CBtransform.SetCurSel(detect_parameters->detect_transform);

	// check that spike detection parameters are compatible with current data doc
	auto detection_channel = detect_parameters->detect_channel;
	const auto p_dat = GetDocument()->m_p_dat;
	const int scan_count = p_dat->get_waveformat()->scan_count;
	if (detection_channel >= scan_count)
		detection_channel = 0;

	m_chart_data_filtered.set_channel_list_y(0, detection_channel, detect_parameters->detect_transform);
	m_CBtransform2.SetCurSel(m_pSpkList->get_detection_parameters()->extract_transform);

	CChanlistItem* channel_item = m_chart_data_filtered.get_channel_list_item(0);
	channel_item->SetColor(static_cast<WORD>(detection_channel));

	m_chart_data_filtered.get_data_from_doc(); 

	const auto detect_threshold = detect_parameters->detect_threshold_bin;
	m_thresholdval = channel_item->ConvertDataBinsToVolts(detect_threshold) * 1000.f;
	if (m_chart_data_filtered.horizontal_tags.get_tag_list_size() < 1)
		m_chart_data_filtered.horizontal_tags.add_tag(detect_threshold, 0);
	else
		m_chart_data_filtered.horizontal_tags.set_tag_val(0, detect_threshold);

	// update spike channel displayed
	m_chart_spike_bar.set_spike_list(m_pSpkList);
	m_chart_spike_shape.set_spike_list(m_pSpkList);
}

void ViewSpikeDetection::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	serialize_windows_state(b_save, m_i_detect_parameters);
	const auto selected_tab = m_tabCtrl.GetCurSel();
	serialize_windows_state(b_restore, selected_tab);
	update_detection_settings(selected_tab);
	*pResult = 0;
}

void ViewSpikeDetection::OnToolsEditstimulus()
{
	m_pSpkDoc->sort_stimulus_array();

	DlgEditStimArray dlg;
	dlg.intervals = m_pSpkDoc->m_stimulus_intervals;
	dlg.m_sampling_rate = m_samplingRate;
	dlg.intervals_saved = GetDocument()->stimulus_saved;
	;
	if (IDOK == dlg.DoModal())
	{
		m_pSpkDoc->m_stimulus_intervals = dlg.intervals;
		update_VT_tags();
		m_chart_spike_bar.Invalidate();
		m_chart_data_filtered.Invalidate();
		m_chart_data_source.Invalidate();
		m_pSpkDoc->SetModifiedFlag(TRUE);
	}
}
void ViewSpikeDetection::OnEnChangeChanselected()
{
	if (mm_ichanselected.m_bEntryDone)
	{
		mm_ichanselected.OnEnChange(this, m_ichanselected, 1, -1);
		SetDlgItemInt(IDC_CHANSELECTED, m_ichanselected);
	}
}

void ViewSpikeDetection::OnEnChangeChanselected2()
{
	if (mm_ichanselected2.m_bEntryDone)
	{
		mm_ichanselected2.OnEnChange(this, m_ichanselected2, 1, -1);
		SetDlgItemInt(IDC_CHANSELECTED2, m_ichanselected2);
	}
}

void ViewSpikeDetection::OnCbnSelchangeTransform2()
{
	const auto method = m_CBtransform2.GetCurSel(); 
	const auto data_document = GetDocument()->m_p_dat;

	const auto detect_parameters = m_pSpkList->get_detection_parameters();
	const auto doc_chan = detect_parameters->extract_channel;
	const short span = data_document->get_transformed_data_span(method); 
	detect_parameters->extract_transform = method; 

	// pre-load data
	const auto spike_length = m_pSpkList->get_spike_length();
	const auto spike_pre_threshold = detect_parameters->detect_pre_threshold;
	auto ii_time = m_pSpkList->get_spike(0)->get_time() - spike_pre_threshold;
	auto l_rw_first0 = ii_time - spike_length;
	auto l_rw_last0 = ii_time + spike_length;
	if (!data_document->load_raw_data(&l_rw_first0, &l_rw_last0, span))
		return; 
	auto p_data = data_document->load_transformed_data(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto total_spikes = m_pSpkList->get_spikes_count();
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		constexpr short offset = 1;
		Spike* p_spike = m_pSpkList->get_spike(i_spike);
		// make sure that source data are loaded and get pointer to it (p_data)
		ii_time = p_spike->get_time();
		auto l_rw_first = ii_time - spike_pre_threshold; 
		auto l_rw_last = l_rw_first + spike_length; 
		if (!data_document->load_raw_data(&l_rw_first, &l_rw_last, span))
			break; 

		p_data = data_document->load_transformed_data(l_rw_first, l_rw_last, method, doc_chan);
		const auto p_data_spike0 = p_data + (ii_time - spike_pre_threshold - l_rw_first) * offset;
		p_spike->transfer_data_to_spike_buffer(p_data_spike0, offset, m_pSpkList->get_spike_length());

		// n channels should be 1 if they come from the transform buffer as data are not interleaved...
		p_spike->center_spike_amplitude(0, spike_length, 1); // 1=center average
	}
	m_pSpkDoc->SetModifiedFlag(TRUE);

	short max, min;
	m_pSpkList->get_total_max_min(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	m_chart_spike_shape.set_yw_ext_org(m_chart_spike_shape.get_yw_extent(), middle);
	m_chart_spike_bar.set_yw_ext_org(m_chart_spike_shape.get_yw_extent(), middle);
	update_spike_shape_window_scale(FALSE);

	highlight_spikes(TRUE);
	update_legends();
	update_tabs();
}

void ViewSpikeDetection::update_tabs()
{
	// load initial data
	const BOOL b_replace = (m_tabCtrl.GetItemCount() == m_pSpkDoc->get_spike_list_size());
	if (!b_replace)
		m_tabCtrl.DeleteAllItems();

	// load list of detection parameters
	const auto current_spike_list_index = m_pSpkDoc->get_spike_list_current_index();
	for (auto i = 0; i < m_pSpkDoc->get_spike_list_size(); i++)
	{
		CString cs;
		const auto current_spike_list = m_pSpkDoc->set_spike_list_current_index(i);
		cs.Format(_T("#%i %s"), i, (LPCTSTR)current_spike_list->get_detection_parameters()->comment);
		if (!b_replace)
			m_tabCtrl.InsertItem(i, cs);
		else
		{
			TCITEM tab_ctrl_item;
			tab_ctrl_item.mask = TCIF_TEXT;
			const auto p_cs_buffer = cs.GetBuffer(0);
			tab_ctrl_item.pszText = p_cs_buffer;
			m_tabCtrl.SetItem(i, &tab_ctrl_item);
			cs.ReleaseBuffer();
		}
	}
	m_pSpkDoc->set_spike_list_current_index(current_spike_list_index);

	m_i_detect_parameters = m_pSpkDoc->get_spike_list_current_index();
	m_tabCtrl.SetCurSel(m_i_detect_parameters);
}
