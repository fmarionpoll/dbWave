#include "stdafx.h"
#include <math.h>
#include "resource.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "adinterv.h"
#include "adexperi.h"
#include "dtacq32.h"
#include "CyberAmp.h"

#include <Olxdadefs.h>
#include <olxdaapi.h>

#include "ChartData.h"
#include "CViewADcontinuous.h"
#include "ChildFrm.h"
#include "DlgADInputParms.h"
#include "DlgConfirmSave.h"
#include "DlgDAChannels.h"
#include "DlgdtBoard.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MIN_DURATION 0.1
#define MIN_BUFLEN	32

IMPLEMENT_DYNCREATE(CADContView, CFormView)

CADContView::CADContView()
	: CFormView(CADContView::IDD)
{
	m_BkColor = GetSysColor(COLOR_BTNFACE);		// set color for edit button
	m_pEditBkBrush = new CBrush(m_BkColor);		// background color = like a button
	ASSERT(m_pEditBkBrush != NULL);				// check brush

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
	if (m_ADC_inprogress)
		StopAcquisition(TRUE);

	if (m_DAC_inprogress)
		DAC_StopAndLiberateBuffers();

	if (m_bFoundDTOPenLayerDLL)
	{
		// TODO: save data here 
		if (m_Acq32_ADC.GetHDass() != NULL)
			ADC_DeleteBuffers();

		if (m_Acq32_DAC.GetHDass() != NULL)
			DAC_DeleteBuffers();
	}
	delete m_pEditBkBrush;		// brush for edit controls	
	CFormView::OnDestroy();
}

HBRUSH CADContView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr;
	switch (nCtlColor)
	{
	case CTLCOLOR_EDIT:
	case CTLCOLOR_MSGBOX:
		// Set color to green on black and return the background brush.
		pDC->SetBkColor(m_BkColor);
		hbr = (HBRUSH)m_pEditBkBrush->GetSafeHandle();
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
	BOOL flagAD = ADC_OpenSubSystem(cardName);

	BOOL flagDA = DAC_OpenSubSystem(cardName);
	if (flagDA)
		m_bStartOutPutMode = 0;
	m_bSimultaneousStart = m_bsimultaneousStartDA && m_bsimultaneousStartAD;

	// display additional interface elements
	int bShow = (flagAD ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_ADPARAMETERS)->ShowWindow(bShow);
	GetDlgItem(IDC_ADPARAMETERS2)->ShowWindow(bShow);
	GetDlgItem(IDC_COMBOSTARTOUTPUT)->ShowWindow(bShow);

	bShow = (flagDA ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DAPARAMETERS2)->ShowWindow(bShow);
	GetDlgItem(IDC_DAGROUP)->ShowWindow(bShow);
	GetDlgItem(IDC_COMBOSTARTOUTPUT)->ShowWindow(bShow);
	GetDlgItem(IDC_STARTSTOP2)->ShowWindow(bShow);
	SetCombostartoutput(m_pDAC_options->bAllowDA);

	return TRUE;
}

BOOL CADContView::ADC_OpenSubSystem(CString cardName)
{
	try
	{
		m_Acq32_ADC.SetBoard(cardName);
		int nAD = m_Acq32_ADC.GetDevCaps(OLDC_ADELEMENTS);		// make sure A/D is available
		if (nAD < 1)
			return FALSE;
		m_Acq32_ADC.SetSubSysType(OLSS_AD);						// select A/D system
		m_Acq32_ADC.SetSubSysElement(0);
		ASSERT(m_Acq32_ADC.GetHDass() != NULL);
		m_bsimultaneousStartAD = m_Acq32_ADC.GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	}

	// save parameters into CWaveFormat
	CWaveFormat* pWFormat = &(m_pADC_options->waveFormat);
	float max = m_Acq32_ADC.GetMaxRange();						// maximum input voltage
	float min = m_Acq32_ADC.GetMinRange();						// minimum input voltage
	pWFormat->fullscale_Volts = (float)(max - min);

	// convert into bin scale (nb of divisions)
	int iresolution = m_Acq32_ADC.GetResolution();
	pWFormat->binspan = ((1L << iresolution) - 1);

	// set max channel number according to input configuration m_numchansMAX
	m_pADC_options->bChannelType = m_Acq32_ADC.GetChannelType();
	if (m_pADC_options->bChannelType == OLx_CHNT_SINGLEENDED)
		m_numchansMAX = m_Acq32_ADC.GetSSCaps(OLSSC_MAXSECHANS);
	else
		m_numchansMAX = m_Acq32_ADC.GetSSCaps(OLSSC_MAXDICHANS);

	// data encoding (binary or offset encoding)
	pWFormat->mode_encoding = (int)m_Acq32_ADC.GetEncoding();
	if (pWFormat->mode_encoding == OLx_ENC_BINARY)
		pWFormat->binzero = pWFormat->binspan / 2 + 1;
	else if (pWFormat->mode_encoding == OLx_ENC_2SCOMP)
		pWFormat->binzero = 0;

	// load infos concerning frequency, dma chans, programmable gains
	m_freqmax = m_Acq32_ADC.GetSSCapsEx(OLSSCE_MAXTHROUGHPUT);	// m_dfMaxThroughput

	// TODO tell sourceview here under which format are data
	// TODO save format of data into temp document
	// float volts = (float) ((pWFormat->fullscale_Volts) 
	//				/(pWFormat->binspan) * value  -pWFormat->fullscale_Volts/2);
	// TODO: update max min of chan 1 with gain && instrumental gain
	//UpdateChanLegends(0);
	//UpdateHorizontalRulerBar();
	//UpdateVerticalRulerBar();

	return TRUE;
}

