#include "StdAfx.h"
#include "datafile_CEDSpike2.h"
#include "dataheader_Atlab.H"
#include "ceds64int.h"
#include <Olxdadefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CDataFileFromCEDSpike2, CDataFileX)
const int LENCEDSON = 16;
const char CEDSON64[] = "CED_DATA";

numberIDToText CDataFileFromCEDSpike2::errorMessages[] = {
	{S64_OK, _T("There was no error")},
	{NO_FILE, _T("Attempt to use when file not open, or use of an invalid file handle, or no spare file handle")},
	{
		NO_BLOCK, _T(
			"Failed to allocate a disk block when writing to the file.The disk is probably full, or there was a disk error.")
	},
	{CALL_AGAIN, _T("This is a long operation, call again.")},
	{NO_ACCESS, _T("This operation was not allowed.")},
	{NO_MEMORY, _T("Out of memory reading a 32 - bit son file.")},
	{NO_CHANNEL, _T("A channel does not exist.")},
	{CHANNEL_USED, _T("Attempt to reuse a channel that already exists.")},
	{CHANNEL_TYPE, _T("The channel cannot be used for this operation.")},
	{PAST_EOF, _T("Read past the end of the file.")},
	{WRONG_FILE, _T("Attempt to open wrong file type.This is not a SON file.")},
	{NO_EXTRA, _T("A request to read user data is outside the extra data region.")},
	{BAD_READ, _T("A read error(disk error was detected.This is an operating system error.")},
	{BAD_WRITE, _T("Something went wrong writing data.This is an operating system error.")},
	{CORRUPT_FILE, _T("The file is bad or an attempt to write corrupted data.")},
	{PAST_SOF, _T("An attempt was made to access data before the start of the file.")},
	{READ_ONLY, _T("Attempt to write to a read only file.")},
	{BAD_PARAM, _T("A bad parameter to a call into the SON library.")},
	{OVER_WRITE, _T("An attempt was made to over - write data when not allowed.")},
	{MORE_DATA, _T("A file is bigger than the header says; maybe not closed correctly.")}
};

CDataFileFromCEDSpike2::CDataFileFromCEDSpike2()
{
	m_bHeaderSize = 512;
	m_ulOffsetData = m_bHeaderSize;
	m_ulOffsetHeader = 0;
	m_idType = DOCTYPE_SMR;
	m_csType = CEDSON64;
}

CDataFileFromCEDSpike2::~CDataFileFromCEDSpike2()
{
	CloseDataFile();
}

#ifdef _DEBUG
void CDataFileFromCEDSpike2::AssertValid() const
{
	CDataFileX::AssertValid();
}

void CDataFileFromCEDSpike2::Dump(CDumpContext& dc) const
{
	CDataFileX::Dump(dc);
}
#endif //_DEBUG

bool CDataFileFromCEDSpike2::OpenDataFile(CString& sz_path_name, UINT u_open_flag)
{
	//ATLTRACE2("--- CED OpenDataFile (old nFid= %i) \n", m_nFid );
	m_nFid = S64Open(CT2A(sz_path_name), -1);
	if (m_nFid <= 0)
	{
		CString message = CString("OpenCEDDataFile error: ") + getErrorMessage(m_nFid);
		AfxMessageBox(message, MB_OK);
		return false;
	}
	return true;
}

void CDataFileFromCEDSpike2::CloseDataFile()
{
	if (m_nFid >= 0)
	{
		//ATLTRACE2("--- CED CloseDataFile (old nFid= %i) \n", m_nFid);
		int flag = S64Close(m_nFid);
		if (flag < 0)
		{
			CString message = CString("CloseCEDDataFile error: ") + getErrorMessage(flag);
			AfxMessageBox(message, MB_OK);
		}
		m_nFid = -1;
	}
}

int CDataFileFromCEDSpike2::CheckFileType(CString& cs_fileName)
{
	cs_fileName.MakeLower();
	if (cs_fileName.Find(_T("smr")) != -1)
	{
		return DOCTYPE_SMR;
	}
	return DOCTYPE_UNKNOWN;
}

