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

BOOL DataTranslation_AD::InitSubSystem(options_input* pADC_options)
{
	try
	{
		ASSERT(GetHDass() != NULL);

		m_pOptions = pADC_options;
		CWaveFormat* pWFormat = &(m_pOptions->wave_format);

		pWFormat->full_scale_volts = GetMaxRange() - GetMinRange();
		int iresolution = GetResolution();
		pWFormat->bin_span = ((1L << iresolution) - 1);

		// set max channel number according to input configuration m_numchansMAX
		m_pOptions->b_channel_type = GetChannelType();
		if (m_pOptions->b_channel_type == OLx_CHNT_SINGLEENDED)
			m_numchansMAX = GetSSCaps(OLSSC_MAXSECHANS);
		else
			m_numchansMAX = GetSSCaps(OLSSC_MAXDICHANS);

		// data encoding (binary or offset encoding)
		pWFormat->mode_encoding = static_cast<int>(GetEncoding());
		if (pWFormat->mode_encoding == OLx_ENC_BINARY)
			pWFormat->bin_zero = pWFormat->bin_span / 2 + 1;
		else if (pWFormat->mode_encoding == OLx_ENC_2SCOMP)
			pWFormat->bin_zero = 0;

		// load infos concerning frequency, dma chans, programmable gains
		m_freqmax = GetSSCapsEx(OLSSCE_MAXTHROUGHPUT); // m_dfMaxThroughput


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
		if (m_pOptions->b_channel_type == OLx_CHNT_SINGLEENDED && GetSSCaps(OLSSC_SUP_SINGLEENDED) == NULL)
			m_pOptions->b_channel_type = OLx_CHNT_DIFFERENTIAL;
		if (m_pOptions->b_channel_type == OLx_CHNT_DIFFERENTIAL && GetSSCaps(OLSSC_SUP_DIFFERENTIAL) == NULL)
			m_pOptions->b_channel_type = OLx_CHNT_SINGLEENDED;
		SetChannelType(m_pOptions->b_channel_type);
		if (m_pOptions->b_channel_type == OLx_CHNT_SINGLEENDED)
			m_numchansMAX = GetSSCaps(OLSSC_MAXSECHANS);
		else
			m_numchansMAX = GetSSCaps(OLSSC_MAXDICHANS);
		// limit scan_count to m_numchansMAX -
		// this limits the nb of data acquisition channels to max-1 if one wants to use the additional I/O input "pseudo"channel
		// so far, it seems acceptable...
		if (pWFormat->scan_count > m_numchansMAX)
			pWFormat->scan_count = m_numchansMAX;

		// set frequency to value requested, set frequency and get the value returned
		double clockrate = static_cast<double>(pWFormat->sampling_rate_per_channel) * static_cast<double>(pWFormat->scan_count);
		SetFrequency(clockrate); // set sampling frequency (total throughput)
		clockrate = GetFrequency();
		pWFormat->sampling_rate_per_channel = static_cast<float>(clockrate) / float(pWFormat->scan_count);

		// update channel list (chan & gain)

		SetListSize(pWFormat->scan_count);
		for (int i = 0; i < pWFormat->scan_count; i++)
		{
			// transfer data from CWaveChan to chanlist of the A/D subsystem
			CWaveChan* p_channel = (m_pOptions->chan_array).get_p_channel(i);
			if (p_channel->am_adchannel > m_numchansMAX - 1 && p_channel->am_adchannel != 16)
				p_channel->am_adchannel = m_numchansMAX - 1;
			SetChannelList(i, p_channel->am_adchannel);
			SetGainList(i, p_channel->am_gainAD);
			const double dGain = GetGainList(i);
			p_channel->am_gainAD = static_cast<short>(dGain);
			// compute dependent parameters
			p_channel->am_amplifiergain = static_cast<double>(p_channel->am_gainheadstage) * static_cast<double>(p_channel
				->am_gainpre) * static_cast<double>(p_channel->am_gainpost);
			p_channel->am_gaintotal = p_channel->am_amplifiergain * static_cast<double>(p_channel->am_gainAD);
			p_channel->am_resolutionV = static_cast<double>(pWFormat->full_scale_volts) / p_channel->am_gaintotal /
				static_cast<double>(pWFormat->bin_span);
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

void DataTranslation_AD::DeclareBuffers(options_input* pADC_options)
{
	DeleteBuffers();

	CWaveFormat* pWFormat = &(pADC_options->wave_format);
	// make sure that buffer length contains at least nacq chans
	if (pWFormat->buffer_size < pWFormat->scan_count * m_pOptions->i_under_sample)
		pWFormat->buffer_size = pWFormat->scan_count * m_pOptions->i_under_sample;

	// define buffer length
	const float sweepduration = m_pOptions->sweep_duration;
	const long chsweeplength = static_cast<long>(float(sweepduration) * pWFormat->sampling_rate_per_channel / float(
		m_pOptions->i_under_sample));
	m_chbuflen = chsweeplength * m_pOptions->i_under_sample / pWFormat->buffer_n_items;
	m_buflen = m_chbuflen * pWFormat->scan_count;

	// declare buffers to DT
	for (int i = 0; i < pWFormat->buffer_n_items; i++)
	{
		ECODE ecode = olDmAllocBuffer(0, m_buflen, &m_bufhandle);
		ecode = OLNOERROR;
		if ((ecode == OLNOERROR) && (m_bufhandle != nullptr))
			SetQueue(long(m_bufhandle));
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
		} while (hBuf != nullptr);
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
		} while (hBuf != nullptr);
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



