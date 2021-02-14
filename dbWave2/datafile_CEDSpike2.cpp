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
	
	int flag = isCharacterPatternPresent(bufRead, sizeof(bufRead), CEDSON64, sizeof(CEDSON64));
	if (flag == DOCTYPE_UNKNOWN)
		flag = isCharacterPatternPresent(bufRead + 2, sizeof(bufRead)-2, CEDSON32, sizeof(CEDSON32));
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

	// get comments
	pWFormat->cs_comment.Empty();
	pWFormat->csStimulus.Empty();
	pWFormat->csConcentration.Empty();
	pWFormat->csSensillum.Empty();
	for (int nInd = 1; nInd <= 5; nInd++) {
		int sizeComment = S64GetFileComment(m_nFid, nInd, nullptr, -1);
		if (sizeComment > 0) {
			char* buffer = new char[sizeComment];
			int flag = S64GetFileComment(m_nFid, nInd, buffer, 0);
			TRACE(" comment %i = %s\n", nInd, buffer);
		}
	}

	// get global data and n channels
	double timeBase = S64GetTimeBase(m_nFid);
	int lowestFreeChan = S64GetFreeChan(m_nFid);
	int maxChan = S64MaxChans(m_nFid);
	/*
	long long arrayTimeDate[8]{}; // 0=1/100s(0-99); 1=s(0-59); 2=min(0-59); 3=h(0-23); 4=day(1-31); 5=month (1-12); 6=year(1980-2200)
	int flag = S64TimeDate(m_nFid, &arrayTimeDate[0], nullptr, 1); // TOTO what is iMode? (last parameter)
	*/
	pWFormat->scan_count = 0;
	if (lowestFreeChan > 0) {
		for (int nChan = 0; nChan < lowestFreeChan; nChan++) {
			int chanType = S64ChanType(m_nFid, nChan);

			switch (chanType) {
			case CHANTYPE_Adc:
				addAdcChannelFromCEDFile(nChan, pArray);
				pWFormat->scan_count++;
				pWFormat->chrate = 1.0 / (S64ChanDivide(m_nFid, nChan) * S64GetTimeBase(m_nFid));
				pWFormat->sample_count = S64ChanMaxTime(m_nFid, nChan) * pWFormat->chrate;
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

void  CDataFileFromCEDSpike2::addAdcChannelFromCEDFile(int nChan, CWaveChanArray* pArray) {
	int i = pArray->chanArray_add();
	CWaveChan* pChan = (CWaveChan*)pArray->get_p_channel(i);
	pChan->am_chanID = nChan;
	pChan->am_csamplifier.Empty();			// amplifier type
	pChan->am_csheadstage.Empty();			// headstage type
	pChan->am_csComment.Empty();			// channel comment
	int sizeComment = S64GetChanComment(m_nFid, nChan, nullptr, -1);
	if (sizeComment > 0) {
		char* buffer = new char[sizeComment];
		int flag = S64GetChanComment(m_nFid, nChan, buffer, 0);
		pChan->am_csComment = CString(buffer);
		TRACE(" chan %i comment= %s\n", nChan, buffer);
	}

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
	pChan->am_resolutionV	= 1.;
	pChan->am_gainamplifier	= 1. / pChan->am_resolutionV;
	pChan->am_gaintotal		= pChan->am_gainamplifier;

	//pChan->am_resolutionV		= cfsHeader.sensitivity[i] / 2000.;
	//pChan->am_gainamplifier	= 1. / pChan->am_resolutionV;	// fractional gain
	//pChan->am_gaintotal		= pChan->am_gainamplifier;
}
