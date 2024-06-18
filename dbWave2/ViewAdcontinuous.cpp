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

IMPLEMENT_DYNCREATE(ViewADcontinuous, dbTableView)

ViewADcontinuous::ViewADcontinuous()
	: dbTableView(IDD)
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
	DDX_Control(pDX, IDC_COMBOBOARD, m_Combo_ADcard);
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

	if (m_b_found_dt_open_layer_dll_)
	{
		// TODO: save data here 
		if (m_Acq32_AD.GetHDass() != NULL)
			m_Acq32_AD.DeleteBuffers();

		if (m_Acq32_DA.GetHDass() != NULL)
			m_Acq32_DA.DeleteBuffers();
	}

	CFormView::OnDestroy();
	delete m_p_background_brush_;
}

HBRUSH ViewADcontinuous::OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor)
{
	HBRUSH hbr;
	switch (nCtlColor)
	{
	case CTLCOLOR_EDIT:
	case CTLCOLOR_MSGBOX:
		pDC->SetBkColor(m_background_color_);
		hbr = static_cast<HBRUSH>(m_p_background_brush_->GetSafeHandle());
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
	VERIFY(m_Combo_StartOutput.SubclassDlgItem(IDC_COMBOSTARTOUTPUT, this));

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
	m_BiasButton.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hBias)));
	m_ZoomButton.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hZoom)));
	m_UnZoomButton.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), reinterpret_cast<LPARAM>(static_cast<HANDLE>(m_hUnZoom)));

	const BOOL b32BitIcons = afxGlobalData.m_nBitsPerPixel >= 16;
	m_btnStartStop_AD.SetImage(b32BitIcons ? IDB_CHECK32 : IDB_CHECK);
	m_btnStartStop_AD.SetCheckedImage(b32BitIcons ? IDB_CHECKNO32 : IDB_CHECKNO);
	CMFCButton::EnableWindowsTheming(false);
	m_btnStartStop_AD.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;

	// scrollbar
	VERIFY(m_scroll_y_.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scroll_y_.SetScrollRange(0, 100);
}

void ViewADcontinuous::get_acquisition_parameters_from_data_file() 
{
	const auto pdbDoc = GetDocument();
	const auto pDat = pdbDoc->open_current_data_file();
	if (pDat != nullptr)
	{
		pDat->read_data_infos();
		options_input_data_->waveFormat.copy(pDat->get_waveformat());
		options_input_data_->chanArray.chan_array_set_size(options_input_data_->waveFormat.scan_count);
		options_input_data_->chanArray.Copy(pDat->get_wavechan_array());
		options_input_data_->waveFormat.bADwritetofile = m_bADwritetofile;
	}
}

void ViewADcontinuous::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	AttachControls();

	const auto pApp = static_cast<CdbWaveApp*>(AfxGetApp());
	options_input_data_ = &(pApp->options_acq_data);
	options_output_data_ = &(pApp->options_output_data);

	m_b_found_dt_open_layer_dll_ = FALSE;
	m_bADwritetofile = options_input_data_->waveFormat.bADwritetofile;
	m_bStartOutPutMode = options_output_data_->b_allow_output_data;
	m_Combo_StartOutput.SetCurSel(m_bStartOutPutMode);

	// if current document, load parameters from current document into the local set of parameters
	get_acquisition_parameters_from_data_file();

	// create data file and copy data acquisition parameters into it
	m_inputDataFile.OnNewDocument(); 
	m_inputDataFile.get_waveformat()->copy( &options_input_data_->waveFormat);
	options_input_data_->chanArray.chan_array_set_size(options_input_data_->waveFormat.scan_count);
	m_inputDataFile.get_wavechan_array()->Copy(&options_input_data_->chanArray);
	m_chartDataAD.AttachDataFile(&m_inputDataFile);

	pApp->m_ad_card_found = FindDTOpenLayersBoards();
	if (pApp->m_ad_card_found)
	{
		InitOutput_AD();
		InitializeAmplifiers(); 
		m_Acq32_DA.InitSubSystem(options_input_data_);
		m_Acq32_DA.ClearAllOutputs();
	}
	else
	{
		m_btnStartStop_AD.ShowWindow(SW_HIDE);
		m_Button_SamplingMode.ShowWindow(SW_HIDE);
		m_Button_OutputChannels.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DAGROUP)->ShowWindow(SW_HIDE);
		m_Combo_StartOutput.ShowWindow(SW_HIDE);
		m_Button_StartStop_DA.ShowWindow(SW_HIDE);
	}

	UpdateGainScroll();
	UpdateRadioButtons();
	GetParent()->PostMessage(WM_MYMESSAGE, NULL, MAKELPARAM(m_cursorstate, HINT_SETMOUSECURSOR));
}

