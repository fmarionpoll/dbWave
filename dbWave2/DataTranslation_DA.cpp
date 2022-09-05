#include "StdAfx.h"
#include "DataTranslation_DA.h"
#include <OLERRORS.H>
#include <Olmem.h>
#include <Olxdadefs.h>
#include "DlgDAChannels.h"
#include "OPTIONS_INPUTDATA.h"
#include "IntervalPoints.h"


BOOL DataTranslation_DA::OpenSubSystem(const CString card_name)
{
	try
	{
		SetBoard(card_name);
		if (GetDevCaps(OLSS_DA) == 0)
			return FALSE;

		SetSubSysType(OLSS_DA);
		const int nDA = GetDevCaps(OLDC_DAELEMENTS);
		if (nDA < 1)
			return FALSE;
		SetSubSysElement(0);
		ASSERT(GetHDass() != NULL),
			m_bsimultaneousStart_DA = GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);
	}
	catch (COleDispatchException* e)
	{
		DispatchException(e);
		return FALSE;
	}
	return TRUE;
}

BOOL DataTranslation_DA::ClearAllOutputs()
{
	try
	{
		if (GetHDass() == NULL)
			return FALSE;

		if (GetSSCaps(OLSSC_SUP_SINGLEVALUE) == FALSE)
		{
			AfxMessageBox(_T("D/A SubSystem cannot run in single value mode"));
			return FALSE;
		}
		ClearError();
		SetDataFlow(OLx_DF_SINGLEVALUE);
		long out_value = 0;
		if (GetEncoding() == OLx_ENC_BINARY)
			out_value = static_cast<WORD>((out_value ^ m_msbit) & m_lRes);

		Config();
		int nchansmax = GetSSCaps(OLSSC_NUMCHANNELS) - 1;
		for (int i = 0; i < 2; i++)
		{
			PutSingleValue(i, 1.0, out_value);
		}
	}
	catch (COleDispatchException* e)
	{
		DispatchException(e);
		return FALSE;
	}
	return TRUE;
}

BOOL DataTranslation_DA::InitSubSystem(const OPTIONS_INPUTDATA* pADC_options)
{
	const auto ADC_channel_samplingrate = double(pADC_options->waveFormat.sampling_rate_per_channel);
	const int ADC_trigger_mode = int( pADC_options->waveFormat.trig_mode);
	try
	{
		if (GetHDass() == NULL)
			return FALSE;

		// Set up the ADC - multiple wrap so we can get buffer reused	
		SetDataFlow(OLx_DF_CONTINUOUS);
		SetWrapMode(OLx_WRP_NONE);
		SetDmaUsage(static_cast<short>(GetSSCaps(OLSSC_NUMDMACHANS)));

		// set clock the same as for A/D
		SetClockSource(OLx_CLK_INTERNAL);
		const double clock_rate = ADC_channel_samplingrate;
		SetFrequency(clock_rate); // set sampling frequency (total throughput)

		// set trigger mode
		int trig = ADC_trigger_mode;
		if (trig > OLx_TRG_EXTRA)
			trig = 0;
		SetTrigger(trig);

		SetChannelList();
		const double resolution_factor = pow(2.0, GetResolution());
		m_msbit = static_cast<long>(pow(2.0, double(GetResolution()) - 1.));
		m_lRes = static_cast<long>(resolution_factor - 1.);

		for (int i = 0; i < _options_OutputData.outputparms_array.GetSize(); i++)
		{
			OUTPUTPARMS* pParms = &_options_OutputData.outputparms_array.GetAt(i);
			//MSequence(TRUE, pParms);
			if (pParms->bDigital)
				continue;
			const double delta = static_cast<double>(GetMaxRange()) - static_cast<double>(GetMinRange());
			pParms->ampUp = pParms->dAmplitudeMaxV * resolution_factor / delta;
			pParms->ampLow = pParms->dAmplitudeMinV * resolution_factor / delta;
		}

		// pass parameters to the board and check if errors
		ClearError();
		Config();
	}
	catch (COleDispatchException* e)
	{
		DispatchException(e);
		return FALSE;
	}
	return TRUE;
}

