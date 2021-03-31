#include "StdAfx.h"
#include <math.h>
#include "resource.h"
#include "AcqDataDoc.h"
//#include "Editctrl.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "Adinterv.h"
#include "Adexperi.h"
#include "dtacq32.h"
#include "CyberAmp.h"
//#include "Cscale.h"
#include "chart.h"
#include "ChartData.h"
#include "./include/DataTranslation/Olxdadefs.h"
#include "./include/DataTranslation/Olxdaapi.h"
#include "CViewADcontinuous.h"
#include "ChildFrm.h"
#include "MainFrm.h"
#include "DlgConfirmSave.h"
#include "DlgDAChannels.h"
//#include "DlgDAOutputsParms.h"
#include "DlgdtBoard.h"
#include "DlgADInputParms.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MIN_DURATION 0.1
#define MIN_BUFLEN	32

IMPLEMENT_DYNCREATE(CViewADContinuous, CFormView)

CViewADContinuous::CViewADContinuous()
	: CFormView(CViewADContinuous::IDD)
	, m_bStartOutPutMode(0), m_cursorstate(0), m_hBias(nullptr), m_hZoom(nullptr), m_yscaleFactor(0), m_VBarMode(0),
	m_bFoundDTOPenLayerDLL(0), m_ecode(0), m_pADC_options(nullptr), m_ADC_bufhandle(nullptr), m_ADC_buflen(0),
	m_pDAC_options(nullptr), m_DACdigitalchannel(0), m_DACdigitalfirst(0), m_DAClistsize(0), m_DACmsbit(0),
	m_DAClRes(0), m_DAC_bufhandle(nullptr), m_DAC_buflen(0), m_DAC_chbuflen(0), m_DAC_nBuffersFilledSinceStart(0),
	m_DAC_frequency(0), m_sweeplength(0), m_chsweep1(0)
	//, m_chsweep2(0), m_chsweepRefresh(0), m_bytesweepRefresh(0), m_fclockrate(0)
{
	m_sweepduration = 1.0f;
	m_bADwritetofile = FALSE;
	m_ptableSet = nullptr;
	m_ADC_inprogress = FALSE; // no A/D in progress
	m_DAC_inprogress = FALSE; // no D/A in progress
	m_bchanged = FALSE; // data unchanged
	m_bAskErase = FALSE; // no warning when data are erased
	m_chsweeplength = 0;
	m_ADC_chbuflen = 0;
	m_bFileOpen = FALSE;
	m_numchansMAX = 16;
	m_freqmax = 50000.f;
	m_bSimultaneousStart = FALSE;
	m_bhidesubsequent = FALSE;

	m_BkColor = GetSysColor(COLOR_BTNFACE); // set color for edit button
	m_pEditBkBrush = new CBrush(m_BkColor); // background color = like a button
	ASSERT(m_pEditBkBrush != NULL); // check brush

	m_bEnableActiveAccessibility = FALSE;
	m_bsimultaneousStartAD = FALSE;
	m_bsimultaneousStartDA = FALSE;
	m_ADC_yRulerBar.AttachScopeWnd(&m_ChartDataWnd, FALSE);
	m_bADC_IsPresent = FALSE;
	m_bDAC_IsPresent = FALSE;
}

CViewADContinuous::~CViewADContinuous()
{
	for (auto i = 0; i < alligatorparameters_ptr_array.GetSize(); i++)
	{
		const auto ptr = alligatorparameters_ptr_array.GetAt(i);
		delete ptr;
	}
}

void CViewADContinuous::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANALOGTODIGIT, m_ADC_DTAcq32);
	DDX_Control(pDX, IDC_DIGITTOANALOG, m_DAC_DTAcq32);
	DDX_Control(pDX, IDC_COMBOBOARD, m_ADcardCombo);
	DDX_Control(pDX, IDC_STARTSTOP, m_btnStartStop);
	DDX_CBIndex(pDX, IDC_COMBOSTARTOUTPUT, m_bStartOutPutMode);

	DDX_Control(pDX, IDC_USBPXXS1CTL, m_Alligator);
}

BEGIN_MESSAGE_MAP(CViewADContinuous, CFormView)
	ON_MESSAGE(WM_MYMESSAGE, &CViewADContinuous::OnMyMessage)
	ON_COMMAND(ID_HARDWARE_ADCHANNELS, &CViewADContinuous::ADC_OnHardwareChannelsDlg)
	ON_COMMAND(ID_HARDWARE_ADINTERVALS, &CViewADContinuous::ADC_OnHardwareIntervalsDlg)
	ON_COMMAND(ID_HARDWARE_DEFINEEXPERIMENT, &CViewADContinuous::ADC_OnHardwareDefineexperiment)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ADPARAMETERS, &CViewADContinuous::ADC_OnHardwareIntervalsDlg)
	ON_BN_CLICKED(IDC_ADPARAMETERS2, &CViewADContinuous::ADC_OnHardwareChannelsDlg)
	ON_BN_CLICKED(IDC_GAIN_button, &CViewADContinuous::OnBnClickedGainbutton)
	ON_BN_CLICKED(IDC_BIAS_button, &CViewADContinuous::OnBnClickedBiasbutton)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_DAPARAMETERS2, &CViewADContinuous::DAC_OnBnClickedParameters2)
	ON_CBN_SELCHANGE(IDC_COMBOBOARD, &CViewADContinuous::OnCbnSelchangeComboboard)
	ON_BN_CLICKED(IDC_STARTSTOP, &CViewADContinuous::ADC_OnBnClickedStartstop)
	ON_BN_CLICKED(IDC_WRITETODISK, &CViewADContinuous::OnBnClickedWriteToDisk)
	ON_BN_CLICKED(IDC_OSCILLOSCOPE, &CViewADContinuous::OnBnClickedOscilloscope)
	ON_BN_CLICKED(IDC_CARDFEATURES, &CViewADContinuous::OnBnClickedCardFeatures)
	ON_CBN_SELCHANGE(IDC_COMBOSTARTOUTPUT, &CViewADContinuous::OnCbnSelchangeCombostartoutput)
	ON_BN_CLICKED(IDC_STARTSTOP2, &CViewADContinuous::DAC_OnBnClickedStartStop)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CViewADContinuous, CFormView)
	ON_EVENT(CViewADContinuous, IDC_ANALOGTODIGIT, 1, CViewADContinuous::ADC_OnBufferDone, VTS_NONE)
	ON_EVENT(CViewADContinuous, IDC_ANALOGTODIGIT, 2, CViewADContinuous::ADC_OnQueueDone, VTS_NONE)
	ON_EVENT(CViewADContinuous, IDC_ANALOGTODIGIT, 4, CViewADContinuous::ADC_OnTriggerError, VTS_NONE)
	ON_EVENT(CViewADContinuous, IDC_ANALOGTODIGIT, 5, CViewADContinuous::ADC_OnOverrunError, VTS_NONE)

	ON_EVENT(CViewADContinuous, IDC_DIGITTOANALOG, 1, CViewADContinuous::DAC_OnBufferDone, VTS_NONE)
	ON_EVENT(CViewADContinuous, IDC_DIGITTOANALOG, 5, CViewADContinuous::DAC_OnOverrunError, VTS_NONE)
	ON_EVENT(CViewADContinuous, IDC_DIGITTOANALOG, 2, CViewADContinuous::DAC_OnQueueDone, VTS_NONE)
	ON_EVENT(CViewADContinuous, IDC_DIGITTOANALOG, 4, CViewADContinuous::DAC_OnTriggerError, VTS_NONE)

	ON_EVENT(CViewADContinuous, IDC_USBPXXS1CTL, 1, CViewADContinuous::DeviceConnectedUsbpxxs1ctl1, VTS_I4)
	ON_EVENT(CViewADContinuous, IDC_USBPXXS1CTL, 2, CViewADContinuous::DeviceDisconnectedUsbpxxs1ctl1, VTS_I4)
END_EVENTSINK_MAP()

void CViewADContinuous::OnDestroy()
{
	if (m_ADC_inprogress)
		OnStop(TRUE);

	if (m_DAC_inprogress)
		DAC_StopAndLiberateBuffers();

	if (m_bFoundDTOPenLayerDLL)
	{
		// TODO: save data here
		if (m_ADC_DTAcq32.GetHDass() != NULL)
			ADC_DeleteBuffers();

		if (m_DAC_DTAcq32.GetHDass() != NULL)
			DAC_DeleteBuffers();
	}
	delete m_pEditBkBrush;		// brush for edit controls
	CFormView::OnDestroy();
}

HBRUSH CViewADContinuous::OnCtlColor(CDC* p_dc, CWnd* p_wnd, const UINT n_ctl_color)
{
	HBRUSH hbr;
	switch (n_ctl_color)
	{
	case CTLCOLOR_EDIT:
	case CTLCOLOR_MSGBOX:
		// Set color to green on black and return the background brush.
		p_dc->SetBkColor(m_BkColor);
		hbr = static_cast<HBRUSH>(m_pEditBkBrush->GetSafeHandle());
		break;

	default:
		hbr = CFormView::OnCtlColor(p_dc, p_wnd, n_ctl_color);
		break;
	}
	return hbr;
}

void CViewADContinuous::OnCbnSelchangeComboboard()
{
	const auto isel = m_ADcardCombo.GetCurSel();
	CString cs_card_name;
	m_ADcardCombo.GetLBText(isel, cs_card_name);
	SelectDTOpenLayersBoard(cs_card_name);
}

BOOL CViewADContinuous::FindDTOpenLayersBoards()
{
	m_ADcardCombo.ResetContent();

	// load board name - skip dialog if only one is present
	const UINT ui_num_boards = m_ADC_DTAcq32.GetNumBoards();
	if (ui_num_boards == 0)
	{
		m_ADcardCombo.AddString(_T("No Board"));
		m_ADcardCombo.SetCurSel(0);
		return FALSE;
	}

	for (UINT i = 0; i < ui_num_boards; i++)
		m_ADcardCombo.AddString(m_ADC_DTAcq32.GetBoardList(static_cast<short>(i)));

	auto isel = 0;
	// if name already defined, check if board present
	if (!(m_pADC_options->waveFormat).csADcardName.IsEmpty())
		isel = m_ADcardCombo.FindString(-1, (m_pADC_options->waveFormat).csADcardName);
	if (isel < 0)
		isel = 0;

	m_ADcardCombo.SetCurSel(isel);
	m_boardName = m_ADC_DTAcq32.GetBoardList(isel);
	SelectDTOpenLayersBoard(m_boardName);
	return TRUE;
}