BOOL CDataFileFromCEDSpike2::ReadDataInfos(CWaveBuf* pBuf)
{
	CWaveFormat* pWFormat = pBuf->GetpWaveFormat();
	CWaveChanArray* pArray = pBuf->GetpWavechanArray();
	// Read file header
	auto bflag = TRUE;

	//	tentative
	pWFormat->fullscale_volts = 5.0f; // 10 V full scale
	pWFormat->binspan = 65536; // 16 bits resolution
	pWFormat->binzero = 0;
	pWFormat->mode_encoding = OLx_ENC_BINARY;
	pWFormat->mode_clock = INTERNAL_CLOCK;
	pWFormat->mode_trigger = INTERNAL_TRIGGER;
	pWFormat->csADcardName = "CED1401";

	// get comments
	pWFormat->cs_comment.Empty();
	pWFormat->csStimulus.Empty();
	pWFormat->csConcentration.Empty();
	pWFormat->csSensillum.Empty();

	// get global data and n channels
	double timeBase = S64GetTimeBase(m_nFid);
	int lowestFreeChan = S64GetFreeChan(m_nFid);
	int maxChan = S64MaxChans(m_nFid);

	TTimeDate arrayGetTimeDate{};
	int flag = S64TimeDate(m_nFid, (long long*)&arrayGetTimeDate, nullptr, -1);
	if (flag < 0)
	{
		CString message = CString("Error reading date ") + getErrorMessage(flag) + CString(" \n");
		AfxMessageBox(message, MB_OK);
		return false;
	}
	pWFormat->acqtime = CTime(
		arrayGetTimeDate.wYear, arrayGetTimeDate.ucMon, arrayGetTimeDate.ucDay,
		arrayGetTimeDate.ucHour, arrayGetTimeDate.ucMin, arrayGetTimeDate.ucSec);
	pWFormat->scan_count = 0;
	pArray->ChanArray_removeAll();
	int adcChan = -1;

	for (int cedChan = 1; cedChan < maxChan; cedChan++)
	{
		// lowestFreeChan
		int chanType = S64ChanType(m_nFid, cedChan);
		if (chanType == 0)
			continue;

		CString descriptor;
		switch (chanType)
		{
		case CHANTYPE_Adc:
			{
				descriptor = "Adc data";
				int i = pArray->ChanArray_add();
				auto pChan = pArray->Get_p_channel(i);
				read_ChannelParameters(pChan, cedChan);
				pWFormat->scan_count++;
				pWFormat->sampling_rate_per_channel = static_cast<float>(1.0 / (pChan->am_CEDticksPerSample * S64GetTimeBase(m_nFid)));
				pWFormat->sample_count = static_cast<long>(pChan->am_CEDmaxTimeInTicks / pChan->am_CEDticksPerSample);
				adcChan = cedChan;
			}
			break;
		case CHANTYPE_EventFall:
			descriptor = "Event on falling edge";
			read_EventFall(cedChan, pBuf);
			break;
		case CHANTYPE_EventRise:
			descriptor = "Event on rising edge";
			break;
		case CHANTYPE_EventBoth:
			descriptor = "Event on both edges";
			break;
		case CHANTYPE_Marker:
			descriptor = "Marker data";
			break;
		case CHANTYPE_WaveMark:
			descriptor = "WaveMark data";
			break;
		case CHANTYPE_RealMark:
			descriptor = "RealMark data";
			break;
		case CHANTYPE_TextMark:
			descriptor = "TextMark data";
			break;
		case CHANTYPE_RealWave:
			descriptor = "RealWave data";
			break;
		case CHANTYPE_unused:
		default:
			descriptor = "unused channel";
			break;
		}
		//CString comment;
		//comment.Format(_T(" chan %i ["), cedChan);
		//comment += _T("] title = ") + descriptor + read_ChannelTitle(cedChan);
		//comment += _T(" comment =") + read_ChannelComment(cedChan) + _T("\n");
		//ATLTRACE2(comment);
	}
	if (adcChan >= 0)
		convert_VTtags_Ticks_to_ADintervals(pBuf, adcChan);

	CTagList* pTags = pBuf->GetpVTtags();
	if (pTags != nullptr && pTags->GetNTags() > 0 && pWFormat->scan_count > 0)
	{
		for (int i = 0; i < pArray->ChanArray_getSize(); i++)
		{
			auto pChan = pArray->Get_p_channel(i);
			if (m_ticksPerSample != pChan->am_CEDticksPerSample)
				m_ticksPerSample = pChan->am_CEDticksPerSample;
		}
		CTag* pTag = pTags->GetTag(0);
		m_llFileOffset = pTag->m_lTicks / m_ticksPerSample;
		int newlength = pWFormat->sample_count - static_cast<int>(m_llFileOffset);
		pWFormat->sample_count = newlength;
	}

	return TRUE;
}