void DataTranslation_DA::SetChannelList()
{
	int nanalogOutputs = 0;
	int ndigitalOutputs = 0;

	for (int i = 0; i < _options_OutputData.outputparms_array.GetSize(); i++)
	{
		const OUTPUTPARMS* pParms = &_options_OutputData.outputparms_array.GetAt(i);
		if (!pParms->bON)
			continue;
		if (!pParms->bDigital)
			nanalogOutputs++;
		else
			ndigitalOutputs++;
	}
	m_digitalchannel = 0;
	int nchans = nanalogOutputs;
	nchans += (ndigitalOutputs > 0);

	try
	{
		// number of D/A channels
		int nchansmax = GetSSCaps(OLSSC_NUMCHANNELS);
		ASSERT(nchans <= nchansmax);
		SetListSize(nchans);

		if (nanalogOutputs)
		{
			for (int i = 0; i < nchansmax; i++)
			{
				if (nanalogOutputs > 0)
				{
					CDTAcq32::SetChannelList(i, i);
					nanalogOutputs--;
					m_digitalchannel++;
				}
			}
		}

		if (ndigitalOutputs)
		{
			CDTAcq32::SetChannelList(m_digitalchannel, nchansmax - 1);
		}

		m_listsize = GetListSize();
	}
	catch (COleDispatchException* e)
	{
		DispatchException(e);
	}
}

void DataTranslation_DA::DeleteBuffers()
{
	try
	{
		if (GetHDass() == NULL)
			return;

		Flush(); // clean
		HBUF hBuf; // handle to buffer
		do
		{
			// loop until all buffers are removed
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
		DispatchException(e);
	}
}

void DataTranslation_DA::DeclareAndFillBuffers(const OPTIONS_INPUTDATA* pADC_options)
{
	const float sweepduration = pADC_options->sweepduration;
	const float chrate = pADC_options->waveFormat.sampling_rate_per_channel;
	const int nbuffers = pADC_options->waveFormat.bufferNitems;
	
	DeleteBuffers();
	m_frequency = chrate;

	// define buffer length
	long chsweeplength = static_cast<long>(sweepduration * chrate);
	m_chbuflen = chsweeplength / nbuffers;
	m_buflen = m_chbuflen * m_listsize;

	for (int i = 0; i < _options_OutputData.outputparms_array.GetSize(); i++)
	{
		OUTPUTPARMS* outputparms_array = &(_options_OutputData.outputparms_array.GetAt(i));
		outputparms_array->lastphase = 0;
		outputparms_array->lastamp = 0;
	}

	// declare buffers to DT
	m_nBuffersFilledSinceStart = 0;
	for (int i = 0; i <= nbuffers; i++)
	{
		ECODE ecode = olDmAllocBuffer(0, m_buflen, &m_bufhandle);
		short* pDTbuf;
		ecode = olDmGetBufferPtr(m_bufhandle, (void**)&pDTbuf);
		FillBuffer(pDTbuf);
		if ((ecode == OLNOERROR) && (m_bufhandle != nullptr))
		{
			SetQueue(long(m_bufhandle));
		}
	}
}

void DataTranslation_DA::ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, int chan)
{
	for (int i = chan; i < m_buflen; i += m_listsize)
		*(pDTbuf + i) = static_cast<WORD>((*(pDTbuf + i) ^ m_msbit) & m_lRes);
}

void DataTranslation_DA::FillBufferWith_SINUSOID(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double phase = outputparms_array->lastphase;
	double freq = outputparms_array->dFrequency / m_frequency;
	const double amplitude = (outputparms_array->ampUp - outputparms_array->ampLow) / 2;
	const double offset = (outputparms_array->ampUp + outputparms_array->ampLow) / 2;
	const int n_channels = m_listsize;

	const double pi2 = 3.1415927 * 2;
	freq = freq * pi2;
	for (int i = chan; i < m_buflen; i += n_channels)
	{
		*(pDTbuf + i) = static_cast<short>(cos(phase) * amplitude + offset);
		if (*(pDTbuf + i) > m_msbit)
			*(pDTbuf + i) = static_cast<short>(m_msbit - 1);
		phase += freq;
		if (phase > pi2)
			phase -= pi2;
	}

	if (GetEncoding() == OLx_ENC_BINARY)
		ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
	outputparms_array->lastamp = amplitude;
}

