#include "StdAfx.h"
#include "DataTranslation_AD.h"

#include "include/DataTranslation/OLERRORS.H"
#include "include/DataTranslation/Olxdadefs.h"

BOOL DataTranslation_AD::OpenSubSystem(const CString card_name)
{
	try
	{
		SetBoard(card_name);
		const int number_of_ADElements = GetDevCaps(OLDC_ADELEMENTS);
		if (number_of_ADElements < 1)
			return FALSE;
		SetSubSysType(OLSS_AD);
		SetSubSysElement(0);
		ASSERT(GetHDass() != NULL);
		m_bsimultaneousStart_AD = GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
		return FALSE;
	}

	// save parameters into CWaveFormat
	CWaveFormat* pWFormat = &(m_pOptions->waveFormat);
	pWFormat->fullscale_volts = GetMaxRange() - GetMinRange();

	// convert into bin scale (nb of divisions)
	int iresolution = GetResolution();
	pWFormat->binspan = ((1L << iresolution) - 1);

	// set max channel number according to input configuration m_numchansMAX
	m_pOptions->bChannelType = GetChannelType();
	if (m_pOptions->bChannelType == OLx_CHNT_SINGLEENDED)
		m_numchansMAX = GetSSCaps(OLSSC_MAXSECHANS);
	else
		m_numchansMAX = GetSSCaps(OLSSC_MAXDICHANS);

	// data encoding (binary or offset encoding)
	pWFormat->mode_encoding = (int)GetEncoding();
	if (pWFormat->mode_encoding == OLx_ENC_BINARY)
		pWFormat->binzero = pWFormat->binspan / 2 + 1;
	else if (pWFormat->mode_encoding == OLx_ENC_2SCOMP)
		pWFormat->binzero = 0;

	// load infos concerning frequency, dma chans, programmable gains
	m_freqmax = GetSSCapsEx(OLSSCE_MAXTHROUGHPUT);	// m_dfMaxThroughput

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

BOOL DataTranslation_AD::InitSubSystem(OPTIONS_ACQDATA* pADC_options)
{
	try
	{
		ASSERT(GetHDass() != NULL);

		// store all values within global parameters array
		m_pOptions = pADC_options;
		CWaveFormat* poptions_acqdatawaveFormat = &(m_pOptions->waveFormat);

		// Set up the ADC - no wrap so we can get buffer reused	
		SetDataFlow(OLx_DF_CONTINUOUS);
		SetWrapMode(OLx_WRP_NONE);
		SetDmaUsage((short)GetSSCaps(OLSSC_NUMDMACHANS));
		SetClockSource(OLx_CLK_INTERNAL);

		// set trigger mode
		int trig = poptions_acqdatawaveFormat->trig_mode;
		if (trig > OLx_TRG_EXTRA) 		trig = 0;
		SetTrigger(trig);

		// number of channels
		if (m_pOptions->bChannelType == OLx_CHNT_SINGLEENDED && GetSSCaps(OLSSC_SUP_SINGLEENDED) == NULL)
			m_pOptions->bChannelType = OLx_CHNT_DIFFERENTIAL;
		if (m_pOptions->bChannelType == OLx_CHNT_DIFFERENTIAL && GetSSCaps(OLSSC_SUP_DIFFERENTIAL) == NULL)
			m_pOptions->bChannelType = OLx_CHNT_SINGLEENDED;
		SetChannelType(m_pOptions->bChannelType);
		if (m_pOptions->bChannelType == OLx_CHNT_SINGLEENDED)
			m_numchansMAX = GetSSCaps(OLSSC_MAXSECHANS);
		else
			m_numchansMAX = GetSSCaps(OLSSC_MAXDICHANS);
		// limit scan_count to m_numchansMAX -
		// this limits the nb of data acquisition channels to max-1 if one wants to use the additional I/O input "pseudo"channel
		// so far, it seems acceptable...
		if (poptions_acqdatawaveFormat->scan_count > m_numchansMAX)
			poptions_acqdatawaveFormat->scan_count = m_numchansMAX;

		// set frequency to value requested, set frequency and get the value returned
		double clockrate = double(poptions_acqdatawaveFormat->chrate) * double(poptions_acqdatawaveFormat->scan_count);
		SetFrequency(clockrate);			// set sampling frequency (total throughput)
		clockrate = GetFrequency();
		poptions_acqdatawaveFormat->chrate = (float)clockrate / poptions_acqdatawaveFormat->scan_count;

		// update channel list (chan & gain)

		SetListSize(poptions_acqdatawaveFormat->scan_count);
		for (int i = 0; i < poptions_acqdatawaveFormat->scan_count; i++)
		{
			// transfer data from CWaveChan to chanlist of the A/D subsystem
			CWaveChan* pChannel = (m_pOptions->chanArray).Get_p_channel(i);
			if (pChannel->am_adchannel > m_numchansMAX - 1 && pChannel->am_adchannel != 16)
				pChannel->am_adchannel = m_numchansMAX - 1;
			SetChannelList(i, pChannel->am_adchannel);
			SetGainList(i, pChannel->am_gainAD);
			const double dGain = GetGainList(i);
			pChannel->am_gainAD = (short)dGain;
			// compute dependent parameters
			pChannel->am_gainamplifier = double(pChannel->am_gainheadstage) * double(pChannel->am_gainpre) * double(pChannel->am_gainpost);
			pChannel->am_gaintotal = pChannel->am_gainamplifier * double(pChannel->am_gainAD);
			pChannel->am_resolutionV = double(poptions_acqdatawaveFormat->fullscale_volts) / pChannel->am_gaintotal / double(poptions_acqdatawaveFormat->binspan);
		}

		// pass parameters to the board and check if errors
		ClearError();
		Config();
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
		return FALSE;
	}
	return TRUE;
}

void DataTranslation_AD::DeclareBuffers()
{
	DeleteBuffers();

	// make sure that buffer length contains at least nacq chans
	CWaveFormat* pWFormat = &(m_pOptions->waveFormat); // get pointer to m_pADC_options wave format
	if (pWFormat->buffersize < pWFormat->scan_count * m_pOptions->iundersample)
		pWFormat->buffersize = pWFormat->scan_count * m_pOptions->iundersample;

	// define buffer length
	const float sweepduration = m_pOptions->sweepduration;
	const long chsweeplength = long(float(sweepduration) * pWFormat->chrate / float(m_pOptions->iundersample));
	m_chbuflen = chsweeplength * m_pOptions->iundersample / pWFormat->bufferNitems;
	m_buflen = m_chbuflen * pWFormat->scan_count;

	// declare buffers to DT
	for (int i = 0; i < pWFormat->bufferNitems; i++)
	{
		ECODE ecode = olDmAllocBuffer(0, m_buflen, &m_bufhandle);
		ecode = OLNOERROR;
		if ((ecode == OLNOERROR) && (m_bufhandle != NULL))
			SetQueue(long(m_bufhandle)); // but buffer onto Ready queue
	}
}

void DataTranslation_AD::DeleteBuffers()
{
	try {
		if (GetHDass() == NULL)
			return;
		Flush();
		HBUF hBuf = NULL;
		do {
			hBuf = (HBUF)GetQueue();
			if (hBuf != NULL)
				if (olDmFreeBuffer(hBuf) != OLNOERROR)
					AfxMessageBox(_T("Error Freeing Buffer"));
		} while (hBuf != NULL);
		m_bufhandle = hBuf;
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
	}
}

void DataTranslation_AD::StopAndLiberateBuffers()
{
	if (!m_inprogress)
		return;

	try {
		Stop();
		Flush();							// flush all buffers to Done Queue
		HBUF hBuf;
		do {
			hBuf = (HBUF)GetQueue();
			if (hBuf != NULL) SetQueue(long(hBuf));
		} while (hBuf != NULL);
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
	}
	m_inprogress = FALSE;
}



void DataTranslation_AD::DTLayerError(COleDispatchException* e)
{
	CString myError;
	myError.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError));
	myError += e->m_strDescription;
	AfxMessageBox(myError);
	e->Delete();
}

void DataTranslation_AD::Start()
{
	Config();
	Start();
	m_inprogress = TRUE;
}

short* DataTranslation_AD::OnBufferDone()
{
	// get buffer off done list	
	m_bufhandle = (HBUF) GetQueue();
	if (m_bufhandle == NULL)
		return nullptr;

	// get pointer to buffer
	short* pDTbuf = nullptr;
	ECODE m_ecode = olDmGetBufferPtr(m_bufhandle, (void**)&pDTbuf);
	if (m_ecode == OLNOERROR)
	{
		return pDTbuf;
	}
	return nullptr;
}

