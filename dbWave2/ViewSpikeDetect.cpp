// TODO
// convert threshold into volts & back to binary (cope with variable gains)
// cf: UpdateFileParameters

#include "StdAfx.h"
#include "dbWave.h"
#include "resource.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "MainFrm.h"
#include "DlgEditStimArray.h"
#include "ViewSpikeDetect.h"

#include "DlgCopyAs.h"
#include "DlgDataSeries.h"
#include "DlgProgress.h"
#include "DlgSpikeDetect.h"
#include "DlgSpikeEdit.h"
#include "DlgXYParameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto b_restore = 0;
constexpr auto b_save = 1;

IMPLEMENT_DYNCREATE(CViewSpikeDetection, CdbTableView)

CViewSpikeDetection::CViewSpikeDetection()
	: CdbTableView(IDD)
{
	m_bEnableActiveAccessibility = FALSE;
}

CViewSpikeDetection::~CViewSpikeDetection()
{
	if (m_pSpkDoc != nullptr) {
		saveCurrentSpkFile();
	}
	// save spike detection parameters
	const auto p_array = m_pArrayFromApp->GetChanArray(m_scancount_doc);
	*p_array = m_spk_detect_array_current;
}

void CViewSpikeDetection::DoDataExchange(CDataExchange* pDX)
{
	CdbTableView::DoDataExchange(pDX);

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

BEGIN_MESSAGE_MAP(CViewSpikeDetection, CdbTableView)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()

	ON_MESSAGE(WM_MYMESSAGE, &CViewSpikeDetection::OnMyMessage)

	ON_COMMAND(ID_FORMAT_FIRSTFRAME, &CViewSpikeDetection::OnFirstFrame)
	ON_COMMAND(ID_FORMAT_LASTFRAME, &CViewSpikeDetection::OnLastFrame)
	ON_COMMAND(ID_FORMAT_SETORDINATES, &CViewSpikeDetection::OnFormatXscale)
	ON_COMMAND(ID_FORMAT_ALLDATA, &CViewSpikeDetection::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &CViewSpikeDetection::OnFormatYscaleCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &CViewSpikeDetection::OnFormatYscaleGainadjust)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, &CViewSpikeDetection::OnFormatSplitcurves)
	ON_COMMAND(ID_TOOLS_DETECT_PARMS, &CViewSpikeDetection::OnToolsDetectionparameters)
	ON_COMMAND(ID_TOOLS_DETECT, &CViewSpikeDetection::OnMeasureAll)
	ON_COMMAND(ID_TOOLS_EDITSTIMULUS, &CViewSpikeDetection::OnToolsEditstimulus)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, &CViewSpikeDetection::OnToolsEdittransformspikes)
	ON_COMMAND(ID_TOOLS_DATASERIES, &CViewSpikeDetection::OnToolsDataseries)
	ON_COMMAND(ID_EDIT_COPY, &CViewSpikeDetection::OnEditCopy)
	ON_COMMAND(ID_FILE_SAVE, &CViewSpikeDetection::OnFileSave)
	ON_COMMAND(ID_FORMAT_XSCALE, &CViewSpikeDetection::OnFormatXscale)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_CBN_SELCHANGE(IDC_SOURCECHAN, &CViewSpikeDetection::OnSelchangeDetectchan)
	ON_CBN_SELCHANGE(IDC_TRANSFORM, &CViewSpikeDetection::OnSelchangeTransform)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CViewSpikeDetection::OnSelchangeDetectMode)
	ON_CBN_SELCHANGE(IDC_TRANSFORM2, &CViewSpikeDetection::OnCbnSelchangeTransform2)

	ON_EN_CHANGE(IDC_THRESHOLDVAL, &CViewSpikeDetection::OnEnChangeThresholdval)
	ON_EN_CHANGE(IDC_TIMEFIRST, &CViewSpikeDetection::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &CViewSpikeDetection::OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_SPIKENO, &CViewSpikeDetection::OnEnChangeSpikeno)
	ON_EN_CHANGE(IDC_SPIKEWINDOWAMPLITUDE, &CViewSpikeDetection::OnEnChangeSpkWndAmplitude)
	ON_EN_CHANGE(IDC_SPIKEWINDOWLENGTH, &CViewSpikeDetection::OnEnChangeSpkWndLength)
	ON_EN_CHANGE(IDC_CHANSELECTED, &CViewSpikeDetection::OnEnChangeChanselected)
	ON_EN_CHANGE(IDC_CHANSELECTED2, &CViewSpikeDetection::OnEnChangeChanselected2)

	ON_BN_CLICKED(IDC_MEASUREALL, &CViewSpikeDetection::OnMeasureAll)
	ON_BN_CLICKED(IDC_CLEAR, &CViewSpikeDetection::OnClear)
	ON_BN_CLICKED(IDC_ARTEFACT, &CViewSpikeDetection::OnArtefact)
	ON_BN_CLICKED(IDC_BIAS, &CViewSpikeDetection::OnBnClickedBiasbutton)
	ON_BN_CLICKED(IDC_GAIN, &CViewSpikeDetection::OnBnClickedGainbutton)
	ON_BN_CLICKED(IDC_LOCATEBTTN, &CViewSpikeDetection::OnBnClickedLocatebttn)
	ON_BN_CLICKED(IDC_CLEARALL, &CViewSpikeDetection::OnBnClickedClearall)
	ON_BN_CLICKED(IDC_MEASURE, &CViewSpikeDetection::OnMeasure)
	ON_BN_CLICKED(IDC_GAIN2, &CViewSpikeDetection::OnBnClickedGain2)
	ON_BN_CLICKED(IDC_BIAS2, &CViewSpikeDetection::OnBnClickedBias2)

	//ON_NOTIFY(NM_CLICK, IDC_TAB1,		&CdbTableView::OnNMClickTab1)
END_MESSAGE_MAP()

void CViewSpikeDetection::OnFileSave()
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

BOOL CViewSpikeDetection::OnMove(UINT nIDMoveCommand)
{
	saveCurrentSpkFile();
	return CdbTableView::OnMove(nIDMoveCommand);
}

void CViewSpikeDetection::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
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

void CViewSpikeDetection::OnActivateView(BOOL activate, CView* activated_view, CView* de_activated_view)
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
		p_app->options_viewdata.viewdata = *(m_ChartDataWnd_Source.GetScopeParameters());
	}
	CdbTableView::OnActivateView(activate, activated_view, de_activated_view);
}

void CViewSpikeDetection::update_legends()
{
	const auto l_first = m_ChartDataWnd_Source.GetDataFirst();
	const auto l_last = m_ChartDataWnd_Source.GetDataLast();
	m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);

	// draw charts
	m_ChartSpkWnd_Bar.SetTimeIntervals(l_first, l_last);
	m_ChartSpkWnd_Shape.SetTimeIntervals(l_first, l_last);
	update_spike_shape_window_scale(FALSE);

	// update text abscissa and horizontal scroll position
	m_timefirst = static_cast<float>(l_first) / m_samplingRate;
	m_timelast = static_cast<float>(l_last + 1) / m_samplingRate;
	m_spikeno = m_pSpkList->m_selected_spike;
	m_bartefact = FALSE;
	if (m_spikeno > 0)
	{
		const auto p_s = m_pSpkList->GetSpikeElemt(m_spikeno);
		m_bartefact = (p_s->get_class() < 0);
	}

	update_file_scroll();
	update_combo_box();
	update_VT_tags();
	update_legend_detection_wnd();

	// update data displayed
	m_ChartSpkWnd_Bar.Invalidate();
	m_ChartDataWnd_Detect.Invalidate();
	m_ChartDataWnd_Source.Invalidate();
	m_ChartSpkWnd_Shape.Invalidate();
	UpdateData(FALSE);

	// update number of spikes
	const auto n_spikes = static_cast<UINT>(m_pSpkList->GetTotalSpikes());
	if (n_spikes != GetDlgItemInt(IDC_NBSPIKES_NB))
	{
		SetDlgItemInt(IDC_NBSPIKES_NB, n_spikes);
		GetDlgItem(IDC_NBSPIKES_NB)->Invalidate();
	}
}

void CViewSpikeDetection::update_spike_file(BOOL bUpdateInterface)
{
	// update spike doc and temporary spike list
	const auto pdb_doc = GetDocument();
	//CString filename = pdb_doc->GetDB_CurrentSpkFileName();
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
	{
		// at least one spike list is available, select list[0]
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(0);
	}

	// no spikes list available, create one
	if (m_pSpkList == nullptr)
	{
		// create new list here
		ASSERT(m_pSpkDoc->GetSpkList_Size() == 0);
		const auto i_size = m_spk_detect_array_current.GetSize();
		m_pSpkDoc->SetSpkList_Size(i_size);
		for (auto i = 0; i < i_size; i++)
		{
			auto p_l = m_pSpkDoc->SetSpkList_AsCurrent(i);
			if (p_l == nullptr)
			{
				m_pSpkDoc->AddSpkList();
				p_l = m_pSpkDoc->GetSpkList_Current();
			}
			p_l->InitSpikeList(pdb_doc->m_pDat, m_spk_detect_array_current.GetItem(i));
		}
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(0);
		ASSERT(m_pSpkList != nullptr);
	}

	m_ChartSpkWnd_Bar.SetSourceData(m_pSpkList, pdb_doc);
	m_ChartSpkWnd_Bar.SetPlotMode(PLOT_BLACK, 0);
	m_ChartSpkWnd_Shape.SetSourceData(m_pSpkList, pdb_doc);
	m_ChartSpkWnd_Shape.SetPlotMode(PLOT_BLACK, 0);
	update_VT_tags();

	// update interface elements
	m_spikeno = -1;
	if (bUpdateInterface)
	{
		UpdateTabs();
		update_detection_controls();
		highlight_spikes(TRUE);
	}

	m_ChartSpkWnd_Bar.Invalidate();
	m_ChartSpkWnd_Shape.Invalidate();

	// update nb spikes
	const auto n_spikes = static_cast<UINT>(m_pSpkList->GetTotalSpikes());
	if (n_spikes != GetDlgItemInt(IDC_NBSPIKES_NB))
		SetDlgItemInt(IDC_NBSPIKES_NB, n_spikes);
}

void CViewSpikeDetection::highlight_spikes(BOOL flag)
{
	if (flag && m_pSpkList != nullptr && m_pSpkList->GetTotalSpikes() < 1)
	{
		CDWordArray* p_intervals = &m_DWintervals;

		// init array
		const auto array_size = m_pSpkList->GetTotalSpikes() * 2 + 3;
		m_DWintervals.SetSize(array_size);
		m_DWintervals.SetAt(0, 0);
		m_DWintervals.SetAt(1, static_cast<DWORD>(RGB(255, 0, 0))); // red 
		m_DWintervals.SetAt(2, 1);
		const auto max = m_pSpkList->GetTotalSpikes();
		auto j_index = 3;
		auto spike_length = m_pSpkList->GetSpikeLength();
		const auto spike_pre_trigger = m_pSpkList->GetSpikePretrig();
		spike_length--;

		for (auto i = 0; i < max; i++)
		{
			const auto p_s = m_pSpkList->GetSpikeElemt(i);
			const auto l_first = p_s->get_time() - spike_pre_trigger;
			m_DWintervals.SetAt(j_index, l_first);
			j_index++;
			m_DWintervals.SetAt(j_index, l_first + spike_length);
			j_index++;
		}

		// tell source_view to highlight spk
		m_ChartDataWnd_Detect.SetHighlightData(p_intervals);
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.SetHighlightData(p_intervals);
		m_ChartDataWnd_Source.Invalidate();
	}
}

void CViewSpikeDetection::update_file_parameters(BOOL bUpdateInterface)
{
	update_data_file(bUpdateInterface);
	update_spike_file(bUpdateInterface);
	if (bUpdateInterface)
		update_legends();
}

BOOL CViewSpikeDetection::check_detection_settings()
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

