// * m_displayData   display source data (all channels)
// * m_displayDetect holds a display of source data displayed as transformed for spike detection
// * spk_bar_wnd_ display spikes detected
// * m_spkShapeView display spikes detected (superimposed)
//
// TODO
// convert threshold into volts & back to binary (cope with variable gains)
// cf: UpdateFileParameters

/////////////////////////////////////////////////////
#include "StdAfx.h"
//#include <math.h>
//#include "Cscale.h"
//#include "dbMainTable.h"
//#include "ChildFrm.h"
#include "dbWave.h"
#include "dbWave_constants.h"
#include "resource.h"
#include "chart.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "Xyparame.h"
#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "Spikedetec.h"
#include "Editspik.h"
#include "Vdseries.h"
#include "Copyasdl.h"
#include "MainFrm.h"
#include "DlgEditStimArray.h"
#include "DlgProg.h"
#include "ViewSpikeDetect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto BRESTORE = 0;
constexpr auto BSAVE = 1;

IMPLEMENT_DYNCREATE(CViewSpikeDetection, CDaoRecordView)

CViewSpikeDetection::CViewSpikeDetection()
	: CDaoRecordView(CViewSpikeDetection::IDD)
{
	m_bEnableActiveAccessibility = FALSE;
}

CViewSpikeDetection::~CViewSpikeDetection()
{
	if (p_spike_doc_ != nullptr)
		SaveCurrentSpikeFile();
}

BOOL CViewSpikeDetection::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
		//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

void CViewSpikeDetection::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);

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

BEGIN_MESSAGE_MAP(CViewSpikeDetection, CDaoRecordView)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()

	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)

	ON_COMMAND(ID_FORMAT_FIRSTFRAME, OnFirstFrame)
	ON_COMMAND(ID_FORMAT_LASTFRAME, OnLastFrame)
	ON_COMMAND(ID_FORMAT_SETORDINATES, OnFormatXscale)
	ON_COMMAND(ID_FORMAT_ALLDATA, OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, OnFormatYscaleCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, OnFormatYscaleGainadjust)
	ON_COMMAND(ID_FORMAT_SPLITCURVES, OnFormatSplitcurves)
	ON_COMMAND(ID_TOOLS_DETECT_PARMS, OnToolsDetectionparameters)
	ON_COMMAND(ID_TOOLS_DETECT, OnMeasureAll)
	ON_COMMAND(ID_TOOLS_EDITSTIMULUS, OnToolsEditstimulus)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, OnToolsEdittransformspikes)
	ON_COMMAND(ID_TOOLS_DATASERIES, OnToolsDataseries)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FORMAT_XSCALE, OnFormatXscale)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_CBN_SELCHANGE(IDC_SOURCECHAN, OnSelchangeDetectchan)
	ON_CBN_SELCHANGE(IDC_TRANSFORM, OnSelchangeTransform)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeDetectMode)
	ON_CBN_SELCHANGE(IDC_TRANSFORM2, &CViewSpikeDetection::OnCbnSelchangeTransform2)

	ON_EN_CHANGE(IDC_THRESHOLDVAL, OnEnChangeThresholdval)
	ON_EN_CHANGE(IDC_TIMEFIRST, OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_SPIKENO, OnEnChangeSpikeno)
	ON_EN_CHANGE(IDC_SPIKEWINDOWAMPLITUDE, OnEnChangeSpkWndAmplitude)
	ON_EN_CHANGE(IDC_SPIKEWINDOWLENGTH, OnEnChangeSpkWndLength)
	ON_EN_CHANGE(IDC_CHANSELECTED, OnEnChangeChanselected)
	ON_EN_CHANGE(IDC_CHANSELECTED2, &CViewSpikeDetection::OnEnChangeChanselected2)

	ON_BN_CLICKED(IDC_MEASUREALL, OnMeasureAll)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_ARTEFACT, OnArtefact)
	ON_BN_CLICKED(IDC_BIAS, OnBnClickedBiasbutton)
	ON_BN_CLICKED(IDC_GAIN, OnBnClickedGainbutton)
	ON_BN_CLICKED(IDC_LOCATEBTTN, OnBnClickedLocatebttn)
	ON_BN_CLICKED(IDC_CLEARALL, OnBnClickedClearall)
	ON_BN_CLICKED(IDC_MEASURE, &CViewSpikeDetection::OnMeasure)
	ON_BN_CLICKED(IDC_GAIN2, &CViewSpikeDetection::OnBnClickedGain2)
	ON_BN_CLICKED(IDC_BIAS2, &CViewSpikeDetection::OnBnClickedBias2)

	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CViewSpikeDetection::OnNMClickTab1)
END_MESSAGE_MAP()

void CViewSpikeDetection::OnFileSave()
{
	CFile f;
	CFileDialog dlg(FALSE,
		_T("spk"),										// default filename extension
		GetDocument()->GetDB_CurrentSpkFileName(),		// initial file name
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Awave Spikes (*.spk) | *.spk |All Files (*.*) | *.* ||"));

	if (IDOK == dlg.DoModal())
	{
		p_spike_doc_->OnSaveDocument(dlg.GetPathName());
		GetDocument()->SetDB_nbspikes(p_spike_doc_->GetSpkList_Current()->GetTotalSpikes());
		GetDocument()->SetDB_nbspikeclasses(1);
		p_spike_doc_->SetModifiedFlag(FALSE);
	}
}

BOOL CViewSpikeDetection::OnMove(UINT nIDMoveCommand)
{
	SaveCurrentSpikeFile();
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	GetDocument()->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CViewSpikeDetection::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_binit) {
		switch (LOWORD(lHint))
		{
		case HINT_CLOSEFILEMODIFIED:	// close modified file: save
			SaveCurrentSpikeFile();
			break;
		case HINT_DOCMOVERECORD:
		case HINT_DOCHASCHANGED:		// file has changed?
			UpdateFileParameters(TRUE);
			break;
		case HINT_REPLACEVIEW:
		default:
			break;
		}
	}
}