void CDataFileFromCEDSpike2::read_ChannelParameters(CWaveChan* pChan, int cedChan)
{
	pChan->am_CEDchanID = cedChan;
	pChan->am_csamplifier.Empty(); // amplifier type
	pChan->am_csheadstage.Empty(); // headstage type
	pChan->am_csComment.Empty(); // channel comment
	pChan->am_csComment = read_ChannelComment(cedChan);

	pChan->am_adchannel = 0; // channel scan list
	pChan->am_gainAD = 1; // channel gain list
	pChan->am_gainheadstage = 1; // assume headstage gain = 1
	pChan->am_amplifierchan = 0; // assume 1 channel / amplifier
	pChan->am_gainpre = 1; // assume gain -pre = 1
	pChan->am_gainpost = 1; // assume gain -post = 1
	pChan->am_notchfilt = 0; // assume no notch filter
	pChan->am_lowpass = 0; // assume not low pass filtering
	pChan->am_offset = 0.0f; // assume no offset compensation
	pChan->am_csInputpos = "DC"; // assume input + = DC
	pChan->am_csInputneg = "GND"; // assume input - = GND

	int flag = S64GetChanOffset(m_nFid, cedChan, &pChan->am_CEDoffset);
	flag = S64GetChanScale(m_nFid, cedChan, &pChan->am_CEDscale);

	pChan->am_gainamplifier = pChan->am_CEDscale * 10.;
	pChan->am_resolutionV = 2.5 / pChan->am_gainamplifier / 65536;
	pChan->am_gaintotal = pChan->am_gainamplifier;

	pChan->am_CEDticksPerSample = S64ChanDivide(m_nFid, cedChan);
	pChan->am_CEDmaxTimeInTicks = S64ChanMaxTime(m_nFid, cedChan);
}

CString CDataFileFromCEDSpike2::read_ChannelComment(int cedChan)
{
	int sizeComment = S64GetChanComment(m_nFid, cedChan, nullptr, -1);
	CString comment;
	if (sizeComment > 0)
	{
		auto buffer = new char[sizeComment];
		int flag = S64GetChanComment(m_nFid, cedChan, buffer, 0);
		comment = CString(buffer);
		delete[] buffer;
	}
	return comment;
}

CString CDataFileFromCEDSpike2::read_ChannelTitle(int cedChan)
{
	int sizeComment = S64GetChanTitle(m_nFid, cedChan, nullptr, -1);
	CString comment;
	if (sizeComment > 0)
	{
		auto buffer = new char[sizeComment];
		int flag = S64GetChanTitle(m_nFid, cedChan, buffer, 0);
		comment = CString(buffer);
		delete[] buffer;
	}
	return comment;
}

CString CDataFileFromCEDSpike2::read_FileComment(int nInd)
{
	int sizeComment = S64GetFileComment(m_nFid, nInd, nullptr, -1);
	CString comment;
	if (sizeComment > 0)
	{
		auto buffer = new char[sizeComment];
		int flag = S64GetFileComment(m_nFid, nInd, buffer, 0);
		comment = CString(buffer);
		delete[] buffer;
	}
	return comment;
}

long CDataFileFromCEDSpike2::ReadAdcData(long l_First, long nbPointsAllChannels, short* pBuffer, CWaveChanArray* pArray)
{
	int scan_count = pArray->ChanArray_getSize();
	long long llDataNValues = nbPointsAllChannels / scan_count / sizeof(short);
	int nValuesRead = -1;
	long long tFirst{};
	long long ll_First = l_First + m_llFileOffset;

	for (int ichan = 0; ichan < scan_count; ichan++)
	{
		CWaveChan* pChan = pArray->Get_p_channel(ichan);
		// TODO: create channel buffer
		size_t numberBytes = static_cast<int>(llDataNValues) * sizeof(short);
		memset(pBuffer, 0, numberBytes);
		nValuesRead = read_ChannelData(pChan, pBuffer, ll_First, llDataNValues);
	}
	// TODO: combine channels buffers to build interleaved data 
	return nValuesRead;
}