void CViewSpikeDetection::update_data_file(BOOL bUpdateInterface)
{
	const auto pdb_doc = GetDocument();
	//CString filename = pdb_doc->GetDB_CurrentDatFileName();
	if (pdb_doc->OpenCurrentDataFile() == nullptr)
		return;

	const auto p_data_file = pdb_doc->m_pDat;
	p_data_file->ReadDataInfos();
	const auto wave_format = p_data_file->GetpWaveFormat();

	// if the number of data channels of the data source has changed, load a new set of parameters
	// keep one array of spike detection parameters per data acquisition configuration (ie nb of acquisition channels)
	if (m_scancount_doc != wave_format->scan_count)
	{
		// save current set of parameters if scan count >= 0 this might not be necessary
		const auto channel_array = m_pArrayFromApp->GetChanArray(m_scancount_doc);
		if (m_scancount_doc >= 0)
			*channel_array = m_spk_detect_array_current;
		// Get parameters from the application array
		m_scancount_doc = wave_format->scan_count;
		m_spk_detect_array_current = *(m_pArrayFromApp->GetChanArray(m_scancount_doc));
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
	m_ChartDataWnd_Detect.AttachDataFile(p_data_file);
	m_ChartDataWnd_Source.AttachDataFile(p_data_file);

	// update source view display
	if (m_ChartDataWnd_Detect.GetChanlistSize() < 1)
	{
		m_ChartDataWnd_Detect.RemoveAllChanlistItems();
		m_ChartDataWnd_Detect.AddChanlistItem(0, 0);
		CChanlistItem* channel_item = m_ChartDataWnd_Detect.GetChanlistItem(0);
		channel_item->SetColor(0);
		m_ChartDataWnd_Detect.m_HZtags.RemoveAllTags();
		m_p_detect_parameters->detectThreshold = channel_item->ConvertVoltsToDataBins(m_p_detect_parameters->detectThresholdmV / 1000.f);
		m_ChartDataWnd_Detect.m_HZtags.AddTag(m_p_detect_parameters->detectThreshold, 0);
	}

	//add all channels to detection window
	auto channel_list_size = m_ChartDataWnd_Source.GetChanlistSize();
	const int n_document_channels = wave_format->scan_count;
	for (auto i = 0; i < n_document_channels; i++)
	{
		// check if present in the list
		auto b_present = FALSE;
		for (auto j = channel_list_size - 1; j >= 0; j--)
		{
			// test if this data chan is present + no transformation
			const CChanlistItem* channel_item = m_ChartDataWnd_Source.GetChanlistItem(j);
			if (channel_item->GetSourceChan() == i
				&& channel_item->GetTransformMode() == 0)
			{
				b_present = TRUE;
				break;
			}
		}
		if (b_present == FALSE) // no display chan contains that doc chan
		{
			m_ChartDataWnd_Source.AddChanlistItem(i, 0);
			channel_list_size++;
		}
		m_ChartDataWnd_Source.GetChanlistItem(i)->SetColor(static_cast<WORD>(i));
	}

	// if browse through another file ; keep previous display parameters & load data
	auto l_first = m_ChartDataWnd_Detect.GetDataFirst();
	auto l_last = m_ChartDataWnd_Detect.GetDataLast();
	if (options_view_data->bEntireRecord && bUpdateInterface)
	{
		l_first = 0;
		l_last = p_data_file->GetDOCchanLength() - 1;
	}
	m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);
	m_ChartDataWnd_Source.GetDataFromDoc(l_first, l_last);

	if (bUpdateInterface)
	{
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.Invalidate();
		// adjust scroll bar (size of button and left/right limits)
		m_filescroll_infos.fMask = SIF_ALL;
		m_filescroll_infos.nMin = 0;
		m_filescroll_infos.nMax = m_ChartDataWnd_Detect.GetDataLast();
		m_filescroll_infos.nPos = 0;
		m_filescroll_infos.nPage = m_ChartDataWnd_Detect.GetDataLast() - m_ChartDataWnd_Detect.GetDataFirst() + 1;
		m_filescroll.SetScrollInfo(&m_filescroll_infos);

		m_datacomments = wave_format->GetComments(_T(" "));
		m_samplingRate = wave_format->sampling_rate_per_channel;
		m_bValidThreshold = FALSE;
	}
}

void CViewSpikeDetection::update_combos_detect_and_transforms()
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

void CViewSpikeDetection::define_stretch_parameters()
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

void CViewSpikeDetection::define_sub_classed_items()
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
	VERIFY(m_ChartSpkWnd_Shape.SubclassDlgItem(IDC_DISPLAYSPIKES, this));
	VERIFY(m_ChartSpkWnd_Bar.SubclassDlgItem(IDC_DISPLAYBARS, this));
	VERIFY(m_ChartDataWnd_Detect.SubclassDlgItem(IDC_DISPLAYDETECT, this));
	VERIFY(m_ChartDataWnd_Source.SubclassDlgItem(IDC_DISPLAYDATA, this));

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

void CViewSpikeDetection::OnInitialUpdate()
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

	CdbTableView::OnInitialUpdate();

	// load file data
	if (m_ChartDataWnd_Detect.m_HZtags.GetNTags() < 1)
		m_ChartDataWnd_Detect.m_HZtags.AddTag(0, 0);

	update_file_parameters(TRUE);
	m_ChartDataWnd_Detect.SetScopeParameters(&(options_view_data->viewdata));
	m_ChartDataWnd_Detect.Invalidate();
	m_ChartDataWnd_Source.SetScopeParameters(&(options_view_data->viewdata));
	m_ChartDataWnd_Source.Invalidate();
}

LRESULT CViewSpikeDetection::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	int threshold = LOWORD(lParam); // value associated
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
		m_p_detect_parameters->detectThreshold = m_ChartDataWnd_Detect.m_HZtags.GetValue(threshold);
		m_thresholdval = m_ChartDataWnd_Detect.GetChanlistItem(0)
			->ConvertDataBinsToVolts(
				m_ChartDataWnd_Detect.m_HZtags.GetValue(threshold)) * 1000.f;
		m_p_detect_parameters->detectThresholdmV = m_thresholdval;
		mm_thresholdval.m_bEntryDone = TRUE;
		OnEnChangeThresholdval();
		break;

		// ----------------------------- select bar/display bars or zoom
	case HINT_CHANGEHZLIMITS: 
		m_ChartDataWnd_Detect.GetDataFromDoc(m_ChartSpkWnd_Bar.GetTimeFirst(), m_ChartSpkWnd_Bar.GetTimeLast());
		m_ChartDataWnd_Source.GetDataFromDoc(m_ChartSpkWnd_Bar.GetTimeFirst(), m_ChartSpkWnd_Bar.GetTimeLast());
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
		m_ChartSpkWnd_Shape.Invalidate();
		break;

	case HINT_VIEWSIZECHANGED:
		if (i_id == m_ChartDataWnd_Source.GetDlgCtrlID())
		{
			const auto l_first = m_ChartDataWnd_Source.GetDataFirst(); // get source data time range
			const auto l_last = m_ChartDataWnd_Source.GetDataLast();
			m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);
		}
		// else if(iID == m_displayDetect.GetDlgCtrlID())
		// UpdateLegends updates data window from m_displayDetect
		update_legends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_view_data->viewspkdetectfiltered = *m_ChartDataWnd_Detect.GetScopeParameters();
		options_view_data->viewspkdetectdata = *m_ChartDataWnd_Source.GetScopeParameters();
		options_view_data->viewspkdetectspk = *m_ChartSpkWnd_Bar.GetScopeParameters();
		options_view_data->viewspkdetectbars = *m_ChartSpkWnd_Shape.GetScopeParameters();
		break;

	case HINT_DEFINEDRECT:
		if (m_cursor_state == CURSOR_CROSS)
		{
			const auto rect = m_ChartDataWnd_Detect.GetDefinedRect();
			int l_limit_left = m_ChartDataWnd_Detect.GetDataOffsetfromPixel(rect.left);
			int l_limit_right = m_ChartDataWnd_Detect.GetDataOffsetfromPixel(rect.right);
			if (l_limit_left > l_limit_right)
			{
				const int i = l_limit_right;
				l_limit_right = l_limit_left;
				l_limit_left = i;
			}
			m_pSpkDoc->m_stimIntervals.SetAtGrow(m_pSpkDoc->m_stimIntervals.n_items, l_limit_left);
			m_pSpkDoc->m_stimIntervals.n_items++;
			m_pSpkDoc->m_stimIntervals.SetAtGrow(m_pSpkDoc->m_stimIntervals.n_items, l_limit_right);
			m_pSpkDoc->m_stimIntervals.n_items++;
			update_VT_tags();

			m_ChartSpkWnd_Bar.Invalidate();
			m_ChartDataWnd_Detect.Invalidate();
			m_ChartDataWnd_Source.Invalidate();
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
		int lvalue = m_pSpkDoc->m_stimIntervals.GetAt(threshold);
		if (i_id == m_ChartDataWnd_Detect.GetDlgCtrlID())
			lvalue = m_ChartDataWnd_Detect.m_VTtags.GetTagLVal(threshold);
		else if (i_id == m_ChartDataWnd_Source.GetDlgCtrlID())
			lvalue = m_ChartDataWnd_Source.m_VTtags.GetTagLVal(threshold);

		m_pSpkDoc->m_stimIntervals.SetAt(threshold, lvalue);
		update_VT_tags();

		m_ChartSpkWnd_Bar.Invalidate();
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.Invalidate();
		m_pSpkDoc->SetModifiedFlag(TRUE);
	}
	break;

	case WM_LBUTTONDOWN:
	case HINT_LMOUSEBUTTONDOW_CTRL:
	{
		const int cx = LOWORD(lParam);
		const int l_limit_left = m_ChartDataWnd_Detect.GetDataOffsetfromPixel(cx);
		m_pSpkDoc->m_stimIntervals.SetAtGrow(m_pSpkDoc->m_stimIntervals.n_items, l_limit_left);
		m_pSpkDoc->m_stimIntervals.n_items++;
		update_VT_tags();

		m_ChartSpkWnd_Bar.Invalidate();
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.Invalidate();
		m_pSpkDoc->SetModifiedFlag(TRUE);
	}
	break;

	case HINT_HITSPIKE_SHIFT: // spike is selected or deselected
	{
		long l_first;
		long l_last;
		m_pSpkList->GetRangeOfSpikeFlagged(l_first, l_last);
		const auto l_time = m_pSpkList->GetSpikeTime(threshold);
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
		if (i_id == m_ChartDataWnd_Detect.GetDlgCtrlID())
		{
			if (m_ichanselected != threshold)
			{
				m_ichanselected = threshold; // get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED, m_ichanselected);
			}
		}
		else if (i_id == m_ChartDataWnd_Source.GetDlgCtrlID())
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

void CViewSpikeDetection::OnFirstFrame()
{
	scroll_File(SB_LEFT, 1L);
}

void CViewSpikeDetection::OnLastFrame()
{
	scroll_File(SB_RIGHT, 1L);
}

void CViewSpikeDetection::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == nullptr)
	{
		CdbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
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
		m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);
		m_ChartDataWnd_Source.GetDataFromDoc(l_first, l_last);
		update_legends();
		break;

	default:
		scroll_File(nSBCode, nPos);
		break;
	}
}

void CViewSpikeDetection::update_file_scroll()
{
	m_filescroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_filescroll_infos.nPos = m_ChartDataWnd_Detect.GetDataFirst();
	m_filescroll_infos.nPage = m_ChartDataWnd_Detect.GetDataLast() - m_ChartDataWnd_Detect.GetDataFirst() + 1;
	m_filescroll.SetScrollInfo(&m_filescroll_infos);
}

void CViewSpikeDetection::scroll_File(UINT nSBCode, UINT nPos)
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
		b_result = m_ChartDataWnd_Source.ScrollDataFromDoc(nSBCode);
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		b_result = m_ChartDataWnd_Source.GetDataFromDoc(
			static_cast<long>(nPos) * (m_ChartDataWnd_Source.GetDocumentLast()) / 100L);
		break;
	default: // NOP: set position only
		break;
	}
	// adjust display
	if (b_result)
		update_legends();
}

void CViewSpikeDetection::OnFormatYscaleCentercurve()
{
	m_ChartDataWnd_Detect.CenterChan(0);
	m_ChartDataWnd_Detect.Invalidate();

	for (auto i = 0; i < m_ChartDataWnd_Source.GetChanlistSize(); i++)
		m_ChartDataWnd_Source.CenterChan(i);
	m_ChartDataWnd_Source.Invalidate();

	m_ChartSpkWnd_Bar.CenterCurve();
	m_ChartSpkWnd_Bar.Invalidate();

	m_ChartSpkWnd_Shape.SetYWExtOrg(m_ChartSpkWnd_Bar.GetYWExtent(), m_ChartSpkWnd_Bar.GetYWOrg());
	update_spike_shape_window_scale(TRUE);
	m_ChartSpkWnd_Shape.Invalidate();
}

void CViewSpikeDetection::OnFormatYscaleGainadjust()
{
	m_ChartDataWnd_Detect.MaxgainChan(0);
	m_ChartDataWnd_Detect.SetChanlistVoltsExtent(-1, nullptr);
	m_ChartDataWnd_Detect.Invalidate();

	for (int i = 0; i < m_ChartDataWnd_Source.GetChanlistSize(); i++)
		m_ChartDataWnd_Source.MaxgainChan(i);
	m_ChartDataWnd_Source.SetChanlistVoltsExtent(-1, nullptr);
	m_ChartDataWnd_Source.Invalidate();

	m_ChartSpkWnd_Bar.MaxCenter();
	m_ChartSpkWnd_Bar.Invalidate();

	m_ChartSpkWnd_Shape.SetYWExtOrg(m_ChartSpkWnd_Bar.GetYWExtent(), m_ChartSpkWnd_Bar.GetYWOrg());
	m_ChartSpkWnd_Shape.SetXWExtOrg(m_pSpkList->GetSpikeLength(), 0);
	update_spike_shape_window_scale(FALSE);
	m_ChartSpkWnd_Shape.Invalidate();
}

