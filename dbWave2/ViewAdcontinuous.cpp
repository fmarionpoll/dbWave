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

IMPLEMENT_DYNCREATE(ViewADcontinuous, CFormView)

ViewADcontinuous::ViewADcontinuous()
	: CFormView(IDD)
{
	m_bEnableActiveAccessibility = FALSE;
	m_AD_yRulerBar.AttachScopeWnd(&m_chartDataAD, FALSE);
}

ViewADcontinuous::~ViewADcontinuous()
= default;

void ViewADcontinuous::DoDataExchange(CDataExchange * pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANALOGTODIGIT, m_Acq32_AD);
	DDX_Control(pDX, IDC_DIGITTOANALOG, m_Acq32_DA);
	DDX_Control(pDX, IDC_COMBOBOARD, m_ADcardCombo);
	DDX_Control(pDX, IDC_STARTSTOP, m_btnStartStop_AD);
	DDX_CBIndex(pDX, IDC_COMBOSTARTOUTPUT, m_bStartOutPutMode);
	DDX_Control(pDX, IDC_STARTSTOP2, m_Button_StartStop_DA);
	DDX_Control(pDX, IDC_SAMPLINGMODE, m_Button_SamplingMode);
	DDX_Control(pDX, IDC_DAPARAMETERS2, m_Button_OutputChannels);
	DDX_Control(pDX, IDC_WRITETODISK, m_Button_WriteToDisk);
	DDX_Control(pDX, IDC_OSCILLOSCOPE, m_Button_Oscilloscope);
}

BEGIN_MESSAGE_MAP(ViewADcontinuous, CFormView)
	ON_MESSAGE(WM_MYMESSAGE, &ViewADcontinuous::OnMyMessage)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &ViewADcontinuous::OnInputChannels)
	ON_COMMAND(ID_HARDWARE_ADINTERVALS, &ViewADcontinuous::OnSamplingMode)
	ON_COMMAND(ID_HARDWARE_DEFINEEXPERIMENT, &ViewADcontinuous::OnHardwareDefineexperiment)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_SAMPLINGMODE, &ViewADcontinuous::OnSamplingMode)
	ON_BN_CLICKED(IDC_INPUTCHANNELS, &ViewADcontinuous::OnInputChannels)
	ON_BN_CLICKED(IDC_GAIN_button, &ViewADcontinuous::OnBnClickedGainbutton)
	ON_BN_CLICKED(IDC_BIAS_button, &ViewADcontinuous::OnBnClickedBiasbutton)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_DAPARAMETERS2, &ViewADcontinuous::OnBnClickedDaparameters2)
	ON_CBN_SELCHANGE(IDC_COMBOBOARD, &ViewADcontinuous::OnCbnSelchangeComboboard)
	ON_BN_CLICKED(IDC_STARTSTOP, &ViewADcontinuous::OnBnClickedStartstop)
	ON_BN_CLICKED(IDC_WRITETODISK, &ViewADcontinuous::OnBnClickedWriteToDisk)
	ON_BN_CLICKED(IDC_OSCILLOSCOPE, &ViewADcontinuous::OnBnClickedOscilloscope)
	ON_BN_CLICKED(IDC_CARDFEATURES, &ViewADcontinuous::OnBnClickedCardfeatures)
	ON_CBN_SELCHANGE(IDC_COMBOSTARTOUTPUT, &ViewADcontinuous::OnCbnSelchangeCombostartoutput)
	ON_BN_CLICKED(IDC_STARTSTOP2, &ViewADcontinuous::OnBnClickedStartstop2)
	ON_BN_CLICKED(IDC_UNZOOM, &ViewADcontinuous::OnBnClickedUnzoom)
END_MESSAGE_MAP()

void ViewADcontinuous::OnDestroy()
{
	if (m_Acq32_AD.IsInProgress())
		StopAcquisition();

	m_Acq32_DA.StopAndLiberateBuffers();

	if (m_bFoundDTOPenLayerDLL)
	{
		// TODO: save data here 
		if (m_Acq32_AD.GetHDass() != NULL)
			m_Acq32_AD.DeleteBuffers();

		if (m_Acq32_DA.GetHDass() != NULL)
			m_Acq32_DA.DeleteBuffers();
	}

	CFormView::OnDestroy();
	delete m_pBackgroundBrush;
}

HBRUSH ViewADcontinuous::OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor)
{
	HBRUSH hbr;
	switch (nCtlColor)
	{
	case CTLCOLOR_EDIT:
	case CTLCOLOR_MSGBOX:
		// Set color to green on black and return the background brush.
		pDC->SetBkColor(m_backgroundColor);
		hbr = static_cast<HBRUSH>(m_pBackgroundBrush->GetSafeHandle());
		break;

	default:
		hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
		break;
	}
	return hbr;
}

BEGIN_EVENTSINK_MAP(ViewADcontinuous, CFormView)

	ON_EVENT(ViewADcontinuous, IDC_ANALOGTODIGIT, 1, ViewADcontinuous::OnBufferDone_ADC, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_ANALOGTODIGIT, 2, ViewADcontinuous::OnQueueDone_ADC, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_ANALOGTODIGIT, 4, ViewADcontinuous::OnTriggerError_ADC, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_ANALOGTODIGIT, 5, ViewADcontinuous::OnOverrunError_ADC, VTS_NONE)

	ON_EVENT(ViewADcontinuous, IDC_DIGITTOANALOG, 1, ViewADcontinuous::OnBufferDone_DAC, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_DIGITTOANALOG, 5, ViewADcontinuous::OnOverrunError_DAC, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_DIGITTOANALOG, 2, ViewADcontinuous::OnQueueDone_DAC, VTS_NONE)
	ON_EVENT(ViewADcontinuous, IDC_DIGITTOANALOG, 4, ViewADcontinuous::OnTriggerError_DAC, VTS_NONE)

END_EVENTSINK_MAP()

