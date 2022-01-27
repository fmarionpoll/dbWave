#include "stdafx.h"
#include "DataTranslationADC.h"

#include "chanlistitem.h"
#include "include/DataTranslation/OLERRORS.H"
#include "include/DataTranslation/Olxdadefs.h"

BOOL DataTranslationADC::ADC_OpenSubSystem(CString cardName)
{
	try
	{
		SetBoard(cardName);
		const int number_of_ADElements = GetDevCaps(OLDC_ADELEMENTS);
		if (number_of_ADElements < 1)
			return FALSE;
		SetSubSysType(OLSS_AD);
		SetSubSysElement(0);
		ASSERT(GetHDass() != NULL);
		m_bsimultaneousStartAD = GetSSCaps(OLSSC_SUP_SIMULTANEOUS_START);
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
		return FALSE;
	}

	// save parameters into CWaveFormat
	CWaveFormat* pWFormat = &(m_pADC_options->waveFormat);
	pWFormat->fullscale_volts = GetMaxRange() - GetMinRange();

	// convert into bin scale (nb of divisions)
	int iresolution = GetResolution();
	pWFormat->binspan = ((1L << iresolution) - 1);

	// set max channel number according to input configuration m_numchansMAX
	m_pADC_options->bChannelType = GetChannelType();
	if (m_pADC_options->bChannelType == OLx_CHNT_SINGLEENDED)
		m_ADC_numchansMAX = GetSSCaps(OLSSC_MAXSECHANS);
	else
		m_ADC_numchansMAX = GetSSCaps(OLSSC_MAXDICHANS);

	// data encoding (binary or offset encoding)
	pWFormat->mode_encoding = (int)GetEncoding();
	if (pWFormat->mode_encoding == OLx_ENC_BINARY)
		pWFormat->binzero = pWFormat->binspan / 2 + 1;
	else if (pWFormat->mode_encoding == OLx_ENC_2SCOMP)
		pWFormat->binzero = 0;

	// load infos concerning frequency, dma chans, programmable gains
	m_ADC_freqmax = GetSSCapsEx(OLSSCE_MAXTHROUGHPUT);	// m_dfMaxThroughput

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

BOOL DataTranslationADC::ADC_InitSubSystem(OPTIONS_ACQDATA* pADC_options)
{
	try
	{
		ASSERT(GetHDass() != NULL);

		// store all values within global parameters array
		m_pADC_options = pADC_options;
		CWaveFormat* poptions_acqdatawaveFormat = &(m_pADC_options->waveFormat);

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
		if (m_pADC_options->bChannelType == OLx_CHNT_SINGLEENDED && GetSSCaps(OLSSC_SUP_SINGLEENDED) == NULL)
			m_pADC_options->bChannelType = OLx_CHNT_DIFFERENTIAL;
		if (m_pADC_options->bChannelType == OLx_CHNT_DIFFERENTIAL && GetSSCaps(OLSSC_SUP_DIFFERENTIAL) == NULL)
			m_pADC_options->bChannelType = OLx_CHNT_SINGLEENDED;
		SetChannelType(m_pADC_options->bChannelType);
		if (m_pADC_options->bChannelType == OLx_CHNT_SINGLEENDED)
			m_ADC_numchansMAX = GetSSCaps(OLSSC_MAXSECHANS);
		else
			m_ADC_numchansMAX = GetSSCaps(OLSSC_MAXDICHANS);
		// limit scan_count to m_numchansMAX -
		// this limits the nb of data acquisition channels to max-1 if one wants to use the additional I/O input "pseudo"channel
		// so far, it seems acceptable...
		if (poptions_acqdatawaveFormat->scan_count > m_ADC_numchansMAX)
			poptions_acqdatawaveFormat->scan_count = m_ADC_numchansMAX;

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
			CWaveChan* pChannel = (m_pADC_options->chanArray).Get_p_channel(i);
			if (pChannel->am_adchannel > m_ADC_numchansMAX - 1 && pChannel->am_adchannel != 16)
				pChannel->am_adchannel = m_ADC_numchansMAX - 1;
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

void DataTranslationADC::ADC_DeclareBuffers()
{
	ADC_DeleteBuffers();

	// make sure that buffer length contains at least nacq chans
	CWaveFormat* pWFormat = &(m_pADC_options->waveFormat); // get pointer to m_pADC_options wave format
	if (pWFormat->buffersize < pWFormat->scan_count * m_pADC_options->iundersample)
		pWFormat->buffersize = pWFormat->scan_count * m_pADC_options->iundersample;

	// define buffer length
	float m_sweepduration = m_pADC_options->sweepduration;
	long m_chsweeplength = long(float(m_sweepduration) * pWFormat->chrate / float(m_pADC_options->iundersample));
	m_ADC_chbuflen = m_chsweeplength * m_pADC_options->iundersample / pWFormat->bufferNitems;
	m_ADC_buflen = m_ADC_chbuflen * pWFormat->scan_count;

	// declare buffers to DT
	for (int i = 0; i < pWFormat->bufferNitems; i++)
	{
		ECODE ecode = olDmAllocBuffer(0, m_ADC_buflen, &m_ADC_bufhandle);
		ecode = OLNOERROR;
		if ((ecode == OLNOERROR) && (m_ADC_bufhandle != NULL))
			SetQueue(long(m_ADC_bufhandle)); // but buffer onto Ready queue
	}
}

void DataTranslationADC::ADC_DeleteBuffers()
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
		m_ADC_bufhandle = hBuf;
	}
	catch (COleDispatchException* e)
	{
		DTLayerError(e);
	}
}

void DataTranslationADC::ADC_StopAndLiberateBuffers()
{
	if (!m_ADC_inprogress)
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
	m_ADC_inprogress = FALSE;
}

void DataTranslationADC::ADC_Transfer(short* pDTbuf0)
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
		const int iundersample = m_pADC_options->iundersample;
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
				*pDest = short(SUM / iundersample);
				pDest += pWFormat->scan_count;
			}
		}
	}
	// update byte length of buffer
	m_bytesweepRefresh = m_chsweepRefresh * int(sizeof(short)) * int(pWFormat->scan_count);
}