BOOL CADContView::ADC_InitSubSystem()
{
	try
	{
		ASSERT(m_Acq32_ADC.GetHDass() != NULL);

		// store all values within global parameters array
		CWaveFormat* poptions_acqdatawaveFormat = &(m_pADC_options->waveFormat);

		// Set up the ADC - no wrap so we can get buffer reused	
		m_Acq32_ADC.SetDataFlow(OLx_DF_CONTINUOUS);
		m_Acq32_ADC.SetWrapMode(OLx_WRP_NONE);
		m_Acq32_ADC.SetDmaUsage((short)m_Acq32_ADC.GetSSCaps(OLSSC_NUMDMACHANS));
		m_Acq32_ADC.SetClockSource(OLx_CLK_INTERNAL);

		// set trigger mode
		int trig = poptions_acqdatawaveFormat->trig_mode;
		if (trig > OLx_TRG_EXTRA) 		trig = 0;
		m_Acq32_ADC.SetTrigger(trig);

		// number of channels
		if (m_pADC_options->bChannelType == OLx_CHNT_SINGLEENDED && m_Acq32_ADC.GetSSCaps(OLSSC_SUP_SINGLEENDED) == NULL)
			m_pADC_options->bChannelType = OLx_CHNT_DIFFERENTIAL;
		if (m_pADC_options->bChannelType == OLx_CHNT_DIFFERENTIAL && m_Acq32_ADC.GetSSCaps(OLSSC_SUP_DIFFERENTIAL) == NULL)
			m_pADC_options->bChannelType = OLx_CHNT_SINGLEENDED;
		m_Acq32_ADC.SetChannelType(m_pADC_options->bChannelType);
		if (m_pADC_options->bChannelType == OLx_CHNT_SINGLEENDED)
			m_numchansMAX = m_Acq32_ADC.GetSSCaps(OLSSC_MAXSECHANS);
		else
			m_numchansMAX = m_Acq32_ADC.GetSSCaps(OLSSC_MAXDICHANS);
		// limit scan_count to m_numchansMAX -
		// this limits the nb of data acquisition channels to max-1 if one wants to use the additional I/O input "pseudo"channel
		// so far, it seems acceptable...
		if (poptions_acqdatawaveFormat->scan_count > m_numchansMAX)
			poptions_acqdatawaveFormat->scan_count = m_numchansMAX;

		// set frequency to value requested, set frequency and get the value returned
		double clockrate = double (poptions_acqdatawaveFormat->chrate) * double(poptions_acqdatawaveFormat->scan_count);
		m_Acq32_ADC.SetFrequency(clockrate);			// set sampling frequency (total throughput)
		clockrate = m_Acq32_ADC.GetFrequency();
		poptions_acqdatawaveFormat->chrate = (float)clockrate / poptions_acqdatawaveFormat->scan_count;

		// update channel list (chan & gain)

		m_Acq32_ADC.SetListSize(poptions_acqdatawaveFormat->scan_count);
		for (int i = 0; i < poptions_acqdatawaveFormat->scan_count; i++)
		{
			// transfer data from CWaveChan to chanlist of the A/D subsystem
			CWaveChan* pChannel = (m_pADC_options->chanArray).Get_p_channel(i);
			if (pChannel->am_adchannel > m_numchansMAX - 1 && pChannel->am_adchannel != 16)
				pChannel->am_adchannel = m_numchansMAX - 1;
			m_Acq32_ADC.SetChannelList(i, pChannel->am_adchannel);
			m_Acq32_ADC.SetGainList(i, pChannel->am_gainAD);
			double dGain = m_Acq32_ADC.GetGainList(i);
			pChannel->am_gainAD = (short)dGain;
			// compute dependent parameters
			pChannel->am_gainamplifier = double(pChannel->am_gainheadstage) * double(pChannel->am_gainpre) * double(pChannel->am_gainpost);
			pChannel->am_gaintotal = pChannel->am_gainamplifier * double(pChannel->am_gainAD);
			pChannel->am_resolutionV = double(poptions_acqdatawaveFormat->fullscale_Volts) / pChannel->am_gaintotal / double(poptions_acqdatawaveFormat->binspan);
		}

		// pass parameters to the board and check if errors
		m_Acq32_ADC.ClearError();
		m_Acq32_ADC.Config();
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	}

	// AD system is changed:  update AD buffers & change encoding: it is changed on-the-fly in the transfer loop
	*(m_inputDataFile.GetpWavechanArray()) = m_pADC_options->chanArray;
	*(m_inputDataFile.GetpWaveFormat()) = m_pADC_options->waveFormat;

	ADC_DeclareBuffers();
	return TRUE;
}

void CADContView::ADC_DeclareBuffers()
{
	// close data buffers
	ADC_DeleteBuffers();

	// make sure that buffer length contains at least nacq chans
	CWaveFormat* pWFormat = &(m_pADC_options->waveFormat); // get pointer to m_pADC_options wave format
	if (pWFormat->buffersize < pWFormat->scan_count * m_pADC_options->iundersample)
		pWFormat->buffersize = pWFormat->scan_count * m_pADC_options->iundersample;

	// define buffer length
	m_sweepduration = m_pADC_options->sweepduration;
	m_chsweeplength = (long)(m_sweepduration * pWFormat->chrate / (float)m_pADC_options->iundersample);
	m_ADC_chbuflen = m_chsweeplength * m_pADC_options->iundersample / pWFormat->bufferNitems;
	m_ADC_buflen = m_ADC_chbuflen * pWFormat->scan_count;

	// declare buffers to DT
	ECODE ecode;
	for (int i = 0; i < pWFormat->bufferNitems; i++)
	{
		ecode = olDmAllocBuffer(0, m_ADC_buflen, &m_ADC_bufhandle);
		ecode = OLNOERROR;
		if ((ecode == OLNOERROR) && (m_ADC_bufhandle != NULL))
			m_Acq32_ADC.SetQueue((long)m_ADC_bufhandle); // but buffer onto Ready queue
	}

	// set sweep length to the nb of data buffers
	(m_inputDataFile.GetpWaveFormat())->sample_count = m_chsweeplength * (long)pWFormat->scan_count;	// ?
	m_inputDataFile.AdjustBUF(m_chsweeplength);
	*(m_inputDataFile.GetpWaveFormat()) = *pWFormat;	// save settings into data file	

														// update display length (and also the text - abcissa)	
	m_ADsourceView.AttachDataFile(&m_inputDataFile);
	m_ADsourceView.ResizeChannels(0, m_chsweeplength);
	if (m_ADsourceView.GetChanlistSize() != pWFormat->scan_count)
	{
		m_ADsourceView.RemoveAllChanlistItems();
		for (int j = 0; j < pWFormat->scan_count; j++)
		{
			m_ADsourceView.AddChanlistItem(j, 0);
		}
	}

	// adapt source view 
	int iextent = MulDiv(pWFormat->binspan, 12, 10);
	if (m_pADC_options->izoomCursel != 0)
		iextent = m_pADC_options->izoomCursel;
	int ioffset = 0;

	for (int i = 0; i < pWFormat->scan_count; i++)
	{
		CChanlistItem* pD = m_ADsourceView.GetChanlistItem(i);
		pD->SetYzero(ioffset);
		pD->SetYextent(iextent);
		pD->SetColor(i);
		float docVoltsperb;
		m_inputDataFile.GetWBVoltsperBin(i, &docVoltsperb);
		pD->SetDataBinFormat(pWFormat->binzero, pWFormat->binspan);
		pD->SetDataVoltsFormat(docVoltsperb, pWFormat->fullscale_Volts);
	}
	m_ADsourceView.Invalidate();
	UpdateData(FALSE);
}

void CADContView::ADC_DeleteBuffers()
{
	try {
		if (m_Acq32_ADC.GetHDass() == NULL)
			return;
		m_Acq32_ADC.Flush();	// clean
		HBUF hBuf = NULL;			// handle to buffer
		do {				// loop until all buffers are removed
			hBuf = (HBUF)m_Acq32_ADC.GetQueue();
			if (hBuf != NULL)
				if (olDmFreeBuffer(hBuf) != OLNOERROR)
					AfxMessageBox(_T("Error Freeing Buffer"));
		} while (hBuf != NULL);
		m_ADC_bufhandle = hBuf;
	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
	}
}

