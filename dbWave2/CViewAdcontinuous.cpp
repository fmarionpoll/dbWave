#include "StdAfx.h"
#include <OLERRORS.H>
#include <Olxdaapi.h>

#include "resource.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "DlgADIntervals.h"
#include "DlgADExperiment.h"
#include "dtacq32.h"
#include "CyberAmp.h"
#include "ChartData.h"
#include "CViewADcontinuous.h"
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

constexpr auto MIN_DURATION = 0.1;
constexpr auto MIN_BUFLEN = 32;

IMPLEMENT_DYNCREATE(CADContView, CFormView)

CADContView::CADContView()
	: CFormView(IDD)
{
	m_bEnableActiveAccessibility = FALSE;
	m_AD_yRulerBar.AttachScopeWnd(&m_chartDataAD, FALSE);
}

CADContView::~CADContView()
= default;

void CADContView::DoDataExchange(CDataExchange * pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANALOGTODIGIT, m_Acq32_AD);
	DDX_Control(pDX, IDC_DIGITTOANALOG, m_Acq32_DA);
	DDX_Control(pDX, IDC_COMBOBOARD, m_ADcardCombo);
	DDX_Control(pDX, IDC_STARTSTOP, m_btnStartStop_AD);
	DDX_CBIndex(pDX, IDC_COMBOSTARTOUTPUT, m_bStartOutPutMode);
	//DDX_Control(pDX, IDC_GAIN_button, m_GainButton);
	//DDX_Control(pDX, IDC_BIAS_button, m_BiasButton);
	//DDX_Control(pDX, IDC_COMBOSTARTOUTPUT, m_ComboStartOutput);
	DDX_Control(pDX, IDC_STARTSTOP2, m_Button_StartStop_DA);
	DDX_Control(pDX, IDC_ADPARAMETERS, m_Button_SamplingMode);
	DDX_Control(pDX, IDC_DAPARAMETERS2, m_Button_OutputChannels);
	DDX_Control(pDX, IDC_WRITETODISK, m_Button_WriteToDisk);
	DDX_Control(pDX, IDC_OSCILLOSCOPE, m_Button_Oscilloscope);
}

BEGIN_MESSAGE_MAP(CADContView, CFormView)
	ON_MESSAGE(WM_MYMESSAGE, &CADContView::OnMyMessage)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &CADContView::OnHardwareAdchannels)
	ON_COMMAND(ID_HARDWARE_ADINTERVALS, &CADContView::OnHardwareAdintervals)
	ON_COMMAND(ID_HARDWARE_DEFINEEXPERIMENT, &CADContView::OnHardwareDefineexperiment)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ADPARAMETERS, &CADContView::OnHardwareAdintervals)
	ON_BN_CLICKED(IDC_ADPARAMETERS2, &CADContView::OnHardwareAdchannels)
	ON_BN_CLICKED(IDC_GAIN_button, &CADContView::OnBnClickedGainbutton)
	ON_BN_CLICKED(IDC_BIAS_button, &CADContView::OnBnClickedBiasbutton)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_DAPARAMETERS2, &CADContView::OnBnClickedDaparameters2)
	ON_CBN_SELCHANGE(IDC_COMBOBOARD, &CADContView::OnCbnSelchangeComboboard)
	ON_BN_CLICKED(IDC_STARTSTOP, &CADContView::OnBnClickedStartstop)
	ON_BN_CLICKED(IDC_WRITETODISK, &CADContView::OnBnClickedWriteToDisk)
	ON_BN_CLICKED(IDC_OSCILLOSCOPE, &CADContView::OnBnClickedOscilloscope)
	ON_BN_CLICKED(IDC_CARDFEATURES, &CADContView::OnBnClickedCardfeatures)
	ON_CBN_SELCHANGE(IDC_COMBOSTARTOUTPUT, &CADContView::OnCbnSelchangeCombostartoutput)
	ON_BN_CLICKED(IDC_STARTSTOP2, &CADContView::OnBnClickedStartstop2)
END_MESSAGE_MAP()

void CADContView::OnDestroy()
{
	if (m_Acq32_AD.IsInProgress())
		StopAcquisition(TRUE);

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

HBRUSH CADContView::OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor)
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