void CViewSpikeDetection::OnFormatSplitcurves()
{
	m_ChartDataWnd_Detect.SplitChans();
	m_ChartDataWnd_Detect.SetChanlistVoltsExtent(-1, nullptr);
	m_ChartDataWnd_Detect.Invalidate();

	m_ChartDataWnd_Source.SplitChans();
	m_ChartDataWnd_Source.SetChanlistVoltsExtent(-1, nullptr);
	m_ChartDataWnd_Source.Invalidate();

	// center curve and display bar & spikes
	m_ChartSpkWnd_Bar.MaxCenter();
	m_ChartSpkWnd_Bar.Invalidate();

	m_ChartSpkWnd_Shape.SetYWExtOrg(m_ChartSpkWnd_Bar.GetYWExtent(), m_ChartSpkWnd_Bar.GetYWOrg());
	update_spike_shape_window_scale(FALSE);
	m_ChartSpkWnd_Shape.Invalidate();
}

void CViewSpikeDetection::OnFormatAlldata()
{
	const auto l_last = GetDocument()->m_pDat->GetDOCchanLength();
	m_ChartDataWnd_Detect.ResizeChannels(0, l_last);
	m_ChartDataWnd_Detect.GetDataFromDoc(0, l_last);

	m_ChartDataWnd_Source.ResizeChannels(0, l_last);
	m_ChartDataWnd_Source.GetDataFromDoc(0, l_last);

	const auto x_we = m_pSpkList->GetSpikeLength();
	if (x_we != m_ChartSpkWnd_Shape.GetXWExtent() || 0 != m_ChartSpkWnd_Shape.GetXWOrg())
		m_ChartSpkWnd_Shape.SetXWExtOrg(x_we, 0);
	m_ChartSpkWnd_Bar.CenterCurve();
	m_ChartSpkWnd_Shape.SetYWExtOrg(m_ChartSpkWnd_Bar.GetYWExtent(), m_ChartSpkWnd_Bar.GetYWOrg());

	update_legends();
}

void CViewSpikeDetection::update_detection_parameters()
{
	// refresh pointer to spike detection array
	const auto detect_array_size = m_spk_detect_array_current.GetSize();
	ASSERT(m_i_detect_parameters < detect_array_size);
	m_p_detect_parameters = m_spk_detect_array_current.GetItem(m_i_detect_parameters);
	update_detection_settings(m_i_detect_parameters);

	// make sure that tabs are identical to what has been changed
	UpdateTabs();
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

void CViewSpikeDetection::OnToolsDetectionparameters()
{
	DlgSpikeDetect dlg;
	dlg.m_dbDoc = GetDocument()->m_pDat;
	dlg.m_iDetectParmsDlg = m_i_detect_parameters; // index spk detect parm currently selected / array
	dlg.m_pDetectSettingsArray = &m_spk_detect_array_current; // spike detection parameters array
	dlg.mdPM = options_view_data;
	dlg.m_pChartDataDetectWnd = &m_ChartDataWnd_Detect;
	if (IDOK == dlg.DoModal())
	{
		// copy modified parameters into array
		// update HorizontalCursorList on both sourceView & histogram
		m_i_detect_parameters = dlg.m_iDetectParmsDlg;
		update_detection_parameters();
	}
}

void CViewSpikeDetection::OnSelchangeDetectchan()
{
	UpdateData(TRUE);
	m_p_detect_parameters->detectChan = m_CBdetectChan.GetCurSel();
	m_p_detect_parameters->bChanged = TRUE;
	m_ChartDataWnd_Detect.SetChanlistOrdinates(0, m_p_detect_parameters->detectChan, m_p_detect_parameters->detectTransform);
	CChanlistItem* pchan = m_ChartDataWnd_Detect.GetChanlistItem(0);
	m_p_detect_parameters->detectThreshold = pchan->ConvertVoltsToDataBins(m_p_detect_parameters->detectThresholdmV / 1000.f);
	m_ChartDataWnd_Detect.MoveHZtagtoVal(0, m_p_detect_parameters->detectThreshold);
	m_ChartDataWnd_Detect.GetDataFromDoc();
	m_ChartDataWnd_Detect.AutoZoomChan(0);
	m_ChartDataWnd_Detect.Invalidate();
}

void CViewSpikeDetection::OnSelchangeTransform()
{
	UpdateData(TRUE);
	m_p_detect_parameters->detectTransform = m_CBtransform.GetCurSel();
	m_p_detect_parameters->bChanged = TRUE;
	m_ChartDataWnd_Detect.SetChanlistTransformMode(0, m_p_detect_parameters->detectTransform);
	m_ChartDataWnd_Detect.GetDataFromDoc();
	m_ChartDataWnd_Detect.AutoZoomChan(0);
	m_ChartDataWnd_Detect.Invalidate();
	update_legend_detection_wnd();
}

void CViewSpikeDetection::OnMeasureAll()
{
	detect_all(TRUE);
}

void CViewSpikeDetection::OnMeasure()
{
	detect_all(FALSE);
}

void CViewSpikeDetection::detect_all(BOOL bAll)
{
	// init spike document (erase spike list data & intervals)
	m_bDetected = TRUE;
	m_pSpkDoc->SetModifiedFlag(TRUE);

	const auto db_document = GetDocument();
	const auto data_document = db_document->m_pDat;
	const auto data_document_name = db_document->GetDB_CurrentDatFileName();
	m_pSpkDoc->m_acqfile = data_document_name;
	m_pSpkDoc->InitSourceDoc(data_document);

	m_pSpkDoc->SetDetectionDate(CTime::GetCurrentTime());
	auto old_spike_list_index = db_document->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();
	m_spikeno = -1;

	// check if detection parameters are ok? prevent detection from a channel that does not exist
	const auto p_dat = db_document->m_pDat;
	if (p_dat == nullptr)
		return;
	const auto wave_format = p_dat->GetpWaveFormat();
	const auto chanmax = wave_format->scan_count - 1;
	for (auto i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		const auto pspk_dp = m_spk_detect_array_current.GetItem(i);
		if (pspk_dp->extractChan > chanmax)
		{
			MessageBox(_T(
				"Check spike detection parameters \n- one of the detection channel requested \nis not available in the source data"));
			return;
		}
	}

	// adjust size of spklist array
	if (m_spk_detect_array_current.GetSize() != m_pSpkDoc->GetSpkList_Size())
		m_pSpkDoc->SetSpkList_Size(m_spk_detect_array_current.GetSize());

	// detect spikes from all chans marked as such
	for (int i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		if (!bAll && m_i_detect_parameters != i)
			continue;
		// detect missing data channel
		if ((m_spk_detect_array_current.GetItem(i))->extractChan > chanmax)
			continue;

		// select new spike list (list with no spikes for stimulus channel)
		CSpikeList* spike_list = m_pSpkDoc->SetSpkList_AsCurrent(i);
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
			m_ChartSpkWnd_Bar.Invalidate();
			m_ChartDataWnd_Detect.Invalidate();
			m_ChartDataWnd_Source.Invalidate();
		}
	}

	// save spike file
	saveCurrentSpkFile();

	// display data
	if (old_spike_list_index < 0)
		old_spike_list_index = 0;
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(old_spike_list_index);

	m_ChartSpkWnd_Bar.SetSourceData(m_pSpkList, db_document);
	m_ChartSpkWnd_Shape.SetSourceData(m_pSpkList, db_document);


	// center spikes, change nb spikes and update content of draw buttons
	if (options_view_data->bMaximizeGain
		|| m_ChartSpkWnd_Bar.GetYWExtent() == 0
		|| m_ChartSpkWnd_Bar.GetYWOrg() == 0
		|| m_ChartSpkWnd_Shape.GetYWExtent() == 0
		|| m_ChartSpkWnd_Shape.GetYWOrg() == 0)
	{
		m_ChartSpkWnd_Bar.MaxCenter();
		m_ChartSpkWnd_Shape.SetYWExtOrg(m_ChartSpkWnd_Bar.GetYWExtent(), m_ChartSpkWnd_Bar.GetYWOrg());
		int spklen = 60;
		if (m_pSpkList != nullptr)
			spklen = m_pSpkList->GetSpikeLength();
		m_ChartSpkWnd_Shape.SetXWExtOrg(spklen, 0);
		update_spike_shape_window_scale(FALSE);
	}

	highlight_spikes(TRUE);
	update_legends();
	UpdateTabs();
}

int CViewSpikeDetection::detect_stimulus_1(int channel_index)
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
	auto l_data_first = m_ChartDataWnd_Detect.GetDataFirst();
	const auto l_data_last = m_ChartDataWnd_Detect.GetDataLast();

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

		// DETECT STIM ---------------------------------------------------------------
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

			const int pct_achieved = (cx - l_data_first0) * 100 / l_data_len;
			dlg.SetPos(pct_achieved);
			CString comment;
			comment.Format(_T("Processing stimulus event: %i"), m_pSpkDoc->m_stimIntervals.n_items + 1);
			dlg.SetStatus(comment);

			if (dlg.CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				{
					l_last = l_data_last;
					const auto stimulus_intervals = &(m_pSpkDoc->m_stimIntervals);
					stimulus_intervals->RemoveAll();
					m_pSpkDoc->m_stimIntervals.n_items = 0;
					break;
				}

			// check if already present and insert it at the proper place
			const auto stimulus_intervals = &(m_pSpkDoc->m_stimIntervals);
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
				m_pSpkDoc->m_stimIntervals.n_items++;
			}
		}
		l_data_first = l_last + 1; // update for next loop
	}

	return m_pSpkDoc->m_stimIntervals.n_items;
}

int CViewSpikeDetection::detect_method_1(WORD schan)
{
	SPKDETECTPARM* pspkDP = m_spk_detect_array_current.GetItem(schan);
	if (pspkDP->extractTransform != pspkDP->detectTransform &&
		pspkDP->extractTransform != 0)
	{
		AfxMessageBox(
			_T("Options not implemented yet!\nd chan == extr chan or !extr chan=0\nChange detection parameters"));
		return m_pSpkList->GetTotalSpikes();
	}

	// set parameters (copy array into local parms)
	const short threshold = pspkDP->detectThreshold; // threshold value
	const auto method = pspkDP->detectTransform; // how source data are transformed
	const auto sourcechan = pspkDP->detectChan; // source channel
	const auto prethreshold = pspkDP->prethreshold; // pts before threshold
	const auto refractory = pspkDP->refractory; // refractory period
	const auto postthreshold = pspkDP->extractNpoints - prethreshold;

	// get parameters from document
	auto p_dat = GetDocument()->m_pDat;
	int nchans; // number of data chans / source buffer
	const auto p_buf = p_dat->LoadRawDataParams(&nchans);
	const auto nspan = p_dat->GetTransfDataSpan(method); // nb pts to read before transf

	// adjust detection method: if threshold lower than data zero detect lower crossing
	auto b_cross_upw = TRUE;
	if (threshold < 0)
		b_cross_upw = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = m_ChartDataWnd_Detect.GetDataFirst(); // index first pt to test
	auto l_data_last = m_ChartDataWnd_Detect.GetDataLast(); // index last pt to test
	if (l_data_first < prethreshold + nspan)
		l_data_first = static_cast<long>(prethreshold) + nspan;
	if (l_data_last > p_dat->GetDOCchanLength() - postthreshold - nspan)
		l_data_last = p_dat->GetDOCchanLength() - postthreshold - nspan;

	// loop through data defined in the lineview window
	while (l_data_first < l_data_last)
	{
		auto l_rw_first = l_data_first - prethreshold; // index very first pt within buffers
		auto l_rw_last = l_data_last; // index very last pt within buffers
		if (!p_dat->LoadRawData(&l_rw_first, &l_rw_last, nspan)) // load data from file
			break; // exit if error reported
		if (!p_dat->BuildTransfData(method, sourcechan)) // transfer data into a buffer with a single channel
			break; // exit if fail

		// load a chunk of data and see if any spikes are detected within it
		// compute initial offset (address of first point
		auto l_last = l_rw_last - postthreshold;
		if (l_last > l_data_last)
			l_last = l_data_last;
		const int i_buf_first = l_data_first - p_dat->GetDOCchanIndexFirst();
		const auto p_data_first = p_dat->GetpTransfDataElmt(i_buf_first);

		// DETECT SPIKES ---------------------------------------------------------------
		// detect event if value above threshold
		long iitime;
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
				iitime = cx; // init spike time
				// loop to search max
				for (auto i = cx; i < cx + refractory; i++, p_data1++)
				{
					if (max < *p_data1) // search max
					{
						max = *p_data1;
						p_data = p_data1; // p_data = "center" of spike
						iitime = i; // iitime = time of spike
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
				iitime = cx;
				for (auto i = cx; i < cx + refractory; i++, p_data1++)
				{
					if (min > *p_data1)
					{
						min = *p_data1;
						p_data = p_data1; // p_data = "center" of spike
						iitime = i;
					}
				}
			}

			// ........................................ SPIKE DETECTED
			if (pspkDP->extractTransform == pspkDP->detectTransform)
			{
				const auto p_m = p_data - prethreshold;
				m_pSpkList->AddSpike(p_m, 1, iitime, sourcechan, 0, TRUE);
			}
			else // extract from raw data
			{
				const auto pM = p_buf
					+ nchans * (iitime - prethreshold - l_rw_first + nspan)
					+ pspkDP->extractChan;
				m_pSpkList->AddSpike(pM, nchans, iitime, sourcechan, 0, TRUE);
			}

			// update loop parameters
			cx = iitime + refractory;
		}

		///////////////////////////////////////////////////////////////
		l_data_first = cx + 1; // update for next loop
	}

	return m_pSpkList->GetTotalSpikes();
}

