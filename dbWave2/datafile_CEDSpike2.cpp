#include "stdafx.h"
#include "datafile_CEDSpike2.h"
#include "dataheader_Atlab.h"
#include "ceds64int.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CDataFileFromCEDSpike2, CDataFileX)
const int	LENCEDSON	= 16;
const char	CEDSON64[] = "CED_DATA";


CDataFileFromCEDSpike2::CDataFileFromCEDSpike2()
{
	m_bHeaderSize		= 512;
	m_ulOffsetData		= m_bHeaderSize;
	m_ulOffsetHeader	= 0;
	m_idType			= DOCTYPE_SMR;
	m_csType			= CEDSON64;
	m_csFiledesc		= "SMRFILE";
}

CDataFileFromCEDSpike2::~CDataFileFromCEDSpike2()
{
	closeDataFile();
	ATLTRACE2("closed Data File\n");
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

bool CDataFileFromCEDSpike2::isOpened(CString& sz_path_name)
{
	return (m_nFid > 0);
}

bool CDataFileFromCEDSpike2::openDataFile(CString& sz_path_name, UINT u_open_flag)
{
	m_nFid = S64Open(CT2A(sz_path_name), -1);
	if (m_nFid <= 0) {
		AfxMessageBox(AFX_IDP_FAILED_TO_OPEN_DOC);
		return false;
	}
	return true;
}

void CDataFileFromCEDSpike2::closeDataFile()
{
	if (m_nFid >= 0) {
		int flag = S64Close(m_nFid);
		if (flag < 0) {
			CString errorMessage;
			errorMessage.Format(_T("Failed to close data file\nError code=%i"), flag);
			AfxMessageBox(errorMessage, MB_OK);
		}
		m_nFid = -1;
	}	
}

int CDataFileFromCEDSpike2::CheckFileType(CString& cs_fileName)
{
	cs_fileName.MakeLower();
	if (cs_fileName.Find(_T("smr")) != -1) {
		return DOCTYPE_SMR;
	}
	return DOCTYPE_UNKNOWN;
}

BOOL CDataFileFromCEDSpike2::ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray)
{
	// Read file header
	auto bflag = TRUE;
	m_pWFormat = pWFormat;
	m_pArray = pArray;

	//	tentative
	pWFormat->fullscale_Volts	= 5.0f;			// 10 V full scale
	pWFormat->binspan			= 65536;		// 16 bits resolution
	pWFormat->binzero			= 0;
	pWFormat->mode_encoding		= OLx_ENC_BINARY;
	pWFormat->mode_clock		= INTERNAL_CLOCK;
	pWFormat->mode_trigger		= INTERNAL_TRIGGER;
	pWFormat->csADcardName		= "CED1401";

	// get comments
	pWFormat->cs_comment.Empty();
	pWFormat->csStimulus.Empty();
	pWFormat->csConcentration.Empty();
	pWFormat->csSensillum.Empty();

	// get global data and n channels
	double timeBase		= S64GetTimeBase(m_nFid);
	int lowestFreeChan	= S64GetFreeChan(m_nFid);
	int maxChan			= S64MaxChans(m_nFid);
	
	TTimeDate arrayGetTimeDate{};
	int flag = S64TimeDate(m_nFid, (long long*)&arrayGetTimeDate, nullptr, -1);
	pWFormat->acqtime = CTime(
		arrayGetTimeDate.wYear, arrayGetTimeDate.ucMon, arrayGetTimeDate.ucDay, 
		arrayGetTimeDate.ucHour, arrayGetTimeDate.ucMin, arrayGetTimeDate.ucSec);

	pWFormat->scan_count = 0;
	pArray->chanArray_removeAll();
	if (lowestFreeChan > 0) {
		for (int nChan = 0; nChan < lowestFreeChan; nChan++) {
			int chanType = S64ChanType(m_nFid, nChan);
			ATLTRACE2(" chan %i comment= %s ...\n", nChan, getChannelComment(nChan));
			switch (chanType) {
			case CHANTYPE_Adc:
			{
				ATLTRACE2("%2d Adc channel\n", nChan);
				CWaveChan* pChan = getAdcChannel(nChan, pArray);
				pWFormat->scan_count++;
				pWFormat->chrate = (float) (1.0 / (pChan->am_CEDticksPerSample * S64GetTimeBase(m_nFid)));
				pWFormat->sample_count = (long) (pChan->am_CEDmaxTimeInTicks / pChan->am_CEDticksPerSample);
			}
				break;
			case CHANTYPE_EventFall:	
				ATLTRACE2("%2d Event on falling edge\n", nChan); break;
			case CHANTYPE_EventRise:	
				ATLTRACE2("%2d Event on rising edge\n", nChan); break;
			case CHANTYPE_EventBoth:	
				ATLTRACE2("%2d Event on both edges\n", nChan); break;
			case CHANTYPE_Marker:		
				ATLTRACE2("%2d Marker data\n", nChan); break;
			case CHANTYPE_WaveMark:		
				ATLTRACE2("%2d WaveMark data\n", nChan); break;
			case CHANTYPE_RealMark:		
				ATLTRACE2("%2d RealMark data\n", nChan); break;
			case CHANTYPE_TextMark:		
				ATLTRACE2("%2d TextMark data\n", nChan); break;
			case CHANTYPE_RealWave:		
				ATLTRACE2("%2d RealWave data\n", nChan); break;
			case CHANTYPE_unused:
			default:
				ATLTRACE2("%2d unused channel (type = %i)\n", nChan, chanType);
				break;
			}
		}
		//pWFormat->chrate = (float)(1. / cfsHeader.sample_interval);
		//pWFormat->sample_count = cfsHeader.number_of_samples;
	}
	return TRUE;
}