void CViewSpikeDetection::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// activate view
	if (bActivate)
	{
		auto p_main_frame = (CMainFrame*)AfxGetMainWnd();
		p_main_frame->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, (LPARAM)pActivateView->GetDocument());
	}
	// exit view
	else
	{
		SaveCurrentSpikeFile();
		SerializeWindowsState(BSAVE);
		const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
		p_app->options_viewspikes.bincrflagonsave = ((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck();
		((CdbWaveApp*)AfxGetApp())->options_viewdata.viewdata = *(m_ChartDataWnd_Source.GetScopeParameters());
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CViewSpikeDetection::UpdateLegends()
{
	const auto l_first = m_ChartDataWnd_Source.GetDataFirst();		// get source data time range
	const auto l_last = m_ChartDataWnd_Source.GetDataLast();
	m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);

	// draw dependent buttons
	m_displaySpk_BarView.SetTimeIntervals(l_first, l_last);		// spike bars
	m_displaySpk_Shape.SetTimeIntervals(l_first, l_last);		// superimposed spikes
	UpdateSpkShapeWndScale(FALSE); 						// get parms from file/prevent spkshape draw

	// update text abcissa and horizontal scroll position
	m_timefirst = l_first / m_samplingRate;
	m_timelast = (l_last + 1) / m_samplingRate;
	m_spikeno = p_spikelist_->m_selspike;
	m_bartefact = FALSE;
	if (m_spikeno > 0)
	{
		const auto p_s = p_spikelist_->GetSpikeElemt(m_spikeno);
		m_bartefact = (p_s->get_class() < 0);
	}

	UpdateFileScroll();		// adjust scroll bar to file position
	UpdateCB();				// update combo box
	UpdateVTtags();			// set VT tags
	UpdateLegendDetectionWnd();

	// update data displayed
	m_displaySpk_BarView.Invalidate();
	m_ChartDataWnd_Detect.Invalidate();
	m_ChartDataWnd_Source.Invalidate();
	m_displaySpk_Shape.Invalidate();
	UpdateData(FALSE);

	// update number of spikes
	const auto nspikes = UINT(p_spikelist_->GetTotalSpikes());
	if (nspikes != GetDlgItemInt(IDC_NBSPIKES_NB))
	{
		SetDlgItemInt(IDC_NBSPIKES_NB, nspikes);
		GetDlgItem(IDC_NBSPIKES_NB)->Invalidate();
	}
}

void CViewSpikeDetection::SaveCurrentSpikeFile()
{
	auto p_doc = GetDocument();

	// save spike file if anything has changed
	if (p_spike_doc_ != nullptr && (p_spike_doc_->IsModified() || m_bDetected))
	{
		// save file data name
		const auto pdat_doc = GetDocument()->m_pDat;
		const auto docname = GetDocument()->GetDB_CurrentDatFileName();
		p_spike_doc_->m_acqfile = docname;
		p_spike_doc_->InitSourceDoc(pdat_doc);						// init file doc, etc
		p_spike_doc_->SetDetectionDate(CTime::GetCurrentTime());	// detection date

		const auto filename = p_doc->SetDB_CurrentSpikeFileName();
		p_spike_doc_->OnSaveDocument(filename);

		// save nb spikes into database
		/*int nlist = m_pspkDocVSD->GetSpkListSize();*/
		p_spikelist_ = p_spike_doc_->SetSpkList_AsCurrent(0);
		const auto nspikes = p_spikelist_->GetTotalSpikes();
		GetDocument()->SetDB_nbspikes(nspikes);
		if (!p_spikelist_->IsClassListValid())
			p_spikelist_->UpdateClassList();
		const auto nbclasses = p_spikelist_->GetNbclasses();
		GetDocument()->SetDB_nbspikeclasses(nbclasses);
		m_bDetected = FALSE;

		if (((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck())
		{
			auto flag = GetDocument()->GetDB_CurrentRecordFlag();
			flag++;
			GetDocument()->SetDB_CurrentRecordFlag(flag);
		}
	}

	// save spike detection parameters
	const auto parray = m_pArrayFromApp->GetChanArray(m_scancount_doc);
	*parray = m_parmsCurrent;
}

void CViewSpikeDetection::UpdateSpikeFile(BOOL bUpdateInterface)
{
	// update spike doc and temporary spike list
	auto pdb_doc = GetDocument();
	//CString filename = pdb_doc->GetDB_CurrentSpkFileName();
	if (pdb_doc->OpenCurrentSpikeFile() == nullptr)
	{
		// file not found: create new object, and create file
		auto* pspk = new CSpikeDoc;
		ASSERT(pspk != NULL);
		pdb_doc->m_pSpk = pspk;
		p_spike_doc_ = pspk;
		p_spike_doc_->OnNewDocument();
		p_spike_doc_->ClearData();
		if (options_viewdata->bDetectWhileBrowse)
			OnMeasureAll();
	}
	else
	{
		p_spike_doc_ = pdb_doc->m_pSpk;
		p_spike_doc_->SetModifiedFlag(FALSE);
		p_spike_doc_->SetPathName(pdb_doc->GetDB_CurrentSpkFileName(), FALSE);
	}

	// select a spikelist
	//int icurspklist = GetDocument()->GetcurrentSpkDocument()->GetSpkList_CurrentIndex();
	p_spikelist_ = p_spike_doc_->GetSpkList_Current();
	if (p_spikelist_ == nullptr && p_spike_doc_->GetSpkList_Size() > 0)
	{
		// at least one spike list is available, select list[0]
		p_spikelist_ = p_spike_doc_->SetSpkList_AsCurrent(0);
	}

	// no spikes list available, create one
	if (p_spikelist_ == nullptr)
	{
		// create new list here
		ASSERT(p_spike_doc_->GetSpkList_Size() == 0);
		const auto isize = m_parmsCurrent.GetSize();
		p_spike_doc_->SetSpkList_Size(isize);
		for (auto i = 0; i < isize; i++)
		{
			auto p_l = p_spike_doc_->SetSpkList_AsCurrent(i);
			if (p_l == nullptr)
			{
				p_spike_doc_->AddSpkList();
				p_l = p_spike_doc_->GetSpkList_Current();
			}
			p_l->InitSpikeList(pdb_doc->m_pDat, m_parmsCurrent.GetItem(i));
		}
		p_spikelist_ = p_spike_doc_->SetSpkList_AsCurrent(0);
		ASSERT(p_spikelist_ != nullptr);
	}

	m_displaySpk_BarView.SetSourceData(p_spikelist_, pdb_doc);
	m_displaySpk_BarView.SetPlotMode(PLOT_BLACK, 0);
	m_displaySpk_Shape.SetSourceData(p_spikelist_, pdb_doc);
	m_displaySpk_Shape.SetPlotMode(PLOT_BLACK, 0);
	UpdateVTtags();

	// update interface elements
	m_spikeno = -1;
	if (bUpdateInterface)
	{
		UpdateTabs();
		UpdateDetectionControls();
		HighlightSpikes(TRUE);
	}

	m_displaySpk_BarView.Invalidate();
	m_displaySpk_Shape.Invalidate();

	// update nb spikes
	auto const nspikes = UINT(p_spikelist_->GetTotalSpikes());
	if (nspikes != GetDlgItemInt(IDC_NBSPIKES_NB))
		SetDlgItemInt(IDC_NBSPIKES_NB, nspikes);
}

void CViewSpikeDetection::HighlightSpikes(BOOL flag)
{
	CDWordArray* p_d_wintervals = nullptr;	  // remove setting if no spikes of if flag is false
	if (flag && p_spikelist_ != nullptr && p_spikelist_->GetTotalSpikes() < 1)
	{
		p_d_wintervals = &m_DWintervals;

		// init array
		const auto arraysize = p_spikelist_->GetTotalSpikes() * 2 + 3;
		m_DWintervals.SetSize(arraysize);				// prepare space for all spike interv
		m_DWintervals.SetAt(0, 0);						// source channel
		m_DWintervals.SetAt(1, DWORD(RGB(255, 0, 0)));	// red color
		m_DWintervals.SetAt(2, 1);						// pen size
		const auto max = p_spikelist_->GetTotalSpikes(); 	// nb of spikes
		auto jindex = 3;									// index / DWord array
		auto spklen = p_spikelist_->GetSpikeLength();		// pre-trig and spike length
		const auto spkpretrig = p_spikelist_->GetSpikePretrig();
		spklen--;

		for (auto i = 0; i < max; i++)
		{
			const auto p_s = p_spikelist_->GetSpikeElemt(i);
			const auto l_first = p_s->get_time() - spkpretrig;
			m_DWintervals.SetAt(jindex, l_first);
			jindex++;
			m_DWintervals.SetAt(jindex, l_first + spklen);
			jindex++;
		}

		// tell sourceview to highlight spk
		m_ChartDataWnd_Detect.SetHighlightData(p_d_wintervals);
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.SetHighlightData(p_d_wintervals);
		m_ChartDataWnd_Source.Invalidate();
	}
}

void CViewSpikeDetection::UpdateFileParameters(BOOL bUpdateInterface)
{
	UpdateDataFile(bUpdateInterface);
	UpdateSpikeFile(bUpdateInterface);
	if (bUpdateInterface)
		UpdateLegends();
}

BOOL CViewSpikeDetection::CheckDetectionSettings()
{
	auto flag = TRUE;
	ASSERT(m_pDetectParms != NULL);
	ASSERT_VALID(m_pDetectParms);
	if (nullptr == m_pDetectParms)
	{
		m_iDetectParms = GetDocument()->GetcurrentSpkDocument()->GetSpkList_CurrentIndex();
		m_pDetectParms = m_parmsCurrent.GetItem(m_iDetectParms);
	}

	// get infos from data file
	auto p_data_file = GetDocument()->m_pDat;
	p_data_file->ReadDataInfos();
	const auto pwave_format = p_data_file->GetpWaveFormat();

	// check detection and extraction channels
	if (m_pDetectParms->detectChan < 0
		|| m_pDetectParms->detectChan >= pwave_format->scan_count)
	{
		m_pDetectParms->detectChan = 0;
		AfxMessageBox(_T("Spike detection parameters: detection channel modified"));
		flag = FALSE;
	}

	if (m_pDetectParms->extractChan < 0
		|| m_pDetectParms->extractChan >= pwave_format->scan_count)
	{
		m_pDetectParms->extractChan = 0;
		AfxMessageBox(_T("Spike detection parameters: channel extracted modified"));
		flag = FALSE;
	}
	return flag;
}

void CViewSpikeDetection::UpdateDataFile(BOOL bUpdateInterface)
{
	auto pdb_doc = GetDocument();
	//CString filename = pdb_doc->GetDB_CurrentDatFileName();
	if (pdb_doc->OpenCurrentDataFile() == nullptr)
		return;

	const auto p_data_file = pdb_doc->m_pDat;
	p_data_file->ReadDataInfos();
	const auto pchan_array = p_data_file->GetpWavechanArray();
	const auto pwave_format = p_data_file->GetpWaveFormat();

	// if the number of data channels of the data source has changed, load a new set of parameters
	// keep one array of spike detection parameters per data acquisition configuration (ie nb of acquisition channels)
	if (m_scancount_doc != pwave_format->scan_count)
	{
		// save current set of parameters if scancount >= 0 this might not be necessary
		const auto parray = m_pArrayFromApp->GetChanArray(m_scancount_doc);
		if (m_scancount_doc >= 0)
			*parray = m_parmsCurrent;
		// Get parms from the application array
		m_scancount_doc = pwave_format->scan_count;
		m_parmsCurrent = *(m_pArrayFromApp->GetChanArray(m_scancount_doc));
		// select by default the first set of detection parms
		m_iDetectParms = 0;
		m_pDetectParms = m_parmsCurrent.GetItem(m_iDetectParms);
	}

	// check if detection parms are valid
	CheckDetectionSettings();

	// update combo boxes
	if (m_CBdetectChan.GetCount() != pwave_format->scan_count)
	{
		UpdateCombosDetectChanAndTransforms();
	}
	// change doc attached to lineviewbutton
	m_ChartDataWnd_Detect.AttachDataFile(p_data_file);
	m_ChartDataWnd_Source.AttachDataFile(p_data_file);

	// update sourceview display
	if (m_ChartDataWnd_Detect.GetChanlistSize() < 1)
	{
		m_ChartDataWnd_Detect.RemoveAllChanlistItems();
		m_ChartDataWnd_Detect.AddChanlistItem(0, 0);
		m_ChartDataWnd_Detect.SetChanlistColor(0, 0);
		m_ChartDataWnd_Detect.DelAllHZtags();
		m_pDetectParms->detectThreshold = m_ChartDataWnd_Detect.ConvertChanlistVoltstoDataBins(0, m_pDetectParms->detectThresholdmV / 1000.f);
		m_ChartDataWnd_Detect.AddHZtag(m_pDetectParms->detectThreshold, 0);
	}

	//add all channels to detection window
	auto lnvchans = m_ChartDataWnd_Source.GetChanlistSize();
	const int ndocchans = pwave_format->scan_count;
	for (auto i = 0; i < ndocchans; i++)
	{										// check if present in the list
		auto b_present = FALSE;				// pessimistic
		for (auto j = lnvchans - 1; j >= 0; j--)// check all channels / display list
		{									// test if this data chan is present + no transf
			if (m_ChartDataWnd_Source.GetChanlistSourceChan(j) == i
				&& m_ChartDataWnd_Source.GetChanlistTransformMode(j) == 0)
			{
				b_present = TRUE;			// the wanted chan is present: stop loopint through disp list
				break;						// and examine next doc channel
			}
		}
		if (b_present == FALSE)				// no display chan contains that doc chan
		{
			m_ChartDataWnd_Source.AddChanlistItem(i, 0);	// add this channel
			lnvchans++;
		}
		m_ChartDataWnd_Source.SetChanlistColor(i, i);
	}

	// if browse through another file ; keep previous display parameters & load data
	auto l_first = m_ChartDataWnd_Detect.GetDataFirst();
	auto l_last = m_ChartDataWnd_Detect.GetDataLast();
	if (options_viewdata->bEntireRecord && bUpdateInterface)
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
		m_filescroll_infos.nMax = p_data_file->GetDOCchanLength() - 1;
		m_filescroll_infos.nPos = 0;
		m_filescroll_infos.nPage = m_ChartDataWnd_Detect.GetDataLast() - m_ChartDataWnd_Detect.GetDataFirst() + 1;
		m_filescroll.SetScrollInfo(&m_filescroll_infos);

		m_datacomments = pwave_format->GetComments(_T(" "));
		m_samplingRate = pwave_format->chrate;
		m_bValidThreshold = FALSE;
	}
}

void CViewSpikeDetection::UpdateCombosDetectChanAndTransforms()
{
	auto pdb_doc = GetDocument();
	const auto p_data_file = pdb_doc->m_pDat;
	const auto pchan_array = p_data_file->GetpWavechanArray();
	const auto pwave_format = p_data_file->GetpWaveFormat();

	// load channel names
	CString comment;
	m_CBdetectChan.ResetContent();
	const int imax = pwave_format->scan_count;
	for (auto i = 0; i < imax; i++)
	{
		comment.Format(_T("%i: "), i);
		comment += pchan_array->get_p_channel(i)->am_csComment;
		VERIFY(m_CBdetectChan.AddString(comment) != CB_ERR);
	}

	// load transforms names
	m_CBtransform.ResetContent();
	m_CBtransform2.ResetContent();
	const int jmax = p_data_file->GetTransfDataNTypes();
	for (auto j = 0; j < jmax; j++)
	{
		VERIFY(m_CBtransform.AddString(p_data_file->GetTransfDataName(j)) != CB_ERR);
		VERIFY(m_CBtransform2.AddString(p_data_file->GetTransfDataName(j)) != CB_ERR);
	}
	m_CBdetectChan.SetCurSel(m_pDetectParms->detectChan);
}

void CViewSpikeDetection::DefineStretchParameters()
{
	// top right ----------------------------------------
	m_stretch.AttachParent(this);				// attach formview pointer
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

void CViewSpikeDetection::DefineSubClassedItems()
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
	VERIFY(m_displaySpk_Shape.SubclassDlgItem(IDC_DISPLAYSPIKES, this));
	VERIFY(m_displaySpk_BarView.SubclassDlgItem(IDC_DISPLAYBARS, this));
	VERIFY(m_ChartDataWnd_Detect.SubclassDlgItem(IDC_DISPLAYDETECT, this));
	VERIFY(m_ChartDataWnd_Source.SubclassDlgItem(IDC_DISPLAYDATA, this));

	// load left scrollbar and button
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY, this));
	m_scrolly.SetScrollRange(0, 100);
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS)->PostMessage(BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)m_hBias);
	GetDlgItem(IDC_GAIN)->PostMessage(BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)m_hZoom);
	VERIFY(m_scrolly2.SubclassDlgItem(IDC_SCROLLY2, this));
	m_scrolly2.SetScrollRange(0, 100);
	m_hBias2 = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom2 = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS2)->PostMessage(BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)m_hBias2);
	GetDlgItem(IDC_GAIN2)->PostMessage(BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)m_hZoom2);
}

void CViewSpikeDetection::OnInitialUpdate()
{
	// load spike detection parameters from .INI file
	const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	m_pArrayFromApp = &(p_app->spkDA);				// spike detection array parms
	options_viewdata = &(p_app->options_viewdata);	// browse/print parms

	DefineStretchParameters();
	m_binit = TRUE;

	DefineSubClassedItems();
	// set bin_cr_flag_on_save
	((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->SetCheck(p_app->options_viewspikes.bincrflagonsave);

	// load data file parameters and build curves
	CDaoRecordView::OnInitialUpdate();

	// load file data
	if (m_ChartDataWnd_Detect.GetNHZtags() < 1)
		m_ChartDataWnd_Detect.AddHZtag(0, 0);

	UpdateFileParameters(TRUE);
	m_ChartDataWnd_Detect.SetScopeParameters(&(options_viewdata->viewdata));
	m_ChartDataWnd_Detect.Invalidate();
	m_ChartDataWnd_Source.SetScopeParameters(&(options_viewdata->viewdata));
	m_ChartDataWnd_Source.Invalidate();
}

void CViewSpikeDetection::OnDestroy()
{
	CDaoRecordView::OnDestroy();
}

void CViewSpikeDetection::OnSize(UINT nType, int cx, int cy)
{
	if (m_binit)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			m_stretch.ResizeControls(nType, cx, cy);
			break;
		default:
			break;
		}
	}
	CDaoRecordView::OnSize(nType, cx, cy);
}

#ifdef _DEBUG
void CViewSpikeDetection::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CViewSpikeDetection::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CViewSpikeDetection::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}

#endif //_DEBUG

CDaoRecordset* CViewSpikeDetection::OnGetRecordset()
{
	return GetDocument()->GetDB_Recordset();
}

LRESULT CViewSpikeDetection::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	int threshold = LOWORD(lParam);	// value associated
	const int i_id = HIWORD(lParam);

	// ----------------------------- change mouse cursor (all 3 items)
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
	{
		if (threshold > CURSOR_VERTICAL)	// clip cursor shape to max 2
			threshold = 0;
		if (threshold == CURSOR_MEASURE)
			threshold = CURSOR_VERTICAL;
		m_cursorstate = threshold;
		SetViewMouseCursor(threshold);	// change cursor val in the other button
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
	}
	break;

	// ----------------------------- move horizontal cursor / source data
	case HINT_MOVEHZTAG:
		m_pDetectParms->detectThreshold = m_ChartDataWnd_Detect.GetHZtagVal(threshold);
		m_thresholdval = m_ChartDataWnd_Detect.ConvertChanlistDataBinsToMilliVolts(0, m_ChartDataWnd_Detect.GetHZtagVal(threshold));
		m_pDetectParms->detectThresholdmV = m_thresholdval;
		mm_thresholdval.m_bEntryDone = TRUE;
		OnEnChangeThresholdval();
		break;

		// ----------------------------- select bar/display bars or zoom
	case HINT_CHANGEHZLIMITS:		// abcissa have changed
		m_ChartDataWnd_Detect.GetDataFromDoc(m_displaySpk_BarView.GetTimeFirst(), m_displaySpk_BarView.GetTimeLast());
		m_ChartDataWnd_Source.GetDataFromDoc(m_displaySpk_BarView.GetTimeFirst(), m_displaySpk_BarView.GetTimeLast());
		UpdateLegends();
		break;

	case HINT_HITSPIKE:				// spike is selected or deselected
		SelectSpikeNo(threshold, FALSE);
		UpdateSpikeDisplay();
		break;

	case HINT_DBLCLKSEL:
		if (threshold < 0)
			threshold = 0;
		m_spikeno = threshold;
		OnToolsEdittransformspikes();
		break;

	case HINT_CHANGEZOOM:
		UpdateSpkShapeWndScale(TRUE);
		m_displaySpk_Shape.Invalidate();
		break;

	case HINT_VIEWSIZECHANGED:
		if (i_id == m_ChartDataWnd_Source.GetDlgCtrlID())
		{
			const auto l_first = m_ChartDataWnd_Source.GetDataFirst();		// get source data time range
			const auto l_last = m_ChartDataWnd_Source.GetDataLast();
			m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);
		}
		// else if(iID == m_displayDetect.GetDlgCtrlID())
		// UpdateLegends updates data window from m_displayDetect
		UpdateLegends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_viewdata->viewspkdetectfiltered = *m_ChartDataWnd_Detect.GetScopeParameters();
		options_viewdata->viewspkdetectdata = *m_ChartDataWnd_Source.GetScopeParameters();
		options_viewdata->viewspkdetectspk = *m_displaySpk_BarView.GetScopeParameters();
		options_viewdata->viewspkdetectbars = *m_displaySpk_Shape.GetScopeParameters();
		break;

	case HINT_DEFINEDRECT:
		if (m_cursorstate == CURSOR_MEASURE)
		{
			const auto rect = m_ChartDataWnd_Detect.GetDefinedRect();
			int l_limit_left = m_ChartDataWnd_Detect.GetDataOffsetfromPixel(rect.left);
			int l_limit_right = m_ChartDataWnd_Detect.GetDataOffsetfromPixel(rect.right);
			if (l_limit_left > l_limit_right)
			{
				int i = l_limit_right;
				l_limit_right = l_limit_left;
				l_limit_left = i;
			}
			p_spike_doc_->m_stimIntervals.intervalsArray.SetAtGrow(p_spike_doc_->m_stimIntervals.nitems, l_limit_left);
			p_spike_doc_->m_stimIntervals.nitems++;
			p_spike_doc_->m_stimIntervals.intervalsArray.SetAtGrow(p_spike_doc_->m_stimIntervals.nitems, l_limit_right);
			p_spike_doc_->m_stimIntervals.nitems++;
			UpdateVTtags();

			m_displaySpk_BarView.Invalidate();
			m_ChartDataWnd_Detect.Invalidate();
			m_ChartDataWnd_Source.Invalidate();
			p_spike_doc_->SetModifiedFlag(TRUE);
		}
		break;

	case HINT_SELECTSPIKES:
		UpdateSpikeDisplay();
		break;

		//case HINT_HITVERTTAG:	 //11	// vertical tag hit				lowp = tag index
		//case HINT_MOVEVERTTAG: //12	// vertical tag has moved 		lowp = new pixel / selected tag
	case HINT_CHANGEVERTTAG: //13
	{
		int lvalue = p_spike_doc_->m_stimIntervals.intervalsArray.GetAt(threshold);
		if (i_id == m_ChartDataWnd_Detect.GetDlgCtrlID())
			lvalue = m_ChartDataWnd_Detect.GetVTtagLval(threshold);
		else if (i_id == m_ChartDataWnd_Source.GetDlgCtrlID())
			lvalue = m_ChartDataWnd_Source.GetVTtagLval(threshold);

		p_spike_doc_->m_stimIntervals.intervalsArray.SetAt(threshold, lvalue);
		UpdateVTtags();

		m_displaySpk_BarView.Invalidate();
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.Invalidate();
		p_spike_doc_->SetModifiedFlag(TRUE);
	}
	break;

	case WM_LBUTTONDOWN:
	case HINT_LMOUSEBUTTONDOW_CTRL:
	{
		const int cx = LOWORD(lParam);
		//int cy = HIWORD(lParam);
		const int l_limit_left = m_ChartDataWnd_Detect.GetDataOffsetfromPixel(cx);
		p_spike_doc_->m_stimIntervals.intervalsArray.SetAtGrow(p_spike_doc_->m_stimIntervals.nitems, l_limit_left);
		p_spike_doc_->m_stimIntervals.nitems++;
		UpdateVTtags();

		m_displaySpk_BarView.Invalidate();
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.Invalidate();
		p_spike_doc_->SetModifiedFlag(TRUE);
	}
	break;

	case HINT_HITSPIKE_SHIFT:				// spike is selected or deselected
	{
		long l_first;
		long l_last;
		p_spikelist_->GetRangeOfSpikeFlagged(l_first, l_last);
		const auto l_time = p_spikelist_->GetSpikeTime(threshold);
		if (l_time < l_first)
			l_first = l_time;
		if (l_time > l_last)
			l_last = l_time;
		p_spikelist_->FlagRangeOfSpikes(l_first, l_last, TRUE);
		UpdateSpikeDisplay();
	}
	break;

	case HINT_HITSPIKE_CTRL:				// add/remove selected spike to/from the group of spikes selected
		SelectSpikeNo(threshold, TRUE);
		UpdateSpikeDisplay();
		break;

	case HINT_HITCHANNEL:					// change channel if different
		if (i_id == m_ChartDataWnd_Detect.GetDlgCtrlID())
		{
			if (m_ichanselected != threshold)
			{
				m_ichanselected = threshold;	// get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED, m_ichanselected);
			}
		}
		else if (i_id == m_ChartDataWnd_Source.GetDlgCtrlID())
		{
			if (m_ichanselected2 != threshold)
			{
				m_ichanselected2 = threshold;	// get ID of new channel
				SetDlgItemInt(IDC_CHANSELECTED2, m_ichanselected2);
			}
		}
		break;

	default:
		break;
	}
	return 0L;
}

