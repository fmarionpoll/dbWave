#include "StdAfx.h"
#include <cmath>
#include "OLERRORS.H"
#include "Olxdadefs.h"
#include "Olxdaapi.h"

#include "resource.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "Adinterv.h"
#include "Adexperi.h"
#include "dtacq32.h"
#include "CyberAmp.h"

#include "ChartData.h"
#include "CViewADcontinuous.h"
#include "ChildFrm.h"
#include "DlgADInputParms.h"
#include "DlgConfirmSave.h"
#include "DlgDAChannels.h"
#include "DlgdtBoard.h"
#include "MainFrm.h"
#include "USBPxxS1.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto MIN_DURATION = 0.1;
constexpr auto MIN_BUFLEN = 32;

IMPLEMENT_DYNCREATE(CADContView, CFormView)

CADContView::CADContView()
	: CFormView(CADContView::IDD)
{
	m_bEnableActiveAccessibility = FALSE;
	m_ADC_yRulerBar.AttachScopeWnd(&m_ADsourceView, FALSE);
}

CADContView::~CADContView()
{
}

void CADContView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANALOGTODIGIT, m_Acq32_ADC);
	DDX_Control(pDX, IDC_DIGITTOANALOG, m_Acq32_DAC);
	DDX_Control(pDX, IDC_COMBOBOARD, m_ADcardCombo);
	DDX_Control(pDX, IDC_STARTSTOP, m_btnStartStop);
	DDX_CBIndex(pDX, IDC_COMBOSTARTOUTPUT, m_bStartOutPutMode);
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
	if (m_Acq32_ADC.ADC_IsInProgress())
		StopAcquisition(TRUE);

	m_Acq32_DAC.DAC_StopAndLiberateBuffers();

	if (m_bFoundDTOPenLayerDLL)
	{
		// TODO: save data here 
		if (m_Acq32_ADC.GetHDass() != NULL)
			m_Acq32_ADC.ADC_DeleteBuffers();

		if (m_Acq32_DAC.GetHDass() != NULL)
			m_Acq32_DAC.DAC_DeleteBuffers();
	}

	CFormView::OnDestroy();
	delete m_pBackgroundBrush;
}