void ViewADcontinuous::AttachControls()
{
	// attach controls
	VERIFY(m_chartDataAD.SubclassDlgItem(IDC_DISPLAYDATA, this));
	VERIFY(m_AD_yRulerBar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_AD_xRulerBar.SubclassDlgItem(IDC_XSCALE, this));
	VERIFY(m_BiasButton.SubclassDlgItem(IDC_BIAS_button, this));
	VERIFY(m_ZoomButton.SubclassDlgItem(IDC_GAIN_button, this));
	VERIFY(m_UnZoomButton.SubclassDlgItem(IDC_UNZOOM, this));
	VERIFY(m_ComboStartOutput.SubclassDlgItem(IDC_COMBOSTARTOUTPUT, this));

	m_AD_yRulerBar.AttachScopeWnd(&m_chartDataAD, FALSE);
	m_AD_xRulerBar.AttachScopeWnd(&m_chartDataAD, TRUE);
	m_chartDataAD.AttachExternalXRuler(&m_AD_xRulerBar);
	m_chartDataAD.AttachExternalYRuler(&m_AD_yRulerBar);
	m_chartDataAD.m_bNiceGrid = TRUE;

	m_stretch.AttachParent(this);
	m_stretch.newProp(IDC_DISPLAYDATA, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_XSCALE, XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_YSCALE, SZEQ_XLEQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SCROLLY_scrollbar, SZEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_UNZOOM, SZEQ_XREQ, SZEQ_YTEQ);

	// bitmap buttons: load icons & set buttons
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	m_hUnZoom = AfxGetApp()->LoadIcon(IDI_UNZOOM);
	m_BiasButton.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), LPARAM(static_cast<HANDLE>(m_hBias)));
	m_ZoomButton.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), LPARAM(static_cast<HANDLE>(m_hZoom)));
	m_UnZoomButton.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), LPARAM(static_cast<HANDLE>(m_hUnZoom)));

	const BOOL b32BitIcons = afxGlobalData.m_nBitsPerPixel >= 16;
	m_btnStartStop_AD.SetImage(b32BitIcons ? IDB_CHECK32 : IDB_CHECK);
	m_btnStartStop_AD.SetCheckedImage(b32BitIcons ? IDB_CHECKNO32 : IDB_CHECKNO);
	CMFCButton::EnableWindowsTheming(false);
	m_btnStartStop_AD.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;

	// scrollbar
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);
}

void ViewADcontinuous::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	AttachControls();

	const auto pApp = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	m_pOptions_AD = &(pApp->options_acqdata);

	m_pOptions_DA = &(pApp->options_outputdata);
	m_bFoundDTOPenLayerDLL = FALSE;
	m_bADwritetofile = m_pOptions_AD->waveFormat.bADwritetofile;
	m_bStartOutPutMode = m_pOptions_DA->bAllowDA;
	m_ComboStartOutput.SetCurSel(m_bStartOutPutMode);

	// open document and remove database filters
	CdbWaveDoc* pdbDoc = static_cast<CdbWaveDoc*>(GetDocument());
	m_ptableSet = &pdbDoc->m_pDB->m_mainTableSet;
	m_ptableSet->m_strFilter.Empty();
	m_ptableSet->ClearFilters();
	m_ptableSet->RefreshQuery();

	// if current document, load parameters from current document into the local set of parameters
	if (pdbDoc->m_pDB->GetNRecords() > 0)
	{
		pdbDoc->OpenCurrentDataFile(); 
		AcqDataDoc* pDat = pdbDoc->m_pDat;
		if (pDat != nullptr)
		{
			m_pOptions_AD->waveFormat.Copy( pDat->GetpWaveFormat()); 
			m_pOptions_AD->chanArray.ChanArray_setSize(m_pOptions_AD->waveFormat.scan_count);
			m_pOptions_AD->chanArray.Copy(pDat->GetpWavechanArray());
			m_pOptions_AD->waveFormat.bADwritetofile = m_bADwritetofile;
		}
	}

	// create data file and copy data acquisition parameters into it
	m_inputDataFile.OnNewDocument(); 
	m_inputDataFile.GetpWaveFormat()->Copy( &m_pOptions_AD->waveFormat);
	m_pOptions_AD->chanArray.ChanArray_setSize(m_pOptions_AD->waveFormat.scan_count);
	m_inputDataFile.GetpWavechanArray()->Copy(&m_pOptions_AD->chanArray);
	m_chartDataAD.AttachDataFile(&m_inputDataFile);

	pApp->m_bADcardFound = FindDTOpenLayersBoards();
	if (pApp->m_bADcardFound)
	{
		InitOutput_AD();
		InitializeAmplifiers(); 
		m_Acq32_DA.InitSubSystem(m_pOptions_AD);
		m_Acq32_DA.ClearAllOutputs();
	}
	else
	{
		m_btnStartStop_AD.ShowWindow(SW_HIDE);
		m_Button_SamplingMode.ShowWindow(SW_HIDE);
		m_Button_OutputChannels.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DAGROUP)->ShowWindow(SW_HIDE);
		m_ComboStartOutput.ShowWindow(SW_HIDE);
		m_Button_StartStop_DA.ShowWindow(SW_HIDE);
	}

	UpdateGainScroll();
	UpdateRadioButtons();
	GetParent()->PostMessage(WM_MYMESSAGE, NULL, MAKELPARAM(m_cursorstate, HINT_SETMOUSECURSOR));
}

void ViewADcontinuous::OnCbnSelchangeComboboard()
{
	const int item_selected = m_ADcardCombo.GetCurSel();
	CString card_name;
	m_ADcardCombo.GetLBText(item_selected, card_name);
	SelectDTOpenLayersBoard(card_name);
}

