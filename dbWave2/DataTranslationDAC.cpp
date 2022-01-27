#include "stdafx.h"
#include "DataTranslationDAC.h"

#include <olerrors.h>
#include <olmem.h>
#include <Olxdadefs.h>

#include "DlgDAChannels.h"


BOOL DataTranslationDAC::DAC_OpenSubSystem(const CString cardName)
{
	try
	{
		SetBoard(cardName);
		if (GetDevCaps(OLSS_DA) == 0)
			return FALSE;

		SetSubSysType(OLSS_DA);
		const int nDA = GetDevCaps(OLDC_DAELEMENTS);
		if (nDA < 1)
			return FALSE;
		SetSubSysElement(0);
		ASSERT(GetHDass() != NULL),
			m_bsimultaneousStartDA = GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);

	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
		return FALSE;
	}
	return TRUE;
}

BOOL DataTranslationDAC::DAC_ClearAllOutputs()
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
			out_value = WORD((out_value ^ m_DACmsbit) & m_DAClRes);

		Config();
		int nchansmax = GetSSCaps(OLSSC_NUMCHANNELS) - 1;
		for (int i = 0; i < 2; i++)
		{
			PutSingleValue(i, 1.0, out_value);
		}
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
		return FALSE;
	}
	return TRUE;
}

BOOL DataTranslationDAC::DAC_InitSubSystem()
{
	try
	{
		if (GetHDass() == NULL)
			return FALSE;

		// Set up the ADC - multiple wrap so we can get buffer reused	
		SetDataFlow(OLx_DF_CONTINUOUS);
		SetWrapMode(OLx_WRP_NONE);
		SetDmaUsage(short(GetSSCaps(OLSSC_NUMDMACHANS)));

		// set clock the same as for A/D
		SetClockSource(OLx_CLK_INTERNAL);
		const double clockrate = m_pADC_options->waveFormat.chrate;
		SetFrequency(clockrate);		// set sampling frequency (total throughput)

		// set trigger mode
		int trig = m_pADC_options->waveFormat.trig_mode;
		if (trig > OLx_TRG_EXTRA)
			trig = 0;
		SetTrigger(trig);

		DAC_SetChannelList();
		const double resolutionfactor = pow(2.0, GetResolution());
		m_DACmsbit = long(pow(2.0, double(GetResolution()) - 1.));
		m_DAClRes = long(resolutionfactor - 1.);

		for (int i = 0; i < m_pDAC_options->outputparms_array.GetSize(); i++)
		{
			OUTPUTPARMS* pParms = &m_pDAC_options->outputparms_array.GetAt(i);
			//DAC_MSequence(TRUE, pParms);
			if (pParms->bDigital)
				continue;
			const double delta = double(GetMaxRange()) - double(GetMinRange());
			pParms->ampUp = pParms->dAmplitudeMaxV * resolutionfactor / delta;
			pParms->ampLow = pParms->dAmplitudeMinV * resolutionfactor / delta;
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

void DataTranslationDAC::DAC_SetChannelList()
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
		int nchansmax = GetSSCaps(OLSSC_NUMCHANNELS);
		ASSERT(nchans <= nchansmax);
		SetListSize(nchans);

		if (nanalogOutputs)
		{
			for (int i = 0; i < nchansmax; i++)
			{
				if (nanalogOutputs > 0)
				{
					SetChannelList(i, i);
					nanalogOutputs--;
					m_DACdigitalchannel++;
				}
			}
		}

		if (ndigitalOutputs)
		{
			SetChannelList(m_DACdigitalchannel, nchansmax - 1);
		}

		m_DAClistsize = GetListSize();
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
	}
}

void DataTranslationDAC::DAC_DeleteBuffers()
{
	try {
		if (GetHDass() == NULL)
			return;

		Flush();	// clean
		HBUF hBuf;			// handle to buffer
		do {				// loop until all buffers are removed
			hBuf = (HBUF)GetQueue();
			if (hBuf != NULL)
				if (olDmFreeBuffer(hBuf) != OLNOERROR)
					AfxMessageBox(_T("Error Freeing Buffer"));
		} while (hBuf != NULL);
		m_DAC_bufhandle = hBuf;
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
	}
}

void DataTranslationDAC::DAC_DeclareAndFillBuffers()
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
	for (int i = 0; i <= nbuffers; i++)
	{
		ECODE ecode = olDmAllocBuffer(0, m_DAC_buflen, &m_DAC_bufhandle);
		short* pDTbuf;
		ecode = olDmGetBufferPtr(m_DAC_bufhandle, (void**)&pDTbuf);
		DAC_FillBuffer(pDTbuf);
		if ((ecode == OLNOERROR) && (m_DAC_bufhandle != nullptr))
		{
			SetQueue(long(m_DAC_bufhandle));
		}
	}
}

void DataTranslationDAC::DAC_ConvertbufferFrom2ComplementsToOffsetBinary(short* pDTbuf, int chan)
{
	for (int i = chan; i < m_DAC_buflen; i += m_DAClistsize)
		*(pDTbuf + i) = (WORD)((*(pDTbuf + i) ^ m_DACmsbit) & m_DAClRes);
}