void CViewSpikeDetection::OnFirstFrame()
{
	OnFileScroll(SB_LEFT, 1L);
}

void CViewSpikeDetection::OnLastFrame()
{
	OnFileScroll(SB_RIGHT, 1L);
}

void CViewSpikeDetection::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// CDaoRecordView scroll bar: pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// trap messages from CScrollBarEx
	long l_first;
	long l_last;
	CString cs;
	switch (nSBCode)
	{
	case SB_THUMBTRACK:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL);
		l_first = m_filescroll_infos.nPos;
		l_last = l_first + m_filescroll_infos.nPage - 1;
		m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);
		m_ChartDataWnd_Source.GetDataFromDoc(l_first, l_last);
		UpdateLegends();
		break;

	case SB_THUMBPOSITION:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL);
		l_first = m_filescroll_infos.nPos;
		l_last = l_first + m_filescroll_infos.nPage - 1;
		m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);
		m_ChartDataWnd_Source.GetDataFromDoc(l_first, l_last);
		UpdateLegends();
		break;

	default:
		OnFileScroll(nSBCode, nPos);
		break;
	}
}

void CViewSpikeDetection::UpdateFileScroll()
{
	m_filescroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_filescroll_infos.nPos = m_ChartDataWnd_Detect.GetDataFirst();
	m_filescroll_infos.nPage = m_ChartDataWnd_Detect.GetDataLast() - m_ChartDataWnd_Detect.GetDataFirst() + 1;
	m_filescroll.SetScrollInfo(&m_filescroll_infos);
}

void CViewSpikeDetection::OnFileScroll(UINT nSBCode, UINT nPos)
{
	auto b_result = FALSE;
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			// scroll to the start
	case SB_LINELEFT:		// scroll one line left
	case SB_LINERIGHT:		// scroll one line right
	case SB_PAGELEFT:		// scroll one page left
	case SB_PAGERIGHT:		// scroll one page right
	case SB_RIGHT:			// scroll to end right
		b_result = m_ChartDataWnd_Source.ScrollDataFromDoc(nSBCode);
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		b_result = m_ChartDataWnd_Source.GetDataFromDoc(long(nPos) * (m_ChartDataWnd_Source.GetDocumentLast()) / long(100));
		break;
	default:				// NOP: set position only
		break;
	}
	// adjust display
	if (b_result)
		UpdateLegends();
}

void CViewSpikeDetection::OnFormatYscaleCentercurve()
{
	m_ChartDataWnd_Detect.CenterChan(0);
	m_ChartDataWnd_Detect.Invalidate();

	for (auto i = 0; i < m_ChartDataWnd_Source.GetChanlistSize(); i++)
		m_ChartDataWnd_Source.CenterChan(i);
	m_ChartDataWnd_Source.Invalidate();

	m_displaySpk_BarView.CenterCurve();
	m_displaySpk_BarView.Invalidate();

	m_displaySpk_Shape.SetYWExtOrg(m_displaySpk_BarView.GetYWExtent(), m_displaySpk_BarView.GetYWOrg());
	UpdateSpkShapeWndScale(TRUE);
	m_displaySpk_Shape.Invalidate();
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

	m_displaySpk_BarView.MaxCenter();
	m_displaySpk_BarView.Invalidate();

	m_displaySpk_Shape.SetYWExtOrg(m_displaySpk_BarView.GetYWExtent(), m_displaySpk_BarView.GetYWOrg());
	m_displaySpk_Shape.SetXWExtOrg(p_spikelist_->GetSpikeLength(), 0);
	UpdateSpkShapeWndScale(FALSE);
	m_displaySpk_Shape.Invalidate();
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
	m_displaySpk_BarView.MaxCenter();
	m_displaySpk_BarView.Invalidate();

	m_displaySpk_Shape.SetYWExtOrg(m_displaySpk_BarView.GetYWExtent(), m_displaySpk_BarView.GetYWOrg());
	UpdateSpkShapeWndScale(FALSE);
	m_displaySpk_Shape.Invalidate();
}

void CViewSpikeDetection::OnFormatAlldata()
{
	const auto l_last = GetDocument()->m_pDat->GetDOCchanLength();
	m_ChartDataWnd_Detect.ResizeChannels(0, l_last);
	m_ChartDataWnd_Detect.GetDataFromDoc(0, l_last);

	m_ChartDataWnd_Source.ResizeChannels(0, l_last);
	m_ChartDataWnd_Source.GetDataFromDoc(0, l_last);

	const auto x_we = p_spikelist_->GetSpikeLength();
	if (x_we != m_displaySpk_Shape.GetXWExtent() || 0 != m_displaySpk_Shape.GetXWOrg())
		m_displaySpk_Shape.SetXWExtOrg(x_we, 0);
	m_displaySpk_BarView.CenterCurve();
	m_displaySpk_Shape.SetYWExtOrg(m_displaySpk_BarView.GetYWExtent(), m_displaySpk_BarView.GetYWOrg());

	UpdateLegends();
}

void CViewSpikeDetection::UpdateDetectionParameters()
{
	// refresh pointer to spike detection array
	const auto narrays = m_parmsCurrent.GetSize();
	ASSERT(m_iDetectParms < narrays);
	m_pDetectParms = m_parmsCurrent.GetItem(m_iDetectParms);
	UpdateDetectionSettings(m_iDetectParms);

	// make sure that tabs are identical to what has been changed
	UpdateTabs();
	UpdateCB();
	UpdateLegendDetectionWnd();

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
	CSpikeDetectDlg dlg;
	dlg.m_dbDoc = GetDocument()->m_pDat;
	dlg.m_iDetectParmsDlg = m_iDetectParms;			// index spk detect parm currently selected / array
	dlg.m_pDetectSettingsArray = &m_parmsCurrent;	// spike detection parameters array
	dlg.mdPM = options_viewdata;
	dlg.m_pChartDataDetectWnd = &m_ChartDataWnd_Detect;
	if (IDOK == dlg.DoModal())
	{
		// copy modified parameters into array
		// update HorizontalCursorList on both sourceView & histogram
		m_iDetectParms = dlg.m_iDetectParmsDlg;
		UpdateDetectionParameters();
	}
}

void CViewSpikeDetection::OnSelchangeDetectchan()
{
	UpdateData(TRUE);
	m_pDetectParms->detectChan = m_CBdetectChan.GetCurSel(); // get new channel
	m_pDetectParms->bChanged = TRUE;			// and flag it
	m_ChartDataWnd_Detect.SetChanlistOrdinates(0, m_pDetectParms->detectChan, m_pDetectParms->detectTransform);
	m_pDetectParms->detectThreshold = m_ChartDataWnd_Detect.ConvertChanlistVoltstoDataBins(0, m_pDetectParms->detectThresholdmV / 1000.f);
	m_ChartDataWnd_Detect.MoveHZtagtoVal(0, m_pDetectParms->detectThreshold);

	m_ChartDataWnd_Detect.GetDataFromDoc(); 				// load data
	m_ChartDataWnd_Detect.AutoZoomChan(0);				// vertical position of channel
	m_ChartDataWnd_Detect.Invalidate();
}

void CViewSpikeDetection::OnSelchangeTransform()
{
	UpdateData(TRUE);
	m_pDetectParms->detectTransform = m_CBtransform.GetCurSel();// get new method
	m_pDetectParms->bChanged = TRUE;			// save new method and flag it
	m_ChartDataWnd_Detect.SetChanlistTransformMode(0, m_pDetectParms->detectTransform);
	m_ChartDataWnd_Detect.GetDataFromDoc(); 				// load data
	m_ChartDataWnd_Detect.AutoZoomChan(0);				// vertical position of channel
	m_ChartDataWnd_Detect.Invalidate();
}

void CViewSpikeDetection::OnMeasureAll()
{
	DetectAll(TRUE);
}

void CViewSpikeDetection::OnMeasure()
{
	DetectAll(FALSE);
}

void CViewSpikeDetection::DetectAll(BOOL bAll)
{
	// init spike document (erase spike list data & intervals)
	m_bDetected = TRUE;	
	p_spike_doc_->SetModifiedFlag(TRUE);
	p_spike_doc_->SetDetectionDate(CTime::GetCurrentTime());
	auto ioldlist = GetDocument()->GetcurrentSpkDocument()->GetSpkList_CurrentIndex();
	m_spikeno = -1;

	// check if detection parameters are ok? prevent detection from a channel that does not exist
	const auto p_dat = GetDocument()->m_pDat;
	if (p_dat == nullptr)
		return;
	const auto wave_format = p_dat->GetpWaveFormat();
	const auto chanmax = wave_format->scan_count - 1;
	for (auto i = 0; i < m_parmsCurrent.GetSize(); i++)
	{
		const auto pspk_dp = m_parmsCurrent.GetItem(i);
		if (pspk_dp->extractChan > chanmax)
		{
			MessageBox(_T("Check spike detection parameters \n- one of the detection channel requested \nis not available in the source data"));
			return;
		}
	}

	// adjust size of spklist array
	if (m_parmsCurrent.GetSize() != p_spike_doc_->GetSpkList_Size())
		p_spike_doc_->SetSpkList_Size(m_parmsCurrent.GetSize());

	// detect spikes from all chans marked as such
	for (int i = 0; i < m_parmsCurrent.GetSize(); i++)
	{
		if (!bAll && m_iDetectParms != i)
			continue;
		// detect missing data channel
		if ((m_parmsCurrent.GetItem(i))->extractChan > chanmax)
			continue;

		// select new spike list (list with no spikes for stimulus channel)
		CSpikeList* pspklist = p_spike_doc_->SetSpkList_AsCurrent(i);
		if (pspklist == nullptr)
		{
			p_spike_doc_->AddSpkList();
			pspklist = p_spike_doc_->GetSpkList_Current();
		}

		p_spikelist_ = pspklist;
		if (p_spikelist_->GetTotalSpikes() == 0)
		{
			SPKDETECTPARM* pFC = m_parmsCurrent.GetItem(i);
			ASSERT_VALID(pFC);
			ASSERT(pFC != NULL);
			p_spikelist_->InitSpikeList(GetDocument()->m_pDat, pFC);
		}
		if ((m_parmsCurrent.GetItem(i))->detectWhat == DETECT_SPIKES)
		{
			DetectMethod1(i);							// detect spikes
		}
		else
		{
			DetectStim1(i);								// detect stimulus
			UpdateVTtags();								// display vertical bars
			m_displaySpk_BarView.Invalidate();
			m_ChartDataWnd_Detect.Invalidate();
			m_ChartDataWnd_Source.Invalidate();
		}
	}

	// save spike file
	SaveCurrentSpikeFile();

	// display data
	if (ioldlist < 0)
		ioldlist = 0;
	p_spikelist_ = p_spike_doc_->SetSpkList_AsCurrent(ioldlist);

	m_displaySpk_BarView.SetSourceData(p_spikelist_, GetDocument());
	m_displaySpk_Shape.SetSourceData(p_spikelist_, GetDocument());
	

	// center spikes, change nb spikes and update content of draw buttons
	if (options_viewdata->bMaximizeGain
		|| m_displaySpk_BarView.GetYWExtent() == 0
		|| m_displaySpk_BarView.GetYWOrg() == 0
		|| m_displaySpk_Shape.GetYWExtent() == 0
		|| m_displaySpk_Shape.GetYWOrg() == 0)
	{
		m_displaySpk_BarView.MaxCenter();
		m_displaySpk_Shape.SetYWExtOrg(m_displaySpk_BarView.GetYWExtent(), m_displaySpk_BarView.GetYWOrg());
		int spklen = 60;
		if (p_spikelist_ != nullptr)
			spklen = p_spikelist_->GetSpikeLength(); m_displaySpk_Shape.SetXWExtOrg(spklen, 0);
		UpdateSpkShapeWndScale(FALSE);
	}

	HighlightSpikes(TRUE);
	UpdateLegends();
	UpdateTabs();
}