BEGIN_EVENTSINK_MAP(CADContView, CFormView)

	ON_EVENT(CADContView, IDC_ANALOGTODIGIT, 1, CADContView::OnBufferDone_ADC, VTS_NONE)
	ON_EVENT(CADContView, IDC_ANALOGTODIGIT, 2, CADContView::OnQueueDone_ADC, VTS_NONE)
	ON_EVENT(CADContView, IDC_ANALOGTODIGIT, 4, CADContView::OnTriggerError_ADC, VTS_NONE)
	ON_EVENT(CADContView, IDC_ANALOGTODIGIT, 5, CADContView::OnOverrunError_ADC, VTS_NONE)

	ON_EVENT(CADContView, IDC_DIGITTOANALOG, 1, CADContView::OnBufferDone_DAC, VTS_NONE)
	ON_EVENT(CADContView, IDC_DIGITTOANALOG, 5, CADContView::OnOverrunError_DAC, VTS_NONE)
	ON_EVENT(CADContView, IDC_DIGITTOANALOG, 2, CADContView::OnQueueDone_DAC, VTS_NONE)
	ON_EVENT(CADContView, IDC_DIGITTOANALOG, 4, CADContView::OnTriggerError_DAC, VTS_NONE)

END_EVENTSINK_MAP()


void CADContView::OnInitialUpdate()
{
	// attach controls
	VERIFY(m_chartDataAD.SubclassDlgItem(IDC_DISPLAYDATA, this));
	VERIFY(m_AD_yRulerBar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_AD_xRulerBar.SubclassDlgItem(IDC_XSCALE, this));
	VERIFY(m_BiasButton.SubclassDlgItem(IDC_BIAS_button, this));
	VERIFY(m_GainButton.SubclassDlgItem(IDC_GAIN_button, this));
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
	m_stretch.newProp(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_SCROLLY_scrollbar, SZEQ_XREQ, YTEQ_YBEQ);

	// bitmap buttons: load icons & set buttons
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	m_BiasButton.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), LPARAM(static_cast<HANDLE>(m_hBias)));
	m_GainButton.SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), LPARAM(static_cast<HANDLE>(m_hZoom)));

	const BOOL b32BitIcons = afxGlobalData.m_nBitsPerPixel >= 16;
	m_btnStartStop_AD.SetImage(b32BitIcons ? IDB_CHECK32 : IDB_CHECK);
	m_btnStartStop_AD.SetCheckedImage(b32BitIcons ? IDB_CHECKNO32 : IDB_CHECKNO);
	CMFCButton::EnableWindowsTheming(false);
	m_btnStartStop_AD.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;

	// scrollbar
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	const auto pApp = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	m_pOptions_AD = &(pApp->options_acqdata);
	m_pOptions_DA = &(pApp->options_outputdata);
	m_bFoundDTOPenLayerDLL = FALSE;
	m_bADwritetofile = m_pOptions_AD->waveFormat.bADwritetofile;
	m_bStartOutPutMode = m_pOptions_DA->bAllowDA;
	m_ComboStartOutput.SetCurSel(m_bStartOutPutMode);

	// open document and remove database filters
	CdbWaveDoc* pdbDoc = GetDocument();
	m_ptableSet = &pdbDoc->m_pDB->m_mainTableSet;
	m_ptableSet->m_strFilter.Empty();
	m_ptableSet->ClearFilters();
	m_ptableSet->RefreshQuery();
	CFormView::OnInitialUpdate();

	// if current document, load parameters from current document into the local set of parameters
	// if current document does not exist, do nothing
	if (pdbDoc->m_pDB->GetNRecords() > 0)
	{
		pdbDoc->OpenCurrentDataFile(); // read data descriptors from current data file
		AcqDataDoc* pDat = pdbDoc->m_pDat; // get a pointer to the data file itself
		if (pDat != nullptr)
		{
			m_pOptions_AD->waveFormat = *(pDat->GetpWaveFormat()); // read data header
			m_pOptions_AD->chanArray.ChanArray_setSize(m_pOptions_AD->waveFormat.scan_count);
			m_pOptions_AD->chanArray = *pDat->GetpWavechanArray(); // get channel descriptors
			// restore state of "write-to-file" parameter that was just erased
			m_pOptions_AD->waveFormat.bADwritetofile = m_bADwritetofile;
		}
	}

	// create data file and copy data acquisition parameters into it
	m_inputDataFile.OnNewDocument(); // create a file to receive incoming data (A/D)
	*(m_inputDataFile.GetpWaveFormat()) = m_pOptions_AD->waveFormat; // copy data formats into this file
	m_pOptions_AD->chanArray.ChanArray_setSize(m_pOptions_AD->waveFormat.scan_count);
	*(m_inputDataFile.GetpWavechanArray()) = m_pOptions_AD->chanArray;
	m_chartDataAD.AttachDataFile(&m_inputDataFile); // prepare display area

	pApp->m_bADcardFound = FindDTOpenLayersBoards();
	if (pApp->m_bADcardFound)
	{
		InitOutput_AD();
		InitAcquisitionInputFile();
		InitAcquisitionDisplay();
		InitializeAmplifiers(); // control cyberamplifier OR Alligator Or none?

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

	UpdateChanLegends(0);
	UpdateRadioButtons();

	// tell mmdi parent which cursor is active
	GetParent()->PostMessage(WM_MYMESSAGE, NULL, MAKELPARAM(m_cursorstate, HINT_SETMOUSECURSOR));
}

