// TODO
// convert threshold into volts & back to binary (cope with variable gains)
// cf: UpdateFileParameters

#include "StdAfx.h"
#include "dbWave.h"
#include "ViewSpikeDetect.h"

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
		saveCurrentSpkFile();
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
	DDX_Text(pDX, IDC_SPIKENO, m_spikeno);
	DDX_Check(pDX, IDC_ARTEFACT, m_bartefact);
	DDX_Text(pDX, IDC_THRESHOLDVAL, m_thresholdval);
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
		_T("spk"), // default filename extension
		GetDocument()->GetDB_CurrentSpkFileName(), // initial file name
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Awave Spikes (*.spk) | *.spk |All Files (*.*) | *.* ||"));

	if (IDOK == dlg.DoModal())
	{
		m_pSpkDoc->OnSaveDocument(dlg.GetPathName());
		GetDocument()->SetDB_n_spikes(m_pSpkDoc->GetSpkList_Current()->GetTotalSpikes());
		GetDocument()->SetDB_n_spike_classes(1);
		m_pSpkDoc->SetModifiedFlag(FALSE);
	}
}

BOOL ViewSpikeDetection::OnMove(UINT nIDMoveCommand)
{
	saveCurrentSpkFile();
	return dbTableView::OnMove(nIDMoveCommand);
}

void ViewSpikeDetection::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_binit)
	{
		switch (LOWORD(lHint))
		{
		case HINT_CLOSEFILEMODIFIED: // close modified file: save
			saveCurrentSpkFile();
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
		p_main_frame->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, LPARAM(activated_view->GetDocument()));
	}
	else
	{
		serialize_windows_state(b_save);
		const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
		p_app->options_viewdata.viewdata = *(m_chart_data_source.GetScopeParameters());
	}
	dbTableView::OnActivateView(activate, activated_view, de_activated_view);
}

void ViewSpikeDetection::update_legends()
{
	const auto l_first = m_chart_data_source.GetDataFirst();
	const auto l_last = m_chart_data_source.GetDataLast();
	m_chart_data_filtered.GetDataFromDoc(l_first, l_last);

	// draw charts
	m_chart_spike_bar.SetTimeIntervals(l_first, l_last);
	m_chart_spike_shape.SetTimeIntervals(l_first, l_last);
	update_spike_shape_window_scale(FALSE);

	// update text abscissa and horizontal scroll position
	m_timefirst = static_cast<float>(l_first) / m_samplingRate;
	m_timelast = static_cast<float>(l_last + 1) / m_samplingRate;
	m_spikeno = m_pSpkList->m_selected_spike;

	if (m_spikeno > m_pSpkList->GetTotalSpikes())
	{
		m_pSpkList->m_selected_spike = -1;
		m_spikeno = m_pSpkList->m_selected_spike;
	}

	m_bartefact = FALSE;
	if (m_spikeno > 0)
	{
		const auto p_s = m_pSpkList->GetSpike(m_spikeno);
		m_bartefact = (p_s->get_class() < 0);
	}

	update_file_scroll();
	update_combo_box();
	update_VT_tags();
	update_legend_detection_wnd();
	update_number_of_spikes();

	// update data displayed
	m_chart_spike_bar.Invalidate();
	m_chart_data_filtered.Invalidate();
	m_chart_data_source.Invalidate();
	m_chart_spike_shape.Invalidate();
	UpdateData(FALSE);
}

void ViewSpikeDetection::update_spike_file(BOOL bUpdateInterface)
{
	const auto pdb_doc = GetDocument();
	if (pdb_doc->OpenCurrentSpikeFile() == nullptr)
	{
		// file not found: create new object, and create file
		auto* p_spike = new CSpikeDoc;
		ASSERT(p_spike != NULL);
		pdb_doc->m_pSpk = p_spike;
		m_pSpkDoc = p_spike;
		m_pSpkDoc->OnNewDocument();
		m_pSpkDoc->ClearData();
		if (options_view_data->bDetectWhileBrowse)
			OnMeasureAll();
	}
	else
	{
		m_pSpkDoc = pdb_doc->m_pSpk;
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(pdb_doc->GetDB_CurrentSpkFileName(), FALSE);
	}

	// select a spike list
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	if (m_pSpkList == nullptr && m_pSpkDoc->GetSpkList_Size() > 0)
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(0);

	// no spikes list available, create one
	if (m_pSpkList == nullptr)
	{
		// create new list here
		ASSERT(m_pSpkDoc->GetSpkList_Size() == 0);
		const auto i_size = m_spk_detect_array_current.GetSize();
		m_pSpkDoc->SetSpkList_Size(i_size);
		for (auto i = 0; i < i_size; i++)
		{
			auto spike_list_current = m_pSpkDoc->SetSpkList_AsCurrent(i);
			if (spike_list_current == nullptr)
			{
				m_pSpkDoc->AddSpkList();
				spike_list_current = m_pSpkDoc->GetSpkList_Current();
			}
			spike_list_current->InitSpikeList(pdb_doc->m_pDat, m_spk_detect_array_current.GetItem(i));
		}
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(0);
		ASSERT(m_pSpkList != nullptr);
	}

	m_chart_spike_bar.SetSourceData_spklist_dbwavedoc(m_pSpkList, pdb_doc);
	m_chart_spike_bar.SetPlotMode(PLOT_BLACK, 0);
	m_chart_spike_shape.SetSourceData(m_pSpkList, pdb_doc);
	m_chart_spike_shape.SetPlotMode(PLOT_BLACK, 0);
	update_VT_tags();

	// update interface elements
	m_spikeno = -1;
	if (bUpdateInterface)
	{
		update_tabs();
		update_detection_controls();
		highlight_spikes(TRUE);
		update_number_of_spikes();
		m_chart_spike_bar.Invalidate();
		m_chart_spike_shape.Invalidate();
	}
}

void ViewSpikeDetection::highlight_spikes(BOOL flag)
{
	if (!flag || m_pSpkList == nullptr || m_pSpkList->GetTotalSpikes() < 1) 
		return;
	
	const auto array_size = m_pSpkList->GetTotalSpikes() * 2 + 3;
	m_DWintervals.SetSize(array_size);
	m_DWintervals.SetAt(0, 0);
	m_DWintervals.SetAt(1, static_cast<DWORD>(RGB(255, 0, 0))); 
	m_DWintervals.SetAt(2, 1);
	const auto total_spikes = m_pSpkList->GetTotalSpikes();
	auto j_index = 3;
	auto spike_length = m_pSpkList->GetSpikeLength();
	const auto spike_pre_trigger = m_pSpkList->GetDetectParms()->prethreshold;
	spike_length--;

	for (auto i = 0; i < total_spikes; i++)
	{
		const auto p_s = m_pSpkList->GetSpike(i);
		const auto l_first = p_s->get_time() - spike_pre_trigger;
		m_DWintervals.SetAt(j_index, l_first);
		j_index++;
		m_DWintervals.SetAt(j_index, l_first + spike_length);
		j_index++;
	}

	// tell source_view to highlight spk
	m_chart_data_filtered.SetHighlightData(&m_DWintervals);
	m_chart_data_filtered.Invalidate();
	m_chart_data_source.SetHighlightData(&m_DWintervals);
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
		m_i_detect_parameters = GetDocument()->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();
		m_p_detect_parameters = m_spk_detect_array_current.GetItem(m_i_detect_parameters);
	}

	// get infos from data file
	const auto data_file = GetDocument()->m_pDat;
	data_file->ReadDataInfos();
	const auto wave_format = data_file->GetpWaveFormat();

	// check detection and extraction channels
	if (m_p_detect_parameters->detectChan < 0
		|| m_p_detect_parameters->detectChan >= wave_format->scan_count)
	{
		m_p_detect_parameters->detectChan = 0;
		AfxMessageBox(_T("Spike detection parameters: detection channel modified"));
		flag = FALSE;
	}

	if (m_p_detect_parameters->extractChan < 0
		|| m_p_detect_parameters->extractChan >= wave_format->scan_count)
	{
		m_p_detect_parameters->extractChan = 0;
		AfxMessageBox(_T("Spike detection parameters: channel extracted modified"));
		flag = FALSE;
	}
	return flag;
}

boolean ViewSpikeDetection::update_data_file(BOOL bUpdateInterface)
{
	const auto pdb_doc = GetDocument();
	if (pdb_doc->OpenCurrentDataFile() == nullptr)
		return false;

	const auto p_data_file = pdb_doc->m_pDat;
	p_data_file->ReadDataInfos();
	const auto wave_format = p_data_file->GetpWaveFormat();

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
	if (m_chart_data_filtered.GetChanlistSize() < 1)
	{
		m_chart_data_filtered.RemoveAllChanlistItems();
		m_chart_data_filtered.AddChanlistItem(0, 0);
		CChanlistItem* channel_item = m_chart_data_filtered.GetChanlistItem(0);
		channel_item->SetColor(0);
		m_chart_data_filtered.m_HZtags.RemoveAllTags();
		m_p_detect_parameters->detectThreshold = channel_item->ConvertVoltsToDataBins(m_p_detect_parameters->detectThresholdmV / 1000.f);
		m_chart_data_filtered.m_HZtags.AddTag(m_p_detect_parameters->detectThreshold, 0);
	}

	//add all channels to detection window
	auto channel_list_size = m_chart_data_source.GetChanlistSize();
	const int n_document_channels = wave_format->scan_count;
	for (auto i = 0; i < n_document_channels; i++)
	{
		// check if present in the list
		auto b_present = FALSE;
		for (auto j = channel_list_size - 1; j >= 0; j--)
		{
			// test if this data chan is present + no transformation
			const CChanlistItem* channel_item = m_chart_data_source.GetChanlistItem(j);
			if (channel_item->GetSourceChan() == i
				&& channel_item->GetTransformMode() == 0)
			{
				b_present = TRUE;
				break;
			}
		}
		if (b_present == FALSE) // no display chan contains that doc chan
		{
			m_chart_data_source.AddChanlistItem(i, 0);
			channel_list_size++;
		}
		m_chart_data_source.GetChanlistItem(i)->SetColor(static_cast<WORD>(i));
	}

	// if browse through another file ; keep previous display parameters & load data
	auto l_first = m_chart_data_filtered.GetDataFirst();
	auto l_last = m_chart_data_filtered.GetDataLast();
	if (options_view_data->bEntireRecord && bUpdateInterface)
	{
		l_first = 0;
		l_last = p_data_file->GetDOCchanLength() - 1;
	}
	m_chart_data_filtered.GetDataFromDoc(l_first, l_last);
	m_chart_data_source.GetDataFromDoc(l_first, l_last);

	if (bUpdateInterface)
	{
		m_chart_data_filtered.Invalidate();
		m_chart_data_source.Invalidate();
		// adjust scroll bar (size of button and left/right limits)
		m_filescroll_infos.fMask = SIF_ALL;
		m_filescroll_infos.nMin = 0;
		m_filescroll_infos.nMax = m_chart_data_filtered.GetDataLast();
		m_filescroll_infos.nPos = 0;
		m_filescroll_infos.nPage = m_chart_data_filtered.GetDataLast() - m_chart_data_filtered.GetDataFirst() + 1;
		m_filescroll.SetScrollInfo(&m_filescroll_infos);

		m_datacomments = wave_format->GetComments(_T(" "));
		m_samplingRate = wave_format->sampling_rate_per_channel;
		m_bValidThreshold = FALSE;
	}
	return true;
}