BOOL ViewADcontinuous::FindDTOpenLayersBoards()
{
	m_ADcardCombo.ResetContent();

	// load board name - skip dialog if only one is present
	const short uiNumBoards = m_Acq32_AD.GetNumBoards();
	if (uiNumBoards == 0)
	{
		m_ADcardCombo.AddString(_T("No Board"));
		m_ADcardCombo.SetCurSel(0);
		return FALSE;
	}

	for (short i = 0; i < uiNumBoards; i++)
		m_ADcardCombo.AddString(m_Acq32_AD.GetBoardList(i));

	short isel = 0;
	// if name already defined, check if board present
	if (!(m_pOptions_AD->waveFormat).csADcardName.IsEmpty())
		isel = short(m_ADcardCombo.FindString(-1, (m_pOptions_AD->waveFormat).csADcardName));
	if (isel < 0)
		isel = 0;

	m_ADcardCombo.SetCurSel(isel);
	m_boardName = m_Acq32_AD.GetBoardList(isel);
	SelectDTOpenLayersBoard(m_boardName);
	return TRUE;
}

BOOL ViewADcontinuous::SelectDTOpenLayersBoard(const CString& card_name)
{
	// get infos
	m_bFoundDTOPenLayerDLL = TRUE;
	(m_pOptions_AD->waveFormat).csADcardName = card_name;

	// connect A/D subsystem and display/hide buttons
	m_bStartOutPutMode = 0;
	const BOOL flag_AD = m_Acq32_AD.OpenSubSystem(card_name);
	const BOOL flag_DA = m_Acq32_DA.OpenSubSystem(card_name);
	if (flag_DA)
		m_bStartOutPutMode = 0;
	m_bSimultaneousStart = m_bsimultaneousStartDA && m_Acq32_AD.IsSimultaneousStart();

	// display additional interface elements
	int show = (flag_AD ? SW_SHOW : SW_HIDE);
	m_Button_SamplingMode.ShowWindow(show);
	m_Button_OutputChannels.ShowWindow(show);
	m_ComboStartOutput.ShowWindow(show);

	show = (flag_DA ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DAPARAMETERS2)->ShowWindow(show);
	GetDlgItem(IDC_DAGROUP)->ShowWindow(show);
	m_ComboStartOutput.ShowWindow(show);
	m_Button_StartStop_DA.ShowWindow(show);
	if (show == SW_SHOW)
		SetCombostartoutput(m_pOptions_DA->bAllowDA);

	return TRUE;
}

void ViewADcontinuous::StopAcquisition()
{
	if (!m_Acq32_AD.IsInProgress())
		return;

	// special treatment if simultaneous list
	if (m_bSimultaneousStart && m_bStartOutPutMode == 0)
	{
		HSSLIST hSSlist;
		CHAR error_string[255];
		ECODE ecode = olDaGetSSList(&hSSlist);
		olDaGetErrorString(ecode, error_string, 255);
		ecode = olDaReleaseSSList(hSSlist);
		olDaGetErrorString(ecode, error_string, 255);
	}

	// stop AD, liberate DTbuffers
	m_Acq32_AD.StopAndLiberateBuffers();
	m_chartDataAD.ADdisplayStop();
	m_bchanged = TRUE;

	// stop DA, liberate buffers
	if (m_bStartOutPutMode == 0)
		m_Acq32_DA.StopAndLiberateBuffers();

	// close file and update display
	if (m_bFileOpen)
	{
		save_and_close_file();
		UpdateViewDataFinal();
	}
}

void ViewADcontinuous::save_and_close_file()
{
	m_inputDataFile.AcqDoc_DataAppendStop();
	const CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();

	// if burst data acquisition mode ------------------------------------
	if (m_bhidesubsequent)
	{
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
			m_csNameArray.Add(m_szFileName);
		else
			m_inputDataFile.AcqDeleteFile();
	}

	// normal data acquisition mode --------------------------------------
	else
	{
		int result = IDCANCEL;
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
		{
			DlgConfirmSave dlg;
			dlg.m_csfilename = m_szFileName;
			result = dlg.DoModal();
		}
		// if no data or user answered no, erase the data
		if (IDOK != result)
		{
			m_inputDataFile.AcqDeleteFile();
		}
		else
		{
			// -----------------------------------------------------
			// if current document name is the same, it means something happened and we have erased a previously existing file
			// if so, skip
			// otherwise add data file name to the database
			auto* pdb_doc = static_cast<CdbWaveDoc*>(GetDocument());
			if (m_szFileName.CompareNoCase(pdb_doc->DB_GetCurrentDatFileName(FALSE)) != 0)
			{
				// add document to database
				m_csNameArray.Add(m_szFileName);
				TransferFilesToDatabase();
				UpdateViewDataFinal();
			}
		}
	}
}

void ViewADcontinuous::UpdateViewDataFinal()
{
	// update view data	
	auto* pdb_doc = static_cast<CdbWaveDoc*>(GetDocument());
	AcqDataDoc* pDocDat = pdb_doc->OpenCurrentDataFile();
	if (pDocDat == nullptr)
	{
		ATLTRACE2(_T("error reading current document "));
		return;
	}
	pDocDat->ReadDataInfos();
	const long length_doc_channel = pDocDat->GetDOCchanLength();
	m_chartDataAD.AttachDataFile(pDocDat);
	m_chartDataAD.ResizeChannels(m_chartDataAD.GetRectWidth(), length_doc_channel);
	m_chartDataAD.GetDataFromDoc(0, length_doc_channel);
}

void ViewADcontinuous::TransferFilesToDatabase()
{
	const auto pdbDoc = static_cast<CdbWaveDoc*>(GetDocument());
	pdbDoc->ImportFileList(m_csNameArray); // add file name(s) to the list of records in the database
	m_csNameArray.RemoveAll(); // clear file names

	CdbTableMain* pSet = &(pdbDoc->m_pDB->m_mainTableSet);
	pSet->BuildAndSortIDArrays();
	pSet->RefreshQuery();
	pdbDoc->DB_SetCurrentRecordPosition(pdbDoc->m_pDB->GetNRecords() - 1);
	pdbDoc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
}

BOOL ViewADcontinuous::InitOutput_DA()
{
	m_DA_present = m_Acq32_DA.InitSubSystem(m_pOptions_AD);
	if (m_bStartOutPutMode == 0 && m_DA_present)
		m_Acq32_DA.DeclareAndFillBuffers(m_pOptions_AD);
	return m_DA_present;
}

