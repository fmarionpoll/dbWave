#include "StdAfx.h"
#include "dataheader_Atlab.H"
#include "datafile_Atlab.h"

#include <iomanip>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CDataFileATLAB, CDataFileX)

CDataFileATLAB::CDataFileATLAB() : m_pArray(nullptr)
{
	m_bHeaderSize = 1024;
	m_ulOffsetData = m_bHeaderSize;
	m_ulOffsetHeader = 0;
	m_idType = DOCTYPE_ATLAB;
	m_csType = _T("ATLAB");
	m_pWFormat = nullptr;
}

CDataFileATLAB::~CDataFileATLAB()
{
}

#ifdef _DEBUG
void CDataFileATLAB::AssertValid() const
{
	CDataFileX::AssertValid();
}

void CDataFileATLAB::Dump(CDumpContext& dc) const
{
	CDataFileX::Dump(dc);
}
#endif //_DEBUG

BOOL CDataFileATLAB::ReadDataInfos(CWaveBuf* pBuf)
{
	CWaveFormat* pWFormat = pBuf->GetpWaveFormat();
	CWaveChanArray* pArray = pBuf->GetpWavechanArray();
	// Read file header
	auto bflag = TRUE;
	m_pWFormat = pWFormat;
	m_pArray = pArray;

	auto const p_header = new char[m_bHeaderSize];
	ASSERT(p_header != NULL);
	Seek(m_ulOffsetHeader, CFile::begin);	// position pointer
	Read(p_header, m_bHeaderSize);			// read header

	// get A/D card type
	auto pchar = p_header + DEVID;
	const auto devid = *reinterpret_cast<short*>(pchar);
	switch (devid)
	{
	case 1:		pWFormat->csADcardName = _T("DT2828");		break;
	case 2:		pWFormat->csADcardName = _T("DT2827");		break;
	case 4:		pWFormat->csADcardName = _T("DT2821_F_DI");	break;
	case 8:		pWFormat->csADcardName = _T("DT2821_F_SE");	break;
	case 16:	pWFormat->csADcardName = _T("DT2821");		break;
	case 32:	pWFormat->csADcardName = _T("DT2821_G_DI");	break;
	case 64:	pWFormat->csADcardName = _T("DT2821_G_SE");	break;
	case 128:	pWFormat->csADcardName = _T("DT2823");		break;
	case 256:	pWFormat->csADcardName = _T("DT2825");		break;
	case 512:	pWFormat->csADcardName = _T("DT2824");		break;
	default:	pWFormat->csADcardName = _T("DT_OTHER");	break;
	}

	// A/D acquisition mode -- not implemented
	//unsigned short devflags = (short) *(pHeader+DEVFLAGS);

	// number of data acquisition channels
	pchar = p_header + SCNCNT;
	pWFormat->scan_count = *(short*)pchar;

	// check if file is not corrupted
	pchar = p_header + SAMCNT;
	auto plong = reinterpret_cast<long*>(pchar);
	const auto len1 = static_cast<long>(GetLength() - m_bHeaderSize) / 2;
	if (len1 != *plong)
	{
		*plong = len1;
		SeekToBegin();
		Write(p_header, m_bHeaderSize);
		bflag = 2;
	}
	pWFormat->sample_count = *plong;

	// decode ATLAB header and fill in windows structure
	// ---------------- specific DT2821 differential mode
	pWFormat->fullscale_Volts = 20.f;					// 20 mv full scale (+10 V to -10 V)
	pWFormat->binspan = 4096;							// 12 bits resolution
	pWFormat->binzero = 2048;							// offset binary encoding

	// 1) version independent parameters
	//pWFormat->wversion = 1;							// initial version

	// get device flags & encoding
	//DEV_FLAGS devflags = (DEV_FLAGS)((short) *(pHeader+DEVFLAGS));

	// load data acquisition comment
	auto pcomment = p_header + ACQCOM;					// get pointer to acqcom
	int i;
	for (i = 0; i < ACQCOM_LEN; i++)					// loop over acqcom (80 chars)
		if (*(pcomment + i) == 0)
			*(pcomment + i) = ' ';	// erase zeros
	pcomment = p_header + ACQCOM;						// restore pointer
	auto atl_comment = CStringA(pcomment, ACQCOM_LEN);	// load comment into string
	pWFormat->cs_comment = atl_comment;
	pWFormat->cs_comment.TrimRight();

	// assume that comment is standardized and has the following fields
	pWFormat->csStimulus = atl_comment.Mid(0, 20);
	pWFormat->csStimulus.TrimRight();
	pWFormat->csConcentration = atl_comment.Mid(20, 10);
	pWFormat->csConcentration.TrimRight();
	pWFormat->csSensillum = atl_comment.Mid(30, 10);
	pWFormat->csSensillum.TrimRight();
	pWFormat->cs_comment.Empty();

	const auto pfirst = p_header + ACQDATE;
	const auto str_date_time = CString(pfirst, 19);
	COleDateTime myDtTime{};
	CTime sys_time ={};
	if (myDtTime.ParseDateTime(str_date_time))
	{
		SYSTEMTIME st;
		if (myDtTime.GetAsSystemTime(st))
		{
			sys_time = st;
		}
	}
	else
	{
		ASSERT(FALSE);
		sys_time = CTime::GetCurrentTime();
	}
	pWFormat->acqtime = sys_time;

	// clock period, sampling rate/chan and file duration
	pchar = p_header + CLKPER;
	//plong = (INT32*)pchar;
	auto pplong = reinterpret_cast<INT32*>(pchar);
	const auto clock_rate = 4.0E6f / static_cast<float>(*pplong);
	pWFormat->chrate = clock_rate / pWFormat->scan_count;
	pWFormat->duration = pWFormat->sample_count / clock_rate;

	// fill channel description with minimal infos
	pchar = p_header + CHANLST;
	auto pchanlist = reinterpret_cast<short*>(pchar);
	pchar = p_header + GAINLST;
	auto pgainlist = reinterpret_cast<short*>(pchar);
	const auto pcomtlist = p_header + CHANCOM;
	pchar = p_header + XGAIN;
	auto* pxgainlist = reinterpret_cast<float*>(pchar);
	for (i = 0; i < pWFormat->scan_count; i++)
	{
		pArray->ChanArray_add();
		auto* p_chan = pArray->Get_p_channel(i);
		p_chan->am_adchannel = *pchanlist; pchanlist++;		// acq chan
		p_chan->am_gainAD = *pgainlist; pgainlist++;		// gain on the A/D card
		auto j = static_cast<short>(strlen(pcomtlist));
		if (j > 40 || j < 0)
			j = 40;
		p_chan->am_csComment = CStringA(pcomtlist, j);		// chan comment
		p_chan->am_gainamplifier = *pxgainlist;				// total gain (ampli + A/D card)
		p_chan->am_gaintotal = p_chan->am_gainamplifier * static_cast<float>(p_chan->am_gainAD);
		// TODO: check if resolution is computed correctly
		//pChan->am_resolutionV = 20. / (double) (pWFormat->binspan)  / pChan->am_gaintotal;
		p_chan->am_resolutionV = pWFormat->fullscale_Volts / p_chan->am_gaintotal / pWFormat->binspan;
		pgainlist++;
	}

	// init as if no amplifier were present
	for (i = 0; i < pWFormat->scan_count; i++)
		InitDummyChansInfo(i);

	// 2) version dependent parameters
	pchar = p_header + VERSION;
	const auto version = *reinterpret_cast<short*>(pchar);
	pchar = p_header + CYBERA320;
	auto* pcyber_a320 = reinterpret_cast<CYBERAMP*>(pchar);
	short* pshort;
	switch (version)
	{
	case 0:	// version 0: transfer parms from xgain && cyber to channel_instrm
		//AfxMessageBox("Atlab header version 0", MB_OK);

		if (pcyber_a320->ComSpeed == 0)						// cyberAmp??
			break;
		InitChansFromCyberA320(p_header, version);
		pWFormat->trig_mode = 0;							//OLx_TRG_SOFT;
		pchar = p_header + TIMING;
		pshort = reinterpret_cast<short*>(pchar);
		if (*pshort & EXTERNAL_TRIGGER)
			pWFormat->trig_mode = 1;						//OLx_TRG_EXTERN;
		bflag = 2;
		break;

	case 1:	// version 1: trig parameters explicitely set
		if (pcyber_a320->ComSpeed == 0)						// cyberAmp??
			break;
		InitChansFromCyberA320(p_header, version);
		pchar = p_header + TRIGGER_MODE; pshort = (short*)pchar;
		pWFormat->trig_mode = *pshort;
		pchar = p_header + TRIGGER_CHAN; pshort = (short*)pchar;
		pWFormat->trig_chan = *pshort;
		pchar = p_header + TRIGGER_THRESHOLD; pshort = (short*)pchar;
		pWFormat->trig_threshold = *pshort;
		break;

	default: // version before version 0
		pWFormat->trig_mode = 0;							//OLx_TRG_SOFT;
		pchar = p_header + TIMING;
		pshort = reinterpret_cast<short*>(pchar);
		if (*pshort & EXTERNAL_TRIGGER)
			pWFormat->trig_mode = 1;						//OLx_TRG_EXTERN;
		break;
	}

	// init additional parameters
	delete[] p_header;
	return bflag;
}