int CViewSpikeDetection::DetectStim1(int ichan)
{
	// set parameters (copy array into local parms)
	const auto psti_d = m_parmsCurrent.GetItem(ichan);
	const auto threshold = psti_d->detectThreshold;		// threshold value
	const auto method = psti_d->detectTransform;		// how source data are transformed
	const auto sourcechan = psti_d->detectChan;			// source channel

	// get parameters from document
	auto p_dat = GetDocument()->m_pDat;
	//int nchans;										// number of data chans / source buffer
	//short* p_buffer = pDat->LoadRawDataParams(&nchans);	// get address of reading data buffer
	const auto nspan = p_dat->GetTransfDataSpan(method);	// nb pts to read before transf

	// set detection mode
	// 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);
	auto b_cross_upw = TRUE;
	if (psti_d->detectMode == MODE_OFF_ON || psti_d->detectMode == MODE_OFF_OFF)
		b_cross_upw = FALSE;
	auto b_mode = TRUE;
	if (psti_d->detectMode == MODE_ON_ON	|| psti_d->detectMode == MODE_OFF_OFF)
		b_mode = FALSE;

	auto b_save_on = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = m_ChartDataWnd_Detect.GetDataFirst();	// index first pt to test
	const auto l_data_last = m_ChartDataWnd_Detect.GetDataLast();		// index last pt to test

	// plot progress dialog box
	CProgressDlg dlg;
	dlg.Create();
	//int istep = 0;
	dlg.SetRange(0, 100);
	const auto l_data_len = l_data_last - l_data_first;
	const auto l_data_first0 = l_data_first;
	dlg.SetWindowText(_T("Detect trigger events..."));

	// loop through data defined in the lineview window
	while (l_data_first < l_data_last)
	{
		auto l_rw_first = l_data_first;					// index very first pt within buffers
		auto l_rw_last = l_data_last;					// index very last pt within buffers
		if (!p_dat->LoadRawData(&l_rw_first, &l_rw_last, nspan))
			break;									// exit if error reported
		if (!p_dat->BuildTransfData(method, sourcechan))
			break;

		// compute initial offset (address of first point)
		auto l_last = l_rw_last;
		if (l_last > l_data_last)
			l_last = l_data_last;
		const int i_buf_first = l_data_first - p_dat->GetDOCchanIndexFirst();
		const auto p_data_first = p_dat->GetpTransfDataElmt(i_buf_first);

		// DETECT STIM ---------------------------------------------------------------
		// detect event if value above threshold
		for (long cx = l_data_first; cx <= l_last; cx++)
		{
			// set message for dialog box
			// check if user wants to stop

			const auto p_data = p_data_first + (cx - l_data_first);
			const int val = *p_data;
			if (b_cross_upw)
			{
				if (val <= threshold)			// find first point above threshold
					continue;						// no: loop to next point

				// add element
				b_cross_upw = FALSE;
				b_save_on = ~b_save_on;
				if (!b_mode && !b_save_on)
					continue;
			}
			else
			{
				if (val >= threshold)			// test if a spike is present
					continue;					// no: loop to next point

				// add element
				b_cross_upw = TRUE;
				b_save_on = ~b_save_on;
				if (!b_mode && !b_save_on)
					continue;
			}
			// check if user does not want to continue
			const int cxpos = (cx - l_data_first0) * 100 / l_data_len;
			dlg.SetPos(cxpos);
			CString cscomment;
			cscomment.Format(_T("Processing stimulus event: %i"), p_spike_doc_->m_stimIntervals.nitems + 1);
			dlg.SetStatus(cscomment);

			if (dlg.CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				{
					// set condition to stop detection
					l_last = l_data_last;
					// clear stimulus detected
					auto p_sti = &(p_spike_doc_->m_stimIntervals);
					p_sti->intervalsArray.RemoveAll();
					p_spike_doc_->m_stimIntervals.nitems = 0;
					break;
				}

			// check if already present and insert it at the proper place
			auto p_sti = &(p_spike_doc_->m_stimIntervals);
			const auto jitter = 2;		// allow some jitter in the detection (+-2)
			auto flag = TRUE;
			int i2;
			for (i2 = 0; i2 < p_sti->intervalsArray.GetSize(); i2++)
			{
				const auto lval = p_sti->intervalsArray.GetAt(i2);
				if (cx <= (lval + jitter) && cx >= (lval - jitter))
				{
					flag = FALSE;		// no new stim - already detected at that time
					break;
				}
				if (cx < lval - jitter)
				{
					flag = TRUE;			// new stim to insert before i
					break;
				}
			}
			if (flag)
			{
				p_sti->intervalsArray.InsertAt(i2, cx);
				p_spike_doc_->m_stimIntervals.nitems++;
			}
		}
		///////////////////////////////////////////////////////////////
		l_data_first = l_last + 1;					// update for next loop
	}

	return p_spike_doc_->m_stimIntervals.nitems;
}

int CViewSpikeDetection::DetectMethod1(WORD schan)
{
	SPKDETECTPARM* pspkDP = m_parmsCurrent.GetItem(schan);
	if (pspkDP->extractTransform != pspkDP->detectTransform &&
		pspkDP->extractTransform != 0)
	{
		AfxMessageBox(_T("Options not implemented yet!\nd chan == extr chan or !extr chan=0\nChange detection parameters"));
		return  p_spikelist_->GetTotalSpikes();
	}

	// set parameters (copy array into local parms)
	const short threshold		= pspkDP->detectThreshold;			// threshold value
	const auto method			= pspkDP->detectTransform;			// how source data are transformed
	const auto sourcechan		= pspkDP->detectChan;				// source channel
	const auto prethreshold		= pspkDP->prethreshold;				// pts before threshold
	const auto refractory		= pspkDP->refractory;				// refractory period
	const auto postthreshold	= pspkDP->extractNpoints - prethreshold;

	// get parameters from document
	auto p_dat = GetDocument()->m_pDat;
	int nchans;													// number of data chans / source buffer
	const auto p_buf = p_dat->LoadRawDataParams(&nchans);
	const auto nspan = p_dat->GetTransfDataSpan(method);		// nb pts to read before transf

	// adjust detection method: if threshold lower than data zero detect lower crossing
	auto b_cross_upw = TRUE;
	if (threshold < 0)
		b_cross_upw = FALSE;

	// get data detection limits and clip limits according to size of spikes
	auto l_data_first = m_ChartDataWnd_Detect.GetDataFirst();			// index first pt to test
	auto l_data_last = m_ChartDataWnd_Detect.GetDataLast();			// index last pt to test
	if (l_data_first < prethreshold + nspan)
		l_data_first = static_cast<long>(prethreshold) + nspan;
	if (l_data_last > p_dat->GetDOCchanLength() - postthreshold - nspan)
		l_data_last = p_dat->GetDOCchanLength() - postthreshold - nspan;

	// loop through data defined in the lineview window
	while (l_data_first < l_data_last)
	{
		auto l_rw_first = l_data_first - prethreshold;			// index very first pt within buffers
		auto l_rw_last = l_data_last;							// index very last pt within buffers
		if (!p_dat->LoadRawData(&l_rw_first, &l_rw_last, nspan))	// load data from file
			break;												// exit if error reported
		if (!p_dat->BuildTransfData(method, sourcechan))		// transfer data into a buffer with a single channel
			break;												// exit if fail

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
				if (*p_data < threshold)	// test if a spike is present
					continue;				// no: loop to next point

				// search max and threshold crossing
				auto max = *p_data;			// init max
				auto p_data1 = p_data;		// init pointer
				iitime = cx;				// init spike time
				// loop to search max
				for (auto i = cx; i < cx + refractory; i++, p_data1++)
				{
					if (max < *p_data1)		// search max
					{
						max = *p_data1;
						p_data = p_data1;		// p_data = "center" of spike
						iitime = i;			// iitime = time of spike
					}
				}
			}
			// detect < threshold ......... if found, search for min
			else
			{
				if (*p_data > threshold)		// test if a spike is present
					continue;				// no: loop to next point

				// search min and threshold crossing
				auto min = *p_data;
				auto p_data1 = p_data;
				iitime = cx;
				for (auto i = cx; i < cx + refractory; i++, p_data1++)
				{
					if (min > * p_data1)
					{
						min = *p_data1;
						p_data = p_data1;			// p_data = "center" of spike
						iitime = i;
					}
				}
			}

			// ........................................ SPIKE DETECTED
			if (pspkDP->extractTransform == pspkDP->detectTransform)
			{
				const auto p_m = p_data - prethreshold;
				p_spikelist_->AddSpike(p_m, 1, iitime, sourcechan, 0, TRUE);
			}
			else  // extract from raw data
			{
				const auto pM = p_buf
					+ nchans * (iitime - prethreshold - l_rw_first + nspan)
					+ pspkDP->extractChan;
				p_spikelist_->AddSpike(pM, nchans, iitime, sourcechan, 0, TRUE);
			}

			// update loop parameters
			cx = iitime + refractory;
		}

		///////////////////////////////////////////////////////////////
		l_data_first = cx + 1;		// update for next loop
	}

	return p_spikelist_->GetTotalSpikes();
}

void CViewSpikeDetection::OnToolsEdittransformspikes()
{
	CSpikeEditDlg dlg;							// dialog box
	dlg.m_yextent = m_displaySpk_Shape.GetYWExtent();// load display parameters
	dlg.m_yzero = m_displaySpk_Shape.GetYWOrg();	// ordinates
	dlg.m_xextent = m_displaySpk_Shape.GetXWExtent();// and
	dlg.m_xzero = m_displaySpk_Shape.GetXWOrg();	// abcissa
	dlg.m_spikeno = m_spikeno;					// load index of selected spike
	p_spikelist_->RemoveAllSpikeFlags();
	dlg.m_pdbWaveDoc = GetDocument();
	dlg.m_parent = this;

	// open dialog box and wait for response
	dlg.DoModal();

	m_spikeno = dlg.m_spikeno;
	SelectSpikeNo(m_spikeno, FALSE);
	UpdateSpikeDisplay();
	if (dlg.m_bchanged)
		p_spike_doc_->SetModifiedFlag(TRUE);

	UpdateLegends();
}

void CViewSpikeDetection::OnFormatXscale()
{
	XYParametersDlg dlg;
	CWnd* pFocus = GetFocus();

	if (pFocus != nullptr && m_ChartDataWnd_Detect.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		dlg.m_yzero = m_ChartDataWnd_Detect.GetChanlistYzero(m_pDetectParms->detectChan);
		dlg.m_yextent = m_ChartDataWnd_Detect.GetChanlistYextent(m_pDetectParms->detectChan);
		dlg.m_bDisplaysource = TRUE;
	}
	else if (pFocus != nullptr && m_displaySpk_BarView.m_hWnd == pFocus->m_hWnd)
	{
		dlg.m_xparam = FALSE;
		dlg.m_yzero = m_displaySpk_BarView.GetYWOrg();
		dlg.m_yextent = m_displaySpk_BarView.GetYWExtent();
		dlg.m_bDisplaybars = TRUE;
	}
	else
	{
		dlg.m_xzero = m_displaySpk_Shape.GetXWOrg();
		dlg.m_xextent = m_displaySpk_Shape.GetXWExtent();
		dlg.m_yzero = m_displaySpk_Shape.GetYWOrg();
		dlg.m_yextent = m_displaySpk_Shape.GetYWExtent();
		dlg.m_bDisplayspikes = TRUE;
	}

	if (IDOK == dlg.DoModal())
	{
		if (dlg.m_bDisplaysource)
		{
			m_ChartDataWnd_Detect.SetChanlistYzero(0, dlg.m_yzero);
			m_ChartDataWnd_Detect.SetChanlistYextent(0, dlg.m_yextent);
			m_ChartDataWnd_Detect.Invalidate();
		}
		if (dlg.m_bDisplaybars)
		{
			m_displaySpk_BarView.SetYWExtOrg(dlg.m_yextent, dlg.m_yzero);
			m_displaySpk_BarView.Invalidate();
		}
		if (dlg.m_bDisplayspikes)
		{
			m_displaySpk_Shape.SetYWExtOrg(dlg.m_yextent, dlg.m_yzero);
			m_displaySpk_Shape.SetXWExtOrg(dlg.m_xextent, dlg.m_xzero);
			m_displaySpk_Shape.Invalidate();
		}
	}
}

void CViewSpikeDetection::OnBnClickedClearall()
{
	m_spikeno = -1;						// unselect spike
	m_displaySpk_BarView.SelectSpike(-1);		// deselect spike bars
	m_displaySpk_Shape.SelectSpikeShape(-1);// deselect superimposed spikes

	// update spike list
	for (int i = 0; i < p_spike_doc_->GetSpkList_Size(); i++)
	{
		CSpikeList* pspklist = p_spike_doc_->SetSpkList_AsCurrent(i);
		pspklist->InitSpikeList(GetDocument()->m_pDat, nullptr);
	}
	p_spikelist_ = p_spike_doc_->GetSpkList_Current();
	ASSERT(p_spikelist_ != NULL);

	HighlightSpikes(FALSE);				// remove display of spikes
	m_displaySpk_Shape.SetSourceData(p_spikelist_, GetDocument());
	p_spike_doc_->m_stimIntervals.nitems = 0;		// zero stimuli
	p_spike_doc_->m_stimIntervals.intervalsArray.RemoveAll();

	UpdateDetectionParameters();
	UpdateVTtags();						// update display of vertical tags
	UpdateLegends();					// change legends
	p_spike_doc_->SetModifiedFlag(TRUE);	// mark spike document as changed
}

