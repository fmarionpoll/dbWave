// adcontvi.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "resource.h"
#include "acqdatad.h"
#include "editctrl.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "adinterv.h"
#include "adexperi.h"
#include "dtacq32.h"
#include "CyberAmp.h"
#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"
#include <oltypes.h>
#include <olerrors.h>
#include <Olxdadefs.h>
#include <olxdaapi.h>
#include "ViewADcontinuous.h"
#include "ChildFrm.h"
#include "MainFrm.h"
#include "ConfirmSaveDlg.h"
#include "DAChannelsDlg.h"
#include "DAOutputsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MIN_DURATION 0.1
#define MIN_BUFLEN	32


/////////////////////////////////////////////////////////////////////////////
// CADContView

IMPLEMENT_DYNCREATE(CADContView, CFormView)

	
// --------------------------------------------------------------------------
CADContView::CADContView()
	: CFormView(CADContView::IDD)
	, m_bOutputsEnabled(FALSE)
	, m_yupper(0)
	, m_ylower(0)
{
	m_sweepduration = 1.0f;
	m_bADwritetofile = FALSE;
	m_ptableSet = NULL;
	m_bADinprogress=FALSE;					// no A/D in progress
	m_bDAinprogress=FALSE;					// no D/A in progress
	m_bchanged=FALSE;						// data unchanged
	m_bAskErase=FALSE;						// no warning when data are erased	
	m_chsweeplength=0;
	m_chDTbuflen=0;
	m_bFileOpen = FALSE;
	m_numchansMAX = 8;
	m_freqmax	= 50000.f;
	m_bOutputsEnabled = FALSE;
	m_bSimultaneousStart=FALSE;
	m_bhidesubsequent=FALSE;

	m_BkColor = GetSysColor(COLOR_BTNFACE);	// set color for edit button
	m_pEditBkBrush = new CBrush(m_BkColor);	// background color = like a button
	ASSERT(m_pEditBkBrush != NULL);			// check brush

	m_bEnableActiveAccessibility=FALSE;
}

// --------------------------------------------------------------------------

CADContView::~CADContView()
{
}

// --------------------------------------------------------------------------

void CADContView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_XLAST, m_sweepduration);
	DDX_Control(pDX, IDC_ANALOGTODIGIT, m_ADC);
	DDX_Control(pDX, IDC_DIGITTOANALOG, m_DAC);
	DDX_Check(pDX, IDC_ENABLEOUTPUT, m_bOutputsEnabled);
	DDX_Control(pDX, IDC_XSCALE, m_adxscale);
	DDX_Control(pDX, IDC_YSCALE, m_adyscale);
	DDX_Text(pDX, IDC_YUPPER, m_yupper);
	DDX_Text(pDX, IDC_YLOWER, m_ylower);
	DDX_Control(pDX, IDC_COMBOBOARD, m_ADcardCombo);
	DDX_Control(pDX, IDC_STARTSTOP, m_btnStartStop);
}

// --------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CADContView, CFormView)
	ON_MESSAGE(WM_MYMESSAGE, &CADContView::OnMyMessage)
	ON_EN_CHANGE(IDC_XLAST, &CADContView::OnEnChangeDuration)
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
	ON_BN_CLICKED(IDC_ENABLEOUTPUT, &CADContView::OnBnClickedEnableoutput)
	ON_BN_CLICKED(IDC_DAPARAMETERS, &CADContView::OnBnClickedDaparameters)
	ON_BN_CLICKED(IDC_DAPARAMETERS2, &CADContView::OnBnClickedDaparameters2)
	ON_CBN_SELCHANGE(IDC_COMBOBOARD, &CADContView::OnCbnSelchangeComboboard)
	ON_BN_CLICKED(IDC_STARTSTOP, &CADContView::OnBnClickedStartstop)
	ON_EN_CHANGE(IDC_YLOWER, &CADContView::OnEnChangeYlower)
	ON_EN_CHANGE(IDC_YUPPER, &CADContView::OnEnChangeYupper)
	ON_BN_CLICKED(IDC_RADIO1, &CADContView::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CADContView::OnBnClickedRadio2)
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

void CADContView::OnDestroy() 
{
	if (m_bADinprogress)
		StopAD(TRUE);

	if (m_bDAinprogress)
		StopDA();

	if (m_bFoundDTOPenLayerDLL)
	{
		// save data here ... TODO
		if (m_ADC.GetHDass() != NULL)
			AD_DeleteBuffers();
		if (m_DAC.GetHDass() != NULL)
			DA_DeleteBuffers();
	}
	delete m_pEditBkBrush;		// brush for edit controls	
	CFormView::OnDestroy();
}

// --------------------------------------------------------------------------

HBRUSH CADContView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	switch (nCtlColor)
	{
		case CTLCOLOR_EDIT:
		case CTLCOLOR_MSGBOX:
			// Set color to green on black and return the background brush.
			pDC->SetBkColor(m_BkColor);
			hbr = (HBRUSH) m_pEditBkBrush->GetSafeHandle();
			break;
 
	   default:
			hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
			break;
	}
	return hbr;
}

/////////////////////////////////////////////////////////////////////////////
// DT Openlayer board functions
/////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// open DT layer board connection to driver
//
// all parameters are stored or come from the parameters array
// OPTIONS_ACQDATA, structure that is present in the program's main memory
// m_ADC.GetHDass() = NULL

BOOL CADContView::FindDTOpenLayersBoard()
{		
	// load board name - skip dialog if only one is present
	UINT uiNumBoards = m_ADC.GetNumBoards();
	BOOL flag = (uiNumBoards > 0 ? TRUE: FALSE);

	m_ADcardCombo.ResetContent();
	for(UINT i=0; i < uiNumBoards; i++)
		m_ADcardCombo.AddString (m_ADC.GetBoardList(i));

	int isel = 0;
	switch (uiNumBoards)
	{
	case 0:												// no board present: exit
		m_ADcardCombo.AddString(_T("No Board"));
		m_ADcardCombo.SetCurSel(0);
		return FALSE; 
		break;
	case 1:												// only one board: no fuss, stick to it!
		isel = 0;
		break;
	default:											// more than on board (unlikely?!)
		// if name already defined, check if board present
		if (!(m_pacqD->waveFormat).csADcardName.IsEmpty())
			isel = m_ADcardCombo.FindString(-1, (m_pacqD->waveFormat).csADcardName);
		// former card not found, take the first in the list
		if (isel < 0)
			isel = 0;
		break;
	}
	m_ADcardCombo.SetCurSel(isel);
	SelectDTOpenLayersBoard (m_ADC.GetBoardList(isel));
	return flag;
}

BOOL CADContView::SelectDTOpenLayersBoard(CString cardName)
{
	// get infos
	m_bFoundDTOPenLayerDLL = TRUE;	
	(m_pacqD->waveFormat).csADcardName = cardName;
	
	// assume here that only 1 card is connected and that both systems are on the card
	// otherwise, we need to split this routine into 2 different routines
	// and add another combo on the view to select the 2 systems indenpendently

	// connect A/D subsystem and display/hide buttons
	BOOL flagAD = FALSE;
	BOOL bsimultaneousStartAD=FALSE;
	try
	{
		m_ADC.SetBoard(cardName);		// select board
		flagAD = AD_OpenSubSystem();
		bsimultaneousStartAD = m_ADC.GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);
	}
	catch(COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		flagAD = FALSE;
	}

	int bShow = (flagAD ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_ADPARAMETERS)->ShowWindow(bShow);
	GetDlgItem(IDC_ADPARAMETERS2)->ShowWindow(bShow);

	// connect D/A subsystem and display/hide buttons
	BOOL flagDA = FALSE;
	BOOL bsimultaneousStartDA=FALSE;
	try
	{
		m_DAC.SetBoard(cardName);
		flagDA = DA_OpenSubSystem();
		if (flagDA)
			bsimultaneousStartDA= m_DAC.GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);
		else
			m_bOutputsEnabled = FALSE;
	}
	catch(COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		flagDA=FALSE;
	}

	// display additional interface elements
	bShow = (flagDA ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_ENABLEOUTPUT)->ShowWindow(bShow);
	GetDlgItem(IDC_DAPARAMETERS)->ShowWindow(bShow);
	GetDlgItem(IDC_DAPARAMETERS2)->ShowWindow(bShow);
	GetDlgItem(IDC_DAGROUP)->ShowWindow(bShow);

	// simultaneous A/D and D/A
	m_bSimultaneousStart = bsimultaneousStartDA && bsimultaneousStartAD;
	return TRUE;
}

void CADContView::OnCbnSelchangeComboboard()
{
	int isel = m_ADcardCombo.GetCurSel();
	CString csCardName;
	m_ADcardCombo.GetLBText(isel, csCardName);
	SelectDTOpenLayersBoard(csCardName);
}