void CViewSpikeDetection::OnToolsEdittransformspikes()
{
	DlgSpikeEdit dlg; // dialog box
	dlg.m_yextent = m_ChartSpkWnd_Shape.GetYWExtent(); // load display parameters
	dlg.m_yzero = m_ChartSpkWnd_Shape.GetYWOrg(); // ordinates
	dlg.m_xextent = m_ChartSpkWnd_Shape.GetXWExtent(); // and
	dlg.m_xzero = m_ChartSpkWnd_Shape.GetXWOrg(); // abcissa
	dlg.m_spikeno = m_spikeno; // load index of selected spike
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

void CViewSpikeDetection::OnFormatXscale()
{
	DlgXYParameters dlg;
	CWnd* pFocus = GetFocus();

	if (pFocus != nullptr && m_ChartDataWnd_Detect.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		CChanlistItem* pchan = m_ChartDataWnd_Detect.GetChanlistItem(m_p_detect_parameters->detectChan);
		dlg.m_yzero = pchan->GetYzero();
		dlg.m_yextent = pchan->GetYextent();
		dlg.m_bDisplaysource = TRUE;
	}
	else if (pFocus != nullptr && m_ChartSpkWnd_Bar.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		dlg.m_yzero = m_ChartSpkWnd_Bar.GetYWOrg();
		dlg.m_yextent = m_ChartSpkWnd_Bar.GetYWExtent();
		dlg.m_bDisplaybars = TRUE;
	}
	else
	{
		dlg.m_xzero = m_ChartSpkWnd_Shape.GetXWOrg();
		dlg.m_xextent = m_ChartSpkWnd_Shape.GetXWExtent();
		dlg.m_yzero = m_ChartSpkWnd_Shape.GetYWOrg();
		dlg.m_yextent = m_ChartSpkWnd_Shape.GetYWExtent();
		dlg.m_bDisplayspikes = TRUE;
	}

	if (IDOK == dlg.DoModal())
	{
		if (dlg.m_bDisplaysource)
		{
			CChanlistItem* chan = m_ChartDataWnd_Detect.GetChanlistItem(0);
			chan->SetYzero(dlg.m_yzero);
			chan->SetYextent(dlg.m_yextent);
			m_ChartDataWnd_Detect.Invalidate();
		}
		if (dlg.m_bDisplaybars)
		{
			m_ChartSpkWnd_Bar.SetYWExtOrg(dlg.m_yextent, dlg.m_yzero);
			m_ChartSpkWnd_Bar.Invalidate();
		}
		if (dlg.m_bDisplayspikes)
		{
			m_ChartSpkWnd_Shape.SetYWExtOrg(dlg.m_yextent, dlg.m_yzero);
			m_ChartSpkWnd_Shape.SetXWExtOrg(dlg.m_xextent, dlg.m_xzero);
			m_ChartSpkWnd_Shape.Invalidate();
		}
	}
}

void CViewSpikeDetection::OnBnClickedClearall()
{
	m_spikeno = -1; // unselect spike
	m_ChartSpkWnd_Bar.SelectSpike(-1); // deselect spike bars
	m_ChartSpkWnd_Shape.SelectSpikeShape(-1); // deselect superimposed spikes

	// update spike list
	for (int i = 0; i < m_pSpkDoc->GetSpkList_Size(); i++)
	{
		CSpikeList* pspklist = m_pSpkDoc->SetSpkList_AsCurrent(i);
		pspklist->InitSpikeList(GetDocument()->m_pDat, nullptr);
	}
	m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	ASSERT(m_pSpkList != NULL);

	highlight_spikes(FALSE); // remove display of spikes
	m_ChartSpkWnd_Shape.SetSourceData(m_pSpkList, GetDocument());
	m_pSpkDoc->m_stimIntervals.n_items = 0; // zero stimuli
	m_pSpkDoc->m_stimIntervals.RemoveAll();

	update_detection_parameters();
	update_VT_tags(); // update display of vertical tags
	update_legends(); // change legends
	m_pSpkDoc->SetModifiedFlag(TRUE); // mark spike document as changed
}

void CViewSpikeDetection::OnClear()
{
	m_spikeno = -1;
	m_ChartSpkWnd_Bar.SelectSpike(-1);
	m_ChartSpkWnd_Shape.SelectSpikeShape(-1);

	m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	m_pSpkList->InitSpikeList(GetDocument()->m_pDat, nullptr);
	highlight_spikes(FALSE);

	if (m_pSpkList->GetdetectWhat() == DETECT_STIMULUS)
	{
		m_pSpkDoc->m_stimIntervals.n_items = 0;
		m_pSpkDoc->m_stimIntervals.RemoveAll();
		update_VT_tags();
	}

	update_legends();
	m_pSpkDoc->SetModifiedFlag(TRUE);
	// TODO : upate database?
}

void CViewSpikeDetection::OnEnChangeSpikeno()
{
	if (mm_spikeno.m_bEntryDone)
	{
		auto spikeno = m_spikeno;
		switch (mm_spikeno.m_nChar)
		{
			// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			spikeno = m_spikeno;
			break;
		case VK_UP:
		case VK_PRIOR:
			spikeno++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			spikeno--;
			break;
		default:;
		}
		// check boundaries
		if (spikeno < -1)
			spikeno = -1;
		if (spikeno >= m_pSpkList->GetTotalSpikes())
			spikeno = m_pSpkList->GetTotalSpikes() - 1;

		// change display if necessary
		mm_spikeno.m_bEntryDone = FALSE; // clear flag
		mm_spikeno.m_nChar = 0; // empty buffer
		mm_spikeno.SetSel(0, -1); // select all text

		// update spike num and center display on the selected spike
		select_spike_no(spikeno, FALSE);
		update_spike_display();
	}
}

void CViewSpikeDetection::OnArtefact()
{
	UpdateData(TRUE); // load value from control
	const auto nspikes = m_pSpkList->GetSpikeFlagArrayCount();
	if (nspikes < 1)
	{
		m_bartefact = FALSE; // no action if spike index < 0
	}
	else
	{
		// load old class nb
		ASSERT(nspikes >= 0);
		for (auto i = 0; i < nspikes; i++)
		{
			const auto spikeno = m_pSpkList->GetSpikeFlagArrayAt(i);
			auto spkclass = m_pSpkList->GetSpikeClass(spikeno);
			// if artefact: set class to negative value
			if (m_bartefact && spkclass >= 0)
				spkclass = -(spkclass + 1);

			// if not artefact: if spike has negative class, set to positive value
			else if (spkclass < 0)
				spkclass = -(spkclass + 1);
			m_pSpkList->SetSpikeClass(spikeno, spkclass);
		}

		m_pSpkDoc->SetModifiedFlag(TRUE);
		saveCurrentSpkFile();
	}
	m_spikeno = -1;

	const auto iSelParms = m_tabCtrl.GetCurSel();
	m_p_detect_parameters = m_spk_detect_array_current.GetItem(iSelParms);
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(iSelParms);

	select_spike_no(m_spikeno, FALSE);
	update_spike_display();

	// update nb spikes
	const UINT unspikes = m_pSpkList->GetTotalSpikes();
	if (unspikes != GetDlgItemInt(IDC_NBSPIKES_NB))
		SetDlgItemInt(IDC_NBSPIKES_NB, unspikes);
}

void CViewSpikeDetection::align_display_to_current_spike()
{
	if (m_spikeno < 0)
		return;

	const auto l_spike_time = m_pSpkList->GetSpikeTime(m_spikeno);
	//long l_first = m_displayDetect.GetDataFirst();		// get source data time range
	//long l_last = m_displayDetect.GetDataLast();

	if (l_spike_time < m_ChartDataWnd_Detect.GetDataFirst()
		|| l_spike_time > m_ChartDataWnd_Detect.GetDataLast())
	{
		const auto l_size = m_ChartDataWnd_Detect.GetDataLast() - m_ChartDataWnd_Detect.GetDataFirst();
		auto l_first = l_spike_time - l_size / 2;
		if (l_first < 0)
			l_first = 0;
		auto l_last = l_first + l_size - 1;
		if (l_last > m_ChartDataWnd_Detect.GetDocumentLast())
		{
			l_last = m_ChartDataWnd_Detect.GetDocumentLast();
			l_first = l_last - l_size + 1;
		}
		m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);
		m_ChartDataWnd_Source.GetDataFromDoc(l_first, l_last);
		update_legends();
	}
}

void CViewSpikeDetection::update_spike_shape_window_scale(const BOOL b_set_from_controls)
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
			ix_we = m_pSpkList->GetextractNpoints();
		ASSERT(ix_we != 0);
		m_ChartSpkWnd_Shape.SetXWExtOrg(ix_we, m_ChartSpkWnd_Shape.GetXWOrg());

		// set amplitude
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->GetWindowText(cs);
		if (!cs.IsEmpty())
		{
			const auto y = static_cast<float>(_ttof(cs)) / 1000.0f;
			iy_we = static_cast<int>(y / m_pSpkList->GetAcqVoltsperBin());
		}
		if (iy_we == 0)
			iy_we = m_ChartSpkWnd_Shape.GetYWExtent();
		m_ChartSpkWnd_Shape.SetYWExtOrg(iy_we, m_ChartSpkWnd_Shape.GetYWOrg());
	}
	else
	{
		ix_we = m_ChartSpkWnd_Shape.GetXWExtent();
		iy_we = m_ChartSpkWnd_Shape.GetYWExtent();
	}

	if (ix_we != NULL && iy_we != NULL)
	{
		const auto yunit = m_ChartSpkWnd_Shape.GetExtent_mV() / m_ChartSpkWnd_Shape.GetNyScaleCells();
		m_ChartSpkWnd_Shape.SetyScaleUnitValue(yunit);
		const auto xunit = m_ChartSpkWnd_Shape.GetExtent_ms() / m_ChartSpkWnd_Shape.GetNxScaleCells();
		m_ChartSpkWnd_Shape.SetxScaleUnitValue(xunit);
	}

	// output values
	_stprintf_s(m_sz_buffer, 64, _T("%.3lf"), m_ChartSpkWnd_Shape.GetExtent_mV());
	SetDlgItemText(IDC_SPIKEWINDOWAMPLITUDE, m_sz_buffer);
	const auto yunit = m_ChartSpkWnd_Shape.GetExtent_mV() / m_ChartSpkWnd_Shape.GetNyScaleCells();
	m_ChartSpkWnd_Shape.SetyScaleUnitValue(yunit);

	_stprintf_s(m_sz_buffer, 64, _T("%.3lf"), m_ChartSpkWnd_Shape.GetExtent_ms());
	SetDlgItemText(IDC_SPIKEWINDOWLENGTH, m_sz_buffer);
	const auto xunit = m_ChartSpkWnd_Shape.GetExtent_ms() / m_ChartSpkWnd_Shape.GetNxScaleCells();
	m_ChartSpkWnd_Shape.SetxScaleUnitValue(xunit);
}

void CViewSpikeDetection::select_spike_no(int spikeno, BOOL bMultipleSelection)
{
	// load spike parameters
	if (spikeno >= 0)
	{
		// get address of spike parms
		const auto p_spike_element = m_pSpkList->GetSpikeElemt(spikeno);
		m_bartefact = (p_spike_element->get_class() < 0);
		if (bMultipleSelection) // TRUE= multiple selection
		{
			/*auto nflaggedspikes =*/
			m_pSpkList->ToggleSpikeFlag(spikeno);
			if (m_pSpkList->GetSpikeFlagArrayCount() < 1)
				spikeno = -1;
			if (m_spikeno == spikeno)
				spikeno = 0;
		}
		else
		{
			m_pSpkList->SetSingleSpikeFlag(spikeno);
		}
		m_spikeno = spikeno;
		align_display_to_current_spike();
	}
	else
	{
		m_pSpkList->RemoveAllSpikeFlags();
		m_bartefact = FALSE;
	}
}

void CViewSpikeDetection::update_spike_display()
{
	// update spike display windows
	m_ChartSpkWnd_Bar.Invalidate(TRUE);
	m_ChartSpkWnd_Shape.Invalidate(TRUE);

	// update Dlg interface
	GetDlgItem(IDC_SPIKENO)->EnableWindow(!(m_pSpkList->GetSpikeFlagArrayCount() > 1));
	SetDlgItemInt(IDC_SPIKENO, m_spikeno, TRUE);
	CheckDlgButton(IDC_ARTEFACT, m_bartefact);
}