void ViewSpikeDetection::update_combos_detect_and_transforms()
{
	const auto db_document = GetDocument();
	const auto p_data_file = db_document->m_pDat;
	const auto channel_array = p_data_file->GetpWavechanArray();
	const auto wave_format = p_data_file->GetpWaveFormat();

	// load channel names
	CString comment;
	m_CBdetectChan.ResetContent();
	const int channel_count = wave_format->scan_count;
	for (auto i = 0; i < channel_count; i++)
	{
		comment.Format(_T("%i: "), i);
		comment += channel_array->Get_p_channel(i)->am_csComment;
		VERIFY(m_CBdetectChan.AddString(comment) != CB_ERR);
	}

	// load transforms names
	m_CBtransform.ResetContent();
	m_CBtransform2.ResetContent();
	const int n_transform_types = p_data_file->GetTransfDataNTypes();
	for (auto j = 0; j < n_transform_types; j++)
	{
		VERIFY(m_CBtransform.AddString(p_data_file->GetTransfDataName(j)) != CB_ERR);
		VERIFY(m_CBtransform2.AddString(p_data_file->GetTransfDataName(j)) != CB_ERR);
	}
	m_CBdetectChan.SetCurSel(m_p_detect_parameters->detectChan);
}

void ViewSpikeDetection::define_stretch_parameters()
{
	m_stretch.AttachParent(this);

	// top right ----------------------------------------
	m_stretch.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_EMPTYPICTURE, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newSlaveProp(IDC_DISPLAYDATA, XLEQ_XREQ, SZPR_YTEQ, IDC_EMPTYPICTURE);
	m_stretch.newSlaveProp(IDC_DISPLAYDETECT, XLEQ_XREQ, SZPR_YBEQ, IDC_EMPTYPICTURE);

	m_stretch.newSlaveProp(IDC_CHANSELECTED2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	m_stretch.newSlaveProp(IDC_GAIN2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	m_stretch.newSlaveProp(IDC_BIAS2, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDATA);
	m_stretch.newSlaveProp(IDC_SCROLLY2, SZEQ_XREQ, YTEQ_YBEQ, IDC_DISPLAYDATA);
	m_stretch.newSlaveProp(IDC_STATICDISPLAYDATA, SZEQ_XLEQ, YTEQ_YBEQ, IDC_DISPLAYDATA);

	m_stretch.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newSlaveProp(IDC_CHANSELECTED, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	m_stretch.newSlaveProp(IDC_GAIN, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	m_stretch.newSlaveProp(IDC_BIAS, SZEQ_XREQ, SZEQ_YTEQ, IDC_DISPLAYDETECT);
	m_stretch.newSlaveProp(IDC_SCROLLY, SZEQ_XREQ, YTEQ_YBEQ, IDC_DISPLAYDETECT);
	m_stretch.newSlaveProp(IDC_STATICDISPLAYDETECT, SZEQ_XLEQ, YTEQ_YBEQ, IDC_DISPLAYDETECT);

	// bottom right  ------------------------------------
	m_stretch.newProp(IDC_DISPLAYBARS, XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_STATICDISPLAYBARS, SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_DURATIONTEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SPIKEWINDOWLENGTH, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_MINTEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SPIKEWINDOWAMPLITUDE, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_NBSPIKES_NB, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_NBSPIKES_TEXT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TRANSFORM2, SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_STATIC3, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SPIKENO, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_ARTEFACT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_DISPLAYSPIKES, SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SOURCE, SZEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_XSCALE, XLEQ_XREQ, SZEQ_YBEQ);
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
		(LPARAM)static_cast<HANDLE>(m_hBias));
	GetDlgItem(IDC_GAIN)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		(LPARAM)static_cast<HANDLE>(m_hZoom));
	VERIFY(m_scrolly2.SubclassDlgItem(IDC_SCROLLY2, this));
	m_scrolly2.SetScrollRange(0, 100);
	m_hBias2 = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom2 = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS2)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		(LPARAM)static_cast<HANDLE>(m_hBias2));
	GetDlgItem(IDC_GAIN2)->PostMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
		(LPARAM)static_cast<HANDLE>(m_hZoom2));
}

void ViewSpikeDetection::OnInitialUpdate()
{
	// load spike detection parameters from .INI file
	const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	m_pArrayFromApp = &(p_app->spkDA); 
	options_view_data = &(p_app->options_viewdata); 

	define_stretch_parameters();
	m_binit = TRUE;
	m_autoIncrement = true;
	m_autoDetect = true;

	define_sub_classed_items();

	dbTableView::OnInitialUpdate();

	// load file data
	if (m_chart_data_filtered.m_HZtags.GetNTags() < 1)
		m_chart_data_filtered.m_HZtags.AddTag(0, 0);

	update_file_parameters(TRUE);
	m_chart_data_filtered.SetScopeParameters(&(options_view_data->viewdata));
	m_chart_data_filtered.Invalidate();
	m_chart_data_source.SetScopeParameters(&(options_view_data->viewdata));
	m_chart_data_source.Invalidate();
}

LRESULT ViewSpikeDetection::OnMyMessage(WPARAM wParam, LPARAM lParam)
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
		m_p_detect_parameters->detectThreshold = m_chart_data_filtered.m_HZtags.GetValue(threshold);
		m_thresholdval = m_chart_data_filtered.GetChanlistItem(0)
			->ConvertDataBinsToVolts(
				m_chart_data_filtered.m_HZtags.GetValue(threshold)) * 1000.f;
		m_p_detect_parameters->detectThresholdmV = m_thresholdval;
		mm_thresholdval.m_bEntryDone = TRUE;
		OnEnChangeThresholdval();
		break;

		// ----------------------------- select bar/display bars or zoom
	case HINT_CHANGEHZLIMITS: 
		m_chart_data_filtered.GetDataFromDoc(m_chart_spike_bar.GetTimeFirst(), m_chart_spike_bar.GetTimeLast());
		m_chart_data_source.GetDataFromDoc(m_chart_spike_bar.GetTimeFirst(), m_chart_spike_bar.GetTimeLast());
		update_legends();
		break;

	case HINT_HITSPIKE: // spike is selected or deselected
		select_spike_no(threshold, FALSE);
		update_spike_display();
		break;

	case HINT_DBLCLKSEL:
		if (threshold < 0)
			threshold = 0;
		m_spikeno = threshold;
		OnToolsEdittransformspikes();
		break;

	case HINT_CHANGEZOOM:
		update_spike_shape_window_scale(TRUE);
		m_chart_spike_shape.Invalidate();
		break;

	case HINT_VIEWSIZECHANGED:
		if (i_id == m_chart_data_source.GetDlgCtrlID())
		{
			const auto l_first = m_chart_data_source.GetDataFirst(); // get source data time range
			const auto l_last = m_chart_data_source.GetDataLast();
			m_chart_data_filtered.GetDataFromDoc(l_first, l_last);
		}
		// else if(iID == m_displayDetect.GetDlgCtrlID())
		// UpdateLegends updates data window from m_displayDetect
		update_legends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_view_data->viewspkdetectfiltered = *m_chart_data_filtered.GetScopeParameters();
		options_view_data->viewspkdetectdata = *m_chart_data_source.GetScopeParameters();
		options_view_data->viewspkdetectspk = *m_chart_spike_bar.GetScopeParameters();
		options_view_data->viewspkdetectbars = *m_chart_spike_shape.GetScopeParameters();
		break;

	case HINT_DEFINEDRECT:
		if (m_cursor_state == CURSOR_CROSS)
		{
			const auto rect = m_chart_data_filtered.GetDefinedRect();
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
			lvalue = m_chart_data_filtered.m_VTtags.GetTagLVal(threshold);
		else if (i_id == m_chart_data_source.GetDlgCtrlID())
			lvalue = m_chart_data_source.m_VTtags.GetTagLVal(threshold);

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
		long l_first;
		long l_last;
		m_pSpkList->GetRangeOfSpikeFlagged(l_first, l_last);
		const auto l_time = m_pSpkList->GetSpike(threshold)->get_time();
		if (l_time < l_first)
			l_first = l_time;
		if (l_time > l_last)
			l_last = l_time;
		m_pSpkList->FlagRangeOfSpikes(l_first, l_last, TRUE);
		update_spike_display();
	}
	break;

	case HINT_HITSPIKE_CTRL: // add/remove selected spike to/from the group of spikes selected
		select_spike_no(threshold, TRUE);
		update_spike_display();
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
		m_chart_data_filtered.GetDataFromDoc(l_first, l_last);
		m_chart_data_source.GetDataFromDoc(l_first, l_last);
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
	m_filescroll_infos.nPos = m_chart_data_filtered.GetDataFirst();
	m_filescroll_infos.nPage = m_chart_data_filtered.GetDataLast() - m_chart_data_filtered.GetDataFirst() + 1;
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
		b_result = m_chart_data_source.ScrollDataFromDoc(nSBCode);
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		b_result = m_chart_data_source.GetDataFromDoc(
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
	m_chart_data_filtered.CenterChan(0);
	m_chart_data_filtered.Invalidate();

	for (auto i = 0; i < m_chart_data_source.GetChanlistSize(); i++)
		m_chart_data_source.CenterChan(i);
	m_chart_data_source.Invalidate();

	m_chart_spike_bar.CenterCurve();
	m_chart_spike_bar.Invalidate();

	m_chart_spike_shape.SetYWExtOrg(m_chart_spike_bar.GetYWExtent(), m_chart_spike_bar.GetYWOrg());
	update_spike_shape_window_scale(TRUE);
	m_chart_spike_shape.Invalidate();
}