// get Digital to analog parameters
BOOL CADContView::DA_OpenSubSystem() 
{
	try
	{
		m_DAC.SetSubSysType(OLSS_DA);
	}
	catch(COleDispatchException* e)
	{
		//AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	} 
	// select D/A system
	try {
		int nDA = m_DAC.GetDevCaps(OLDC_DAELEMENTS);		// make sure D/A is available
		if (nDA < 1)
			return FALSE;
		m_DAC.SetSubSysElement(0);
	}
	catch(COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	} 
	// get capacities of the subsystem
	float max		= m_DAC.GetMaxRange();					// maximum out voltage
	float min		= m_DAC.GetMinRange();					// minimum out voltage
	int iresolution = m_DAC.GetResolution();				// nbits resolution: DT9818=16
	int nchannels	= m_DAC.GetSSCaps(OLSSC_NUMCHANNELS);	// DT9818: 2x 16 bits + 1 for the 8 digital outputs
	int iencoding	= m_DAC.GetEncoding();					// encoding mode DT9818: OLx_ENC_BINARY
	ASSERT(iencoding == OLx_ENC_BINARY);

	// check we have a correct handle to the system
	if(m_DAC.GetHDass() == NULL)
	{
		AfxMessageBox(_T("Digital-to-Analog Subsystem is not available."));
		return FALSE;
	}
	return TRUE;
}

// get Analog to Digital parameters
BOOL CADContView::AD_OpenSubSystem() 
{
	try
	{
		int nAD = m_ADC.GetDevCaps(OLDC_ADELEMENTS);		// make sure A/D is available
		if (nAD < 1)
			return FALSE;
		m_ADC.SetSubSysType(OLSS_AD);						// select A/D system
		m_ADC.SetSubSysElement(0);
	}
	catch(COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	}
	
	// save parameters into CWaveFormat
	CWaveFormat* pWFormat = &(m_pacqD->waveFormat); 
	float max = m_ADC.GetMaxRange();						// maximum input voltage
	float min = m_ADC.GetMinRange();						// minimum input voltage
	pWFormat->fullscale_Volts = (float) (max-min);

	// convert into bin scale (nb of divisions)
	int iresolution = m_ADC.GetResolution();
	pWFormat->fullscale_bins = ((1L << iresolution) - 1);

	// set max channel number according to input configuration m_numchansMAX
	m_pacqD->bChannelType = m_ADC.GetChannelType();
	if (m_pacqD->bChannelType == OLx_CHNT_SINGLEENDED)
		m_numchansMAX = m_ADC.GetSSCaps(OLSSC_MAXSECHANS);
	else 
		m_numchansMAX = m_ADC.GetSSCaps(OLSSC_MAXDICHANS);

	// data encoding (binary or offset encoding)
	pWFormat->mode_encoding = (int) m_ADC.GetEncoding();
	if (pWFormat->mode_encoding == OLx_ENC_BINARY)
		pWFormat->binzero = pWFormat->fullscale_bins/2+1;
	else if (pWFormat->mode_encoding == OLx_ENC_2SCOMP)
		pWFormat->binzero = 0;

	// load infos concerning frequency, dma chans, programmable gains
	m_freqmax	= m_ADC.GetSSCapsEx(OLSSCE_MAXTHROUGHPUT);	// m_dfMaxThroughput

	// TODO tell sourceview here under which format are data
	// TODO save format of data into temp document
	// float volts = (float) ((pWFormat->fullscale_Volts) 
	//				/(pWFormat->fullscale_bins) * value  -pWFormat->fullscale_Volts/2);
	// TODO: update max min of chan 1 with gain && instrumental gain
	//UpdateChanLegends(0);
	//UpdateHorizontalRulerBar();
	//UpdateVerticalRulerBar();

	// check that subsystem is here
	if(m_ADC.GetHDass() == NULL)
	{
		AfxMessageBox(_T("Analog-to-Digital Subsystem is not available."));
		return FALSE;
	}
	return TRUE;
}

// --------------------------------------------------------------------------
// set up configuration of the AD parameters of the board
// called when a data acquisition parameter is changed
// copy parameters stored within m_pacqdef
//
// all parameters are stored or come from the parameters array OPTIONS_ACQDATA
// This structure is present in the program's main memory at the end of this routine
// This parameters array is passed to the current document 
// and the data buffer is modified accordingly to handle the flow of data

BOOL CADContView::AD_InitSubSystem()
{
	// make sure we have a valid handle
	if (m_ADC.GetHDass() == NULL)
		if (!FindDTOpenLayersBoard())
			return FALSE;

	// store all values within global parameters array
	CWaveFormat* pAcqDwaveFormat = &(m_pacqD->waveFormat);

	// Set up the ADC - no wrap so we can get buffer reused	
	m_ADC.SetDataFlow(OLx_DF_CONTINUOUS);
	m_ADC.SetWrapMode(OLx_WRP_NONE);
	m_ADC.SetDmaUsage((short) m_ADC.GetSSCaps(OLSSC_NUMDMACHANS));
	m_ADC.SetClockSource(OLx_CLK_INTERNAL);

	// set trigger mode
	int trig = pAcqDwaveFormat->trig_mode;
	if (trig > OLx_TRG_EXTRA) 		trig = 0;
	m_ADC.SetTrigger(trig);

	// number of channels
	if (m_pacqD->bChannelType == OLx_CHNT_SINGLEENDED && m_ADC.GetSSCaps(OLSSC_SUP_SINGLEENDED) == NULL)
		m_pacqD->bChannelType = OLx_CHNT_DIFFERENTIAL;
	if (m_pacqD->bChannelType == OLx_CHNT_DIFFERENTIAL && m_ADC.GetSSCaps(OLSSC_SUP_DIFFERENTIAL) == NULL)
		m_pacqD->bChannelType = OLx_CHNT_SINGLEENDED;
	m_ADC.SetChannelType(m_pacqD->bChannelType);
	if (m_pacqD->bChannelType == OLx_CHNT_SINGLEENDED)
		m_numchansMAX = m_ADC.GetSSCaps(OLSSC_MAXSECHANS);
	else
		m_numchansMAX = m_ADC.GetSSCaps(OLSSC_MAXDICHANS);
	// limit scan_count to m_numchansMAX -
	// this limits the nb of data acquisition channels to max-1 if one wants to use the additional I/O input "pseudo"channel
	// so far, it seems acceptable...
	if (pAcqDwaveFormat->scan_count > m_numchansMAX)
		pAcqDwaveFormat->scan_count = m_numchansMAX;

	// set frequency to value requested, set frequency and get the value returned
	double clockrate = pAcqDwaveFormat->chrate*pAcqDwaveFormat->scan_count;
	m_ADC.SetFrequency(clockrate);			// set sampling frequency (total throughput)
	clockrate = m_ADC.GetFrequency();
	pAcqDwaveFormat->chrate = (float) clockrate / pAcqDwaveFormat->scan_count;

	// update channel list (chan & gain)
	// pD->SetBinFormat(docVoltsperb, pwaveFormat->binzero, pwaveFormat->fullscale_bins);
	// 
	m_ADC.SetListSize(pAcqDwaveFormat->scan_count);
	for (int i = 0; i < pAcqDwaveFormat->scan_count; i++)
	{
		// transfer data from CWaveChan to chanlist of the A/D subsystem
		CWaveChan* pChannel = (m_pacqD->chanArray).GetWaveChan(i);
		if ( pChannel->am_adchannel> m_numchansMAX-1 && pChannel->am_adchannel != 16)
			 pChannel->am_adchannel= m_numchansMAX-1;
		m_ADC.SetChannelList(i, pChannel->am_adchannel);
		m_ADC.SetGainList(i, pChannel->am_adgain);
		double dGain = m_ADC.GetGainList(i);
		pChannel->am_adgain = (short) dGain;
		// compute dependent parameters
		pChannel->am_gainfract = pChannel->am_gainheadstage * (float) pChannel->am_gainpre * (float) pChannel->am_gainpost;
		pChannel->am_totalgain = pChannel->am_gainfract * pChannel->am_adgain;
		pChannel->am_resolutionV = pAcqDwaveFormat->fullscale_Volts / pChannel->am_totalgain / pAcqDwaveFormat->fullscale_bins;
	}

	// pass parameters to the board and check if errors
	try
	{
		m_ADC.ClearError();
		m_ADC.Config();
	}
	catch(COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	}

	// AD system is changed:  update AD buffers & change encoding: it is changed on-the-fly in the transfer loop
	*(m_inputDataFile.GetpWavechanArray()) = m_pacqD->chanArray;
	*(m_inputDataFile.GetpWaveFormat())	= m_pacqD->waveFormat;

	AD_DeclareBuffers();
	return TRUE;
}

BOOL CADContView::DA_InitSubSystem()
{
	// define system parameters only if requested
	if (!m_bOutputsEnabled)
		return FALSE;

	// make sure we have a valid handle
	if (m_DAC.GetHDass() == NULL)
		if (!FindDTOpenLayersBoard())
			return FALSE;

	// store all values within global parameters array
	CWaveFormat* pAcqDwaveFormat = &(m_pacqD->waveFormat);
	
	// Set up the ADC - multiple wrap so we can get buffer reused	
	m_DAC.SetDataFlow(OLx_DF_CONTINUOUS);
	m_DAC.SetWrapMode(OLx_WRP_NONE);
	m_DAC.SetDmaUsage((short) m_DAC.GetSSCaps(OLSSC_NUMDMACHANS));

	// sampling rate (set / get)
	m_DAC.SetClockSource(OLx_CLK_INTERNAL);
	// set clock the same as for A/D
	double clockrate = pAcqDwaveFormat->chrate;
	m_DAC.SetFrequency(clockrate);			// set sampling frequency (total throughput)
	clockrate = m_DAC.GetFrequency();		// make sure it works

	// set trigger mode
	int trig = pAcqDwaveFormat->trig_mode;
	if (trig > OLx_TRG_EXTRA) trig = 0;
	m_DAC.SetTrigger(trig);

	// number of channels
	//int nchannels	= m_DAC.GetSSCaps(OLSSC_NUMCHANNELS);	// DT9818: 2x 16 bits + 1 for the 8 digital outputs

	// update channel list (chan & gain)
	// check here that list size is correct and that max chan is ok?
	//m_DAC.SetListSize(pAcqDwaveFormat->scan_count);
	//for (int i = 0; i < pAcqDwaveFormat->scan_count; i++)
	//{
	//	CWaveChan* pChannel = (m_pacqD->chanArray).GetWaveChan(i);
	//	if ( pChannel->am_adchannel> m_numchansMAX-1 && pChannel->am_adchannel != 16)
	//		 pChannel->am_adchannel= m_numchansMAX-1;
	//	m_DAC.SetChannelList(i, pChannel->am_adchannel);
	//	m_DAC.SetGainList(i, pChannel->am_adgain);
	//	double dGain = m_DAC.GetGainList(i);
	//	pChannel->am_adgain = (short) dGain;
	//}

	// pass parameters to the board and check if errors
	try
	{
		m_DAC.ClearError();
		m_DAC.Config();
	}
	catch(COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	}

	// AD system is changed:  update AD buffers
	DA_DeclareBuffers();
	return TRUE;
}

// --------------------------------------------------------------------------
// delete adbuffers

void CADContView::AD_DeleteBuffers()
{
	// exit if not connected
	if (m_ADC.GetHDass() == NULL)
		return;

	m_ADC.Flush();		// clean
	HBUF hBuf;			// handle to buffer
	do					// loop until all buffers are removed
	{
		hBuf = (HBUF)m_ADC.GetQueue();
		if(hBuf != NULL)
		{
			if (olDmFreeBuffer(hBuf) != OLNOERROR)
				AfxMessageBox(_T("Error Freeing Buffer"));
		}
	} while (hBuf != NULL);
	m_ADbufhandle = hBuf;
	return;
}

void CADContView::DA_DeleteBuffers()
{
	// exit if not connected
	if (m_DAC.GetHDass() == NULL)
		return;

	m_DAC.Flush();		// clean
	HBUF hBuf;			// handle to buffer
	do					// loop until all buffers are removed
	{
		hBuf = (HBUF)m_DAC.GetQueue();
		if(hBuf != NULL)
		{
			if (olDmFreeBuffer(hBuf) != OLNOERROR)
				AfxMessageBox(_T("Error Freeing Buffer"));
		}
	} while (hBuf != NULL);
	m_DAbufhandle = hBuf;
	return;
}

// --------------------------------------------------------------------------
// clear old buffers and attach buffers to the transfer list
// variables:
// pWFormat->bufferNitems		n buffers
// m_buffersize		buffer size (total nb data points)

void CADContView::AD_DeclareBuffers()
{
	// close data buffers
	AD_DeleteBuffers();

	// make sure that buffer length contains at least nacq chans
	CWaveFormat* pWFormat = &(m_pacqD->waveFormat); // get pointer to m_pacqD wave format
	if (pWFormat->buffersize < pWFormat->scan_count*m_pacqD->iundersample)
		pWFormat->buffersize = pWFormat->scan_count*m_pacqD->iundersample;

	// define buffer length
	m_sweepduration = m_pacqD->sweepduration;
	m_chsweeplength = (long) (m_sweepduration* pWFormat->chrate / (float) m_pacqD->iundersample);
	m_chDTbuflen = m_chsweeplength * m_pacqD->iundersample / pWFormat->bufferNitems;
	m_ADbuflen = m_chDTbuflen * pWFormat->scan_count;
	
	// declare buffers to DT
	ECODE ecode;
	for (int i=0; i < pWFormat->bufferNitems; i++)
	{ 
		ecode = olDmAllocBuffer(0, m_ADbuflen, &m_ADbufhandle);
		ecode = OLNOERROR;
		if((ecode == OLNOERROR)&&(m_ADbufhandle != NULL))
			m_ADC.SetQueue((long)m_ADbufhandle); // but buffer onto Ready queue
	}

	// set sweep length to the nb of data buffers
	(m_inputDataFile.GetpWaveFormat())->sample_count = m_chsweeplength * (long) pWFormat->scan_count;	// ?
	m_inputDataFile.AdjustBUF(m_chsweeplength);
	*(m_inputDataFile.GetpWaveFormat()) = *pWFormat;	// save settings into data file	

	// update display length (and also the text - abcissa)	
	m_ADsourceView.AttachDataFile(&m_inputDataFile, 0);
	m_ADsourceView.ResizeChannels(0, m_chsweeplength);
	if (m_ADsourceView.GetChanlistSize() != pWFormat->scan_count)
	{
		m_ADsourceView.RemoveAllChanlistItems();
		for (int j = 0; j< pWFormat->scan_count; j++)
			m_ADsourceView.AddChanlistItem(j, 0);
	}

	// adapt source view 
	int iextent =  MulDiv(pWFormat->fullscale_bins, 12, 10);
	if (m_pacqD->izoomCursel != 0)
		iextent = m_pacqD->izoomCursel;
	int ioffset = 0; 
	for (int i = 0; i < pWFormat->scan_count; i++)
	{    	
		m_ADsourceView.SetChanlistZero(i, ioffset);	// combine calls into one?
		m_ADsourceView.SetChanlistYextent(i, iextent);
		m_ADsourceView.SetChanlistColor(i, i);
	}
	m_ADsourceView.Invalidate();
	UpdateData(FALSE);
}

void CADContView::DA_DeclareBuffers()
{
	// close data buffers
	DA_DeleteBuffers();

	// make sure that buffer length contains at least nacq chans
	int scan_count=1;
	CWaveFormat* pWFormat = &(m_pacqD->waveFormat); // get pointer to m_pacqD wave format

	// define buffer length
	float sweepduration = m_pacqD->sweepduration;
	long chsweeplength	= (long) (sweepduration* pWFormat->chrate / (float) m_pacqD->iundersample);
	long chDAbuflen		= chsweeplength * m_pacqD->iundersample / pWFormat->bufferNitems;
	long DAbuflen		= chDAbuflen; // * pWFormat->scan_count;
	
	// declare buffers to DT
	ECODE ecode;
	for (int i=0; i <= pWFormat->bufferNitems; i++)
	{ 
		ecode = olDmAllocBuffer(0, DAbuflen, &m_DAbufhandle);
		short* pDTbuf;
		ecode = olDmGetBufferPtr(m_DAbufhandle,(void **)&pDTbuf);
		DA_FillBuffer(pDTbuf);
		if((ecode == OLNOERROR)&&(m_DAbufhandle != NULL))
			m_DAC.SetQueue((long)m_DAbufhandle);			// put buffer onto Ready queue
	}
}

// --------------------------------------------------------------------------
void CADContView::StopDA()
{
	// stop DA, liberate DTbuffers
	try {
		m_DAC.Abort();	// abort any subsystem operations
		m_DAC.Reset();	// reset the subsystem to a known state
		m_DAC.Flush();	// flush all buffers to Done Queue
		HBUF hBuf;
		do {
			hBuf = (HBUF) m_DAC.GetQueue();
			if(hBuf != NULL) { 
				ECODE ecode = olDmFreeBuffer(hBuf);
				if(ecode != OLNOERROR)
					AfxMessageBox(_T("Could not free Buffer"));
			}
		}	while(hBuf != NULL);		
	}
	catch(COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
	}
	m_bDAinprogress=FALSE; 

	// reset D/A outputs to zero
	try {
		m_DAC.SetDataFlow(OLx_DF_SINGLEVALUE);
		m_DAC.Config();
		long nchannels	= m_DAC.GetSSCaps(OLSSC_NUMCHANNELS)-1;	// DT9818: 2x 16 bits + 1 for the 8 digital outputs
		for (long i = 0; i < nchannels; i++)
		{
			long Value = VoltsToValue(&m_DAC, 0., 1.);
			m_DAC.PutSingleValue(i, 1., Value);
		}
	}
	catch(COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
	}
}


void CADContView::StopAD(BOOL bDisplayErrorMsg)
{
	// special treatment if simultaneous list
	if (m_bSimultaneousStart)
	{
		HSSLIST hSSlist;
		CHAR errstr[255];
		ECODE ecode;

		// create a simultaneous start list
		ecode = olDaGetSSList(&hSSlist);
		olDaGetErrorString(ecode,errstr,255);
		ecode = olDaReleaseSSList(hSSlist);    
		olDaGetErrorString(ecode,errstr,255);
	}

	// stop AD, liberate DTbuffers
	try {
		m_ADC.Abort();							// abort any subsystem operations
		m_ADC.Reset();							// reset the subsystem to a known state
		m_ADC.Flush();							// flush all buffers to Done Queue
		HBUF hBuf;
		do {
			hBuf = (HBUF) m_ADC.GetQueue();
			if(hBuf != NULL) m_ADC.SetQueue((long)hBuf);
		}	while(hBuf != NULL);
		m_ADsourceView.ADdisplayStop();
		m_bchanged=TRUE;
		m_bADinprogress=FALSE;
	}
	catch(COleDispatchException* e)
	{
		if (bDisplayErrorMsg)
			AfxMessageBox(e->m_strDescription);
		e->Delete();
	}

	// stop DA, liberate buffers
	if (m_bDAinprogress)
		StopDA();

	// close file and update display
	if (m_bFileOpen)
	{
		m_inputDataFile.AcqDoc_DataAppendStop();
		CWaveFormat* pWFormat= m_inputDataFile.GetpWaveFormat();

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
				CConfirmSaveDlg dlg;
				dlg.m_csfilename=m_szFileName;
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
				if (m_szFileName.CompareNoCase(pdbDoc->DBGetCurrentDatFileName(FALSE)) != 0)
				{
					// add document to database
					m_csNameArray.Add(m_szFileName);
					TransferFilesToDatabase();
					UpdateViewDataFinal();
					return;
				}
			}
		}

		// update view data	
		long lsizeDOCchan = m_inputDataFile.GetDOCchanLength();
		m_ADsourceView.AttachDataFile(&m_inputDataFile, lsizeDOCchan);
		m_ADsourceView.ResizeChannels(m_ADsourceView.Width(), lsizeDOCchan);
		m_ADsourceView.GetDataFromDoc(0, lsizeDOCchan);
	}
}