void CViewSpikeDetection::OnClear()
{
	m_spikeno = -1;						// unselect spike
	m_displaySpk_BarView.SelectSpike(-1);		// deselect spike bars
	m_displaySpk_Shape.SelectSpikeShape(-1);// deselect superimposed spikes

	p_spikelist_ = p_spike_doc_->GetSpkList_Current();
	p_spikelist_->InitSpikeList(GetDocument()->m_pDat, nullptr);
	HighlightSpikes(FALSE);				// remove display of spikes

	if (p_spikelist_->GetdetectWhat() == DETECT_STIMULUS)
	{
		p_spike_doc_->m_stimIntervals.nitems = 0;		// zero stimuli
		p_spike_doc_->m_stimIntervals.intervalsArray.RemoveAll();
		UpdateVTtags();					// update display of vertical tags
	}

	UpdateLegends();					// change legends
	p_spike_doc_->SetModifiedFlag(TRUE);	// mark spike document as changed
}

void CViewSpikeDetection::OnEnChangeSpikeno()
{
	if (mm_spikeno.m_bEntryDone)
	{
		auto spikeno = m_spikeno;
		switch (mm_spikeno.m_nChar)
		{				// load data from edit controls
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
		if (spikeno >= p_spikelist_->GetTotalSpikes())
			spikeno = p_spikelist_->GetTotalSpikes() - 1;

		// change display if necessary
		mm_spikeno.m_bEntryDone = FALSE;	// clear flag
		mm_spikeno.m_nChar = 0;			// empty buffer
		mm_spikeno.SetSel(0, -1);		// select all text

		// update spike num and center display on the selected spike
		SelectSpikeNo(spikeno, FALSE);
		UpdateSpikeDisplay();
	}
}

void CViewSpikeDetection::OnArtefact()
{
	UpdateData(TRUE);						// load value from control
	const auto nspikes = p_spikelist_->GetSpikeFlagArrayCount();
	if (nspikes < 1)
	{
		m_bartefact = FALSE;				// no action if spike index < 0
	}
	else
	{										// load old class nb
		ASSERT(nspikes >= 0);
		for (auto i = 0; i < nspikes; i++)
		{
			const auto spikeno = p_spikelist_->GetSpikeFlagArrayAt(i);
			auto spkclass = p_spikelist_->GetSpikeClass(spikeno);
			// if artefact: set class to negative value
			if (m_bartefact && spkclass >= 0)
				spkclass = -(spkclass + 1);

			// if not artefact: if spike has negative class, set to positive value
			else if (spkclass < 0)
				spkclass = -(spkclass + 1);
			p_spikelist_->SetSpikeClass(spikeno, spkclass);
		}

		p_spike_doc_->SetModifiedFlag(TRUE);
		SaveCurrentSpikeFile();
	}
	m_spikeno = -1;

	const auto iSelParms = m_tabCtrl.GetCurSel();
	m_pDetectParms = m_parmsCurrent.GetItem(iSelParms);
	p_spikelist_ = p_spike_doc_->SetSpkList_AsCurrent(iSelParms);

	SelectSpikeNo(m_spikeno, FALSE);
	UpdateSpikeDisplay();

	// update nb spikes
	const UINT unspikes = p_spikelist_->GetTotalSpikes();
	if (unspikes != GetDlgItemInt(IDC_NBSPIKES_NB))
		SetDlgItemInt(IDC_NBSPIKES_NB, unspikes);
}

void CViewSpikeDetection::AlignDisplayToCurrentSpike()
{
	if (m_spikeno < 0)
		return;

	const auto l_spike_time = p_spikelist_->GetSpikeTime(m_spikeno);
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
		UpdateLegends();
	}
}

void CViewSpikeDetection::UpdateSpkShapeWndScale(const BOOL b_set_from_controls)
{
	// get current values
	int ix_we;
	auto iy_we = 0;

	// if set from controls, get value from the controls
	if (b_set_from_controls && p_spikelist_->GetTotalSpikes() > 0)
	{
		// set time scale
		CString cs;
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->GetWindowText(cs);
		const auto x = static_cast<float>(_ttof(cs)) / 1000.0f;
		ix_we = static_cast<int>(p_spikelist_->GetAcqSampRate() * x);
		if (ix_we == 0)
			ix_we = p_spikelist_->GetextractNpoints();
		ASSERT(ix_we != 0);
		m_displaySpk_Shape.SetXWExtOrg(ix_we, m_displaySpk_Shape.GetXWOrg());

		// set amplitude
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->GetWindowText(cs);
		if (!cs.IsEmpty())
		{
			const auto y = static_cast<float>(_ttof(cs)) / 1000.0f;
			iy_we = static_cast<int>(y / p_spikelist_->GetAcqVoltsperBin());
		}
		if (iy_we == 0)
			iy_we = m_displaySpk_Shape.GetYWExtent();
		m_displaySpk_Shape.SetYWExtOrg(iy_we, m_displaySpk_Shape.GetYWOrg());
	}
	else
	{
		ix_we = m_displaySpk_Shape.GetXWExtent();
		iy_we = m_displaySpk_Shape.GetYWExtent();
	}

	if (ix_we != NULL && iy_we != NULL)
	{
		const auto yunit = m_displaySpk_Shape.GetExtent_mV() / m_displaySpk_Shape.GetNyScaleCells();
		m_displaySpk_Shape.SetyScaleUnitValue(yunit);
		const auto xunit = m_displaySpk_Shape.GetExtent_ms() / m_displaySpk_Shape.GetNxScaleCells();
		m_displaySpk_Shape.SetxScaleUnitValue(xunit);
	}

	// output values
	_stprintf_s(m_szbuf, 64, _T("%.3lf"), m_displaySpk_Shape.GetExtent_mV());
	SetDlgItemText(IDC_SPIKEWINDOWAMPLITUDE, m_szbuf);
	const auto yunit = m_displaySpk_Shape.GetExtent_mV() / m_displaySpk_Shape.GetNyScaleCells();
	m_displaySpk_Shape.SetyScaleUnitValue(yunit);

	_stprintf_s(m_szbuf, 64, _T("%.3lf"), m_displaySpk_Shape.GetExtent_ms());
	SetDlgItemText(IDC_SPIKEWINDOWLENGTH, m_szbuf);
	const auto xunit = m_displaySpk_Shape.GetExtent_ms() / m_displaySpk_Shape.GetNxScaleCells();
	m_displaySpk_Shape.SetxScaleUnitValue(xunit);
}

void CViewSpikeDetection::SelectSpikeNo(int spikeno, BOOL bMultipleSelection)
{
	// load spike parameters
	if (spikeno >= 0)
	{	// get address of spike parms
		const auto p_spike_element = p_spikelist_->GetSpikeElemt(spikeno);
		m_bartefact = (p_spike_element->get_class() < 0);
		if (bMultipleSelection)	// TRUE= multiple selection
		{
			/*auto nflaggedspikes =*/ p_spikelist_->ToggleSpikeFlag(spikeno);
			if (p_spikelist_->GetSpikeFlagArrayCount() < 1)
				spikeno = -1;
			if (m_spikeno == spikeno)
				spikeno = 0;
		}
		else
		{
			p_spikelist_->SetSingleSpikeFlag(spikeno);
		}
		m_spikeno = spikeno;
		AlignDisplayToCurrentSpike();
	}
	else
	{
		p_spikelist_->RemoveAllSpikeFlags();
		m_bartefact = FALSE;
	}
}

void CViewSpikeDetection::UpdateSpikeDisplay()
{
	// update spike display windows
	m_displaySpk_BarView.Invalidate(TRUE);
	m_displaySpk_Shape.Invalidate(TRUE);

	// update Dlg interface
	GetDlgItem(IDC_SPIKENO)->EnableWindow(!(p_spikelist_->GetSpikeFlagArrayCount() > 1));
	SetDlgItemInt(IDC_SPIKENO, m_spikeno, TRUE);
	CheckDlgButton(IDC_ARTEFACT, m_bartefact);
}

void CViewSpikeDetection::OnEnChangeThresholdval()
{
	if (mm_thresholdval.m_bEntryDone) {
		auto thresholdval = m_thresholdval;
		switch (mm_thresholdval.m_nChar)
		{				// load data from edit controls
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
			m_pDetectParms->detectThresholdmV = thresholdval;
			m_pDetectParms->detectThreshold = m_ChartDataWnd_Detect.ConvertChanlistVoltstoDataBins(0, m_thresholdval / 1000.f);
			m_ChartDataWnd_Detect.MoveHZtagtoVal(0, m_pDetectParms->detectThreshold);
		}

		mm_thresholdval.m_bEntryDone = FALSE;
		mm_thresholdval.m_nChar = 0;
		CString cs;
		cs.Format(_T("%.3f"), m_thresholdval);
		GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
		// change detection threshold method to manual
		mm_thresholdval.SetSel(0, -1); 		// select all text
	}
}

void CViewSpikeDetection::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone) {
		switch (mm_timefirst.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);		// load data from edit controls
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
		m_ChartDataWnd_Detect.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate), static_cast<long>(m_timelast * m_samplingRate));
		m_ChartDataWnd_Source.GetDataFromDoc(static_cast<long>(m_timefirst * m_samplingRate), static_cast<long>(m_timelast * m_samplingRate));
		UpdateLegends();

		mm_timefirst.m_bEntryDone = FALSE;
		mm_timefirst.m_nChar = 0;
		mm_timefirst.SetSel(0, -1); 	//select all text
	}
}

void CViewSpikeDetection::OnEnChangeTimelast()
{
	if (mm_timelast.m_bEntryDone) {
		switch (mm_timelast.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);		// load data from edit controls
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
		m_ChartDataWnd_Detect.GetDataFromDoc((long)(m_timefirst * m_samplingRate), (long)(m_timelast * m_samplingRate));
		m_ChartDataWnd_Source.GetDataFromDoc((long)(m_timefirst * m_samplingRate), (long)(m_timelast * m_samplingRate));
		UpdateLegends();

		mm_timelast.m_bEntryDone = FALSE;
		mm_timelast.m_nChar = 0;
		mm_timelast.SetSel(0, -1); 	//select all text
	}
}

void CViewSpikeDetection::OnToolsDataseries()
{
	// init dialog data
	CDataSeriesDlg dlg;
	dlg.m_pChartDataWnd = &m_ChartDataWnd_Detect;
	dlg.m_pdbDoc = GetDocument()->m_pDat;
	dlg.m_listindex = 0;

	// invoke dialog box
	dlg.DoModal();
	if (m_ChartDataWnd_Detect.GetChanlistSize() < 1)
	{
		m_ChartDataWnd_Detect.RemoveAllChanlistItems();
		m_ChartDataWnd_Detect.AddChanlistItem(m_pDetectParms->detectChan, m_pDetectParms->detectTransform);
	}
	UpdateLegends();
}

void CViewSpikeDetection::PrintDataCartridge(CDC* p_dc, CChartDataWnd* pDataChartWnd, CRect* prect, BOOL bComments, BOOL bBars)
{
	SCOPESTRUCT* pStruct = pDataChartWnd->GetScopeParameters();
	const auto b_draw_f = pStruct->bDrawframe;
	pStruct->bDrawframe = TRUE;
	pDataChartWnd->Print(p_dc, prect, (options_viewdata->bcontours == 1));
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
	CCopyAsDlg dlg;
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

		if (!dlg.m_bgraphics)
			m_ChartDataWnd_Detect.CopyAsText(dlg.m_ioption, dlg.m_iunit, dlg.m_nabcissa);
		else
		{
			SerializeWindowsState(BSAVE);

			CRect old_rect1;								// save size of lineview windows
			m_ChartDataWnd_Detect.GetWindowRect(&old_rect1);
			CRect old_rect2;
			m_ChartDataWnd_Source.GetWindowRect(&old_rect2);

			const CRect rect(0, 0, options_viewdata->hzResolution, options_viewdata->vtResolution);
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
			const auto p_old_brush = (CBrush*)m_dc.SelectStockObject(BLACK_BRUSH);
			CClientDC attrib_dc(this);					// Create and attach attribute DC
			m_dc.SetAttribDC(attrib_dc.GetSafeHdc());	// from current screen

			// print comments : set font
			m_pOldFont = nullptr;
			const auto oldsize = options_viewdata->fontsize;
			options_viewdata->fontsize = 10;
			PrintCreateFont();
			m_dc.SetBkMode(TRANSPARENT);
			options_viewdata->fontsize = oldsize;
			m_pOldFont = m_dc.SelectObject(&m_fontPrint);
			const int lineheight = m_logFont.lfHeight + 5;
			auto ypxrow = 0;
			const auto xcol = 10;

			// comment and descriptors
			auto comments = GetDocument()->ExportDatabaseData(1);
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
			const auto rspkwidth = MulDiv(m_displaySpk_Shape.GetRectWidth(), rectdata.Width(), m_displaySpk_Shape.GetRectWidth() + m_ChartDataWnd_Detect.GetRectWidth());
			const auto rdataheight = MulDiv(m_ChartDataWnd_Detect.GetRectHeight(), rectdata.Height(), m_ChartDataWnd_Detect.GetRectHeight() * 2 + m_displaySpk_BarView.GetRectHeight());
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
			m_displaySpk_BarView.Print(&m_dc, &rectbars);

			// display spike shapes
			auto rectspk = rect;					// compute output rectangle
			rectspk.left += separator;
			rectspk.right = rect.left + rspkwidth;
			rectspk.bottom = rect.bottom - 2 * lineheight;
			rectspk.top = rectspk.bottom - rectbars.Height();
			m_displaySpk_Shape.Print(&m_dc, &rectspk);
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
				EmptyClipboard();							// prepare clipboard
				SetClipboardData(CF_ENHMETAFILE, h_emf_tmp);	// put data
				CloseClipboard();							// close clipboard
			}
			else
			{
				// Someone else has the Clipboard open...
				DeleteEnhMetaFile(h_emf_tmp);					// delete data
				MessageBeep(0);							// tell user something is wrong!
				AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
			}
			// restore initial conditions
			SerializeWindowsState(BRESTORE);
		}
	}
}