void ViewSpikeDetection::OnFormatYscaleGainadjust()
{
	m_chart_data_filtered.MaxgainChan(0);
	m_chart_data_filtered.SetChanlistVoltsExtent(-1, nullptr);
	m_chart_data_filtered.Invalidate();

	for (int i = 0; i < m_chart_data_source.GetChanlistSize(); i++)
		m_chart_data_source.MaxgainChan(i);
	m_chart_data_source.SetChanlistVoltsExtent(-1, nullptr);
	m_chart_data_source.Invalidate();

	m_chart_spike_bar.MaxCenter();
	m_chart_spike_bar.Invalidate();

	m_chart_spike_shape.SetYWExtOrg(m_chart_spike_bar.GetYWExtent(), m_chart_spike_bar.GetYWOrg());
	m_chart_spike_shape.SetXWExtOrg(m_pSpkList->GetSpikeLength(), 0);
	update_spike_shape_window_scale(FALSE);
	m_chart_spike_shape.Invalidate();
}

void ViewSpikeDetection::OnFormatSplitcurves()
{
	m_chart_data_filtered.SplitChans();
	m_chart_data_filtered.SetChanlistVoltsExtent(-1, nullptr);
	m_chart_data_filtered.Invalidate();

	m_chart_data_source.SplitChans();
	m_chart_data_source.SetChanlistVoltsExtent(-1, nullptr);
	m_chart_data_source.Invalidate();

	// center curve and display bar & spikes
	m_chart_spike_bar.MaxCenter();
	m_chart_spike_bar.Invalidate();

	m_chart_spike_shape.SetYWExtOrg(m_chart_spike_bar.GetYWExtent(), m_chart_spike_bar.GetYWOrg());
	update_spike_shape_window_scale(FALSE);
	m_chart_spike_shape.Invalidate();
}

void ViewSpikeDetection::OnFormatAlldata()
{
	const auto l_last = GetDocument()->m_pDat->GetDOCchanLength();
	m_chart_data_filtered.ResizeChannels(0, l_last);
	m_chart_data_filtered.GetDataFromDoc(0, l_last);

	m_chart_data_source.ResizeChannels(0, l_last);
	m_chart_data_source.GetDataFromDoc(0, l_last);

	const auto x_we = m_pSpkList->GetSpikeLength();
	if (x_we != m_chart_spike_shape.GetXWExtent() || 0 != m_chart_spike_shape.GetXWOrg())
		m_chart_spike_shape.SetXWExtOrg(x_we, 0);
	m_chart_spike_bar.CenterCurve();
	m_chart_spike_shape.SetYWExtOrg(m_chart_spike_bar.GetYWExtent(), m_chart_spike_bar.GetYWOrg());

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
	dlg.m_dbDoc = GetDocument()->m_pDat;
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
	m_p_detect_parameters->detectChan = m_CBdetectChan.GetCurSel();
	m_p_detect_parameters->bChanged = TRUE;
	m_chart_data_filtered.SetChanlistOrdinates(0, m_p_detect_parameters->detectChan, m_p_detect_parameters->detectTransform);
	const CChanlistItem* channel_list_item = m_chart_data_filtered.GetChanlistItem(0);
	m_p_detect_parameters->detectThreshold = channel_list_item->ConvertVoltsToDataBins(m_p_detect_parameters->detectThresholdmV / 1000.f);
	m_chart_data_filtered.MoveHZtagtoVal(0, m_p_detect_parameters->detectThreshold);
	m_chart_data_filtered.GetDataFromDoc();
	m_chart_data_filtered.AutoZoomChan(0);
	m_chart_data_filtered.Invalidate();
}