HBRUSH CADContView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr;
	switch (nCtlColor)
	{
	case CTLCOLOR_EDIT:
	case CTLCOLOR_MSGBOX:
		// Set color to green on black and return the background brush.
		pDC->SetBkColor(m_backgroundColor);
		hbr = HBRUSH(m_pBackgroundBrush->GetSafeHandle());
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

void CADContView::OnCbnSelchangeComboboard()
{
	int isel = m_ADcardCombo.GetCurSel();
	CString csCardName;
	m_ADcardCombo.GetLBText(isel, csCardName);
	SelectDTOpenLayersBoard(csCardName);
}

BOOL CADContView::FindDTOpenLayersBoards()
{
	m_ADcardCombo.ResetContent();

	// load board name - skip dialog if only one is present
	UINT uiNumBoards = m_Acq32_ADC.GetNumBoards();
	if (uiNumBoards == 0)
	{
		m_ADcardCombo.AddString(_T("No Board"));
		m_ADcardCombo.SetCurSel(0);
		return FALSE;
	}

	for (UINT i = 0; i < uiNumBoards; i++)
		m_ADcardCombo.AddString(m_Acq32_ADC.GetBoardList(i));

	int isel = 0;
	// if name already defined, check if board present
	if (!(m_pADC_options->waveFormat).csADcardName.IsEmpty())
		isel = m_ADcardCombo.FindString(-1, (m_pADC_options->waveFormat).csADcardName);
	if (isel < 0)
		isel = 0;

	m_ADcardCombo.SetCurSel(isel);
	m_boardName = m_Acq32_ADC.GetBoardList(isel);
	SelectDTOpenLayersBoard(m_boardName);
	return TRUE;
}

BOOL CADContView::SelectDTOpenLayersBoard(CString cardName)
{
	// get infos
	m_bFoundDTOPenLayerDLL = TRUE;
	(m_pADC_options->waveFormat).csADcardName = cardName;

	// connect A/D subsystem and display/hide buttons
	m_bStartOutPutMode = 0;
	const BOOL flag_AD = m_Acq32_ADC.ADC_OpenSubSystem(cardName);
	const BOOL flag_DA = m_Acq32_DAC.DAC_OpenSubSystem(cardName);
	if (flag_DA)
		m_bStartOutPutMode = 0;
	m_bSimultaneousStart = m_bsimultaneousStartDA && m_bsimultaneousStartAD;

	// display additional interface elements
	int show = (flag_AD ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_ADPARAMETERS)->ShowWindow(show);
	GetDlgItem(IDC_ADPARAMETERS2)->ShowWindow(show);
	GetDlgItem(IDC_COMBOSTARTOUTPUT)->ShowWindow(show);

	show = (flag_DA ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DAPARAMETERS2)->ShowWindow(show);
	GetDlgItem(IDC_DAGROUP)->ShowWindow(show);
	GetDlgItem(IDC_COMBOSTARTOUTPUT)->ShowWindow(show);
	GetDlgItem(IDC_STARTSTOP2)->ShowWindow(show);
	if (show == SW_SHOW)
		SetCombostartoutput(m_pDAC_options->bAllowDA);

	return TRUE;
}

void CADContView::StopAcquisition(BOOL bDisplayErrorMsg)
{
	if (!m_Acq32_ADC.ADC_IsInProgress())
		return;

	// special treatment if simultaneous list
	if (m_bSimultaneousStart && m_bStartOutPutMode == 0)
	{
		HSSLIST hSSlist;
		CHAR errstr[255];
		ECODE ecode;
		ecode = olDaGetSSList(&hSSlist);
		olDaGetErrorString(ecode, errstr, 255);
		ecode = olDaReleaseSSList(hSSlist);
		olDaGetErrorString(ecode, errstr, 255);
	}

	// stop AD, liberate DTbuffers
	m_Acq32_ADC.ADC_StopAndLiberateBuffers();

	// stop DA, liberate buffers
	if (m_bStartOutPutMode == 0)
		m_Acq32_DAC.DAC_StopAndLiberateBuffers();

	// close file and update display
	if (m_bFileOpen)
	{
		SaveAndCloseFile();
		// update view data	
		const long lsizeDOCchan = m_acquiredDataFile.GetDOCchanLength();
		m_ADsourceView.AttachDataFile(&m_acquiredDataFile);
		m_ADsourceView.ResizeChannels(m_ADsourceView.GetRectWidth(), lsizeDOCchan);
		m_ADsourceView.GetDataFromDoc(0, lsizeDOCchan);
	}
}

void CADContView::SaveAndCloseFile()
{
	m_acquiredDataFile.AcqDoc_DataAppendStop();
	CWaveFormat* pWFormat = m_acquiredDataFile.GetpWaveFormat();

	// if burst data acquisition mode ------------------------------------
	if (m_bhidesubsequent)
	{
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
			m_csNameArray.Add(m_szFileName);
		else
			m_acquiredDataFile.AcqDeleteFile();
	}

	// normal data acquisition mode --------------------------------------
	else
	{
		int ires = IDCANCEL;
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
		{
			CDlgConfirmSave dlg;
			dlg.m_csfilename = m_szFileName;
			ires = dlg.DoModal();
		}
		// if no data or user answered no, erase the data
		if (IDOK != ires)
		{
			m_acquiredDataFile.AcqDeleteFile();
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
	CAcqDataDoc * pDocDat = pdbDoc->OpenCurrentDataFile();
	if (pDocDat == nullptr)
	{
		CdbWdatabase* pDB = pdbDoc->m_pDB;
		int nrecords = pDB->GetNRecords();
		ATLTRACE2(_T("error reading current document"));
		return;
	}
	pDocDat->ReadDataInfos();
	long lsizeDOCchan = pDocDat->GetDOCchanLength();
	m_ADsourceView.AttachDataFile(pDocDat);
	m_ADsourceView.ResizeChannels(m_ADsourceView.GetRectWidth(), lsizeDOCchan);
	m_ADsourceView.GetDataFromDoc(0, lsizeDOCchan);
}

void CADContView::TransferFilesToDatabase()
{
	CdbWaveDoc* pdbDoc = GetDocument();
	pdbDoc->ImportFileList(m_csNameArray);				// add file name(s) to the list of records in the database
	m_csNameArray.RemoveAll();									// clear file names

	CdbMainTable* pSet = &(GetDocument()->m_pDB->m_mainTableSet);
	pSet->BuildAndSortIDArrays();
	pSet->RefreshQuery();
	pdbDoc->SetDB_CurrentRecordPosition(pdbDoc->m_pDB->GetNRecords() - 1);
	pdbDoc->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);
}

BOOL CADContView::StartAcquisition()
{
	// set display
	if (m_bADwritetofile && !Defineexperiment())
	{
		StopAcquisition(FALSE);
		UpdateStartStop(m_Acq32_ADC.ADC_IsInProgress());
		return FALSE;
	}

	m_ADC_present = m_Acq32_ADC.ADC_InitSubSystem();
	if (!m_ADC_present)
		return FALSE;

	m_DAC_present = m_Acq32_DAC.DAC_InitSubSystem(m_pADC_options->waveFormat.chrate, m_pADC_options->waveFormat.trig_mode);
	if (m_bStartOutPutMode == 0 && m_DAC_present)
		m_Acq32_DAC.DAC_DeclareAndFillBuffers(
			m_pADC_options->sweepduration, 
			m_pADC_options->waveFormat.chrate, 
			m_pADC_options->waveFormat.bufferNitems);
	
	// start AD display
	m_chsweep1 = 0;
	m_chsweep2 = -1;
	m_ADsourceView.ADdisplayStart(m_chsweeplength);
	CWaveFormat* pWFormat = m_acquiredDataFile.GetpWaveFormat();
	pWFormat->sample_count = 0;							// no samples yet
	pWFormat->chrate = pWFormat->chrate / m_pADC_options->iundersample;
	m_fclockrate = pWFormat->chrate * pWFormat->scan_count;
	pWFormat->acqtime = CTime::GetCurrentTime();

	// data format
	pWFormat->binspan = (m_pADC_options->waveFormat).binspan;
	pWFormat->fullscale_volts = (m_pADC_options->waveFormat).fullscale_volts;
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
			UpdateStartStop(m_Acq32_ADC.ADC_IsInProgress());
			return FALSE;
		}
	}

	// starting mode of A/D if no simultaneous list
	if (!m_bSimultaneousStart || m_bStartOutPutMode != 0)
	{
		m_Acq32_ADC.ADC_Start();
		if (m_bStartOutPutMode == 0 && m_DAC_present)
			m_Acq32_DAC.DAC_Start();
	}

	// starting A/D when simultaneous list 
	else
	{
		BOOL retval;
		HSSLIST hSSlist;
		CHAR errstr[255];

		// create simultaneous starting list
		ECODE ecode = olDaGetSSList(&hSSlist);
		olDaGetErrorString(ecode, errstr, 255);

		
		// DA system
		m_Acq32_DAC.Config();
		ecode = olDaPutDassToSSList(hSSlist, (HDASS)m_Acq32_DAC.GetHDass());
		if (ecode != OLNOERROR)
		{
			retval = ecode;
			ecode = olDaReleaseSSList(hSSlist);
			return retval;
		}

		// AD system
		m_Acq32_ADC.Config();
		ecode = olDaPutDassToSSList(hSSlist, (HDASS)m_Acq32_ADC.GetHDass());
		if (ecode != OLNOERROR)
		{
			retval = ecode;
			ecode = olDaReleaseSSList(hSSlist);
			return retval;
		}

		// prestart
		ecode = olDaSimultaneousPrestart(hSSlist);
		if (ecode != OLNOERROR) {
			olDaGetErrorString(ecode, errstr, 255);
			displayolDaErrorMessage(errstr);
		}

		// start simultaneously
		ecode = olDaSimultaneousStart(hSSlist);
		if (ecode != OLNOERROR) {
			olDaGetErrorString(ecode, errstr, 255);
			displayolDaErrorMessage(errstr);
		}

		m_Acq32_ADC.ADC_SetInProgress();
		m_Acq32_DAC.DAC_SetInProgress();
	}

	return TRUE;
}