void CADContView::OnCbnSelchangeComboboard()
{
	const int item_selected = m_ADcardCombo.GetCurSel();
	CString card_name;
	m_ADcardCombo.GetLBText(item_selected, card_name);
	SelectDTOpenLayersBoard(card_name);
}

BOOL CADContView::FindDTOpenLayersBoards()
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

BOOL CADContView::SelectDTOpenLayersBoard(const CString cardName)
{
	// get infos
	m_bFoundDTOPenLayerDLL = TRUE;
	(m_pOptions_AD->waveFormat).csADcardName = cardName;

	// connect A/D subsystem and display/hide buttons
	m_bStartOutPutMode = 0;
	const BOOL flag_AD = m_Acq32_AD.OpenSubSystem(cardName);
	const BOOL flag_DA = m_Acq32_DA.OpenSubSystem(cardName);
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

void CADContView::StopAcquisition(BOOL bDisplayErrorMsg)
{
	if (!m_Acq32_AD.IsInProgress())
		return;

	// special treatment if simultaneous list
	if (m_bSimultaneousStart && m_bStartOutPutMode == 0)
	{
		HSSLIST hSSlist;
		CHAR errstr[255];
		ECODE ecode = olDaGetSSList(&hSSlist);
		olDaGetErrorString(ecode, errstr, 255);
		ecode = olDaReleaseSSList(hSSlist);
		olDaGetErrorString(ecode, errstr, 255);
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
		SaveAndCloseFile();
		// update view data	
		const long lsizeDOCchan = m_inputDataFile.GetDOCchanLength();
		m_chartDataAD.AttachDataFile(&m_inputDataFile);
		m_chartDataAD.ResizeChannels(m_chartDataAD.GetRectWidth(), lsizeDOCchan);
		m_chartDataAD.GetDataFromDoc(0, lsizeDOCchan);
	}
}

void CADContView::SaveAndCloseFile()
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
		int ires = IDCANCEL;
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
		{
			DlgConfirmSave dlg;
			dlg.m_csfilename = m_szFileName;
			ires = dlg.DoModal();
		}
		// if no data or user answered no, erase the data
		if (IDOK != ires)
		{
			m_inputDataFile.AcqDeleteFile();
		}
		else
		{
			// -----------------------------------------------------
			// if current document name is the same, it means something happened and we have erased a previously existing file
			// if so, skip
			// otherwise add data file name to the database
			CdbWaveDoc* pdbDoc = GetDocument();
			if (m_szFileName.CompareNoCase(pdbDoc->GetDB_CurrentDatFileName(FALSE)) != 0)
			{
				// add document to database
				m_csNameArray.Add(m_szFileName);
				TransferFilesToDatabase();
				UpdateViewDataFinal();
			}
		}
	}
}

void CADContView::UpdateViewDataFinal()
{
	// update view data	
	CdbWaveDoc* pdbDoc = GetDocument();
	AcqDataDoc* pDocDat = pdbDoc->OpenCurrentDataFile();
	if (pDocDat == nullptr)
	{
		ATLTRACE2(_T("error reading current document "));
		return;
	}
	pDocDat->ReadDataInfos();
	const long lsizeDOCchan = pDocDat->GetDOCchanLength();
	m_chartDataAD.AttachDataFile(pDocDat);
	m_chartDataAD.ResizeChannels(m_chartDataAD.GetRectWidth(), lsizeDOCchan);
	m_chartDataAD.GetDataFromDoc(0, lsizeDOCchan);
}

void CADContView::TransferFilesToDatabase()
{
	CdbWaveDoc* pdbDoc = GetDocument();
	pdbDoc->ImportFileList(m_csNameArray); // add file name(s) to the list of records in the database
	m_csNameArray.RemoveAll(); // clear file names

	CdbMainTable* pSet = &(GetDocument()->m_pDB->m_mainTableSet);
	pSet->BuildAndSortIDArrays();
	pSet->RefreshQuery();
	pdbDoc->SetDB_CurrentRecordPosition(pdbDoc->m_pDB->GetNRecords() - 1);
	pdbDoc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
}

BOOL CADContView::InitOutput_DA()
{
	m_DA_present = m_Acq32_DA.InitSubSystem(m_pOptions_AD);
	if (m_bStartOutPutMode == 0 && m_DA_present)
		m_Acq32_DA.DeclareAndFillBuffers(m_pOptions_AD);
	return m_DA_present;
}