void CViewSpikeDetection::OnSelchangeDetectMode()
{
	UpdateData(TRUE);
	m_pDetectParms->detectWhat = m_CBdetectWhat.GetCurSel();
	UpdateCB();
	UpdateLegendDetectionWnd();
	m_ChartDataWnd_Detect.GetDataFromDoc(); 		// load data
	m_ChartDataWnd_Detect.AutoZoomChan(0);		// vertical position of channel
	m_ChartDataWnd_Detect.Invalidate();
}

void CViewSpikeDetection::UpdateCB()
{
	m_CBdetectChan.SetCurSel(m_pDetectParms->detectChan);
	m_CBtransform.SetCurSel(m_pDetectParms->detectTransform);
	m_ChartDataWnd_Detect.SetChanlistOrdinates(0, m_pDetectParms->detectChan, m_pDetectParms->detectTransform);
	m_pDetectParms->detectThreshold = m_ChartDataWnd_Detect.ConvertChanlistVoltstoDataBins(0, m_thresholdval / 1000.f);
	m_ChartDataWnd_Detect.SetHZtagChan(0, 0);
	m_ChartDataWnd_Detect.SetHZtagVal(0, m_pDetectParms->detectThreshold);
	m_pDetectParms->detectThresholdmV = m_thresholdval;
}

void CViewSpikeDetection::UpdateLegendDetectionWnd() {
	CString text;
	GetDlgItem(IDC_TRANSFORM)->GetWindowTextW(text);
	GetDlgItem(IDC_STATICDISPLAYDETECT)->SetWindowTextW(text);
	GetDlgItem(IDC_STATICDISPLAYDETECT)->Invalidate();

}

void CViewSpikeDetection::UpdateVTtags()
{
	m_displaySpk_BarView.DelAllVTtags();
	m_ChartDataWnd_Detect.DelAllVTtags();
	m_ChartDataWnd_Source.DelAllVTtags();
	if (p_spike_doc_->m_stimIntervals.nitems == 0)
		return;

	for (auto i = 0; i < p_spike_doc_->m_stimIntervals.intervalsArray.GetSize(); i++)
	{
		const int cx = p_spike_doc_->m_stimIntervals.intervalsArray.GetAt(i);
		m_displaySpk_BarView.AddVTLtag(cx);
		m_ChartDataWnd_Detect.AddVTLtag(cx);
		m_ChartDataWnd_Source.AddVTLtag(cx);
	}
}

void CViewSpikeDetection::PrintComputePageSize()
{
	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlg(FALSE);
	VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	const auto h_dc = dlg.CreatePrinterDC();     // to delete at the end -- see doc!
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

void CViewSpikeDetection::PrintFileBottomPage(CDC* p_dc, CPrintInfo* p_info)
{
	auto t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"), // %d:%d",
		p_info->m_nCurPage, p_info->GetMaxPage(),
		t.GetDay(), t.GetMonth(), t.GetYear());
	const auto ch_date = GetDocument()->GetDB_CurrentSpkFileName();
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_viewdata->horzRes / 2, options_viewdata->vertRes - 57, ch_date);
}

CString CViewSpikeDetection::PrintConvertFileIndex(const long l_first, const long l_last)
{
	CString cs_unit = _T(" s");			// get time,  prepare time unit
	CString cs_comment;
	float x_scale_factor;				// scale factor returned by changeunit
	const auto x1 = m_ChartDataWnd_Detect.ChangeUnit(static_cast<float>(l_first) / m_samplingRate, &cs_unit, &x_scale_factor);
	const auto fraction1 = static_cast<int>((x1 - int(x1)) * float(1000.));	// separate fractional part
	const auto x2 = l_last / (m_samplingRate * x_scale_factor);
	const auto fraction2 = static_cast<int>((x2 - int(x2)) * float(1000.));
	cs_comment.Format(_T("time = %i.%03.3i - %i.%03.3i %s"), static_cast<int>(x1), fraction1, static_cast<int>(x2), fraction2, static_cast<LPCTSTR>(cs_unit));
	return cs_comment;
}

