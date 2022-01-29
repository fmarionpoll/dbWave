#include "StdAfx.h"
#include "DataTranslation_AD.h"

#include "OLERRORS.H"


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

	return TRUE;
}

BOOL DataTranslation_AD::InitSubSystem(OPTIONS_ACQDATA* pADC_options)
{
	try
	{
		ASSERT(GetHDass() != NULL);

		m_pOptions = pADC_options;
		CWaveFormat* pWFormat = &(m_pOptions->waveFormat);

		pWFormat->fullscale_volts = GetMaxRange() - GetMinRange();
		int iresolution = GetResolution();
		pWFormat->binspan = ((1L << iresolution) - 1);

		// set max channel number according to input configuration m_numchansMAX
		m_pOptions->bChannelType = GetChannelType();
		if (m_pOptions->bChannelType == OLx_CHNT_SINGLEENDED)
			m_numchansMAX = GetSSCaps(OLSSC_MAXSECHANS);
		else
			m_numchansMAX = GetSSCaps(OLSSC_MAXDICHANS);

		// data encoding (binary or offset encoding)
		pWFormat->mode_encoding = static_cast<int>(GetEncoding());
		if (pWFormat->mode_encoding == OLx_ENC_BINARY)
			pWFormat->binzero = pWFormat->binspan / 2 + 1;
		else if (pWFormat->mode_encoding == OLx_ENC_2SCOMP)
			pWFormat->binzero = 0;

		// load infos concerning frequency, dma chans, programmable gains
		m_freqmax = GetSSCapsEx(OLSSCE_MAXTHROUGHPUT); // m_dfMaxThroughput

		// TODO tell sourceview here under which format are data
		// TODO save format of data into temp document
		// float volts = (float) ((pWFormat->fullscale_Volts) 
		//				/(pWFormat->binspan) * value  -pWFormat->fullscale_Volts/2);
		// TODO: update max min of chan 1 with gain && instrumental gain
		//UpdateChanLegends(0);
		//UpdateHorizontalRulerBar();
		//UpdateVerticalRulerBar();

		// Set up the ADC - no wrap so we can get buffer reused	
		SetDataFlow(OLx_DF_CONTINUOUS);
		SetWrapMode(OLx_WRP_NONE);
		SetDmaUsage(static_cast<short>(GetSSCaps(OLSSC_NUMDMACHANS)));
		SetClockSource(OLx_CLK_INTERNAL);

		// set trigger mode
		int trig = pWFormat->trig_mode;
		if (trig > OLx_TRG_EXTRA)
			trig = 0;
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
		if (pWFormat->scan_count > m_numchansMAX)
			pWFormat->scan_count = m_numchansMAX;

		// set frequency to value requested, set frequency and get the value returned
		double clockrate = static_cast<double>(pWFormat->chrate) * static_cast<double>(pWFormat->scan_count);
		SetFrequency(clockrate); // set sampling frequency (total throughput)
		clockrate = GetFrequency();
		pWFormat->chrate = static_cast<float>(clockrate) / pWFormat->scan_count;

		// update channel list (chan & gain)

		SetListSize(pWFormat->scan_count);
		for (int i = 0; i < pWFormat->scan_count; i++)
		{
			// transfer data from CWaveChan to chanlist of the A/D subsystem
			CWaveChan* pChannel = (m_pOptions->chanArray).Get_p_channel(i);
			if (pChannel->am_adchannel > m_numchansMAX - 1 && pChannel->am_adchannel != 16)
				pChannel->am_adchannel = m_numchansMAX - 1;
			SetChannelList(i, pChannel->am_adchannel);
			SetGainList(i, pChannel->am_gainAD);
			const double dGain = GetGainList(i);
			pChannel->am_gainAD = static_cast<short>(dGain);
			// compute dependent parameters
			pChannel->am_gainamplifier = static_cast<double>(pChannel->am_gainheadstage) * static_cast<double>(pChannel
				->am_gainpre) * static_cast<double>(pChannel->am_gainpost);
			pChannel->am_gaintotal = pChannel->am_gainamplifier * static_cast<double>(pChannel->am_gainAD);
			pChannel->am_resolutionV = static_cast<double>(pWFormat->fullscale_volts) / pChannel->am_gaintotal /
				static_cast<double>(pWFormat->binspan);
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

void DataTranslation_AD::DeclareBuffers(CWaveFormat* pWFormat)
{
	DeleteBuffers();

	// make sure that buffer length contains at least nacq chans
	if (pWFormat->buffersize < pWFormat->scan_count * m_pOptions->iundersample)
		pWFormat->buffersize = pWFormat->scan_count * m_pOptions->iundersample;

	// define buffer length
	const float sweepduration = m_pOptions->sweepduration;
	const long chsweeplength = static_cast<long>(float(sweepduration) * pWFormat->chrate / float(
		m_pOptions->iundersample));
	m_chbuflen = chsweeplength * m_pOptions->iundersample / pWFormat->bufferNitems;
	m_buflen = m_chbuflen * pWFormat->scan_count;

	// declare buffers to DT
	for (int i = 0; i < pWFormat->bufferNitems; i++)
	{
		ECODE ecode = olDmAllocBuffer(0, m_buflen, &m_bufhandle);
		ecode = OLNOERROR;
		if ((ecode == OLNOERROR) && (m_bufhandle != nullptr))
			SetQueue(long(m_bufhandle)); // but buffer onto Ready queue
	}
}

void DataTranslation_AD::DeleteBuffers()
{
	try
	{
		if (GetHDass() == NULL)
			return;
		Flush();
		HBUF hBuf = nullptr;
		do
		{
			hBuf = (HBUF)GetQueue();
			if (hBuf != nullptr)
				if (olDmFreeBuffer(hBuf) != OLNOERROR)
					AfxMessageBox(_T("Error Freeing Buffer"));
		}
		while (hBuf != nullptr);
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

	try
	{
		Stop();
		Flush(); // flush all buffers to Done Queue
		HBUF hBuf;
		do
		{
			hBuf = (HBUF)GetQueue();
			if (hBuf != nullptr) SetQueue(long(hBuf));
		}
		while (hBuf != nullptr);
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
	myError.Format(_T("DT-Open Layers Error: %i "), static_cast<int>(e->m_scError));
	myError += e->m_strDescription;
	AfxMessageBox(myError);
	e->Delete();
}

void DataTranslation_AD::ConfigAndStart()
{
	Config();
	Start();
	m_inprogress = TRUE;
}

short* DataTranslation_AD::OnBufferDone()
{
	// get buffer off done list	
	m_bufhandle = (HBUF)GetQueue();
	if (m_bufhandle == nullptr)
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

long DataTranslation_AD::VoltsToValue(float fVolts, double dfGain)
{
	const long lRes = static_cast<long>(pow(2., double(GetResolution())));

	float f_min = 0.F;
	if (GetMinRange() != 0.F)
		f_min = GetMinRange() / static_cast<float>(dfGain);

	float f_max = 0.F;
	if (GetMaxRange() != 0.F)
		f_max = GetMaxRange() / static_cast<float>(dfGain);

	//clip input to range
	if (fVolts > f_max)
		fVolts = f_max;
	if (fVolts < f_min)
		fVolts = f_min;

	//if 2's comp encoding
	long l_value;
	if (GetEncoding() == OLx_ENC_2SCOMP)
	{
		l_value = static_cast<long>((fVolts - (f_min + f_max) / 2) * lRes / (f_max - f_min));
		// adjust for binary wrap if any
		if (l_value == (lRes / 2))
			l_value -= 1;
	}
	else
	{
		// convert to offset binary
		l_value = static_cast<long>((fVolts - f_min) * lRes / (f_max - f_min));
		// adjust for binary wrap if any
		if (l_value == lRes)
			l_value -= 1;
	}
	return l_value;
}

float DataTranslation_AD::ValueToVolts(long lVal, double dfGain)
{
	const long lRes = static_cast<long>(pow(2.0, double(GetResolution())));
	float f_min = 0.F;
	if (GetMinRange() != 0.F)
		f_min = GetMinRange() / static_cast<float>(dfGain);

	float f_max = 0.F;
	if (GetMaxRange() != 0.F)
		f_max = GetMaxRange() / static_cast<float>(dfGain);

	//make sure value is sign extended if 2's comp
	if (GetEncoding() == OLx_ENC_2SCOMP)
	{
		lVal = lVal & (lRes - 1);
		if (lVal >= (lRes / 2))
			lVal = lVal - lRes;
	}

	// convert to volts
	float f_volts = static_cast<float>(lVal) * (f_max - f_min) / lRes;
	if (GetEncoding() == OLx_ENC_2SCOMP)
		f_volts = f_volts + ((f_max + f_min) / 2);
	else
		f_volts = f_volts + f_min;

	return f_volts;
}