void CADContView::displayolDaErrorMessage(CHAR* errstr)
{
	CString csError;
	CStringA cstringa(errstr);
	csError = cstringa;
	AfxMessageBox(csError);
}

#ifdef _DEBUG
void CADContView::AssertValid() const
{
	CFormView::AssertValid();
}

void CADContView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CdbWaveDoc* CADContView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}

#endif //_DEBUG

CDaoRecordset* CADContView::OnGetRecordset()
{
	return m_ptableSet;
}

void CADContView::OnInitialUpdate()
{
	// attach controls
	VERIFY(m_ADsourceView.SubclassDlgItem(IDC_DISPLAYDATA, this));
	VERIFY(m_ADC_yRulerBar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_ADC_xRulerBar.SubclassDlgItem(IDC_XSCALE, this));
	m_ADC_yRulerBar.AttachScopeWnd(&m_ADsourceView, FALSE);
	m_ADC_xRulerBar.AttachScopeWnd(&m_ADsourceView, TRUE);
	m_ADsourceView.AttachExternalXRuler(&m_ADC_xRulerBar);
	m_ADsourceView.AttachExternalYRuler(&m_ADC_yRulerBar);
	m_ADsourceView.m_bNiceGrid = TRUE;

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
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, WPARAM(IMAGE_ICON), LPARAM(HANDLE(m_hBias)));
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, WPARAM(IMAGE_ICON), LPARAM(HANDLE(m_hZoom)));

	const BOOL b32BitIcons = afxGlobalData.m_nBitsPerPixel >= 16;
	m_btnStartStop.SetImage(b32BitIcons ? IDB_CHECK32 : IDB_CHECK);
	m_btnStartStop.SetCheckedImage(b32BitIcons ? IDB_CHECKNO32 : IDB_CHECKNO);
	m_btnStartStop.EnableWindowsTheming(false); // true
	m_btnStartStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D; //BUTTONSTYLE_SEMIFLAT;

	// scrollbar
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	// CFormView init CFile
	auto pApp = static_cast<CdbWaveApp*>(AfxGetApp());
	m_pADC_options = &(pApp->options_acqdata);								// address of data acquisition parameters
	m_pDAC_options = &(pApp->options_outputdata);								// address of data output parameters
	m_bFoundDTOPenLayerDLL = FALSE;								// assume there is no card
	m_bADwritetofile = m_pADC_options->waveFormat.bADwritetofile;
	m_bStartOutPutMode = m_pDAC_options->bAllowDA;
	((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->SetCurSel(m_bStartOutPutMode);

	// open document and remove database filters
	CdbWaveDoc* pdbDoc = GetDocument();							// data document with database
	m_ptableSet = &pdbDoc->m_pDB->m_mainTableSet;					// database itself
	m_ptableSet->m_strFilter.Empty();
	m_ptableSet->ClearFilters();
	m_ptableSet->RefreshQuery();
	CFormView::OnInitialUpdate();

	// if current document, load parameters from current document into the local set of parameters
	// if current document does not exist, do nothing
	if (pdbDoc->m_pDB->GetNRecords() > 0) {
		pdbDoc->OpenCurrentDataFile();							// read data descriptors from current data file
		CAcqDataDoc* pDat = pdbDoc->m_pDat;						// get a pointer to the data file itself
		if (pDat != NULL)
		{
			m_pADC_options->waveFormat = *(pDat->GetpWaveFormat());	// read data header
			m_pADC_options->chanArray.ChanArray_setSize(m_pADC_options->waveFormat.scan_count);
			m_pADC_options->chanArray = *pDat->GetpWavechanArray();	// get channel descriptors
			// restore state of "write-to-file" parameter that was just erased
			m_pADC_options->waveFormat.bADwritetofile = m_bADwritetofile;
		}
	}

	// create data file and copy data acquisition parameters into it
	m_acquiredDataFile.OnNewDocument();							// create a file to receive incoming data (A/D)
	*(m_acquiredDataFile.GetpWaveFormat()) = m_pADC_options->waveFormat;	// copy data formats into this file
	m_pADC_options->chanArray.ChanArray_setSize(m_pADC_options->waveFormat.scan_count);
	*(m_acquiredDataFile.GetpWavechanArray()) = m_pADC_options->chanArray;
	m_ADsourceView.AttachDataFile(&m_acquiredDataFile);		// prepare display area

	pApp->m_bADcardFound = FindDTOpenLayersBoards();			// open DT Open Layers board
	if (pApp->m_bADcardFound)
	{
		m_Acq32_ADC.ADC_InitSubSystem();									// connect A/D DT OpenLayer subsystem
		InitCyberAmp();											// control cyberamplifier
		
		m_Acq32_DAC.DAC_InitSubSystem(m_pADC_options->waveFormat.chrate, m_pADC_options->waveFormat.trig_mode);
		m_Acq32_DAC.DAC_ClearAllOutputs();
	}
	else
	{
		GetDlgItem(IDC_STARTSTOP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADPARAMETERS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADPARAMETERS2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DAGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBOSTARTOUTPUT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STARTSTOP2)->ShowWindow(SW_HIDE);
	}

	UpdateChanLegends(0);
	UpdateRadioButtons();

	// tell mmdi parent which cursor is active
	GetParent()->PostMessage(WM_MYMESSAGE, NULL, MAKELPARAM(m_cursorstate, HINT_SETMOUSECURSOR));
}

void CADContView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// update sent from within this class
	if (pSender == this)
	{
		ASSERT(GetDocument() != NULL);
		m_ADsourceView.Invalidate();		// display data
	}
}