BOOL CViewSpikeDetection::PrintGetFileSeriesIndexFromPage(int page, int& filenumber, long& l_first)
{
	// loop until we get all rows
	const auto totalrows = m_nbrowsperpage * (page - 1);
	l_first = m_lprintFirst;
	filenumber = 0;						// file list index
	if (options_viewdata->bPrintSelection)			// current file if selection only
		filenumber = m_file0;
	else
		GetDocument()->DBMoveFirst();

	auto very_last = m_lprintFirst + m_lprintLen;
	if (options_viewdata->bEntireRecord)
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
	if (!options_viewdata->bMultirowDisplay || !options_viewdata->bEntireRecord)
	{
		filenumber++;
		if (filenumber >= m_nfiles)
			return FALSE;

		GetDocument()->DBMoveNext();
		if (l_first < GetDocument()->GetDB_DataLen() - 1)
		{
			if (options_viewdata->bEntireRecord)
				very_last = GetDocument()->GetDB_DataLen() - 1;
		}
	}
	else
	{
		l_first += m_lprintLen;
		if (l_first >= very_last)
		{
			filenumber++;						// next index
			if (filenumber >= m_nfiles)		// last file ??
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
	CString str_comment;   					// scratch pad
	const CString tab(_T("    "));					// use 4 spaces as tabulation character
	const CString rc(_T("\n"));						// next line

	// document's name, date and time
	const auto p_data_file = GetDocument()->m_pDat;
	const auto pwave_format = p_data_file->GetpWaveFormat();
	if (options_viewdata->bDocName || options_viewdata->bAcqDateTime)// print doc infos?
	{
		if (options_viewdata->bDocName)					// print file name
			str_comment += GetDocument()->GetDB_CurrentDatFileName() + tab;

		if (options_viewdata->bAcqDateTime)				// print data acquisition date & time
		{
			const auto date = (pwave_format->acqtime).Format(_T("%#d %B %Y %X")); //("%c");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_viewdata->bAcqComment)
		str_comment += GetDocument()->ExportDatabaseData(); //pwaveFormat->GetComments(" ") +rc;

	return str_comment;
}

CString CViewSpikeDetection::PrintDataBars(CDC* p_dc, CChartDataWnd* pDataChartWnd, CRect* rect)
{
	CString cs;
	const CString rc(_T("\r"));
	const CString tab(_T("     "));
	const auto p_old_brush = (CBrush*)p_dc->SelectStockObject(BLACK_BRUSH);
	CString cs_unit;									// string for voltage or time unit
	const CPoint	bar_origin(-10, -10);						// bar origin at 10,10 pts on the left lower corner of the rectangle
	//CSize	barWidth = CSize(1,1);					// width of bars (5 pixels)
	auto ihorz_bar = pDataChartWnd->GetRectWidth() / 10;	// initial horz bar length 1/10th of display rect
	auto ivert_bar = pDataChartWnd->GetRectHeight() / 3;	// initial vert bar height 1/3rd  of display rect

	///// time abscissa ///////////////////////////
	auto str_comment = PrintConvertFileIndex(pDataChartWnd->GetDataFirst(), pDataChartWnd->GetDataLast());

	///// horizontal time bar ///////////////////////////
	if (options_viewdata->bTimeScaleBar)
	{
		// convert bar size into time units and back into pixels
		cs_unit = _T(" s");											// initial time unit
		const auto xtperpixel = pDataChartWnd->GetTimeperPixel();
		const auto z = xtperpixel * ihorz_bar;							// convert 1/10 of the length of the data displayed into time
		float	x_scale_factor;
		const auto x = pDataChartWnd->ChangeUnit(z, &cs_unit, &x_scale_factor); // convert time into a scaled time
		const auto k = pDataChartWnd->NiceUnit(x);							// convert the (scaled) time value into time expressed as an integral
		ihorz_bar = static_cast<int>((float(k) * x_scale_factor) / xtperpixel); // compute how much pixels it makes
		// print out the scale and units
		cs.Format(_T("horz bar = %i %s"), k, static_cast<LPCTSTR>(cs_unit));
		str_comment += cs + rc;
		// draw horizontal line
		ihorz_bar = MulDiv(ihorz_bar, rect->Width(), pDataChartWnd->GetRectWidth());
		p_dc->MoveTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y);
		p_dc->LineTo(rect->left + bar_origin.x + ihorz_bar, rect->bottom - bar_origin.y);
	}

	///// vertical voltage bars ///////////////////////////
	float	y_scale_factor;											// compute a good unit for channel 0
	cs_unit = _T(" V");												// initial voltage unit
	// convert bar size into voltage units and back into pixels
	const auto vperpixel = pDataChartWnd->GetChanlistVoltsperPixel(0);
	const auto zvolts = vperpixel * ivert_bar;							// convert 1/3 of the height into voltage
	const auto zscale = pDataChartWnd->ChangeUnit(zvolts, &cs_unit, &y_scale_factor);	// convert voltage into a scale voltage
	const auto znice = static_cast<float>(pDataChartWnd->NiceUnit(zscale));			// convert the (scaled) time value into time expressed as an integral
	ivert_bar = static_cast<int>(znice * y_scale_factor / vperpixel);			// compute how much pixels it makes

	if (options_viewdata->bVoltageScaleBar)
	{
		ivert_bar = MulDiv(ivert_bar, rect->Height(), pDataChartWnd->GetRectHeight());
		p_dc->MoveTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y);
		p_dc->LineTo(rect->left + bar_origin.x, rect->bottom - bar_origin.y - ivert_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_viewdata->bChansComment || options_viewdata->bVoltageScaleBar || options_viewdata->bChanSettings)
	{
		const auto imax = pDataChartWnd->GetChanlistSize();	// number of data channels
		for (auto ichan = 0; ichan < imax; ichan++)		// loop
		{
			// skip channels not printed
			if (!pDataChartWnd->GetChanlistflagPrintVisible(ichan))
				continue;
			// boucler sur les commentaires de chan n a chan 0...
			cs.Format(_T("chan#%i "), ichan);			// channel number
			str_comment += cs;
			if (options_viewdata->bVoltageScaleBar)				// bar scale value
			{
				const auto z = static_cast<float>(ivert_bar) * pDataChartWnd->GetChanlistVoltsperPixel(ichan);
				const auto x = z / y_scale_factor;
				const auto j = pDataChartWnd->NiceUnit(x);
				cs.Format(_T("vert bar = %i %s "), j, static_cast<LPCTSTR>(cs_unit));	// store val into comment
				str_comment += cs;
			}
			// print chan comment
			if (options_viewdata->bChansComment)
			{
				str_comment += tab;
				str_comment += pDataChartWnd->GetChanlistComment(ichan);
			}
			str_comment += rc;
			// print amplifiers settings (gain & filter), next line
			if (options_viewdata->bChanSettings)
			{
				const WORD channb = pDataChartWnd->GetChanlistSourceChan(ichan);
				const auto pchan_array = GetDocument()->m_pDat->GetpWavechanArray();
				const auto p_chan = pchan_array->get_p_channel(channb);
				cs.Format(_T("headstage=%s  g=%li LP=%i  IN+=%s  IN-=%s"),
					static_cast<LPCTSTR>(p_chan->am_csheadstage), static_cast<long>(p_chan->am_gaintotal), p_chan->am_lowpass, static_cast<LPCTSTR>(p_chan->am_csInputpos), static_cast<LPCTSTR>(p_chan->am_csInputneg));
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
	if (options_viewdata->bVoltageScaleBar && p_spikelist_->GetTotalSpikes() > 0)
	{
		// the following assume that spikes are higher than 1 mV...
		cs_unit = _T(" mV");
		z = m_displaySpk_Shape.GetExtent_mV() / 2.0f;		// half of total height
		k = static_cast<int>(z);					// clip to int
		if ((double(z) - k) > 0.5)					// make sure it is not zero
			k++;
		if (bAll)
		{
			if (k > 0)
				wsprintf(m_szbuf, _T("Vbar=%i %s"), k, static_cast<LPCTSTR>(cs_unit));
			else
				_stprintf_s(m_szbuf, 64, _T("Vbar=%f.3 mV"), z);
			cs_comment = m_szbuf;					// store val into comment
			str_comment = cs_comment + rc;
		}

		// display bar
		const auto p_old_brush = (CBrush*)p_dc->SelectStockObject(BLACK_BRUSH);
		if (k > 0)
			z = static_cast<float>(k) / z;
		const auto vert_bar = static_cast<int>(rect->Height() * z) / 2;

		// compute coordinates of the rect
		CRect rect_vert_bar;		// vertical bar rectangle
		const auto bar_width = CSize(5, 5);
		rect_vert_bar.left = rect->left - options_viewdata->textseparator;
		rect_vert_bar.right = rect_vert_bar.left + bar_width.cx;
		rect_vert_bar.top = rect->top + (rect->Height() - vert_bar) / 2;
		rect_vert_bar.bottom = rect_vert_bar.top + vert_bar;
		p_dc->Rectangle(&rect_vert_bar);
		p_dc->SelectObject(p_old_brush);
	}

	// spike duration
	if (p_spikelist_->GetTotalSpikes() > 0 && bAll)
	{
		z = m_displaySpk_Shape.GetExtent_ms();
		cs_unit = _T(" ms");
		k = static_cast<int>(z);
		wsprintf(m_szbuf, _T("Horz=%i."), k);
		cs_comment = m_szbuf;		// store val into comment
		k = static_cast<int>(1000.0f * (z - float(k)));
		wsprintf(m_szbuf, _T("%i %s"), k, static_cast<LPCTSTR>(cs_unit));
		cs_comment += m_szbuf;		// store val into comment
		str_comment += cs_comment;
		str_comment += rc;
	}

	// number of spikes
	k = p_spikelist_->GetTotalSpikes();
	wsprintf(m_szbuf, _T("n spk= %i"), k);
	cs_comment = m_szbuf;
	str_comment += cs_comment;
	str_comment += rc;

	return str_comment;
}

void CViewSpikeDetection::SerializeWindowsState(BOOL bSave, int itab)
{
	auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());	// pointer to list of pointers to store parameters
	if (itab < 0 || itab >= m_tabCtrl.GetItemCount())
	{
		int itab_sel = m_tabCtrl.GetCurSel();				// current selected tab
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
		m_displaySpk_BarView.Serialize(ar);
		m_displaySpk_Shape.Serialize(ar);
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
			m_displaySpk_BarView.Serialize(ar);
			m_displaySpk_Shape.Serialize(ar);
			ar.Close();					// close archive
		}
		else
		{
			*m_ChartDataWnd_Source.GetScopeParameters() = options_viewdata->viewspkdetectdata;
			*m_ChartDataWnd_Detect.GetScopeParameters() = options_viewdata->viewspkdetectfiltered;
			*m_displaySpk_BarView.GetScopeParameters() = options_viewdata->viewspkdetectspk;
			*m_displaySpk_Shape.GetScopeParameters() = options_viewdata->viewspkdetectbars;
			//OnFormatSplitcurves();
		}
	}
}

BOOL CViewSpikeDetection::OnPreparePrinting(CPrintInfo* pInfo)
{
	// save current state of the windows
	SerializeWindowsState(BSAVE);

	// printing margins
	if (options_viewdata->vertRes <= 0 || options_viewdata->horzRes <= 0
		|| options_viewdata->horzRes != pInfo->m_rectDraw.Width()
		|| options_viewdata->vertRes != pInfo->m_rectDraw.Height())
		PrintComputePageSize();

	// nb print pages?
	int npages = PrintGetNPages();
	pInfo->SetMaxPage(npages);			// one page printing/preview
	pInfo->m_nNumPreviewPages = 1;  	// preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	//------------------------------------------------------
	if (options_viewdata->bPrintSelection != pInfo->m_pPD->PrintSelection())
	{
		options_viewdata->bPrintSelection = pInfo->m_pPD->PrintSelection();
		npages = PrintGetNPages();
		pInfo->SetMaxPage(npages);
	}
	return TRUE;
}

int	CViewSpikeDetection::PrintGetNPages()
{
	// how many rows per page?
	const auto size_row = options_viewdata->HeightDoc + options_viewdata->heightSeparator;
	m_nbrowsperpage = m_printRect.Height() / size_row;
	if (m_nbrowsperpage == 0)		// prevent zero pages
		m_nbrowsperpage = 1;

	int ntotal_rows;						// number of rectangles -- or nb of rows
	auto pdb_doc = GetDocument();

	// compute number of rows according to bmultirow & bentirerecord flag
	m_lprintFirst = m_ChartDataWnd_Detect.GetDataFirst();
	m_lprintLen = m_ChartDataWnd_Detect.GetDataLast() - m_lprintFirst + 1;
	m_file0 = GetDocument()->GetDB_CurrentRecordPosition();
	ASSERT(m_file0 >= 0);
	m_nfiles = 1;
	auto ifile0 = m_file0;
	auto ifile1 = m_file0;
	if (!options_viewdata->bPrintSelection)
	{
		ifile0 = 0;
		m_nfiles = pdb_doc->GetDB_NRecords();
		ifile1 = m_nfiles;
	}

	// only one row per file
	if (!options_viewdata->bMultirowDisplay || !options_viewdata->bEntireRecord)
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
			auto nrows = len / m_lprintLen;	// how many rows for this file?
			if (len > nrows * m_lprintLen)	// remainder?
				nrows++;
			ntotal_rows += static_cast<int>(nrows);		// update nb of rows
		}
	}

	if (m_file0 >= 0)
	{
		try
		{
			pdb_doc->SetDB_CurrentRecordPosition(m_file0);
			pdb_doc->OpenCurrentDataFile();
		}
		catch (CDaoException* e) { DisplayDaoException(e, 3); e->Delete(); }
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
	memset(&m_logFont, 0, sizeof(LOGFONT));			// prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial"));		// Arial font
	m_logFont.lfHeight = options_viewdata->fontsize;			// font height
	m_pOldFont = nullptr;
	m_fontPrint.CreateFontIndirect(&m_logFont);
}

void CViewSpikeDetection::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_pOldFont = p_dc->SelectObject(&m_fontPrint);

	// --------------------- RWhere = rectangle/row in which we plot the data, rWidth = row width
	const auto r_width = options_viewdata->WidthDoc;					// page margins
	const auto r_height = options_viewdata->HeightDoc;					// page margins
	CRect r_where(m_printRect.left, 				// printing rectangle for one line of data
		m_printRect.top,
		m_printRect.left + r_width,
		m_printRect.top + r_height);
	auto rw2 = r_where;								// printing rectangle - constant
	rw2.OffsetRect(-r_where.left, -r_where.top);		// set RW2 origin = 0,0

	// define spike shape area (rect): same height as data area
	auto r_sp_kheight = options_viewdata->spkheight;
	auto r_sp_kwidth = options_viewdata->spkwidth;
	if (r_sp_kheight == 0)
	{
		r_sp_kheight = r_height - options_viewdata->fontsize * 4;
		r_sp_kwidth = r_sp_kheight / 2;
		if (r_sp_kwidth < MulDiv(r_where.Width(), 10, 100))
			r_sp_kwidth = MulDiv(r_where.Width(), 10, 100);
		options_viewdata->spkheight = r_sp_kheight;
		options_viewdata->spkwidth = r_sp_kwidth;
	}

	// save current draw mode (it will be modified to print only one channel)
	const WORD chan0Drawmode = 1;
	//WORD chan1Drawmode = 0;
	if (!options_viewdata->bFilterDataSource)
		m_ChartDataWnd_Detect.SetChanlistTransformMode(0, 0);

	p_dc->SetMapMode(MM_TEXT);						// change map mode to text (1 pixel = 1 logical point)
	PrintFileBottomPage(p_dc, pInfo);				// print bottom - text, date, etc

	// --------------------- load data corresponding to the first row of current page
	int filenumber;									// file number and file index
	long l_first;									// index first data point / first file
	auto very_last = m_lprintFirst + m_lprintLen;	// index last data point / current file
	const int curpage = pInfo->m_nCurPage;				// get current page number
	PrintGetFileSeriesIndexFromPage(curpage, filenumber, l_first);
	if (l_first < GetDocument()->GetDB_DataLen() - 1)
		UpdateFileParameters(FALSE);
	if (options_viewdata->bEntireRecord)
		very_last = GetDocument()->GetDB_DataLen() - 1;

	// loop through all files	--------------------------------------------------------
	for (auto i = 0; i < m_nbrowsperpage; i++)
	{
		const auto old_dc = p_dc->SaveDC();					// save DC

		// first : set rectangle where data will be printed
		auto comment_rect = r_where;					// save RWhere for comments
		p_dc->SetMapMode(MM_TEXT);					// 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); 				// set text align mode

		// load data and adjust display rectangle ----------------------------------------
		// set data rectangle to half height to the row height
		m_rData = r_where;								// adjust data rect/ file data rect
		if (options_viewdata->bPrintSpkBars)
			m_rData.bottom = m_rData.top + r_where.Height() / 2; // half vertic size = data
		m_rData.left += (r_sp_kwidth + options_viewdata->textseparator);
		const auto oldsize = m_rData.Width();					// save display rect size

		// make sure enough data fit into this rectangle, otherwise clip rect
		auto l_last = l_first + m_lprintLen;				// compute last pt to load
		if (l_last > very_last)							// check end across file length
			l_last = very_last;
		if ((l_last - l_first + 1) < m_lprintLen)				// adjust rect to length of data
			m_rData.right = (oldsize * (l_last - l_first)) / m_lprintLen + m_rData.left;
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

				// if option requested, clip output to rect
		if (options_viewdata->bClipRect)							// clip curve display
			p_dc->IntersectClipRect(&m_rData);			// (eventually)

		// print detected channel only data
		m_ChartDataWnd_Detect.SetChanlistflagPrintVisible(0, chan0Drawmode);
		m_ChartDataWnd_Detect.ResizeChannels(m_rData.Width(), 0);
		m_ChartDataWnd_Detect.GetDataFromDoc(l_first, l_last);	// load data from file
		m_ChartDataWnd_Detect.Print(p_dc, &m_rData);			// print data
		p_dc->SelectClipRgn(nullptr);						// no more clipping

		// print spike bars ---------------------------------------------------------------
		if (options_viewdata->bPrintSpkBars)
		{
			CRect BarsRect = r_where;					// adjust data rect/ file data rect
			BarsRect.top = m_rData.bottom;				// copy coordinates
			BarsRect.left = m_rData.left;
			BarsRect.right = m_rData.right;

			m_displaySpk_BarView.SetTimeIntervals(l_first, l_last);	// define time interval
			m_displaySpk_BarView.Print(p_dc, &BarsRect);			// print data
		}

		// print spkform within a square (same width as height) ---------------------------
		m_rSpike = r_where;								// get file rectangle
		m_rSpike.right = m_rSpike.left + r_sp_kwidth;
		m_rSpike.left += options_viewdata->textseparator;			// remove space for 4 lines of
		m_rSpike.bottom = m_rSpike.top + r_sp_kheight;	// legends

		m_displaySpk_Shape.SetTimeIntervals(l_first, l_last);
		m_displaySpk_Shape.Print(p_dc, &m_rSpike);

		// restore DC and print comments ---------------------------------------------------
		p_dc->RestoreDC(old_dc);						// restore Display context
		p_dc->SetMapMode(MM_TEXT);					// 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr);					// no more clipping
		p_dc->SetViewportOrg(0, 0);					// org = 0,0

		// print data Bars & get comments according to row within file
		CString cs_comment;
		const BOOL b_all = (l_first == m_lprintFirst);
		if (b_all)									// first row = full comment
		{
			cs_comment += PrintGetFileInfos();		// describe file, intervals & comments /chan
			cs_comment += PrintDataBars(p_dc, &m_ChartDataWnd_Detect, &m_rData);	// bars and bar legends
		}
		else
		{	// other rows: time intervals only
			cs_comment = PrintConvertFileIndex(m_ChartDataWnd_Detect.GetDataFirst(), m_ChartDataWnd_Detect.GetDataLast());
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_viewdata->textseparator + comment_rect.Width(), 0);
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
		m_rSpike.left -= options_viewdata->textseparator;
		m_rSpike.top = m_rSpike.bottom;
		m_rSpike.bottom += m_logFont.lfHeight * 3;
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), m_rSpike, n_format);
		p_dc->SetTextAlign(ui_flag);
		//--_____________________________________________________________________--------
		//--|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||--------

				// update file parameters for next row --------------------------------------------

		r_where.OffsetRect(0, r_height + options_viewdata->heightSeparator);
		const auto ifile = filenumber;
		if (!PrintGetNextRow(filenumber, l_first, very_last))
		{
			//i = m_nbrowsperpage;
			break;
		}
		if (ifile != filenumber)
		{
			UpdateFileParameters(FALSE);
			m_ChartDataWnd_Detect.SetChanlistflagPrintVisible(0, 0); // cancel printing channel zero
		}
	}

	// end of file loop : restore initial conditions
	m_ChartDataWnd_Detect.SetChanlistflagPrintVisible(0, 1);
	if (!options_viewdata->bFilterDataSource)
		m_ChartDataWnd_Detect.SetChanlistTransformMode(0, m_pDetectParms->detectTransform);

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
	m_displaySpk_Shape.SetTimeIntervals(m_lFirst0, m_lLast0);
	UpdateFileParameters(TRUE);

	m_bIsPrinting = FALSE;
	SerializeWindowsState(BRESTORE);
}

void CViewSpikeDetection::OnBnClickedBiasbutton()
{
	((CButton*)GetDlgItem(IDC_BIAS))->SetState(1);
	((CButton*)GetDlgItem(IDC_GAIN))->SetState(0);
	SetVBarMode(BAR_BIAS, IDC_SCROLLY);
}

void CViewSpikeDetection::OnBnClickedBias2()
{
	((CButton*)GetDlgItem(IDC_BIAS2))->SetState(1);
	((CButton*)GetDlgItem(IDC_GAIN2))->SetState(0);
	SetVBarMode(BAR_BIAS, IDC_SCROLLY2);
}

void CViewSpikeDetection::OnBnClickedGainbutton()
{
	((CButton*)GetDlgItem(IDC_BIAS))->SetState(0);
	((CButton*)GetDlgItem(IDC_GAIN))->SetState(1);
	SetVBarMode(BAR_GAIN, IDC_SCROLLY);
}

void CViewSpikeDetection::OnBnClickedGain2()
{
	((CButton*)GetDlgItem(IDC_BIAS2))->SetState(0);
	((CButton*)GetDlgItem(IDC_GAIN2))->SetState(1);
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
		CDaoRecordView::OnVScroll(nSBCode, nPos, pScrollBar);
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
			MulDiv(m_ChartDataWnd_Detect.GetChanlistYextent(m_ichanselected), 100, YEXTENT_MAX) + 50, TRUE);
	else
		m_scrolly2.SetScrollPos(
			MulDiv(m_ChartDataWnd_Source.GetChanlistYextent(m_ichanselected2), 100, YEXTENT_MAX) + 50, TRUE);
}

void CViewSpikeDetection::OnGainScroll(UINT nSBCode, UINT nPos, int iID)
{
	CChartDataWnd* p_view = &m_ChartDataWnd_Detect;
	int ichan = m_ichanselected;
	if (iID == IDC_SCROLLY2)
	{
		p_view = &m_ChartDataWnd_Source;
		ichan = m_ichanselected2;
	}
	int lSize = p_view->GetChanlistYextent(ichan);

	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:		lSize = YEXTENT_MIN; break;		// .................scroll to the start
	case SB_LINELEFT:	lSize -= lSize / 10 + 1; break;	// .................scroll one line left
	case SB_LINERIGHT:	lSize += lSize / 10 + 1; break;	// .................scroll one line right
	case SB_PAGELEFT:	lSize -= lSize / 2 + 1; break;		// .................scroll one page left
	case SB_PAGERIGHT:	lSize += lSize + 1; break;		// .................scroll one page right
	case SB_RIGHT:		lSize = YEXTENT_MAX; break;		// .................scroll to end right
	case SB_THUMBPOSITION:			// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBTRACK:	lSize = MulDiv(nPos - 50, YEXTENT_MAX, 100); break;
	default:			break;							// .................NOP: set position only
	}
	// change y extent
	if (lSize > 0) //&& lSize<=YEXTENT_MAX)
	{
		p_view->SetChanlistYextent(ichan, lSize);
		UpdateLegends();
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		UpdateGainScroll(iID);
}