BOOL ViewADcontinuous::InitOutput_AD()
{
	m_AD_present = m_Acq32_AD.InitSubSystem(m_pOptions_AD);
	if (m_AD_present)
	{
		m_Acq32_AD.DeclareBuffers(m_pOptions_AD);
		InitAcquisitionInputFile();
		InitAcquisitionDisplay();
	}
	return m_AD_present;
}

void ViewADcontinuous::InitAcquisitionDisplay()
{
	const CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);
	m_chartDataAD.AttachDataFile(&m_inputDataFile);
	m_chartDataAD.ResizeChannels(0, m_chsweeplength);
	if (m_chartDataAD.GetChanlistSize() != pWFormat->scan_count)
	{
		m_chartDataAD.RemoveAllChanlistItems();
		for (int j = 0; j < pWFormat->scan_count; j++)
		{
			m_chartDataAD.AddChanlistItem(j, 0);
		}
	}

	// adapt source view 
	int i_extent = MulDiv(pWFormat->binspan, 12, 10);
	if (m_pOptions_AD->izoomCursel != 0)
		i_extent = m_pOptions_AD->izoomCursel;

	for (int i = 0; i < pWFormat->scan_count; i++)
	{
		constexpr int i_offset = 0;
		CChanlistItem* pD = m_chartDataAD.GetChanlistItem(i);
		pD->SetYzero(i_offset);
		pD->SetYextent(i_extent);
		pD->SetColor(static_cast<WORD>(i));
		float doc_volts_per_bin;
		m_inputDataFile.GetWBVoltsperBin(i, &doc_volts_per_bin);
		pD->SetDataBinFormat(pWFormat->binzero, pWFormat->binspan);
		pD->SetDataVoltsFormat(doc_volts_per_bin, pWFormat->fullscale_volts);
	}

	UpdateGainScroll();
	m_chartDataAD.Invalidate();
}

BOOL ViewADcontinuous::StartAcquisition()
{
	// set display
	if (m_bADwritetofile && !define_experiment())
	{
		StopAcquisition();
		UpdateStartStop(m_Acq32_AD.IsInProgress());
		return FALSE;
	}

	InitOutput_DA();
	InitOutput_AD();

	// start AD display
	m_channel_sweep_start = 0;
	m_channel_sweep_end = -1;
	m_chartDataAD.ADdisplayStart(m_chsweeplength);
	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();
	pWFormat->sample_count = 0; // no samples yet
	pWFormat->sampling_rate_per_channel = pWFormat->sampling_rate_per_channel / static_cast<float>(m_pOptions_AD->iundersample);
	m_fclockrate = pWFormat->sampling_rate_per_channel * static_cast<float>(pWFormat->scan_count);
	pWFormat->acqtime = CTime::GetCurrentTime();

	// data format
	pWFormat->binspan = (m_pOptions_AD->waveFormat).binspan;
	pWFormat->fullscale_volts = (m_pOptions_AD->waveFormat).fullscale_volts;
	// trick: if OLx_ENC_BINARY, it is changed on the fly within AD_Transfer function 
	// when a DT buffer into a Coptions_acqdataataDoc buffer
	pWFormat->mode_encoding = OLx_ENC_2SCOMP;
	pWFormat->binzero = 0;

	// start acquisition and save data to file?
	if (m_bADwritetofile && (pWFormat->trig_mode == OLx_TRG_EXTRA + 1))
	{
		if (AfxMessageBox(_T("Start data acquisition"), MB_OKCANCEL) != IDOK)
		{
			StopAcquisition();
			UpdateStartStop(m_Acq32_AD.IsInProgress());
			return FALSE;
		}
	}

	// start
	if (!m_bSimultaneousStart || m_bStartOutPutMode != 0)
	{
		m_Acq32_AD.ConfigAndStart();
		if (m_bStartOutPutMode == 0 && m_DA_present)
			m_Acq32_DA.ConfigAndStart();
	}
	else
	{
		StartSimultaneousList();
	}

	return TRUE;
}

ECODE ViewADcontinuous::StartSimultaneousList()
{
	ECODE code_returned;
	HSSLIST hSSlist;
	CHAR error_string[255];

	// create simultaneous starting list
	ECODE ecode = olDaGetSSList(&hSSlist);
	olDaGetErrorString(ecode, error_string, 255);

	// DA system
	m_Acq32_DA.Config();
	ecode = olDaPutDassToSSList(hSSlist, (HDASS)m_Acq32_DA.GetHDass());
	if (ecode != OLNOERROR)
	{
		code_returned = olDaReleaseSSList(hSSlist);
		TRACE("error %i \n", code_returned);
		return ecode;
	}

	// AD system
	m_Acq32_AD.Config();
	ecode = olDaPutDassToSSList(hSSlist, (HDASS)m_Acq32_AD.GetHDass());
	if (ecode != OLNOERROR)
	{
		code_returned = olDaReleaseSSList(hSSlist);
		TRACE("error %i \n", code_returned);
		return ecode;
	}

	// prestart
	ecode = olDaSimultaneousPrestart(hSSlist);
	if (ecode != OLNOERROR)
	{
		olDaGetErrorString(ecode, error_string, 255);
		display_ol_da_error_message(error_string);
	}

	// start simultaneously
	ecode = olDaSimultaneousStart(hSSlist);
	if (ecode != OLNOERROR)
	{
		olDaGetErrorString(ecode, error_string, 255);
		display_ol_da_error_message(error_string);
	}

	m_Acq32_AD.SetInProgress();
	m_Acq32_DA.SetInProgress();
	return ecode;
}

void ViewADcontinuous::display_ol_da_error_message(const CHAR * error_string) const
{
	CString csError;
	const CStringA string_a(error_string);
	csError = string_a;
	AfxMessageBox(csError);
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
	return m_ptableSet;
}