void CADContView::ADC_StopAndLiberateBuffers()
{
	try {
		m_Acq32_ADC.Stop();
		m_Acq32_ADC.Flush();							// flush all buffers to Done Queue
		HBUF hBuf;
		do {
			hBuf = (HBUF)m_Acq32_ADC.GetQueue();
			if (hBuf != NULL) m_Acq32_ADC.SetQueue((long)hBuf);
		} while (hBuf != NULL);
		m_ADsourceView.ADdisplayStop();
		m_bchanged = TRUE;
	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
	}
	m_ADC_inprogress = FALSE;
}

BOOL CADContView::DAC_OpenSubSystem(CString cardName)
{
	try
	{
		m_Acq32_DAC.SetBoard(cardName);
		if (m_Acq32_DAC.GetDevCaps(OLSS_DA) == 0)
			return FALSE;

		m_Acq32_DAC.SetSubSysType(OLSS_DA);
		int nDA = m_Acq32_DAC.GetDevCaps(OLDC_DAELEMENTS);
		if (nDA < 1)
			return FALSE;
		m_Acq32_DAC.SetSubSysElement(0);
		ASSERT(m_Acq32_DAC.GetHDass() != NULL),
			m_bsimultaneousStartDA = m_Acq32_DAC.GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);

	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CADContView::DAC_ClearAllOutputs()
{
	try
	{
		if (m_Acq32_DAC.GetHDass() == NULL)
			return FALSE;

		if (m_Acq32_DAC.GetSSCaps(OLSSC_SUP_SINGLEVALUE) == FALSE)
		{
			AfxMessageBox(_T("D/A SubSystem cannot run in single value mode"));
			return FALSE;
		}
		m_Acq32_DAC.ClearError();
		m_Acq32_DAC.SetDataFlow(OLx_DF_SINGLEVALUE);
		long OutValue = 0;
		if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
			OutValue = (WORD)((OutValue ^ m_DACmsbit) & m_DAClRes);

		m_Acq32_DAC.Config();
		int nchansmax = m_Acq32_DAC.GetSSCaps(OLSSC_NUMCHANNELS) - 1;
		for (int i = 0; i < 2; i++)
		{
			m_Acq32_DAC.PutSingleValue(i, 1.0, OutValue);
		}
	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CADContView::DAC_InitSubSystem()
{
	try
	{
		if (m_Acq32_DAC.GetHDass() == NULL)
			return FALSE;

		// Set up the ADC - multiple wrap so we can get buffer reused	
		m_Acq32_DAC.SetDataFlow(OLx_DF_CONTINUOUS);
		m_Acq32_DAC.SetWrapMode(OLx_WRP_NONE);
		m_Acq32_DAC.SetDmaUsage((short)m_Acq32_DAC.GetSSCaps(OLSSC_NUMDMACHANS));

		// set clock the same as for A/D
		m_Acq32_DAC.SetClockSource(OLx_CLK_INTERNAL);
		double clockrate = m_pADC_options->waveFormat.chrate;
		m_Acq32_DAC.SetFrequency(clockrate);		// set sampling frequency (total throughput)

		// set trigger mode
		int trig = m_pADC_options->waveFormat.trig_mode;
		if (trig > OLx_TRG_EXTRA)
			trig = 0;
		m_Acq32_DAC.SetTrigger(trig);

		DAC_SetChannelList();
		double resolutionfactor = pow(2.0, m_Acq32_DAC.GetResolution());
		m_DACmsbit = long(pow(2.0, (m_Acq32_DAC.GetResolution() - 1.)));
		m_DAClRes = long(resolutionfactor - 1.);

		for (int i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
		{
			OUTPUTPARMS* pParms = &m_pDAC_options->outputparms_array.GetAt(i);
			//DAC_MSequence(TRUE, pParms);
			if (pParms->bDigital)
				continue;
			double delta = double(m_Acq32_DAC.GetMaxRange()) - double(m_Acq32_DAC.GetMinRange());
			pParms->ampUp = pParms->dAmplitudeMaxV * resolutionfactor / delta;
			pParms->ampLow = pParms->dAmplitudeMinV * resolutionfactor / delta;
		}

		// pass parameters to the board and check if errors
		m_Acq32_DAC.ClearError();
		m_Acq32_DAC.Config();
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

void CADContView::DAC_SetChannelList()
{
	int nanalogOutputs = 0;
	int ndigitalOutputs = 0;

	for (int i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
	{
		OUTPUTPARMS* pParms = &m_pDAC_options->outputparms_array.GetAt(i);
		if (!pParms->bON)
			continue;
		if (!pParms->bDigital)
			nanalogOutputs++;
		else
			ndigitalOutputs++;
	}
	m_DACdigitalchannel = 0;
	int nchans = nanalogOutputs;
	nchans += (ndigitalOutputs > 0);

	try
	{
		// number of D/A channels
		int nchansmax = m_Acq32_DAC.GetSSCaps(OLSSC_NUMCHANNELS);
		ASSERT(nchans <= nchansmax);
		m_Acq32_DAC.SetListSize(nchans);

		if (nanalogOutputs)
		{
			for (int i = 0; i < nchansmax; i++)
			{
				if (nanalogOutputs > 0)
				{
					m_Acq32_DAC.SetChannelList(i, i);
					nanalogOutputs--;
					m_DACdigitalchannel++;
				}
			}
		}

		if (ndigitalOutputs)
		{
			m_Acq32_DAC.SetChannelList(m_DACdigitalchannel, nchansmax - 1);
		}

		m_DAClistsize = m_Acq32_DAC.GetListSize();
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
	}
}

void CADContView::DAC_DeleteBuffers()
{
	try {
		if (m_Acq32_DAC.GetHDass() == NULL)
			return;

		m_Acq32_DAC.Flush();	// clean
		HBUF hBuf;			// handle to buffer
		do {				// loop until all buffers are removed
			hBuf = (HBUF)m_Acq32_DAC.GetQueue();
			if (hBuf != NULL)
				if (olDmFreeBuffer(hBuf) != OLNOERROR)
					AfxMessageBox(_T("Error Freeing Buffer"));
		} while (hBuf != NULL);
		m_DAC_bufhandle = hBuf;
	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
	}
}

void CADContView::DAC_DeclareAndFillBuffers()
{
	// close data buffers
	DAC_DeleteBuffers();

	// get current parms from A/D conversion
	CWaveFormat* pWFormat = &(m_pADC_options->waveFormat);
	m_DAC_frequency = pWFormat->chrate;

	// define buffer length
	float sweepduration = m_pADC_options->sweepduration;
	long chsweeplength = (long)(sweepduration * pWFormat->chrate);
	int nbuffers = pWFormat->bufferNitems;
	m_DAC_chbuflen = chsweeplength / nbuffers;
	m_DAC_buflen = m_DAC_chbuflen * m_DAClistsize;

	for (int i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
	{
		OUTPUTPARMS* outputparms_array = &(m_pDAC_options->outputparms_array.GetAt(i));
		outputparms_array->lastphase = 0;
		outputparms_array->lastamp = 0;
	}

	// declare buffers to DT
	m_DAC_nBuffersFilledSinceStart = 0;
	ECODE ecode;
	for (int i = 0; i <= nbuffers; i++)
	{
		ecode = olDmAllocBuffer(0, m_DAC_buflen, &m_DAC_bufhandle);
		short* pDTbuf;
		ecode = olDmGetBufferPtr(m_DAC_bufhandle, (void**)&pDTbuf);
		DAC_FillBuffer(pDTbuf);
		if ((ecode == OLNOERROR) && (m_DAC_bufhandle != NULL))
		{
			m_Acq32_DAC.SetQueue((long)m_DAC_bufhandle);
		}
	}
}

void CADContView::DAC_ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, int chan)
{
	for (int i = chan; i < m_DAC_buflen; i += m_DAClistsize)
		*(pDTbuf + i) = (WORD)((*(pDTbuf + i) ^ m_DACmsbit) & m_DAClRes);
}

void CADContView::DAC_FillBufferWith_SINUSOID(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	phase = outputparms_array->lastphase;
	double	Freq = outputparms_array->dFrequency / m_DAC_frequency;
	double	amp = (outputparms_array->ampUp - outputparms_array->ampLow) / 2;
	double offset = (outputparms_array->ampUp + outputparms_array->ampLow) / 2;
	int nchans = m_DAClistsize;

	double pi2 = 3.1415927 * 2;
	Freq = Freq * pi2;
	for (int i = chan; i < m_DAC_buflen; i += nchans)
	{
		*(pDTbuf + i) = (short)(cos(phase) * amp + offset);
		// clip value
		if (*(pDTbuf + i) > m_DACmsbit)
			*(pDTbuf + i) = (short)(m_DACmsbit - 1);
		phase += Freq;
		if (phase > pi2)
			phase -= pi2;
	}

	if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
	outputparms_array->lastamp = amp;
}

void CADContView::DAC_FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	phase = outputparms_array->lastphase;
	double	Freq = outputparms_array->dFrequency / m_DAC_frequency;
	double	ampUp = outputparms_array->ampUp;
	double	ampLow = outputparms_array->ampLow;
	double	amp = ampUp;
	int		nchans = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += nchans)
	{
		if (phase < 0)
			amp = ampUp;
		else
			amp = ampLow;
		*(pDTbuf + i) = (WORD) amp;
		phase += Freq;
		if (phase > 0.5)
			phase -= 1;
	}

	if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
}

void CADContView::DAC_FillBufferWith_TRIANGLE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	phase = outputparms_array->lastphase;
	double	Freq = outputparms_array->dFrequency / m_DAC_frequency;
	double	amp = outputparms_array->ampUp;
	int nchans = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += nchans)
	{
		*(pDTbuf + i) = (WORD)(2 * phase * amp);
		// clip value
		if (*(pDTbuf + i) >= m_DACmsbit)
			*(pDTbuf + i) = (short)(m_DACmsbit - 1);
		phase = phase + 2 * Freq;
		if (phase > 0.5)
		{
			phase -= 1;
			amp--;
		}
	}

	if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
	outputparms_array->lastamp = amp;
}

void CADContView::DAC_FillBufferWith_RAMP(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	amp = outputparms_array->ampUp;
	int nchans = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += nchans)
		*(pDTbuf + i) = (WORD) amp;

	if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = amp;
}

void CADContView::DAC_FillBufferWith_CONSTANT(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double delta = double(m_Acq32_DAC.GetMaxRange()) - double(m_Acq32_DAC.GetMinRange());
	double	amp = outputparms_array->value * pow(2.0, m_Acq32_DAC.GetResolution()) / delta;
	int nchans = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += nchans)
		*(pDTbuf + i) = (WORD) amp;

	if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = amp;
}

void CADContView::DAC_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	ampUp = outputparms_array->ampUp;
	double  ampLow = outputparms_array->ampLow;
	int		nchans = m_DAClistsize;

	CIntervalsAndWordsSeries* pstim = &outputparms_array->sti;
	double	chFreqRatio = m_DAC_frequency / pstim->chrate;
	long	buffer_start = m_DAC_nBuffersFilledSinceStart * m_DAC_chbuflen;
	long	buffer_end = (m_DAC_nBuffersFilledSinceStart + 1) * m_DAC_chbuflen;
	long	buffer_ii = buffer_start;
	int		interval = 0;

	BOOL	wamp = FALSE;
	long	stim_end = 0;

	// find end = first interval after buffer_end; find start 
	for (interval = 0; interval < pstim->GetSize(); interval++)
	{
		stim_end = (long)(pstim->GetIntervalPointAt(interval).ii * chFreqRatio);
		if (stim_end > buffer_start)
			break;
		wamp = pstim->GetIntervalPointAt(interval).w;
	}
	double amp = ampUp * wamp + ampLow * !wamp;
	WORD wout = (WORD) amp;
	if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
		wout = (WORD)(wout ^ m_DACmsbit) & m_DAClRes;

	// fill buffer
	for (int i = chan; i < m_DAC_buflen; i += nchans, buffer_ii++)
	{
		*(pDTbuf + i) = wout;

		if ((interval < pstim->GetSize()) && buffer_ii >= stim_end)
		{
			interval++;
			wamp = FALSE;
			if (interval < pstim->GetSize())
				stim_end = (long)(pstim->GetIntervalPointAt(interval).ii * chFreqRatio);
			wamp = pstim->GetIntervalPointAt(interval - 1).w;
			amp = ampUp * wamp + ampLow * !wamp;
			wout = (WORD) amp;
			if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
				wout = (WORD)(wout ^ m_DACmsbit) & m_DAClRes;
		}
	}
	outputparms_array->lastamp = amp;
}