void DataTranslation_DA::FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double phase = outputparms_array->lastphase;
	const double freq = outputparms_array->dFrequency / m_frequency;
	const double amp_up = outputparms_array->ampUp;
	const double amp_low = outputparms_array->ampLow;
	const int n_channels = m_listsize;

	for (int i = chan; i < m_buflen; i += n_channels)
	{
		double amp;
		if (phase < 0)
			amp = amp_up;
		else
			amp = amp_low;
		*(pDTbuf + i) = static_cast<short>(amp);
		phase += freq;
		if (phase > 0.5)
			phase -= 1;
	}

	if (GetEncoding() == OLx_ENC_BINARY)
		ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
}

void DataTranslation_DA::FillBufferWith_TRIANGLE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double phase = outputparms_array->lastphase;
	const double freq = outputparms_array->dFrequency / m_frequency;
	double amp = outputparms_array->ampUp;
	int n_channels = m_listsize;

	for (int i = chan; i < m_buflen; i += n_channels)
	{
		*(pDTbuf + i) = static_cast<short>(2 * phase * amp);
		// clip value
		if (*(pDTbuf + i) >= m_msbit)
			*(pDTbuf + i) = static_cast<short>(m_msbit - 1);
		phase = phase + 2 * freq;
		if (phase > 0.5)
		{
			phase -= 1;
			amp--;
		}
	}

	if (GetEncoding() == OLx_ENC_BINARY)
		ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
	outputparms_array->lastamp = amp;
}

void DataTranslation_DA::FillBufferWith_RAMP(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double amp = outputparms_array->ampUp;
	int nchans = m_listsize;

	for (int i = chan; i < m_buflen; i += nchans)
		*(pDTbuf + i) = static_cast<WORD>(amp);

	if (GetEncoding() == OLx_ENC_BINARY)
		ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = amp;
}

void DataTranslation_DA::FillBufferWith_CONSTANT(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double delta = static_cast<double>(GetMaxRange()) - static_cast<double>(GetMinRange());
	double amp = outputparms_array->value * pow(2.0, GetResolution()) / delta;
	int nchans = m_listsize;

	for (int i = chan; i < m_buflen; i += nchans)
		*(pDTbuf + i) = static_cast<WORD>(amp);

	if (GetEncoding() == OLx_ENC_BINARY)
		ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = amp;
}

void DataTranslation_DA::FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double ampUp = outputparms_array->ampUp;
	double ampLow = outputparms_array->ampLow;
	int nchans = m_listsize;

	CIntervalPoints* pstim = &outputparms_array->sti;
	double chFreqRatio = m_frequency / pstim->chrate;
	long buffer_start = m_nBuffersFilledSinceStart * m_chbuflen;
	long buffer_end = (m_nBuffersFilledSinceStart + 1) * m_chbuflen;
	long buffer_ii = buffer_start;
	int interval = 0;

	BOOL wamp = FALSE;
	long stim_end = 0;

	// find end = first interval after buffer_end; find start 
	for (interval = 0; interval < pstim->GetSize(); interval++)
	{
		stim_end = static_cast<long>(pstim->GetIntervalPointAt(interval).ii * chFreqRatio);
		if (stim_end > buffer_start)
			break;
		wamp = pstim->GetIntervalPointAt(interval).w;
	}
	double amp = ampUp * wamp + ampLow * !wamp;
	WORD wout = static_cast<WORD>(amp);
	if (GetEncoding() == OLx_ENC_BINARY)
		wout = static_cast<WORD>(wout ^ m_msbit) & m_lRes;

	// fill buffer
	for (int i = chan; i < m_buflen; i += nchans, buffer_ii++)
	{
		*(pDTbuf + i) = wout;

		if ((interval < pstim->GetSize()) && buffer_ii >= stim_end)
		{
			interval++;
			wamp = FALSE;
			if (interval < pstim->GetSize())
				stim_end = static_cast<long>(pstim->GetIntervalPointAt(interval).ii * chFreqRatio);
			wamp = pstim->GetIntervalPointAt(interval - 1).w;
			amp = ampUp * wamp + ampLow * !wamp;
			wout = static_cast<WORD>(amp);
			if (GetEncoding() == OLx_ENC_BINARY)
				wout = static_cast<WORD>(wout ^ m_msbit) & m_lRes;
		}
	}
	outputparms_array->lastamp = amp;
}