void ViewADcontinuous::OnCbnSelchangeComboboard()
{
	const int item_selected = m_Combo_ADcard.GetCurSel();
	CString card_name;
	m_Combo_ADcard.GetLBText(item_selected, card_name);
	SelectDTOpenLayersBoard(card_name);
}

BOOL ViewADcontinuous::FindDTOpenLayersBoards()
{
	m_Combo_ADcard.ResetContent();

	// load board name - skip dialog if only one is present
	const short uiNumBoards = m_Acq32_AD.GetNumBoards();
	if (uiNumBoards == 0)
	{
		m_Combo_ADcard.AddString(_T("No Board"));
		m_Combo_ADcard.SetCurSel(0);
		return FALSE;
	}

	for (short i = 0; i < uiNumBoards; i++)
		m_Combo_ADcard.AddString(m_Acq32_AD.GetBoardList(i));

	short isel = 0;
	// if name already defined, check if board present
	if (!(options_input_data_->waveFormat).csADcardName.IsEmpty())
		isel = static_cast<short>(m_Combo_ADcard.FindString(-1, (options_input_data_->waveFormat).csADcardName));
	if (isel < 0)
		isel = 0;

	m_Combo_ADcard.SetCurSel(isel);
	m_boardName = m_Acq32_AD.GetBoardList(isel);
	SelectDTOpenLayersBoard(m_boardName);
	return TRUE;
}