void CADContView::DAC_MSequence(BOOL bStart, OUTPUTPARMS* outputparms_array)
{
	outputparms_array->count--;
	if (outputparms_array->count == 0) {
		outputparms_array->count = outputparms_array->mseq_iRatio + 1;
		if (bStart) {
			outputparms_array->num = outputparms_array->mseq_iSeed;
			outputparms_array->bit1 = 1;
			outputparms_array->bit33 = 0;
			outputparms_array->count = 1;
		}
		UINT bit13 = ((outputparms_array->num & 0x1000) != 0);
		outputparms_array->bit1 = (bit13 == outputparms_array->bit33) ? 0 : 1;
		outputparms_array->bit33 = ((outputparms_array->num & 0x80000000) != 0);
		outputparms_array->num = (outputparms_array->num << 1) + outputparms_array->bit1;
	}
}

void CADContView::DAC_FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	int DAClistsize = m_DAClistsize;
	double x = 0;
	int mseqOffsetDelay = outputparms_array->mseq_iDelay;

	for (int i = chan; i < m_DAC_buflen; i += DAClistsize)
	{
		x = 0;
		if (outputparms_array->mseq_iDelay > 0)
			outputparms_array->mseq_iDelay--;
		else
		{
			x = outputparms_array->ampLow;
			if (outputparms_array->mseq_iDelay == 0) {
				DAC_MSequence(FALSE, outputparms_array);
				x = (outputparms_array->bit1 * outputparms_array->ampUp) + (!outputparms_array->bit1 * outputparms_array->ampLow);
			}
		}
		*(pDTbuf + i) = (WORD)x;
	}

	if (m_Acq32_DAC.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = x;
}