void CADContView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CMainFrame* pmF = (CMainFrame*)AfxGetMainWnd();
	// activate view
	if (bActivate)
	{
		pmF->ActivatePropertyPane(FALSE);
		((CChildFrame*)pmF->MDIGetActive())->m_cursorstate = 0;
	}
	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CADContView::OnSize(UINT nType, int cx, int cy)
{
	switch (nType)
	{
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
		StopAcquisition(TRUE);
		UpdateStartStop(m_Acq32_ADC.ADC_IsInProgress());
		
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
	int code = HIWORD(lParam);	// code parameter
	// code = 0: chan hit 			lowp = channel
	// code = 1: cursor change		lowp = new cursor value
	// code = 2: horiz cursor hit	lowp = cursor index	
	int lowp = LOWORD(lParam);	// value associated with code

	switch (code)
	{
	case HINT_SETMOUSECURSOR:
		if (lowp > CURSOR_ZOOM)
			lowp = 0;
		m_cursorstate = m_ADsourceView.SetMouseCursorType(lowp);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
		break;

	default:
		if (lowp == 0) MessageBeep(MB_ICONEXCLAMATION);
		break;
	}
	return 0L;
}

void CADContView::OnBnClickedStartstop()
{
	BOOL bADStart = TRUE;

	// Start
	if (m_btnStartStop.IsChecked())
	{
		bADStart = StartAcquisition();
		if (bADStart)
		{
			if ((m_acquiredDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
				OnBufferDone_ADC();
		}
		else
		{
			bADStart = FALSE;
			StopAcquisition(FALSE);
		}
	}
	// Stop
	else
	{
		bADStart = FALSE;
		StopAcquisition(TRUE);
		if (m_bhidesubsequent)
		{
			TransferFilesToDatabase();
			UpdateViewDataFinal();
		}
		else
			if (m_bchanged && m_bAskErase)
				if (AfxMessageBox(IDS_ACQDATA_SAVEYESNO, MB_YESNO) == IDYES)
					m_bchanged = FALSE;
	}
	// change display items
	UpdateStartStop(m_Acq32_ADC.ADC_IsInProgress());
}

void CADContView::UpdateStartStop(BOOL bStart)
{
	bool isrunning = m_Acq32_ADC.ADC_IsInProgress();
	if (bStart)
	{
		m_btnStartStop.SetWindowText(_T("STOP"));
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);
	}
	else
	{
		m_btnStartStop.SetWindowText(_T("START"));
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
		if (m_pADC_options->baudiblesound)
			Beep(500, 400);
		ASSERT(isrunning == FALSE);
	}
	m_btnStartStop.SetCheck(isrunning);
	// change display
	m_ADsourceView.Invalidate();
}

void CADContView::OnHardwareDefineexperiment()
{
	Defineexperiment();
}

BOOL CADContView::Defineexperiment()
{
	CString csfilename;
	m_bFileOpen = FALSE;

	// define experiment (dialog)
	if (!m_bhidesubsequent)
	{
		ADExperimentDlg dlg;		// create dialog box
		dlg.m_pADC_options = m_pADC_options;
		dlg.m_pdbDoc = GetDocument();
		dlg.m_bhidesubsequent = m_bhidesubsequent;

		// save here current file name and path
		// ...	
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
		m_pADC_options->exptnumber++;
		csBufTemp.Format(_T("%06.6lu"), m_pADC_options->exptnumber);
		csfilename = m_pADC_options->csPathname + m_pADC_options->csBasename + csBufTemp + _T(".dat");

		// check if this file is already present, exit if not...
		CFileStatus status;
		int iIDresponse = IDYES;	// default: go on if file not found
		if (CFile::GetStatus(csfilename, status))
			iIDresponse = AfxMessageBox(IDS_FILEOVERWRITE, MB_YESNO | MB_ICONWARNING);
		// no .. find first available number
		if (IDNO == iIDresponse)
		{
			BOOL flag = TRUE;
			while (flag)
			{
				m_pADC_options->exptnumber++;
				csBufTemp.Format(_T("%06.6lu"), m_pADC_options->exptnumber);
				csfilename = m_pADC_options->csPathname + m_pADC_options->csBasename + csBufTemp + _T(".dat");
				flag = CFile::GetStatus(csfilename, status);
			}
			const CString cs = _T("The Next available file name is: ") + csfilename;
			iIDresponse = AfxMessageBox(cs, MB_YESNO | MB_ICONWARNING);
		}
	}
	// close current file and open new file to prepare it for adding chunks of data
	m_acquiredDataFile.AcqCloseFile();
	if (!m_acquiredDataFile.CreateAcqFile(csfilename))
		return FALSE;
	m_szFileName = csfilename;
	m_acquiredDataFile.AcqDoc_DataAppendStart();
	m_bFileOpen = TRUE;
	return TRUE;
}

void CADContView::OnHardwareAdchannels()
{
	UpdateData(TRUE);

	CDlgADInputs dlg;
	dlg.m_pwFormat = &(m_pADC_options->waveFormat);
	dlg.m_pchArray = &(m_pADC_options->chanArray);  
	dlg.m_numchansMAXDI = m_Acq32_ADC.GetSSCaps(OLSSC_MAXDICHANS);
	dlg.m_numchansMAXSE = m_Acq32_ADC.GetSSCaps(OLSSC_MAXSECHANS);
	dlg.m_bchantype = m_pADC_options->bChannelType;
	dlg.m_bchainDialog = TRUE;
	dlg.m_bcommandAmplifier = TRUE;

	const auto p_alligator = new CUSBPxxS1();
	dlg.m_palligator = p_alligator;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_pADC_options->bChannelType = dlg.m_bchantype;
		m_Acq32_ADC.ADC_InitSubSystem();
		UpdateData(FALSE);
		UpdateChanLegends(0);
	}
	delete p_alligator;

	if (dlg.m_postmessage != NULL)
		ChainDialog(dlg.m_postmessage);
}

void CADContView::OnHardwareAdintervals()
{
	ADIntervalsDlg dlg;
	CWaveFormat* pWFormat = &(m_pADC_options->waveFormat);
	dlg.m_pwaveFormat = pWFormat;
	dlg.m_ratemin = 1.0f;
	dlg.m_ratemax = float(m_Acq32_ADC.ADC_GetMaximumFrequency() / pWFormat->scan_count);
	dlg.m_bufferWsizemax = UINT(65536) * 4;
	dlg.m_undersamplefactor = m_pADC_options->iundersample;
	dlg.m_baudiblesound = m_pADC_options->baudiblesound;
	dlg.m_sweepduration = m_sweepduration;
	dlg.m_bchainDialog = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		StopAcquisition(TRUE);
		UpdateStartStop(m_Acq32_ADC.ADC_IsInProgress());
		
		m_pADC_options->iundersample = int(dlg.m_undersamplefactor);
		m_pADC_options->baudiblesound = dlg.m_baudiblesound;
		m_sweepduration = dlg.m_sweepduration;
		m_pADC_options->sweepduration = m_sweepduration;
		m_Acq32_ADC.ADC_InitSubSystem();
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
	UpdateStartStop(m_Acq32_ADC.ADC_IsInProgress());
	AfxMessageBox(IDS_ACQDATA_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void CADContView::OnQueueDone_ADC()
{
	StopAcquisition(TRUE);
	UpdateStartStop(m_Acq32_ADC.ADC_IsInProgress());
	AfxMessageBox(IDS_ACQDATA_TOOFAST);
}

void CADContView::OnOverrunError_ADC()
{
	StopAcquisition(TRUE);
	UpdateStartStop(m_Acq32_ADC.ADC_IsInProgress());
	AfxMessageBox(IDS_ACQDATA_OVERRUN);
}

void CADContView::OnOverrunError_DAC()
{
	m_Acq32_DAC.DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_OVERRUN);
}

void CADContView::OnQueueDone_DAC()
{
	m_Acq32_DAC.DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TOOFAST);
}

void CADContView::OnTriggerError_DAC()
{
	m_Acq32_DAC.DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void CADContView::OnBufferDone_ADC()
{
	m_Acq32_ADC.ADC_OnBufferDone();
}

void CADContView::OnBufferDone_DAC()
{
	m_Acq32_DAC.DAC_OnBufferDone();
}



BOOL CADContView::InitCyberAmp()
{
	CCyberAmp m_cyber;
	BOOL bcyberPresent = FALSE;
	int nchans = (m_pADC_options->chanArray).ChanArray_getSize();

	// test if Cyberamp320 selected
	for (int i = 0; i < nchans; i++)
	{
		CWaveChan* pchan = (m_pADC_options->chanArray).Get_p_channel(i);

		int a = pchan->am_csamplifier.Find(_T("CyberAmp"));
		int b = pchan->am_csamplifier.Find(_T("Axon Instrument"));
		if (a == 0 || b == 0)
		{
			// test if cyberamp present
			if (!bcyberPresent)
				bcyberPresent = (m_cyber.Initialize() == C300_SUCCESS);
			if (!bcyberPresent) {
				AfxMessageBox(_T("CyberAmp not found"), MB_OK);
				continue;
			}

			// chan, gain, filter +, lowpass, notch	
			m_cyber.SetHPFilter(pchan->am_amplifierchan, C300_POSINPUT, pchan->am_csInputpos);
			m_cyber.SetmVOffset(pchan->am_amplifierchan, pchan->am_offset);

			m_cyber.SetNotchFilter(pchan->am_amplifierchan, pchan->am_notchfilt);
			m_cyber.SetGain(pchan->am_amplifierchan, int(pchan->am_gaintotal / (double (pchan->am_gainheadstage) * double(pchan->am_gainAD))));
			m_cyber.SetLPFilter(pchan->am_amplifierchan, int(pchan->am_lowpass));
			int errorcode = m_cyber.C300_FlushCommandsAndAwaitResponse();
		}
	}
	return bcyberPresent;
}

void CADContView::OnBnClickedGainbutton()
{
	((CButton*) GetDlgItem(IDC_BIAS_button))->SetState(0);
	((CButton*) GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

void CADContView::OnBnClickedBiasbutton()
{
	// set bias down and set gain up CButton	
	((CButton*) GetDlgItem(IDC_BIAS_button))->SetState(1);
	((CButton*) GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

void CADContView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
	const int ichan = 0;			// TODO see which channel is selected
	int l_size = m_ADsourceView.GetChanlistItem(ichan)->GetYextent();

	// get corresponding data
	switch (nSBCode)
	{
		// .................scroll to the start
	case SB_LEFT:		l_size = YEXTENT_MIN; break;
		// .................scroll one line left
	case SB_LINELEFT:	l_size -= l_size / 10 + 1; break;
		// .................scroll one line right
	case SB_LINERIGHT:	l_size += l_size / 10 + 1; break;
		// .................scroll one page left
	case SB_PAGELEFT:	l_size -= l_size / 2 + 1; break;
		// .................scroll one page right
	case SB_PAGERIGHT:	l_size += l_size + 1; break;
		// .................scroll to end right
	case SB_RIGHT:		l_size = YEXTENT_MAX; break;
		// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	l_size = MulDiv(nPos - 50, YEXTENT_MAX, 100); break;
		// .................NOP: set position only
	default:			break;
	}

	// change y extent
	if (l_size > 0) //&& lSize<=YEXTENT_MAX)
	{
		// assume that all channels are displayed at the same gain & offset
		const CWaveFormat* pWFormat = &(m_pADC_options->waveFormat);
		const int ichanfirst = 0;
		const int ichanlast = pWFormat->scan_count - 1;

		for (int channel = ichanfirst; channel <= ichanlast; channel++) 
		{
			CChanlistItem* pChan = m_ADsourceView.GetChanlistItem(channel);
			pChan->SetYextent(l_size);
		}
		m_ADsourceView.Invalidate();
		UpdateChanLegends(0);
		m_pADC_options->izoomCursel = l_size;
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
	int ichan = 0;			// TODO: see which channel is selected
	CChanlistItem* pChan = m_ADsourceView.GetChanlistItem(ichan);
	int lSize = pChan->GetYzero() - pChan->GetDataBinZero();
	int yextent = pChan->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			// scroll to the start
		lSize = YZERO_MIN;
		break;
	case SB_LINELEFT:		// scroll one line left
		lSize -= yextent / 100 + 1;
		break;
	case SB_LINERIGHT:		// scroll one line right
		lSize += yextent / 100 + 1;
		break;
	case SB_PAGELEFT:		// scroll one page left
		lSize -= yextent / 10 + 1;
		break;
	case SB_PAGERIGHT:		// scroll one page right
		lSize += yextent / 10 + 1;
		break;
	case SB_RIGHT:			// scroll to end right
		lSize = YZERO_MAX;
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		lSize = int(nPos - 50) * int(YZERO_SPAN / 100);
		break;
	default:				// NOP: set position only
		break;
	}

	// try to read data with this new size
	if (lSize > YZERO_MIN && lSize < YZERO_MAX)
	{
		CWaveFormat* pWFormat = &(m_pADC_options->waveFormat);
		int ichanfirst = 0;
		int ichanlast = pWFormat->scan_count - 1;
		for (int ichan = ichanfirst; ichan <= ichanlast; ichan++)
		{
			CChanlistItem* pChan = m_ADsourceView.GetChanlistItem(ichan);
			pChan->SetYzero(lSize + pChan->GetDataBinZero());
		}
		m_ADsourceView.Invalidate();
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
	int ichan = 0;			// TODO see which channel is selected
	CChanlistItem* pChan = m_ADsourceView.GetChanlistItem(ichan);
	int iPos = (int)((pChan->GetYzero() - pChan->GetDataBinZero())
		* 100 / (int)YZERO_SPAN) + (int)50;
	m_scrolly.SetScrollPos(iPos, TRUE);
}

void CADContView::UpdateGainScroll()
{
	// assume that all channels are displayed at the same gain & offset
	int ichan = 0;
	CChanlistItem* pChan = m_ADsourceView.GetChanlistItem(ichan);
	m_scrolly.SetScrollPos(MulDiv(pChan->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
}

void CADContView::UpdateChanLegends(int chan)
{
	int ichan = 0;
	CChanlistItem* pChan = m_ADsourceView.GetChanlistItem(ichan);
	int yzero = pChan->GetYzero();
	int yextent = pChan->GetYextent();
	float mVperbin = pChan->GetVoltsperDataBin() * 1000.0f;
	int binzero = 0;
}

float CADContView::ValueToVolts(CDTAcq32* pSS, long lVal, double dfGain)
{
	const long lRes = long(pow(2.0, double(pSS->GetResolution())));
	float f_min = 0.F;
	if (pSS->GetMinRange() != 0.F)
		f_min = pSS->GetMinRange() / float(dfGain);

	float f_max = 0.F;
	if (pSS->GetMaxRange() != 0.F)
		f_max = pSS->GetMaxRange() / float(dfGain);

	//make sure value is sign extended if 2's comp
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
	{
		lVal = lVal & (lRes - 1);
		if (lVal >= (lRes / 2))
			lVal = lVal - lRes;
	}

	// convert to volts
	float f_volts = float(lVal) * (f_max - f_min) / lRes;
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
		f_volts = f_volts + ((f_max + f_min) / 2);
	else
		f_volts = f_volts + f_min;

	return f_volts;
}

long CADContView::VoltsToValue(CDTAcq32* pSS, float fVolts, double dfGain)
{
	const long lRes = long(pow(2., double(pSS->GetResolution())));

	float f_min = 0.F;
	if (pSS->GetMinRange() != 0.F)
		f_min = pSS->GetMinRange() / float(dfGain);

	float f_max = 0.F;
	if (pSS->GetMaxRange() != 0.F)
		f_max = pSS->GetMaxRange() / float(dfGain);

	//clip input to range
	if (fVolts > f_max) 
		fVolts = f_max;
	if (fVolts < f_min) 
		fVolts = f_min;

	//if 2's comp encoding
	long l_value;
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
	{
		l_value = long((fVolts - (f_min + f_max) / 2) * lRes / (f_max - f_min));
		// adjust for binary wrap if any
		if (l_value == (lRes / 2)) 
			l_value -= 1;
	}
	else
	{
		// convert to offset binary
		l_value = long((fVolts - f_min) * lRes / (f_max - f_min));
		// adjust for binary wrap if any
		if (l_value == lRes) 
			l_value -= 1;
	}
	return l_value;
}

void CADContView::OnCbnSelchangeCombostartoutput()
{
	m_bStartOutPutMode = ((CComboBox*) GetDlgItem(IDC_COMBOSTARTOUTPUT))->GetCurSel();
	m_pDAC_options->bAllowDA = m_bStartOutPutMode;
	GetDlgItem(IDC_STARTSTOP2)->EnableWindow(m_bStartOutPutMode != 0);
}

void CADContView::SetCombostartoutput(int option)
{
	((CComboBox*) GetDlgItem(IDC_COMBOSTARTOUTPUT))->SetCurSel(option);
	option = ((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->GetCurSel();
	m_bStartOutPutMode = option;
	m_pDAC_options->bAllowDA = option;
	GetDlgItem(IDC_STARTSTOP2)->EnableWindow(m_bStartOutPutMode != 0);
}

void CADContView::OnBnClickedDaparameters2()
{
	CDlgDAChannels dlg;
	const auto isize = m_pDAC_options->outputparms_array.GetSize();
	if (isize < 10)
		m_pDAC_options->outputparms_array.SetSize(10);
	dlg.outputparms_array.SetSize(10);
	for (auto i = 0; i < 10; i++)
		dlg.outputparms_array[i] = m_pDAC_options->outputparms_array[i];
	const CWaveFormat* wave_format = &(m_pADC_options->waveFormat);
	dlg.m_samplingRate = wave_format->chrate;

	if (IDOK == dlg.DoModal())
	{
		for (int i = 0; i < 10; i++)
			m_pDAC_options->outputparms_array[i] = dlg.outputparms_array[i];
		m_Acq32_DAC.DAC_SetChannelList();
		GetDlgItem(IDC_STARTSTOP2)->EnableWindow(m_Acq32_DAC.DAC_GetDigitalChannel() > 0);
	}
}

void CADContView::OnBnClickedWriteToDisk()
{
	m_bADwritetofile = TRUE;
	m_pADC_options->waveFormat.bADwritetofile = m_bADwritetofile;
	m_acquiredDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CADContView::OnBnClickedOscilloscope()
{
	m_bADwritetofile = FALSE;
	m_pADC_options->waveFormat.bADwritetofile = m_bADwritetofile;
	m_acquiredDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CADContView::UpdateRadioButtons()
{
	if (m_bADwritetofile)
		((CButton*) GetDlgItem(IDC_WRITETODISK))->SetCheck(BST_CHECKED);
	else
		((CButton*) GetDlgItem(IDC_OSCILLOSCOPE))->SetCheck(BST_CHECKED);
	UpdateData(TRUE);
}

void CADContView::OnBnClickedCardfeatures()
{
	CDlgDataTranslationBoard dlg;
	dlg.m_pAnalogIN = &m_Acq32_ADC;
	dlg.m_pAnalogOUT = &m_Acq32_DAC;
	dlg.DoModal();
}

void CADContView::OnBnClickedStartstop2()
{
	CString cs;
	if (m_Acq32_DAC.DAC_IsInProgress())
	{
		cs = _T("Start");
		StopOutput();
	}
	else
	{
		cs = _T("Stop");
		StartOutput();
	}
	GetDlgItem(IDC_STARTSTOP2)->SetWindowTextW(cs);
}

BOOL CADContView::StartOutput()
{
	if (!m_Acq32_DAC.DAC_InitSubSystem(m_pADC_options->waveFormat.chrate, m_pADC_options->waveFormat.trig_mode))
		return FALSE;
	m_Acq32_DAC.DAC_DeclareAndFillBuffers(
		m_pADC_options->sweepduration,
		m_pADC_options->waveFormat.chrate,
		m_pADC_options->waveFormat.bufferNitems);
	m_Acq32_DAC.DAC_Start();
	return TRUE;
}

void CADContView::StopOutput()
{
	m_Acq32_DAC.DAC_StopAndLiberateBuffers();
}

