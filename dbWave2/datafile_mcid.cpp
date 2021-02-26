// CDataFileMCID.cpp : implementation file
//

#include "StdAfx.h"
#include "dataheader_Atlab.H"
#include "datafile_mcid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CDataFileMCID, CDataFileX)

CDataFileMCID::CDataFileMCID()
{
	m_bHeaderSize = sizeof(MCID_HEADER);
	m_ulOffsetData = m_bHeaderSize;
	m_ulOffsetHeader = 0;
	m_idType = DOCTYPE_MCID;
	m_csType = _T("MCID");
}

CDataFileMCID::~CDataFileMCID()
{
}

#ifdef _DEBUG
void CDataFileMCID::AssertValid() const
{
	CDataFileX::AssertValid();
}

void CDataFileMCID::Dump(CDumpContext& dc) const
{
	CDataFileX::Dump(dc);
}
#endif //_DEBUG

BOOL CDataFileMCID::ReadDataInfos(CWaveBuf* pBuf)
{
	CWaveFormat* pWFormat = pBuf->GetpWaveFormat();
	CWaveChanArray* pArray = pBuf->GetpWavechanArray();
	MCID_HEADER mcidHeader;
	Seek(0, CFile::begin);
	Read(&mcidHeader, sizeof(MCID_HEADER));

	m_bHeaderSize = sizeof(MCID_HEADER) + mcidHeader.application_header;
	m_ulOffsetData = m_bHeaderSize;

	pWFormat->scan_count = mcidHeader.number_of_channels;		// 4   number of channels in scan list

	for (UINT i = 0; i < mcidHeader.number_of_channels; i++)
	{
		pArray->ChanArray_add();
		CWaveChan* pChan = (CWaveChan*)pArray->Get_p_channel(i);
		pChan->am_csComment = CString(" ");
		pChan->am_resolutionV = mcidHeader.sensitivity[i] / 2000.;
		pChan->am_gainamplifier = 1. / pChan->am_resolutionV;

		pChan->am_adchannel = 0;					// channel scan list
		pChan->am_gainAD = 1;						// channel gain list
		pChan->am_csamplifier = CString(" ");		// amplifier type
		pChan->am_csheadstage = CString(" ");		// headstage type
		pChan->am_gainheadstage = 1;				// assume headstage gain = 1
		pChan->am_amplifierchan = 0;				// assume 1 channel / amplifier
		pChan->am_gainpre = 1;						// assume gain -pre and -post = 1
		pChan->am_gainpost = 1;
		pChan->am_notchfilt = 0;					// assume no notch filter
		pChan->am_lowpass = 0;						// assume not low pass filtering
		pChan->am_offset = 0.0f;					// assume no offset compensation
		pChan->am_csInputpos = "DC";				// assume input + = DC
		pChan->am_csInputneg = "GND";				// assume input - = GND
		pChan->am_gaintotal = pChan->am_gainamplifier;
	}

	//	tentative
	pWFormat->fullscale_Volts = 5.0f;				// 10 V full scale
	pWFormat->binspan = 65536;						// 16 bits resolution
	pWFormat->binzero = 0;

	pWFormat->mode_encoding = OLx_ENC_BINARY;
	pWFormat->mode_clock = INTERNAL_CLOCK;
	pWFormat->mode_trigger = INTERNAL_TRIGGER;
	pWFormat->chrate = (float)(1. / mcidHeader.sample_interval);
	pWFormat->sample_count = mcidHeader.number_of_samples;

	pWFormat->cs_comment.Empty();
	pWFormat->csStimulus.Empty();
	pWFormat->csConcentration.Empty();
	pWFormat->csSensillum.Empty();

	return TRUE;
}

int CDataFileMCID::CheckFileType(CString& cs_filename)
{
	BOOL flag = DOCTYPE_UNKNOWN;
	MCID_HEADER mcidHeader;
	Seek(0, CFile::begin);
	Read(&mcidHeader, sizeof(MCID_HEADER));
	m_bHeaderSize = mcidHeader.application_header;
	m_ulOffsetData = mcidHeader.application_header;

	// test "mcid"
	/*if (tab[8]==166 && tab[9]==166 && tab[10]==166 && tab[11]==166)
		flag = m_idType;*/
	return flag;
}