void CADContView::DAC_Dig_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	WORD	ampLow = 0;
	WORD	ampUp = 1;
	ampUp = ampUp << outputparms_array->iChan;
	int		nchans = m_DAClistsize;

	CIntervalsAndWordsSeries* pstim = &outputparms_array->sti;
	double	chFreqRatio = m_DAC_frequency / pstim->chrate;
	long	buffer_start = m_DAC_nBuffersFilledSinceStart * m_DAC_chbuflen;
	long	buffer_end = (m_DAC_nBuffersFilledSinceStart + 1) * m_DAC_chbuflen;
	long	buffer_ii = buffer_start;
	int		interval = 0;

	BOOL	wamp = 0;
	long	stim_end = 0;

	// find end = first interval after buffer_end; find start 
	for (interval = 0; interval < pstim->GetSize(); interval++)
	{
		stim_end = (long)(pstim->GetIntervalPointAt(interval).ii * chFreqRatio);
		if (stim_end > buffer_start)
			break;
		wamp = pstim->GetIntervalPointAt(interval).w;
	}
	WORD wout = ampLow;
	if (wamp > 0)
		wout = ampUp;

	// fill buffer
	for (int i = chan; i < m_DAC_buflen; i += nchans, buffer_ii++)
	{
		if (m_DACdigitalfirst == 0)
			*(pDTbuf + i) = wout;
		else
			*(pDTbuf + i) |= wout;

		if ((interval < pstim->GetSize()) && buffer_ii >= stim_end)
		{
			interval++;
			if (interval < pstim->GetSize())
				stim_end = (long)(pstim->GetIntervalPointAt(interval).ii * chFreqRatio);
			wamp = pstim->GetIntervalPointAt(interval - 1).w;
			if (wamp > 0)
				wout = ampUp;
			else
				wout = ampLow;
		}
	}
}

void CADContView::DAC_Dig_FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	phase = outputparms_array->lastphase;
	WORD	amp = 0;
	WORD	ampUp = 1;
	ampUp = ampUp << outputparms_array->iChan;
	WORD	ampLow = 0;
	double	Freq = outputparms_array->dFrequency / m_DAC_frequency;
	int nchans = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += nchans)
	{
		if (phase < 0)
			amp = ampUp;
		else
			amp = ampLow;

		if (m_DACdigitalfirst == 0)
			*(pDTbuf + i) = amp;
		else
			*(pDTbuf + i) |= amp;

		phase += Freq;
		if (phase > 0.5)
			phase -= 1;
	}
	outputparms_array->lastphase = phase;
}

void CADContView::DAC_Dig_FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	WORD	ampLow = 0;
	WORD	ampUp = 1;
	ampUp = ampUp << outputparms_array->iChan;
	int DAClistsize = m_DAClistsize;
	double x = 0;
	int mseqOffsetDelay = outputparms_array->mseq_iDelay;

	for (int i = chan; i < m_DAC_buflen; i += DAClistsize)
	{
		x = 0;
		if (outputparms_array->mseq_iDelay > 0)
			outputparms_array->mseq_iDelay--;
		else
		{
			x = outputparms_array->ampLow;
			if (outputparms_array->mseq_iDelay == 0) {
				DAC_MSequence(FALSE, outputparms_array);
				x = double(outputparms_array->bit1) * double(ampUp) + double(!outputparms_array->bit1 )* double(ampLow);
			}
		}
		if (m_DACdigitalfirst == 0)
			*(pDTbuf + i) = WORD(x);
		else
			*(pDTbuf + i) |= WORD(x);
	}
	outputparms_array->lastamp = x;
}