void ViewADcontinuous::OnUpdate(CView * pSender, LPARAM lHint, CObject * pHint)
{
	// update sent from within this class
	if (pSender == this)
	{
		ASSERT(GetDocument() != NULL);
		m_chartDataAD.Invalidate(); // display data
	}
}

void ViewADcontinuous::OnActivateView(BOOL bActivate, CView * pActivateView, CView * pDeactiveView)
{
	const auto pmF = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (bActivate)
	{
		pmF->ActivatePropertyPane(FALSE);
		dynamic_cast<CChildFrame*>(pmF->MDIGetActive())->m_cursor_state = 0;
	}
	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void ViewADcontinuous::OnSize(UINT nType, int cx, int cy)
{
	switch (nType)
	{
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
		if (m_Acq32_AD.IsInProgress())
		{
			StopAcquisition();
			UpdateStartStop(false);
		}

		if (cx <= 0 || cy <= 0)
			break;
		m_stretch.ResizeControls(nType, cx, cy);
		break;
	default:
		break;
	}
	CFormView::OnSize(nType, cx, cy);
}

LRESULT ViewADcontinuous::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	// message emitted by IDC_DISPLAREA_button	
	//if (j == IDC_DISPLAREA_button)		// always true here...
	//int j = wParam;				// control ID of sender

	// parameters
	const int code = HIWORD(lParam); // code parameter
	// code = 0: chan hit 			lowp = channel
	// code = 1: cursor change		lowp = new cursor value
	// code = 2: horiz cursor hit	lowp = cursor index	
	int lowp = LOWORD(lParam); // value associated with code

	if (code == HINT_SETMOUSECURSOR) {
		if (lowp > CURSOR_ZOOM)
			lowp = 0;
		m_cursorstate = m_chartDataAD.SetMouseCursorType(lowp);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
	}
	else {
		if (lowp == 0)
			MessageBeep(MB_ICONEXCLAMATION);
	}
	return 0L;
}

void ViewADcontinuous::OnBnClickedStartstop()
{
	if (m_btnStartStop_AD.IsChecked())
	{
		if (StartAcquisition())
		{
			if ((m_inputDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
				OnBufferDone_ADC();
		}
		else
		{
			StopAcquisition();
		}
	}
	else
	{
		StopAcquisition();
		if (m_bhidesubsequent)
		{
			TransferFilesToDatabase();
			UpdateViewDataFinal();
		}
		else if (m_bchanged && m_bAskErase)
			if (AfxMessageBox(IDS_ACQDATA_SAVEYESNO, MB_YESNO) == IDYES)
				m_bchanged = FALSE;
	}
	UpdateStartStop(m_Acq32_AD.IsInProgress());
}

void ViewADcontinuous::UpdateStartStop(BOOL bStart)
{
	//bool isrunning = m_Acq32_AD.IsInProgress();
	if (bStart)
	{
		m_btnStartStop_AD.SetWindowText(_T("STOP"));
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);
	}
	else
	{
		m_btnStartStop_AD.SetWindowText(_T("START"));
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
		if (m_pOptions_AD->baudiblesound)
			Beep(500, 400);
		ASSERT(m_Acq32_AD.IsInProgress() == FALSE);
	}
	m_btnStartStop_AD.SetCheck(bStart);
	m_chartDataAD.Invalidate();
}

void ViewADcontinuous::OnHardwareDefineexperiment()
{
	define_experiment();
}

BOOL ViewADcontinuous::define_experiment()
{
	CString file_name;
	m_bFileOpen = FALSE;
	if (!m_bhidesubsequent)
	{
		DlgADExperiment dlg;
		dlg.m_bFilename = true;
		dlg.m_pADC_options = m_pOptions_AD;
		dlg.m_pdbDoc = static_cast<CdbWaveDoc*>(GetDocument());
		dlg.m_bhidesubsequent = m_bhidesubsequent;
		if (IDOK != dlg.DoModal())
			return FALSE;
		m_bhidesubsequent = dlg.m_bhidesubsequent;
		file_name = dlg.m_szFileName;
	}

	// hide define experiment dialog
	else
	{
		// build file name
		CString csBufTemp;
		m_pOptions_AD->exptnumber++;
		csBufTemp.Format(_T("%06.6lu"), m_pOptions_AD->exptnumber);
		file_name = m_pOptions_AD->csPathname + m_pOptions_AD->csBasename + csBufTemp + _T(".dat");

		// check if this file is already present, exit if not...
		CFileStatus status;
		int iIDresponse = IDYES; 
		if (CFile::GetStatus(file_name, status))
			iIDresponse = AfxMessageBox(IDS_FILEOVERWRITE, MB_YESNO | MB_ICONWARNING);
		// no .. find first available number
		while (IDNO == iIDresponse)
		{
			BOOL flag = TRUE;
			while (flag)
			{
				m_pOptions_AD->exptnumber++;
				csBufTemp.Format(_T("%06.6lu"), m_pOptions_AD->exptnumber);
				file_name = m_pOptions_AD->csPathname + m_pOptions_AD->csBasename + csBufTemp + _T(".dat");
				flag = CFile::GetStatus(file_name, status);
			}
			const CString cs = _T("Next available file name: ") + file_name;
			iIDresponse = AfxMessageBox(cs, MB_YESNO | MB_ICONWARNING);
		}
	}
	// close current file and open new file to prepare it for adding chunks of data
	m_inputDataFile.AcqCloseFile();
	if (!m_inputDataFile.CreateAcqFile(file_name))
		return FALSE;
	m_szFileName = file_name;
	m_inputDataFile.AcqDoc_DataAppendStart();
	m_bFileOpen = TRUE;
	return TRUE;
}

void ViewADcontinuous::OnInputChannels()
{
	UpdateData(TRUE);

	DlgADInputs dlg;
	dlg.m_pwFormat = &(m_pOptions_AD->waveFormat);
	dlg.m_pchArray = &(m_pOptions_AD->chanArray);
	dlg.m_numchansMAXDI = m_Acq32_AD.GetSSCaps(OLSSC_MAXDICHANS);
	dlg.m_numchansMAXSE = m_Acq32_AD.GetSSCaps(OLSSC_MAXSECHANS);
	dlg.m_bchantype = m_pOptions_AD->bChannelType;
	dlg.m_bchainDialog = TRUE;
	dlg.m_bcommandAmplifier = TRUE;

	const auto p_alligator = new CUSBPxxS1();
	dlg.m_palligator = p_alligator;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_pOptions_AD->bChannelType = dlg.m_bchantype;
		InitOutput_AD();
		UpdateData(FALSE);
		UpdateGainScroll();
	}
	delete p_alligator;

	if (dlg.m_postmessage != NULL)
		ChainDialog(dlg.m_postmessage);
}