BOOL CADContView::InitOutput_AD()
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

void CADContView::InitAcquisitionDisplay()
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
	int iextent = MulDiv(pWFormat->binspan, 12, 10);
	if (m_pOptions_AD->izoomCursel != 0)
		iextent = m_pOptions_AD->izoomCursel;

	for (int i = 0; i < pWFormat->scan_count; i++)
	{
		constexpr int ioffset = 0;
		CChanlistItem* pD = m_chartDataAD.GetChanlistItem(i);
		pD->SetYzero(ioffset);
		pD->SetYextent(iextent);
		pD->SetColor(WORD(i));
		float doc_voltsperb;
		m_inputDataFile.GetWBVoltsperBin(i, &doc_voltsperb);
		pD->SetDataBinFormat(pWFormat->binzero, pWFormat->binspan);
		pD->SetDataVoltsFormat(doc_voltsperb, pWFormat->fullscale_volts);
	}
	m_chartDataAD.Invalidate();
}


BOOL CADContView::StartAcquisition()
{
	// set display
	if (m_bADwritetofile && !Defineexperiment())
	{
		StopAcquisition(FALSE);
		UpdateStartStop(m_Acq32_AD.IsInProgress());
		return FALSE;
	}

	InitOutput_DA();
	InitOutput_AD();

	// start AD display
	m_chsweep1 = 0;
	m_chsweep2 = -1;
	m_chartDataAD.ADdisplayStart(m_chsweeplength);
	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();
	pWFormat->sample_count = 0; // no samples yet
	pWFormat->chrate = pWFormat->chrate / float(m_pOptions_AD->iundersample);
	m_fclockrate = pWFormat->chrate * float(pWFormat->scan_count);
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
			StopAcquisition(FALSE);
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

ECODE CADContView::StartSimultaneousList()
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
		return ecode;
	}

	// AD system
	m_Acq32_AD.Config();
	ecode = olDaPutDassToSSList(hSSlist, (HDASS)m_Acq32_AD.GetHDass());
	if (ecode != OLNOERROR)
	{
		code_returned = olDaReleaseSSList(hSSlist);
		return ecode;
	}

	// prestart
	ecode = olDaSimultaneousPrestart(hSSlist);
	if (ecode != OLNOERROR)
	{
		olDaGetErrorString(ecode, error_string, 255);
		DisplayolDaErrorMessage(error_string);
	}

	// start simultaneously
	ecode = olDaSimultaneousStart(hSSlist);
	if (ecode != OLNOERROR)
	{
		olDaGetErrorString(ecode, error_string, 255);
		DisplayolDaErrorMessage(error_string);
	}

	m_Acq32_AD.SetInProgress();
	m_Acq32_DA.SetInProgress();
	return ecode;
}

void CADContView::DisplayolDaErrorMessage(const CHAR * errstr) const
{
	CString csError;
	const CStringA cstringa(errstr);
	csError = cstringa;
	AfxMessageBox(csError);
}

#ifdef _DEBUG
void CADContView::AssertValid() const
{
	CFormView::AssertValid();
}

void CADContView::Dump(CDumpContext & dc) const
{
	CFormView::Dump(dc);
}

CdbWaveDoc* CADContView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return dynamic_cast<CdbWaveDoc*>(m_pDocument);
}

#endif //_DEBUG

CDaoRecordset* CADContView::OnGetRecordset()
{
	return m_ptableSet;
}

void CADContView::OnUpdate(CView * pSender, LPARAM lHint, CObject * pHint)
{
	// update sent from within this class
	if (pSender == this)
	{
		ASSERT(GetDocument() != NULL);
		m_chartDataAD.Invalidate(); // display data
	}
}

