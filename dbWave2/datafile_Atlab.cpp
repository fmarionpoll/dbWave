#include "StdAfx.h"
#include "dataheader_Atlab.H"
#include "datafile_Atlab.h"
#include <iomanip>


IMPLEMENT_DYNCREATE(CDataFileATLAB, CDataFileX)

CDataFileATLAB::CDataFileATLAB()
{
	m_bHeaderSize = 1024;
	m_ulOffsetData = m_bHeaderSize;
	m_ulOffsetHeader = 0;
	m_idType = DOCTYPE_ATLAB;
	m_csType = _T("ATLAB");
}

CDataFileATLAB::~CDataFileATLAB()
= default;

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

BOOL CDataFileATLAB::ReadDataInfos(CWaveBuf* p_buf)
{
	CWaveFormat* p_WaveFormat = p_buf->GetpWaveFormat();
	CWaveChanArray* pWavechanArray = p_buf->GetpWavechanArray();
	// Read file header
	auto bflag = TRUE;
	m_pWFormat = p_WaveFormat;
	m_pArray = pWavechanArray;

	const auto p_header = new char[m_bHeaderSize];
	ASSERT(p_header != NULL);
	Seek(m_ulOffsetHeader, begin);
	Read(p_header, m_bHeaderSize);

	// get A/D card type
	auto pchar = p_header + DEVID;
	const auto devid = *reinterpret_cast<short*>(pchar);
	switch (devid)
	{
	case 1: p_WaveFormat->csADcardName = _T("DT2828");
		break;
	case 2: p_WaveFormat->csADcardName = _T("DT2827");
		break;
	case 4: p_WaveFormat->csADcardName = _T("DT2821_F_DI");
		break;
	case 8: p_WaveFormat->csADcardName = _T("DT2821_F_SE");
		break;
	case 16: p_WaveFormat->csADcardName = _T("DT2821");
		break;
	case 32: p_WaveFormat->csADcardName = _T("DT2821_G_DI");
		break;
	case 64: p_WaveFormat->csADcardName = _T("DT2821_G_SE");
		break;
	case 128: p_WaveFormat->csADcardName = _T("DT2823");
		break;
	case 256: p_WaveFormat->csADcardName = _T("DT2825");
		break;
	case 512: p_WaveFormat->csADcardName = _T("DT2824");
		break;
	default: p_WaveFormat->csADcardName = _T("DT_OTHER");
		break;
	}

	// A/D acquisition mode -- not implemented
	//unsigned short devflags = (short) *(pHeader+DEVFLAGS);

	// number of data acquisition channels
	pchar = p_header + SCNCNT;
	p_WaveFormat->scan_count = *reinterpret_cast<short*>(pchar);

	// check if file is not corrupted
	pchar = p_header + SAMCNT;
	auto plong = reinterpret_cast<long*>(pchar);
	const auto len1 = static_cast<long>(GetLength() - m_bHeaderSize) / 2;
	if (len1 != *plong)
	{
		SeekToBegin();
		Write(p_header, m_bHeaderSize);
		bflag = 2;
	}
	p_WaveFormat->sample_count = len1;

	// ---------------- specific DT2821 differential mode
	p_WaveFormat->fullscale_volts = 20.f; // 20 mv full scale (+10 V to -10 V)
	p_WaveFormat->binspan = 4096; // 12 bits resolution
	p_WaveFormat->binzero = 2048; // offset binary encoding
	p_WaveFormat->wversion = 1; // initial version

	// get device flags & encoding
	//DEV_FLAGS devflags = (DEV_FLAGS)((short) *(pHeader+DEVFLAGS));

	// load data acquisition comment
	auto pcomment = p_header + ACQCOM;
	for (int i = 0; i < ACQCOM_LEN; i++)
		if (*(pcomment + i) == 0)
			*(pcomment + i) = ' ';
	pcomment = p_header + ACQCOM; // restore pointer
	const auto atl_comment = CStringA(pcomment, ACQCOM_LEN); // load comment into string
	p_WaveFormat->cs_comment = atl_comment;
	p_WaveFormat->cs_comment.TrimRight();

	// assume that comment is standardized and has the following fields
	p_WaveFormat->csStimulus = atl_comment.Mid(0, 20);
	p_WaveFormat->csStimulus.TrimRight();
	p_WaveFormat->csConcentration = atl_comment.Mid(20, 10);
	p_WaveFormat->csConcentration.TrimRight();
	p_WaveFormat->csSensillum = atl_comment.Mid(30, 10);
	p_WaveFormat->csSensillum.TrimRight();
	p_WaveFormat->cs_comment.Empty();

	const auto pfirst = p_header + ACQDATE;
	const auto str_date_time = CString(pfirst, 19);
	COleDateTime myDtTime{};
	CTime sys_time = {};
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
	p_WaveFormat->acqtime = sys_time;

	// clock period, sampling rate/chan and file duration
	pchar = p_header + CLKPER;
	plong = reinterpret_cast<long*>(pchar);
	const auto clock_rate = 4.0E6f / static_cast<float>(*plong);

	p_WaveFormat->sampling_rate_per_channel = clock_rate / static_cast<float>(p_WaveFormat->scan_count);
	p_WaveFormat->duration = static_cast<float>(p_WaveFormat->sample_count) / clock_rate;

	// fill channel description with minimal infos
	pchar = p_header + CHANLST;
	auto p_channels_list = reinterpret_cast<short*>(pchar);
	pchar = p_header + GAINLST;
	auto p_ADgain_list = reinterpret_cast<short*>(pchar);
	const auto pcomtlist = p_header + CHANCOM;
	pchar = p_header + XGAIN;
	const auto* p_xgain_list = reinterpret_cast<float*>(pchar);

	for (int i = 0; i < p_WaveFormat->scan_count; i++, p_channels_list++, p_ADgain_list++, p_xgain_list++)
	{
		pWavechanArray->ChanArray_add();
		auto* p_chan = pWavechanArray->Get_p_channel(i);
		p_chan->am_adchannel = *p_channels_list;
		p_chan->am_gainAD = *p_ADgain_list;

		auto j = static_cast<short>(strlen(pcomtlist));
		if (j > 40 || j < 0)
			j = 40;
		p_chan->am_csComment = CStringA(pcomtlist, j); // chan comment
		p_chan->am_gainamplifier = *p_xgain_list; // total gain (ampli + A/D card)
		p_chan->am_gaintotal = p_chan->am_gainamplifier * static_cast<double>(p_chan->am_gainAD);
		// TODO: check if resolution is computed correctly
		p_chan->am_resolutionV = static_cast<double>(p_WaveFormat->fullscale_volts) / p_chan->am_gaintotal /
			p_WaveFormat->binspan;
	}

	// init as if no amplifier were present
	for (int i = 0; i < p_WaveFormat->scan_count; i++)
		init_dummy_chans_info(i);

	// 2) version dependent parameters
	pchar = p_header + VERSION;
	const auto version = *reinterpret_cast<short*>(pchar);
	pchar = p_header + CYBERA320;
	const auto* pcyber_a320 = reinterpret_cast<CYBERAMP*>(pchar);
	switch (version)
	{
		short* pshort;
	case 0: // version 0: transfer parms from xgain && cyber to channel_instrm

		if (pcyber_a320->ComSpeed == 0) // cyberAmp??
			break;
		init_channels_from_cyber_a320(p_header);
		p_WaveFormat->trig_mode = 0; //OLx_TRG_SOFT;
		pchar = p_header + TIMING;
		pshort = reinterpret_cast<short*>(pchar);
		if (*pshort & EXTERNAL_TRIGGER)
			p_WaveFormat->trig_mode = 1; //OLx_TRG_EXTERN;
		bflag = 2;
		break;

	case 1: // version 1: trig parameters explicitely set
		if (pcyber_a320->ComSpeed == 0) // cyberAmp??
			break;
		init_channels_from_cyber_a320(p_header);
		pchar = p_header + TRIGGER_MODE;
		pshort = reinterpret_cast<short*>(pchar);
		p_WaveFormat->trig_mode = *pshort;
		pchar = p_header + TRIGGER_CHAN;
		pshort = reinterpret_cast<short*>(pchar);
		p_WaveFormat->trig_chan = *pshort;
		pchar = p_header + TRIGGER_THRESHOLD;
		pshort = reinterpret_cast<short*>(pchar);
		p_WaveFormat->trig_threshold = *pshort;
		break;

	default: // version before version 0
		p_WaveFormat->trig_mode = 0; //OLx_TRG_SOFT;
		pchar = p_header + TIMING;
		pshort = reinterpret_cast<short*>(pchar);
		if (*pshort & EXTERNAL_TRIGGER)
			p_WaveFormat->trig_mode = 1; //OLx_TRG_EXTERN;
		break;
	}


	delete[] p_header;
	return bflag;
}