void CADContView::UpdateViewDataFinal()
{
	// update view data	
	CdbWaveDoc* pdbDoc = GetDocument();
	BOOL flag = pdbDoc->OpenCurrentDataFile();
	if (!flag)
	{
		CdbWdatabase* pDB = pdbDoc->m_pDB;
		int nrecords = pDB->GetNRecords();
		ATLTRACE2(_T("error reading current document"));
		return;
	}
	CAcqDataDoc* pDocDat = pdbDoc->m_pDat;
	pDocDat->ReadDataInfos();
	long lsizeDOCchan = pDocDat->GetDOCchanLength();
	m_ADsourceView.AttachDataFile(pDocDat, lsizeDOCchan);
	m_ADsourceView.ResizeChannels(m_ADsourceView.Width(), lsizeDOCchan);
	m_ADsourceView.GetDataFromDoc(0, lsizeDOCchan);
}

void CADContView::TransferFilesToDatabase()
{
	CdbWaveDoc* pdbDoc = GetDocument();
	pdbDoc->ImportDescFromFileList(m_csNameArray);				// add file name(s) to the list of records in the database
	m_csNameArray.RemoveAll();									// clear file names

	CdbMainTable* pSet = &(GetDocument()->m_pDB->m_tableSet);
	pSet->BuildAndSortIDArrays();
	pSet->RefreshQuery();
	int nfiles = pdbDoc->m_pDB->GetNRecords();
	pdbDoc->DBSetCurrentRecordPosition(nfiles-1);
	pdbDoc->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);
}