void CADContView::OnActivateView(BOOL bActivate, CView * pActivateView, CView * pDeactiveView)
{
	const auto pmF = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	// activate view
	if (bActivate)
	{
		pmF->ActivatePropertyPane(FALSE);
		dynamic_cast<CChildFrame*>(pmF->MDIGetActive())->m_cursorstate = 0;
	}
	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CADContView::OnSize(UINT nType, int cx, int cy)
{
	switch (nType)
	{
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
		if (m_Acq32_AD.IsInProgress())
		{
			StopAcquisition(TRUE);
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

LRESULT CADContView::OnMyMessage(WPARAM wParam, LPARAM lParam)
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

void CADContView::OnBnClickedStartstop()
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
			StopAcquisition(FALSE);
		}
	}
	else
	{
		StopAcquisition(TRUE);
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

void CADContView::UpdateStartStop(BOOL bStart)
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

void CADContView::OnHardwareDefineexperiment()
{
	Defineexperiment();
}

BOOL CADContView::Defineexperiment()
{
	CString csfilename;
	m_bFileOpen = FALSE;
	if (!m_bhidesubsequent)
	{
		DlgADExperiment dlg;
		dlg.m_bFilename = true;
		dlg.m_pADC_options = m_pOptions_AD;
		dlg.m_pdbDoc = GetDocument();
		dlg.m_bhidesubsequent = m_bhidesubsequent;
		if (IDOK != dlg.DoModal())
			return FALSE;
		m_bhidesubsequent = dlg.m_bhidesubsequent;
		csfilename = dlg.m_szFileName;
	}

	// hide define experiment dialog
	else
	{
		// build file name
		CString csBufTemp;
		m_pOptions_AD->exptnumber++;
		csBufTemp.Format(_T("%06.6lu"), m_pOptions_AD->exptnumber);
		csfilename = m_pOptions_AD->csPathname + m_pOptions_AD->csBasename + csBufTemp + _T(".dat");

		// check if this file is already present, exit if not...
		CFileStatus status;
		int iIDresponse = IDYES; // default: go on if file not found
		if (CFile::GetStatus(csfilename, status))
			iIDresponse = AfxMessageBox(IDS_FILEOVERWRITE, MB_YESNO | MB_ICONWARNING);
		// no .. find first available number
		if (IDNO == iIDresponse)
		{
			BOOL flag = TRUE;
			while (flag)
			{
				m_pOptions_AD->exptnumber++;
				csBufTemp.Format(_T("%06.6lu"), m_pOptions_AD->exptnumber);
				csfilename = m_pOptions_AD->csPathname + m_pOptions_AD->csBasename + csBufTemp + _T(".dat");
				flag = CFile::GetStatus(csfilename, status);
			}
			const CString cs = _T("The Next available file name is: ") + csfilename;
			iIDresponse = AfxMessageBox(cs, MB_YESNO | MB_ICONWARNING);
		}
	}
	// close current file and open new file to prepare it for adding chunks of data
	m_inputDataFile.AcqCloseFile();
	if (!m_inputDataFile.CreateAcqFile(csfilename))
		return FALSE;
	m_szFileName = csfilename;
	m_inputDataFile.AcqDoc_DataAppendStart();
	m_bFileOpen = TRUE;
	return TRUE;
}

void CADContView::OnHardwareAdchannels()
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
		InitAcquisitionInputFile();
		InitAcquisitionDisplay();
		UpdateData(FALSE);
		UpdateChanLegends(0);
	}
	delete p_alligator;

	if (dlg.m_postmessage != NULL)
		ChainDialog(dlg.m_postmessage);
}

void CADContView::OnHardwareAdintervals()
{
	DlgADIntervals dlg;
	CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);
	dlg.m_pwaveFormat = pWFormat;
	dlg.m_ratemin = 1.0f;
	dlg.m_ratemax = float(m_Acq32_AD.GetMaximumFrequency() / pWFormat->scan_count);
	dlg.m_bufferWsizemax = UINT(65536) * 4;
	dlg.m_undersamplefactor = m_pOptions_AD->iundersample;
	dlg.m_baudiblesound = m_pOptions_AD->baudiblesound;
	dlg.m_sweepduration = m_sweepduration;
	dlg.m_bchainDialog = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		StopAcquisition(TRUE);
		UpdateStartStop(m_Acq32_AD.IsInProgress());

		m_pOptions_AD->iundersample = int(dlg.m_undersamplefactor);
		m_pOptions_AD->baudiblesound = dlg.m_baudiblesound;
		m_sweepduration = dlg.m_sweepduration;
		m_pOptions_AD->sweepduration = m_sweepduration;
		InitOutput_AD();
		InitAcquisitionInputFile();
		InitAcquisitionDisplay();
		UpdateData(FALSE);
	}

	if (dlg.m_postmessage != NULL)
		ChainDialog(dlg.m_postmessage);
}

void CADContView::ChainDialog(WORD iID)
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

void CADContView::OnTriggerError_ADC()
{
	StopAcquisition(TRUE);
	UpdateStartStop(m_Acq32_AD.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void CADContView::OnQueueDone_ADC()
{
	StopAcquisition(TRUE);
	UpdateStartStop(m_Acq32_AD.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_TOOFAST);
}

void CADContView::OnOverrunError_ADC()
{
	StopAcquisition(TRUE);
	UpdateStartStop(m_Acq32_AD.IsInProgress());
	AfxMessageBox(IDS_ACQDATA_OVERRUN);
}

void CADContView::OnOverrunError_DAC()
{
	m_Acq32_DA.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_OVERRUN);
}

void CADContView::OnQueueDone_DAC()
{
	m_Acq32_DA.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TOOFAST);
}