CWaveChan* CDataFileFromCEDSpike2::getAdcChannel(int nChan, CWaveChanArray* pArray) {
	int i = pArray->chanArray_add();
	CWaveChan* pChan = (CWaveChan*)pArray->get_p_channel(i);
	pChan->am_CEDchanID = nChan;
	pChan->am_csamplifier.Empty();			// amplifier type
	pChan->am_csheadstage.Empty();			// headstage type
	pChan->am_csComment.Empty();			// channel comment
	pChan->am_csComment = getChannelComment(nChan);
	
	pChan->am_adchannel		= 0;			// channel scan list
	pChan->am_gainAD		= 1;			// channel gain list
	pChan->am_gainheadstage	= 1;			// assume headstage gain = 1
	pChan->am_amplifierchan	= 0;			// assume 1 channel / amplifier
	pChan->am_gainpre		= 1;			// assume gain -pre = 1
	pChan->am_gainpost		= 1;			// assume gain -post = 1
	pChan->am_notchfilt		= 0;			// assume no notch filter
	pChan->am_lowpass		= 0;			// assume not low pass filtering
	pChan->am_offset		= 0.0f;			// assume no offset compensation
	pChan->am_csInputpos	= "DC";			// assume input + = DC
	pChan->am_csInputneg	= "GND";		// assume input - = GND

	int flag = S64GetChanOffset(m_nFid, nChan, &pChan->am_CEDoffset);
	flag = S64GetChanScale(m_nFid, nChan, &pChan->am_CEDscale);

	pChan->am_gainamplifier = pChan->am_CEDscale *10.;
	pChan->am_resolutionV	= 2.5 / pChan->am_gainamplifier / 65536;
	pChan->am_gaintotal		= pChan->am_gainamplifier;

	pChan->am_CEDticksPerSample = S64ChanDivide(m_nFid, nChan);
	pChan->am_CEDmaxTimeInTicks = S64ChanMaxTime(m_nFid, nChan);

	return pChan;
}

CString  CDataFileFromCEDSpike2::getChannelComment(int nChan) {
	int sizeComment = S64GetChanComment(m_nFid, nChan, nullptr, -1);
	CString comment;
	if (sizeComment > 0) {
		char* buffer = new char[sizeComment];
		int flag = S64GetChanComment(m_nFid, nChan, buffer, 0);
		comment = CString(buffer);
		delete[] buffer;
	}
	return comment;
}

CString  CDataFileFromCEDSpike2::getFileComment(int nInd) {
	int sizeComment = S64GetFileComment(m_nFid, nInd, nullptr, -1);
	CString comment;
	if (sizeComment > 0) {
		char* buffer = new char[sizeComment];
		int flag = S64GetFileComment(m_nFid, nInd, buffer, 0);
		comment = CString(buffer);
		delete[] buffer;
	}
	return comment;
}

long CDataFileFromCEDSpike2::ReadAdcData(long dataIndex, long nbPointsAllChannels, short* pBuffer, CWaveChanArray* pArray)
{
	int scan_count = pArray->chanArray_getSize();
	long long llDataNValues = nbPointsAllChannels/ sizeof(short) / scan_count;
	int nValuesRead = -1;
	long long tFirst{};
	long long llDataIndex = dataIndex;

	for (int ichan = 0; ichan < scan_count; ichan++) {
		CWaveChan* pChan = pArray->get_p_channel(ichan);
		short* pData = pBuffer;
		nValuesRead = ReadOneChanAdcData(pChan, pData, llDataIndex, llDataNValues);
	}
	return nValuesRead;
}

long CDataFileFromCEDSpike2::ReadOneChanAdcData(CWaveChan* pChan, short* pData, long long llDataIndex, long long llDataNValues) {

	const int nMask = 0;
	int chanID = pChan->am_CEDchanID;
	long long ticksPerSample = S64ChanDivide(m_nFid, chanID);
	long long tFrom = llDataIndex * ticksPerSample;
	long long tUpto = (llDataIndex + llDataNValues) * ticksPerSample;
	long long tFirst{};
	const int nValuesMax = (int) llDataNValues;
	int nValuesRead = S64ReadWaveS(m_nFid, chanID, pData, nValuesMax, tFrom, tUpto, &tFirst, nMask);

	CStringA message;
	message.Format("ReadDataBlock l_first= %i nPoints = %i nValuesRead = %i \n", (int) llDataIndex, (int) llDataNValues, nValuesRead);
	ATLTRACE2(message);

	if (nValuesRead > 0) { 
		int nValuesActuallyRead = nValuesRead;
		if (tFirst > tFrom) {
			long iFirst = (long)(tFirst / ticksPerSample);
			long offset = (long) (iFirst - llDataIndex);
			
			size_t number = (long)(nValuesRead * sizeof(short));
			memmove(pData + offset, pData, number);
			size_t number2 = (offset - 1) * sizeof(short);
			memset(pData, 0, number2);

			message.Format("memset [0, 1]= %i, %i memmove [offset]= %i : %i \n", *pData, *(pData+1), offset, *(pData+offset));
			ATLTRACE2(message);
			nValuesActuallyRead += offset;
		}

		// read another chunk if reading was interrupted by a gap
		if (nValuesActuallyRead < llDataNValues) {
			int new_llDataNValues = (int) ( llDataNValues - nValuesActuallyRead);
			long long new_llDataIndex = llDataIndex + nValuesActuallyRead;
			short* new_pData = pData + nValuesRead;
			ReadOneChanAdcData(pChan, new_pData, new_llDataIndex, new_llDataNValues);
		}
	}
	return nValuesRead;
}