void ViewADcontinuous::OnSamplingMode()
{
	DlgADIntervals dlg;
	CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);
	dlg.m_p_wave_format = pWFormat;
	dlg.m_rate_minimum = 1.0f;
	dlg.m_rate_maximum = static_cast<float>(m_Acq32_AD.GetMaximumFrequency() / pWFormat->scan_count);
	dlg.m_buffer_W_size_maximum = static_cast<UINT>(65536) * 4;
	dlg.m_under_sample_factor = m_pOptions_AD->iundersample;
	dlg.m_b_audible_sound = m_pOptions_AD->baudiblesound;
	dlg.m_acquisition_duration = m_pOptions_AD->duration_to_acquire;
	dlg.m_sweep_duration = m_sweepduration;
	dlg.m_b_chain_dialog = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		if (m_Acq32_AD.IsInProgress()) {
			StopAcquisition();
			UpdateStartStop(m_Acq32_AD.IsInProgress());
		}

		m_pOptions_AD->iundersample = static_cast<int>(dlg.m_under_sample_factor);
		m_pOptions_AD->baudiblesound = dlg.m_b_audible_sound;
		m_pOptions_AD->duration_to_acquire = dlg.m_acquisition_duration;
		m_sweepduration = dlg.m_sweep_duration;
		m_pOptions_AD->sweepduration = m_sweepduration;
		InitOutput_AD();
		UpdateData(FALSE);
	}

	if (dlg.m_postmessage != NULL)
		ChainDialog(dlg.m_postmessage);
}

void ViewADcontinuous::ChainDialog(WORD iID)
{
	WORD menuID;
	switch (iID)
	{
	case IDC_ADINTERVALS:
		menuID = ID_HARDWARE_ADINTERVALS;
		break;
	case IDC_ADCHANNELS:
		menuID = ID_HARDWARE_ADCHANNELS;
		break;
	default:
		return;
	}
	PostMessage(WM_COMMAND, menuID, NULL);
}

void ViewADcontinuous::OnTriggerError_ADC()
{
	StopAcquisition();
	UpdateStartStop(m_Acq32_AD.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void ViewADcontinuous::OnQueueDone_ADC()
{
	StopAcquisition();
	UpdateStartStop(m_Acq32_AD.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_TOOFAST);
}

void ViewADcontinuous::OnOverrunError_ADC()
{
	StopAcquisition();
	UpdateStartStop(m_Acq32_AD.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_OVERRUN);
}

void ViewADcontinuous::OnOverrunError_DAC()
{
	m_Acq32_DA.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_OVERRUN);
}

void ViewADcontinuous::OnQueueDone_DAC()
{
	m_Acq32_DA.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TOOFAST);
}