void CViewSpikeDetection::OnEnChangeThresholdval()
{
	if (mm_thresholdval.m_bEntryDone)
	{
		auto thresholdval = m_thresholdval;
		switch (mm_thresholdval.m_nChar)
		{
			// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			thresholdval = m_thresholdval;
			break;
		case VK_UP:
		case VK_PRIOR:
			thresholdval++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			thresholdval--;
			break;
		default:;
		}

		// change display if necessary
		if (m_thresholdval != thresholdval)
		{
			m_thresholdval = thresholdval;
			m_p_detect_parameters->detectThresholdmV = thresholdval;
			CChanlistItem* pchan = m_ChartDataWnd_Detect.GetChanlistItem(0);
			m_p_detect_parameters->detectThreshold = pchan->ConvertVoltsToDataBins(m_thresholdval / 1000.f);
			m_ChartDataWnd_Detect.MoveHZtagtoVal(0, m_p_detect_parameters->detectThreshold);
		}

		mm_thresholdval.m_bEntryDone = FALSE;
		mm_thresholdval.m_nChar = 0;
		CString cs;
		cs.Format(_T("%.3f"), m_thresholdval);
		GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
		// change detection threshold method to manual
		mm_thresholdval.SetSel(0, -1); // select all text
	}
}

void CViewSpikeDetection::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone)
	{
		switch (mm_timefirst.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
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
		m_ChartDataWnd_Detect.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		m_ChartDataWnd_Source.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		update_legends();

		mm_timefirst.m_bEntryDone = FALSE;
		mm_timefirst.m_nChar = 0;
		mm_timefirst.SetSel(0, -1); //select all text
	}
}

void CViewSpikeDetection::OnEnChangeTimelast()
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
		m_ChartDataWnd_Detect.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		m_ChartDataWnd_Source.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate),
			static_cast<long>(m_timelast * m_samplingRate));
		update_legends();

		mm_timelast.m_bEntryDone = FALSE;
		mm_timelast.m_nChar = 0;
		mm_timelast.SetSel(0, -1); //select all text
	}
}

void CViewSpikeDetection::OnToolsDataseries()
{
	// init dialog data
	DlgDataSeries dlg;
	dlg.m_pChartDataWnd = &m_ChartDataWnd_Detect;
	dlg.m_pdbDoc = GetDocument()->m_pDat;
	dlg.m_listindex = 0;

	// invoke dialog box
	dlg.DoModal();
	if (m_ChartDataWnd_Detect.GetChanlistSize() < 1)
	{
		m_ChartDataWnd_Detect.RemoveAllChanlistItems();
		m_ChartDataWnd_Detect.AddChanlistItem(m_p_detect_parameters->detectChan, m_p_detect_parameters->detectTransform);
	}
	update_legends();
}

void CViewSpikeDetection::PrintDataCartridge(CDC* p_dc, ChartData* pDataChartWnd, CRect* prect, BOOL bComments,
	BOOL bBars)
{
	SCOPESTRUCT* pStruct = pDataChartWnd->GetScopeParameters();
	const auto b_draw_f = pStruct->bDrawframe;
	pStruct->bDrawframe = TRUE;
	pDataChartWnd->Print(p_dc, prect, (options_view_data->bcontours == 1));
	pStruct->bDrawframe = b_draw_f;

	// data vertical and horizontal bars

	const auto comments = PrintDataBars(p_dc, pDataChartWnd, prect);

	const int xcol = prect->left;
	const int ypxrow = prect->top;
	//const int lineheight = m_logFont.lfHeight + 5;
	p_dc->TextOut(xcol, ypxrow, comments);
	//ypxrow += lineheight;

	p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	//UINT n_format = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
	//auto rect_comment = *prect;
	//rect_comment.top = ypxrow;
	//rect_comment.right = prect->left;
}

void CViewSpikeDetection::OnEditCopy()
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
			m_ChartDataWnd_Detect.CopyAsText(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			serialize_windows_state(b_save);

			CRect old_rect1; // save size of lineview windows
			m_ChartDataWnd_Detect.GetWindowRect(&old_rect1);
			CRect old_rect2;
			m_ChartDataWnd_Source.GetWindowRect(&old_rect2);

			const CRect rect(0, 0, options_view_data->hzResolution, options_view_data->vtResolution);
			m_npixels0 = m_ChartDataWnd_Detect.GetRectWidth();

			// create metafile
			CMetaFileDC m_dc;
			auto rect_bound = rect;
			rect_bound.right *= 32;
			rect_bound.bottom *= 30;
			const auto p_dc_ref = GetDC();
			auto cs_title = _T("dbWave\0") + GetDocument()->m_pDat->GetTitle();
			cs_title += _T("\0\0");
			const auto hm_dc = m_dc.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
			ASSERT(hm_dc != NULL);

			// Draw document in metafile.
			CPen black_pen(PS_SOLID, 0, RGB(0, 0, 0));
			const auto poldpen = m_dc.SelectObject(&black_pen);
			const auto p_old_brush = static_cast<CBrush*>(m_dc.SelectStockObject(BLACK_BRUSH));
			CClientDC attrib_dc(this); // Create and attach attribute DC
			m_dc.SetAttribDC(attrib_dc.GetSafeHdc()); // from current screen

			// print comments : set font
			m_pOldFont = nullptr;
			const auto oldsize = options_view_data->fontsize;
			options_view_data->fontsize = 10;
			PrintCreateFont();
			m_dc.SetBkMode(TRANSPARENT);
			options_view_data->fontsize = oldsize;
			m_pOldFont = m_dc.SelectObject(&m_fontPrint);
			const int lineheight = m_logFont.lfHeight + 5;
			auto ypxrow = 0;
			const auto xcol = 10;

			// comment and descriptors
			auto comments = GetDocument()->Export_DatabaseData(1);
			m_dc.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;

			// abcissa
			comments = _T("Abcissa: ");
			CString content;
			GetDlgItem(IDC_TIMEFIRST)->GetWindowText(content);
			comments += content;
			comments += _T(" - ");
			GetDlgItem(IDC_TIMELAST)->GetWindowText(content);
			comments += content;
			m_dc.TextOut(xcol, ypxrow, comments);
			//ypxrow += lineheight;

			// define display sizes - dataview & datadetect are same, spkshape & spkbar = as on screen
			auto rectdata = rect;
			rectdata.top -= -3 * lineheight;
			const auto rspkwidth = MulDiv(m_ChartSpkWnd_Shape.GetRectWidth(), rectdata.Width(),
				m_ChartSpkWnd_Shape.GetRectWidth() + m_ChartDataWnd_Detect.GetRectWidth());
			const auto rdataheight = MulDiv(m_ChartDataWnd_Detect.GetRectHeight(), rectdata.Height(),
				m_ChartDataWnd_Detect.GetRectHeight() * 2 + m_ChartSpkWnd_Bar.
				GetRectHeight());
			const auto separator = rspkwidth / 10;

			// display curves : data
			rectdata.bottom = rect.top + rdataheight - separator / 2;
			rectdata.left = rect.left + rspkwidth + separator;
			PrintDataCartridge(&m_dc, &m_ChartDataWnd_Source, &rectdata, TRUE, TRUE);

			// display curves: detect channel
			rectdata.top = rectdata.bottom + separator;
			rectdata.bottom = rectdata.top + rdataheight;
			PrintDataCartridge(&m_dc, &m_ChartDataWnd_Detect, &rectdata, TRUE, TRUE);

			// display spike bars
			auto rectbars = rectdata;
			rectbars.top = rectdata.bottom + separator;
			rectbars.bottom = rect.bottom - 2 * lineheight;
			m_ChartSpkWnd_Bar.Print(&m_dc, &rectbars);

			// display spike shapes
			auto rectspk = rect; // compute output rectangle
			rectspk.left += separator;
			rectspk.right = rect.left + rspkwidth;
			rectspk.bottom = rect.bottom - 2 * lineheight;
			rectspk.top = rectspk.bottom - rectbars.Height();
			m_ChartSpkWnd_Shape.Print(&m_dc, &rectspk);
			comments = PrintSpkShapeBars(&m_dc, &rectspk, TRUE);

			auto rect_comment = rect;
			//rect_comment.top = ypxrow;
			rect_comment.right = rectdata.left;
			rect_comment.top = rectspk.bottom;
			const UINT n_format = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
			m_dc.DrawText(comments, comments.GetLength(), rect_comment, n_format);
			m_dc.SelectObject(p_old_brush);

			if (m_pOldFont != nullptr)
				m_dc.SelectObject(m_pOldFont);
			m_fontPrint.DeleteObject();

			// restore oldpen
			m_dc.SelectObject(poldpen);
			ReleaseDC(p_dc_ref);

			// Close metafile
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
			serialize_windows_state(b_restore);
		}
	}
}

void CViewSpikeDetection::OnSelchangeDetectMode()
{
	UpdateData(TRUE);
	m_p_detect_parameters->detectWhat = m_CBdetectWhat.GetCurSel();
	update_combo_box();
	update_legend_detection_wnd();
	m_ChartDataWnd_Detect.GetDataFromDoc();
	m_ChartDataWnd_Detect.AutoZoomChan(0);
	m_ChartDataWnd_Detect.Invalidate();
}

void CViewSpikeDetection::update_combo_box()
{
	m_CBdetectChan.SetCurSel(m_p_detect_parameters->detectChan);
	m_CBtransform.SetCurSel(m_p_detect_parameters->detectTransform);
	m_ChartDataWnd_Detect.SetChanlistOrdinates(0, m_p_detect_parameters->detectChan, m_p_detect_parameters->detectTransform);
	m_p_detect_parameters->detectThreshold = m_ChartDataWnd_Detect.GetChanlistItem(0)->ConvertVoltsToDataBins(
		m_thresholdval / 1000.f);
	m_ChartDataWnd_Detect.m_HZtags.SetTagChan(0, 0);
	m_ChartDataWnd_Detect.m_HZtags.SetTagVal(0, m_p_detect_parameters->detectThreshold);
	m_p_detect_parameters->detectThresholdmV = m_thresholdval;
}

void CViewSpikeDetection::update_legend_detection_wnd()
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

void CViewSpikeDetection::update_VT_tags()
{
	m_ChartSpkWnd_Bar.m_VTtags.RemoveAllTags();
	m_ChartDataWnd_Detect.m_VTtags.RemoveAllTags();
	m_ChartDataWnd_Source.m_VTtags.RemoveAllTags();
	if (m_pSpkDoc->m_stimIntervals.n_items == 0)
		return;

	for (auto i = 0; i < m_pSpkDoc->m_stimIntervals.GetSize(); i++)
	{
		const int cx = m_pSpkDoc->m_stimIntervals.GetAt(i);
		m_ChartSpkWnd_Bar.m_VTtags.AddLTag(cx, 0);
		m_ChartDataWnd_Detect.m_VTtags.AddLTag(cx, 0);
		m_ChartDataWnd_Source.m_VTtags.AddLTag(cx, 0);
	}
}

void CViewSpikeDetection::PrintComputePageSize()
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
	m_printRect.right = options_view_data->horzRes - options_view_data->rightPageMargin;
	m_printRect.bottom = options_view_data->vertRes - options_view_data->bottomPageMargin;
	m_printRect.left = options_view_data->leftPageMargin;
	m_printRect.top = options_view_data->topPageMargin;
}

void CViewSpikeDetection::PrintFileBottomPage(CDC* p_dc, CPrintInfo* p_info)
{
	auto t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"), // %d:%d",
		p_info->m_nCurPage, p_info->GetMaxPage(),
		t.GetDay(), t.GetMonth(), t.GetYear());
	const auto ch_date = GetDocument()->GetDB_CurrentSpkFileName();
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_view_data->horzRes / 2, options_view_data->vertRes - 57, ch_date);
}

CString CViewSpikeDetection::PrintConvertFileIndex(const long l_first, const long l_last)
{
	CString cs_unit = _T(" s"); // get time,  prepare time unit
	CString cs_comment;
	float x_scale_factor; // scale factor returned by changeunit
	const auto x1 = m_ChartDataWnd_Detect.ChangeUnit(static_cast<float>(l_first) / m_samplingRate, &cs_unit,
		&x_scale_factor);
	const auto fraction1 = static_cast<int>((x1 - static_cast<int>(x1)) * static_cast<float>(1000.));
	// separate fractional part
	const auto x2 = l_last / (m_samplingRate * x_scale_factor);
	const auto fraction2 = static_cast<int>((x2 - static_cast<int>(x2)) * static_cast<float>(1000.));
	cs_comment.Format(_T("time = %i.%03.3i - %i.%03.3i %s"), static_cast<int>(x1), fraction1, static_cast<int>(x2),
		fraction2, (LPCTSTR)cs_unit);
	return cs_comment;
}

BOOL CViewSpikeDetection::PrintGetFileSeriesIndexFromPage(int page, int& filenumber, long& l_first)
{
	// loop until we get all rows
	const auto totalrows = m_nbrowsperpage * (page - 1);
	l_first = m_lprintFirst;
	filenumber = 0; // file list index
	if (options_view_data->bPrintSelection) // current file if selection only
		filenumber = m_file0;
	else
		GetDocument()->DBMoveFirst();

	auto very_last = m_lprintFirst + m_lprintLen;
	if (options_view_data->bEntireRecord)
		very_last = GetDocument()->GetDB_DataLen() - 1;

	for (auto row = 0; row < totalrows; row++)
	{
		if (!PrintGetNextRow(filenumber, l_first, very_last))
			break;
	}

	return TRUE;
}