void CADContView::OnTriggerError_DAC()
{
	m_Acq32_DA.StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void CADContView::OnBufferDone_ADC()
{
	short* pDTbuf = m_Acq32_AD.OnBufferDone();
	if (pDTbuf == nullptr)
		return;

	ADC_Transfer(pDTbuf);
	ADC_TransferToFile();
	ADC_TransferToChart();
}

void CADContView::ADC_Transfer(short* pDTbuf0)
{
	// get pointer to file waveFormat
	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();
	short* pdataBuf = m_inputDataFile.GetpRawDataBUF();

	m_chsweep1 = m_chsweep2 + 1;							// update data abcissa on the screen 
	if (m_chsweep1 >= m_chsweeplength)						// if data are out of the screen, wrap
		m_chsweep1 = 0;
	m_chsweep2 = m_chsweep1 + m_Acq32_AD.Getchbuflen() - 1; // abcissa of the last data point
	m_chsweepRefresh = m_chsweep2 - m_chsweep1 + 1;			// number of data points to refresh on the screen
	pdataBuf += (m_chsweep1 * pWFormat->scan_count);

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
		memcpy(pdataBuf, pDTbuf0, m_Acq32_AD.Getbuflen() * sizeof(short));
	}
	// undersampling (assume that buffer length is a multiple of iundersample) and copy into data file buffer
	else
	{
		short* pdataBuf2 = pdataBuf;
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

void CADContView::ADC_TransferToChart()
{
	const CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat(); // pointer to data descriptor

	// then: display options_acqdataataDoc buffer
	if (pWFormat->bOnlineDisplay) // display data if requested
	{
		short* pdataBuf = m_inputDataFile.GetpRawDataBUF();
		m_chartDataAD.ADdisplayBuffer(pdataBuf, m_chsweepRefresh);
	}
	const float duration = static_cast<float>(pWFormat->sample_count) / m_fclockrate;
	CString cs;
	cs.Format(_T("%.3lf"), duration); 
	SetDlgItemText(IDC_STATIC1, cs);
}

void CADContView::ADC_TransferToFile()
{
	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat(); // pointer to data descriptor
	pWFormat->sample_count += m_bytesweepRefresh / 2; // update total sample count
	const float duration = static_cast<float>(pWFormat->sample_count) / m_fclockrate;

	short* pdataBuf = m_inputDataFile.GetpRawDataBUF();
	pdataBuf += (m_chsweep1 * pWFormat->scan_count);

	// first thing to do: save data to file
	if (pWFormat->bADwritetofile) // write buffer to file
	{
		const BOOL flag = m_inputDataFile.AcqDoc_DataAppend(pdataBuf, m_bytesweepRefresh);
		ASSERT(flag);
		// end of acquisition
		if (duration >= pWFormat->duration)
		{
			StopAcquisition(TRUE);
			if (m_bhidesubsequent)
			{
				if (!StartAcquisition())
					StopAcquisition(TRUE); // if bADStart = wrong, then stop AD
				else
				{
					if ((m_inputDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
						OnBufferDone_ADC(); // TODO check what was that: ADC_OnBufferDone();
				}
				return;
			}
			UpdateStartStop(FALSE);
			return;
		}
	}
	else // no file I/O: refresh screen pos
	{
		if (pWFormat->sample_count >= m_chsweeplength * pWFormat->scan_count)
			pWFormat->sample_count = 0;
	}
	m_Acq32_AD.ReleaseLastBufferToQueue(); // tell ADdriver that data buffer is free
}

void CADContView::OnBufferDone_DAC()
{
	m_Acq32_DA.OnBufferDone();
}

void CADContView::InitializeAmplifiers()
{
	InitCyberAmp(); // TODO init others and look at pwaveformat
}

BOOL CADContView::InitCyberAmp()
{
	CCyberAmp m_cyber;
	BOOL bcyberPresent = FALSE;
	int nchans = (m_pOptions_AD->chanArray).ChanArray_getSize();

	// test if Cyberamp320 selected
	for (int i = 0; i < nchans; i++)
	{
		CWaveChan* pchan = (m_pOptions_AD->chanArray).Get_p_channel(i);

		int a = pchan->am_csamplifier.Find(_T("CyberAmp"));
		int b = pchan->am_csamplifier.Find(_T("Axon Instrument"));
		if (a == 0 || b == 0)
		{
			// test if cyberamp present
			if (!bcyberPresent)
				bcyberPresent = (m_cyber.Initialize() == C300_SUCCESS);
			if (!bcyberPresent)
			{
				AfxMessageBox(_T("CyberAmp not found"), MB_OK);
				continue;
			}

			// chan, gain, filter +, lowpass, notch	
			m_cyber.SetHPFilter(pchan->am_amplifierchan, C300_POSINPUT, pchan->am_csInputpos);
			m_cyber.SetmVOffset(pchan->am_amplifierchan, pchan->am_offset);

			m_cyber.SetNotchFilter(pchan->am_amplifierchan, pchan->am_notchfilt);
			m_cyber.SetGain(
				pchan->am_amplifierchan,
				int(pchan->am_gaintotal / (double(pchan->am_gainheadstage) * double(pchan->am_gainAD))));
			m_cyber.SetLPFilter(pchan->am_amplifierchan, int(pchan->am_lowpass));
			int errorcode = m_cyber.C300_FlushCommandsAndAwaitResponse();
		}
	}
	return bcyberPresent;
}

void CADContView::OnBnClickedGainbutton()
{
	m_BiasButton.SetState(0);
	m_GainButton.SetState(1);
	SetVBarMode(BAR_GAIN);
}

void CADContView::OnBnClickedBiasbutton()
{
	// set bias down and set gain up CButton	
	m_BiasButton.SetState(1);
	m_GainButton.SetState(0);
	SetVBarMode(BAR_BIAS);
}

void CADContView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// CDataView scroll: vertical scroll bar
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

void CADContView::SetVBarMode(short bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}

void CADContView::OnGainScroll(UINT nSBCode, UINT nPos)
{
	// assume that all channels are displayed at the same gain & offset
	const int ichan = 0; // TODO see which channel is selected
	int l_size = m_chartDataAD.GetChanlistItem(ichan)->GetYextent();

	// get corresponding data
	switch (nSBCode)
	{
		// .................scroll to the start
	case SB_LEFT: l_size = YEXTENT_MIN;
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
	case SB_RIGHT: l_size = YEXTENT_MAX;
		break;
		// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: l_size = MulDiv(nPos - 50, YEXTENT_MAX, 100);
		break;
		// .................NOP: set position only
	default: break;
	}

	// change y extent
	if (l_size > 0) //&& lSize<=YEXTENT_MAX)
	{
		// assume that all channels are displayed at the same gain & offset
		const CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);
		const int ichanfirst = 0;
		const int ichanlast = pWFormat->scan_count - 1;

		for (int channel = ichanfirst; channel <= ichanlast; channel++)
		{
			CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(channel);
			pChan->SetYextent(l_size);
		}
		m_chartDataAD.Invalidate();
		UpdateChanLegends(0);
		m_pOptions_AD->izoomCursel = l_size;
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
	{
		UpdateGainScroll();
		UpdateChanLegends(0);
		UpdateData(false);
	}
}

void CADContView::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	// assume that all channels are displayed at the same gain & offset
	int ichan = 0; // TODO: see which channel is selected
	CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(ichan);
	int lSize = pChan->GetYzero() - pChan->GetDataBinZero();
	const int yextent = pChan->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT: // scroll to the start
		lSize = YZERO_MIN;
		break;
	case SB_LINELEFT: // scroll one line left
		lSize -= yextent / 100 + 1;
		break;
	case SB_LINERIGHT: // scroll one line right
		lSize += yextent / 100 + 1;
		break;
	case SB_PAGELEFT: // scroll one page left
		lSize -= yextent / 10 + 1;
		break;
	case SB_PAGERIGHT: // scroll one page right
		lSize += yextent / 10 + 1;
		break;
	case SB_RIGHT: // scroll to end right
		lSize = YZERO_MAX;
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos			
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		lSize = static_cast<int>(nPos - 50) * (YZERO_SPAN / 100);
		break;
	default: // NOP: set position only
		break;
	}

	// try to read data with this new size
	if (lSize > YZERO_MIN && lSize < YZERO_MAX)
	{
		const CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat);
		constexpr int ichanfirst = 0;
		const int ichanlast = pWFormat->scan_count - 1;
		for (int ichan = ichanfirst; ichan <= ichanlast; ichan++)
		{
			CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(ichan);
			pChan->SetYzero(lSize + pChan->GetDataBinZero());
		}
		m_chartDataAD.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
	{
		UpdateBiasScroll();
		UpdateChanLegends(0);
		UpdateData(false);
	}
}

void CADContView::UpdateBiasScroll()
{
	// assume that all channels are displayed at the same gain & offset
	const int ichan = 0;	// TODO see which channel is selected
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(ichan);
	const int iPos = (pChan->GetYzero() - pChan->GetDataBinZero())
		* 100 / static_cast<int>(YZERO_SPAN) + 50;
	m_scrolly.SetScrollPos(iPos, TRUE);
}

void CADContView::UpdateGainScroll()
{
	// assume that all channels are displayed at the same gain & offset
	const int ichan = 0;
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(ichan);
	m_scrolly.SetScrollPos(MulDiv(pChan->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
}

void CADContView::UpdateChanLegends(int chan)
{
	const int ichan = 0;
	const CChanlistItem* pChan = m_chartDataAD.GetChanlistItem(ichan);
	int yzero = pChan->GetYzero();
	int yextent = pChan->GetYextent();
	float mVperbin = pChan->GetVoltsperDataBin() * 1000.0f;
	int binzero = 0;
}

void CADContView::OnCbnSelchangeCombostartoutput()
{
	m_bStartOutPutMode = m_ComboStartOutput.GetCurSel();
	m_pOptions_DA->bAllowDA = m_bStartOutPutMode;
	m_Button_StartStop_DA.EnableWindow(m_bStartOutPutMode != 0);
}

void CADContView::SetCombostartoutput(int option)
{
	m_ComboStartOutput.SetCurSel(option);
	option = m_ComboStartOutput.GetCurSel();
	m_bStartOutPutMode = option;
	m_pOptions_DA->bAllowDA = option;
	m_Button_StartStop_DA.EnableWindow(m_bStartOutPutMode != 0);
}

void CADContView::OnBnClickedDaparameters2()
{
	DlgDAChannels dlg;
	const auto isize = m_pOptions_DA->outputparms_array.GetSize();
	if (isize < 10)
		m_pOptions_DA->outputparms_array.SetSize(10);
	dlg.outputparms_array.SetSize(10);
	for (auto i = 0; i < 10; i++)
		dlg.outputparms_array[i] = m_pOptions_DA->outputparms_array[i];
	const CWaveFormat* wave_format = &(m_pOptions_AD->waveFormat);
	dlg.m_samplingRate = wave_format->chrate;

	if (IDOK == dlg.DoModal())
	{
		for (int i = 0; i < 10; i++)
			m_pOptions_DA->outputparms_array[i] = dlg.outputparms_array[i];
		m_Acq32_DA.SetChannelList();
		m_Button_StartStop_DA.EnableWindow(m_Acq32_DA.GetDigitalChannel() > 0);
	}
}

void CADContView::OnBnClickedWriteToDisk()
{
	m_bADwritetofile = TRUE;
	m_pOptions_AD->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CADContView::OnBnClickedOscilloscope()
{
	m_bADwritetofile = FALSE;
	m_pOptions_AD->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CADContView::UpdateRadioButtons()
{
	if (m_bADwritetofile)
		m_Button_WriteToDisk.SetCheck(BST_CHECKED);
	else
		m_Button_Oscilloscope.SetCheck(BST_CHECKED);
	UpdateData(TRUE);
}

void CADContView::OnBnClickedCardfeatures()
{
	DlgDataTranslationBoard dlg;
	dlg.m_pAnalogIN = &m_Acq32_AD;
	dlg.m_pAnalogOUT = &m_Acq32_DA;
	dlg.DoModal();
}

void CADContView::OnBnClickedStartstop2()
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

BOOL CADContView::StartOutput()
{
	if (!m_Acq32_DA.InitSubSystem(m_pOptions_AD))
		return FALSE;
	m_Acq32_DA.DeclareAndFillBuffers(m_pOptions_AD);
	m_Acq32_AD.DeclareBuffers(m_pOptions_AD);
	m_Acq32_DA.ConfigAndStart();
	return TRUE;
}

void CADContView::StopOutput()
{
	m_Acq32_DA.StopAndLiberateBuffers();
}

void CADContView::InitAcquisitionInputFile() 
{
	//TODO make sure m_chsweeplength > 0
	CWaveFormat* pWFormat = &(m_pOptions_AD->waveFormat); // get pointer to m_pADC_options wave format

	m_chsweeplength = static_cast<long>(m_sweepduration * pWFormat->chrate / static_cast<float>(m_pOptions_AD->iundersample));
	// AD system is changed:  update AD buffers & change encoding: it is changed on-the-fly in the transfer loop
	*(m_inputDataFile.GetpWavechanArray()) = m_pOptions_AD->chanArray;
	*(m_inputDataFile.GetpWaveFormat()) = m_pOptions_AD->waveFormat;

	
	// set sweep length to the nb of data buffers
	(m_inputDataFile.GetpWaveFormat())->sample_count = m_chsweeplength * static_cast<long>(pWFormat->scan_count);
	m_inputDataFile.AdjustBUF(m_chsweeplength);
}