void DataTranslationDAC::DAC_FillBufferWith_SINUSOID(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	phase = outputparms_array->lastphase;
	double	freq = outputparms_array->dFrequency / m_DAC_frequency;
	const double amplitude = (outputparms_array->ampUp - outputparms_array->ampLow) / 2;
	const double offset = (outputparms_array->ampUp + outputparms_array->ampLow) / 2;
	const int DAC_n_channels = m_DAClistsize;

	const double pi2 = 3.1415927 * 2;
	freq = freq * pi2;
	for (int i = chan; i < m_DAC_buflen; i += DAC_n_channels)
	{
		*(pDTbuf + i) = short(cos(phase) * amplitude + offset);
		if (*(pDTbuf + i) > m_DACmsbit)
			*(pDTbuf + i) = short(m_DACmsbit - 1);
		phase += freq;
		if (phase > pi2)
			phase -= pi2;
	}

	if (GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
	outputparms_array->lastamp = amplitude;
}

void DataTranslationDAC::DAC_FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	phase = outputparms_array->lastphase;
	const double freq = outputparms_array->dFrequency / m_DAC_frequency;
	const double amp_up = outputparms_array->ampUp;
	const double amp_low = outputparms_array->ampLow;
	const int DAC_n_channels = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += DAC_n_channels)
	{
		double	amp;
		if (phase < 0)
			amp = amp_up;
		else
			amp = amp_low;
		*(pDTbuf + i) = short(amp);
		phase += freq;
		if (phase > 0.5)
			phase -= 1;
	}

	if (GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
}

void DataTranslationDAC::DAC_FillBufferWith_TRIANGLE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double phase = outputparms_array->lastphase;
	const double freq = outputparms_array->dFrequency / m_DAC_frequency;
	double	amp = outputparms_array->ampUp;
	int DAC_n_channels = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += DAC_n_channels)
	{
		*(pDTbuf + i) = short(2 * phase * amp);
		// clip value
		if (*(pDTbuf + i) >= m_DACmsbit)
			*(pDTbuf + i) = short(m_DACmsbit - 1);
		phase = phase + 2 * freq;
		if (phase > 0.5)
		{
			phase -= 1;
			amp--;
		}
	}

	if (GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastphase = phase;
	outputparms_array->lastamp = amp;
}

void DataTranslationDAC::DAC_FillBufferWith_RAMP(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double	amp = outputparms_array->ampUp;
	int nchans = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += nchans)
		*(pDTbuf + i) = (WORD) amp;

	if (GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = amp;
}

void DataTranslationDAC::DAC_FillBufferWith_CONSTANT(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
{
	double delta = double(GetMaxRange()) - double(GetMinRange());
	double	amp = outputparms_array->value * pow(2.0, GetResolution()) / delta;
	int nchans = m_DAClistsize;

	for (int i = chan; i < m_DAC_buflen; i += nchans)
		*(pDTbuf + i) = (WORD) amp;

	if (GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = amp;
}

void DataTranslationDAC::DAC_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
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
	if (GetEncoding() == OLx_ENC_BINARY)
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
			if (GetEncoding() == OLx_ENC_BINARY)
				wout = (WORD)(wout ^ m_DACmsbit) & m_DAClRes;
		}
	}
	outputparms_array->lastamp = amp;
}

void DataTranslationDAC::DAC_MSequence(BOOL bStart, OUTPUTPARMS* outputparms_array)
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

void DataTranslationDAC::DAC_FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
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

	if (GetEncoding() == OLx_ENC_BINARY)
		DAC_ConvertbufferFrom2ComplementsToOffsetBinary(pDTbuf, chan);

	outputparms_array->lastamp = x;
}

void DataTranslationDAC::DAC_Dig_FillBufferWith_ONOFFSeq(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
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

void DataTranslationDAC::DAC_Dig_FillBufferWith_SQUARE(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
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

void DataTranslationDAC::DAC_Dig_FillBufferWith_MSEQ(short* pDTbuf, int chan, OUTPUTPARMS* outputparms_array)
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
				x = double(outputparms_array->bit1) * double(ampUp) + double(!outputparms_array->bit1) * double(ampLow);
			}
		}
		if (m_DACdigitalfirst == 0)
			*(pDTbuf + i) = WORD(x);
		else
			*(pDTbuf + i) |= WORD(x);
	}
	outputparms_array->lastamp = x;
}

void DataTranslationDAC::DAC_FillBuffer(short* pDTbuf)
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

void DataTranslationDAC::DAC_StopAndLiberateBuffers()
{
	try {
		Stop();
		Flush();	// flush all buffers to Done Queue
		HBUF hBuf;
		do {
			hBuf = (HBUF)GetQueue();
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
		DTLayerError(e);
	}
	m_DAC_inprogress = FALSE;
}


void DataTranslationDAC::DTLayerError(COleDispatchException* e)
{
	CString myError;
	myError.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError));
	myError += e->m_strDescription;
	AfxMessageBox(myError);
	e->Delete();
}


void DataTranslationDAC::OnBufferDone_DAC()
{
	// get buffer off done list	
	m_DAC_bufhandle = (HBUF) GetQueue();
	if (m_DAC_bufhandle == NULL)
		return;

	// get pointer to buffer
	short* pDTbuf;
	ECODE m_ecode = olDmGetBufferPtr(m_DAC_bufhandle, (void**)&pDTbuf);

	if (m_ecode == OLNOERROR)
	{
		DAC_FillBuffer(pDTbuf);
		SetQueue((long)m_DAC_bufhandle);
	}
}

