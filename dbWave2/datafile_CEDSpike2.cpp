#include "stdafx.h"
#include "datafile_CEDSpike2.h"
#include "dataheader_Atlab.h"
#include "ceds64int.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CDataFileFromCEDSpike2, CDataFileX)
const int	LENCEDSON	= 16;
const char	CEDSON32[]	= "(C) CED 87";
const char	CEDSON64[]	= "S64";

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
	return false;
}

bool CDataFileFromCEDSpike2::openDataFile(CString& sz_path_name, UINT u_open_flag)
{
	m_nFid = S64Open(CT2A(sz_path_name), u_open_flag);
	if (m_nFid < 0) {
		AfxMessageBox(AFX_IDP_FAILED_TO_OPEN_DOC);
		return false;
	}
	return true;
}

void CDataFileFromCEDSpike2::closeDataFile()
{
	int flag = S64Close(m_nFid);
	if (flag < 0) {

		AfxMessageBox(_T("Failed to close data file"), MB_OK); 
	}
}

int CDataFileFromCEDSpike2::CheckFileType(CFile* f)
{
	char bufRead[LENCEDSON] = { 0 };
	f->Seek(0, CFile::begin);
	f->Read(bufRead, sizeof(bufRead));
	
	int flag = isPatternPresent(bufRead, sizeof(bufRead), CEDSON64, sizeof(CEDSON64));
	if (flag == DOCTYPE_UNKNOWN)
		flag = isPatternPresent(bufRead + 2, sizeof(bufRead)-2, CEDSON32, sizeof(CEDSON32));
	return flag;
}

BOOL CDataFileFromCEDSpike2::ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray)
{
	// Read file header
	auto bflag = TRUE;
	m_pWFormat = pWFormat;
	m_pArray = pArray;

	//	tentative
	pWFormat->fullscale_Volts = 5.0f;			// 10 V full scale
	pWFormat->binspan = 65536;		// 16 bits resolution
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
	for (int nInd = 1; nInd <= 5; nInd++) {
		ATLTRACE2(" comment %i = %s\n", nInd, getFileComment(nInd));
	}

	// get global data and n channels
	double timeBase = S64GetTimeBase(m_nFid);
	int lowestFreeChan = S64GetFreeChan(m_nFid);
	int maxChan = S64MaxChans(m_nFid);
	

	struct TTimeDate            // bit compatible with TSONTimeDate
	{
		uint8_t ucHun;          //!< hundreths of a second, 0-99
		uint8_t ucSec;          //!< seconds, 0-59
		uint8_t ucMin;          //!< minutes, 0-59
		uint8_t ucHour;         //!< hour - 24 hour clock, 0-23
		uint8_t ucDay;          //!< day of month, 1-31
		uint8_t ucMon;          //!< month of year, 1-12
		uint16_t wYear;         //!< year 1980-65535! 0 means unset.

		//! Sets the contents to 0
		void clear() { ucHun = ucSec = ucMin = ucHour = ucDay = ucMon = 0; wYear = 0; }
	};
	TTimeDate arrayGetTimeDate{};
	int flag = S64TimeDate(m_nFid, (long long*)&arrayGetTimeDate, nullptr, -1);
	
	pWFormat->scan_count = 0;
	if (lowestFreeChan > 0) {
		for (int nChan = 0; nChan < lowestFreeChan; nChan++) {
			int chanType = S64ChanType(m_nFid, nChan);
			ATLTRACE2(" chan %i comment= %s ...\n", nChan, getChannelComment(nChan));
			switch (chanType) {
			case CHANTYPE_Adc:
			{
				getAdcChannel(nChan, pArray);
				pWFormat->scan_count++;
				long long ticksPerSample = S64ChanDivide(m_nFid, nChan);
				pWFormat->chrate = (float) (1.0 / (ticksPerSample * S64GetTimeBase(m_nFid)));
				long long maxTimeInTicks = S64ChanMaxTime(m_nFid, nChan);
				pWFormat->sample_count = (long) (maxTimeInTicks / ticksPerSample);
			}
				break;
			case CHANTYPE_EventFall:
			case CHANTYPE_EventRise:
			case CHANTYPE_EventBoth:
			case CHANTYPE_Marker:
			case CHANTYPE_WaveMark:
			case CHANTYPE_RealMark:
			case CHANTYPE_TextMark:
			case CHANTYPE_RealWave:
			case CHANTYPE_unused:
			default:
				break;
			}
		}
		//pWFormat->chrate = (float)(1. / cfsHeader.sample_interval);
		//pWFormat->sample_count = cfsHeader.number_of_samples;
	}
	return TRUE;
}

void  CDataFileFromCEDSpike2::getAdcChannel(int nChan, CWaveChanArray* pArray) {
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

	// TODO get proper values from C64 file
	int flag = S64GetChanOffset(m_nFid, nChan, &pChan->am_CEDoffset);
	flag = S64GetChanScale(m_nFid, nChan, &pChan->am_CEDscale);

	pChan->am_resolutionV	= 1.;
	pChan->am_gainamplifier	= 1. / pChan->am_resolutionV;
	pChan->am_gaintotal		= pChan->am_gainamplifier;

	//pChan->am_resolutionV		= cfsHeader.sensitivity[i] / 2000.;
	//pChan->am_gainamplifier	= 1. / pChan->am_resolutionV;	// fractional gain
	//pChan->am_gaintotal		= pChan->am_gainamplifier;
}

CString  CDataFileFromCEDSpike2::getChannelComment(int nChan) {
	int sizeComment = S64GetChanComment(m_nFid, nChan, nullptr, -1);
	if (sizeComment > 0) {
		char* buffer = new char[sizeComment];
		int flag = S64GetChanComment(m_nFid, nChan, buffer, 0);
		return CString(buffer);
	}
	return nullptr;
}

CString  CDataFileFromCEDSpike2::getFileComment(int nInd) {
	int sizeComment = S64GetFileComment(m_nFid, nInd, nullptr, -1);
	if (sizeComment > 0) {
		char* buffer = new char[sizeComment];
		int flag = S64GetFileComment(m_nFid, nInd, buffer, 0);
		return CString(buffer);
	}
	return nullptr;
}

long CDataFileFromCEDSpike2::ReadAdcData(long dataIndex, long nbpoints, short* pBuffer, CWaveFormat* pWFormat)
{
	// seek and read CFile
	const LONGLONG l_off = (LONGLONG(dataIndex) * sizeof(short)) + m_ulOffsetData;
	Seek(l_off, CFile::begin);
	const long l_size = Read(pBuffer, nbpoints);
	// adjust dependent parameters
	return l_size / sizeof(short);

	int nMax = nbpoints; // TODO: divide by nchans if several data acquisition channels?
	int nChan = 1;
	long long ticksPerSample = S64ChanDivide(m_nFid, nChan);
	long long tFrom = dataIndex * ticksPerSample;

	/*
	MATINT_API int S64ReadWaveS(const int nFid, const int nChan, short* pData, const int nMax,
        const long long tFrom, const long long tUpto, long long* tFirst, const int nMask);
	*/
}