void CViewSpikeDetection::UpdateBiasScroll(int iID)
{
	if (iID == IDC_SCROLLY)
	{
		const auto i_pos = int((m_ChartDataWnd_Detect.GetChanlistYzero(m_ichanselected) - m_ChartDataWnd_Detect.GetChanlistBinZero(m_ichanselected))
			* 100 / int(YZERO_SPAN)) + int(50);
		m_scrolly.SetScrollPos(i_pos, TRUE);
	}
	else
	{
		const auto i_pos = static_cast<int>((m_ChartDataWnd_Source.GetChanlistYzero(m_ichanselected2) - m_ChartDataWnd_Source.GetChanlistBinZero(
			m_ichanselected2))
			* 100 / int(YZERO_SPAN)) + int(50);
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

	auto l_size = p_view->GetChanlistYzero(ichan) - p_view->GetChanlistBinZero(ichan);
	const auto yextent = p_view->GetChanlistYextent(ichan);
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			l_size = YZERO_MIN; break;		// scroll to the start
	case SB_LINELEFT:		l_size -= yextent / 100 + 1; break;	// scroll one line left
	case SB_LINERIGHT:		l_size += yextent / 100 + 1; break;	// scroll one line right
	case SB_PAGELEFT:		l_size -= yextent / 10 + 1; break;	// scroll one page left
	case SB_PAGERIGHT:		l_size += yextent / 10 + 1; break;	// scroll one page right
	case SB_RIGHT:			l_size = YZERO_MAX; break;		// scroll to end right
	case SB_THUMBPOSITION:	// scroll to pos = nPos	// drag scroll box -- pos = nPos
	case SB_THUMBTRACK:		l_size = (nPos - 50) * (YZERO_SPAN / 100); break;
	default:				break;							// NOP: set position only
	}

	// try to read data with this new size
	if (l_size > YZERO_MIN && l_size < YZERO_MAX)
	{
		p_view->SetChanlistYzero(ichan, l_size + p_view->GetChanlistBinZero(ichan));
		p_view->Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		UpdateBiasScroll(iID);
}

void CViewSpikeDetection::OnEnChangeSpkWndAmplitude()
{
	if (mm_spkWndAmplitude.m_bEntryDone) {
		auto y = m_displaySpk_Shape.GetExtent_mV();
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
		const auto y_we = static_cast<int>(float(m_displaySpk_Shape.GetYWExtent()) * y / yold);
		m_displaySpk_Shape.SetYWExtOrg(y_we, m_displaySpk_Shape.GetYWOrg());
		m_displaySpk_Shape.SetyScaleUnitValue(y / m_displaySpk_Shape.GetNyScaleCells());
		m_displaySpk_Shape.Invalidate();

		// update the dialog control
		mm_spkWndAmplitude.m_bEntryDone = FALSE;
		mm_spkWndAmplitude.m_nChar = 0;
		mm_spkWndAmplitude.SetSel(0, -1); 	//select all text
		cs.Format(_T("%.3f"), y);
		GetDlgItem(IDC_SPIKEWINDOWAMPLITUDE)->SetWindowText(cs);
	}
}

void CViewSpikeDetection::OnEnChangeSpkWndLength()
{
	if (mm_spkWndDuration.m_bEntryDone) {
		auto x = m_displaySpk_Shape.GetExtent_ms();
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
		const auto x_we = static_cast<int>(float(m_displaySpk_Shape.GetXWExtent()) * x / xold);
		m_displaySpk_Shape.SetXWExtOrg(x_we, m_displaySpk_Shape.GetXWOrg());
		m_displaySpk_Shape.SetxScaleUnitValue(x / m_displaySpk_Shape.GetNxScaleCells());
		m_displaySpk_Shape.Invalidate();

		// update the dialog control
		mm_spkWndDuration.m_bEntryDone = FALSE;
		mm_spkWndDuration.m_nChar = 0;
		mm_spkWndDuration.SetSel(0, -1); 	//select all text
		cs.Format(_T("%.3f"), x);
		GetDlgItem(IDC_SPIKEWINDOWLENGTH)->SetWindowText(cs);
	}
}

void CViewSpikeDetection::OnBnClickedLocatebttn()
{
	int max, min;
	m_ChartDataWnd_Detect.GetChanlistMaxMin(0, &max, &min);

	// modify value
	m_pDetectParms->detectThreshold = (max + min) / 2;
	m_thresholdval = m_ChartDataWnd_Detect.ConvertChanlistDataBinsToMilliVolts(0, m_pDetectParms->detectThreshold);
	m_pDetectParms->detectThresholdmV = m_thresholdval;
	// update user-interface: edit control and threshold bar in sourceview
	CString cs;
	cs.Format(_T("%.3f"), m_thresholdval);
	GetDlgItem(IDC_THRESHOLDVAL)->SetWindowText(cs);
	m_ChartDataWnd_Detect.MoveHZtagtoVal(0, m_pDetectParms->detectThreshold);
	m_ChartDataWnd_Detect.Invalidate();
}

void CViewSpikeDetection::UpdateDetectionSettings(int iSelParms)
{
	// check size of spike detection parameters
	if (iSelParms >= m_parmsCurrent.GetSize())
	{
		// load new set of parameters from spike list
		const auto isize = p_spike_doc_->GetSpkList_Size();
		m_parmsCurrent.SetSize(isize);
		for (int i = 0; i < isize; i++)
		{
			// select new spike list (list with no spikes for stimulus channel)
			auto pspklist = p_spike_doc_->SetSpkList_AsCurrent(i);
			ASSERT(pspklist != NULL);
			const auto ps_d = pspklist->GetDetectParms();
			m_parmsCurrent.SetItem(i, ps_d);		// copy content of spklist parm into m_parmsCurrent
		}
	}

	// loop over each spike detection set to update spike detection parameters
	for (auto i = 0; i < m_parmsCurrent.GetSize(); i++)
	{
		// select new spike list (list with no spikes for stimulus channel)
		auto pspklist = p_spike_doc_->SetSpkList_AsCurrent(i);
		const auto p_sd = m_parmsCurrent.GetItem(i);
		if (pspklist == nullptr)
		{
			p_spike_doc_->AddSpkList();
			pspklist = p_spike_doc_->GetSpkList_Current();
			pspklist->InitSpikeList(GetDocument()->m_pDat, p_sd);
		}
		else
			pspklist->SetDetectParms(p_sd);
	}

	// set new parameters
	p_spikelist_->m_selspike = m_spikeno;			// save spike selected
	m_iDetectParms = iSelParms;
	//GetDocument()->GetcurrentSpkDocument()->SetSpkList_CurrentIndex(iSelParms);
	m_pDetectParms = m_parmsCurrent.GetItem(iSelParms);
	p_spikelist_ = p_spike_doc_->SetSpkList_AsCurrent(iSelParms);
	if (p_spikelist_ != nullptr)
		HighlightSpikes(TRUE);

	UpdateDetectionControls();
	UpdateLegends();
}

void CViewSpikeDetection::UpdateDetectionControls()
{
	m_CBdetectWhat.SetCurSel(p_spikelist_->GetdetectWhat());
	m_CBdetectChan.SetCurSel(p_spikelist_->GetdetectChan());
	m_CBtransform.SetCurSel(p_spikelist_->GetdetectTransform());	// this should remains = to detection parameters

	// check that spike detection parameters are compatible with current data doc
	auto detectchan = p_spikelist_->GetdetectChan();
	const auto p_dat = GetDocument()->m_pDat;;
	const int maxchan = p_dat->GetpWaveFormat()->scan_count;
	if (detectchan >= maxchan)
		detectchan = 0;

	m_ChartDataWnd_Detect.SetChanlistOrdinates(0, detectchan, p_spikelist_->GetdetectTransform());
	m_CBtransform2.SetCurSel(p_spikelist_->GetextractTransform());
	m_ChartDataWnd_Detect.SetChanlistColor(0, detectchan);

	m_ChartDataWnd_Detect.GetDataFromDoc(); 		// load data
	//if (options_viewdata->bSplitCurves)
	//	m_displayDataFile.SplitChans();

	const auto ithreshold = p_spikelist_->GetdetectThreshold();
	m_thresholdval = m_ChartDataWnd_Detect.ConvertChanlistDataBinsToMilliVolts(0, ithreshold);
	if (m_ChartDataWnd_Detect.GetNHZtags() < 1)
		m_ChartDataWnd_Detect.AddHZtag(ithreshold, 0);
	else
		m_ChartDataWnd_Detect.SetHZtagVal(0, ithreshold);

	// update spike channel displayed
	m_displaySpk_BarView.SetSpkList(p_spikelist_);
	m_displaySpk_Shape.SetSpkList(p_spikelist_);
}

void CViewSpikeDetection::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	SerializeWindowsState(BSAVE, m_iDetectParms);
	const auto icursel = m_tabCtrl.GetCurSel();
	SerializeWindowsState(BRESTORE, icursel);
	UpdateDetectionSettings(icursel);
	*pResult = 0;
}

void CViewSpikeDetection::OnToolsEditstimulus()
{
	p_spike_doc_->SortStimArray();

	CEditStimArrayDlg dlg;
	dlg.intervalsandlevels_ptr_array.RemoveAll();
	dlg.intervalsandlevels_ptr_array.Add(&p_spike_doc_->m_stimIntervals);
	dlg.m_rate = m_samplingRate;
	dlg.m_pstimsaved = &GetDocument()->m_stimsaved;

	if (IDOK == dlg.DoModal())
	{
		UpdateVTtags();
		m_displaySpk_BarView.Invalidate();
		m_ChartDataWnd_Detect.Invalidate();
		m_ChartDataWnd_Source.Invalidate();
		p_spike_doc_->SetModifiedFlag(TRUE);
	}
}

void CViewSpikeDetection::OnEnChangeChanselected()
{
	if (mm_ichanselected.m_bEntryDone) {
		switch (mm_ichanselected.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);		// load data from edit controls
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
	if (mm_ichanselected2.m_bEntryDone) {
		switch (mm_ichanselected2.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);		// load data from edit controls
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
	const auto method = m_CBtransform2.GetCurSel();				// this is the extract method requested
	auto p_dat_doc = GetDocument()->m_pDat;

	auto const doc_chan = p_spikelist_->GetextractChan();		// source channel
	//auto const nchans = p_dat_doc->GetpWaveFormat()->scan_count;	// number of data chans / source buffer
	short const offset = 1; //(method>0) ? 1 : nchans;				// offset between points / detection
	short const nspan = p_dat_doc->GetTransfDataSpan(method);		// nb pts to read before transf
	p_spikelist_->SetextractTransform(method);					// save transform method

	// pre-load data
	const auto spikelen = p_spikelist_->GetSpikeLength();
	const auto spkpretrig = p_spikelist_->GetSpikePretrig();
	auto iitime = p_spikelist_->GetSpikeTime(0) - spkpretrig;
	auto l_rw_first0 = iitime - spikelen;
	auto l_rw_last0 = iitime + spikelen;
	if (!p_dat_doc->LoadRawData(&l_rw_first0, &l_rw_last0, nspan))
		return;													// exit if error reported
	auto p_data = p_dat_doc->LoadTransfData(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto totalspikes = p_spikelist_->GetTotalSpikes();
	for (auto ispk = 0; ispk < totalspikes; ispk++)
	{
		// make sure that source data are loaded and get pointer to it (p_data)
		iitime = p_spikelist_->GetSpikeTime(ispk);
		auto l_rw_first = iitime - spkpretrig;						// first point
		auto l_rw_last = l_rw_first + spikelen;						// last pt needed
		if (!p_dat_doc->LoadRawData(&l_rw_first, &l_rw_last, nspan))
			break;												// exit loop if error reported

		p_data = p_dat_doc->LoadTransfData(l_rw_first, l_rw_last, method, doc_chan);
		const auto p_data_spike0 = p_data + (iitime - spkpretrig - l_rw_first) * offset;
		p_spikelist_->TransferDataToSpikeBuffer(ispk, p_data_spike0, offset);  // nchans should be 1 if they come from the transform buffer as data are not interleaved...
		p_spikelist_->CenterSpikeAmplitude(ispk, 0, spikelen, 1); // 1=center average
	}
	p_spike_doc_->SetModifiedFlag(TRUE);

	int max, min;
	p_spikelist_->GetTotalMaxMin(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	m_displaySpk_Shape.SetYWExtOrg(m_displaySpk_Shape.GetYWExtent(), middle);
	m_displaySpk_BarView.SetYWExtOrg(m_displaySpk_Shape.GetYWExtent(), middle);
	UpdateSpkShapeWndScale(FALSE);

	HighlightSpikes(TRUE);								// highlight spikes
	UpdateLegends();
	UpdateTabs();
}

void CViewSpikeDetection::OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto iSelParms = m_tabCtrl.GetCurSel();
	UpdateDetectionSettings(iSelParms);
	*pResult = 0;
}

void CViewSpikeDetection::UpdateTabs()
{
	// load initial data
	const BOOL b_replace = (m_tabCtrl.GetItemCount() == p_spike_doc_->GetSpkList_Size());
	if (!b_replace)
		m_tabCtrl.DeleteAllItems();

	// load list of detection parameters
	const auto currlist = p_spike_doc_->GetSpkList_CurrentIndex();
	for (auto i = 0; i < p_spike_doc_->GetSpkList_Size(); i++)
	{
		CString cs;
		const auto pspklist = p_spike_doc_->SetSpkList_AsCurrent(i);
		cs.Format(_T("#%i %s"), i, static_cast<LPCTSTR>(pspklist->GetComment()));
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
	p_spike_doc_->SetSpkList_AsCurrent(currlist);

	m_iDetectParms = GetDocument()->GetcurrentSpkDocument()->GetSpkList_CurrentIndex();
	m_tabCtrl.SetCurSel(m_iDetectParms);
}