void CADContView::DAC_FillBuffer(short* pDTbuf)
{
	int janalog = 0;
	m_DACdigitalfirst = 0;
	for (int i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
	{
		OUTPUTPARMS* pParms = &m_pDAC_options->outputparms_array.GetAt(i);
		if (!pParms->bON)
			continue;

		if (!pParms->bDigital)
		{
			switch (pParms->iWaveform)
			{
			case DA_SINEWAVE:
				DAC_FillBufferWith_SINUSOID(pDTbuf, janalog, pParms);
				break;
			case DA_SQUAREWAVE:
				DAC_FillBufferWith_SQUARE(pDTbuf, janalog, pParms);
				break;
			case DA_TRIANGLEWAVE:
				DAC_FillBufferWith_TRIANGLE(pDTbuf, janalog, pParms);
				break;
			case DA_LINEWAVE:
				DAC_FillBufferWith_RAMP(pDTbuf, janalog, pParms);
				break;
			case DA_SEQUENCEWAVE:
				DAC_FillBufferWith_ONOFFSeq(pDTbuf, janalog, pParms);
				break;
			case DA_MSEQWAVE:
				DAC_FillBufferWith_MSEQ(pDTbuf, janalog, pParms);
				break;
			case DA_CONSTANT:
			default:
				DAC_FillBufferWith_CONSTANT(pDTbuf, janalog, pParms);
				break;
			}
			janalog++;
		}
		else
		{
			switch (pParms->iWaveform)
			{
			case DA_SQUAREWAVE:
				DAC_Dig_FillBufferWith_SQUARE(pDTbuf, m_DACdigitalchannel, pParms);
				break;
			case DA_SEQUENCEWAVE:
				DAC_Dig_FillBufferWith_ONOFFSeq(pDTbuf, m_DACdigitalchannel, pParms);
				break;
			case DA_MSEQWAVE:
				DAC_Dig_FillBufferWith_MSEQ(pDTbuf, m_DACdigitalchannel, pParms);
				break;
			case DA_ONE:
				break;
			case DA_ZERO:
			default:
				break;
			}
			m_DACdigitalfirst++;
		}
	}
	m_DAC_nBuffersFilledSinceStart++;
}

void CADContView::DAC_StopAndLiberateBuffers()
{
	try {
		m_Acq32_DAC.Stop();
		m_Acq32_DAC.Flush();	// flush all buffers to Done Queue
		HBUF hBuf;
		do {
			hBuf = (HBUF)m_Acq32_DAC.GetQueue();
			if (hBuf != NULL) {
				ECODE ecode = olDmFreeBuffer(hBuf);
				if (ecode != OLNOERROR)
					AfxMessageBox(_T("Could not free Buffer"));
			}
		} while (hBuf != NULL);

		DAC_ClearAllOutputs();
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
	}
	m_DAC_inprogress = FALSE;
}

void CADContView::StopAcquisition(BOOL bDisplayErrorMsg)
{
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
	if (m_ADC_inprogress)
		ADC_StopAndLiberateBuffers();

	// stop DA, liberate buffers
	if (m_DAC_inprogress && m_bStartOutPutMode == 0)
		DAC_StopAndLiberateBuffers();

	// close file and update display
	if (m_bFileOpen)
	{
		SaveAndCloseFile();
		// update view data	
		long lsizeDOCchan = m_inputDataFile.GetDOCchanLength();
		m_ADsourceView.AttachDataFile(&m_inputDataFile);
		m_ADsourceView.ResizeChannels(m_ADsourceView.GetRectWidth(), lsizeDOCchan);
		m_ADsourceView.GetDataFromDoc(0, lsizeDOCchan);
	}
}

void CADContView::SaveAndCloseFile()
{
	m_inputDataFile.AcqDoc_DataAppendStop();
	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();

	// if burst data acquisition mode ------------------------------------
	if (m_bhidesubsequent)
	{
		if (pWFormat->sample_count > 1) // make sure real data have been acquired
			m_csNameArray.Add(m_szFileName);
		else
			m_inputDataFile.AcqDeleteFile();
		return;
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
			m_inputDataFile.AcqDeleteFile();
			return;
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
				return;
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
		UpdateStartStop(FALSE);
		return FALSE;
	}

	if (!ADC_InitSubSystem())
		return FALSE;

	if (m_bStartOutPutMode == 0 && DAC_InitSubSystem())
		DAC_DeclareAndFillBuffers();

	// start AD display
	m_chsweep1 = 0;
	m_chsweep2 = -1;
	m_ADsourceView.ADdisplayStart(m_chsweeplength);
	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();
	pWFormat->sample_count = 0;							// no samples yet
	pWFormat->chrate = pWFormat->chrate / m_pADC_options->iundersample;
	m_fclockrate = pWFormat->chrate * pWFormat->scan_count;
	pWFormat->acqtime = CTime::GetCurrentTime();

	// data format
	pWFormat->binspan = (m_pADC_options->waveFormat).binspan;
	pWFormat->fullscale_Volts = (m_pADC_options->waveFormat).fullscale_Volts;
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
			UpdateStartStop(FALSE);
			return FALSE;
		}
	}

	// starting mode of A/D if no simultaneous list
	if (!m_bSimultaneousStart || m_bStartOutPutMode != 0)
	{
		try
		{
			m_Acq32_ADC.Config();
			m_Acq32_ADC.Start();
			m_ADC_inprogress = TRUE;
			m_DAC_inprogress = FALSE;
			if (m_bStartOutPutMode == 0)
			{
				m_Acq32_DAC.Config();
				m_Acq32_DAC.Start();
				m_DAC_inprogress = TRUE;
			}
		}
		catch (COleDispatchException* e)
		{
			CString myError;
			myError.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); myError += e->m_strDescription;
			AfxMessageBox(myError);
			e->Delete();
		}
	}

	// starting A/D when simultaneous list 
	else
	{
		BOOL retval;
		HSSLIST hSSlist;
		CHAR errstr[255];
		ECODE ecode;

		// create simultaneous starting list
		ecode = olDaGetSSList(&hSSlist);
		olDaGetErrorString(ecode, errstr, 255);

		m_Acq32_ADC.Config();
		m_Acq32_DAC.Config();

		// DA system
		ecode = olDaPutDassToSSList(hSSlist, (HDASS)m_Acq32_DAC.GetHDass());
		if (ecode != OLNOERROR)
		{
			retval = ecode;
			ecode = olDaReleaseSSList(hSSlist);
			return retval;
		}

		// AD system
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

		m_ADC_inprogress = TRUE;
		m_DAC_inprogress = TRUE;
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
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)m_hBias);
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)m_hZoom);

	BOOL b32BitIcons = afxGlobalData.m_nBitsPerPixel >= 16;
	m_btnStartStop.SetImage(b32BitIcons ? IDB_CHECK32 : IDB_CHECK);
	m_btnStartStop.SetCheckedImage(b32BitIcons ? IDB_CHECKNO32 : IDB_CHECKNO);
	m_btnStartStop.EnableWindowsTheming(false); // true
	m_btnStartStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D; //BUTTONSTYLE_SEMIFLAT;

	// scrollbar
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	// CFormView init CFile
	CdbWaveApp* pApp = (CdbWaveApp*)AfxGetApp();
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
	m_inputDataFile.OnNewDocument();							// create a file to receive incoming data (A/D)
	*(m_inputDataFile.GetpWaveFormat()) = m_pADC_options->waveFormat;	// copy data formats into this file
	m_pADC_options->chanArray.ChanArray_setSize(m_pADC_options->waveFormat.scan_count);
	*(m_inputDataFile.GetpWavechanArray()) = m_pADC_options->chanArray;
	m_ADsourceView.AttachDataFile(&m_inputDataFile);		// prepare display area

	pApp->m_bADcardFound = FindDTOpenLayersBoards();			// open DT Open Layers board
	if (pApp->m_bADcardFound)
	{
		ADC_InitSubSystem();									// connect A/D DT OpenLayer subsystem
		InitCyberAmp();											// control cyberamplifier
		DAC_InitSubSystem();									// connect D/A DT OpenLayers subsystem
		DAC_ClearAllOutputs();
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
		if (m_ADC_inprogress)
		{
			StopAcquisition(TRUE);
			UpdateStartStop(FALSE);
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
			if ((m_inputDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
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
	UpdateStartStop(bADStart);
}

void CADContView::UpdateStartStop(BOOL bStart)
{
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
		ASSERT(m_ADC_inprogress == FALSE);
	}
	m_btnStartStop.SetCheck(m_ADC_inprogress);
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
			CString cs = _T("The Next available file name is: ") + csfilename;
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
	if (m_ADC_inprogress)
	{
		StopAcquisition(TRUE);
		UpdateStartStop(FALSE);
	}
	CDlgADInputParms dlg;

	// init dialog data
	dlg.m_pwFormat = &(m_pADC_options->waveFormat);
	dlg.m_pchArray = &(m_pADC_options->chanArray);  
	dlg.m_numchansMAXDI = m_Acq32_ADC.GetSSCaps(OLSSC_MAXDICHANS);
	dlg.m_numchansMAXSE = m_Acq32_ADC.GetSSCaps(OLSSC_MAXSECHANS);
	dlg.m_bchantype = m_pADC_options->bChannelType;
	dlg.m_bchainDialog = TRUE;
	dlg.m_bcommandAmplifier = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		// make sure that buffer size is a multiple of the nb of chans
		m_pADC_options->bChannelType = dlg.m_bchantype;
		ADC_InitSubSystem();
		UpdateData(FALSE);
		UpdateChanLegends(0);

		if (dlg.m_postmessage != NULL)
			ChainDialog(dlg.m_postmessage);
	}
}

void CADContView::OnHardwareAdintervals()
{
	if (m_ADC_inprogress)
	{
		StopAcquisition(TRUE);
		UpdateStartStop(FALSE);
	}
	ADIntervalsDlg dlg;
	// init dialog data 
	CWaveFormat* pWFormat = &(m_pADC_options->waveFormat);
	dlg.m_pwaveFormat = pWFormat;
	dlg.m_ratemin = 1.0f;
	dlg.m_ratemax = (float)(m_freqmax / pWFormat->scan_count);
	dlg.m_bufferWsizemax = (UINT)65536 * 4;
	dlg.m_undersamplefactor = m_pADC_options->iundersample;
	dlg.m_baudiblesound = m_pADC_options->baudiblesound;
	dlg.m_sweepduration = m_sweepduration;
	dlg.m_bchainDialog = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_pADC_options->iundersample = dlg.m_undersamplefactor;
		m_pADC_options->baudiblesound = dlg.m_baudiblesound;
		m_sweepduration = dlg.m_sweepduration;
		m_pADC_options->sweepduration = m_sweepduration;
		ADC_InitSubSystem();
		UpdateData(FALSE);

		if (dlg.m_postmessage != NULL)
			ChainDialog(dlg.m_postmessage);
	}
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
		break;
	}
	PostMessage(WM_COMMAND, menuID, NULL);
	return;
}