BOOL CViewSpikeDetection::PrintGetNextRow(int& filenumber, long& l_first, long& very_last)
{
	if (!options_view_data->bMultirowDisplay || !options_view_data->bEntireRecord)
	{
		filenumber++;
		if (filenumber >= m_nfiles)
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
			filenumber++; // next index
			if (filenumber >= m_nfiles) // last file ??
				return FALSE;

			GetDocument()->DBMoveNext();
			very_last = GetDocument()->GetDB_DataLen() - 1;
			l_first = m_lprintFirst;
		}
	}
	return TRUE;
}

CString CViewSpikeDetection::PrintGetFileInfos()
{
	CString str_comment; // scratch pad
	const CString tab(_T("    ")); // use 4 spaces as tabulation character
	const CString rc(_T("\n")); // next line

	// document's name, date and time
	const auto p_data_file = GetDocument()->m_pDat;
	const auto pwave_format = p_data_file->GetpWaveFormat();
	if (options_view_data->bDocName || options_view_data->bAcqDateTime) // print doc infos?
	{
		if (options_view_data->bDocName) // print file name
			str_comment += GetDocument()->GetDB_CurrentDatFileName() + tab;

		if (options_view_data->bAcqDateTime) // print data acquisition date & time
		{
			const auto date = (pwave_format->acqtime).Format(_T("%#d %B %Y %X")); //("%c");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_view_data->bAcqComment)
		str_comment += GetDocument()->Export_DatabaseData(); //pwaveFormat->GetComments(" ") +rc;

	return str_comment;
}

CString CViewSpikeDetection::PrintDataBars(CDC* p_dc, ChartData* pDataChartWnd, CRect* rect)
{
	CString cs;
	const CString rc(_T("\r"));
	const CString tab(_T("     "));
	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
	CString cs_unit; // string for voltage or time unit
	const CPoint bar_origin(-10, -10); // bar origin at 10,10 pts on the left lower corner of the rectangle
	//CSize	barWidth = CSize(1,1);					// width of bars (5 pixels)
	auto ihorz_bar = pDataChartWnd->GetRectWidth() / 10; // initial horz bar length 1/10th of display rect
	auto ivert_bar = pDataChartWnd->GetRectHeight() / 3; // initial vert bar height 1/3rd  of display rect

	///// time abscissa ///////////////////////////
	auto str_comment = PrintConvertFileIndex(pDataChartWnd->GetDataFirst(), pDataChartWnd->GetDataLast());

	///// horizontal time bar ///////////////////////////
	if (options_view_data->bTimeScaleBar)
	{
		// convert bar size into time units and back into pixels
		cs_unit = _T(" s"); // initial time unit
		const auto xtperpixel = pDataChartWnd->GetTimeperPixel();
		const auto z = xtperpixel * ihorz_bar; // convert 1/10 of the length of the data displayed into time
		float x_scale_factor;
		const auto x = pDataChartWnd->ChangeUnit(z, &cs_unit, &x_scale_factor); // convert time into a scaled time
		const auto k = pDataChartWnd->NiceUnit(x); // convert the (scaled) time value into time expressed as an integral
		ihorz_bar = static_cast<int>((static_cast<float>(k) * x_scale_factor) / xtperpixel);
		// compute how much pixels it makes
		// print out the scale and units
		cs.Format(_T("horz bar = %i %s"), k, (LPCTSTR)cs_unit);
		str_comment += cs + rc;
		// draw horizontal line
		ihorz_bar = MulDiv(ihorz_bar, rect->Width(), pDataChartWnd->GetRectWidth());
		p_dc->MoveTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y);
		p_dc->LineTo(rect->left + bar_origin.x + ihorz_bar, rect->bottom - bar_origin.y);
	}

	///// vertical voltage bars ///////////////////////////
	float y_scale_factor; // compute a good unit for channel 0
	cs_unit = _T(" V"); // initial voltage unit
	// convert bar size into voltage units and back into pixels
	const auto vperpixel = pDataChartWnd->GetChanlistVoltsperPixel(0);
	const auto zvolts = vperpixel * ivert_bar; // convert 1/3 of the height into voltage
	const auto zscale = pDataChartWnd->ChangeUnit(zvolts, &cs_unit, &y_scale_factor);
	// convert voltage into a scale voltage
	const auto znice = static_cast<float>(pDataChartWnd->NiceUnit(zscale));
	// convert the (scaled) time value into time expressed as an integral
	ivert_bar = static_cast<int>(znice * y_scale_factor / vperpixel); // compute how much pixels it makes

	if (options_view_data->bVoltageScaleBar)
	{
		ivert_bar = MulDiv(ivert_bar, rect->Height(), pDataChartWnd->GetRectHeight());
		p_dc->MoveTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y);
		p_dc->LineTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y - ivert_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_view_data->bChansComment || options_view_data->bVoltageScaleBar || options_view_data->bChanSettings)
	{
		const auto imax = pDataChartWnd->GetChanlistSize(); // number of data channels
		for (auto ichan = 0; ichan < imax; ichan++) // loop
		{
			CChanlistItem* pchan = pDataChartWnd->GetChanlistItem(ichan);
			// skip channels not printed
			if (!pchan->GetflagPrintVisible())
				continue;
			// boucler sur les commentaires de chan n a chan 0...
			cs.Format(_T("chan#%i "), ichan); // channel number
			str_comment += cs;
			if (options_view_data->bVoltageScaleBar) // bar scale value
			{
				const auto z = static_cast<float>(ivert_bar) * pDataChartWnd->GetChanlistVoltsperPixel(ichan);
				const auto x = z / y_scale_factor;
				const auto j = pDataChartWnd->NiceUnit(x);
				cs.Format(_T("vert bar = %i %s "), j, (LPCTSTR)cs_unit); // store value into comment
				str_comment += cs;
			}
			// print chan comment
			if (options_view_data->bChansComment)
			{
				str_comment += tab;
				str_comment += pchan->GetComment();
			}
			str_comment += rc;
			// print amplifiers settings (gain & filter), next line
			if (options_view_data->bChanSettings)
			{
				const WORD channb = pchan->GetSourceChan();
				const auto pchan_array = GetDocument()->m_pDat->GetpWavechanArray();
				const auto p_chan = pchan_array->Get_p_channel(channb);
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

CString CViewSpikeDetection::PrintSpkShapeBars(CDC* p_dc, CRect* rect, BOOL bAll)
{
	const CString rc("\n");
	CString str_comment;
	CString cs_comment;
	CString cs_unit;
	float z;
	int k;

	///// vertical voltage bars ///////////////////////////
	if (options_view_data->bVoltageScaleBar && m_pSpkList->GetTotalSpikes() > 0)
	{
		// the following assume that spikes are higher than 1 mV...
		cs_unit = _T(" mV");
		z = m_ChartSpkWnd_Shape.GetExtent_mV() / 2.0f; // half of total height
		k = static_cast<int>(z); // clip to int
		if ((static_cast<double>(z) - k) > 0.5) // make sure it is not zero
			k++;
		if (bAll)
		{
			if (k > 0)
				wsprintf(m_sz_buffer, _T("Vbar=%i %s"), k, static_cast<LPCTSTR>(cs_unit));
			else
				_stprintf_s(m_sz_buffer, 64, _T("Vbar=%f.3 mV"), z);
			cs_comment = m_sz_buffer; // store value into comment
			str_comment = cs_comment + rc;
		}

		// display bar
		const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
		if (k > 0)
			z = static_cast<float>(k) / z;
		const auto vert_bar = static_cast<int>(rect->Height() * z) / 2;

		// compute coordinates of the rect
		CRect rect_vert_bar; // vertical bar rectangle
		const auto bar_width = CSize(5, 5);
		rect_vert_bar.left = rect->left - options_view_data->textseparator;
		rect_vert_bar.right = rect_vert_bar.left + bar_width.cx;
		rect_vert_bar.top = rect->top + (rect->Height() - vert_bar) / 2;
		rect_vert_bar.bottom = rect_vert_bar.top + vert_bar;
		p_dc->Rectangle(&rect_vert_bar);
		p_dc->SelectObject(p_old_brush);
	}

	// spike duration
	if (m_pSpkList->GetTotalSpikes() > 0 && bAll)
	{
		z = m_ChartSpkWnd_Shape.GetExtent_ms();
		cs_unit = _T(" ms");
		k = static_cast<int>(z);
		wsprintf(m_sz_buffer, _T("Horz=%i."), k);
		cs_comment = m_sz_buffer; // store value into comment
		k = static_cast<int>(1000.0f * (z - static_cast<float>(k)));
		wsprintf(m_sz_buffer, _T("%i %s"), k, static_cast<LPCTSTR>(cs_unit));
		cs_comment += m_sz_buffer; // store value into comment
		str_comment += cs_comment;
		str_comment += rc;
	}

	// number of spikes
	k = m_pSpkList->GetTotalSpikes();
	wsprintf(m_sz_buffer, _T("n spk= %i"), k);
	cs_comment = m_sz_buffer;
	str_comment += cs_comment;
	str_comment += rc;

	return str_comment;
}

void CViewSpikeDetection::serialize_windows_state(BOOL bSave, int itab)
{
	auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp()); // pointer to list of pointers to store parameters
	if (itab < 0 || itab >= m_tabCtrl.GetItemCount())
	{
		int itab_sel = m_tabCtrl.GetCurSel(); // current selected tab
		if (itab_sel < 0)
			itab_sel = 0;
		itab = itab_sel;
	}

	// adjust size of the array
	if (p_app->viewspikesmemfile_ptr_array.GetSize() == 0)
		p_app->viewspikesmemfile_ptr_array.SetSize(1);

	if (p_app->viewspikesmemfile_ptr_array.GetSize() < m_tabCtrl.GetItemCount())
		p_app->viewspikesmemfile_ptr_array.SetSize(m_tabCtrl.GetItemCount());
	CMemFile* p_mem_file = nullptr;
	if (p_app->viewspikesmemfile_ptr_array.GetSize() > 0 && p_app->viewspikesmemfile_ptr_array.GetSize() > itab)
		p_mem_file = p_app->viewspikesmemfile_ptr_array.GetAt(itab);

	// save display parameters
	if (bSave)
	{
		if (p_mem_file == nullptr)
		{
			p_mem_file = new CMemFile;
			ASSERT(p_mem_file != NULL);
			p_app->viewspikesmemfile_ptr_array.SetAt(itab, p_mem_file);
		}
		// save data into archive
		CArchive ar(p_mem_file, CArchive::store);
		p_mem_file->SeekToBegin();
		m_ChartDataWnd_Source.Serialize(ar);
		m_ChartDataWnd_Detect.Serialize(ar);
		m_ChartSpkWnd_Bar.Serialize(ar);
		m_ChartSpkWnd_Shape.Serialize(ar);
		ar.Close();
	}

	// restore display parameters
	else
	{
		if (p_mem_file != nullptr)
		{
			CArchive ar(p_mem_file, CArchive::load);
			p_mem_file->SeekToBegin();
			m_ChartDataWnd_Source.Serialize(ar);
			m_ChartDataWnd_Detect.Serialize(ar);
			m_ChartSpkWnd_Bar.Serialize(ar);
			m_ChartSpkWnd_Shape.Serialize(ar);
			ar.Close(); // close archive
		}
		else
		{
			*m_ChartDataWnd_Source.GetScopeParameters() = options_view_data->viewspkdetectdata;
			*m_ChartDataWnd_Detect.GetScopeParameters() = options_view_data->viewspkdetectfiltered;
			*m_ChartSpkWnd_Bar.GetScopeParameters() = options_view_data->viewspkdetectspk;
			*m_ChartSpkWnd_Shape.GetScopeParameters() = options_view_data->viewspkdetectbars;
			//OnFormatSplitcurves();
		}
	}
}

BOOL CViewSpikeDetection::OnPreparePrinting(CPrintInfo* pInfo)
{
	// save current state of the windows
	serialize_windows_state(b_save);

	// printing margins
	if (options_view_data->vertRes <= 0 || options_view_data->horzRes <= 0
		|| options_view_data->horzRes != pInfo->m_rectDraw.Width()
		|| options_view_data->vertRes != pInfo->m_rectDraw.Height())
		PrintComputePageSize();

	// nb print pages?
	int npages = PrintGetNPages();
	pInfo->SetMaxPage(npages); // one page printing/preview
	pInfo->m_nNumPreviewPages = 1; // preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (!DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	//------------------------------------------------------
	if (options_view_data->bPrintSelection != pInfo->m_pPD->PrintSelection())
	{
		options_view_data->bPrintSelection = pInfo->m_pPD->PrintSelection();
		npages = PrintGetNPages();
		pInfo->SetMaxPage(npages);
	}
	return TRUE;
}

int CViewSpikeDetection::PrintGetNPages()
{
	// how many rows per page?
	const auto size_row = options_view_data->HeightDoc + options_view_data->heightSeparator;
	m_nbrowsperpage = m_printRect.Height() / size_row;
	if (m_nbrowsperpage == 0) // prevent zero pages
		m_nbrowsperpage = 1;

	int ntotal_rows; // number of rectangles -- or nb of rows
	auto pdb_doc = GetDocument();

	// compute number of rows according to bmultirow & bentirerecord flag
	m_lprintFirst = m_ChartDataWnd_Detect.GetDataFirst();
	m_lprintLen = m_ChartDataWnd_Detect.GetDataLast() - m_lprintFirst + 1;
	m_file0 = GetDocument()->GetDB_CurrentRecordPosition();
	ASSERT(m_file0 >= 0);
	m_nfiles = 1;
	auto ifile0 = m_file0;
	auto ifile1 = m_file0;
	if (!options_view_data->bPrintSelection)
	{
		ifile0 = 0;
		m_nfiles = pdb_doc->GetDB_NRecords();
		ifile1 = m_nfiles;
	}

	// only one row per file
	if (!options_view_data->bMultirowDisplay || !options_view_data->bEntireRecord)
		ntotal_rows = m_nfiles;

	// multirows per file
	else
	{
		ntotal_rows = 0;
		pdb_doc->SetDB_CurrentRecordPosition(ifile0);
		for (auto i = ifile0; i < ifile1; i++, pdb_doc->DBMoveNext())
		{
			// get size of document for all files
			auto len = pdb_doc->GetDB_DataLen();
			if (len <= 0)
			{
				pdb_doc->OpenCurrentDataFile();
				len = pdb_doc->m_pDat->GetDOCchanLength();
				const auto len1 = GetDocument()->GetDB_DataLen() - 1;
				ASSERT(len == len1);
				pdb_doc->SetDB_DataLen(len);
			}
			len -= m_lprintFirst;
			auto nrows = len / m_lprintLen; // how many rows for this file?
			if (len > nrows * m_lprintLen) // remainder?
				nrows++;
			ntotal_rows += static_cast<int>(nrows); // update nb of rows
		}
	}

	if (m_file0 >= 0)
	{
		pdb_doc->SetDB_CurrentRecordPosition(m_file0);
		pdb_doc->OpenCurrentDataFile();
	}

	// npages
	auto npages = ntotal_rows / m_nbrowsperpage;
	if (ntotal_rows > m_nbrowsperpage * npages)
		npages++;

	return npages;
}

void CViewSpikeDetection::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_ChartDataWnd_Detect.GetDataFirst();
	m_lLast0 = m_ChartDataWnd_Detect.GetDataLast();
	m_npixels0 = m_ChartDataWnd_Detect.GetRectWidth();
	PrintCreateFont();
	p_dc->SetBkMode(TRANSPARENT);
}

void CViewSpikeDetection::PrintCreateFont()
{
	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial")); // Arial font
	m_logFont.lfHeight = options_view_data->fontsize; // font height
	m_pOldFont = nullptr;
	m_fontPrint.CreateFontIndirect(&m_logFont);
}

void CViewSpikeDetection::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
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
		m_ChartDataWnd_Detect.SetChanlistTransformMode(0, 0);

	p_dc->SetMapMode(MM_TEXT); // change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(p_dc, pInfo); // print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int filenumber; // file number and file index
	long l_first; // index first data point / first file
	auto very_last = m_lprintFirst + m_lprintLen; // index last data point / current file
	const int curpage = pInfo->m_nCurPage; // get current page number
	PrintGetFileSeriesIndexFromPage(curpage, filenumber, l_first);
	if (l_first < GetDocument()->GetDB_DataLen() - 1)
		update_file_parameters(FALSE);
	if (options_view_data->bEntireRecord)
		very_last = GetDocument()->GetDB_DataLen() - 1;

	// loop through all files	--------------------------------------------------------
	for (auto i = 0; i < m_nbrowsperpage; i++)
	{
		const auto old_dc = p_dc->SaveDC(); // save DC

		// first : set rectangle where data will be printed
		auto comment_rect = r_where; // save RWhere for comments
		p_dc->SetMapMode(MM_TEXT); // 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); // set text align mode

		// load data and adjust display rectangle ----------------------------------------
		// set data rectangle to half height to the row height
		m_rData = r_where; // adjust data rect/ file data rect
		if (options_view_data->bPrintSpkBars)
			m_rData.bottom = m_rData.top + r_where.Height() / 2; // half vertic size = data
		m_rData.left += (r_sp_kwidth + options_view_data->textseparator);
		const auto oldsize = m_rData.Width(); // save display rect size

		// make sure enough data fit into this rectangle, otherwise clip rect
		auto l_last = l_first + m_lprintLen; // compute last pt to load
		if (l_last > very_last) // check end across file length
			l_last = very_last;
		if ((l_last - l_first + 1) < m_lprintLen) // adjust rect to length of data
			m_rData.right = (oldsize * (l_last - l_first)) / m_lprintLen + m_rData.left;
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// if option requested, clip output to rect
		if (options_view_data->bClipRect) // clip curve display
			p_dc->IntersectClipRect(&m_rData); // (eventually)

		// print detected channel only data
		m_ChartDataWnd_Detect.GetChanlistItem(0)->SetflagPrintVisible(chan0Drawmode);
		m_ChartDataWnd_Detect.ResizeChannels(m_rData.Width(), 0);
		m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last); // load data from file
		m_ChartDataWnd_Detect.Print(p_dc, &m_rData); // print data
		p_dc->SelectClipRgn(nullptr); // no more clipping

		// print spike bars ---------------------------------------------------------------
		if (options_view_data->bPrintSpkBars)
		{
			CRect BarsRect = r_where; // adjust data rect/ file data rect
			BarsRect.top = m_rData.bottom; // copy coordinates
			BarsRect.left = m_rData.left;
			BarsRect.right = m_rData.right;

			m_ChartSpkWnd_Bar.SetTimeIntervals(l_first, l_last); // define time interval
			m_ChartSpkWnd_Bar.Print(p_dc, &BarsRect); // print data
		}

		// print spkform within a square (same width as height) ---------------------------
		m_rSpike = r_where; // get file rectangle
		m_rSpike.right = m_rSpike.left + r_sp_kwidth;
		m_rSpike.left += options_view_data->textseparator; // remove space for 4 lines of
		m_rSpike.bottom = m_rSpike.top + r_sp_kheight; // legends

		m_ChartSpkWnd_Shape.SetTimeIntervals(l_first, l_last);
		m_ChartSpkWnd_Shape.Print(p_dc, &m_rSpike);

		// restore DC and print comments ---------------------------------------------------
		p_dc->RestoreDC(old_dc); // restore Display context
		p_dc->SetMapMode(MM_TEXT); // 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr); // no more clipping
		p_dc->SetViewportOrg(0, 0); // org = 0,0

		// print data Bars & get comments according to row within file
		CString cs_comment;
		const BOOL b_all = (l_first == m_lprintFirst);
		if (b_all) // first row = full comment
		{
			cs_comment += PrintGetFileInfos(); // describe file, intervals & comments /chan
			cs_comment += PrintDataBars(p_dc, &m_ChartDataWnd_Detect, &m_rData); // bars and bar legends
		}
		else
		{
			// other rows: time intervals only
			cs_comment = PrintConvertFileIndex(m_ChartDataWnd_Detect.GetDataFirst(),
				m_ChartDataWnd_Detect.GetDataLast());
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_view_data->textseparator + comment_rect.Width(), 0);
		comment_rect.right = m_printRect.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		const UINT n_format = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect, n_format);

		// print comments & bar / spike shape
		cs_comment.Empty();
		m_rSpike.right = m_rSpike.left + r_sp_kheight;
		cs_comment = PrintSpkShapeBars(p_dc, &m_rSpike, b_all);
		m_rSpike.right = m_rSpike.left + r_sp_kwidth;
		m_rSpike.left -= options_view_data->textseparator;
		m_rSpike.top = m_rSpike.bottom;
		m_rSpike.bottom += m_logFont.lfHeight * 3;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), m_rSpike, n_format);
		p_dc->SetTextAlign(ui_flag);
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

		// update file parameters for next row --------------------------------------------

		r_where.OffsetRect(0, r_height + options_view_data->heightSeparator);
		const auto ifile = filenumber;
		if (!PrintGetNextRow(filenumber, l_first, very_last))
		{
			//i = m_nbrowsperpage;
			break;
		}
		if (ifile != filenumber)
		{
			update_file_parameters(FALSE);
			m_ChartDataWnd_Detect.GetChanlistItem(0)->SetflagPrintVisible(0); // cancel printing channel zero
		}
	}

	// end of file loop : restore initial conditions
	m_ChartDataWnd_Detect.GetChanlistItem(0)->SetflagPrintVisible(1);
	if (!options_view_data->bFilterDataSource)
		m_ChartDataWnd_Detect.SetChanlistTransformMode(0, m_p_detect_parameters->detectTransform);

	if (m_pOldFont != nullptr)
		p_dc->SelectObject(m_pOldFont);
}