void DataTranslationADC::DTLayerError(COleDispatchException* e)
{
	CString myError;
	myError.Format(_T("DT-Open Layers Error: %i "), int(e->m_scError));
	myError += e->m_strDescription;
	AfxMessageBox(myError);
	e->Delete();
}

void DataTranslationADC::ADC_Start()
{
	Config();
	Start();
	m_ADC_inprogress = TRUE;
}

void DataTranslationADC::ADC_OnBufferDone()
{
	// get buffer off done list	
	m_ADC_bufhandle = (HBUF) GetQueue();
	if (m_ADC_bufhandle == NULL)
		return;

	// get pointer to buffer
	short* pDTbuf;
	ECODE m_ecode = olDmGetBufferPtr(m_ADC_bufhandle, (void**)&pDTbuf);
	if (m_ecode == OLNOERROR)
	{
		// update length of data acquired
		ADC_Transfer(pDTbuf);										// transfer data into intermediary buffer

		CWaveFormat* pWFormat = m_inputDataFile.GetpWaveFormat();	// pointer to data descriptor
		pWFormat->sample_count += m_bytesweepRefresh / 2;				// update total sample count
		const float duration = float(pWFormat->sample_count) / m_fclockrate;

		short* pdataBuf = m_inputDataFile.GetpRawDataBUF();
		pdataBuf += (m_chsweep1 * pWFormat->scan_count);

		// first thing to do: save data to file
		if (pWFormat->bADwritetofile)								// write buffer to file
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
						StopAcquisition(TRUE);	// if bADStart = wrong, then stop AD
					else
					{
						if ((m_inputDataFile.GetpWaveFormat())->trig_mode == OLx_TRG_EXTERN)
							ADC_OnBufferDone();
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
		SetQueue(long(m_ADC_bufhandle));				// tell ADdriver that data buffer is free

		// then: display options_acqdataataDoc buffer
		if (pWFormat->bOnlineDisplay)								// display data if requested
			m_ADsourceView.ADdisplayBuffer(pdataBuf, m_chsweepRefresh);
		CString cs;
		cs.Format(_T("%.3lf"), duration);							// update total time on the screen
		SetDlgItemText(IDC_STATIC1, cs);							// update time elapsed
	}
}