void ViewSpikeDetection::OnSelchangeTransform()
{
	UpdateData(TRUE);
	m_p_detect_parameters->detectTransform = m_CBtransform.GetCurSel();
	m_p_detect_parameters->bChanged = TRUE;
	m_chart_data_filtered.SetChanlistTransformMode(0, m_p_detect_parameters->detectTransform);
	m_chart_data_filtered.GetDataFromDoc();
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
	const auto data_document = db_document->m_pDat;
	m_pSpkDoc->SetAcqFilename(db_document->GetDB_CurrentDatFileName());
	m_pSpkDoc->InitSourceDoc(data_document);

	m_pSpkDoc->SetDetectionDate(CTime::GetCurrentTime());
	auto old_spike_list_index = db_document->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();
	m_spikeno = -1;

	// check if detection parameters are ok? prevent detection from a channel that does not exist
	const auto p_dat = db_document->m_pDat;
	if (p_dat == nullptr)
		return;
	const auto wave_format = p_dat->GetpWaveFormat();
	const auto chan_max = wave_format->scan_count - 1;
	for (auto i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		const auto spike_detect_array = m_spk_detect_array_current.GetItem(i);
		if (spike_detect_array->extractChan > chan_max)
		{
			MessageBox(_T(
				"Check spike detection parameters \n- one of the detection channel requested \nis not available in the source data"));
			return;
		}
	}

	// adjust size of spike list array
	if (m_spk_detect_array_current.GetSize() != m_pSpkDoc->GetSpkList_Size())
		m_pSpkDoc->SetSpkList_Size(m_spk_detect_array_current.GetSize());

	// detect spikes from all channels marked as such
	for (int i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		if (!bAll && m_i_detect_parameters != i)
			continue;
		// detect missing data channel
		if (m_spk_detect_array_current.GetItem(i)->extractChan > chan_max)
			continue;

		// select new spike list (list with no spikes for stimulus channel)
		SpikeList* spike_list = m_pSpkDoc->SetSpkList_AsCurrent(i);
		if (spike_list == nullptr)
		{
			m_pSpkDoc->AddSpkList();
			spike_list = m_pSpkDoc->GetSpkList_Current();
		}

		m_pSpkList = spike_list;
		if (m_pSpkList->GetTotalSpikes() == 0)
		{
			SPKDETECTPARM* pFC = m_spk_detect_array_current.GetItem(i);
			ASSERT_VALID(pFC);
			ASSERT(pFC != NULL);
			m_pSpkList->InitSpikeList(db_document->m_pDat, pFC);
		}
		if ((m_spk_detect_array_current.GetItem(i))->detectWhat == DETECT_SPIKES)
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
	saveCurrentSpkFile();

	// display data
	if (old_spike_list_index < 0)
		old_spike_list_index = 0;
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(old_spike_list_index);

	m_chart_spike_bar.SetSourceData_spklist_dbwavedoc(m_pSpkList, db_document);
	m_chart_spike_shape.SetSourceData(m_pSpkList, db_document);


	// center spikes, change nb spikes and update content of draw buttons
	if (options_view_data->bMaximizeGain
		|| m_chart_spike_bar.GetYWExtent() == 0
		|| m_chart_spike_bar.GetYWOrg() == 0
		|| m_chart_spike_shape.GetYWExtent() == 0
		|| m_chart_spike_shape.GetYWOrg() == 0)
	{
		m_chart_spike_bar.MaxCenter();
		m_chart_spike_shape.SetYWExtOrg(m_chart_spike_bar.GetYWExtent(), m_chart_spike_bar.GetYWOrg());
		int spike_length = 60;
		if (m_pSpkList != nullptr)
			spike_length = m_pSpkList->GetSpikeLength();
		m_chart_spike_shape.SetXWExtOrg(spike_length, 0);
		update_spike_shape_window_scale(FALSE);
	}

	highlight_spikes(FALSE);
	update_legends();
	update_tabs();
}

int ViewSpikeDetection::detect_stimulus_1(int channel_index)
{
	const auto detect_parameters = m_spk_detect_array_current.GetItem(channel_index);
	const auto threshold = detect_parameters->detectThreshold;
	const auto detect_transform = detect_parameters->detectTransform; 
	const auto source_channel = detect_parameters->detectChan;
	const auto data_document = GetDocument()->m_pDat;
	const auto detect_transform_span = data_document->GetTransfDataSpan(detect_transform); 

	// detect mode: 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);
	auto b_cross_upw = TRUE;
	if (detect_parameters->detectMode == MODE_OFF_ON || detect_parameters->detectMode == MODE_OFF_OFF)
		b_cross_upw = FALSE;
	auto b_mode = TRUE;
	if (detect_parameters->detectMode == MODE_ON_ON || detect_parameters->detectMode == MODE_OFF_OFF)
		b_mode = FALSE;
	auto b_save_on = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = m_chart_data_filtered.GetDataFirst();
	const auto l_data_last = m_chart_data_filtered.GetDataLast();

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
		if (!data_document->LoadRawData(&l_rw_first, &l_rw_last, detect_transform_span))
			break; 
		if (!data_document->BuildTransfData(detect_transform, source_channel))
			break;

		// compute initial offset (address of first point)
		auto l_last = l_rw_last;
		if (l_last > l_data_last)
			l_last = l_data_last;
		const int i_buf_first = l_data_first - data_document->GetDOCchanIndexFirst();
		const auto p_data_first = data_document->GetpTransfDataElmt(i_buf_first);

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
	if (spike_detection_parameters->extractTransform != spike_detection_parameters->detectTransform &&
		spike_detection_parameters->extractTransform != 0)
	{
		AfxMessageBox(
			_T("Options not implemented yet!\nd chan == extr chan or !extr chan=0\nChange detection parameters"));
		return m_pSpkList->GetTotalSpikes();
	}

	// set parameters (copy array into local parms)
	const short threshold = spike_detection_parameters->detectThreshold; // threshold value
	const auto method = spike_detection_parameters->detectTransform; // how source data are transformed
	const auto source_channel = spike_detection_parameters->detectChan; // source channel
	const auto pre_threshold = spike_detection_parameters->prethreshold; // pts before threshold
	const auto refractory = spike_detection_parameters->refractory; // refractory period
	const auto post_threshold = spike_detection_parameters->extractNpoints - pre_threshold;

	// get parameters from document
	const auto p_dat = GetDocument()->m_pDat;
	int n_channels; // number of data channels / source buffer
	const auto p_buf = p_dat->LoadRawDataParams(&n_channels);
	const auto span = p_dat->GetTransfDataSpan(method); // nb pts to read before transf

	// adjust detection method: if threshold lower than data zero detect lower crossing
	auto b_cross_upw = TRUE;
	if (threshold < 0)
		b_cross_upw = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = m_chart_data_filtered.GetDataFirst(); // index first pt to test
	auto l_data_last = m_chart_data_filtered.GetDataLast(); // index last pt to test
	if (l_data_first < pre_threshold + span)
		l_data_first = static_cast<long>(pre_threshold) + span;
	if (l_data_last > p_dat->GetDOCchanLength() - post_threshold - span)
		l_data_last = p_dat->GetDOCchanLength() - post_threshold - span;

	// loop through data defined in the lineview window
	while (l_data_first < l_data_last)
	{
		auto l_rw_first = l_data_first - pre_threshold; // index very first pt within buffers
		auto l_rw_last = l_data_last; // index very last pt within buffers
		if (!p_dat->LoadRawData(&l_rw_first, &l_rw_last, span)) // load data from file
			break; // exit if error reported
		if (!p_dat->BuildTransfData(method, source_channel)) // transfer data into a buffer with a single channel
			break; // exit if fail

		// load a chunk of data and see if any spikes are detected within it
		// compute initial offset (address of first point
		auto l_last = l_rw_last - post_threshold;
		if (l_last > l_data_last)
			l_last = l_data_last;
		const int i_buf_first = l_data_first - p_dat->GetDOCchanIndexFirst();
		const auto p_data_first = p_dat->GetpTransfDataElmt(i_buf_first);

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
				if (*p_data < threshold) // test if a spike is present
					continue; // no: loop to next point

				// search max and threshold crossing
				auto max = *p_data; // init max
				auto p_data1 = p_data; // init pointer
				ii_time = cx; // init spike time
				// loop to search max
				for (auto i = cx; i < cx + refractory; i++, p_data1++)
				{
					if (max < *p_data1) // search max
					{
						max = *p_data1;
						p_data = p_data1; // p_data = "center" of spike
						ii_time = i; // ii_time = time of spike
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
			if (spike_detection_parameters->extractTransform == spike_detection_parameters->detectTransform)
			{
				const auto p_m = p_data - pre_threshold;
				m_pSpkList->AddSpike(p_m, 1, ii_time, source_channel, 0, TRUE);
			}
			else // extract from raw data
			{
				const auto pM = p_buf
					+ n_channels * (ii_time - pre_threshold - l_rw_first + span)
					+ spike_detection_parameters->extractChan;
				m_pSpkList->AddSpike(pM, n_channels, ii_time, source_channel, 0, TRUE);
			}

			// update loop parameters
			cx = ii_time + refractory;
		}

		///////////////////////////////////////////////////////////////
		l_data_first = cx + 1; // update for next loop
	}

	return m_pSpkList->GetTotalSpikes();
}

void ViewSpikeDetection::OnToolsEdittransformspikes()
{
	DlgSpikeEdit dlg; // dialog box
	dlg.m_yextent = m_chart_spike_shape.GetYWExtent();
	dlg.m_yzero = m_chart_spike_shape.GetYWOrg(); 
	dlg.m_xextent = m_chart_spike_shape.GetXWExtent(); 
	dlg.m_xzero = m_chart_spike_shape.GetXWOrg(); 
	dlg.m_spikeno = m_spikeno; 
	m_pSpkList->RemoveAllSpikeFlags();
	dlg.m_pdbWaveDoc = GetDocument();
	dlg.m_parent = this;

	// open dialog box and wait for response
	dlg.DoModal();

	m_spikeno = dlg.m_spikeno;
	select_spike_no(m_spikeno, FALSE);
	update_spike_display();
	if (dlg.m_bchanged)
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
		CChanlistItem* pchan = m_chart_data_filtered.GetChanlistItem(m_p_detect_parameters->detectChan);
		dlg.m_yzero = pchan->GetYzero();
		dlg.m_yextent = pchan->GetYextent();
		dlg.m_bDisplaysource = TRUE;
	}
	else if (pFocus != nullptr && m_chart_spike_bar.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		dlg.m_yzero = m_chart_spike_bar.GetYWOrg();
		dlg.m_yextent = m_chart_spike_bar.GetYWExtent();
		dlg.m_bDisplaybars = TRUE;
	}
	else
	{
		dlg.m_xzero = m_chart_spike_shape.GetXWOrg();
		dlg.m_xextent = m_chart_spike_shape.GetXWExtent();
		dlg.m_yzero = m_chart_spike_shape.GetYWOrg();
		dlg.m_yextent = m_chart_spike_shape.GetYWExtent();
		dlg.m_bDisplayspikes = TRUE;
	}

	if (IDOK == dlg.DoModal())
	{
		if (dlg.m_bDisplaysource)
		{
			CChanlistItem* chan = m_chart_data_filtered.GetChanlistItem(0);
			chan->SetYzero(dlg.m_yzero);
			chan->SetYextent(dlg.m_yextent);
			m_chart_data_filtered.Invalidate();
		}
		if (dlg.m_bDisplaybars)
		{
			m_chart_spike_bar.SetYWExtOrg(dlg.m_yextent, dlg.m_yzero);
			m_chart_spike_bar.Invalidate();
		}
		if (dlg.m_bDisplayspikes)
		{
			m_chart_spike_shape.SetYWExtOrg(dlg.m_yextent, dlg.m_yzero);
			m_chart_spike_shape.SetXWExtOrg(dlg.m_xextent, dlg.m_xzero);
			m_chart_spike_shape.Invalidate();
		}
	}
}

void ViewSpikeDetection::OnBnClickedClearAll()
{
	m_spikeno = -1; 
	m_chart_spike_bar.SelectSpike(m_spikeno);
	m_chart_spike_shape.SelectSpikeShape(m_spikeno);

	// update spike list
	for (int i = 0; i < m_pSpkDoc->GetSpkList_Size(); i++)
	{
		SpikeList* pspklist = m_pSpkDoc->SetSpkList_AsCurrent(i);
		pspklist->InitSpikeList(GetDocument()->m_pDat, nullptr);
	}
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	ASSERT(m_pSpkList != NULL);

	highlight_spikes(FALSE); // remove display of spikes
	m_chart_spike_shape.SetSourceData(m_pSpkList, GetDocument());
	m_pSpkDoc->m_stimulus_intervals.n_items = 0; // zero stimuli
	m_pSpkDoc->m_stimulus_intervals.RemoveAll();

	update_detection_parameters();
	update_VT_tags(); // update display of vertical tags
	update_legends(); // change legends
	m_pSpkDoc->SetModifiedFlag(TRUE); // mark spike document as changed
}

void ViewSpikeDetection::OnClear()
{
	m_spikeno = -1;
	m_chart_spike_bar.SelectSpike(m_spikeno);
	m_chart_spike_shape.SelectSpikeShape(m_spikeno);

	m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	m_pSpkList->InitSpikeList(GetDocument()->m_pDat, nullptr);
	highlight_spikes(FALSE);

	if (m_pSpkList->GetDetectParms()->detectWhat == DETECT_STIMULUS)
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
		auto spike_no = m_spikeno;
		switch (mm_spikeno.m_nChar)
		{
			// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			spike_no = m_spikeno;
			break;
		case VK_UP:
		case VK_PRIOR:
			spike_no++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			spike_no--;
			break;
		default:;
		}
		// check boundaries
		if (spike_no < -1)
			spike_no = -1;
		if (spike_no >= m_pSpkList->GetTotalSpikes())
			spike_no = m_pSpkList->GetTotalSpikes() - 1;

		// change display if necessary
		mm_spikeno.m_bEntryDone = FALSE; // clear flag
		mm_spikeno.m_nChar = 0; // empty buffer
		mm_spikeno.SetSel(0, -1); // select all text

		// update spike num and center display on the selected spike
		select_spike_no(spike_no, FALSE);
		update_spike_display();
	}
}

void ViewSpikeDetection::OnArtefact()
{
	UpdateData(TRUE); // load value from control
	const auto n_spikes = m_pSpkList->GetSpikeFlagArrayCount();
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
			const auto spike_no = m_pSpkList->GetSpikeFlagArrayAt(i);
			Spike* spike = m_pSpkList->GetSpike(spike_no);
			auto spike_class = spike->get_class();

			// if artefact: set class to negative value
			if ((m_bartefact && spike_class >= 0) || (spike_class < 0))
				spike_class = -(spike_class + 1);
			spike->set_class(spike_class);
		}

		m_pSpkDoc->SetModifiedFlag(TRUE);
		saveCurrentSpkFile();
	}
	m_spikeno = -1;

	const auto i_sel_parameters = m_tabCtrl.GetCurSel();
	m_p_detect_parameters = m_spk_detect_array_current.GetItem(i_sel_parameters);
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(i_sel_parameters);

	select_spike_no(m_spikeno, FALSE);
	update_spike_display();
	update_number_of_spikes();
}

void ViewSpikeDetection::update_number_of_spikes()
{
	const int total_spikes = m_pSpkList->GetTotalSpikes();
	if (total_spikes != static_cast<int>(GetDlgItemInt(IDC_NBSPIKES_NB)))
		SetDlgItemInt(IDC_NBSPIKES_NB, total_spikes);
}

void ViewSpikeDetection::align_display_to_current_spike()
{
	if (m_spikeno < 0)
		return;

	const auto l_spike_time = m_pSpkList->GetSpike(m_spikeno)->get_time();
	if (l_spike_time < m_chart_data_filtered.GetDataFirst()
		|| l_spike_time > m_chart_data_filtered.GetDataLast())
	{
		const auto l_size = m_chart_data_filtered.GetDataLast() - m_chart_data_filtered.GetDataFirst();
		auto l_first = l_spike_time - l_size / 2;
		if (l_first < 0)
			l_first = 0;
		auto l_last = l_first + l_size - 1;
		if (l_last > m_chart_data_filtered.GetDocumentLast())
		{
			l_last = m_chart_data_filtered.GetDocumentLast();
			l_first = l_last - l_size + 1;
		}
		m_chart_data_filtered.GetDataFromDoc(l_first, l_last);
		m_chart_data_source.GetDataFromDoc(l_first, l_last);
		update_legends();
	}
}