void CViewSpikeDetection::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	// restore file from index and display parameters
	GetDocument()->SetDB_CurrentRecordPosition(m_file0);

	m_ChartDataWnd_Detect.ResizeChannels(m_npixels0, 0);
	m_ChartDataWnd_Detect.GetDataFromDoc(m_lFirst0, m_lLast0);
	m_ChartSpkWnd_Shape.SetTimeIntervals(m_lFirst0, m_lLast0);
	update_file_parameters(TRUE);

	m_bIsPrinting = FALSE;
	serialize_windows_state(b_restore);
}

void CViewSpikeDetection::OnBnClickedBiasbutton()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN))->SetState(0);
	SetVBarMode(BAR_BIAS, IDC_SCROLLY);
}

void CViewSpikeDetection::OnBnClickedBias2()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS2))->SetState(1);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN2))->SetState(0);
	SetVBarMode(BAR_BIAS, IDC_SCROLLY2);
}

void CViewSpikeDetection::OnBnClickedGainbutton()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN))->SetState(1);
	SetVBarMode(BAR_GAIN, IDC_SCROLLY);
}

void CViewSpikeDetection::OnBnClickedGain2()
{
	static_cast<CButton*>(GetDlgItem(IDC_BIAS2))->SetState(0);
	static_cast<CButton*>(GetDlgItem(IDC_GAIN2))->SetState(1);
	SetVBarMode(BAR_GAIN, IDC_SCROLLY2);
}

void CViewSpikeDetection::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
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
		CdbTableView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CViewSpikeDetection::SetVBarMode(short bMode, int iID)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll(iID);
}