void CADContView::OnTriggerError_ADC()
{
	StopAcquisition(TRUE);
	UpdateStartStop(FALSE);
	AfxMessageBox(IDS_ACQDATA_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void CADContView::OnQueueDone_ADC()
{
	StopAcquisition(TRUE);
	UpdateStartStop(FALSE);
	AfxMessageBox(IDS_ACQDATA_TOOFAST);
}

void CADContView::OnOverrunError_ADC()
{
	StopAcquisition(TRUE);
	UpdateStartStop(FALSE);
	AfxMessageBox(IDS_ACQDATA_OVERRUN);
}

void CADContView::OnOverrunError_DAC()
{
	DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_OVERRUN);
}

void CADContView::OnQueueDone_DAC()
{
	DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TOOFAST);
}

void CADContView::OnTriggerError_DAC()
{
	DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void CADContView::OnBufferDone_ADC()
{
	// get buffer off done list	
	m_ADC_bufhandle = (HBUF)m_Acq32_ADC.GetQueue();
	if (m_ADC_bufhandle == NULL)
		return;

	// get pointer to buffer
	short* pDTbuf;
	m_ecode = olDmGetBufferPtr(m_ADC_bufhandle, (void**)&pDTbuf);
	if (m_ecode == OLNOERROR)
	{
		// update length of data acquired
		ADC_Transfer(pDTbuf);										// transfer data into intermediary buffer

		CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();	// pointer to data descriptor
		pWFormat->sample_count += m_bytesweepRefresh / 2;				// update total sample count
		float duration = pWFormat->sample_count / m_fclockrate;

		short* pdataBuf = m_inputDataFile.GetpRawDataBUF();
		pdataBuf += (m_chsweep1 * pWFormat->scan_count);

		// first thing to do: save data to file
		if (pWFormat->bADwritetofile)								// write buffer to file
		{
			BOOL flag = m_inputDataFile.AcqDoc_DataAppend(pdataBuf, m_bytesweepRefresh);
			ASSERT(flag);
			// end of acquisition
			if (duration >= pWFormat->duration)
			{
				StopAcquisition(TRUE);
				if (m_bhidesubsequent)
				{
					if (!StartAcquisition())
						StopAcquisition(TRUE);	// if bADStart = wrong, then stop AD
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
		else														// no file I/O: refresh screen pos
		{
			if (pWFormat->sample_count >= m_chsweeplength * pWFormat->scan_count)
				pWFormat->sample_count = 0;
		}
		m_Acq32_ADC.SetQueue((long)m_ADC_bufhandle);					// tell ADdriver that data buffer is free

		// then: display options_acqdataataDoc buffer
		if (pWFormat->bOnlineDisplay)								// display data if requested
			m_ADsourceView.ADdisplayBuffer(pdataBuf, m_chsweepRefresh);
		CString cs;
		cs.Format(_T("%.3lf"), duration);							// update total time on the screen
		SetDlgItemText(IDC_STATIC1, cs);							// update time elapsed
	}
}

void CADContView::OnBufferDone_DAC()
{
	// get buffer off done list	
	m_DAC_bufhandle = (HBUF)m_Acq32_DAC.GetQueue();
	if (m_DAC_bufhandle == NULL)
		return;

	// get pointer to buffer
	short* pDTbuf;
	m_ecode = olDmGetBufferPtr(m_DAC_bufhandle, (void**)&pDTbuf);

	if (m_ecode == OLNOERROR)
	{
		DAC_FillBuffer(pDTbuf);
		m_Acq32_DAC.SetQueue((long)m_DAC_bufhandle);
	}
}

void CADContView::ADC_Transfer(short* pDTbuf0)
{
	// get pointer to file waveFormat
	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();
	short* pdataBuf = m_inputDataFile.GetpRawDataBUF();		// options_acqdataataDoc buffer

	m_chsweep1 = m_chsweep2 + 1;								// update data abcissa on the screen 
	if (m_chsweep1 >= m_chsweeplength)						// if data are out of the screen, wrap
		m_chsweep1 = 0;
	m_chsweep2 = m_chsweep1 + m_ADC_chbuflen - 1;				// abcissa of the last data point
	m_chsweepRefresh = m_chsweep2 - m_chsweep1 + 1;			// number of data points to refresh on the screen
	pdataBuf += (m_chsweep1 * pWFormat->scan_count);

	// if offset binary (unsigned words), transform data into signed integers (two's complement)
	if ((m_pADC_options->waveFormat).binzero != NULL)
	{
		WORD binzero = (WORD)(m_pADC_options->waveFormat).binzero;
		WORD* pDTbuf = (WORD*)pDTbuf0;
		for (int j = 0; j < m_ADC_buflen; j++, pDTbuf++)
			*pDTbuf -= binzero;
	}

	// no undersampling.. copy DTbuffer into data file buffer
	if (m_pADC_options->iundersample <= 1)
	{
		memcpy(pdataBuf, pDTbuf0, m_ADC_buflen * sizeof(short));
	}
	// undersampling (assume that buffer length is a multiple of iundersample) and copy into data file buffer
	else
	{
		short* pdataBuf2 = pdataBuf;
		short* pDTbuf = pDTbuf0;
		int iundersample = m_pADC_options->iundersample;
		m_chsweepRefresh = m_chsweepRefresh / iundersample;
		// loop and compute average between consecutive points
		for (int j = 0; j < pWFormat->scan_count; j++, pdataBuf2++, pDTbuf++)
		{
			short* pSource = pDTbuf;
			short* pDest = pdataBuf2;
			for (int i = 0; i < m_ADC_chbuflen; i += iundersample)
			{
				long SUM = 0;
				for (int k = 0; k < iundersample; k++)
				{
					SUM += *pSource;
					pSource += pWFormat->scan_count;
				}
				*pDest = (short)(SUM / iundersample);
				pDest += pWFormat->scan_count;
			}
		}
	}
	// update byte length of buffer
	m_bytesweepRefresh = m_chsweepRefresh * sizeof(short) * pWFormat->scan_count;
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
			m_cyber.SetGain(pchan->am_amplifierchan, int(pchan->am_gaintotal / (double(pchan->am_gainheadstage) * double(pchan->am_gainAD))));
			m_cyber.SetLPFilter(pchan->am_amplifierchan, pchan->am_lowpass);
			int errorcode = m_cyber.C300_FlushCommandsAndAwaitResponse();
		}
	}
	return bcyberPresent;
}

void CADContView::OnBnClickedGainbutton()
{
	((CButton*)GetDlgItem(IDC_BIAS_button))->SetState(0);
	((CButton*)GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

void CADContView::OnBnClickedBiasbutton()
{
	// set bias down and set gain up CButton	
	((CButton*)GetDlgItem(IDC_BIAS_button))->SetState(1);
	((CButton*)GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

void CADContView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == NULL)
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
	int ichan = 0;			// TODO see which channel is selected
	CChanlistItem* pChan = m_ADsourceView.GetChanlistItem(ichan);
	int lSize = pChan->GetYextent();

	// get corresponding data
	switch (nSBCode)
	{
		// .................scroll to the start
	case SB_LEFT:		lSize = YEXTENT_MIN; break;
		// .................scroll one line left
	case SB_LINELEFT:	lSize -= lSize / 10 + 1; break;
		// .................scroll one line right
	case SB_LINERIGHT:	lSize += lSize / 10 + 1; break;
		// .................scroll one page left
	case SB_PAGELEFT:	lSize -= lSize / 2 + 1; break;
		// .................scroll one page right
	case SB_PAGERIGHT:	lSize += lSize + 1; break;
		// .................scroll to end right
	case SB_RIGHT:		lSize = YEXTENT_MAX; break;
		// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	lSize = MulDiv(nPos - 50, YEXTENT_MAX, 100); break;
		// .................NOP: set position only
	default:			break;
	}

	// change y extent
	if (lSize > 0) //&& lSize<=YEXTENT_MAX)
	{
		// assume that all channels are displayed at the same gain & offset
		CWaveFormat* pWFormat = &(m_pADC_options->waveFormat);
		int ichanfirst = 0;
		int ichanlast = pWFormat->scan_count - 1;

		for (int ichan = ichanfirst; ichan <= ichanlast; ichan++) 
		{
			CChanlistItem* pChan = m_ADsourceView.GetChanlistItem(ichan);
			pChan->SetYextent(lSize);
		}
		m_ADsourceView.Invalidate();
		UpdateChanLegends(0);
		m_pADC_options->izoomCursel = lSize;
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
		lSize = (nPos - 50) * (YZERO_SPAN / 100);
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
	long lRes;
	float fMin, fMax, fVolts;
	lRes = (long)pow(2.0, (double)pSS->GetResolution());
	if (pSS->GetMinRange() != 0.F)
		fMin = pSS->GetMinRange() / (float)dfGain;
	else fMin = 0.F;
	if (pSS->GetMaxRange() != 0.F)
		fMax = pSS->GetMaxRange() / (float)dfGain;
	else fMax = 0.F;

	//make sure value is sign extended if 2's comp
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
	{
		lVal = lVal & (lRes - 1);
		if (lVal >= (lRes / 2))
			lVal = lVal - lRes;
	}

	// convert to volts
	fVolts = lVal * ((fMax - fMin) / lRes);

	// adjust DC offset
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
		fVolts = fVolts + ((fMax + fMin) / 2);
	else
		fVolts = fVolts + fMin;

	return fVolts;
}

long CADContView::VoltsToValue(CDTAcq32* pSS, float fVolts, double dfGain)
{
	long lRes;
	float fMin, fMax;
	long lValue;

	lRes = (long)pow(2., (double)pSS->GetResolution());

	if (pSS->GetMinRange() != 0.F)
		fMin = pSS->GetMinRange() / (float)dfGain;
	else fMin = 0.F;
	if (pSS->GetMaxRange() != 0.F)
		fMax = pSS->GetMaxRange() / (float)dfGain;
	else fMax = 0.F;

	//clip input to range
	if (fVolts > fMax) fVolts = fMax;
	if (fVolts < fMin) fVolts = fMin;

	//if 2's comp encoding
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
	{
		lValue = (long)((fVolts - (fMin + fMax) / 2) * lRes / (fMax - fMin));
		// adjust for binary wrap if any
		if (lValue == (lRes / 2)) lValue -= 1;
	}
	else
	{
		// convert to offset binary
		lValue = (long)((fVolts - fMin) * lRes / (fMax - fMin));
		// adjust for binary wrap if any
		if (lValue == lRes) lValue -= 1;
	}
	return lValue;
}

void CADContView::OnCbnSelchangeCombostartoutput()
{
	m_bStartOutPutMode = ((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->GetCurSel();
	m_pDAC_options->bAllowDA = m_bStartOutPutMode;
	BOOL bEnabled = m_bStartOutPutMode != 0;
	GetDlgItem(IDC_STARTSTOP2)->EnableWindow(bEnabled);
}

void CADContView::SetCombostartoutput(int option)
{
	((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->SetCurSel(option);
	option = ((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->GetCurSel();
	m_bStartOutPutMode = option;
	m_pDAC_options->bAllowDA = option;
	BOOL bEnabled = m_bStartOutPutMode != 0;
	GetDlgItem(IDC_STARTSTOP2)->EnableWindow(bEnabled);
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
	CWaveFormat* wave_format = &(m_pADC_options->waveFormat);
	dlg.m_samplingRate = wave_format->chrate;

	if (IDOK == dlg.DoModal())
	{
		for (int i = 0; i < 10; i++)
			m_pDAC_options->outputparms_array[i] = dlg.outputparms_array[i];
		DAC_SetChannelList();
		GetDlgItem(IDC_STARTSTOP2)->EnableWindow(m_DACdigitalchannel > 0);
	}
}

void CADContView::OnBnClickedWriteToDisk()
{
	m_bADwritetofile = TRUE;
	m_pADC_options->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CADContView::OnBnClickedOscilloscope()
{
	m_bADwritetofile = FALSE;
	m_pADC_options->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CADContView::UpdateRadioButtons()
{
	if (m_bADwritetofile)
		((CButton*)GetDlgItem(IDC_WRITETODISK))->SetCheck(BST_CHECKED);
	else
		((CButton*)GetDlgItem(IDC_OSCILLOSCOPE))->SetCheck(BST_CHECKED);
	UpdateData(TRUE);
}

void CADContView::OnBnClickedCardfeatures()
{
	CDlgDataTranslationBoard dlg;
	dlg.m_pAnalogIN = &m_Acq32_ADC;
	dlg.m_pAnalogOUT = &m_Acq32_DAC;
	int iout = dlg.DoModal();
	iout++;
}

void CADContView::OnBnClickedStartstop2()
{
	CString cs;
	if (m_DAC_inprogress)
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
	if (!DAC_InitSubSystem())
		return FALSE;
	DAC_DeclareAndFillBuffers();
	try
	{
		m_DAC_inprogress = FALSE;
		m_Acq32_DAC.Config();
		m_Acq32_DAC.Start();
		m_DAC_inprogress = TRUE;
	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
	}
	return TRUE;
}

void CADContView::StopOutput()
{
	if (m_DAC_inprogress)
		DAC_StopAndLiberateBuffers();
}