void ViewSpikeDetection::update_spike_shape_window_scale(const BOOL b_set_from_controls)
{
	// get current values
	int ix_we;
	auto iy_we = 0;

	// if set from controls, get value from the controls
	if (b_set_from_controls && m_pSpkList->GetTotalSpikes() > 0)
	{
		// set time scale
		CString cs;
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->GetWindowText(cs);
		const auto x = static_cast<float>(_ttof(cs)) / 1000.0f;
		ix_we = static_cast<int>(m_pSpkList->GetAcqSampRate() * x);
		if (ix_we == 0)
			ix_we = m_pSpkList->GetDetectParms()->extractNpoints;
		ASSERT(ix_we != 0);
		m_chart_spike_shape.SetXWExtOrg(ix_we, m_chart_spike_shape.GetXWOrg());
		
		// set amplitude
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->GetWindowText(cs);
		if (!cs.IsEmpty())
		{
			const auto y = static_cast<float>(_ttof(cs)) / 1000.0f;
			iy_we = static_cast<int>(y / m_pSpkList->GetAcqVoltsperBin());
		}
		if (iy_we == 0)
			iy_we = m_chart_spike_shape.GetYWExtent();
		m_chart_spike_shape.SetYWExtOrg(iy_we, m_chart_spike_shape.GetYWOrg());
	}
	else
	{
		ix_we = m_chart_spike_shape.GetXWExtent();
		iy_we = m_chart_spike_shape.GetYWExtent();
	}

	if (ix_we != NULL && iy_we != NULL)
	{
		const float x = m_chart_spike_shape.GetExtent_ms() / static_cast<float>(m_chart_spike_shape.GetNxScaleCells());
		m_chart_spike_shape.SetxScaleUnitValue(x);

		const float y = m_chart_spike_shape.GetExtent_mV() / static_cast<float>(m_chart_spike_shape.GetNyScaleCells());
		m_chart_spike_shape.SetyScaleUnitValue(y);
		
	}
	
	CString dummy1;
	dummy1.Format(_T("%.3lf"), m_chart_spike_shape.GetExtent_mV());
	SetDlgItemText(IDC_SPIKEWINDOWAMPLITUDE, dummy1);

	CString dummy2;
	dummy2.Format(_T("%.3lf"), m_chart_spike_shape.GetExtent_ms());
	SetDlgItemText(IDC_SPIKEWINDOWLENGTH, dummy2);
}

void ViewSpikeDetection::select_spike_no(int spike_index, BOOL bMultipleSelection)
{
	if (spike_index >= 0)
	{
		m_pSpkList = m_pSpkDoc->GetSpkList_Current();
		const auto p_spike_element = m_pSpkList->GetSpike(spike_index);
		m_bartefact = (p_spike_element->get_class() < 0);
		if (bMultipleSelection)
		{
			m_pSpkList->ToggleSpikeFlag(spike_index);
			if (m_pSpkList->GetSpikeFlagArrayCount() < 1)
				spike_index = -1;
			if (m_spikeno == spike_index)
				spike_index = 0;
		}
		else
		{
			m_pSpkList->SetSingleSpikeFlag(spike_index);
		}
		m_spikeno = spike_index;
		align_display_to_current_spike();
	}
	else
	{
		m_pSpkList->RemoveAllSpikeFlags();
		m_bartefact = FALSE;
	}
}

void ViewSpikeDetection::update_spike_display()
{
	// update spike display windows
	m_chart_spike_bar.Invalidate(TRUE);
	m_chart_spike_shape.Invalidate(TRUE);

	// update Dlg interface
	GetDlgItem(IDC_SPIKENO)->EnableWindow(!(m_pSpkList->GetSpikeFlagArrayCount() > 1));
	SetDlgItemInt(IDC_SPIKENO, m_spikeno, TRUE);
	CheckDlgButton(IDC_ARTEFACT, m_bartefact);
}

void ViewSpikeDetection::OnEnChangeThresholdval()
{
	if (mm_thresholdval.m_bEntryDone)
	{
		auto threshold_value = m_thresholdval;
		switch (mm_thresholdval.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			threshold_value = m_thresholdval;
			break;
		case VK_UP:
		case VK_PRIOR:
			threshold_value++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			threshold_value--;
			break;
		default:;
		}

		// change display if necessary
		if (m_thresholdval != threshold_value)
		{
			m_thresholdval = threshold_value;
			m_p_detect_parameters->detectThresholdmV = threshold_value;
			const CChanlistItem* channel_item = m_chart_data_filtered.GetChanlistItem(0);
			m_p_detect_parameters->detectThreshold = channel_item->ConvertVoltsToDataBins(m_thresholdval / 1000.f);
			m_chart_data_filtered.MoveHZtagtoVal(0, m_p_detect_parameters->detectThreshold);
		}

		mm_thresholdval.m_bEntryDone = FALSE;
		mm_thresholdval.m_nChar = 0;
		CString cs;
		cs.Format(_T("%.3f"), m_thresholdval);
		GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
		mm_thresholdval.SetSel(0, -1);
	}
}

void ViewSpikeDetection::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone)
	{
		switch (mm_timefirst.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR:
			m_timefirst++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_timefirst--;
			break;
		default:;
		}
		m_chart_data_filtered.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		m_chart_data_source.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		update_legends();

		mm_timefirst.m_bEntryDone = FALSE;
		mm_timefirst.m_nChar = 0;
		mm_timefirst.SetSel(0, -1);
	}
}

void ViewSpikeDetection::OnEnChangeTimelast()
{
	if (mm_timelast.m_bEntryDone)
	{
		switch (mm_timelast.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_timelast++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_timelast--;
			break;
		default:;
		}
		m_chart_data_filtered.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		m_chart_data_source.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		update_legends();

		mm_timelast.m_bEntryDone = FALSE;
		mm_timelast.m_nChar = 0;
		mm_timelast.SetSel(0, -1); //select all text
	}
}

void ViewSpikeDetection::OnToolsDataseries()
{
	// init dialog data
	DlgDataSeries dlg;
	dlg.m_pChartDataWnd = &m_chart_data_filtered;
	dlg.m_pdbDoc = GetDocument()->m_pDat;
	dlg.m_listindex = 0;

	// invoke dialog box
	dlg.DoModal();
	if (m_chart_data_filtered.GetChanlistSize() < 1)
	{
		m_chart_data_filtered.RemoveAllChanlistItems();
		m_chart_data_filtered.AddChanlistItem(m_p_detect_parameters->detectChan, m_p_detect_parameters->detectTransform);
	}
	update_legends();
}

void ViewSpikeDetection::PrintDataCartridge(CDC* p_dc, ChartData* pDataChartWnd, CRect* prect, BOOL bComments)
{
	SCOPESTRUCT* p_struct = pDataChartWnd->GetScopeParameters();
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
			m_chart_data_filtered.CopyAsText(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			serialize_windows_state(b_save);

			CRect old_rect1; // save size of line view windows
			m_chart_data_filtered.GetWindowRect(&old_rect1);
			CRect old_rect2;
			m_chart_data_source.GetWindowRect(&old_rect2);

			const CRect rect(0, 0, options_view_data->hzResolution, options_view_data->vtResolution);
			m_npixels0 = m_chart_data_filtered.GetRectWidth();

			// create meta file
			CMetaFileDC m_dc;
			auto rect_bound = rect;
			rect_bound.right *= 32;
			rect_bound.bottom *= 30;
			const auto p_dc_ref = GetDC();
			auto cs_title = _T("dbWave\0") + GetDocument()->m_pDat->GetTitle();
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
			m_pOldFont = nullptr;
			const auto old_font_size = options_view_data->fontsize;
			options_view_data->fontsize = 10;
			PrintCreateFont();
			m_dc.SetBkMode(TRANSPARENT);
			options_view_data->fontsize = old_font_size;
			m_pOldFont = m_dc.SelectObject(&m_fontPrint);
			const int line_height = m_logFont.lfHeight + 5;
			auto row = 0;
			const auto column = 10;

			// comment and descriptors
			auto comments = GetDocument()->Export_DatabaseData(1);
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
			const auto rect_spike_width = MulDiv(m_chart_spike_shape.GetRectWidth(), data_rect.Width(),
				m_chart_spike_shape.GetRectWidth() + m_chart_data_filtered.GetRectWidth());
			const auto rect_data_height = MulDiv(m_chart_data_filtered.GetRectHeight(), data_rect.Height(),
				m_chart_data_filtered.GetRectHeight() * 2 + m_chart_spike_bar.
				GetRectHeight());
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
			m_chart_spike_bar.Print(&m_dc, &rect_bars);

			// display spike shapes
			auto rect_spikes = rect; // compute output rectangle
			rect_spikes.left += separator;
			rect_spikes.right = rect.left + rect_spike_width;
			rect_spikes.bottom = rect.bottom - 2 * line_height;
			rect_spikes.top = rect_spikes.bottom - rect_bars.Height();
			m_chart_spike_shape.Print(&m_dc, &rect_spikes);
			comments = PrintSpkShapeBars(&m_dc, &rect_spikes, TRUE);

			auto rect_comment = rect;
			rect_comment.right = data_rect.left;
			rect_comment.top = rect_spikes.bottom;
			constexpr UINT n_format = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
			m_dc.DrawText(comments, comments.GetLength(), rect_comment, n_format);
			m_dc.SelectObject(p_old_brush);

			if (m_pOldFont != nullptr)
				m_dc.SelectObject(m_pOldFont);
			m_fontPrint.DeleteObject();

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
	m_p_detect_parameters->detectWhat = m_CBdetectWhat.GetCurSel();
	update_combo_box();
	update_legend_detection_wnd();
	m_chart_data_filtered.GetDataFromDoc();
	m_chart_data_filtered.AutoZoomChan(0);
	m_chart_data_filtered.Invalidate();
}

void ViewSpikeDetection::update_combo_box()
{
	m_CBdetectChan.SetCurSel(m_p_detect_parameters->detectChan);
	m_CBtransform.SetCurSel(m_p_detect_parameters->detectTransform);
	m_chart_data_filtered.SetChanlistOrdinates(0, m_p_detect_parameters->detectChan, m_p_detect_parameters->detectTransform);
	m_p_detect_parameters->detectThreshold = m_chart_data_filtered.GetChanlistItem(0)->ConvertVoltsToDataBins(
		m_thresholdval / 1000.f);
	m_chart_data_filtered.m_HZtags.SetTagChan(0, 0);
	m_chart_data_filtered.m_HZtags.SetTagVal(0, m_p_detect_parameters->detectThreshold);
	m_p_detect_parameters->detectThresholdmV = m_thresholdval;
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
	m_chart_spike_bar.m_VTtags.RemoveAllTags();
	m_chart_data_filtered.m_VTtags.RemoveAllTags();
	m_chart_data_source.m_VTtags.RemoveAllTags();
	if (m_pSpkDoc->m_stimulus_intervals.n_items == 0)
		return;

	for (auto i = 0; i < m_pSpkDoc->m_stimulus_intervals.GetSize(); i++)
	{
		const int cx = m_pSpkDoc->m_stimulus_intervals.GetAt(i);
		m_chart_spike_bar.m_VTtags.AddLTag(cx, 0);
		m_chart_data_filtered.m_VTtags.AddLTag(cx, 0);
		m_chart_data_source.m_VTtags.AddLTag(cx, 0);
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
	m_printRect.right = options_view_data->horzRes - options_view_data->rightPageMargin;
	m_printRect.bottom = options_view_data->vertRes - options_view_data->bottomPageMargin;
	m_printRect.left = options_view_data->leftPageMargin;
	m_printRect.top = options_view_data->topPageMargin;
}

void ViewSpikeDetection::PrintFileBottomPage(CDC* p_dc, const CPrintInfo* p_info)
{
	auto t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"),
		p_info->m_nCurPage, p_info->GetMaxPage(),
		t.GetDay(), t.GetMonth(), t.GetYear());
	const auto ch_date = GetDocument()->GetDB_CurrentSpkFileName();
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_view_data->horzRes / 2, options_view_data->vertRes - 57, ch_date);
}