void CDataFileATLAB::init_channels_from_cyber_a320(char* p_header) const
{
	char* pchar = p_header + CYBER_1;
	auto* pcyber1 = reinterpret_cast<CYBERCHAN*>(pchar);
	pchar = p_header + CYBER_2;

	// ATLAB(fred) version 0 did not set chan_cyber properly...
	// if cyber_chanx equal on both channels, assume that parameters are not correctly set
	// look for the first xgain that equals the Cyber gain
	const auto pcyber2 = reinterpret_cast<CYBERCHAN*>(pchar);
	if ((pcyber1->acqchan == pcyber2->acqchan)
		&& m_pWFormat->scan_count > 1)
	{
		AfxMessageBox(_T("cyber channel not correctly set"), MB_OK);
	}

	// default cyberamp description
	else
	{
		if (pcyber1->acqchan <= m_pWFormat->scan_count)
		{
			int chan = pcyber1->acqchan - 1;
			if (chan < 0)
				chan = 0;
			load_channel_from_cyber(chan, reinterpret_cast<char*>(pcyber1));
			if (m_pWFormat->scan_count == static_cast<short>(1))
				pcyber2->acqchan = static_cast<unsigned char>(255);
		}
		if (pcyber2->acqchan <= m_pWFormat->scan_count)
		{
			short chan = pcyber2->acqchan - 1;
			if (chan < 0)
				chan = 0;
			load_channel_from_cyber(chan, reinterpret_cast<char*>(pcyber2));
		}
	}
}

