#include "stdafx.h"
#include "datafile_SMR.h"
#include "ceds64int.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataFileCFS

IMPLEMENT_DYNCREATE(CDataFileSMR, CDataFileX)
const int	LENCEDSON	= 16;
const char	CEDSON32[]	= "(C) CED 87";
const char	CEDSON64[]	= "S64";

CDataFileSMR::CDataFileSMR()
{
	m_bHeaderSize		= 512;
	m_ulOffsetData		= m_bHeaderSize;
	m_ulOffsetHeader	= 0;
	m_idType			= DOCTYPE_SMR;
	m_csType			= CEDSON64;
	m_csFiledesc		= "SMRFILE";
}

CDataFileSMR::~CDataFileSMR()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDataFileCFS diagnostics

#ifdef _DEBUG
void CDataFileSMR::AssertValid() const
{
	CDataFileX::AssertValid();
}

void CDataFileSMR::Dump(CDumpContext& dc) const
{
	CDataFileX::Dump(dc);
}
#endif //_DEBUG

BOOL CDataFileSMR::ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray)
{
	

	//m_bHeaderSize = sizeof(CFS_HEADER) + cfsHeader.application_header;
	//m_ulOffsetData = m_bHeaderSize;

	//pWFormat->scan_count = cfsHeader.number_of_channels;		// 4   number of channels in scan list

	//for (UINT i = 0; i < cfsHeader.number_of_channels; i++)
	//{
	//	pArray->channel_add();
	//	CWaveChan* pChan = (CWaveChan*)pArray->get_p_channel(i);
	//	pChan->am_csComment = CString(" ");			// channel annotation
	//	pChan->am_resolutionV = cfsHeader.sensitivity[i] / 2000.;
	//	pChan->am_gainamplifier = 1. / pChan->am_resolutionV;	// fractional gain

	//	pChan->am_adchannel = 0;					// channel scan list
	//	pChan->am_gainAD = 1;						// channel gain list
	//	pChan->am_csamplifier = CString(" ");		// amplifier type
	//	pChan->am_csheadstage = CString(" ");		// headstage type
	//	pChan->am_gainheadstage = 1;				// assume headstage gain = 1
	//	pChan->am_amplifierchan = 0;				// assume 1 channel / amplifier
	//	pChan->am_gainpre = 1;						// assume gain -pre and -post = 1
	//	pChan->am_gainpost = 1;
	//	pChan->am_notchfilt = 0;					// assume no notch filter
	//	pChan->am_lowpass = 0;						// assume not low pass filtering
	//	pChan->am_offset = 0.0f;					// assume no offset compensation
	//	pChan->am_csInputpos = "DC";				// assume input + = DC
	//	pChan->am_csInputneg = "GND";				// assume input - = GND
	//	pChan->am_gaintotal = pChan->am_gainamplifier;
	//}

	////	tentative
	//pWFormat->fullscale_Volts = 5.0f;				// 10 V full scale
	//pWFormat->binspan = 65536;					// 16 bits resolution
	//pWFormat->binzero = 0;

	//pWFormat->mode_encoding = OLx_ENC_BINARY;
	//pWFormat->mode_clock = INTERNAL_CLOCK;
	//pWFormat->mode_trigger = INTERNAL_TRIGGER;
	//pWFormat->chrate = (float)(1. / cfsHeader.sample_interval);
	//pWFormat->sample_count = cfsHeader.number_of_samples;

	//pWFormat->cs_comment.Empty();
	//pWFormat->csStimulus.Empty();
	//pWFormat->csConcentration.Empty();
	//pWFormat->csSensillum.Empty();

	return TRUE;
}

int CDataFileSMR::CheckFileType(CFile* f)
{
	char bufRead[LENCEDSON] = { 0 };
	f->Seek(0, CFile::begin);
	f->Read(bufRead, sizeof(bufRead));
	
	int flag = isPatternPresent(bufRead, sizeof(bufRead), CEDSON64, sizeof(CEDSON64));
	if (flag == DOCTYPE_UNKNOWN)
		flag = isPatternPresent(bufRead + 2, sizeof(bufRead)-2, CEDSON32, sizeof(CEDSON32));
	return flag;
}