CString ViewSpikeDetection::PrintConvertFileIndex(const long l_first, const long l_last)
{
	CString cs_unit = _T(" s");
	CString cs_comment;
	float x_scale_factor;
	const auto x1 = m_chart_data_filtered.ChangeUnit(static_cast<float>(l_first) / m_samplingRate, &cs_unit,
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
	const auto total_rows = m_nbrowsperpage * (page - 1);
	l_first = m_lprintFirst;
	file_number = 0; 
	if (options_view_data->bPrintSelection) 
		file_number = m_file0;
	else
		GetDocument()->DBMoveFirst();

	auto very_last = m_lprintFirst + m_lprintLen;
	if (options_view_data->bEntireRecord)
		very_last = GetDocument()->GetDB_DataLen() - 1;

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
		if (file_index >= m_nfiles)
			return FALSE;

		GetDocument()->DBMoveNext();
		if (l_first < GetDocument()->GetDB_DataLen() - 1)
		{
			if (options_view_data->bEntireRecord)
				very_last = GetDocument()->GetDB_DataLen() - 1;
		}
	}
	else
	{
		l_first += m_lprintLen;
		if (l_first >= very_last)
		{
			file_index++; // next index
			if (file_index >= m_nfiles) // last file ??
				return FALSE;

			GetDocument()->DBMoveNext();
			very_last = GetDocument()->GetDB_DataLen() - 1;
			l_first = m_lprintFirst;
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
	const auto p_data_file = GetDocument()->m_pDat;
	const auto wave_format = p_data_file->GetpWaveFormat();
	if (options_view_data->bDocName || options_view_data->bAcqDateTime)
	{
		if (options_view_data->bDocName) 
			str_comment += GetDocument()->GetDB_CurrentDatFileName() + tab;

		if (options_view_data->bAcqDateTime) 
		{
			const auto date = (wave_format->acqtime).Format(_T("%#d %B %Y %X")); 
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_view_data->bAcqComment)
		str_comment += GetDocument()->Export_DatabaseData(); 

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
	auto i_horizontal_bar = pDataChartWnd->GetRectWidth() / 10; // initial horizontal bar length 1/10th of display rect
	auto i_vertical_bar = pDataChartWnd->GetRectHeight() / 3; // initial vertical bar height 1/3rd  of display rect

	auto str_comment = PrintConvertFileIndex(pDataChartWnd->GetDataFirst(), pDataChartWnd->GetDataLast());

	///// horizontal time bar ///////////////////////////
	if (options_view_data->bTimeScaleBar)
	{
		// convert bar size into time units and back into pixels
		cs_unit = _T(" s"); 
		const auto time_per_pixel = pDataChartWnd->GetTimeperPixel();
		const auto z = time_per_pixel * static_cast<float>(i_horizontal_bar); // convert 1/10 of the length of the data displayed into time
		float x_scale_factor;
		const auto x = pDataChartWnd->ChangeUnit(z, &cs_unit, &x_scale_factor); // convert time into a scaled time
		const auto k = pDataChartWnd->NiceUnit(x); // convert the (scaled) time value into time expressed as an integral
		i_horizontal_bar = static_cast<int>((static_cast<float>(k) * x_scale_factor) / time_per_pixel);
		// compute how much pixels it makes
		// print out the scale and units
		cs.Format(_T("horz bar = %i %s"), k, (LPCTSTR)cs_unit);
		str_comment += cs + rc;
		// draw horizontal line
		i_horizontal_bar = MulDiv(i_horizontal_bar, rect->Width(), pDataChartWnd->GetRectWidth());
		p_dc->MoveTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y);
		p_dc->LineTo(rect->left + bar_origin.x + i_horizontal_bar, rect->bottom - bar_origin.y);
	}

	///// vertical voltage bars ///////////////////////////
	float y_scale_factor; // compute a good unit for channel 0
	cs_unit = _T(" V"); 

	// convert bar size into voltage units and back into pixels
	const auto volts_per_pixel = pDataChartWnd->GetChanlistVoltsperPixel(0);
	const auto z_volts = volts_per_pixel * static_cast<float>(i_vertical_bar); // convert 1/3 of the height into voltage
	const auto z_scale = pDataChartWnd->ChangeUnit(z_volts, &cs_unit, &y_scale_factor);
	const auto z_nice = static_cast<float>(pDataChartWnd->NiceUnit(z_scale));
	i_vertical_bar = static_cast<int>(z_nice * y_scale_factor / volts_per_pixel); // compute how much pixels it makes

	if (options_view_data->bVoltageScaleBar)
	{
		i_vertical_bar = MulDiv(i_vertical_bar, rect->Height(), pDataChartWnd->GetRectHeight());
		p_dc->MoveTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y);
		p_dc->LineTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y - i_vertical_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_view_data->bChansComment || options_view_data->bVoltageScaleBar || options_view_data->bChanSettings)
	{
		const auto channel_list_size = pDataChartWnd->GetChanlistSize();
		for (auto channel_index = 0; channel_index < channel_list_size; channel_index++) // loop
		{
			CChanlistItem* channel_item = pDataChartWnd->GetChanlistItem(channel_index);
			if (!channel_item->GetflagPrintVisible())
				continue;

			cs.Format(_T("chan#%i "), channel_index); 
			str_comment += cs;
			if (options_view_data->bVoltageScaleBar) 
			{
				const auto z = static_cast<float>(i_vertical_bar) * pDataChartWnd->GetChanlistVoltsperPixel(channel_index);
				const auto x = z / y_scale_factor;
				const auto j = pDataChartWnd->NiceUnit(x);
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
				const auto wave_chan_array = GetDocument()->m_pDat->GetpWavechanArray();
				const auto p_chan = wave_chan_array->Get_p_channel(source_channel);
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
	if (options_view_data->bVoltageScaleBar && m_pSpkList->GetTotalSpikes() > 0)
	{
		// the following assume that spikes are higher than 1 mV...
		const CString cs_unit = _T("mV");
		z = m_chart_spike_shape.GetExtent_mV() / 2.0f; 
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
	if (m_pSpkList->GetTotalSpikes() > 0 && bAll)
	{
		z = m_chart_spike_shape.GetExtent_ms();
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
	k = m_pSpkList->GetTotalSpikes();
	CString dummy5;
	dummy5.Format( _T("n spk= %i"), k);
	str_comment += dummy5;
	str_comment += rc;

	return str_comment;
}

void ViewSpikeDetection::serialize_windows_state(const BOOL save, int tab_index)
{
	const auto p_dbWave_app = dynamic_cast<CdbWaveApp*>(AfxGetApp()); 
	if (tab_index < 0 || tab_index >= m_tabCtrl.GetItemCount())
	{
		int tab_selected = m_tabCtrl.GetCurSel(); 
		if (tab_selected < 0)
			tab_selected = 0;
		tab_index = tab_selected;
	}

	// adjust size of the array
	if (p_dbWave_app->viewspikesmemfile_ptr_array.GetSize() == 0)
		p_dbWave_app->viewspikesmemfile_ptr_array.SetSize(1);

	if (p_dbWave_app->viewspikesmemfile_ptr_array.GetSize() < m_tabCtrl.GetItemCount())
		p_dbWave_app->viewspikesmemfile_ptr_array.SetSize(m_tabCtrl.GetItemCount());
	CMemFile* p_mem_file = nullptr;
	if (p_dbWave_app->viewspikesmemfile_ptr_array.GetSize() > 0 && p_dbWave_app->viewspikesmemfile_ptr_array.GetSize() > tab_index)
		p_mem_file = p_dbWave_app->viewspikesmemfile_ptr_array.GetAt(tab_index);

	// save display parameters
	if (save)
	{
		if (p_mem_file == nullptr)
		{
			p_mem_file = new CMemFile;
			ASSERT(p_mem_file != NULL);
			p_dbWave_app->viewspikesmemfile_ptr_array.SetAt(tab_index, p_mem_file);
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
			*m_chart_data_source.GetScopeParameters() = options_view_data->viewspkdetectdata;
			*m_chart_data_filtered.GetScopeParameters() = options_view_data->viewspkdetectfiltered;
			*m_chart_spike_bar.GetScopeParameters() = options_view_data->viewspkdetectspk;
			*m_chart_spike_shape.GetScopeParameters() = options_view_data->viewspkdetectbars;
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
	m_nbrowsperpage = m_printRect.Height() / size_row;
	if (m_nbrowsperpage == 0)
		m_nbrowsperpage = 1;

	int n_total_rows;
	const auto p_document = GetDocument();

	// compute number of rows according to b_multi_row & b_entire_record flag
	m_lprintFirst = m_chart_data_filtered.GetDataFirst();
	m_lprintLen = m_chart_data_filtered.GetDataLast() - m_lprintFirst + 1;
	m_file0 = GetDocument()->GetDB_CurrentRecordPosition();
	ASSERT(m_file0 >= 0);
	m_nfiles = 1;
	auto i_file_0 = m_file0;
	auto i_file_1 = m_file0;
	if (!options_view_data->bPrintSelection)
	{
		i_file_0 = 0;
		m_nfiles = p_document->GetDB_NRecords();
		i_file_1 = m_nfiles;
	}

	// only one row per file
	if (!options_view_data->bMultirowDisplay || !options_view_data->bEntireRecord)
		n_total_rows = m_nfiles;

	// multiple rows per file
	else
	{
		n_total_rows = 0;
		p_document->SetDB_CurrentRecordPosition(i_file_0);
		for (auto i = i_file_0; i < i_file_1; i++, p_document->DBMoveNext())
		{
			// get size of document for all files
			auto len = p_document->GetDB_DataLen();
			if (len <= 0)
			{
				p_document->OpenCurrentDataFile();
				len = p_document->m_pDat->GetDOCchanLength();
				const auto len1 = GetDocument()->GetDB_DataLen() - 1;
				ASSERT(len == len1);
				p_document->SetDB_DataLen(len);
			}
			len -= m_lprintFirst;
			auto n_rows = len / m_lprintLen; 
			if (len > n_rows * m_lprintLen)
				n_rows++;
			n_total_rows += static_cast<int>(n_rows);
		}
	}

	if (m_file0 >= 0)
	{
		p_document->SetDB_CurrentRecordPosition(m_file0);
		p_document->OpenCurrentDataFile();
	}

	auto n_pages = n_total_rows / m_nbrowsperpage;
	if (n_total_rows > m_nbrowsperpage * n_pages)
		n_pages++;

	return n_pages;
}

void ViewSpikeDetection::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_chart_data_filtered.GetDataFirst();
	m_lLast0 = m_chart_data_filtered.GetDataLast();
	m_npixels0 = m_chart_data_filtered.GetRectWidth();
	PrintCreateFont();
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewSpikeDetection::PrintCreateFont()
{
	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial")); // Arial font
	m_logFont.lfHeight = options_view_data->fontsize; // font height
	m_pOldFont = nullptr;
	m_fontPrint.CreateFontIndirect(&m_logFont);
}

void ViewSpikeDetection::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_pOldFont = p_dc->SelectObject(&m_fontPrint);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	const auto r_width = options_view_data->WidthDoc; // page margins
	const auto r_height = options_view_data->HeightDoc; // page margins
	CRect r_where(m_printRect.left, // printing rectangle for one line of data
		m_printRect.top,
		m_printRect.left + r_width,
		m_printRect.top + r_height);
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
		m_chart_data_filtered.SetChanlistTransformMode(0, 0);

	p_dc->SetMapMode(MM_TEXT); // change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(p_dc, pInfo); // print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int file_index; 
	long index_first_data_point; 
	auto index_last_data_point = m_lprintFirst + m_lprintLen; 
	const auto current_page_number = pInfo->m_nCurPage;
	PrintGetFileSeriesIndexFromPage(current_page_number, file_index, index_first_data_point);
	if (index_first_data_point < GetDocument()->GetDB_DataLen() - 1)
		update_file_parameters(FALSE);
	if (options_view_data->bEntireRecord)
		index_last_data_point = GetDocument()->GetDB_DataLen() - 1;

	// loop through all files	--------------------------------------------------------
	for (auto i = 0; i < m_nbrowsperpage; i++)
	{
		const auto old_dc = p_dc->SaveDC(); // save DC

		// first : set rectangle where data will be printed
		auto comment_rect = r_where; 
		p_dc->SetMapMode(MM_TEXT); 
		p_dc->SetTextAlign(TA_LEFT); 

		// load data and adjust display rectangle ----------------------------------------
		// set data rectangle to half height to the row height
		m_rData = r_where;
		if (options_view_data->bPrintSpkBars)
			m_rData.bottom = m_rData.top + r_where.Height() / 2;
		m_rData.left += (r_sp_kwidth + options_view_data->textseparator);
		const auto old_size = m_rData.Width(); 

		// make sure enough data fit into this rectangle, otherwise clip rect
		auto l_last = index_first_data_point + m_lprintLen; 
		if (l_last > index_last_data_point) 
			l_last = index_last_data_point;
		if ((l_last - index_first_data_point + 1) < m_lprintLen) 
			m_rData.right = (old_size * (l_last - index_first_data_point)) / m_lprintLen + m_rData.left;
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// if option requested, clip output to rect
		if (options_view_data->bClipRect) // clip curve display
			p_dc->IntersectClipRect(&m_rData); // (eventually)

		// print detected channel only data
		m_chart_data_filtered.GetChanlistItem(0)->SetflagPrintVisible(chan0Drawmode);
		m_chart_data_filtered.ResizeChannels(m_rData.Width(), 0);
		m_chart_data_filtered.GetDataFromDoc(index_first_data_point, l_last);
		m_chart_data_filtered.Print(p_dc, &m_rData);
		p_dc->SelectClipRgn(nullptr);

		// print spike bars 
		if (options_view_data->bPrintSpkBars)
		{
			CRect BarsRect = r_where; 
			BarsRect.top = m_rData.bottom;
			BarsRect.left = m_rData.left;
			BarsRect.right = m_rData.right;

			m_chart_spike_bar.SetTimeIntervals(index_first_data_point, l_last);
			m_chart_spike_bar.Print(p_dc, &BarsRect);
		}

		// print spike shape within a square (same width as height) 
		m_rSpike = r_where; 
		m_rSpike.right = m_rSpike.left + r_sp_kwidth;
		m_rSpike.left += options_view_data->textseparator;
		m_rSpike.bottom = m_rSpike.top + r_sp_kheight; 

		m_chart_spike_shape.SetTimeIntervals(index_first_data_point, l_last);
		m_chart_spike_shape.Print(p_dc, &m_rSpike);

		// restore DC and print comments 
		p_dc->RestoreDC(old_dc); 
		p_dc->SetMapMode(MM_TEXT);
		p_dc->SelectClipRgn(nullptr); 
		p_dc->SetViewportOrg(0, 0);

		// print data Bars & get comments according to row within file
		CString cs_comment;
		const BOOL b_all = (index_first_data_point == m_lprintFirst);
		if (b_all) 
		{
			cs_comment += PrintGetFileInfos();
			cs_comment += PrintDataBars(p_dc, &m_chart_data_filtered, &m_rData);
		}
		else
		{
			// other rows: time intervals only
			cs_comment = PrintConvertFileIndex(m_chart_data_filtered.GetDataFirst(),
				m_chart_data_filtered.GetDataLast());
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_view_data->textseparator + comment_rect.Width(), 0);
		comment_rect.right = m_printRect.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		constexpr UINT format_parameters = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect, format_parameters);

		// print comments & bar / spike shape
		cs_comment.Empty();
		m_rSpike.right = m_rSpike.left + r_sp_kheight;
		cs_comment = PrintSpkShapeBars(p_dc, &m_rSpike, b_all);
		m_rSpike.right = m_rSpike.left + r_sp_kwidth;
		m_rSpike.left -= options_view_data->textseparator;
		m_rSpike.top = m_rSpike.bottom;
		m_rSpike.bottom += m_logFont.lfHeight * 3;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), m_rSpike, format_parameters);
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
			m_chart_data_filtered.GetChanlistItem(0)->SetflagPrintVisible(0); // cancel printing channel zero
		}
	}

	// end of file loop : restore initial conditions
	m_chart_data_filtered.GetChanlistItem(0)->SetflagPrintVisible(1);
	if (!options_view_data->bFilterDataSource)
		m_chart_data_filtered.SetChanlistTransformMode(0, m_p_detect_parameters->detectTransform);

	if (m_pOldFont != nullptr)
		p_dc->SelectObject(m_pOldFont);
}

void ViewSpikeDetection::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	// restore file from index and display parameters
	GetDocument()->SetDB_CurrentRecordPosition(m_file0);

	m_chart_data_filtered.ResizeChannels(m_npixels0, 0);
	m_chart_data_filtered.GetDataFromDoc(m_lFirst0, m_lLast0);
	m_chart_spike_shape.SetTimeIntervals(m_lFirst0, m_lLast0);
	update_file_parameters(TRUE);

	m_bIsPrinting = FALSE;
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
			MulDiv(m_chart_data_filtered.GetChanlistItem(m_ichanselected)->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
	else
		m_scrolly2.SetScrollPos(
			MulDiv(m_chart_data_filtered.GetChanlistItem(m_ichanselected2)->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
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
	int y_extent = p_view_data_filtered->GetChanlistItem(selected_channel)->GetYextent();

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
		p_view_data_filtered->GetChanlistItem(selected_channel)->SetYextent(y_extent);
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
		const CChanlistItem* channel_item = m_chart_data_filtered.GetChanlistItem(m_ichanselected);
		const auto i_pos = static_cast<int>((channel_item->GetYzero() - channel_item->GetDataBinZero())
			* 100 / static_cast<int>(YZERO_SPAN)) + static_cast<int>(50);
		m_scrolly.SetScrollPos(i_pos, TRUE);
	}
	else
	{
		const CChanlistItem* channel_item = m_chart_data_filtered.GetChanlistItem(m_ichanselected2);
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

	const CChanlistItem* channel_item = m_chart_data_filtered.GetChanlistItem(selected_channel_index);
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
		CChanlistItem* chan = p_view->GetChanlistItem(selected_channel_index);
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
		auto y = m_chart_spike_shape.GetExtent_mV();
		const auto y_old = y;
		CString cs;
		switch (mm_spkWndAmplitude.m_nChar)
		{
		case VK_RETURN:
			mm_spkWndAmplitude.GetWindowText(cs);
			y = static_cast<float>(_ttof(cs));
			break;
		case VK_UP:
		case VK_PRIOR:
			y++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			y--;
			break;
		default:;
		}

		// compute new extent and change the display
		if (y <= 0)
		{
			y = y_old;
			MessageBeep(-1);
		}
		const auto y_we = static_cast<int>(static_cast<float>(m_chart_spike_shape.GetYWExtent()) * y / y_old);
		m_chart_spike_shape.SetYWExtOrg(y_we, m_chart_spike_shape.GetYWOrg());
		m_chart_spike_shape.SetyScaleUnitValue(y);
		m_chart_spike_shape.Invalidate();

		// update the dialog control
		mm_spkWndAmplitude.m_bEntryDone = FALSE;
		mm_spkWndAmplitude.m_nChar = 0;
		mm_spkWndAmplitude.SetSel(0, -1);
		cs.Format(_T("%.3f"), y);
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->SetWindowText(cs);
	}
}

void ViewSpikeDetection::OnEnChangeSpkWndLength()
{
	if (mm_spkWndDuration.m_bEntryDone)
	{
		auto x = m_chart_spike_shape.GetExtent_ms();
		const auto xold = x;
		CString cs;
		switch (mm_spkWndDuration.m_nChar)
		{
		case VK_RETURN:
			mm_spkWndDuration.GetWindowText(cs);
			x = static_cast<float>(_ttof(cs));
			break;
		case VK_UP:
		case VK_PRIOR:
			x++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			x--;
			break;
		default:;
		}

		// compute new extent and change the display
		if (x <= 0)
		{
			MessageBeep(-1);
			x = xold;
		}
		const auto x_we = static_cast<int>(static_cast<float>(m_chart_spike_shape.GetXWExtent()) * x / xold);
		m_chart_spike_shape.SetXWExtOrg(x_we, m_chart_spike_shape.GetXWOrg());
		m_chart_spike_shape.SetxScaleUnitValue(x);
		m_chart_spike_shape.Invalidate();

		// update the dialog control
		mm_spkWndDuration.m_bEntryDone = FALSE;
		mm_spkWndDuration.m_nChar = 0;
		mm_spkWndDuration.SetSel(0, -1); //select all text
		cs.Format(_T("%.3f"), x);
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->SetWindowText(cs);
	}
}

void ViewSpikeDetection::OnBnClickedLocatebttn()
{
	int max, min;
	const CChanlistItem* channel_item = m_chart_data_filtered.GetChanlistItem(0);
	channel_item->GetMaxMin(&max, &min);

	// modify value
	m_p_detect_parameters->detectThreshold = (max + min) / 2;
	m_thresholdval = channel_item->ConvertDataBinsToVolts(m_p_detect_parameters->detectThreshold) * 1000.f;
	m_p_detect_parameters->detectThresholdmV = m_thresholdval;
	// update user-interface: edit control and threshold bar in sourceview
	CString cs;
	cs.Format(_T("%.3f"), m_thresholdval);
	GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
	m_chart_data_filtered.MoveHZtagtoVal(0, m_p_detect_parameters->detectThreshold);
	m_chart_data_filtered.Invalidate();
}

void ViewSpikeDetection::update_detection_settings(int iSelParms)
{
	// check size of spike detection parameters
	if (iSelParms >= m_spk_detect_array_current.GetSize())
	{
		// load new set of parameters from spike list
		const auto spike_list_size = m_pSpkDoc->GetSpkList_Size();
		m_spk_detect_array_current.SetSize(spike_list_size);
		for (int i = 0; i < spike_list_size; i++)
		{
			// select new spike list (list with no spikes for stimulus channel)
			const auto spike_list_current = m_pSpkDoc->SetSpkList_AsCurrent(i);
			ASSERT(spike_list_current != NULL);
			const auto ps_d = spike_list_current->GetDetectParms();
			m_spk_detect_array_current.SetItem(i, ps_d); 
		}
	}

	// loop over each spike detection set to update spike detection parameters
	for (auto i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		// select new spike list (list with no spikes for stimulus channel)
		auto spike_list_current = m_pSpkDoc->SetSpkList_AsCurrent(i);
		const auto p_sd = m_spk_detect_array_current.GetItem(i);
		if (spike_list_current == nullptr)
		{
			m_pSpkDoc->AddSpkList();
			spike_list_current = m_pSpkDoc->GetSpkList_Current();
			spike_list_current->InitSpikeList(GetDocument()->m_pDat, p_sd);
		}
		else
			spike_list_current->SetDetectParms(p_sd);
	}

	// set new parameters
	m_pSpkList->m_selected_spike = m_spikeno; 
	m_i_detect_parameters = iSelParms;
	m_p_detect_parameters = m_spk_detect_array_current.GetItem(iSelParms);
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(iSelParms);
	if (m_pSpkList != nullptr)
		highlight_spikes(TRUE);

	update_detection_controls();
	update_legends();
}

void ViewSpikeDetection::update_detection_controls()
{
	const SPKDETECTPARM* detect_parameters = m_pSpkList->GetDetectParms();
	m_CBdetectWhat.SetCurSel(detect_parameters->detectWhat);
	m_CBdetectChan.SetCurSel(detect_parameters->detectChan);
	m_CBtransform.SetCurSel(detect_parameters->detectTransform);

	// check that spike detection parameters are compatible with current data doc
	auto detection_channel = detect_parameters->detectChan;
	const auto p_dat = GetDocument()->m_pDat;
	const int scan_count = p_dat->GetpWaveFormat()->scan_count;
	if (detection_channel >= scan_count)
		detection_channel = 0;

	m_chart_data_filtered.SetChanlistOrdinates(0, detection_channel, detect_parameters->detectTransform);
	m_CBtransform2.SetCurSel(m_pSpkList->GetDetectParms()->extractTransform);

	CChanlistItem* channel_item = m_chart_data_filtered.GetChanlistItem(0);
	channel_item->SetColor(static_cast<WORD>(detection_channel));

	m_chart_data_filtered.GetDataFromDoc(); // load data
	//if (options_view_data->bSplitCurves)
	//	m_displayDataFile.SplitChans();

	const auto detect_threshold = detect_parameters->detectThreshold;
	m_thresholdval = channel_item->ConvertDataBinsToVolts(detect_threshold) * 1000.f;
	if (m_chart_data_filtered.m_HZtags.GetNTags() < 1)
		m_chart_data_filtered.m_HZtags.AddTag(detect_threshold, 0);
	else
		m_chart_data_filtered.m_HZtags.SetTagVal(0, detect_threshold);

	// update spike channel displayed
	m_chart_spike_bar.SetSpkList(m_pSpkList);
	m_chart_spike_shape.SetSpkList(m_pSpkList);
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
	m_pSpkDoc->SortStimArray();

	DlgEditStimArray dlg;
	dlg.intervals = m_pSpkDoc->m_stimulus_intervals;
	dlg.m_sampling_rate = m_samplingRate;
	dlg.intervals_saved = GetDocument()->m_stimsaved;
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
		switch (mm_ichanselected.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR:
			m_ichanselected++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_ichanselected--;
			break;
		default:;
		}
		SetDlgItemInt(IDC_CHANSELECTED, m_ichanselected);
	}
}

void ViewSpikeDetection::OnEnChangeChanselected2()
{
	if (mm_ichanselected2.m_bEntryDone)
	{
		switch (mm_ichanselected2.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_ichanselected2++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_ichanselected2--;
			break;
		default:;
		}
		SetDlgItemInt(IDC_CHANSELECTED2, m_ichanselected2);
	}
}

void ViewSpikeDetection::OnCbnSelchangeTransform2()
{
	const auto method = m_CBtransform2.GetCurSel(); 
	const auto data_document = GetDocument()->m_pDat;

	const auto detect_parameters = m_pSpkList->GetDetectParms();
	const auto doc_chan = detect_parameters->extractChan;
	const short span = data_document->GetTransfDataSpan(method); 
	detect_parameters->extractTransform = method; 

	// pre-load data
	const auto spike_length = m_pSpkList->GetSpikeLength();
	const auto spike_pre_threshold = detect_parameters->prethreshold;
	auto ii_time = m_pSpkList->GetSpike(0)->get_time() - spike_pre_threshold;
	auto l_rw_first0 = ii_time - spike_length;
	auto l_rw_last0 = ii_time + spike_length;
	if (!data_document->LoadRawData(&l_rw_first0, &l_rw_last0, span))
		return; 
	auto p_data = data_document->LoadTransfData(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto total_spikes = m_pSpkList->GetTotalSpikes();
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		constexpr short offset = 1;
		Spike* p_spike = m_pSpkList->GetSpike(i_spike);
		// make sure that source data are loaded and get pointer to it (p_data)
		ii_time = p_spike->get_time();
		auto l_rw_first = ii_time - spike_pre_threshold; 
		auto l_rw_last = l_rw_first + spike_length; 
		if (!data_document->LoadRawData(&l_rw_first, &l_rw_last, span))
			break; 

		p_data = data_document->LoadTransfData(l_rw_first, l_rw_last, method, doc_chan);
		const auto p_data_spike0 = p_data + (ii_time - spike_pre_threshold - l_rw_first) * offset;
		p_spike->TransferDataToSpikeBuffer(p_data_spike0, offset);

		// n channels should be 1 if they come from the transform buffer as data are not interleaved...
		p_spike->CenterSpikeAmplitude(0, spike_length, 1); // 1=center average
	}
	m_pSpkDoc->SetModifiedFlag(TRUE);

	int max, min;
	m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	m_chart_spike_shape.SetYWExtOrg(m_chart_spike_shape.GetYWExtent(), middle);
	m_chart_spike_bar.SetYWExtOrg(m_chart_spike_shape.GetYWExtent(), middle);
	update_spike_shape_window_scale(FALSE);

	highlight_spikes(TRUE);
	update_legends();
	update_tabs();
}

void ViewSpikeDetection::update_tabs()
{
	// load initial data
	const BOOL b_replace = (m_tabCtrl.GetItemCount() == m_pSpkDoc->GetSpkList_Size());
	if (!b_replace)
		m_tabCtrl.DeleteAllItems();

	// load list of detection parameters
	const auto current_spike_list_index = m_pSpkDoc->GetSpkList_CurrentIndex();
	for (auto i = 0; i < m_pSpkDoc->GetSpkList_Size(); i++)
	{
		CString cs;
		const auto current_spike_list = m_pSpkDoc->SetSpkList_AsCurrent(i);
		cs.Format(_T("#%i %s"), i, (LPCTSTR)current_spike_list->GetDetectParms()->comment);
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
	m_pSpkDoc->SetSpkList_AsCurrent(current_spike_list_index);

	m_i_detect_parameters = m_pSpkDoc->GetSpkList_CurrentIndex();
	m_tabCtrl.SetCurSel(m_i_detect_parameters);
}