void CViewSpikeDetection::UpdateGainScroll(int iID)
{
	if (iID == IDC_SCROLLY)
		m_scrolly.SetScrollPos(
			MulDiv(m_ChartDataWnd_Detect.GetChanlistItem(m_ichanselected)->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
	else
		m_scrolly2.SetScrollPos(
			MulDiv(m_ChartDataWnd_Detect.GetChanlistItem(m_ichanselected2)->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
}

void CViewSpikeDetection::OnGainScroll(UINT nSBCode, UINT nPos, int iID)
{
	ChartData* p_view = &m_ChartDataWnd_Detect;
	int ichan = m_ichanselected;
	if (iID == IDC_SCROLLY2)
	{
		p_view = &m_ChartDataWnd_Source;
		ichan = m_ichanselected2;
	}
	int lSize = p_view->GetChanlistItem(ichan)->GetYextent();

	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: lSize = YEXTENT_MIN;
		break; // .................scroll to the start
	case SB_LINELEFT: lSize -= lSize / 10 + 1;
		break; // .................scroll one line left
	case SB_LINERIGHT: lSize += lSize / 10 + 1;
		break; // .................scroll one line right
	case SB_PAGELEFT: lSize -= lSize / 2 + 1;
		break; // .................scroll one page left
	case SB_PAGERIGHT: lSize += lSize + 1;
		break; // .................scroll one page right
	case SB_RIGHT: lSize = YEXTENT_MAX;
		break; // .................scroll to end right
	case SB_THUMBPOSITION: // .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBTRACK: lSize = MulDiv(nPos - 50, YEXTENT_MAX, 100);
		break;
	default: break; // .................NOP: set position only
	}
	// change y extent
	if (lSize > 0) //&& lSize<=YEXTENT_MAX)
	{
		p_view->GetChanlistItem(ichan)->SetYextent(lSize);
		update_legends();
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		UpdateGainScroll(iID);
}

void CViewSpikeDetection::UpdateBiasScroll(int iID)
{
	if (iID == IDC_SCROLLY)
	{
		CChanlistItem* pchan = m_ChartDataWnd_Detect.GetChanlistItem(m_ichanselected);
		const auto i_pos = static_cast<int>((pchan->GetYzero() - pchan->GetDataBinZero())
			* 100 / int(YZERO_SPAN)) + static_cast<int>(50);
		m_scrolly.SetScrollPos(i_pos, TRUE);
	}
	else
	{
		CChanlistItem* pchan = m_ChartDataWnd_Detect.GetChanlistItem(m_ichanselected2);
		const auto i_pos = static_cast<int>((pchan->GetYzero() - pchan->GetDataBinZero())
			* 100 / int(YZERO_SPAN)) + static_cast<int>(50);
		m_scrolly2.SetScrollPos(i_pos, TRUE);
	}
}

void CViewSpikeDetection::OnBiasScroll(UINT nSBCode, UINT nPos, int iID)
{
	auto p_view = &m_ChartDataWnd_Detect;
	auto ichan = m_ichanselected;
	if (iID == IDC_SCROLLY2)
	{
		p_view = &m_ChartDataWnd_Source;
		ichan = m_ichanselected2;
	}

	CChanlistItem* pchan = m_ChartDataWnd_Detect.GetChanlistItem(ichan);
	auto l_size = pchan->GetYzero() - pchan->GetDataBinZero();
	const auto yextent = pchan->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: l_size = YZERO_MIN;
		break; // scroll to the start
	case SB_LINELEFT: l_size -= yextent / 100 + 1;
		break; // scroll one line left
	case SB_LINERIGHT: l_size += yextent / 100 + 1;
		break; // scroll one line right
	case SB_PAGELEFT: l_size -= yextent / 10 + 1;
		break; // scroll one page left
	case SB_PAGERIGHT: l_size += yextent / 10 + 1;
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
		CChanlistItem* chan = p_view->GetChanlistItem(ichan);
		chan->SetYzero(l_size + chan->GetDataBinZero());
		p_view->Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		UpdateBiasScroll(iID);
}

void CViewSpikeDetection::OnEnChangeSpkWndAmplitude()
{
	if (mm_spkWndAmplitude.m_bEntryDone)
	{
		auto y = m_ChartSpkWnd_Shape.GetExtent_mV();
		const auto yold = y;
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
			y = yold;
			MessageBeep(-1);
		}
		const auto y_we = static_cast<int>(static_cast<float>(m_ChartSpkWnd_Shape.GetYWExtent()) * y / yold);
		m_ChartSpkWnd_Shape.SetYWExtOrg(y_we, m_ChartSpkWnd_Shape.GetYWOrg());
		m_ChartSpkWnd_Shape.SetyScaleUnitValue(y / m_ChartSpkWnd_Shape.GetNyScaleCells());
		m_ChartSpkWnd_Shape.Invalidate();

		// update the dialog control
		mm_spkWndAmplitude.m_bEntryDone = FALSE;
		mm_spkWndAmplitude.m_nChar = 0;
		mm_spkWndAmplitude.SetSel(0, -1); //select all text
		cs.Format(_T("%.3f"), y);
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->SetWindowText(cs);
	}
}

void CViewSpikeDetection::OnEnChangeSpkWndLength()
{
	if (mm_spkWndDuration.m_bEntryDone)
	{
		auto x = m_ChartSpkWnd_Shape.GetExtent_ms();
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
		const auto x_we = static_cast<int>(static_cast<float>(m_ChartSpkWnd_Shape.GetXWExtent()) * x / xold);
		m_ChartSpkWnd_Shape.SetXWExtOrg(x_we, m_ChartSpkWnd_Shape.GetXWOrg());
		m_ChartSpkWnd_Shape.SetxScaleUnitValue(x / m_ChartSpkWnd_Shape.GetNxScaleCells());
		m_ChartSpkWnd_Shape.Invalidate();

		// update the dialog control
		mm_spkWndDuration.m_bEntryDone = FALSE;
		mm_spkWndDuration.m_nChar = 0;
		mm_spkWndDuration.SetSel(0, -1); //select all text
		cs.Format(_T("%.3f"), x);
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->SetWindowText(cs);
	}
}

void CViewSpikeDetection::OnBnClickedLocatebttn()
{
	int max, min;
	CChanlistItem* pchan = m_ChartDataWnd_Detect.GetChanlistItem(0);
	pchan->GetMaxMin(&max, &min);

	// modify value
	m_p_detect_parameters->detectThreshold = (max + min) / 2;
	m_thresholdval = pchan->ConvertDataBinsToVolts(m_p_detect_parameters->detectThreshold) * 1000.f;
	m_p_detect_parameters->detectThresholdmV = m_thresholdval;
	// update user-interface: edit control and threshold bar in sourceview
	CString cs;
	cs.Format(_T("%.3f"), m_thresholdval);
	GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
	m_ChartDataWnd_Detect.MoveHZtagtoVal(0, m_p_detect_parameters->detectThreshold);
	m_ChartDataWnd_Detect.Invalidate();
}

void CViewSpikeDetection::update_detection_settings(int iSelParms)
{
	// check size of spike detection parameters
	if (iSelParms >= m_spk_detect_array_current.GetSize())
	{
		// load new set of parameters from spike list
		const auto isize = m_pSpkDoc->GetSpkList_Size();
		m_spk_detect_array_current.SetSize(isize);
		for (int i = 0; i < isize; i++)
		{
			// select new spike list (list with no spikes for stimulus channel)
			auto pspklist = m_pSpkDoc->SetSpkList_AsCurrent(i);
			ASSERT(pspklist != NULL);
			const auto ps_d = pspklist->GetDetectParms();
			m_spk_detect_array_current.SetItem(i, ps_d); // copy content of spklist parm into m_spk_detect_array_current
		}
	}

	// loop over each spike detection set to update spike detection parameters
	for (auto i = 0; i < m_spk_detect_array_current.GetSize(); i++)
	{
		// select new spike list (list with no spikes for stimulus channel)
		auto pspklist = m_pSpkDoc->SetSpkList_AsCurrent(i);
		const auto p_sd = m_spk_detect_array_current.GetItem(i);
		if (pspklist == nullptr)
		{
			m_pSpkDoc->AddSpkList();
			pspklist = m_pSpkDoc->GetSpkList_Current();
			pspklist->InitSpikeList(GetDocument()->m_pDat, p_sd);
		}
		else
			pspklist->SetDetectParms(p_sd);
	}

	// set new parameters
	m_pSpkList->m_selected_spike = m_spikeno; // save spike selected
	m_i_detect_parameters = iSelParms;
	//GetDocument()->GetCurrent_Spk_Document()->SetSpkList_CurrentIndex(iSelParms);
	m_p_detect_parameters = m_spk_detect_array_current.GetItem(iSelParms);
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(iSelParms);
	if (m_pSpkList != nullptr)
		highlight_spikes(TRUE);

	update_detection_controls();
	update_legends();
}

void CViewSpikeDetection::update_detection_controls()
{
	m_CBdetectWhat.SetCurSel(m_pSpkList->GetdetectWhat());
	m_CBdetectChan.SetCurSel(m_pSpkList->GetdetectChan());
	m_CBtransform.SetCurSel(m_pSpkList->GetdetectTransform()); // this should remains = to detection parameters

	// check that spike detection parameters are compatible with current data doc
	auto detectchan = m_pSpkList->GetdetectChan();
	const auto p_dat = GetDocument()->m_pDat;
	const int maxchan = p_dat->GetpWaveFormat()->scan_count;
	if (detectchan >= maxchan)
		detectchan = 0;

	m_ChartDataWnd_Detect.SetChanlistOrdinates(0, detectchan, m_pSpkList->GetdetectTransform());
	m_CBtransform2.SetCurSel(m_pSpkList->GetextractTransform());

	CChanlistItem* pchan = m_ChartDataWnd_Detect.GetChanlistItem(0);
	pchan->SetColor(detectchan);

	m_ChartDataWnd_Detect.GetDataFromDoc(); // load data
	//if (options_view_data->bSplitCurves)
	//	m_displayDataFile.SplitChans();

	const auto ithreshold = m_pSpkList->GetdetectThreshold();
	m_thresholdval = pchan->ConvertDataBinsToVolts(ithreshold) * 1000.f;
	if (m_ChartDataWnd_Detect.m_HZtags.GetNTags() < 1)
		m_ChartDataWnd_Detect.m_HZtags.AddTag(ithreshold, 0);
	else
		m_ChartDataWnd_Detect.m_HZtags.SetTagVal(0, ithreshold);

	// update spike channel displayed
	m_ChartSpkWnd_Bar.SetSpkList(m_pSpkList);
	m_ChartSpkWnd_Shape.SetSpkList(m_pSpkList);
}

void CViewSpikeDetection::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	serialize_windows_state(b_save, m_i_detect_parameters);
	const auto selected_tab = m_tabCtrl.GetCurSel();
	serialize_windows_state(b_restore, selected_tab);
	update_detection_settings(selected_tab);
	*pResult = 0;
}

void CViewSpikeDetection::OnToolsEditstimulus()
{
	m_pSpkDoc->SortStimArray();

	DlgEditStimArray dlg;
	dlg.intervals = m_pSpkDoc->m_stimIntervals;
	dlg.m_sampling_rate = m_samplingRate;
	dlg.intervals_saved = GetDocument()->m_stimsaved;
	;
	if (IDOK == dlg.DoModal())
	{
		m_pSpkDoc->m_stimIntervals = dlg.intervals;
		update_VT_tags();
		m_ChartSpkWnd_Bar.Invalidate();
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.Invalidate();
		m_pSpkDoc->SetModifiedFlag(TRUE);
	}
}
void CViewSpikeDetection::OnEnChangeChanselected()
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

void CViewSpikeDetection::OnEnChangeChanselected2()
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

void CViewSpikeDetection::OnCbnSelchangeTransform2()
{
	const auto method = m_CBtransform2.GetCurSel(); // this is the extract method requested
	auto p_dat_doc = GetDocument()->m_pDat;

	const auto doc_chan = m_pSpkList->GetextractChan(); // source channel
	//auto const nchans = p_dat_doc->GetpWaveFormat()->scan_count;	// number of data chans / source buffer
	const short offset = 1; //(method>0) ? 1 : nchans;				// offset between points / detection
	const short nspan = p_dat_doc->GetTransfDataSpan(method); // nb pts to read before transf
	m_pSpkList->SetextractTransform(method); // save transform method

	// pre-load data
	const auto spikelen = m_pSpkList->GetSpikeLength();
	const auto spkpretrig = m_pSpkList->GetSpikePretrig();
	auto iitime = m_pSpkList->GetSpikeTime(0) - spkpretrig;
	auto l_rw_first0 = iitime - spikelen;
	auto l_rw_last0 = iitime + spikelen;
	if (!p_dat_doc->LoadRawData(&l_rw_first0, &l_rw_last0, nspan))
		return; // exit if error reported
	auto p_data = p_dat_doc->LoadTransfData(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto totalspikes = m_pSpkList->GetTotalSpikes();
	for (auto ispk = 0; ispk < totalspikes; ispk++)
	{
		// make sure that source data are loaded and get pointer to it (p_data)
		iitime = m_pSpkList->GetSpikeTime(ispk);
		auto l_rw_first = iitime - spkpretrig; // first point
		auto l_rw_last = l_rw_first + spikelen; // last pt needed
		if (!p_dat_doc->LoadRawData(&l_rw_first, &l_rw_last, nspan))
			break; // exit loop if error reported

		p_data = p_dat_doc->LoadTransfData(l_rw_first, l_rw_last, method, doc_chan);
		const auto p_data_spike0 = p_data + (iitime - spkpretrig - l_rw_first) * offset;
		m_pSpkList->TransferDataToSpikeBuffer(ispk, p_data_spike0, offset);
		// nchans should be 1 if they come from the transform buffer as data are not interleaved...
		m_pSpkList->CenterSpikeAmplitude(ispk, 0, spikelen, 1); // 1=center average
	}
	m_pSpkDoc->SetModifiedFlag(TRUE);

	int max, min;
	m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	m_ChartSpkWnd_Shape.SetYWExtOrg(m_ChartSpkWnd_Shape.GetYWExtent(), middle);
	m_ChartSpkWnd_Bar.SetYWExtOrg(m_ChartSpkWnd_Shape.GetYWExtent(), middle);
	update_spike_shape_window_scale(FALSE);

	highlight_spikes(TRUE);
	update_legends();
	UpdateTabs();
}

void CViewSpikeDetection::UpdateTabs()
{
	// load initial data
	const BOOL b_replace = (m_tabCtrl.GetItemCount() == m_pSpkDoc->GetSpkList_Size());
	if (!b_replace)
		m_tabCtrl.DeleteAllItems();

	// load list of detection parameters
	const auto currlist = m_pSpkDoc->GetSpkList_CurrentIndex();
	for (auto i = 0; i < m_pSpkDoc->GetSpkList_Size(); i++)
	{
		CString cs;
		const auto current_spike_list = m_pSpkDoc->SetSpkList_AsCurrent(i);
		cs.Format(_T("#%i %s"), i, (LPCTSTR)current_spike_list->GetComment());
		if (!b_replace)
			m_tabCtrl.InsertItem(i, cs);
		else
		{
			TCITEM tab_ctrl_item;
			tab_ctrl_item.mask = TCIF_TEXT;
			const auto lptsz_str = cs.GetBuffer(0);
			tab_ctrl_item.pszText = lptsz_str;
			m_tabCtrl.SetItem(i, &tab_ctrl_item);
			cs.ReleaseBuffer();
		}
	}
	m_pSpkDoc->SetSpkList_AsCurrent(currlist);

	m_i_detect_parameters = GetDocument()->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();
	m_tabCtrl.SetCurSel(m_i_detect_parameters);
}