void DataTranslation_DA::MSequence(BOOL bStart, OUTPUTPARMS* outputparms_array)
{
	outputparms_array->count--;
	if (outputparms_array->count == 0)
	{
		outputparms_array->count = outputparms_array->mseq_iRatio + 1;
		if (bStart)
		{
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

void DataTranslation_DA::FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	int DAClistsize = m_listsize;
	double x = 0;
	int mseqOffsetDelay = outputparms_array->mseq_iDelay;

	for (int i = chan; i < m_buflen; i += DAClistsize)
	{
		x = 0;
		if (outputparms_array->mseq_iDelay > 0)
			outputparms_array->mseq_iDelay--;
		else
		{
			x = outputparms_array->ampLow;
			if (outputparms_array->mseq_iDelay == 0)
			{
				MSequence(FALSE, outputparms_array);
				x = (outputparms_array->bit1 * outputparms_array->ampUp) + (!outputparms_array->bit1 * outputparms_array
					->ampLow);
			}
		}
		*(pDTbuf + i) = static_cast<WORD>(x);
	}

	if (GetEncoding() == OLx_ENC_BINARY)
		ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = x;
}

void DataTranslation_DA::Dig_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	WORD ampLow = 0;
	WORD ampUp = 1;
	ampUp = ampUp << outputparms_array->iChan;
	int nchans = m_listsize;

	CIntervalPoints* pstim = &outputparms_array->sti;
	double chFreqRatio = m_frequency / pstim->chrate;
	long buffer_start = m_nBuffersFilledSinceStart * m_chbuflen;
	long buffer_end = (m_nBuffersFilledSinceStart + 1) * m_chbuflen;
	long buffer_ii = buffer_start;
	int interval = 0;

	BOOL wamp = 0;
	long stim_end = 0;

	// find end = first interval after buffer_end; find start 
	for (interval = 0; interval < pstim->GetSize(); interval++)
	{
		stim_end = static_cast<long>(pstim->GetIntervalPointAt(interval).ii * chFreqRatio);
		if (stim_end > buffer_start)
			break;
		wamp = pstim->GetIntervalPointAt(interval).w;
	}
	WORD wout = ampLow;
	if (wamp > 0)
		wout = ampUp;

	// fill buffer
	for (int i = chan; i < m_buflen; i += nchans, buffer_ii++)
	{
		if (m_digitalfirst == 0)
			*(pDTbuf + i) = wout;
		else
			*(pDTbuf + i) |= wout;

		if ((interval < pstim->GetSize()) && buffer_ii >= stim_end)
		{
			interval++;
			if (interval < pstim->GetSize())
				stim_end = static_cast<long>(pstim->GetIntervalPointAt(interval).ii * chFreqRatio);
			wamp = pstim->GetIntervalPointAt(interval - 1).w;
			if (wamp > 0)
				wout = ampUp;
			else
				wout = ampLow;
		}
	}
}

void DataTranslation_DA::Dig_FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double phase = outputparms_array->lastphase;
	WORD amp = 0;
	WORD ampUp = 1;
	ampUp = ampUp << outputparms_array->iChan;
	WORD ampLow = 0;
	double Freq = outputparms_array->dFrequency / m_frequency;
	int nchans = m_listsize;

	for (int i = chan; i < m_buflen; i += nchans)
	{
		if (phase < 0)
			amp = ampUp;
		else
			amp = ampLow;

		if (m_digitalfirst == 0)
			*(pDTbuf + i) = amp;
		else
			*(pDTbuf + i) |= amp;

		phase += Freq;
		if (phase > 0.5)
			phase -= 1;
	}
	outputparms_array->lastphase = phase;
}

