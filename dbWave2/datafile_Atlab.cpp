#include "StdAfx.h"
#include "dataheader_Atlab.H"
#include "datafile_Atlab.h"
#include <iomanip>


IMPLEMENT_DYNCREATE(CDataFileATLAB, CDataFileX)

CDataFileATLAB::CDataFileATLAB()
{
	m_b_header_size = 1024;
	m_ul_offset_data = m_b_header_size;
	m_ul_offset_header = 0;
	m_id_type = DOCTYPE_ATLAB;
	m_cs_type = _T("ATLAB");
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

BOOL CDataFileATLAB::read_data_infos(CWaveBuf* p_buf)
{
	CWaveFormat* p_WaveFormat = p_buf->get_p_wave_format();
	CWaveChanArray* pWavechanArray = p_buf->get_p_wave_chan_array();
	// Read file header
	auto b_flag = TRUE;
	m_p_w_format_ = p_WaveFormat;
	m_p_array_ = pWavechanArray;

	const auto p_header = new char[m_b_header_size];
	ASSERT(p_header != NULL);
	Seek(m_ul_offset_header, begin);
	Read(p_header, m_b_header_size);

	// get A/D card type
	auto p_char = p_header + DEVID;
	const auto device_id = *reinterpret_cast<short*>(p_char);
	switch (device_id)
	{
	case 1: p_WaveFormat->cs_ad_card_name = _T("DT2828");
		break;
	case 2: p_WaveFormat->cs_ad_card_name = _T("DT2827");
		break;
	case 4: p_WaveFormat->cs_ad_card_name = _T("DT2821_F_DI");
		break;
	case 8: p_WaveFormat->cs_ad_card_name = _T("DT2821_F_SE");
		break;
	case 16: p_WaveFormat->cs_ad_card_name = _T("DT2821");
		break;
	case 32: p_WaveFormat->cs_ad_card_name = _T("DT2821_G_DI");
		break;
	case 64: p_WaveFormat->cs_ad_card_name = _T("DT2821_G_SE");
		break;
	case 128: p_WaveFormat->cs_ad_card_name = _T("DT2823");
		break;
	case 256: p_WaveFormat->cs_ad_card_name = _T("DT2825");
		break;
	case 512: p_WaveFormat->cs_ad_card_name = _T("DT2824");
		break;
	default: p_WaveFormat->cs_ad_card_name = _T("DT_OTHER");
		break;
	}

	// A/D acquisition mode -- not implemented
	//unsigned short dev_flags = (short) *(pHeader+DEV_FLAGS);

	// number of data acquisition channels
	p_char = p_header + SCNCNT;
	p_WaveFormat->scan_count = *reinterpret_cast<short*>(p_char);

	// check if file is not corrupted
	p_char = p_header + SAMCNT;
	auto p_long = reinterpret_cast<long*>(p_char);
	const auto len1 = static_cast<long>(GetLength() - m_b_header_size) / 2;
	if (len1 != *p_long)
	{
		SeekToBegin();
		Write(p_header, m_b_header_size);
		b_flag = 2;
	}
	p_WaveFormat->sample_count = len1;

	// ---------------- specific DT2821 differential mode
	p_WaveFormat->full_scale_volts = 20.f; // 20 mv full scale (+10 V to -10 V)
	p_WaveFormat->bin_span = 4096; // 12 bits resolution
	p_WaveFormat->bin_zero = 2048; // offset binary encoding
	p_WaveFormat->w_version = 1; // initial version

	// get device flags & encoding
	//DEV_FLAGS dev_flags = (DEV_FLAGS)((short) *(pHeader+DEV_FLAGS));

	// load data acquisition comment
	auto p_comment = p_header + ACQCOM;
	for (int i = 0; i < ACQCOM_LEN; i++)
		if (*(p_comment + i) == 0)
			*(p_comment + i) = ' ';
	p_comment = p_header + ACQCOM; // restore pointer
	const auto atl_comment = CStringA(p_comment, ACQCOM_LEN); // load comment into string
	p_WaveFormat->cs_comment = atl_comment;
	p_WaveFormat->cs_comment.TrimRight();

	// assume that comment is standardized and has the following fields
	p_WaveFormat->cs_stimulus = atl_comment.Mid(0, 20);
	p_WaveFormat->cs_stimulus.TrimRight();
	p_WaveFormat->cs_concentration = atl_comment.Mid(20, 10);
	p_WaveFormat->cs_concentration.TrimRight();
	p_WaveFormat->cs_sensillum = atl_comment.Mid(30, 10);
	p_WaveFormat->cs_sensillum.TrimRight();
	p_WaveFormat->cs_comment.Empty();

	const auto p_first = p_header + ACQDATE;
	const auto str_date_time = CString(p_first, 19);
	COleDateTime my_dt_time{};
	CTime sys_time {};
	if (my_dt_time.ParseDateTime(str_date_time))
	{
		SYSTEMTIME st;
		if (my_dt_time.GetAsSystemTime(st))
		{
			sys_time = st;
		}
	}
	else
	{
		ASSERT(FALSE);
		sys_time = CTime::GetCurrentTime();
	}
	p_WaveFormat->acquisition_time = sys_time;

	// clock period, sampling rate/chan and file duration
	p_char = p_header + CLKPER;
	p_long = reinterpret_cast<long*>(p_char);
	const auto clock_rate = 4.0E6f / static_cast<float>(*p_long);

	p_WaveFormat->sampling_rate_per_channel = clock_rate / static_cast<float>(p_WaveFormat->scan_count);
	p_WaveFormat->duration = static_cast<float>(p_WaveFormat->sample_count) / clock_rate;

	// fill channel description with minimal infos
	p_char = p_header + CHANLST;
	auto p_channels_list = reinterpret_cast<short*>(p_char);
	p_char = p_header + GAINLST;
	auto p_ad_gain_list = reinterpret_cast<short*>(p_char);
	const auto p_comments_list = p_header + CHANCOM;
	p_char = p_header + XGAIN;
	const auto* p_x_gain_list = reinterpret_cast<float*>(p_char);

	for (int i = 0; i < p_WaveFormat->scan_count; i++, p_channels_list++, p_ad_gain_list++, p_x_gain_list++)
	{
		pWavechanArray->chan_array_add();
		auto* p_chan = pWavechanArray->get_p_channel(i);
		p_chan->am_adchannel = *p_channels_list;
		p_chan->am_gainAD = *p_ad_gain_list;

		auto j = static_cast<short>(strlen(p_comments_list));
		if (j > 40 || j < 0)
			j = 40;
		p_chan->am_csComment = CStringA(p_comments_list, j); // chan comment
		p_chan->am_amplifiergain = *p_x_gain_list; // total gain (amplifier + A/D card)
		p_chan->am_gaintotal = p_chan->am_amplifiergain * static_cast<double>(p_chan->am_gainAD);
		// TODO: check if resolution is computed correctly
		p_chan->am_resolutionV = static_cast<double>(p_WaveFormat->full_scale_volts) / p_chan->am_gaintotal /
			p_WaveFormat->bin_span;
	}

	// init as if no amplifier were present
	for (int i = 0; i < p_WaveFormat->scan_count; i++)
		init_dummy_channels_info(i);

	// 2) version dependent parameters
	p_char = p_header + VERSION;
	const auto version = *reinterpret_cast<short*>(p_char);
	p_char = p_header + CYBERA320;
	const auto* p_cyber_a320 = reinterpret_cast<CYBERAMP*>(p_char);
	switch (version)
	{
		short* p_short;
	case 0: // version 0: transfer parameters from x_gain && cyber to channel_instrument

		if (p_cyber_a320->ComSpeed == 0) // cyberAmp??
			break;
		init_channels_from_cyber_a320(p_header);
		p_WaveFormat->trig_mode = 0; //OLx_TRG_SOFT;
		p_char = p_header + TIMING;
		p_short = reinterpret_cast<short*>(p_char);
		if (*p_short & EXTERNAL_TRIGGER)
			p_WaveFormat->trig_mode = 1; //OLx_TRG_EXTERN;
		b_flag = 2;
		break;

	case 1: // version 1: trig parameters explicitly set
		if (p_cyber_a320->ComSpeed == 0) // cyberAmp??
			break;
		init_channels_from_cyber_a320(p_header);
		p_char = p_header + TRIGGER_MODE;
		p_short = reinterpret_cast<short*>(p_char);
		p_WaveFormat->trig_mode = *p_short;
		p_char = p_header + TRIGGER_CHAN;
		p_short = reinterpret_cast<short*>(p_char);
		p_WaveFormat->trig_chan = *p_short;
		p_char = p_header + TRIGGER_THRESHOLD;
		p_short = reinterpret_cast<short*>(p_char);
		p_WaveFormat->trig_threshold = *p_short;
		break;

	default: // version before version 0
		p_WaveFormat->trig_mode = 0; //OLx_TRG_SOFT;
		p_char = p_header + TIMING;
		p_short = reinterpret_cast<short*>(p_char);
		if (*p_short & EXTERNAL_TRIGGER)
			p_WaveFormat->trig_mode = 1; //OLx_TRG_EXTERN;
		break;
	}


	delete[] p_header;
	return b_flag;
}

void CDataFileATLAB::init_channels_from_cyber_a320(char* p_header) const
{
	char* p_char = p_header + CYBER_1;
	auto* p_cyber1 = reinterpret_cast<CYBERCHAN*>(p_char);
	p_char = p_header + CYBER_2;

	// ATLAB(fred) version 0 did not set chan_cyber properly...
	// if cyber_channel x equal on both channels, assume that parameters are not correctly set
	// look for the first x_ain that equals the Cyber gain
	const auto p_cyber2 = reinterpret_cast<CYBERCHAN*>(p_char);
	if ((p_cyber1->acqchan == p_cyber2->acqchan)
		&& m_p_w_format_->scan_count > 1)
	{
		AfxMessageBox(_T("cyber channel not correctly set"), MB_OK);
	}

	// default cyber_amp description
	else
	{
		if (p_cyber1->acqchan <= m_p_w_format_->scan_count)
		{
			int chan = p_cyber1->acqchan - 1;
			if (chan < 0)
				chan = 0;
			load_channel_from_cyber(chan, reinterpret_cast<char*>(p_cyber1));
			if (m_p_w_format_->scan_count == static_cast<short>(1))
				p_cyber2->acqchan = static_cast<unsigned char>(255);
		}
		if (p_cyber2->acqchan <= m_p_w_format_->scan_count)
		{
			short chan = static_cast<short>(p_cyber2->acqchan - 1);
			if (chan < 0)
				chan = 0;
			load_channel_from_cyber(chan, reinterpret_cast<char*>(p_cyber2));
		}
	}
}

void CDataFileATLAB::load_channel_from_cyber(const int channel, char* p_cyber_chan) const
{
	const auto p_cyb = reinterpret_cast<CYBERCHAN*>(p_cyber_chan);
	// special case if probe == "none"	: exit
	const auto probe = CStringA(&(p_cyb->probe[0]), 8);
	if (probe.CompareNoCase("none    ") == 0
		|| (probe.CompareNoCase("0       ") == 0))
		return;
	const auto p_chan = m_p_array_->get_p_channel(channel);
	p_chan->am_csheadstage = probe;
	p_chan->am_gainheadstage = p_cyb->gainprobe;
	p_chan->am_csamplifier = CString(_T("CyberAmp"));
	p_chan->am_amplifierchan = p_cyb->acqchan;
	p_chan->am_gainpre = p_cyb->gainpre;
	p_chan->am_gainpost = p_cyb->gainpost;
	p_chan->am_notchfilt = p_cyb->notchfilt;
	p_chan->am_lowpass = p_cyb->lowpass;
	p_chan->am_offset = p_cyb->offset;
	p_chan->am_csInputpos = get_cyber_a320_filter(p_cyb->inputpos);
	p_chan->am_csInputneg = get_cyber_a320_filter(p_cyb->inputneg);
	p_chan->am_amplifiergain = 1.;
	p_chan->am_gaintotal = static_cast<double>(p_cyb->gainpre)
		* static_cast<double>(p_cyb->gainpost)
		* static_cast<double>(p_cyb->gainprobe)
		* static_cast<double>(p_chan->am_gainAD);
}

// assume that code is filter cutoff * 10

CString CDataFileATLAB::get_cyber_a320_filter(const int n_code)
{
	CString cs_coupling;
	switch (n_code)
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

void CDataFileATLAB::init_dummy_channels_info(const int chan_list_index) const
{
	auto* p_chan = m_p_array_->get_p_channel(chan_list_index);
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

int CDataFileATLAB::check_file_type(CString& cs_filename)
{
	WORD w_at_lab; // struct for ATLab file
	auto flag = DOCTYPE_UNKNOWN;
	Seek(m_ul_offset_header, begin); // position pointer to start of file
	Read(&w_at_lab, sizeof(w_at_lab)); // Read data

	// test Atlab
	if (w_at_lab == 0xAAAA) //	//&&( tab[2] == 0x07 || tab[2] == 0x06)
		flag = m_id_type;
	return flag;
}