void CDataFileATLAB::InitChansFromCyberA320(char* pHeader, short version)
{
	char* pchar = pHeader + CYBER_1;
	auto* pcyber1 = reinterpret_cast<CYBERCHAN*>(pchar);
	pchar = pHeader + CYBER_2;
	auto pcyber2 = reinterpret_cast<CYBERCHAN*>(pchar);

	// ATLAB(fred) version 0 did not set chan_cyber properly...
	// if cyber_chanx equal on both channels, assume that parameters are not correctly set
	// look for the first xgain that equals the Cyber gain
	if ((pcyber1->acqchan == pcyber2->acqchan)
		&& m_pWFormat->scan_count > 1)
	{
		AfxMessageBox(_T("cyber chan not correctly set"), MB_OK);
	}

	// default cyberamp description
	else
	{
		if (0 <= pcyber1->acqchan && pcyber1->acqchan <= m_pWFormat->scan_count)
		{
			short chan = pcyber1->acqchan - 1;
			if (chan < 0) chan = 0;
			LoadChanFromCyber(chan, reinterpret_cast<char*>(pcyber1));
			if (static_cast<short>(1) == m_pWFormat->scan_count)
				pcyber2->acqchan = static_cast<unsigned char>(255);
		}
		if (0 <= pcyber2->acqchan && pcyber2->acqchan <= m_pWFormat->scan_count)
		{
			short chan = pcyber2->acqchan - 1;
			if (chan < 0)
				chan = 0;
			LoadChanFromCyber(chan, (char*)pcyber2);
		}
	}
}