void DataTranslation_DA::Dig_FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	WORD ampLow = 0;
	WORD ampUp = 1;
	ampUp = ampUp << outputparms_array->iChan;
	int DAClistsize = m_listsize;
	double x = 0;
	int mseqOffsetDelay = outputparms_array->mseq_iDelay;

	for (int i = chan; i < m_buflen; i += DAClistsize)
	{
		x = 0;
		if (outputparms_array->mseq_iDelay > 0)
			outputparms_array->mseq_iDelay--;
		else
		{
			x = outputparms_array->ampLow;
			if (outputparms_array->mseq_iDelay == 0)
			{
				MSequence(FALSE, outputparms_array);
				x = static_cast<double>(outputparms_array->bit1) * static_cast<double>(ampUp) + static_cast<double>(!
					outputparms_array->bit1) * static_cast<double>(ampLow);
			}
		}
		if (m_digitalfirst == 0)
			*(pDTbuf + i) = static_cast<WORD>(x);
		else
			*(pDTbuf + i) |= static_cast<WORD>(x);
	}
	outputparms_array->lastamp = x;
}

void DataTranslation_DA::FillBuffer(short* pDTbuf)
{
	int janalog = 0;
	m_digitalfirst = 0;
	for (int i = 0; i < _options_OutputData.outputparms_array.GetSize(); i++)
	{
		OUTPUTPARMS* pParms = &_options_OutputData.outputparms_array.GetAt(i);
		if (!pParms->bON)
			continue;

		if (!pParms->bDigital)
		{
			switch (pParms->iWaveform)
			{
			case DA_SINEWAVE:
				FillBufferWith_SINUSOID(pDTbuf, janalog, pParms);
				break;
			case DA_SQUAREWAVE:
				FillBufferWith_SQUARE(pDTbuf, janalog, pParms);
				break;
			case DA_TRIANGLEWAVE:
				FillBufferWith_TRIANGLE(pDTbuf, janalog, pParms);
				break;
			case DA_LINEWAVE:
				FillBufferWith_RAMP(pDTbuf, janalog, pParms);
				break;
			case DA_SEQUENCEWAVE:
				FillBufferWith_ONOFFSeq(pDTbuf, janalog, pParms);
				break;
			case DA_MSEQWAVE:
				FillBufferWith_MSEQ(pDTbuf, janalog, pParms);
				break;
			case DA_CONSTANT:
			default:
				FillBufferWith_CONSTANT(pDTbuf, janalog, pParms);
				break;
			}
			janalog++;
		}
		else
		{
			switch (pParms->iWaveform)
			{
			case DA_SQUAREWAVE:
				Dig_FillBufferWith_SQUARE(pDTbuf, m_digitalchannel, pParms);
				break;
			case DA_SEQUENCEWAVE:
				Dig_FillBufferWith_ONOFFSeq(pDTbuf, m_digitalchannel, pParms);
				break;
			case DA_MSEQWAVE:
				Dig_FillBufferWith_MSEQ(pDTbuf, m_digitalchannel, pParms);
				break;
			case DA_ONE:
			case DA_ZERO:
			default:
				break;
			}
			m_digitalfirst++;
		}
	}
	m_nBuffersFilledSinceStart++;
}

void DataTranslation_DA::ConfigAndStart()
{
	Config();
	Start();
	m_inprogress = true;
}

void DataTranslation_DA::StopAndLiberateBuffers()
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
			if (hBuf != nullptr)
			{
				ECODE ecode = olDmFreeBuffer(hBuf);
				if (ecode != OLNOERROR)
					AfxMessageBox(_T("Could not free Buffer"));
			}
		}
		while (hBuf != nullptr);

		ClearAllOutputs();
	}
	catch (COleDispatchException* e)
	{
		DispatchException(e);
	}
	m_inprogress = FALSE;
}


void DataTranslation_DA::DispatchException(COleDispatchException* e)
{
	CString myError;
	myError.Format(_T("DT-Open Layers Error: %i "), static_cast<int>(e->m_scError));
	myError += e->m_strDescription;
	AfxMessageBox(myError);
	e->Delete();
}


void DataTranslation_DA::OnBufferDone()
{
	// get buffer off done list	
	m_bufhandle = (HBUF)GetQueue();
	if (m_bufhandle == nullptr)
		return;

	// get pointer to buffer
	short* pDTbuf;
	ECODE m_ecode = olDmGetBufferPtr(m_bufhandle, (void**)&pDTbuf);

	if (m_ecode == OLNOERROR)
	{
		FillBuffer(pDTbuf);
		SetQueue(reinterpret_cast<long>(m_bufhandle));
	}
}