long CDataFileFromCEDSpike2::read_ChannelData(CWaveChan* pChan, short* pData, long long ll_First, long long llNValues)
{
	const int chanID = pChan->am_CEDchanID;
	const long long ticksPerSample = S64ChanDivide(m_nFid, chanID);
	const long long tUpTo = (ll_First + llNValues - 1) * ticksPerSample;
	const int nMask = 0;
	int numberOfValuesRead = 0;

	while (numberOfValuesRead < llNValues)
	{
		int nMax = static_cast<int>(llNValues) - numberOfValuesRead;
		long long tFrom = (ll_First + numberOfValuesRead) * ticksPerSample;
		if (tFrom >= tUpTo)
			break;

		short* pBuffer = pData + numberOfValuesRead;
		long long tFirst{};
		int nValuesRead = S64ReadWaveS(m_nFid, chanID, pBuffer, nMax, tFrom, tUpTo, &tFirst, nMask);

		//CString message;
		//message.Format(_T("nValuesRead= %i tFrom= %i tUpTo= %i  tFirst= %i\n"), 
		//	nValuesRead, (int) (tFrom/ticksPerSample), 
		//	(int) (tUpTo / ticksPerSample), 
		//	(int) (tFirst / ticksPerSample));
		//ATLTRACE2(message);

		if (nValuesRead <= 0)
			break;

		numberOfValuesRead += nValuesRead;
		if (tFirst > tFrom)
		{
			if (m_bRelocate_if_StartWithGap)
				relocate_ChannelData(pBuffer, tFrom, tFirst, nValuesRead, ticksPerSample);
			numberOfValuesRead += static_cast<long>(tFirst / ticksPerSample);
		}
	}
	return numberOfValuesRead;
}

long CDataFileFromCEDSpike2::relocate_ChannelData(short* pBuffer, long long tFrom, long long tFirst, int nValuesRead,
                                                  long long ticksPerSample)
{
	long offset = static_cast<long>((tFirst - tFrom) / ticksPerSample);
	size_t count = nValuesRead * sizeof(short);
	memmove(pBuffer + offset, pBuffer, count);

	count = (offset - 1) * sizeof(short);
	memset(pBuffer, 0, count);
	return offset;
}

CString CDataFileFromCEDSpike2::getErrorMessage(int flag)
{
	int nItems = sizeof(errorMessages) / sizeof(errorMessages[0]);
	ASSERT(nItems == 20);
	CString errorMsg = _T("error not found");
	for (int i = 0; i < nItems; i++)
	{
		if (flag == errorMessages[i].value)
		{
			errorMsg = errorMessages[i].csText;
			break;
		}
	}
	return errorMsg;
}

void CDataFileFromCEDSpike2::read_EventFall(int cedChan, CWaveBuf* pBuf)
{
	CTagList* pTags = pBuf->GetpVTtags();
	pTags->RemoveAllTags();
	int nRead = 0;
	long long Data = -1;
	do
	{
		nRead = S64ReadEvents(m_nFid, cedChan, &Data, 1, Data + 1, -1, 0);
		if (nRead > 0)
			pTags->AddTag(CTag(Data));
	}
	while (nRead > 0);
}

void CDataFileFromCEDSpike2::convert_VTtags_Ticks_to_ADintervals(CWaveBuf* pBuf, int cedChan)
{
	CTagList* pTags = pBuf->GetpVTtags();
	int ntags = pTags->GetNTags();
	if (ntags > 0)
	{
		const long long ticksPerSample = S64ChanDivide(m_nFid, cedChan);
		for (int i = 0; i < ntags; i++)
		{
			CTag* ptag = pTags->GetTag(i);
			ptag->m_lvalue = static_cast<long>(ptag->m_lTicks / ticksPerSample);
			ptag->m_refchan = cedChan;
		}
	}
}