void ViewADcontinuous::OnTriggerError_DAC()
{
	m_Acq32_DA.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void ViewADcontinuous::OnBufferDone_ADC()
{
	short* pDTbuf = m_Acq32_AD.OnBufferDone();
	if (pDTbuf == nullptr)
		return;

	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();
	ADC_Transfer(pDTbuf, pWFormat);
	ADC_TransferToFile(pWFormat);
	ADC_TransferToChart(pWFormat);
}

void ViewADcontinuous::ADC_Transfer(short* pDTbuf0, const CWaveFormat * pWFormat)
{
	short* pRawDataBuf = m_inputDataFile.GetpRawDataBUF();

	m_channel_sweep_start = m_channel_sweep_end + 1;
	if (m_channel_sweep_start >= m_chsweeplength)
		m_channel_sweep_start = 0;
	m_channel_sweep_end = m_channel_sweep_start + m_Acq32_AD.Getchbuflen() - 1;
	m_chsweepRefresh = m_channel_sweep_end - m_channel_sweep_start + 1;
	pRawDataBuf += (m_channel_sweep_start * pWFormat->scan_count);

	// if offset binary (unsigned words), transform data into signed integers (two's complement)
	if ((m_pOptions_AD->waveFormat).binzero != NULL)
	{
		const WORD binzero = WORD((m_pOptions_AD->waveFormat).binzero);
		auto pDTbuf = reinterpret_cast<WORD*>(pDTbuf0);
		for (int j = 0; j < m_Acq32_AD.Getbuflen(); j++, pDTbuf++)
			*pDTbuf -= binzero;
	}

	// no undersampling.. copy DTbuffer into data file buffer
	if (m_pOptions_AD->iundersample <= 1)
	{
		memcpy(pRawDataBuf, pDTbuf0, m_Acq32_AD.Getbuflen() * sizeof(short));
	}
	// undersampling (assume that buffer length is a multiple of iundersample) and copy into data file buffer
	else
	{
		short* pdataBuf2 = pRawDataBuf;
		short* pDTbuf = pDTbuf0;
		const int undersample_factor = m_pOptions_AD->iundersample;
		m_chsweepRefresh = m_chsweepRefresh / undersample_factor;
		// loop and compute average between consecutive points
		for (int j = 0; j < pWFormat->scan_count; j++, pdataBuf2++, pDTbuf++)
		{
			short* pSource = pDTbuf;
			short* pDest = pdataBuf2;
			for (int i = 0; i < m_Acq32_AD.Getchbuflen(); i += undersample_factor)
			{
				long SUM = 0;
				for (int k = 0; k < undersample_factor; k++)
				{
					SUM += *pSource;
					pSource += pWFormat->scan_count;
				}
				*pDest = static_cast<short>(SUM / undersample_factor);
				pDest += pWFormat->scan_count;
			}
		}
	}
	// update byte length of buffer
	m_bytesweepRefresh = m_chsweepRefresh * int(sizeof(short)) * int(pWFormat->scan_count);
}

void ViewADcontinuous::ADC_TransferToChart(const CWaveFormat * pWFormat)
{
	if (pWFormat->bOnlineDisplay)
	{
		short* pdataBuf = m_inputDataFile.GetpRawDataBUF();
		m_chartDataAD.ADdisplayBuffer(pdataBuf, m_chsweepRefresh);
	}
	const float duration = static_cast<float>(pWFormat->sample_count) / m_fclockrate;
	CString cs;
	cs.Format(_T("%.3lf"), double(duration));
	SetDlgItemText(IDC_STATIC1, cs);
}

void ViewADcontinuous::ADC_TransferToFile(CWaveFormat * pWFormat)
{
	pWFormat->sample_count += m_bytesweepRefresh / 2;
	const float duration = static_cast<float>(pWFormat->sample_count) / m_fclockrate;

	short* pdataBuf = m_inputDataFile.GetpRawDataBUF();
	pdataBuf += (m_channel_sweep_start * pWFormat->scan_count);

	if (pWFormat->bADwritetofile)
	{
		const BOOL flag = m_inputDataFile.AcqDoc_DataAppend(pdataBuf, m_bytesweepRefresh);
		ASSERT(flag);
		// end of acquisition
		if (duration >= m_pOptions_AD->duration_to_acquire)
		{
			StopAcquisition();
			if (m_bhidesubsequent)
			{
				if (!StartAcquisition())
					StopAcquisition();
				else
				{
					if ((m_inputDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
						OnBufferDone_ADC();
				}
				return;
			}
			UpdateStartStop(FALSE);
			return;
		}
	}
	m_Acq32_AD.ReleaseLastBufferToQueue();
}

void ViewADcontinuous::OnBufferDone_DAC()
{
	m_Acq32_DA.OnBufferDone();
}

void ViewADcontinuous::InitializeAmplifiers()
{
	InitCyberAmp(); // TODO init other amplifiers and look at pwaveformat?
}

BOOL ViewADcontinuous::InitCyberAmp() const
{
	CyberAmp cyber_amp;
	BOOL is_CyberAmp_present = FALSE;
	int nchans = (m_pOptions_AD->chanArray).ChanArray_getSize();

	// test if Cyberamp320 selected
	for (int i = 0; i < nchans; i++)
	{
		const CWaveChan* p_wave_channel = (m_pOptions_AD->chanArray).Get_p_channel(i);

		const int a = p_wave_channel->am_csamplifier.Find(_T("CyberAmp"));
		const int b = p_wave_channel->am_csamplifier.Find(_T("Axon Instrument"));
		if (a == 0 || b == 0)
		{
			if (!is_CyberAmp_present)
				is_CyberAmp_present = (cyber_amp.Initialize() == C300_SUCCESS);
			if (!is_CyberAmp_present)
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
	return is_CyberAmp_present;
}

void ViewADcontinuous::OnBnClickedGainbutton()
{
	SetVBarMode(BAR_GAIN);
}

void ViewADcontinuous::OnBnClickedBiasbutton()
{
	SetVBarMode(BAR_BIAS);
}

void ViewADcontinuous::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	if (pScrollBar == nullptr)
	{
		CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	if(m_VBarMode == BAR_GAIN)
		OnGainScroll(nSBCode, nPos);
	else
		OnBiasScroll(nSBCode, nPos);

}

void ViewADcontinuous::SetVBarMode(short bMode)
{
	m_VBarMode = bMode;
	m_BiasButton.SetState(bMode == BAR_BIAS);
	m_ZoomButton.SetState(bMode == BAR_GAIN);
	if (bMode == BAR_GAIN)
		UpdateGainScroll();
	else 
		UpdateBiasScroll();
}

void ViewADcontinuous::OnGainScroll(UINT nSBCode, UINT nPos)
{
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(0);
	int yextent = pChan->GetYextent();
	const int span = pChan->GetDataBinSpan();

	switch (nSBCode)
	{
		case SB_LEFT: yextent = YEXTENT_MIN;
			break;
		case SB_LINELEFT: yextent -= int(float(span) / 100.f) + 1;
			break;
		case SB_LINERIGHT: yextent += int(float(span) / 100.f) + 1;
			break;
		case SB_PAGELEFT: yextent -= int(float(span) / 10.f) + 1;
			break;
		case SB_PAGERIGHT: yextent += int(float(span) / 10.f) + 1;
			break;
		case SB_RIGHT: yextent = YEXTENT_MAX;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK: yextent = MulDiv(int(nPos - 50), pChan->GetDataBinSpan(), 100);
			break;
		default: break;
	}

	if (yextent > 0) 
	{
		const CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);
		const int ichanlast = pWFormat->scan_count - 1;
		for (int channel = 0; channel <= ichanlast; channel++)
		{
			CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(channel);
			pChan->SetYextent(yextent);
		}
		m_chartDataAD.Invalidate();
		m_pOptions_AD->izoomCursel = yextent;
	}

	UpdateGainScroll();
	UpdateData(false);
}

void ViewADcontinuous::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(0);
	int yzero = pChan->GetYzero();
	const int span = pChan->GetDataBinSpan() / 2;
	const int initial = yzero;
	switch (nSBCode)
	{
		case SB_LEFT: 
			yzero = YZERO_MIN;
			break;
		case SB_LINELEFT: 
			yzero -= int(float(span) / 100.f) + 1;
			break;
		case SB_LINERIGHT: 
			yzero += int(float(span) / 100.f) + 1;
			break;
		case SB_PAGELEFT:
			yzero -= int(float(span) / 10.f) + 1;
			break;
		case SB_PAGERIGHT:
			yzero += int(float(span) / 10.f) + 1;
			break;
		case SB_RIGHT: 
			yzero = YZERO_MAX;
			break;
		case SB_THUMBPOSITION: 		
		case SB_THUMBTRACK:
			yzero = static_cast<int>(nPos - 50) * (YZERO_SPAN / 100);
			break;
		default: // NOP: set position only
			break;
	}
	
	const CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);
	constexpr int ichanfirst = 0;
	const int ichanlast = pWFormat->scan_count - 1;
	for (int i = ichanfirst; i <= ichanlast; i++)
	{
		CChanlistItem* pChani = m_chartDataAD.GetChanlistItem(i);
		pChani->SetYzero(yzero);
	}
	m_chartDataAD.Invalidate();

	UpdateBiasScroll();
	UpdateData(false);
}

void ViewADcontinuous::UpdateBiasScroll()
{
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(0);
	m_chartDataAD.UpdateYRuler();
	const int iPos = MulDiv(pChan->GetYzero(), 100, pChan->GetDataBinSpan())+50;
	m_scrolly.SetScrollPos(iPos, TRUE);
}

void ViewADcontinuous::UpdateGainScroll()
{
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(0);
	m_chartDataAD.UpdateYRuler();
	const int iPos = MulDiv(pChan->GetYextent(), 100, pChan->GetDataBinSpan()) + 50;
	m_scrolly.SetScrollPos(iPos, TRUE);
}

void ViewADcontinuous::OnCbnSelchangeCombostartoutput()
{
	m_bStartOutPutMode = m_ComboStartOutput.GetCurSel();
	m_pOptions_DA->bAllowDA = m_bStartOutPutMode;
	m_Button_StartStop_DA.EnableWindow(m_bStartOutPutMode != 0);
}

void ViewADcontinuous::SetCombostartoutput(int option)
{
	m_ComboStartOutput.SetCurSel(option);
	option = m_ComboStartOutput.GetCurSel();
	m_bStartOutPutMode = option;
	m_pOptions_DA->bAllowDA = option;
	m_Button_StartStop_DA.EnableWindow(m_bStartOutPutMode != 0);
}

void ViewADcontinuous::OnBnClickedDaparameters2()
{
	DlgDAChannels dlg;
	const auto isize = m_pOptions_DA->outputparms_array.GetSize();
	if (isize < 10)
		m_pOptions_DA->outputparms_array.SetSize(10);
	dlg.outputparms_array.SetSize(10);
	for (auto i = 0; i < 10; i++)
		dlg.outputparms_array[i] = m_pOptions_DA->outputparms_array[i];
	const CWaveFormat* wave_format = &(m_pOptions_AD->waveFormat);
	dlg.m_samplingRate = wave_format->sampling_rate_per_channel;

	if (IDOK == dlg.DoModal())
	{
		for (int i = 0; i < 10; i++)
			m_pOptions_DA->outputparms_array[i] = dlg.outputparms_array[i];
		m_Acq32_DA.SetChannelList();
		m_Button_StartStop_DA.EnableWindow(m_Acq32_DA.GetDigitalChannel() > 0);
	}
}

void ViewADcontinuous::OnBnClickedWriteToDisk()
{
	m_bADwritetofile = TRUE;
	m_pOptions_AD->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void ViewADcontinuous::OnBnClickedOscilloscope()
{
	m_bADwritetofile = FALSE;
	m_pOptions_AD->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void ViewADcontinuous::UpdateRadioButtons()
{
	if (m_bADwritetofile)
		m_Button_WriteToDisk.SetCheck(BST_CHECKED);
	else
		m_Button_Oscilloscope.SetCheck(BST_CHECKED);
	UpdateData(TRUE);
}

void ViewADcontinuous::OnBnClickedCardfeatures()
{
	DlgDataTranslationBoard dlg;
	dlg.m_pAnalogIN = &m_Acq32_AD;
	dlg.m_pAnalogOUT = &m_Acq32_DA;
	dlg.DoModal();
}

void ViewADcontinuous::OnBnClickedStartstop2()
{
	CString cs;
	if (m_Acq32_DA.IsInProgress())
	{
		cs = _T("Start");
		StopOutput();
	}
	else
	{
		cs = _T("Stop");
		StartOutput();
	}
	m_Button_StartStop_DA.SetWindowTextW(cs);
}

BOOL ViewADcontinuous::StartOutput()
{
	if (!m_Acq32_DA.InitSubSystem(m_pOptions_AD))
		return FALSE;
	m_Acq32_DA.DeclareAndFillBuffers(m_pOptions_AD);
	m_Acq32_AD.DeclareBuffers(m_pOptions_AD);
	m_Acq32_DA.ConfigAndStart();
	return TRUE;
}

void ViewADcontinuous::StopOutput()
{
	m_Acq32_DA.StopAndLiberateBuffers();
}

void ViewADcontinuous::InitAcquisitionInputFile()
{
	const CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);

	m_chsweeplength = static_cast<long>(m_sweepduration * pWFormat->sampling_rate_per_channel / static_cast<float>(m_pOptions_AD->iundersample));
	// AD system is changed:  update AD buffers & change encoding: it is changed on-the-fly in the transfer loop
	m_inputDataFile.GetpWavechanArray()->Copy(&m_pOptions_AD->chanArray);
	m_inputDataFile.GetpWaveFormat()->Copy( &m_pOptions_AD->waveFormat);

	// set sweep length to the nb of data buffers
	m_inputDataFile.GetpWaveFormat()->sample_count = m_chsweeplength * static_cast<long>(pWFormat->scan_count);
	m_inputDataFile.AdjustBUF(m_chsweeplength);
}

void ViewADcontinuous::OnBnClickedUnzoom()
{
	const CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);
	const int iextent = pWFormat->binspan;

	for (int i = 0; i < pWFormat->scan_count; i++)
	{
		constexpr int ioffset = 0;
		CChanlistItem* pD = m_chartDataAD.GetChanlistItem(i);
		pD->SetYzero(ioffset);
		pD->SetYextent(iextent);
	}

	SetVBarMode(BAR_GAIN);
}