// --------------------------------------------------------------------------

BOOL CADContView::StartAD()
{
	// set display
	if (m_bADwritetofile && !Defineexperiment())
	{
		StopAD(FALSE);
		UpdateStartStop(FALSE);
		return FALSE;
	}

	// make sure that analog-to-digital is available
	if (!AD_InitSubSystem())
		return FALSE;
	
	// start AD display
	m_chsweep1 = 0;
	m_chsweep2 = -1;
	m_ADsourceView.ADdisplayStart(m_chsweeplength);
	CWaveFormat* pWFormat= m_inputDataFile.GetpWaveFormat();
	pWFormat->sample_count		= 0;							// no samples yet
	pWFormat->chrate			= pWFormat->chrate/m_pacqD->iundersample;
	m_fclockrate				= pWFormat->chrate*pWFormat->scan_count;
	pWFormat->acqtime = CTime::GetCurrentTime();

	// data format
	pWFormat->fullscale_bins	= (m_pacqD->waveFormat).fullscale_bins;
	pWFormat->fullscale_Volts	= (m_pacqD->waveFormat).fullscale_Volts ;
	// trick: if OLx_ENC_BINARY, it is changed on the fly within AD_Transfer function 
	// when a DT buffer into a CAcqDataDoc buffer
	pWFormat->mode_encoding		=  OLx_ENC_2SCOMP;
	pWFormat->binzero			= 0;

	//
	if (m_bADwritetofile == TRUE)
	{
		switch (pWFormat->trig_mode)
		{
		case OLx_TRG_EXTRA+1:
			if (AfxMessageBox(_T("Start data acquisition"), MB_OKCANCEL) != IDOK)
			{
				StopAD(FALSE);
				//StopDA();				// ???
				UpdateStartStop(FALSE);
				return FALSE;
			}
			break;
		case OLx_TRG_EXTERN:
		case OLx_TRG_SOFT:
		case OLx_TRG_THRESHPOS:
		case OLx_TRG_THRESHNEG:
		default:
			break;
		}
	}

	// start D/A and then A/D 
	StartDA();

	// start acquisition
	m_ADC.ClearError();
	
	// starting mode of A/D if not simultaneous list------------------------
	if (!m_bSimultaneousStart || !m_bOutputsEnabled)
	{
		m_ADC.Config();
		m_ADC.Start();
		m_bADinprogress	=TRUE;	// AD is running
	}
	// starting mode of A/D if simultaneous list ---------------------------
	else
	{
		BOOL retval;
		HSSLIST hSSlist;
		CHAR errstr[255];
		ECODE ecode;

		// clear previous list
		ecode = olDaGetSSList(&hSSlist);
		olDaGetErrorString(ecode,errstr,255);
		ecode = olDaReleaseSSList(hSSlist);    
		olDaGetErrorString(ecode,errstr,255);

		// create simultaneous starting list
		ecode = olDaGetSSList(&hSSlist);
		olDaGetErrorString(ecode,errstr,255);
		
		// put both subsystems onto simultaneous start List
		HDASS hADC = (HDASS) m_ADC.GetHDass();
		int nbuffers = m_ADC.GetListSize();
		ecode = olDaPutDassToSSList (hSSlist, hADC);
		if(ecode != OLNOERROR)
		{
			retval = ecode;
			ecode = olDaReleaseSSList(hSSlist);
			return(retval);
		}
		ecode = olDaPutDassToSSList(hSSlist, (HDASS) m_DAC.GetHDass());
		if(ecode != OLNOERROR)
		{
			retval = ecode;
			ecode = olDaReleaseSSList(hSSlist);
			return(retval);
		}
		// config
		m_ADC.Config();
		m_DAC.Config();

		// prestart
		ecode = olDaSimultaneousPrestart(hSSlist);
		olDaGetErrorString(ecode, errstr, 255);
		if (ecode != NOERROR)
		{
			displayolDaErrorMessage(errstr);
		}


		// start simultaneously
		ecode = olDaSimultaneousStart(hSSlist);
		olDaGetErrorString(ecode, errstr, 255);
		if (ecode != NOERROR)
		{
			displayolDaErrorMessage(errstr);
		}
			
		m_bADinprogress=TRUE;
		m_bDAinprogress=TRUE;
	} // --------------------------------------------------------------------------
	
	return TRUE;
}

void CADContView::displayolDaErrorMessage(CHAR* errstr)
{
	CString csError;
	CStringA cstringa(errstr);
	csError = cstringa;
	AfxMessageBox(csError);
}