void CDataFileATLAB::load_channel_from_cyber(const int channel, char* pcyberchan) const
{
	const auto pcyb = reinterpret_cast<CYBERCHAN*>(pcyberchan);
	// special case if probe == "none"	: exit
	const auto probe = CStringA(&(pcyb->probe[0]), 8);
	if (probe.CompareNoCase("none    ") == 0
		|| (probe.CompareNoCase("0       ") == 0))
		return;
	const auto p_chan = m_pArray->Get_p_channel(channel);
	p_chan->am_csheadstage = probe;
	p_chan->am_gainheadstage = pcyb->gainprobe;
	p_chan->am_csamplifier = CString(_T("CyberAmp"));
	p_chan->am_amplifierchan = pcyb->acqchan;
	p_chan->am_gainpre = pcyb->gainpre;
	p_chan->am_gainpost = pcyb->gainpost;
	p_chan->am_notchfilt = pcyb->notchfilt;
	p_chan->am_lowpass = pcyb->lowpass;
	p_chan->am_offset = pcyb->offset;
	p_chan->am_csInputpos = get_cyber_a320_filter(pcyb->inputpos);
	p_chan->am_csInputneg = get_cyber_a320_filter(pcyb->inputneg);
	p_chan->am_gainamplifier = 1.;
	p_chan->am_gaintotal = static_cast<double>(pcyb->gainpre)
		* static_cast<double>(pcyb->gainpost)
		* static_cast<double>(pcyb->gainprobe)
		* static_cast<double>(p_chan->am_gainAD);
}

// assume that code is filter cutoff * 10

CString CDataFileATLAB::get_cyber_a320_filter(const int ncode)
{
	CString cs_coupling;
	switch (ncode)
	{
	case -10: cs_coupling = _T("GND");
		break;
	case 0: cs_coupling = _T("DC");
		break; // DC
	case 1: cs_coupling = _T("0.1");
		break; // 0.1 Hz
	case 10: cs_coupling = _T("1");
		break; // 1 Hz
	case 100: cs_coupling = _T("10");
		break; // 10 Hz
	case 300: cs_coupling = _T("30");
		break; // 30 Hz
	case 1000: cs_coupling = _T("100");
		break; // 100 Hz
	case 3000: cs_coupling = _T("300");
		break; // 300 Hz
	default: cs_coupling = _T("undefined");
		break;
	}
	return cs_coupling;
}

void CDataFileATLAB::init_dummy_chans_info(const int chanlistindex) const
{
	auto* p_chan = m_pArray->Get_p_channel(chanlistindex);
	p_chan->am_csamplifier = CStringA("Unknown");
	p_chan->am_csheadstage = p_chan->am_csamplifier;
	p_chan->am_gainheadstage = 1;
	p_chan->am_amplifierchan = 1;
	p_chan->am_gainpre = 1; // pre-filter amplification
	p_chan->am_gainpost = 1; // post-filter amplification
	p_chan->am_notchfilt = 0; // notch filter ON/off
	p_chan->am_lowpass = 0; // low pass filter 0=DC 4....30000
	p_chan->am_offset = 0.0f; // input offset
	p_chan->am_csInputpos = CStringA("DC");
	p_chan->am_csInputneg = CStringA("GND");
}

int CDataFileATLAB::CheckFileType(CString& cs_filename)
{
	WORD w_atlab; // struct for ATLab file
	auto flag = DOCTYPE_UNKNOWN;
	Seek(m_ulOffsetHeader, begin); // position pointer to start of file
	Read(&w_atlab, sizeof(w_atlab)); // Read data

	// test Atlab
	if (w_atlab == 0xAAAA) //	//&&( tab[2] == 0x07 || tab[2] == 0x06)
		flag = m_idType;
	return flag;
}