BOOL CViewADContinuous::SelectDTOpenLayersBoard(const CString& card_name)
{
	// get infos
	m_bFoundDTOPenLayerDLL = TRUE;
	(m_pADC_options->waveFormat).csADcardName = card_name;

	// connect A/D subsystem and display/hide buttons
	m_bStartOutPutMode = 0;
	m_bADC_IsPresent = ADC_OpenSubSystem(card_name);
	m_bDAC_IsPresent = DAC_OpenSubSystem(card_name);
	if (m_bDAC_IsPresent)
		m_bStartOutPutMode = 0;
	m_bSimultaneousStart = m_bsimultaneousStartDA && m_bsimultaneousStartAD;

	// display additional interface elements
	auto b_show = (m_bADC_IsPresent ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_ADPARAMETERS)->ShowWindow(b_show);
	GetDlgItem(IDC_ADPARAMETERS2)->ShowWindow(b_show);
	GetDlgItem(IDC_COMBOSTARTOUTPUT)->ShowWindow(b_show);

	b_show = (m_bDAC_IsPresent ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DAPARAMETERS2)->ShowWindow(b_show);
	GetDlgItem(IDC_DAGROUP)->ShowWindow(b_show);
	GetDlgItem(IDC_COMBOSTARTOUTPUT)->ShowWindow(b_show);
	GetDlgItem(IDC_STARTSTOP2)->ShowWindow(b_show);
	SetCombostartoutput(m_pDAC_options->bAllowDA);

	return TRUE;
}

BOOL CViewADContinuous::ADC_OpenSubSystem(const CString card_name)
{
	try
	{
		m_ADC_DTAcq32.SetBoard(card_name);
		const auto n_ad_elements = m_ADC_DTAcq32.GetDevCaps(OLDC_ADELEMENTS);
		if (n_ad_elements < 1)
			return FALSE;
		m_ADC_DTAcq32.SetSubSysType(OLSS_AD);
		m_ADC_DTAcq32.SetSubSysElement(0);
		ASSERT(m_ADC_DTAcq32.GetHDass() != NULL);
		m_bsimultaneousStartAD = m_ADC_DTAcq32.GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	}

	// save parameters into CWaveFormat
	const auto p_wave_format = &(m_pADC_options->waveFormat);
	const auto max = m_ADC_DTAcq32.GetMaxRange(); // maximum input voltage
	const auto min = m_ADC_DTAcq32.GetMinRange(); // minimum input voltage
	p_wave_format->fullscale_Volts = static_cast<float>(max - min);

	// convert into bin scale (nb of divisions)
	const int iresolution = m_ADC_DTAcq32.GetResolution();
	p_wave_format->binspan = ((1L << iresolution) - 1);

	// data encoding (binary or offset encoding)
	p_wave_format->mode_encoding = static_cast<int>(m_ADC_DTAcq32.GetEncoding());
	if (p_wave_format->mode_encoding == OLx_ENC_BINARY)
		p_wave_format->binzero = p_wave_format->binspan / 2 + 1;
	else if (p_wave_format->mode_encoding == OLx_ENC_2SCOMP)
		p_wave_format->binzero = 0;

	// load infos concerning frequency, dma chans, programmable gains
	m_freqmax = m_ADC_DTAcq32.GetSSCapsEx(OLSSCE_MAXTHROUGHPUT);
	// TODO tell sourceview here under which format are data
	// TODO save format of data into temp document
	// float volts = (float) ((wave_format->fullscale_Volts)
	//				/(wave_format->binspan) * value  -wave_format->fullscale_Volts/2);
	// TODO: update max min of chan 1 with gain && instrumental gain
	//UpdateChanLegends(0);
	//UpdateHorizontalRulerBar();
	//UpdateVerticalRulerBar();

	return TRUE;
}

BOOL CViewADContinuous::ADC_InitSubSystem()
{
	try
	{
		ASSERT(m_ADC_DTAcq32.GetHDass() != NULL);

		// store all values within global parameters array
		const auto p_acq_dwave_format = &(m_pADC_options->waveFormat);

		// Set up the ADC - no wrap so we can get buffer reused
		m_ADC_DTAcq32.SetDataFlow(OLx_DF_CONTINUOUS);
		m_ADC_DTAcq32.SetWrapMode(OLx_WRP_NONE);
		m_ADC_DTAcq32.SetDmaUsage(static_cast<short>(m_ADC_DTAcq32.GetSSCaps(OLSSC_NUMDMACHANS)));
		m_ADC_DTAcq32.SetClockSource(OLx_CLK_INTERNAL);

		// set trigger mode
		int trig = p_acq_dwave_format->trig_mode;
		if (trig > OLx_TRG_EXTRA)
			trig = 0;
		m_ADC_DTAcq32.SetTrigger(trig);

		// number of channels
		//if (m_pADC_options->bChannelType == OLx_CHNT_SINGLEENDED && m_ADC_DTAcq32.GetSSCaps(OLSSC_SUP_SINGLEENDED) == NULL)
		//	m_pADC_options->bChannelType = OLx_CHNT_DIFFERENTIAL;
		//if (m_pADC_options->bChannelType == OLx_CHNT_DIFFERENTIAL && m_ADC_DTAcq32.GetSSCaps(OLSSC_SUP_DIFFERENTIAL) == NULL)
		//	m_pADC_options->bChannelType = OLx_CHNT_SINGLEENDED;

		m_ADC_DTAcq32.SetChannelType(m_pADC_options->bChannelType);
		if (m_pADC_options->bChannelType == OLx_CHNT_SINGLEENDED)
			m_numchansMAX = m_ADC_DTAcq32.GetSSCaps(OLSSC_MAXSECHANS);
		else
			m_numchansMAX = m_ADC_DTAcq32.GetSSCaps(OLSSC_MAXDICHANS);
		// limit scan_count to m_numchansMAX -
		// this limits the nb of data acquisition channels to max-1 if one wants to use the additional I/O input "pseudo"channel
		// so far, it seems acceptable...
		if (p_acq_dwave_format->scan_count > m_numchansMAX)
			p_acq_dwave_format->scan_count = m_numchansMAX;

		// set frequency to value requested, set frequency and get the value returned
		auto clockrate = double(p_acq_dwave_format->chrate) * p_acq_dwave_format->scan_count;
		m_ADC_DTAcq32.SetFrequency(clockrate);			// set sampling frequency (total throughput)
		clockrate = m_ADC_DTAcq32.GetFrequency();
		p_acq_dwave_format->chrate = float(clockrate) / p_acq_dwave_format->scan_count;

		// update channel list (chan & gain)
		m_ADC_DTAcq32.SetListSize(p_acq_dwave_format->scan_count);
		for (auto i = 0; i < p_acq_dwave_format->scan_count; i++)
		{
			// transfer data from CWaveChan to chanlist of the A/D subsystem
			const auto p_channel = (m_pADC_options->chanArray).Get_p_channel(i);
			if (p_channel->am_adchannel > m_numchansMAX - 1 && p_channel->am_adchannel != 16)
				p_channel->am_adchannel = m_numchansMAX - 1;
			m_ADC_DTAcq32.SetChannelList(i, p_channel->am_adchannel);
			m_ADC_DTAcq32.SetGainList(i, p_channel->am_gainAD);
			const auto d_gain = m_ADC_DTAcq32.GetGainList(i);
			p_channel->am_gainAD = static_cast<short>(d_gain);
			// compute dependent parameters
			p_channel->am_gainamplifier = double(p_channel->am_gainheadstage) * p_channel->am_gainpre * p_channel->am_gainpost;
			p_channel->am_gaintotal = p_channel->am_gainamplifier * p_channel->am_gainAD;
			p_channel->am_resolutionV = p_acq_dwave_format->fullscale_Volts / p_channel->am_gaintotal / p_acq_dwave_format->binspan;
		}

		// pass parameters to the board and check if errors
		m_ADC_DTAcq32.ClearError();
		m_ADC_DTAcq32.Config();
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

void CViewADContinuous::ADC_DeclareBuffers()
{
	// close data buffers
	ADC_DeleteBuffers();

	// make sure that buffer length contains at least nacq chans
	CWaveFormat* wave_format = &(m_pADC_options->waveFormat); // get pointer to m_pADC_options wave format
	if (wave_format->buffersize < wave_format->scan_count * m_pADC_options->iundersample)
		wave_format->buffersize = wave_format->scan_count * m_pADC_options->iundersample;

	// define buffer length
	m_sweepduration = m_pADC_options->sweepduration;
	m_chsweeplength = static_cast<long>(m_sweepduration * wave_format->chrate / static_cast<float>(m_pADC_options->iundersample));
	m_ADC_chbuflen = m_chsweeplength * m_pADC_options->iundersample / wave_format->bufferNitems;
	m_ADC_buflen = m_ADC_chbuflen * wave_format->scan_count;

	for (auto i = 0; i < wave_format->bufferNitems; i++)
	{
		const auto ecode = olDmAllocBuffer(0, m_ADC_buflen, &m_ADC_bufhandle);
		//ecode = OLNOERROR;
		if ((ecode == OLNOERROR) && (m_ADC_bufhandle != nullptr))
			m_ADC_DTAcq32.SetQueue(long(m_ADC_bufhandle)); // but buffer onto Ready queue
	}

	// set sweep length to the nb of data buffers
	(m_inputDataFile.GetpWaveFormat())->sample_count = m_chsweeplength * long(wave_format->scan_count);	// ?
	m_inputDataFile.AdjustBUF(m_chsweeplength);
	*(m_inputDataFile.GetpWaveFormat()) = *wave_format;	// save settings into data file

														// update display length (and also the text - abcissa)
	m_ChartDataWnd.AttachDataFile(&m_inputDataFile);
	m_ChartDataWnd.ResizeChannels(0, m_chsweeplength);
	if (m_ChartDataWnd.GetChanlistSize() != wave_format->scan_count)
	{
		m_ChartDataWnd.RemoveAllChanlistItems();
		for (auto j = 0; j < wave_format->scan_count; j++)
		{
			m_ChartDataWnd.AddChanlistItem(j, 0);
		}
	}

	// adapt source view
	auto p_app = (CdbWaveApp*)AfxGetApp();
	auto options_viewdata = &(p_app->options_viewdata);
	m_ChartDataWnd.SetScopeParameters(&(options_viewdata->viewdata));
	m_ChartDataWnd.Invalidate();
	UpdateData(FALSE);
}

void CViewADContinuous::ADC_DeleteBuffers()
{
	try {
		if (m_ADC_DTAcq32.GetHDass() == NULL)
			return;
		m_ADC_DTAcq32.Flush();	// clean
		HBUF h_buf;			// handle to buffer
		do {				// loop until all buffers are removed
			h_buf = HBUF(m_ADC_DTAcq32.GetQueue());
			if (h_buf != nullptr)
				if (olDmFreeBuffer(h_buf) != OLNOERROR)
					AfxMessageBox(_T("Error Freeing Buffer"));
		} while (h_buf != nullptr);
		m_ADC_bufhandle = h_buf;
	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError)); myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
	}
}