void CDataFileATLAB::LoadChanFromCyber(short i, char* pcyberchan)
{
	auto pcyb = reinterpret_cast<CYBERCHAN*>(pcyberchan);
	// special case if probe == "none"	: exit
	auto probe = CStringA(&(pcyb->probe[0]), 8);
	if ((probe.CompareNoCase("none    ") == 0)
		|| (probe.CompareNoCase("0       ") == 0))
		return;
	auto pChan = m_pArray->Get_p_channel(i);
	pChan->am_csheadstage = probe;
	pChan->am_gainheadstage = pcyb->gainprobe;
	pChan->am_csamplifier = CString(_T("CyberAmp"));
	pChan->am_amplifierchan = pcyb->acqchan;
	pChan->am_gainpre = pcyb->gainpre; 				// pre-filter amplification
	pChan->am_gainpost = pcyb->gainpost;			// post-filter amplification
	pChan->am_notchfilt = pcyb->notchfilt;			// notch filter ON/off
	pChan->am_lowpass = pcyb->lowpass;				// low pass filter 0=DC 4....30000
	pChan->am_offset = pcyb->offset;				// input offset
	pChan->am_csInputpos = GetCyberA320filter(pcyb->inputpos);
	pChan->am_csInputneg = GetCyberA320filter(pcyb->inputneg);
	pChan->am_gainamplifier = 1.;
	pChan->am_gaintotal = static_cast<double>(pcyb->gainpre) * static_cast<double>(pcyb->gainpost) * static_cast<double>(pcyb->gainprobe) * static_cast<double>(pChan->am_gainAD);
}

// assume that code is filter cutoff * 10

CString CDataFileATLAB::GetCyberA320filter(int ncode)
{
	CString cs_coupling;
	switch (ncode)
	{
	case -10:	cs_coupling = _T("GND"); break;
	case 0:		cs_coupling = _T("DC"); break;		// DC
	case 1:		cs_coupling = _T("0.1"); break;		// 0.1 Hz
	case 10:	cs_coupling = _T("1"); break;		// 1 Hz
	case 100:	cs_coupling = _T("10"); break;		// 10 Hz
	case 300:	cs_coupling = _T("30"); break;		// 30 Hz
	case 1000:	cs_coupling = _T("100"); break;		// 100 Hz
	case 3000:	cs_coupling = _T("300"); break;		// 300 Hz
	default:	cs_coupling = _T("undefined"); break;
	}
	return cs_coupling;
}

void CDataFileATLAB::InitDummyChansInfo(short chanlistindex)
{
	auto* p_chan = m_pArray->Get_p_channel(chanlistindex);
	p_chan->am_csamplifier = CStringA("Unknown");
	p_chan->am_csheadstage = p_chan->am_csamplifier;
	p_chan->am_gainheadstage = 1;
	p_chan->am_amplifierchan = 1;
	p_chan->am_gainpre = 1;							// pre-filter amplification
	p_chan->am_gainpost = 1;						// post-filter amplification
	p_chan->am_notchfilt = 0;						// notch filter ON/off
	p_chan->am_lowpass = 0;							// low pass filter 0=DC 4....30000
	p_chan->am_offset = 0.0f;						// input offset
	p_chan->am_csInputpos = CStringA("DC");
	p_chan->am_csInputneg = CStringA("GND");
}

int CDataFileATLAB::CheckFileType(CString& cs_filename)
{
	WORD w_atlab;									// struct for ATLab file
	auto flag = DOCTYPE_UNKNOWN;
	Seek(m_ulOffsetHeader, CFile::begin);			// position pointer to start of file
	Read(&w_atlab, sizeof(w_atlab));				// Read data

	// test Atlab
	if (w_atlab == 0xAAAA) //	//&&( tab[2] == 0x07 || tab[2] == 0x06)
		flag = m_idType;
	return flag;
}