BOOL CADContView::StartDA()
{
	if (!m_bOutputsEnabled)
		return FALSE;

	// init DA
	if (!DA_InitSubSystem())
		return FALSE;

	m_DAC.ClearError();
	
	if (!m_bSimultaneousStart)
	{
		m_DAC.Config();
		m_DAC.Start();									// starting mode of D/A if not simultaneous list	
		m_bDAinprogress=TRUE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CADContView diagnostics

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

/////////////////////////////////////////////////////////////////////////////
// CADContView overriden functions
/////////////////////////////////////////////////////////////////////////////

CDaoRecordset* CADContView::OnGetRecordset()
{
	return m_ptableSet;
}

// --------------------------------------------------------------------------
// initial settings

void CADContView::OnInitialUpdate()
{
	// attach controls
	VERIFY(m_ADsourceView.SubclassDlgItem(IDC_DISPLAYDATA, this));
	VERIFY(mm_sweepduration.SubclassDlgItem(IDC_XLAST, this));
	VERIFY(mm_yupper.SubclassDlgItem(IDC_YUPPER, this));
	VERIFY(mm_ylower.SubclassDlgItem(IDC_YLOWER, this));
	m_stretch.AttachParent(this);

	m_stretch.newProp(IDC_DISPLAYDATA,		XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_YLOWER,			SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_XLAST,			SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_STATICSWEEPLENGTH,SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_XSCALE, 			XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_GAIN_button, 		SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_BIAS_button,		SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_SCROLLY_scrollbar,SZEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_YSCALE, 			SZEQ_XLEQ, YTEQ_YBEQ);

	// bitmap buttons: load icons & set buttons
	m_hBias=AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom=AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hBias);
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hZoom);
	
	BOOL b32BitIcons = afxGlobalData.m_nBitsPerPixel >= 16;
	m_btnStartStop.SetImage(b32BitIcons ? IDB_CHECK32 : IDB_CHECK);
	m_btnStartStop.SetCheckedImage(b32BitIcons ? IDB_CHECKNO32 : IDB_CHECKNO);
	m_btnStartStop.EnableWindowsTheming(false); // true
	m_btnStartStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D; //BUTTONSTYLE_SEMIFLAT;

	// scrollbar
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	// CFormView init CFile
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	m_pacqD = &(pApp->acqD);									// address of data acquisition parameters
	m_poutD = &(pApp->outD);									// address of data output parameters
	m_bFoundDTOPenLayerDLL = FALSE;								// assume there is no card
	m_bADwritetofile = m_pacqD->waveFormat.bADwritetofile;
	m_bOutputsEnabled = m_poutD->bAllowDA;
	
	// open document and remove database filters
	CdbWaveDoc* pdbDoc = GetDocument();							// data document with database
	m_ptableSet = &pdbDoc->m_pDB->m_tableSet;							// database itself
	m_ptableSet->m_strFilter.Empty();
	m_ptableSet->ClearFilters();
	m_ptableSet->RefreshQuery();
	//pdbDoc->UpdateAllViews(this, HINT_FILTERREMOVED, NULL);
	CFormView::OnInitialUpdate();

	// if current document, load parameters from current document into the local set of parameters
	// if current document does not exist, do nothing
	if (pdbDoc->m_pDB->GetNRecords() > 0) {
		pdbDoc->OpenCurrentDataFile();							// read data descriptors from current data file
		CAcqDataDoc* pDat = pdbDoc->m_pDat;						// get a pointer to the data file itself
		if (pDat != NULL) 
		{
			m_pacqD->waveFormat = *(pDat->GetpWaveFormat());	// read data header
			m_pacqD->chanArray.ChannelSetnum(m_pacqD->waveFormat.scan_count);
			m_pacqD->chanArray = *pDat->GetpWavechanArray();	// get channel descriptors
			// restore state of "write-to-file" parameter that was just erased
			m_pacqD->waveFormat.bADwritetofile = m_bADwritetofile;
		}
	}

	// create data file and copy data acquisition parameters into it
	m_inputDataFile.OnNewDocument();							// create a file to receive incoming data (A/D)
	*(m_inputDataFile.GetpWaveFormat()) = m_pacqD->waveFormat;	// copy data formats into this file
	m_pacqD->chanArray.ChannelSetnum(m_pacqD->waveFormat.scan_count);
	*(m_inputDataFile.GetpWavechanArray()) = m_pacqD->chanArray;
	m_ADsourceView.AttachDataFile(&m_inputDataFile, 10);		// prepare display area
	
	pApp->m_bADcardFound = FindDTOpenLayersBoard();				// open DT Open Layers board
	if (pApp->m_bADcardFound)
	{
		AD_InitSubSystem();										// connect A/D DT OpenLayer subsystem
		InitCyberAmp();											// control cyberamplifier
		DA_InitSubSystem();										// connect D/A DT OpenLayers subsystem
	}
	else
	{
		GetDlgItem(IDC_STARTSTOP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADPARAMETERS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADPARAMETERS2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DAGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ENABLEOUTPUT)->ShowWindow(SW_HIDE);
	}
	
	UpdateChanLegends(0);
	UpdateHorizontalRulerBar();
	UpdateChanVerticalRulerBar(0);
	UpdateRadioButtons();

	// tell mmdi parent which cursor is active
	GetParent()->PostMessage(WM_MYMESSAGE, NULL, MAKELPARAM(m_cursorstate, HINT_SETMOUSECURSOR));
}

// --------------------------------------------------------------------------

void CADContView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// update sent from within this class
	if (pSender == this)
	{
		ASSERT(GetDocument() != NULL);
		m_ADsourceView.Invalidate();		// display data
	}
}

void CADContView::UpdateHorizontalRulerBar()
{
	// update horizontal scale
	CWaveFormat* pwaveFormat = &(m_pacqD->waveFormat);
	float samplingrate = pwaveFormat->chrate;
	float timefirst = m_ADsourceView.GetDataFirst()/samplingrate;
	float timelast = m_ADsourceView.GetDataLast()/samplingrate;
	m_adxscale.SetRange(&timefirst, &timelast);
}

void CADContView::UpdateChanVerticalRulerBar(int chan)
{
	// update vertical scale
	int ichan = 0;
	int max =  m_ADsourceView.FromChanlistPixeltoBin(ichan, 0);
	float xmax = m_ADsourceView.GetChanlistBintoMilliVolts(ichan, max);
	int min = m_ADsourceView.FromChanlistPixeltoBin(ichan, m_ADsourceView.Height());
	float xmin = m_ADsourceView.GetChanlistBintoMilliVolts(ichan, min);
	m_adyscale.SetRange(&xmin, &xmax);
	// TODO update m_ylower & m_yupper
}

// --------------------------------------------------------------------------

void CADContView::OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CMainFrame* pmF = (CMainFrame*)AfxGetMainWnd();
	// activate view
	if (bActivate)
	{
		pmF->SetSecondToolBar(IDR_DBDATATYPE);
		pmF->ActivatePropertyPane(FALSE);
		((CChildFrame*)pmF->MDIGetActive())->m_cursorstate = 0;
	}
		CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

/////////////////////////////////////////////////////////////////////////////
// CADContView message handlers