BOOL ViewADcontinuous::SelectDTOpenLayersBoard(const CString& card_name)
{
	// get infos
	m_b_found_dt_open_layer_dll_ = TRUE;
	(options_input_data_->waveFormat).csADcardName = card_name;

	// connect A/D subsystem and display/hide buttons
	m_bStartOutPutMode = 0;
	const BOOL flag_AD = m_Acq32_AD.OpenSubSystem(card_name);
	const BOOL flag_DA = m_Acq32_DA.OpenSubSystem(card_name);
	if (flag_DA)
		m_bStartOutPutMode = 0;
	m_bSimultaneousStart = m_start_da_simultaneously_ && m_Acq32_AD.IsSimultaneousStart();

	// display additional interface elements
	int show = (flag_AD ? SW_SHOW : SW_HIDE);
	m_Button_SamplingMode.ShowWindow(show);
	m_Button_OutputChannels.ShowWindow(show);
	m_Combo_StartOutput.ShowWindow(show);

	show = (flag_DA ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DAPARAMETERS2)->ShowWindow(show);
	GetDlgItem(IDC_DAGROUP)->ShowWindow(show);
	m_Combo_StartOutput.ShowWindow(show);
	m_Button_StartStop_DA.ShowWindow(show);
	if (show == SW_SHOW)
		SetCombostartoutput(options_output_data_->b_allow_output_data);

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
	m_chartDataAD.stop_display();
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
	const CWaveFormat* pWFormat = m_inputDataFile.get_waveformat();

	// if burst data acquisition mode ------------------------------------
	if (m_b_hide_subsequent_)
	{
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
			m_csNameArray.Add(m_szFileName);
		else
			m_inputDataFile.acq_delete_file();
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
			m_inputDataFile.acq_delete_file();
		}
		else
		{
			// -----------------------------------------------------
			// if current document name is the same, it means something happened and we have erased a previously existing file
			// if so, skip
			// otherwise add data file name to the database
			auto* pdb_doc = GetDocument();
			if (m_szFileName.CompareNoCase(pdb_doc->db_get_current_dat_file_name(FALSE)) != 0)
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
	auto* pdb_doc = GetDocument();
	AcqDataDoc* pDocDat = pdb_doc->open_current_data_file();
	if (pDocDat == nullptr)
	{
		ATLTRACE2(_T("error reading current document "));
		return;
	}
	pDocDat->read_data_infos();
	const long length_doc_channel = pDocDat->get_doc_channel_length();
	m_chartDataAD.AttachDataFile(pDocDat);
	m_chartDataAD.ResizeChannels(m_chartDataAD.GetRectWidth(), length_doc_channel);
	m_chartDataAD.GetDataFromDoc(0, length_doc_channel);
}

void ViewADcontinuous::TransferFilesToDatabase()
{
	const auto pdbDoc = GetDocument();
	pdbDoc->import_file_list(m_csNameArray); // add file name(s) to the list of records in the database
	m_csNameArray.RemoveAll(); // clear file names

	CdbTableMain* pSet = &(pdbDoc->m_pDB->m_mainTableSet);
	pSet->BuildAndSortIDArrays();
	pSet->RefreshQuery();
	pdbDoc->db_set_current_record_position(pdbDoc->m_pDB->get_n_records() - 1);
	pdbDoc->update_all_views_db_wave(nullptr, HINT_DOCMOVERECORD, nullptr);
}

BOOL ViewADcontinuous::InitOutput_DA()
{
	m_DA_present = m_Acq32_DA.InitSubSystem(options_input_data_);
	if (m_bStartOutPutMode == 0 && m_DA_present)
		m_Acq32_DA.DeclareAndFillBuffers(options_input_data_);
	return m_DA_present;
}

BOOL ViewADcontinuous::InitOutput_AD()
{
	m_AD_present = m_Acq32_AD.InitSubSystem(options_input_data_);
	if (m_AD_present)
	{
		m_Acq32_AD.DeclareBuffers(options_input_data_);
		InitAcquisitionInputFile();
		InitAcquisitionDisplay();
	}
	return m_AD_present;
}

void ViewADcontinuous::InitAcquisitionDisplay()
{
	const CWaveFormat* pWFormat = &(options_input_data_->waveFormat);
	m_chartDataAD.AttachDataFile(&m_inputDataFile);
	m_chartDataAD.ResizeChannels(0, m_channel_sweep_length);
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
	if (options_input_data_->izoomCursel != 0)
		i_extent = options_input_data_->izoomCursel;

	for (int i = 0; i < pWFormat->scan_count; i++)
	{
		constexpr int i_offset = 0;
		CChanlistItem* pD = m_chartDataAD.GetChanlistItem(i);
		pD->SetYzero(i_offset);
		pD->SetYextent(i_extent);
		pD->SetColor(static_cast<WORD>(i));
		float doc_volts_per_bin;
		m_inputDataFile.get_volts_per_bin(i, &doc_volts_per_bin);
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
	m_channel_sweep_start_ = 0;
	m_channel_sweep_end_ = -1;
	m_chartDataAD.start_display(m_channel_sweep_length);
	CWaveFormat* pWFormat = m_inputDataFile.get_waveformat();
	pWFormat->sample_count = 0; 
	pWFormat->sampling_rate_per_channel = pWFormat->sampling_rate_per_channel / static_cast<float>(options_input_data_->iundersample);
	m_clock_rate_ = pWFormat->sampling_rate_per_channel * static_cast<float>(pWFormat->scan_count);
	pWFormat->acqtime = CTime::GetCurrentTime();

	// data format
	pWFormat->binspan = (options_input_data_->waveFormat).binspan;
	pWFormat->fullscale_volts = (options_input_data_->waveFormat).fullscale_volts;
	// trick: if OLx_ENC_BINARY, it is changed on the fly within AD_Transfer function 
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
	const auto pmF = static_cast<CMainFrame*>(AfxGetMainWnd());
	if (bActivate)
	{
		pmF->ActivatePropertyPane(FALSE);
		static_cast<CChildFrame*>(pmF->MDIGetActive())->m_cursor_state = 0;
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
	int low_lParam = LOWORD(lParam); // value associated with code

	if (code == HINT_SETMOUSECURSOR) {
		if (low_lParam > CURSOR_ZOOM)
			low_lParam = 0;
		m_cursorstate = m_chartDataAD.set_mouse_cursor_type(low_lParam);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
	}
	else {
		if (low_lParam == 0)
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
			if ((m_inputDataFile.get_waveformat())->trig_mode == OLx_TRG_EXTERN)
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
		if (m_b_hide_subsequent_)
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
		if (options_input_data_->baudiblesound)
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
	if (!m_b_hide_subsequent_)
	{
		DlgADExperiment dlg;
		dlg.m_bFilename = true;
		dlg.options_inputdata_ = options_input_data_;
		dlg.m_pdbDoc = GetDocument();
		dlg.m_bhidesubsequent = m_b_hide_subsequent_;
		if (IDOK != dlg.DoModal())
			return FALSE;
		m_b_hide_subsequent_ = dlg.m_bhidesubsequent;
		file_name = dlg.m_szFileName;
	}

	// hide define experiment dialog
	else
	{
		// build file name
		CString csBufTemp;
		options_input_data_->exptnumber++;
		csBufTemp.Format(_T("%06.6lu"), options_input_data_->exptnumber);
		file_name = options_input_data_->csPathname + options_input_data_->csBasename + csBufTemp + _T(".dat");

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
				options_input_data_->exptnumber++;
				csBufTemp.Format(_T("%06.6lu"), options_input_data_->exptnumber);
				file_name = options_input_data_->csPathname + options_input_data_->csBasename + csBufTemp + _T(".dat");
				flag = CFile::GetStatus(file_name, status);
			}
			const CString cs = _T("Next available file name: ") + file_name;
			iIDresponse = AfxMessageBox(cs, MB_YESNO | MB_ICONWARNING);
		}
	}
	// close current file and open new file to prepare it for adding chunks of data
	m_inputDataFile.acq_close_file();
	if (!m_inputDataFile.acq_create_file(file_name))
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
	dlg.m_pwFormat = &(options_input_data_->waveFormat);
	dlg.m_pchArray = &(options_input_data_->chanArray);
	dlg.m_numchansMAXDI = m_Acq32_AD.GetSSCaps(OLSSC_MAXDICHANS);
	dlg.m_numchansMAXSE = m_Acq32_AD.GetSSCaps(OLSSC_MAXSECHANS);
	dlg.m_bchantype = options_input_data_->bChannelType;
	dlg.m_bchainDialog = TRUE;
	dlg.m_bcommandAmplifier = TRUE;

	const auto p_alligator = new CUSBPxxS1();
	dlg.m_alligator_amplifier = p_alligator;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		options_input_data_->bChannelType = dlg.m_bchantype;
		const boolean is_acquisition_running = m_Acq32_AD.IsInProgress();
		if (dlg.is_AD_changed) 
		{
			if (is_acquisition_running)
				StopAcquisition();
			InitOutput_AD();
		}
		UpdateData(FALSE);
		UpdateGainScroll();
		if (is_acquisition_running && dlg.is_AD_changed)
			StartAcquisition();
	}
	delete p_alligator;

	if (dlg.m_postmessage != NULL)
		ChainDialog(dlg.m_postmessage);
}

void ViewADcontinuous::OnSamplingMode()
{
	DlgADIntervals dlg;
	CWaveFormat* pWFormat = &(options_input_data_->waveFormat);
	dlg.m_p_wave_format = pWFormat;
	dlg.m_rate_minimum = 1.0f;
	dlg.m_rate_maximum = static_cast<float>(m_Acq32_AD.GetMaximumFrequency() / pWFormat->scan_count);
	dlg.m_buffer_W_size_maximum = static_cast<UINT>(65536) * 4;
	dlg.m_under_sample_factor = options_input_data_->iundersample;
	dlg.m_b_audible_sound = options_input_data_->baudiblesound;
	dlg.m_acquisition_duration = options_input_data_->duration_to_acquire;
	dlg.m_sweep_duration = m_sweepduration;
	dlg.m_b_chain_dialog = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		if (m_Acq32_AD.IsInProgress()) {
			StopAcquisition();
			UpdateStartStop(m_Acq32_AD.IsInProgress());
		}

		options_input_data_->iundersample = static_cast<int>(dlg.m_under_sample_factor);
		options_input_data_->baudiblesound = dlg.m_b_audible_sound;
		options_input_data_->duration_to_acquire = dlg.m_acquisition_duration;
		m_sweepduration = dlg.m_sweep_duration;
		options_input_data_->sweepduration = m_sweepduration;
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
	short* p_buffer_done = m_Acq32_AD.OnBufferDone();
	if (p_buffer_done == nullptr)
		return;

	CWaveFormat* wave_format = m_inputDataFile.get_waveformat();
	short* pRawDataBuf = ADC_Transfer(p_buffer_done, wave_format);
	ADC_TransferToFile(wave_format);
	ADC_TransferToChart(pRawDataBuf, wave_format);
}

short* ViewADcontinuous::ADC_Transfer(short* source_data, const CWaveFormat * pWFormat)
{
	short* pRawDataBuf = m_inputDataFile.get_raw_data_buffer();

	m_channel_sweep_start_ = m_channel_sweep_end_ + 1;
	if (m_channel_sweep_start_ >= m_channel_sweep_length)
		m_channel_sweep_start_ = 0;
	m_channel_sweep_end_ = m_channel_sweep_start_ + m_Acq32_AD.Getchbuflen() - 1;
	m_channel_sweep_refresh_ = m_channel_sweep_end_ - m_channel_sweep_start_ + 1;
	pRawDataBuf += (m_channel_sweep_start_ * pWFormat->scan_count);

	// if offset binary (unsigned words), transform data into signed integers (two's complement)
	if ((options_input_data_->waveFormat).binzero != NULL)
	{
		const auto bin_zero_value = static_cast<short>(options_input_data_->waveFormat.binzero);
		short* p_data_acquisition_value = source_data;
		for (int j = 0; j < m_Acq32_AD.Getbuflen(); j++, p_data_acquisition_value++)
			*p_data_acquisition_value -= bin_zero_value;
	}

	if (options_input_data_->iundersample <= 1)
		memcpy(pRawDataBuf, source_data, m_Acq32_AD.Getbuflen() * sizeof(short));
	else
		under_sample_buffer(pRawDataBuf, source_data, pWFormat, options_input_data_->iundersample);

	m_byte_sweep_refresh_ = m_channel_sweep_refresh_ * static_cast<int>(sizeof(short)) * static_cast<int>(pWFormat->scan_count);

	return pRawDataBuf;
}

void ViewADcontinuous::under_sample_buffer(short* pRawDataBuf, short* pDTbuf0, const CWaveFormat* pWFormat, const int under_sample_factor)
{
	short* pdataBuf2 = pRawDataBuf;
	short* pDTbuf = pDTbuf0;
	m_channel_sweep_refresh_ = m_channel_sweep_refresh_ / under_sample_factor;
	// loop and compute average between consecutive points
	for (int j = 0; j < pWFormat->scan_count; j++, pdataBuf2++, pDTbuf++)
	{
		short* pSource = pDTbuf;
		short* pDest = pdataBuf2;
		for (int i = 0; i < m_Acq32_AD.Getchbuflen(); i += under_sample_factor)
		{
			long SUM = 0;
			for (int k = 0; k < under_sample_factor; k++)
			{
				SUM += *pSource;
				pSource += pWFormat->scan_count;
			}
			*pDest = static_cast<short>(SUM / under_sample_factor);
			pDest += pWFormat->scan_count;
		}
	}
}

void ViewADcontinuous::ADC_TransferToChart(short* pdataBuf, const CWaveFormat * pWFormat)
{
	if (pWFormat->bOnlineDisplay)
	{
		//short* pdataBuf = m_inputDataFile.GetpRawDataBUF();
		m_chartDataAD.display_buffer(pdataBuf, m_channel_sweep_refresh_);
	}
	const double duration = static_cast<double>(pWFormat->sample_count) / static_cast<double>(m_clock_rate_);
	CString cs;
	cs.Format(_T("%.3lf"), duration);
	SetDlgItemText(IDC_STATIC1, cs);
}

void ViewADcontinuous::ADC_TransferToFile(CWaveFormat * pWFormat)
{
	pWFormat->sample_count += m_byte_sweep_refresh_ / 2;
	const float duration = static_cast<float>(pWFormat->sample_count) / m_clock_rate_;

	short* pdataBuf = m_inputDataFile.get_raw_data_buffer();
	pdataBuf += (m_channel_sweep_start_ * pWFormat->scan_count);

	if (pWFormat->bADwritetofile)
	{
		const BOOL flag = m_inputDataFile.AcqDoc_DataAppend(pdataBuf, m_byte_sweep_refresh_);
		ASSERT(flag);
		// end of acquisition
		if (duration >= options_input_data_->duration_to_acquire)
		{
			StopAcquisition();
			if (m_b_hide_subsequent_)
			{
				if (!StartAcquisition())
					StopAcquisition();
				else if ((m_inputDataFile.get_waveformat())->trig_mode == OLx_TRG_EXTERN)
					OnBufferDone_ADC();
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
	CyberAmp cyberAmp;
	BOOL cyberAmp_is_present = FALSE;
	const int n_ad_channels = (options_input_data_->chanArray).chan_array_get_size();

	// test if Cyberamp320 selected
	for (int i = 0; i < n_ad_channels; i++)
	{
		const CWaveChan* p_wave_channel = (options_input_data_->chanArray).get_p_channel(i);

		const int a = p_wave_channel->am_csamplifier.Find(_T("CyberAmp"));
		const int b = p_wave_channel->am_csamplifier.Find(_T("Axon Instrument"));
		if (a == 0 || b == 0)
		{
			if (!cyberAmp_is_present)
				cyberAmp_is_present = (cyberAmp.Initialize() == C300_SUCCESS);
			if (!cyberAmp_is_present)
			{
				AfxMessageBox(_T("CyberAmp not found"), MB_OK);
				continue;
			}

			// chan, gain, filter + low-pass, notch	
			cyberAmp.SetHPFilter(p_wave_channel->am_amplifierchan, C300_POSINPUT, p_wave_channel->am_csInputpos);
			cyberAmp.SetmVOffset(p_wave_channel->am_amplifierchan, p_wave_channel->am_offset);

			cyberAmp.SetNotchFilter(p_wave_channel->am_amplifierchan, p_wave_channel->am_notchfilt);
			cyberAmp.SetGain(
				p_wave_channel->am_amplifierchan,
				static_cast<int>(p_wave_channel->am_gaintotal / (static_cast<double>(p_wave_channel->am_gainheadstage) * static_cast<double>(p_wave_channel->
					am_gainAD))));
			cyberAmp.SetLPFilter(p_wave_channel->am_amplifierchan, static_cast<int>(p_wave_channel->am_lowpass));
			cyberAmp.C300_FlushCommandsAndAwaitResponse();
		}
	}
	return cyberAmp_is_present;
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
	int y_extent = pChan->GetYextent();
	const int span = pChan->GetDataBinSpan();

	switch (nSBCode)
	{
		case SB_LEFT: y_extent = YEXTENT_MIN;
			break;
		case SB_LINELEFT: y_extent -= static_cast<int>(static_cast<float>(span) / 100.f) + 1;
			break;
		case SB_LINERIGHT: y_extent += static_cast<int>(static_cast<float>(span) / 100.f) + 1;
			break;
		case SB_PAGELEFT: y_extent -= static_cast<int>(static_cast<float>(span) / 10.f) + 1;
			break;
		case SB_PAGERIGHT: y_extent += static_cast<int>(static_cast<float>(span) / 10.f) + 1;
			break;
		case SB_RIGHT: y_extent = YEXTENT_MAX;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK: y_extent = MulDiv(static_cast<int>(nPos - 50), pChan->GetDataBinSpan(), 100);
			break;
		default: break;
	}

	if (y_extent > 0) 
	{
		const CWaveFormat* pWFormat = &(options_input_data_->waveFormat);
		const int last_channel = pWFormat->scan_count - 1;
		for (int channel = 0; channel <= last_channel; channel++)
		{
			CChanlistItem* ppChan = m_chartDataAD.GetChanlistItem(channel);
			ppChan->SetYextent(y_extent);
		}
		m_chartDataAD.Invalidate();
		options_input_data_->izoomCursel = y_extent;
	}

	UpdateGainScroll();
	UpdateData(false);
}

void ViewADcontinuous::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(0);
	int y_zero = pChan->GetYzero();
	const int span = pChan->GetDataBinSpan() / 2;
	const int initial = y_zero;
	switch (nSBCode)
	{
		case SB_LEFT: 
			y_zero = YZERO_MIN;
			break;
		case SB_LINELEFT: 
			y_zero -= static_cast<int>(static_cast<float>(span) / 100.f) + 1;
			break;
		case SB_LINERIGHT: 
			y_zero += static_cast<int>(static_cast<float>(span) / 100.f) + 1;
			break;
		case SB_PAGELEFT:
			y_zero -= static_cast<int>(static_cast<float>(span) / 10.f) + 1;
			break;
		case SB_PAGERIGHT:
			y_zero += static_cast<int>(static_cast<float>(span) / 10.f) + 1;
			break;
		case SB_RIGHT: 
			y_zero = YZERO_MAX;
			break;
		case SB_THUMBPOSITION: 		
		case SB_THUMBTRACK:
			y_zero = static_cast<int>(nPos - 50) * (YZERO_SPAN / 100);
			break;
		default: // NOP: set position only
			break;
	}
	
	const CWaveFormat* pWFormat = &(options_input_data_->waveFormat);
	constexpr int first_channel = 0;
	const int last_channel = pWFormat->scan_count - 1;
	for (int i = first_channel; i <= last_channel; i++)
	{
		CChanlistItem* pChani = m_chartDataAD.GetChanlistItem(i);
		pChani->SetYzero(y_zero);
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
	m_scroll_y_.SetScrollPos(iPos, TRUE);
}

void ViewADcontinuous::UpdateGainScroll()
{
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(0);
	m_chartDataAD.UpdateYRuler();
	const int iPos = MulDiv(pChan->GetYextent(), 100, pChan->GetDataBinSpan()) + 50;
	m_scroll_y_.SetScrollPos(iPos, TRUE);
}

void ViewADcontinuous::OnCbnSelchangeCombostartoutput()
{
	m_bStartOutPutMode = m_Combo_StartOutput.GetCurSel();
	options_output_data_->b_allow_output_data = m_bStartOutPutMode;
	m_Button_StartStop_DA.EnableWindow(m_bStartOutPutMode != 0);
}

void ViewADcontinuous::SetCombostartoutput(int option)
{
	m_Combo_StartOutput.SetCurSel(option);
	option = m_Combo_StartOutput.GetCurSel();
	m_bStartOutPutMode = option;
	options_output_data_->b_allow_output_data = option;
	m_Button_StartStop_DA.EnableWindow(m_bStartOutPutMode != 0);
}

void ViewADcontinuous::OnBnClickedDaparameters2()
{
	DlgDAChannels dlg;
	const auto i_size = options_output_data_->output_parms_array.GetSize();
	if (i_size < 10)
		options_output_data_->output_parms_array.SetSize(10);
	dlg.outputparms_array.SetSize(10);
	for (auto i = 0; i < 10; i++)
		dlg.outputparms_array[i] = options_output_data_->output_parms_array[i];
	const CWaveFormat* wave_format = &(options_input_data_->waveFormat);
	dlg.m_samplingRate = wave_format->sampling_rate_per_channel;

	if (IDOK == dlg.DoModal())
	{
		for (int i = 0; i < 10; i++)
			options_output_data_->output_parms_array[i] = dlg.outputparms_array[i];
		m_Acq32_DA.SetChannelList();
		m_Button_StartStop_DA.EnableWindow(m_Acq32_DA.GetDigitalChannel() > 0);
	}
}

void ViewADcontinuous::OnBnClickedWriteToDisk()
{
	m_bADwritetofile = TRUE;
	options_input_data_->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.get_waveformat()->bADwritetofile = m_bADwritetofile;
}

void ViewADcontinuous::OnBnClickedOscilloscope()
{
	m_bADwritetofile = FALSE;
	options_input_data_->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.get_waveformat()->bADwritetofile = m_bADwritetofile;
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
	if (!m_Acq32_DA.InitSubSystem(options_input_data_))
		return FALSE;
	m_Acq32_DA.DeclareAndFillBuffers(options_input_data_);
	m_Acq32_AD.DeclareBuffers(options_input_data_);
	m_Acq32_DA.ConfigAndStart();
	return TRUE;
}

void ViewADcontinuous::StopOutput()
{
	m_Acq32_DA.StopAndLiberateBuffers();
}

void ViewADcontinuous::InitAcquisitionInputFile()
{
	const CWaveFormat* pWFormat = &(options_input_data_->waveFormat);

	m_channel_sweep_length = static_cast<long>(m_sweepduration * pWFormat->sampling_rate_per_channel / static_cast<float>(options_input_data_->iundersample));
	// AD system is changed:  update AD buffers & change encoding: it is changed on-the-fly in the transfer loop
	m_inputDataFile.get_wavechan_array()->Copy(&options_input_data_->chanArray);
	m_inputDataFile.get_waveformat()->copy( &options_input_data_->waveFormat);

	// set sweep length to the nb of data buffers
	m_inputDataFile.get_waveformat()->sample_count = m_channel_sweep_length * static_cast<long>(pWFormat->scan_count);
	m_inputDataFile.adjust_buffer(m_channel_sweep_length);
}

void ViewADcontinuous::OnBnClickedUnzoom()
{
	const CWaveFormat* pWFormat = &(options_input_data_->waveFormat);
	const int i_extent = pWFormat->binspan;

	for (int i = 0; i < pWFormat->scan_count; i++)
	{
		constexpr int i_offset = 0;
		CChanlistItem* pD = m_chartDataAD.GetChanlistItem(i);
		pD->SetYzero(i_offset);
		pD->SetYextent(i_extent);
	}

	SetVBarMode(BAR_GAIN);
}