void CViewADContinuous::ADC_StopAndLiberateBuffers()
{
	try {
		m_ADC_DTAcq32.Stop();
		m_ADC_DTAcq32.Flush();							// flush all buffers to Done Queue
		HBUF hBuf;
		do {
			hBuf = HBUF(m_ADC_DTAcq32.GetQueue());
			if (hBuf != nullptr) m_ADC_DTAcq32.SetQueue(long(hBuf));
		} while (hBuf != nullptr);
		m_ChartDataWnd.ADdisplayStop();
		m_bchanged = TRUE;
	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError)); myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
	}
	m_ADC_inprogress = FALSE;
}

BOOL CViewADContinuous::DAC_OpenSubSystem(const CString& card_name)
{
	try
	{
		m_DAC_DTAcq32.SetBoard(card_name);
		if (m_DAC_DTAcq32.GetDevCaps(OLSS_DA) == 0)
			return FALSE;

		m_DAC_DTAcq32.SetSubSysType(OLSS_DA);
		const int n_da_elements = m_DAC_DTAcq32.GetDevCaps(OLDC_DAELEMENTS);
		if (n_da_elements < 1)
			return FALSE;
		m_DAC_DTAcq32.SetSubSysElement(0);
		ASSERT(m_DAC_DTAcq32.GetHDass() != NULL),
			m_bsimultaneousStartDA = m_DAC_DTAcq32.GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);
	}
	catch (COleDispatchException* e)
	{
		CString my_error;
		my_error.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError));
		my_error += e->m_strDescription;
		AfxMessageBox(my_error);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CViewADContinuous::DAC_ClearAllOutputs()
{
	try
	{
		if (m_DAC_DTAcq32.GetHDass() == NULL)
			return FALSE;

		if (m_DAC_DTAcq32.GetSSCaps(OLSSC_SUP_SINGLEVALUE) == FALSE)
		{
			AfxMessageBox(_T("D/A SubSystem cannot run in single value mode"));
			return FALSE;
		}
		m_DAC_DTAcq32.ClearError();
		m_DAC_DTAcq32.SetDataFlow(OLx_DF_SINGLEVALUE);
		long out_value = 0;
		if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
			out_value = WORD((out_value ^ m_DACmsbit) & m_DAClRes);

		m_DAC_DTAcq32.Config();
		//int nchansmax = m_DAC_DTAcq32.GetSSCaps(OLSSC_NUMCHANNELS) - 1;
		for (auto i = 0; i < 2; i++)
		{
			m_DAC_DTAcq32.PutSingleValue(i, 1.0, out_value);
		}
	}
	catch (COleDispatchException* e)
	{
		CString my_error;
		my_error.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError));
		my_error += e->m_strDescription;
		AfxMessageBox(my_error);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CViewADContinuous::DAC_InitSubSystem()
{
	try
	{
		if (m_DAC_DTAcq32.GetHDass() == NULL)
			return FALSE;

		// Set up the ADC - multiple wrap so we can get buffer reused
		m_DAC_DTAcq32.SetDataFlow(OLx_DF_CONTINUOUS);
		m_DAC_DTAcq32.SetWrapMode(OLx_WRP_NONE);
		m_DAC_DTAcq32.SetDmaUsage(short(m_DAC_DTAcq32.GetSSCaps(OLSSC_NUMDMACHANS)));

		// set clock the same as for A/D
		m_DAC_DTAcq32.SetClockSource(OLx_CLK_INTERNAL);
		const double clockrate = m_pADC_options->waveFormat.chrate;
		m_DAC_DTAcq32.SetFrequency(clockrate);		// set sampling frequency (total throughput)

		// set trigger mode
		int trig = m_pADC_options->waveFormat.trig_mode;
		if (trig > OLx_TRG_EXTRA)
			trig = 0;
		m_DAC_DTAcq32.SetTrigger(trig);

		DAC_SetChannelList();
		const auto resolutionfactor = pow(2.0, m_DAC_DTAcq32.GetResolution());
		m_DACmsbit = long(pow(2.0, ((double)m_DAC_DTAcq32.GetResolution() - 1)));
		m_DAClRes = long(resolutionfactor) - 1;

		for (auto i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
		{
			const auto p_parms = &m_pDAC_options->outputparms_array.GetAt(i);
			if (p_parms->bDigital)
				continue;
			p_parms->ampUp = (double(p_parms->dAmplitudeMaxV) * resolutionfactor) / (double(m_DAC_DTAcq32.GetMaxRange()) - m_DAC_DTAcq32.GetMinRange());
			p_parms->ampLow = (double(p_parms->dAmplitudeMinV) * resolutionfactor) / (double(m_DAC_DTAcq32.GetMaxRange()) - m_DAC_DTAcq32.GetMinRange());
		}

		// pass parameters to the board and check if errors
		m_DAC_DTAcq32.ClearError();
		m_DAC_DTAcq32.Config();
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

int CViewADContinuous::DAC_GetNumberOfChans()
{
	auto nanalog_outputs = 0;
	auto ndigital_outputs = 0;

	// first pass: count number of channels requested
	for (auto i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
	{
		const auto p_parms = &m_pDAC_options->outputparms_array.GetAt(i);
		if (!p_parms->bON)
			continue;
		if (p_parms->bDigital)
			ndigital_outputs++;
		else
			nanalog_outputs++;
	}
	m_DACdigitalchannel = 0;
	const auto nchans = nanalog_outputs + (ndigital_outputs > 0);
	return nchans;
}

int CViewADContinuous::DAC_SetChannelList()
{
	m_DAClistsize = DAC_GetNumberOfChans();
	try
	{
		// number of D/A channels
		const int num_channels = m_DAC_DTAcq32.GetSSCaps(OLSSC_NUMCHANNELS);
		ASSERT(m_DAClistsize <= num_channels);
		m_DAC_DTAcq32.SetListSize(m_DAClistsize);
		ASSERT(m_DAClistsize == m_DAC_DTAcq32.GetListSize());

		auto digital_output = 0;
		auto analog_output = 0;
		for (auto i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
		{
			const auto p_parms = &m_pDAC_options->outputparms_array.GetAt(i);
			if (!p_parms->bON)
				continue;

			if (p_parms->bDigital)
				digital_output++;
			else
			{
				m_DAC_DTAcq32.SetChannelList(analog_output, p_parms->iChan);
				analog_output++;
			}
		}

		if (digital_output > 0)
		{
			m_DACdigitalchannel = analog_output;
			m_DAC_DTAcq32.SetChannelList(m_DACdigitalchannel, num_channels - 1);
		}
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
	}
	return m_DAClistsize;
}

void CViewADContinuous::DAC_DeleteBuffers()
{
	try {
		if (m_DAC_DTAcq32.GetHDass() == NULL)
			return;

		m_DAC_DTAcq32.Flush();	// clean
		HBUF h_buf;			// handle to buffer
		do {				// loop until all buffers are removed
			h_buf = HBUF(m_DAC_DTAcq32.GetQueue());
			if (h_buf != nullptr)
				if (olDmFreeBuffer(h_buf) != OLNOERROR)
					AfxMessageBox(_T("Error Freeing Buffer"));
		} while (h_buf != nullptr);
		m_DAC_bufhandle = h_buf;
	}
	catch (COleDispatchException* e)
	{
		CString myError;
		myError.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError));
		myError += e->m_strDescription;
		AfxMessageBox(myError);
		e->Delete();
	}
}

void CViewADContinuous::DAC_DeclareAndFillBuffers()
{
	// close data buffers
	DAC_DeleteBuffers();
	if (m_DAClistsize == 0)
		return;

	// get current parms from A/D conversion
	const auto p_w_format = &(m_pADC_options->waveFormat);
	m_DAC_frequency = p_w_format->chrate;

	// define buffer length
	const auto sweepduration = m_pADC_options->sweepduration;
	const auto chsweeplength = long(sweepduration * p_w_format->chrate);
	const int nbuffers = p_w_format->bufferNitems;
	m_DAC_chbuflen = chsweeplength / nbuffers;
	m_DAC_buflen = m_DAC_chbuflen * m_DAClistsize;

	for (auto i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
	{
		const auto parms_chan = &(m_pDAC_options->outputparms_array.GetAt(i));
		parms_chan->lastphase = 0;
		parms_chan->lastamp = 0;
		parms_chan->bStart = TRUE;
	}

	// declare buffers to DT
	m_DAC_nBuffersFilledSinceStart = 0;
	for (auto i = 0; i <= nbuffers; i++)
	{
		/*auto ecode =*/ olDmAllocBuffer(0, m_DAC_buflen, &m_DAC_bufhandle);
		short* p_dt_buffer;
		const auto ecode = olDmGetBufferPtr(m_DAC_bufhandle, (void**)&p_dt_buffer);
		DAC_FillBuffer(p_dt_buffer);
		if ((ecode == OLNOERROR) && (m_DAC_bufhandle != nullptr))
		{
			m_DAC_DTAcq32.SetQueue(long(m_DAC_bufhandle));
		}
	}
}

void CViewADContinuous::DAC_ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, const int chan) const
{
	for (auto i = chan; i < m_DAC_buflen; i += m_DAClistsize)
		*(pDTbuf + i) = WORD((*(pDTbuf + i) ^ m_DACmsbit) & m_DAClRes);
}

void CViewADContinuous::DAC_FillBufferWith_SINUSOID(short* pDTbuf, int chan, OUTPUTPARMS* parmsChan)
{
	auto phase = parmsChan->lastphase;
	auto freq = parmsChan->dFrequency / m_DAC_frequency;
	const auto amp = (parmsChan->ampUp - parmsChan->ampLow) / 2;
	const auto offset = (parmsChan->ampUp + parmsChan->ampLow) / 2;
	const auto nchans = m_DAClistsize;

	const auto pi2 = 3.1415927 * 2;
	freq = freq * pi2;
	for (auto i = chan; i < m_DAC_buflen; i += nchans)
	{
		*(pDTbuf + i) = short(cos(phase) * amp + offset);
		// clip value
		if (*(pDTbuf + i) > m_DACmsbit)
			*(pDTbuf + i) = short(m_DACmsbit - 1);
		phase += freq;
		if (phase > pi2)
			phase -= pi2;
	}

	if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	parmsChan->lastphase = phase;
	parmsChan->lastamp = amp;
}

void CViewADContinuous::DAC_FillBufferWith_SQUARE(short* pDTbuf, const int chan, OUTPUTPARMS* parms_chan)
{
	auto phase = parms_chan->lastphase;
	const auto freq = parms_chan->dFrequency / m_DAC_frequency;
	const auto amp_up = parms_chan->ampUp;
	const auto amp_low = parms_chan->ampLow;
	const auto nchans = m_DAClistsize;

	for (auto i = chan; i < m_DAC_buflen; i += nchans)
	{
		double amp;
		if (phase < 0)
			amp = amp_up;
		else
			amp = amp_low;
		*(pDTbuf + i) = WORD(amp);
		phase += freq;
		if (phase > 0.5)
			phase -= 1;
	}

	if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	parms_chan->lastphase = phase;
}

void CViewADContinuous::DAC_FillBufferWith_TRIANGLE(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan)
{
	auto phase = parms_chan->lastphase;
	const auto freq = parms_chan->dFrequency / m_DAC_frequency;
	auto amp = parms_chan->ampUp;
	const auto nchans = m_DAClistsize;

	for (auto i = chan; i < m_DAC_buflen; i += nchans)
	{
		*(p_dt_buffer + i) = WORD(2 * phase * amp);
		// clip value
		if (*(p_dt_buffer + i) >= m_DACmsbit)
			*(p_dt_buffer + i) = short(m_DACmsbit - 1);
		phase = phase + 2 * freq;
		if (phase > 0.5)
		{
			phase -= 1;
			amp--;
		}
	}

	if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(p_dt_buffer, chan);

	parms_chan->lastphase = phase;
	parms_chan->lastamp = amp;
}

void CViewADContinuous::DAC_FillBufferWith_RAMP(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan)
{
	const auto amp = parms_chan->ampUp;
	const auto nchans = m_DAClistsize;

	for (auto i = chan; i < m_DAC_buflen; i += nchans)
		*(p_dt_buffer + i) = WORD(amp);

	if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(p_dt_buffer, chan);

	parms_chan->lastamp = amp;
}

void CViewADContinuous::DAC_FillBufferWith_CONSTANT(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan)
{
	const auto amp = (double(parms_chan->value) * pow(2.0, m_DAC_DTAcq32.GetResolution())) / (double(m_DAC_DTAcq32.GetMaxRange()) - m_DAC_DTAcq32.GetMinRange());
	const auto nchans = m_DAClistsize;

	for (auto i = chan; i < m_DAC_buflen; i += nchans)
		*(p_dt_buffer + i) = WORD(amp);

	if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(p_dt_buffer, chan);

	parms_chan->lastamp = amp;
}

void CViewADContinuous::DAC_FillBufferWith_ONOFFSeq(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan)
{
	const auto amp_up = parms_chan->ampUp;
	const auto amp_low = parms_chan->ampLow;
	const auto nchans = m_DAClistsize;

	auto pstim = &parms_chan->sti;
	const auto ch_freq_ratio = m_DAC_frequency / pstim->chrate;
	const auto buffer_start = m_DAC_nBuffersFilledSinceStart * m_DAC_chbuflen;
	//auto buffer_end = (m_DAC_nBuffersFilledSinceStart + 1)*m_DAC_chbuflen;
	auto buffer_ii = buffer_start;
	int interval;
	auto wamp = FALSE;
	long stim_end = 0;

	// find end = first interval after buffer_end; find start
	for (interval = 0; interval < pstim->GetSize(); interval++)
	{
		stim_end = long(pstim->GetIntervalPointAt(interval).ii * ch_freq_ratio);
		if (stim_end > buffer_start)
			break;
		wamp = pstim->GetIntervalPointAt(interval).w;
	}
	auto amp = amp_up * wamp + amp_low * !wamp;
	auto wout = WORD(amp);
	if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
		wout = WORD(wout ^ m_DACmsbit) & m_DAClRes;

	// fill buffer
	for (auto i = chan; i < m_DAC_buflen; i += nchans, buffer_ii++)
	{
		*(p_dt_buffer + i) = wout;

		if ((interval < pstim->GetSize()) && buffer_ii >= stim_end)
		{
			interval++;
			//wamp = FALSE;
			if (interval < pstim->GetSize())
				stim_end = long(pstim->GetIntervalPointAt(interval).ii * ch_freq_ratio);
			wamp = pstim->GetIntervalPointAt(interval - 1).w;
			amp = amp_up * wamp + amp_low * !wamp;
			wout = WORD(amp);
			if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
				wout = WORD(wout ^ m_DACmsbit) & m_DAClRes;
		}
	}
	parms_chan->lastamp = amp;
}

void CViewADContinuous::DAC_MSequence(OUTPUTPARMS* parms_chan)
{
	parms_chan->count--;
	if (parms_chan->count == 0) {
		parms_chan->count = parms_chan->mseq_iRatio + 1;
		if (parms_chan->bStart) {
			parms_chan->num = parms_chan->mseq_iSeed;
			parms_chan->bit1 = 1;
			parms_chan->bit33 = 0;
			parms_chan->count = 1;
			parms_chan->bStart = FALSE;
		}
		const UINT bit13 = ((parms_chan->num & 0x1000) != 0);
		parms_chan->bit1 = (bit13 == parms_chan->bit33) ? 0 : 1;
		parms_chan->bit33 = ((parms_chan->num & 0x80000000) != 0);
		parms_chan->num = (parms_chan->num << 1) + parms_chan->bit1;
	}
}

void CViewADContinuous::DAC_FillBufferWith_MSEQ(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan)
{
	const auto DAClistsize = m_DAClistsize;
	double x = 0;
	//auto mseq_offset_delay = parms_chan->mseq_iDelay;

	for (auto i = chan; i < m_DAC_buflen; i += DAClistsize)
	{
		x = 0;
		if (parms_chan->mseq_iDelay > 0)
			parms_chan->mseq_iDelay--;
		else
		{
			x = parms_chan->ampLow;
			if (parms_chan->mseq_iDelay == 0) {
				DAC_MSequence(parms_chan);
				x = (parms_chan->bit1 * parms_chan->ampUp) + (!parms_chan->bit1 * parms_chan->ampLow);
			}
		}
		*(p_dt_buffer + i) = WORD(x);
	}

	if (m_DAC_DTAcq32.GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(p_dt_buffer, chan);

	parms_chan->lastamp = x;
}

void CViewADContinuous::DAC_Dig_FillBufferWith_ONOFFSeq(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan) const
{
	const WORD amp_low = 0;
	WORD amp_up = 1;
	amp_up = amp_up << parms_chan->iChan;
	const auto nchans = m_DAClistsize;

	auto pstim = &parms_chan->sti;
	const auto ch_freq_ratio = m_DAC_frequency / pstim->chrate;
	const auto buffer_start = m_DAC_nBuffersFilledSinceStart * m_DAC_chbuflen;
	//auto buffer_end = (m_DAC_nBuffersFilledSinceStart + 1)*m_DAC_chbuflen;
	auto buffer_ii = buffer_start;
	int interval;
	auto wamp = 0;
	long	stim_end = 0;

	// find end = first interval after buffer_end; find start
	for (interval = 0; interval < pstim->GetSize(); interval++)
	{
		stim_end = long(pstim->GetIntervalPointAt(interval).ii * ch_freq_ratio);
		if (stim_end > buffer_start)
			break;
		wamp = pstim->GetIntervalPointAt(interval).w;
	}
	auto wout = amp_low;
	if (wamp > 0)
		wout = amp_up;

	// fill buffer
	for (auto i = chan; i < m_DAC_buflen; i += nchans, buffer_ii++)
	{
		if (m_DACdigitalfirst == 0)
			*(p_dt_buffer + i) = wout;
		else
			*(p_dt_buffer + i) |= wout;

		if ((interval < pstim->GetSize()) && buffer_ii >= stim_end)
		{
			interval++;
			if (interval < pstim->GetSize())
				stim_end = long(pstim->GetIntervalPointAt(interval).ii * ch_freq_ratio);
			wamp = pstim->GetIntervalPointAt(interval - 1).w;
			if (wamp > 0)
				wout = amp_up;
			else
				wout = amp_low;
		}
	}
}

void CViewADContinuous::DAC_Dig_FillBufferWith_VAL(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan, const BOOL val) const
{
	const auto nchans = m_DAClistsize;
	const auto w1 = 1 << parms_chan->iChan;
	const auto w0 = 0;

	// fill buffer
	if (m_DACdigitalfirst == 0) {
		auto wout = w0;
		if (val > 0)
			wout = w1;
		for (auto i = chan; i < m_DAC_buflen; i += nchans)
			*(p_dt_buffer + i) = wout;
	}
	else {
		if (val > 0) {
			const auto wout = w1;
			for (auto i = chan; i < m_DAC_buflen; i += nchans)
				*(p_dt_buffer + i) |= wout;
		}
		else
		{
			const auto wout = ~w1;
			for (auto i = chan; i < m_DAC_buflen; i += nchans)
				*(p_dt_buffer + i) &= wout;
		}
	}
}

void CViewADContinuous::DAC_Dig_FillBufferWith_SQUARE(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan) const
{
	auto phase = parms_chan->lastphase;
	WORD amp_up = 1;
	amp_up = amp_up << parms_chan->iChan;
	const WORD	amp_low = 0;
	const auto freq = parms_chan->dFrequency / m_DAC_frequency;
	const auto nchans = m_DAClistsize;

	for (auto i = chan; i < m_DAC_buflen; i += nchans)
	{
		WORD	amp;
		if (phase < 0)
			amp = amp_up;
		else
			amp = amp_low;

		if (m_DACdigitalfirst == 0)
			*(p_dt_buffer + i) = amp;
		else
			*(p_dt_buffer + i) |= amp;

		phase += freq;
		if (phase > 0.5)
			phase -= 1;
	}
	parms_chan->lastphase = phase;
}

void CViewADContinuous::DAC_Dig_FillBufferWith_MSEQ(short* p_dt_buffer, const int chan, OUTPUTPARMS* parms_chan) const
{
	const WORD	amp_low = 0;
	WORD	amp_up = 1;
	amp_up = amp_up << parms_chan->iChan;
	const auto dac_list_size = m_DAClistsize;
	double x = 0;
	//auto mseq_offset_delay = parms_chan->mseq_iDelay;

	for (auto i = chan; i < m_DAC_buflen; i += dac_list_size)
	{
		x = 0;
		if (parms_chan->mseq_iDelay > 0)
			parms_chan->mseq_iDelay--;
		else
		{
			x = parms_chan->ampLow;
			if (parms_chan->mseq_iDelay == 0) {
				DAC_MSequence(parms_chan);
				x = double(parms_chan->bit1) * amp_up + double(!parms_chan->bit1) * amp_low;
			}
		}
		if (m_DACdigitalfirst == 0)
			*(p_dt_buffer + i) = WORD(x);
		else
			*(p_dt_buffer + i) |= WORD(x);
	}
	parms_chan->lastamp = x;
}

void CViewADContinuous::DAC_FillBuffer(short* p_dt_buffer)
{
	auto janalog = 0;
	m_DACdigitalfirst = 0;
	const auto nchans = m_pDAC_options->outputparms_array.GetSize();
	ASSERT(nchans == 10);
	for (auto i = 0; i < nchans; i++)
	{
		const auto p_parms = &m_pDAC_options->outputparms_array.GetAt(i);
		if (!p_parms->bON)
			continue;

		if (!p_parms->bDigital)
		{
			switch (p_parms->iWaveform)
			{
			case DA_SINEWAVE:
				DAC_FillBufferWith_SINUSOID(p_dt_buffer, janalog, p_parms);
				break;
			case DA_SQUAREWAVE:
				DAC_FillBufferWith_SQUARE(p_dt_buffer, janalog, p_parms);
				break;
			case DA_TRIANGLEWAVE:
				DAC_FillBufferWith_TRIANGLE(p_dt_buffer, janalog, p_parms);
				break;
			case DA_LINEWAVE:
				DAC_FillBufferWith_RAMP(p_dt_buffer, janalog, p_parms);
				break;
			case DA_SEQUENCEWAVE:
				DAC_FillBufferWith_ONOFFSeq(p_dt_buffer, janalog, p_parms);
				break;
			case DA_MSEQWAVE:
				DAC_FillBufferWith_MSEQ(p_dt_buffer, janalog, p_parms);
				break;
			case DA_CONSTANT:
			default:
				DAC_FillBufferWith_CONSTANT(p_dt_buffer, janalog, p_parms);
				break;
			}
			janalog++;
		}
		else
		{
			switch (p_parms->iWaveform)
			{
			case DA_SQUAREWAVE:
				DAC_Dig_FillBufferWith_SQUARE(p_dt_buffer, m_DACdigitalchannel, p_parms);
				break;
			case DA_SEQUENCEWAVE:
				DAC_Dig_FillBufferWith_ONOFFSeq(p_dt_buffer, m_DACdigitalchannel, p_parms);
				break;
			case DA_MSEQWAVE:
				DAC_Dig_FillBufferWith_MSEQ(p_dt_buffer, m_DACdigitalchannel, p_parms);
				break;
			case DA_ONE:
				DAC_Dig_FillBufferWith_VAL(p_dt_buffer, m_DACdigitalchannel, p_parms, 1);
				break;
			case DA_ZERO:
			default:
				DAC_Dig_FillBufferWith_VAL(p_dt_buffer, m_DACdigitalchannel, p_parms, 0);
				break;
			}
			m_DACdigitalfirst++;
		}
	}
	m_DAC_nBuffersFilledSinceStart++;
}

void CViewADContinuous::DAC_StopAndLiberateBuffers()
{
	try {
		m_DAC_DTAcq32.Stop();
		m_DAC_DTAcq32.Flush();	// flush all buffers to Done Queue
		HBUF h_buf;
		do {
			h_buf = HBUF(m_DAC_DTAcq32.GetQueue());
			if (h_buf != nullptr) {
				const auto ecode = olDmFreeBuffer(h_buf);
				if (ecode != OLNOERROR)
					AfxMessageBox(_T("Could not free Buffer"));
			}
		} while (h_buf != nullptr);

		DAC_ClearAllOutputs();
	}
	catch (COleDispatchException* e)
	{
		AfxMessageBox(e->m_strDescription);
		e->Delete();
	}
	m_DAC_inprogress = FALSE;
}

void CViewADContinuous::get_dt_error(const ECODE ecode, const BOOL b_display_error) const
{
	if (ecode != OLNOERROR) {
		CHAR errstr[255];
		olDaGetErrorString(ecode, errstr, 255);
		if (b_display_error) {
			CString cs = _T("error ");
			cs += errstr;
			AfxMessageBox(cs);
		}
	}
}

void CViewADContinuous::OnStop(const BOOL b_display_error_msg)
{
	// special treatment if simultaneous list
	if (m_bSimultaneousStart && m_bStartOutPutMode == 0)
	{
		HSSLIST h_ss_list;

		auto ecode = olDaGetSSList(&h_ss_list);
		get_dt_error(ecode, b_display_error_msg);
		ecode = olDaReleaseSSList(h_ss_list);
		get_dt_error(ecode, b_display_error_msg);
	}

	// stop AD, liberate DTbuffers
	if (m_ADC_inprogress)
		ADC_StopAndLiberateBuffers();
	ADC_UpdateStartStop(m_ADC_inprogress);

	// stop DA, liberate buffers
	if (m_DAC_inprogress && m_bStartOutPutMode == 0) {
		DAC_StopAndLiberateBuffers();
	}
	DAC_UpdateStartStop(m_DAC_inprogress);

	// close file and update display
	if (m_bFileOpen)
	{
		SaveAndCloseFile();
		// update view data
		const auto lsizeDOCchan = m_inputDataFile.GetDOCchanLength();
		m_ChartDataWnd.AttachDataFile(&m_inputDataFile);
		m_ChartDataWnd.ResizeChannels(m_ChartDataWnd.GetRectWidth(), lsizeDOCchan);
		m_ChartDataWnd.GetDataFromDoc(0, lsizeDOCchan);
	}
}

void CViewADContinuous::SaveAndCloseFile()
{
	m_inputDataFile.AcqDoc_DataAppendStop();
	const auto wave_format = m_inputDataFile.GetpWaveFormat();

	// if burst data acquisition mode ------------------------------------
	if (m_bhidesubsequent)
	{
		if (wave_format->sample_count > 1) // make sure real data have been acquired
			m_csNameArray.Add(m_szFileName);
		else
			m_inputDataFile.AcqDeleteFile();
	}

	// normal data acquisition mode --------------------------------------
	else
	{
		auto ires = IDCANCEL;
		if (wave_format->sample_count > 1) // make sure real data have been acquired
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

		// -----------------------------------------------------
		// if current document name is the same, it means something happened and we have erased a previously existing file
		// if so, skip
		// otherwise add data file name to the database
		auto pdb_doc = GetDocument();
		if (m_szFileName.CompareNoCase(pdb_doc->GetDB_CurrentDatFileName(FALSE)) != 0)
		{
			// add document to database
			m_csNameArray.Add(m_szFileName);
			TransferFilesToDatabase();
			UpdateViewDataFinal();
		}
	}
}

void CViewADContinuous::UpdateViewDataFinal()
{
	// update view data
	auto pdb_doc = GetDocument();
	auto p_doc_dat = pdb_doc->OpenCurrentDataFile();
	if (p_doc_dat == nullptr)
	{
		AfxMessageBox(_T("error reading current document"), MB_OK);
		return;
	}

	p_doc_dat->ReadDataInfos();
	const auto size_doc_channel = p_doc_dat->GetDOCchanLength();
	m_ChartDataWnd.AttachDataFile(p_doc_dat);
	m_ChartDataWnd.ResizeChannels(m_ChartDataWnd.GetRectWidth(), size_doc_channel);
	m_ChartDataWnd.GetDataFromDoc(0, size_doc_channel);
}

void CViewADContinuous::TransferFilesToDatabase()
{
	auto pdb_doc = GetDocument();
	pdb_doc->ImportFileList(m_csNameArray);	
	m_csNameArray.RemoveAll();	

	auto p_set = &(GetDocument()->m_pDB->m_mainTableSet);
	p_set->BuildAndSortIDArrays();
	p_set->RefreshQuery();
	const int nfiles = pdb_doc->m_pDB->GetNRecords();
	pdb_doc->SetDB_CurrentRecordPosition(nfiles - 1);
	pdb_doc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
}

BOOL CViewADContinuous::OnStart()
{
	// set display
	if (m_bADwritetofile && !ADC_DefineExperimentDlg())
	{
		OnStop(FALSE);
		return FALSE;
	}

	if (!ADC_InitSubSystem())
		return FALSE;

	if (m_bDAC_IsPresent && (m_bStartOutPutMode == 0) && DAC_InitSubSystem())
		DAC_DeclareAndFillBuffers();

	// start AD display
	m_chsweep1 = 0;
	m_chsweep2 = -1;
	m_ChartDataWnd.ADdisplayStart(m_chsweeplength);
	auto wave_format = m_inputDataFile.GetpWaveFormat();
	wave_format->sample_count = 0;							// no samples yet
	wave_format->chrate = wave_format->chrate / m_pADC_options->iundersample;
	m_fclockrate = wave_format->chrate * wave_format->scan_count;
	wave_format->acqtime = CTime::GetCurrentTime();

	// data format
	wave_format->binspan = (m_pADC_options->waveFormat).binspan;
	wave_format->fullscale_Volts = (m_pADC_options->waveFormat).fullscale_Volts;
	// trick: if OLx_ENC_BINARY, it is changed on the fly within AD_Transfer function
	// when a DT buffer into a CAcqDataDoc buffer
	wave_format->mode_encoding = OLx_ENC_2SCOMP;
	wave_format->binzero = 0;

	// start acquisition and save data to file?
	if (m_bADwritetofile && (wave_format->trig_mode == OLx_TRG_EXTRA + 1))
	{
		if (AfxMessageBox(_T("Start data acquisition"), MB_OKCANCEL) != IDOK)
		{
			OnStop(FALSE);
			return FALSE;
		}
	}

	// starting mode of A/D if no simultaneous list
	if (!m_bSimultaneousStart || m_bStartOutPutMode != 0 || m_DAClistsize == 0)
	{
		try
		{
			m_ADC_DTAcq32.Config();
			m_ADC_DTAcq32.Start();
			m_ADC_inprogress = TRUE;
			m_DAC_inprogress = FALSE;

			if (m_bDAC_IsPresent && m_bStartOutPutMode == 0 && m_DAClistsize > 0)
			{
				m_DAC_DTAcq32.Config();
				m_DAC_DTAcq32.Start();
				m_DAC_inprogress = TRUE;
			}
		}
		catch (COleDispatchException* e)
		{
			CString myError;
			myError.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError)); myError += e->m_strDescription;
			AfxMessageBox(myError);
			e->Delete();
		}
	}

	// starting A/D when simultaneous list
	else
	{
		BOOL retval;
		HSSLIST h_ss_list;
		ASSERT(m_bDAC_IsPresent);

		// create simultaneous starting list
		auto ecode = olDaGetSSList(&h_ss_list);
		get_dt_error(ecode, true);

		m_ADC_DTAcq32.Config();
		m_DAC_DTAcq32.Config();

		// DA system
		ecode = olDaPutDassToSSList(h_ss_list, HDASS(m_DAC_DTAcq32.GetHDass()));
		if (ecode != OLNOERROR)
		{
			retval = ecode;
			ecode = olDaReleaseSSList(h_ss_list);
			get_dt_error(ecode, true);
			return retval;
		}

		// AD system
		ecode = olDaPutDassToSSList(h_ss_list, HDASS(m_ADC_DTAcq32.GetHDass()));
		if (ecode != OLNOERROR)
		{
			retval = ecode;
			ecode = olDaReleaseSSList(h_ss_list);
			get_dt_error(ecode, true);
			return retval;
		}

		// prestart
		ecode = olDaSimultaneousPrestart(h_ss_list);
		get_dt_error(ecode, true);

		// start simultaneously
		ecode = olDaSimultaneousStart(h_ss_list);
		get_dt_error(ecode, true);

		m_ADC_inprogress = TRUE;
		m_DAC_inprogress = TRUE;
	}
	ADC_UpdateStartStop(m_ADC_inprogress);
	DAC_UpdateStartStop(m_DAC_inprogress);
	return TRUE;
}

#ifdef _DEBUG
void CViewADContinuous::AssertValid() const
{
	CFormView::AssertValid();
}

void CViewADContinuous::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CdbWaveDoc* CViewADContinuous::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return static_cast<CdbWaveDoc*>(m_pDocument);
}

#endif //_DEBUG

CDaoRecordset* CViewADContinuous::OnGetRecordset()
{
	return m_ptableSet;
}

void CViewADContinuous::OnInitialUpdate()
{
	// attach controls
	VERIFY(m_ChartDataWnd.SubclassDlgItem(IDC_DISPLAYDATA, this));
	VERIFY(m_ADC_yRulerBar.SubclassDlgItem(IDC_YSCALE, this));
	VERIFY(m_ADC_xRulerBar.SubclassDlgItem(IDC_XSCALE, this));
	m_ADC_yRulerBar.AttachScopeWnd(&m_ChartDataWnd, FALSE);
	m_ADC_xRulerBar.AttachScopeWnd(&m_ChartDataWnd, TRUE);
	m_ChartDataWnd.AttachExternalXRuler(&m_ADC_xRulerBar);
	m_ChartDataWnd.AttachExternalYRuler(&m_ADC_yRulerBar);
	m_ChartDataWnd.m_bNiceGrid = TRUE;

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

	const BOOL b32_bit_icons = afxGlobalData.m_nBitsPerPixel >= 16;
	m_btnStartStop.SetImage(b32_bit_icons ? IDB_CHECK32 : IDB_CHECK);
	m_btnStartStop.SetCheckedImage(b32_bit_icons ? IDB_CHECKNO32 : IDB_CHECKNO);
	CMFCButton::EnableWindowsTheming(false);
	m_btnStartStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;

	// scrollbar
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);

	// CFormView init CFile
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	m_pADC_options = &(p_app->options_acqdata);							// address of data acquisition parameters
	m_pDAC_options = &(p_app->options_outputdata);							// address of data output parameters
	m_bFoundDTOPenLayerDLL = FALSE;								// assume there is no card
	m_bADwritetofile = m_pADC_options->waveFormat.bADwritetofile;
	m_bStartOutPutMode = m_pDAC_options->bAllowDA;
	((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->SetCurSel(m_bStartOutPutMode);

	// open document and remove database filters
	auto pdb_doc = GetDocument();								// data document with database
	m_ptableSet = &pdb_doc->m_pDB->m_mainTableSet;				// database itself
	m_ptableSet->m_strFilter.Empty();
	m_ptableSet->ClearFilters();
	m_ptableSet->RefreshQuery();
	CFormView::OnInitialUpdate();

	// if current document, load parameters from current document into the local set of parameters
	// if current document does not exist, do nothing
	if (pdb_doc->m_pDB->GetNRecords() > 0) {
		const auto p_dat = pdb_doc->OpenCurrentDataFile();				// read data descriptors from current data file
		if (p_dat != nullptr)
		{
			m_pADC_options->waveFormat = *(p_dat->GetpWaveFormat());	// read data header
			m_pADC_options->chanArray.ChanArray_setSize(m_pADC_options->waveFormat.scan_count);
			m_pADC_options->chanArray = *p_dat->GetpWavechanArray();	// get channel descriptors
			// restore state of "write-to-file" parameter that was just erased
			m_pADC_options->waveFormat.bADwritetofile = m_bADwritetofile;
		}
	}

	// create data file and copy data acquisition parameters into it
	m_inputDataFile.OnNewDocument();							// create a file to receive incoming data (A/D)
	*(m_inputDataFile.GetpWaveFormat()) = m_pADC_options->waveFormat;	// copy data formats into this file
	m_pADC_options->chanArray.ChanArray_setSize(m_pADC_options->waveFormat.scan_count);
	*(m_inputDataFile.GetpWavechanArray()) = m_pADC_options->chanArray;
	m_ChartDataWnd.AttachDataFile(&m_inputDataFile);			// prepare display area

	// init communication with Alligator
	m_Alligator.USBPxxS1Command(0, ID_INITIALIZE, nullptr, nullptr);

	p_app->m_bADcardFound = FindDTOpenLayersBoards();			// open DT Open Layers board
	if (p_app->m_bADcardFound)
	{
		ADC_InitSubSystem();									// connect A/D DT OpenLayer subsystem
		InitConnectionWithAmplifiers();							// control cyberamplifier
		if (m_bDAC_IsPresent)
		{
			DAC_InitSubSystem();								// connect D/A DT OpenLayers subsystem
			DAC_ClearAllOutputs();
		}
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

void CViewADContinuous::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// update sent from within this class
	if (pSender == this)
	{
		ASSERT(GetDocument() != NULL);
		m_ChartDataWnd.Invalidate();		// display data
	}
}

void CViewADContinuous::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// activate view
	if (bActivate)
	{
		auto pmf = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
		pmf->ActivatePropertyPane(FALSE);
		((CChildFrame*)pmf->MDIGetActive())->m_cursorstate = 0;
	}
	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CViewADContinuous::OnSize(UINT nType, int cx, int cy)
{
	switch (nType)
	{
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
		if (m_ADC_inprogress)
		{
			OnStop(TRUE);
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

LRESULT CViewADContinuous::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	// message emitted by IDC_DISPLAREA_button
	//if (j == IDC_DISPLAREA_button)		// always true here...
	//int j = wParam;				// control ID of sender

	// parameters
	const int code = HIWORD(lParam);	// code parameter
	// code = 0: chan hit 			lowp = channel
	// code = 1: cursor change		lowp = new cursor value
	// code = 2: horiz cursor hit	lowp = cursor index
	int lowp = LOWORD(lParam);	// value associated with code

	switch (code)
	{
	case HINT_SETMOUSECURSOR:
		if (lowp > CURSOR_ZOOM)
			lowp = 0;
		m_cursorstate = m_ChartDataWnd.SetMouseCursorType(lowp);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, 0));
		break;

	default:
		if (lowp == 0) MessageBeep(MB_ICONEXCLAMATION);
		break;
	}
	return 0L;
}

void CViewADContinuous::ADC_OnBnClickedStartstop()
{
	// Start
	if (m_btnStartStop.IsChecked())
	{
		if (OnStart())
		{
			if ((m_inputDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
				ADC_OnBufferDone();
		}
		else
		{
			OnStop(FALSE);
		}
	}
	// Stop
	else
	{
		OnStop(TRUE);
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
}

void CViewADContinuous::ADC_UpdateStartStop(const BOOL b_start)
{
	if (b_start)
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
	m_ChartDataWnd.Invalidate();
}

void CViewADContinuous::ADC_OnHardwareDefineexperiment()
{
	ADC_DefineExperimentDlg();
}

BOOL CViewADContinuous::ADC_DefineExperimentDlg()
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
		if (IDOK == dlg.DoModal()) {
			m_bhidesubsequent = dlg.m_bhidesubsequent;
			csfilename = dlg.m_szFileName;
		}
		else
			return FALSE;
	}

	// hide define experiment dialog
	else
	{
		// build file name
		CString cs_buf_temp;
		m_pADC_options->exptnumber++;
		cs_buf_temp.Format(_T("%06.6lu"), m_pADC_options->exptnumber);
		csfilename = m_pADC_options->csPathname + m_pADC_options->csBasename + cs_buf_temp + _T(".dat");

		// check if this file is already present, exit if not...
		CFileStatus status;
		auto id_response = IDYES;	// default: go on if file not found
		if (CFile::GetStatus(csfilename, status))
			id_response = AfxMessageBox(IDS_FILEOVERWRITE, MB_YESNO | MB_ICONWARNING);
		// no .. find first available number
		if (IDNO == id_response)
		{
			auto flag = TRUE;
			while (flag)
			{
				m_pADC_options->exptnumber++;
				cs_buf_temp.Format(_T("%06.6lu"), m_pADC_options->exptnumber);
				csfilename = m_pADC_options->csPathname + m_pADC_options->csBasename + cs_buf_temp + _T(".dat");
				flag = CFile::GetStatus(csfilename, status);
			}
			const auto cs = _T("The Next available file name is: ") + csfilename;
			AfxMessageBox(cs, MB_YESNO | MB_ICONWARNING);
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

void CViewADContinuous::ADC_OnHardwareChannelsDlg()
{
	if (m_ADC_inprogress)
		OnStop(TRUE);

	CDlgADInputParms dlg;
	dlg.m_pwFormat = &(m_pADC_options->waveFormat);
	dlg.m_pchArray = &(m_pADC_options->chanArray);
	dlg.m_numchansMAXDI = m_ADC_DTAcq32.GetSSCaps(OLSSC_MAXDICHANS);
	dlg.m_numchansMAXSE = m_ADC_DTAcq32.GetSSCaps(OLSSC_MAXSECHANS);
	dlg.m_bchantype = m_pADC_options->bChannelType;
	dlg.m_bchainDialog = TRUE;
	dlg.m_bcommandAmplifier = TRUE;
	dlg.m_pAlligatorAmplifier = &m_Alligator;
	dlg.p_alligatordevice_ptr_array = &alligatorparameters_ptr_array;

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

void CViewADContinuous::ADC_OnHardwareIntervalsDlg()
{
	if (m_ADC_inprogress)
		OnStop(TRUE);

	ADIntervalsDlg dlg;
	const auto wave_format = &(m_pADC_options->waveFormat);
	dlg.m_pwaveFormat = wave_format;
	dlg.m_ratemin = 1.0f;
	dlg.m_ratemax = float(m_freqmax / wave_format->scan_count);
	dlg.m_bufferWsizemax = UINT(65536) * 4;
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

void CViewADContinuous::ChainDialog(WORD iID)
{
	WORD menu_id;
	switch (iID)
	{
	case IDC_ADINTERVALS:
		menu_id = ID_HARDWARE_ADINTERVALS;
		break;
	case IDC_ADCHANNELS:
		menu_id = ID_HARDWARE_ADCHANNELS;
		break;
	default:
		return;
	}
	PostMessage(WM_COMMAND, menu_id, NULL);
}

void CViewADContinuous::ADC_OnTriggerError()
{
	OnStop(TRUE);
	AfxMessageBox(IDS_ACQDATA_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void CViewADContinuous::ADC_OnQueueDone()
{
	OnStop(TRUE);
	AfxMessageBox(IDS_ACQDATA_TOOFAST);
}

void CViewADContinuous::ADC_OnOverrunError()
{
	OnStop(TRUE);
	AfxMessageBox(IDS_ACQDATA_OVERRUN);
}

void CViewADContinuous::DAC_OnOverrunError()
{
	DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_OVERRUN);
}

void CViewADContinuous::DAC_OnQueueDone()
{
	DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TOOFAST);
}

void CViewADContinuous::DAC_OnTriggerError()
{
	DAC_StopAndLiberateBuffers();
	AfxMessageBox(IDS_DAC_TRIGGERERROR, MB_ICONEXCLAMATION | MB_OK);
}

void CViewADContinuous::ADC_OnBufferDone()
{
	// get buffer off done list
	m_ADC_bufhandle = HBUF(m_ADC_DTAcq32.GetQueue());
	if (m_ADC_bufhandle == nullptr)
		return;

	// get pointer to buffer
	short* pDTbuf;
	m_ecode = olDmGetBufferPtr(m_ADC_bufhandle, (void**)&pDTbuf);
	if (m_ecode == OLNOERROR)
	{
		// update length of data acquired
		ADC_Transfer(pDTbuf);
		auto wave_format = m_inputDataFile.GetpWaveFormat();
		wave_format->sample_count += m_bytesweepRefresh / 2;
		const auto duration = wave_format->sample_count / m_fclockrate;
		auto pdata_buf = m_inputDataFile.GetpRawDataBUF();
		pdata_buf += (m_chsweep1 * wave_format->scan_count);

		// first thing to do: save data to file
		if (wave_format->bADwritetofile)
		{
			const auto flag = m_inputDataFile.AcqDoc_DataAppend(pdata_buf, m_bytesweepRefresh);
			ASSERT(flag);
			// end of acquisition
			if (duration >= wave_format->duration)
			{
				OnStop(TRUE);
				if (m_bhidesubsequent)
				{
					if (!OnStart())
						OnStop(TRUE);	// if bADStart = wrong, then stop AD
					else
					{
						if ((m_inputDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
							ADC_OnBufferDone();
					}
				}
				return;
			}
		}
		else														// no file I/O: refresh screen pos
		{
			if (wave_format->sample_count >= m_chsweeplength * wave_format->scan_count)
				wave_format->sample_count = 0;
		}
		m_ADC_DTAcq32.SetQueue(long(m_ADC_bufhandle));				// tell ADdriver that data buffer is free

		// then: display acqDataDoc buffer
		if (wave_format->bOnlineDisplay)								// display data if requested
			m_ChartDataWnd.ADdisplayBuffer(pdata_buf, m_chsweepRefresh);
		CString cs;
		cs.Format(_T("%.3lf"), duration);							// update total time on the screen
		SetDlgItemText(IDC_STATIC1, cs);							// update time elapsed
	}
}

void CViewADContinuous::DAC_OnBufferDone()
{
	// get buffer off done list
	m_DAC_bufhandle = HBUF(m_DAC_DTAcq32.GetQueue());
	if (m_DAC_bufhandle == nullptr)
		return;

	// get pointer to buffer
	short* pDTbuf;
	m_ecode = olDmGetBufferPtr(m_DAC_bufhandle, (void**)&pDTbuf);

	if (m_ecode == OLNOERROR)
	{
		DAC_FillBuffer(pDTbuf);
		m_DAC_DTAcq32.SetQueue((long)m_DAC_bufhandle);
	}
}

void CViewADContinuous::ADC_Transfer(short* pDTbuf0)
{
	// get pointer to file waveFormat
	CWaveFormat* wave_format = m_inputDataFile.GetpWaveFormat();
	short* pdataBuf = m_inputDataFile.GetpRawDataBUF();		// acqDataDoc buffer

	m_chsweep1 = m_chsweep2 + 1;								// update data abcissa on the screen
	if (m_chsweep1 >= m_chsweeplength)						// if data are out of the screen, wrap
		m_chsweep1 = 0;
	m_chsweep2 = m_chsweep1 + m_ADC_chbuflen - 1;				// abcissa of the last data point
	m_chsweepRefresh = m_chsweep2 - m_chsweep1 + 1;			// number of data points to refresh on the screen
	pdataBuf += (m_chsweep1 * wave_format->scan_count);

	// if offset binary (unsigned words), transform data into signed integers (two's complement)
	if ((m_pADC_options->waveFormat).binzero != NULL)
	{
		const auto binzero = WORD((m_pADC_options->waveFormat).binzero);
		auto p_dt_buffer = reinterpret_cast<WORD*>(pDTbuf0);
		for (auto j = 0; j < m_ADC_buflen; j++, p_dt_buffer++)
			*p_dt_buffer -= binzero;
	}

	// no undersampling.. copy DTbuffer into data file buffer
	if (m_pADC_options->iundersample <= 1)
	{
		memcpy(pdataBuf, pDTbuf0, m_ADC_buflen * sizeof(short));
	}
	// undersampling (assume that buffer length is a multiple of iundersample) and copy into data file buffer
	else
	{
		auto pdata_buf2 = pdataBuf;
		auto p_dt_buffer = pDTbuf0;
		const auto iundersample = m_pADC_options->iundersample;
		m_chsweepRefresh = m_chsweepRefresh / iundersample;
		// loop and compute average between consecutive points
		for (auto j = 0; j < wave_format->scan_count; j++, pdata_buf2++, p_dt_buffer++)
		{
			auto p_source = p_dt_buffer;
			auto p_dest = pdata_buf2;
			for (auto i = 0; i < m_ADC_chbuflen; i += iundersample)
			{
				long sum = 0;
				for (auto k = 0; k < iundersample; k++)
				{
					sum += *p_source;
					p_source += wave_format->scan_count;
				}
				*p_dest = static_cast<short>(sum / iundersample);
				p_dest += wave_format->scan_count;
			}
		}
	}
	// update byte length of buffer
	m_bytesweepRefresh = m_chsweepRefresh * sizeof(short) * wave_format->scan_count;
}

BOOL CViewADContinuous::InitConnectionWithAmplifiers()
{
	CCyberAmp m_cyber;
	auto bcyber_present = FALSE;
	const auto nchans = (m_pADC_options->chanArray).ChanArray_getSize();
	for (auto i = 0; i < nchans; i++)
	{
		const auto pchan = (m_pADC_options->chanArray).Get_p_channel(i);

		// test if Cyberamp320 selected
		const auto a = pchan->am_csamplifier.Find(_T("CyberAmp"));
		const auto b = pchan->am_csamplifier.Find(_T("Axon Instrument"));
		if (a >= 0 || b >= 0)
		{
			// test if cyberamp present
			if (!bcyber_present)
				bcyber_present = (m_cyber.Initialize() == C300_SUCCESS);
			if (!bcyber_present) {
				AfxMessageBox(_T("CyberAmp not found"), MB_OK);
				continue;
			}
			m_cyber.SetWaveChanParms(pchan);
		}

		// test if Alligator selected
		if (pchan->am_csamplifier.Find(_T("Alligator")) >= 0)
		{
			m_Alligator.SetWaveChanParms(pchan, static_cast<USBPxxPARAMETERS*>(alligatorparameters_ptr_array.GetAt(0)));
		}
	}
	return bcyber_present;
}

void CViewADContinuous::OnBnClickedGainbutton()
{
	((CButton*)GetDlgItem(IDC_BIAS_button))->SetState(0);
	((CButton*)GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

void CViewADContinuous::OnBnClickedBiasbutton()
{
	// set bias down and set gain up CButton
	((CButton*)GetDlgItem(IDC_BIAS_button))->SetState(1);
	((CButton*)GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

void CViewADContinuous::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// CViewData scroll: vertical scroll bar
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

void CViewADContinuous::SetVBarMode(short bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}

void CViewADContinuous::OnGainScroll(UINT nSBCode, UINT nPos)
{
	// assume that all channels are displayed at the same gain & offset
	const auto ichan = 0;			// TODO see which channel is selected
	auto l_size = m_ChartDataWnd.GetChanlistItem(ichan)->GetYextent();
	switch (nSBCode)
	{
	case SB_LEFT:		l_size = YEXTENT_MIN; break;
	case SB_LINELEFT:	l_size -= l_size / 10 + 1; break;
	case SB_LINERIGHT:	l_size += l_size / 10 + 1; break;
	case SB_PAGELEFT:	l_size -= l_size / 2 + 1; break;
	case SB_PAGERIGHT:	l_size += l_size + 1; break;
	case SB_RIGHT:		l_size = YEXTENT_MAX; break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	l_size = MulDiv(nPos - 50, YEXTENT_MAX, 100); break;
	default:			break;
	}

	// change y extent
	if (l_size > 0) //&& lSize<=YEXTENT_MAX)
	{
		// assume that all channels are displayed at the same gain & offset
		const auto wave_format = &(m_pADC_options->waveFormat);
		const auto ichanfirst = 0;
		const auto ichanlast = wave_format->scan_count - 1;

		for (auto i = ichanfirst; i <= ichanlast; i++)
			m_ChartDataWnd.GetChanlistItem(i)->SetYextent(l_size);
		m_ChartDataWnd.Invalidate();
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

void CViewADContinuous::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	// assume that all channels are displayed at the same gain & offset
	const auto ichan = 0;			// TODO: see which channel is selected
	CChanlistItem* chan = m_ChartDataWnd.GetChanlistItem(ichan);
	auto l_size = chan->GetYzero() - chan->GetDataBinZero();
	const auto yextent = chan->GetYextent();
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:		l_size = YZERO_MIN; break;
	case SB_LINELEFT:	l_size -= yextent / 100 + 1; break;
	case SB_LINERIGHT:	l_size += yextent / 100 + 1; break;
	case SB_PAGELEFT:	l_size -= yextent / 10 + 1; break;
	case SB_PAGERIGHT:	l_size += yextent / 10 + 1; break;
	case SB_RIGHT:		l_size = YZERO_MAX; break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	l_size = (nPos - 50) * (YZERO_SPAN / 100); break;
	default:
		break;
	}

	// try to read data with this new size
	if (l_size > YZERO_MIN && l_size < YZERO_MAX)
	{
		const auto wave_format = &(m_pADC_options->waveFormat);
		const auto ichanfirst = 0;
		const auto ichanlast = wave_format->scan_count - 1;
		for (auto i = ichanfirst; i <= ichanlast; i++)
		{
			CChanlistItem* chan = m_ChartDataWnd.GetChanlistItem(i);
			chan->SetYzero(l_size + chan->GetDataBinZero());
		}
		m_ChartDataWnd.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
	{
		UpdateBiasScroll();
		UpdateChanLegends(0);
		UpdateData(false);
	}
}

void CViewADContinuous::UpdateBiasScroll()
{
	// assume that all channels are displayed at the same gain & offset
	const auto ichan = 0;						// TODO see which channel is selected
	CChanlistItem* chan = m_ChartDataWnd.GetChanlistItem(ichan);
	const auto i_pos = int((chan->GetYzero() - chan->GetDataBinZero())
		* 100 / int(YZERO_SPAN)) + int(50);
	m_scrolly.SetScrollPos(i_pos, TRUE);
}

void CViewADContinuous::UpdateGainScroll()
{
	// assume that all channels are displayed at the same gain & offset
	const auto ichan = 0;
	m_scrolly.SetScrollPos(MulDiv(m_ChartDataWnd.GetChanlistItem(ichan)->GetYextent(), 100, YEXTENT_MAX) + 50, TRUE);
}

void CViewADContinuous::UpdateChanLegends(int chan)
{
	// TODO
	const auto ichan = 0;
	CChanlistItem* pchan = m_ChartDataWnd.GetChanlistItem(ichan);
	auto yzero = pchan->GetYzero();
	auto yextent = pchan->GetYextent();
	auto mv_per_bin = pchan->GetVoltsperDataBin() * 1000.0f;
	//auto binzero = 0;
}

float CViewADContinuous::ValueToVolts(CDTAcq32* pSS, long lVal, double dfGain) const
{
	float f_min, f_max;
	const auto l_res = long(pow(2.0, double(pSS->GetResolution())));
	if (pSS->GetMinRange() != 0.F)
		f_min = pSS->GetMinRange() / float(dfGain);
	else f_min = 0.F;
	if (pSS->GetMaxRange() != 0.F)
		f_max = pSS->GetMaxRange() / float(dfGain);
	else f_max = 0.F;

	//make sure value is sign extended if 2's comp
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
	{
		lVal = lVal & (l_res - 1);
		if (lVal >= (l_res / 2))
			lVal = lVal - l_res;
	}

	// convert to volts
	auto f_volts = lVal * ((f_max - f_min) / l_res);

	// adjust DC offset
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
		f_volts = f_volts + ((f_max + f_min) / 2);
	else
		f_volts = f_volts + f_min;

	return f_volts;
}

long CViewADContinuous::VoltsToValue(CDTAcq32* pSS, float fVolts, double dfGain) const
{
	float f_min;
	float f_max;
	long l_value;

	const auto l_res = long(pow(2., double(pSS->GetResolution())));

	if (pSS->GetMinRange() != 0.F)
		f_min = pSS->GetMinRange() / float(dfGain);
	else f_min = 0.F;
	if (pSS->GetMaxRange() != 0.F)
		f_max = pSS->GetMaxRange() / float(dfGain);
	else f_max = 0.F;

	//clip input to range
	if (fVolts > f_max) fVolts = f_max;
	if (fVolts < f_min) fVolts = f_min;

	//if 2's comp encoding
	if (pSS->GetEncoding() == OLx_ENC_2SCOMP)
	{
		l_value = long((fVolts - (f_min + f_max) / 2) * l_res / (f_max - f_min));
		// adjust for binary wrap if any
		if (l_value == (l_res / 2)) l_value -= 1;
	}
	else
	{
		// convert to offset binary
		l_value = long((fVolts - f_min) * l_res / (f_max - f_min));
		// adjust for binary wrap if any
		if (l_value == l_res) l_value -= 1;
	}
	return l_value;
}

void CViewADContinuous::OnCbnSelchangeCombostartoutput()
{
	m_bStartOutPutMode = ((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->GetCurSel();
	m_pDAC_options->bAllowDA = m_bStartOutPutMode;
	GetDlgItem(IDC_STARTSTOP2)->EnableWindow(m_bStartOutPutMode != 0);
}

void CViewADContinuous::SetCombostartoutput(int option)
{
	((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->SetCurSel(option);
	option = ((CComboBox*)GetDlgItem(IDC_COMBOSTARTOUTPUT))->GetCurSel();
	m_bStartOutPutMode = option;
	m_pDAC_options->bAllowDA = option;
	const BOOL b_enabled = m_bStartOutPutMode != 0;
	GetDlgItem(IDC_STARTSTOP2)->EnableWindow(b_enabled);
}

void CViewADContinuous::DAC_OnBnClickedParameters2()
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
		const auto nchans = DAC_GetNumberOfChans();
		GetDlgItem(IDC_STARTSTOP2)->EnableWindow(nchans > 0);
	}
}

void CViewADContinuous::OnBnClickedWriteToDisk()
{
	m_bADwritetofile = TRUE;
	m_pADC_options->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CViewADContinuous::OnBnClickedOscilloscope()
{
	m_bADwritetofile = FALSE;
	m_pADC_options->waveFormat.bADwritetofile = m_bADwritetofile;
	m_inputDataFile.GetpWaveFormat()->bADwritetofile = m_bADwritetofile;
}

void CViewADContinuous::UpdateRadioButtons()
{
	if (m_bADwritetofile)
		((CButton*)GetDlgItem(IDC_WRITETODISK))->SetCheck(BST_CHECKED);
	else
		((CButton*)GetDlgItem(IDC_OSCILLOSCOPE))->SetCheck(BST_CHECKED);
	UpdateData(TRUE);
}

void CViewADContinuous::OnBnClickedCardFeatures()
{
	CDlgDataTranslationBoard dlg;
	dlg.m_pAnalogIN = &m_ADC_DTAcq32;
	dlg.m_pAnalogOUT = &m_DAC_DTAcq32;
	auto iout = dlg.DoModal();
	//iout++;
}

void CViewADContinuous::DAC_OnBnClickedStartStop()
{
	CString cs;
	if (m_DAC_inprogress)
		DAC_Stop();
	else
		DAC_Start();
}

void CViewADContinuous::DAC_UpdateStartStop(BOOL bStart) const
{
	CString cs;
	if (bStart)
		cs = _T("STOP");
	else
		cs = _T("START");
	GetDlgItem(IDC_STARTSTOP2)->SetWindowTextW(cs);
	GetDlgItem(IDC_STARTSTOP2)->EnableWindow(m_bStartOutPutMode != 0);
	GetDlgItem(IDC_DAPARAMETERS2)->EnableWindow(!bStart);
}

BOOL CViewADContinuous::DAC_Start()
{
	m_DAC_inprogress = FALSE;
	if (DAC_InitSubSystem())
	{
		DAC_DeclareAndFillBuffers();
		try
		{
			m_DAC_DTAcq32.Config();
			m_DAC_DTAcq32.Start();
			m_DAC_inprogress = TRUE;
		}
		catch (COleDispatchException* e)
		{
			CString myError;
			myError.Format(_T("DT-Open Layers Error: %i"), int(e->m_scError)); myError += e->m_strDescription;
			AfxMessageBox(myError);
			e->Delete();
		}
	}
	DAC_UpdateStartStop(m_DAC_inprogress);
	return m_DAC_inprogress;
}

void CViewADContinuous::DAC_Stop()
{
	if (m_DAC_inprogress)
		DAC_StopAndLiberateBuffers();
	DAC_UpdateStartStop(m_DAC_inprogress);
}

//**************************************************************************************
// USBPxxS1 event handler DeviceConnected is executed when a USBPxx-S1 module is
// connected to the USB bus.  The value Handle is passed to us by the operating system
// and is a unique identifier to be used in communication to the device.
// Common practice would be to read all of the previous values stored in the module
// and load up our data structure so that we are aware of the current operation of the
// device.
//**************************************************************************************
void CViewADContinuous::DeviceConnectedUsbpxxs1ctl1(const long handle)
{
	USBPxxPARAMETERS	device1;
	device1.DeviceHandle = handle;
	m_Alligator.readAllParameters(handle, &device1);

	boolean flag_not_found = TRUE;
	for (auto i = 0; i < alligatorparameters_ptr_array.GetSize(); i++)
	{
		const auto ptr = alligatorparameters_ptr_array.GetAt(i);
		if (ptr->DeviceHandle == device1.DeviceHandle)
		{
			flag_not_found = FALSE;
			break;
		}
	}
	if (flag_not_found)
	{
		auto* ptr_new = new USBPxxPARAMETERS();
		*ptr_new = device1;
		alligatorparameters_ptr_array.Add(ptr_new);
	}
	//MessageBox(_T("Alligator amplifier detected"));
}

void CViewADContinuous::DeviceDisconnectedUsbpxxs1ctl1(long Handle)
{
	// TODO: Add your message handler code here
}