// --------------------------------------------------------------------------
// OnSize() update position of controls
void CADContView::OnSize(UINT nType, int cx, int cy) 
{
	switch (nType)
	{
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
		if (m_bADinprogress)
		{
			StopAD(TRUE);
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

// --------------------------------------------------------------------------
// answer to message from either sourceview or other ...

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

// ---------------------------------------------------------------
// toggle data acquisition ON/OFF

void CADContView::OnBnClickedStartstop()
{
	BOOL bADStart=TRUE;
	// Start
	if(m_btnStartStop.IsChecked())
	{
		bADStart = StartAD();
		if (bADStart)
		{
			if ((m_inputDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
				OnBufferDone_ADC();
		}
		else
		{
			bADStart= FALSE;
			StopAD(FALSE);
		}
	}
	// Stop
	else
	{
		bADStart= FALSE;
		StopAD(TRUE);
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
		if (m_pacqD->baudiblesound)
			Beep(500, 400);
		ASSERT( m_bADinprogress ==FALSE);
	}
	m_btnStartStop.SetCheck(m_bADinprogress);
	// change display
	m_ADsourceView.Invalidate();
}

// --------------------------------------------------------------------------
// change total sweep duration - modify buffers

void CADContView::OnEnChangeDuration() 
{
	if (!mm_sweepduration.m_bEntryDone)
		return;

	float duration = m_sweepduration;
	switch (mm_sweepduration.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE);
		duration = m_sweepduration;
		m_sweepduration = 0.f;
		break;
	case VK_UP:
	case VK_PRIOR:
		duration++;
		break;
	case VK_DOWN:
	case VK_NEXT:   
		duration--;	
		break;
	}
	// check boundaries	
	if (duration < MIN_DURATION)	// minimum duration
		duration = (float) MIN_DURATION;
	// max duration ?

	// change display if necessary
	BOOL flag=FALSE;
	if (m_sweepduration != duration)
	{
		StopAD(TRUE);
		UpdateStartStop(FALSE);
		CWaveFormat* pWFormat = &(m_pacqD->waveFormat);
		pWFormat->buffersize = (WORD) (duration 
			* pWFormat->chrate
			* pWFormat->scan_count)
			/ pWFormat->bufferNitems;
		AD_DeclareBuffers();
	}
	// update CMyEdit control
	mm_sweepduration.m_bEntryDone=FALSE;	// clear flag
	mm_sweepduration.m_nChar=0;				// empty buffer
	mm_sweepduration.SetSel(0, -1);			// select all text
	// update data stored
	m_pacqD->sweepduration = m_sweepduration;
	// display result
	UpdateData(FALSE);
	return;
}

// --------------------------------------------------------------------------

//void CADContView::OnWritetoDisk() 
//{
//	UpdateData(TRUE);
//	BOOL bADon = m_bADinprogress;		// save state of data acquisition
//	if (bADon)							// if acquisition running, stop it
//	{
//		StopAD(TRUE);
//		UpdateStartStop(FALSE);
//	}
//	m_pacqD->waveFormat.bADwritetofile = m_bADwritetofile;
//	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
//	if (bADon)							// if acquisition was running, restart
//		OnBnClickedStartstop();
//}

// --------------------------------------------------------------------------

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
		dlg.m_pacqD = m_pacqD;
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
		m_pacqD->exptnumber++;
		csBufTemp.Format(_T("%06.6lu"), m_pacqD->exptnumber);
		csfilename = m_pacqD->csPathname + m_pacqD->csBasename + csBufTemp + _T(".dat");
	
		// check if this file is already present, exit if not...
		CFileStatus status;
		int iIDresponse=IDYES;	// default: go on if file not found
		if (CFile::GetStatus(csfilename, status))
			iIDresponse = AfxMessageBox(IDS_FILEOVERWRITE, MB_YESNO | MB_ICONWARNING);
		// no .. find first available number
		if (IDNO == iIDresponse)
		{
			BOOL flag = TRUE;
			while (flag)
			{
				m_pacqD->exptnumber++;
				csBufTemp.Format(_T("%06.6lu"), m_pacqD->exptnumber);
				csfilename = m_pacqD->csPathname + m_pacqD->csBasename + csBufTemp + _T(".dat");
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

/////////////////////////////////////////////////////////////////////////////////////
// HARDWARE
/////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// OnHardwareAdchannels
#include "adinputparmsdlg.h"

void CADContView::OnHardwareAdchannels() 
{
	if (m_bADinprogress)
	{
		StopAD(TRUE);
		UpdateStartStop(FALSE);
	}
	CADInputParmsDlg dlg;

	// init dialog data
	dlg.m_pwFormat = &(m_pacqD->waveFormat);
	dlg.m_pchArray = &(m_pacqD->chanArray);
	dlg.m_numchansMAXDI = m_ADC.GetSSCaps(OLSSC_MAXDICHANS);
	dlg.m_numchansMAXSE = m_ADC.GetSSCaps(OLSSC_MAXSECHANS);
	dlg.m_bchantype = m_pacqD->bChannelType;
	dlg.m_bchainDialog= TRUE;
	dlg.m_bcommandAmplifier = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		// make sure that buffer size is a multiple of the nb of chans
		m_pacqD->bChannelType = dlg.m_bchantype;
		AD_InitSubSystem();
		UpdateData(FALSE);
		UpdateChanLegends(0);
		UpdateHorizontalRulerBar();
		UpdateChanVerticalRulerBar(0);

		if (dlg.m_postmessage != NULL)
			ChainDialog(dlg.m_postmessage);	
	}
}

// --------------------------------------------------------------------------
// OnHardwareAdintervals

void CADContView::OnHardwareAdintervals()
{
	if (m_bADinprogress)
	{
		StopAD(TRUE);
		UpdateStartStop(FALSE);
	}
	ADIntervalsDlg dlg;
	// init dialog data 
	CWaveFormat* pWFormat	=&(m_pacqD->waveFormat);
	dlg.m_pwaveFormat		=pWFormat;
	dlg.m_ratemin			=1.0f;
	dlg.m_ratemax			=(float) (m_freqmax / pWFormat->scan_count);
	dlg.m_bufferWsizemax	=(UINT) 65536*4;
	dlg.m_undersamplefactor =m_pacqD->iundersample;
	dlg.m_baudiblesound		=m_pacqD->baudiblesound;
	dlg.m_sweepduration		=m_sweepduration;
	dlg.m_bchainDialog = TRUE;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		m_pacqD->iundersample	= dlg.m_undersamplefactor;
		m_pacqD->baudiblesound	= dlg.m_baudiblesound;
		m_sweepduration			= dlg.m_sweepduration;
		m_pacqD->sweepduration	= m_sweepduration;
		AD_InitSubSystem();
		UpdateData(FALSE);
	
		if (dlg.m_postmessage != NULL)
			ChainDialog(dlg.m_postmessage);
	}
}

// --------------------------------------------------------------------------
// Chaindialog
// some dialogs return an ID in a parameter
// (OnHardware.. adchannels, adtrigger, adintervals)
// this routine launch the dialog box corresponding to this ID
// ! this ID must be different from the menu_ID, otherwise, when the
// corresponding button is depressed, the dialog box is CALLED over the
// current one..

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

/////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENTSINK_MAP(CADContView, CFormView)
	//{{AFX_EVENTSINK_MAP(CADContView)
	ON_EVENT(CADContView, IDC_ANALOGTODIGIT, 1 /* BufferDone */, OnBufferDone_ADC, VTS_NONE)
	ON_EVENT(CADContView, IDC_ANALOGTODIGIT, 2 /* QueueDone */, OnQueueDone_ADC, VTS_NONE)
	ON_EVENT(CADContView, IDC_ANALOGTODIGIT, 4 /* TriggerError */, OnTriggerError_ADC, VTS_NONE)
	ON_EVENT(CADContView, IDC_ANALOGTODIGIT, 5 /* OverrunError */, OnOverrunError_ADC, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CADContView, IDC_DIGITTOANALOG, 1, CADContView::OnBufferDone_DAC, VTS_NONE)
	ON_EVENT(CADContView, IDC_DIGITTOANALOG, 5, CADContView::OnOverrunError_DAC, VTS_NONE)
	ON_EVENT(CADContView, IDC_DIGITTOANALOG, 2, CADContView::OnQueueDone_DAC, VTS_NONE)
	ON_EVENT(CADContView, IDC_DIGITTOANALOG, 4, CADContView::OnTriggerError_DAC, VTS_NONE)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////

void CADContView::OnTriggerError_ADC() 
{
	StopAD(TRUE);
	UpdateStartStop(FALSE);
	AfxMessageBox(IDS_ACQDATA_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

// --------------------------------------------------------------------------

void CADContView::OnQueueDone_ADC() 
{
	StopAD(TRUE);
	UpdateStartStop(FALSE);
	AfxMessageBox(IDS_ACQDATA_TOOFAST);
}

// --------------------------------------------------------------------------

void CADContView::OnOverrunError_ADC() 
{
	StopAD(TRUE);
	UpdateStartStop(FALSE);
	AfxMessageBox(IDS_ACQDATA_OVERRUN);
}

// --------------------------------------------------------------------------

void CADContView::OnBufferDone_ADC() 
{
	// get buffer off done list	
	m_ADbufhandle = (HBUF)m_ADC.GetQueue();
	if (m_ADbufhandle == NULL)
		return;

	// get pointer to buffer
	short* pDTbuf;
	m_ecode = olDmGetBufferPtr(m_ADbufhandle,(void **)&pDTbuf);
	if (m_ecode == OLNOERROR)
	{
		// update length of data acquired
		AD_Transfer(pDTbuf);										// transfer data into intermediary buffer

		CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();	// pointer to data descriptor
		pWFormat->sample_count += m_bytesweepRefresh/2;				// update total sample count
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
				StopAD(TRUE);
				if (m_bhidesubsequent)
				{
					if(!StartAD())
						StopAD(TRUE);	// if bADStart = wrong, then stop AD
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
		m_ADC.SetQueue((long)m_ADbufhandle);						// tell ADdriver that data buffer is free

		// then: display acqDataDoc buffer
		if (pWFormat->bOnlineDisplay)								// display data if requested
			m_ADsourceView.ADdisplayBuffer(pdataBuf, m_chsweepRefresh);
		CString cs;
		cs.Format(_T("%.3lf"), duration);								// update total time on the screen
		SetDlgItemText(IDC_STATIC1, cs);							// update time elapsed
	}
}


// --------------------------------------------------------------------------
// transfert DT buffer to acqDataDoc buffer

void CADContView::AD_Transfer(short* pDTbuf0)
{
	// get pointer to file waveFormat
	CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();
	short* pdataBuf = m_inputDataFile.GetpRawDataBUF();		// acqDataDoc buffer

	m_chsweep1=m_chsweep2+1;								// update data abcissa on the screen 
	if (m_chsweep1 >= m_chsweeplength)						// if data are out of the screen, wrap
		m_chsweep1 = 0;
	m_chsweep2 = m_chsweep1 + m_chDTbuflen -1;				// abcissa of the last data point
	m_chsweepRefresh = m_chsweep2 - m_chsweep1 + 1;			// number of data points to refresh on the screen
	pdataBuf += (m_chsweep1 * pWFormat->scan_count);

	// if offset binary (unsigned words), transform data into signed integers (two's complement)
	if ((m_pacqD->waveFormat).binzero != NULL)
	{
		WORD binzero = (WORD) (m_pacqD->waveFormat).binzero;
		WORD* pDTbuf = (WORD*) pDTbuf0;
		for (int j = 0; j< m_ADbuflen; j++, pDTbuf++ ) 
			*pDTbuf -= binzero;
	}

	// no undersampling.. copy DTbuffer into data file buffer
	if (m_pacqD->iundersample <= 1)
	{
		memcpy(pdataBuf, pDTbuf0, m_ADbuflen*sizeof(short));
	}
	// undersampling (assume that buffer length is a multiple of iundersample) and copy into data file buffer
	else 
	{
		short* pdataBuf2 = pdataBuf;
		short* pDTbuf = pDTbuf0;
		int iundersample = m_pacqD->iundersample;
		m_chsweepRefresh = m_chsweepRefresh / iundersample;
		// loop and compute average between consecutive points
		for (int j = 0; j< pWFormat->scan_count; j++, pdataBuf2++, pDTbuf++)
		{
			short* pSource	= pDTbuf;
			short* pDest	= pdataBuf2;
			for (int i=0; i< m_chDTbuflen; i+= iundersample)
			{
				long SUM = 0;
				for (int k = 0; k< iundersample; k++)
				{
					SUM += *pSource;
					pSource += pWFormat->scan_count;
				}
				*pDest = (short) (SUM/iundersample);
				pDest += pWFormat->scan_count;
			}
		}
	}
	// update byte length of buffer
	m_bytesweepRefresh = m_chsweepRefresh * sizeof(short) * pWFormat->scan_count;
}

// --------------------------------------------------------------------------

BOOL CADContView::InitCyberAmp()
{
	CCyberAmp m_cyber;
	BOOL bcyberPresent = FALSE;
	int nchans= (m_pacqD->chanArray).ChannelGetnum();
	
	// test if Cyberamp320 selected
	for (int i = 0; i < nchans; i++)
	{
		CWaveChan* pchan = (m_pacqD->chanArray).GetWaveChan(i);

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
			m_cyber.SetHPFilter(pchan->am_amplifierchan, C300_POSINPUT,	 pchan->am_csInputpos);
			m_cyber.SetmVOffset(pchan->am_amplifierchan, pchan->am_offset);

			m_cyber.SetNotchFilter(pchan->am_amplifierchan, pchan->am_notchfilt);
			m_cyber.SetGain(pchan->am_amplifierchan, (int)(pchan->am_totalgain / (pchan->am_gainheadstage*pchan->am_adgain)));
			m_cyber.SetLPFilter(pchan->am_amplifierchan, (int)(pchan->am_lowpass));
			int errorcode = m_cyber.C300_FlushCommandsAndAwaitResponse();
		}
	}
	return bcyberPresent;
}

// --------------------------------------------------------------------------
// OnClickedGainbutton
// change state of both buttons and update scroll bar (vertical)
// -------------------------------------------------------------------------- 

void CADContView::OnBnClickedGainbutton()
{
	((CButton*) GetDlgItem(IDC_BIAS_button))->SetState(0);
	((CButton*) GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

// --------------------------------------------------------------------------
// OnClickedBiasbutton
// change state of the buttons, update scroll bar (vertical)
// --------------------------------------------------------------------------

void CADContView::OnBnClickedBiasbutton()
{
	// set bias down and set gain up CButton	
	((CButton*) GetDlgItem(IDC_BIAS_button))->SetState(1);
	((CButton*) GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

// --------------------------------------------------------------------------
// OnVScroll
// --------------------------------------------------------------------------

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

// --------------------------------------------------------------------------
// SetVBarMode
// --------------------------------------------------------------------------
void CADContView::SetVBarMode (short bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}

// --------------------------------------------------------------------------
// OnGainScroll()
// --------------------------------------------------------------------------
void CADContView::OnGainScroll(UINT nSBCode, UINT nPos)
{
	// assume that all channels are displayed at the same gain & offset
	int ichan = 0;			// TODO see which channel is selected
	int lSize = m_ADsourceView.GetChanlistYextent(ichan);	

	// get corresponding data
	switch (nSBCode)
	{
	// .................scroll to the start
	case SB_LEFT:		lSize = YEXTENT_MIN; break;
	// .................scroll one line left
	case SB_LINELEFT:	lSize -= lSize/10 +1; break;
	// .................scroll one line right
	case SB_LINERIGHT:	lSize += lSize/10 +1; break;
	// .................scroll one page left
	case SB_PAGELEFT:	lSize -= lSize/2+1; break;
	// .................scroll one page right
	case SB_PAGERIGHT:	lSize += lSize+1; break;
	// .................scroll to end right
	case SB_RIGHT:		lSize = YEXTENT_MAX; break;
	// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	lSize = MulDiv(nPos-50,YEXTENT_MAX,100); break;
	// .................NOP: set position only
	default:			break;			
	}

	// change y extent
	if (lSize>0 ) //&& lSize<=YEXTENT_MAX)
	{
		// assume that all channels are displayed at the same gain & offset
		CWaveFormat* pWFormat = &(m_pacqD->waveFormat);
		int ichanfirst = 0;
		int ichanlast = pWFormat->scan_count-1;

		for (int ichan = ichanfirst; ichan <= ichanlast; ichan++)
			m_ADsourceView.SetChanlistYextent(ichan, lSize);
		m_ADsourceView.Invalidate();
		UpdateChanLegends(0);
		UpdateHorizontalRulerBar();
		m_pacqD->izoomCursel = lSize;
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
	{
		UpdateGainScroll();
		UpdateChanLegends(0);
		UpdateData(false);
	}
	UpdateChanVerticalRulerBar(0);
}

// --------------------------------------------------------------------------
// OnBiasScroll()
// --------------------------------------------------------------------------

void CADContView::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	// assume that all channels are displayed at the same gain & offset
	int ichan = 0;			// TODO: see which channel is selected
	int lSize =  m_ADsourceView.GetChanlistYzero(ichan) - m_ADsourceView.GetChanlistBinZero(ichan);
	int yextent = m_ADsourceView.GetChanlistYextent(ichan);
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			// scroll to the start
		lSize = YZERO_MIN;
		break;
	case SB_LINELEFT:		// scroll one line left
		lSize -= yextent/100+1;
		break;
	case SB_LINERIGHT:		// scroll one line right
		lSize += yextent/100+1;
		break;
	case SB_PAGELEFT:		// scroll one page left
		lSize -= yextent/10+1;
		break;
	case SB_PAGERIGHT:		// scroll one page right
		lSize += yextent/10+1;
		break;
	case SB_RIGHT:			// scroll to end right
		lSize = YZERO_MAX;
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		lSize = (nPos-50)*(YZERO_SPAN/100);
		break;
	default:				// NOP: set position only
		break;			
	}

	// try to read data with this new size
	if (lSize>YZERO_MIN && lSize<YZERO_MAX)
	{
		CWaveFormat* pWFormat = &(m_pacqD->waveFormat);
		int ichanfirst = 0;
		int ichanlast = pWFormat->scan_count-1;
		for (int ichan = ichanfirst; ichan <= ichanlast; ichan++)
			m_ADsourceView.SetChanlistZero(ichan, lSize+ m_ADsourceView.GetChanlistBinZero(ichan));
		m_ADsourceView.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
	{
		UpdateBiasScroll();
		UpdateChanLegends(0);
		UpdateData(false);
	}
	UpdateChanVerticalRulerBar(0);
}

// --------------------------------------------------------------------------
// UpdateBiasScroll()
// -- not very nice code; interface is counterintuitive
// --------------------------------------------------------------------------

void CADContView::UpdateBiasScroll()
{
	// assume that all channels are displayed at the same gain & offset
	int ichan = 0;			// TODO see which channel is selected

	int iPos = (int) ((m_ADsourceView.GetChanlistYzero(ichan)- m_ADsourceView.GetChanlistBinZero(ichan))
					*100/(int)YZERO_SPAN)+(int)50;
	m_scrolly.SetScrollPos(iPos, TRUE);
}

void CADContView::UpdateGainScroll()
{
	// assume that all channels are displayed at the same gain & offset
	int ichan = 0;
	m_scrolly.SetScrollPos(MulDiv(m_ADsourceView.GetChanlistYextent(ichan),  100, YEXTENT_MAX)+50, TRUE);
}

void CADContView::UpdateChanLegends(int chan) 
{
	int ichan = 0;
	int yzero = m_ADsourceView.GetChanlistYzero(ichan);
	int yextent = m_ADsourceView.GetChanlistYextent(ichan);
	//int color = m_ADsourceView.GetChanlistColor(index);
	//m_colors.SetCurSel(color);
	float mVperbin= m_ADsourceView.GetChanlistVoltsperBin(ichan)*1000.0f;
	int binzero = 0;
	m_yupper=( yextent/2 +yzero -binzero)*mVperbin;
	m_ylower=(-yextent/2 +yzero -binzero)*mVperbin;
}

void CADContView::DA_FillBuffer(short* pDTbuf) 
{
	// set data within buffer
	
	// dummy parameters ----------------------------
	double WAVEFREQ = 50.0;			// 50 HZ
	CWaveFormat* pWFormat = &(m_pacqD->waveFormat);
	double OUTFREQ =  pWFormat->chrate;
	double PEAKVOLTAGE = 1.0;		// peakvoltage = 1 V
#define SINE		0
#define SQUARE		1
#define TRIANGLE	2
#define LINE		3
	int WAVEFORM = SINE;

	// dummy parameters ----------------------------
	float sweepduration = m_pacqD->sweepduration;
	long chsweeplength	= (long) (sweepduration* pWFormat->chrate / (float) m_pacqD->iundersample);
	long OUTBUFSIZE		= chsweeplength * m_pacqD->iundersample / pWFormat->bufferNitems;

	double phase = 0;
	double Freq = WAVEFREQ / OUTFREQ;

	long msbit = (long) pow(2.0,(m_DAC.GetResolution() - 1));
	long lRes = (long) pow(2.0, m_DAC.GetResolution()) - 1;

	double amp = PEAKVOLTAGE *  pow(2.0 ,m_DAC.GetResolution()) / (m_DAC.GetMaxRange() - m_DAC.GetMinRange()) ;

	switch(WAVEFORM)
	{
	case SINE: 
		{
			double pi2 = 3.1415927 * 2;
			Freq = Freq * pi2;
			for(int i=0; i< OUTBUFSIZE; i++)
			{
				*(pDTbuf + i) = (short)(cos(phase) * amp);
				// clip value
				if(*(pDTbuf + i) > msbit) 
					*(pDTbuf + i) = (short) (msbit -1);
				phase += Freq;
				if(phase > pi2) 
					phase -= pi2;
			}
		}
		break;
	case SQUARE:
		for(int i=0; i<OUTBUFSIZE; i++)
		{
			if (phase < 0) 
				*(pDTbuf + i) = (WORD)(0 - (amp /2));
			else 
				*(pDTbuf + i) = (WORD)(amp /2);

			phase += Freq;
			if(phase > 0.5) 
				phase -= 1;
		}
		break;
	case TRIANGLE: 
		for(int i=0; i<OUTBUFSIZE; i++)
		{
			*(pDTbuf + i) = (WORD)(2 * phase * amp);
			// clip value
			if(*(pDTbuf + i) >= msbit) 
				*(pDTbuf + i) = (short) (msbit -1);
			phase = phase + 2 * Freq;
			if(phase > 0.5) 
			{
				phase -= 1;
				amp--;
			}
		}
		break;
	case LINE:
		for(int i=0; i<OUTBUFSIZE; i++)
				*(pDTbuf + i) = (WORD) amp;
		break;
	default:
		break;
	}// end switch


	// convert from 2's Complement to offset binary if required
	if(m_DAC.GetEncoding() == OLx_ENC_BINARY)
	{
		for(int i=0;i<OUTBUFSIZE;i++)
			*(pDTbuf+i) = (WORD)( (*(pDTbuf+i) ^ msbit) & lRes);
	}
}

void CADContView::OnBufferDone_DAC()
{
	// get buffer off done list	
	m_DAbufhandle = (HBUF) m_DAC.GetQueue();
	if (m_DAbufhandle == NULL)
		return;

	// get pointer to buffer
	short* pDTbuf;
	m_ecode = olDmGetBufferPtr(m_DAbufhandle,(void **)&pDTbuf);

	if (m_ecode == OLNOERROR)
	{
		DA_FillBuffer(pDTbuf);
		m_DAC.SetQueue((long)m_DAbufhandle);
	}
}

void CADContView::OnOverrunError_DAC()
{
	StopDA();
	AfxMessageBox(IDS_DAC_OVERRUN);
}

void CADContView::OnQueueDone_DAC()
{
	StopDA();
	AfxMessageBox(IDS_DAC_TOOFAST);
}

void CADContView::OnTriggerError_DAC()
{
	StopDA();
	AfxMessageBox(IDS_DAC_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

//This function converts a value from the specified subsystem into a voltage.

float CADContView::ValueToVolts(CDTAcq32* pSS, long lVal, double dfGain)
{
	long lRes;
	float fMin,fMax,fVolts;
	lRes = (long)pow(2.0 ,(double)pSS->GetResolution());
	if(pSS->GetMinRange() != 0.F)
		fMin = pSS->GetMinRange() / (float)dfGain;
	else fMin = 0.F;
	if(pSS->GetMaxRange() != 0.F)
		fMax = pSS->GetMaxRange() / (float)dfGain;
	else fMax = 0.F;

	//make sure value is sign extended if 2's comp
	if(pSS->GetEncoding() == OLx_ENC_2SCOMP)
	{
		lVal = lVal & (lRes -1);
		if(lVal >= (lRes /2)) 
			lVal = lVal - lRes;
	}

	// convert it to volts
	fVolts = lVal * ((fMax - fMin) / lRes);

	// adjust DC offset
	if(pSS->GetEncoding() == OLx_ENC_2SCOMP)
		fVolts = fVolts + ((fMax + fMin)/2);
	else
		fVolts = fVolts + fMin;

	return fVolts;
}

//This function converts a voltage from the specified subsystem into a value.

long CADContView::VoltsToValue(CDTAcq32* pSS,float fVolts,double dfGain)
{
	long lRes;
	float fMin,fMax;
	long lValue;

	lRes = (long)pow(2.,(double)pSS->GetResolution());
	
	if(pSS->GetMinRange() != 0.F)
		fMin = pSS->GetMinRange() / (float)dfGain;
	else fMin = 0.F;
	if(pSS->GetMaxRange() != 0.F)
		fMax = pSS->GetMaxRange() / (float)dfGain;
	else fMax = 0.F;

	//clip input to range
	if(fVolts > fMax) fVolts = fMax;
	if(fVolts < fMin) fVolts = fMin;

	//if 2's comp encoding
	if(pSS->GetEncoding() == OLx_ENC_2SCOMP)
	{
		lValue = (long)((fVolts - (fMin + fMax) / 2) * lRes / (fMax - fMin));
		// adjust for binary wrap if any
		if(lValue == (lRes /2 )) lValue -= 1;
	}
	else
	{
		// convert to offset binary
		lValue = (long)((fVolts - fMin) * lRes / (fMax - fMin));
		// adjust for binary wrap if any
		if(lValue == lRes) lValue -= 1;
	}
	return lValue;
}

void CADContView::OnBnClickedEnableoutput()
{
	m_bOutputsEnabled = ((CButton*) GetDlgItem(IDC_ENABLEOUTPUT))->GetCheck();
	m_poutD->bAllowDA = m_bOutputsEnabled;
}

void CADContView::OnBnClickedDaparameters()
{
	CDAOutputsDlg dlg;
		dlg.poutD = m_poutD;
	if (dlg.DoModal() == IDOK)
	{
	}

}

void CADContView::OnBnClickedDaparameters2()
{
	CDAChannelsDlg dlg;
	dlg.poutD = m_poutD;
	if (dlg.DoModal() == IDOK)
	{
	}
}

void CADContView::OnEnChangeYlower()
{
	if (!mm_ylower.m_bEntryDone)
		return;

	switch (mm_ylower.m_nChar)
	{
	case VK_RETURN:
		UpdateData(TRUE);
		break;
	case VK_UP:
	case VK_PRIOR:
		m_ylower++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_ylower--;
		break;
	}	
	
	for (int i=0; i < m_ADsourceView.GetChanlistSize(); i++)
		m_ADsourceView.SetChanlistmilliVoltsMaxMin(i, m_yupper, m_ylower);
	m_ADsourceView.Invalidate();
	UpdateGainScroll();
	UpdateBiasScroll();
	int ichan = 0;
	UpdateChanLegends(ichan);
	UpdateChanVerticalRulerBar(ichan);

	mm_ylower.m_bEntryDone=FALSE;
	mm_ylower.m_nChar=0;
	mm_ylower.SetSel(0, -1); 	//select all text
}

void CADContView::OnEnChangeYupper()
{
	if (!mm_yupper.m_bEntryDone)
		return;

	switch (mm_yupper.m_nChar)
	{
	case VK_RETURN:
		UpdateData(TRUE);
		break;
	case VK_UP:
	case VK_PRIOR:
		m_yupper++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_yupper--;
		break;
	}	
	
	for (int i=0; i < m_ADsourceView.GetChanlistSize(); i++)
		m_ADsourceView.SetChanlistmilliVoltsMaxMin(i, m_yupper, m_ylower);
	m_ADsourceView.Invalidate();
	UpdateGainScroll();
	UpdateBiasScroll();
	int ichan = 0;
	UpdateChanLegends(ichan);
	UpdateChanVerticalRulerBar(ichan);

	mm_yupper.m_bEntryDone=FALSE;
	mm_yupper.m_nChar=0;
	mm_yupper.SetSel(0, -1); 	//select all text
}


// Write to disk
void CADContView::OnBnClickedRadio1()
{
	//if (m_bADwritetofile)
	//	return;
	m_bADwritetofile=TRUE;
	m_pacqD->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;

//	
//	BOOL bADon = m_bADinprogress;		// save state of data acquisition
//	if (bADon)							// if acquisition running, stop it
//	{
//		StopAD(TRUE);
//		UpdateStartStop(FALSE);
//	}
//	m_pacqD->waveFormat.bADwritetofile = m_bADwritetofile;
//	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
//	if (bADon)							// if acquisition was running, restart
//		OnBnClickedStartstop();
//}

}

// Oscilloscope
void CADContView::OnBnClickedRadio2()
{
	m_bADwritetofile=FALSE;
	m_pacqD->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CADContView::UpdateRadioButtons()
{
	// set the oscilloscope / write to disk button
	if (m_bADwritetofile)
		((CButton * )GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
	else
		((CButton * )GetDlgItem(IDC_RADIO2))->SetCheck(BST_CHECKED);
	UpdateData();
}