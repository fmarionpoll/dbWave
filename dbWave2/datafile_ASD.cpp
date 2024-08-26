#include "StdAfx.h"
#include "dataheader_Atlab.H"
#include "datafile_ASD.h"

#include <Olxdadefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto DT_WAVE = 0x01;
constexpr auto DT_SPIKE = 0x02;
constexpr auto DT_HISTOGRAM = 0x04;
constexpr auto DT_ASCIITEXT = 0x08;
constexpr auto DT_RICHTEXT = 0x10;

// Macros to swap from Big Endian to Little Endian
#define SWAPWORD(x) MAKEWORD( HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG( SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)) )


IMPLEMENT_DYNCREATE(CDataFileASD, CDataFileX)

CDataFileASD::CDataFileASD()
{
	m_id_type = DOCTYPE_ASDSYNTECH;
	m_cs_type = _T("ASDsyntech");
	m_cs_old_string_id_ = "AutoSpike 97 Data File";
	m_cs_string_id_ = "AutoSpike-32 Data File";
	m_w_id_ = 0xAAAA;
	m_ul_offset_header = 29;
}

CDataFileASD::~CDataFileASD()
{
}

#ifdef _DEBUG
void CDataFileASD::AssertValid() const
{
	CDataFileX::AssertValid();
}

void CDataFileASD::Dump(CDumpContext& dc) const
{
	CDataFileX::Dump(dc);
}
#endif //_DEBUG

BOOL CDataFileASD::read_data_infos(CWaveBuf* p_buf)
{
	CWaveFormat* wave_format = p_buf->get_p_wave_format();
	CWaveChanArray* wavechan_array = p_buf->get_p_wave_chan_array();

	Seek(29, begin); 
	CString cs_name;
	auto ch = ' ';
	while (ch != 0) // (1) signal name
	{
		Read(&ch, sizeof(char));
		if (ch != 0) cs_name += ch;
	}

	DWORD dw;
	WORD w;
	Read(&dw, sizeof(DWORD));
	const DWORD rec_factor = SWAPLONG(dw);
	const auto gain_post = static_cast<short>(rec_factor); // max is 32768
	constexpr auto gain_pre = 1;
	ASSERT(rec_factor <= 32768);

	Read(&dw, sizeof(DWORD));
	const auto sample_rate = static_cast<double>(SWAPLONG(dw)) / 1000.0;

	Read(&dw, sizeof(DWORD));
	const DWORD count = SWAPLONG(dw);

	for (DWORD index = 0; index < count; index++) // (6-7) percent and time
	{
		Read(&w, sizeof(WORD));
		int percent = SWAPWORD(w);
		Read(&dw, sizeof(DWORD));
		auto time = SWAPLONG(dw);
	}

	Read(&dw, sizeof(DWORD));
	const UINT ui_count = SWAPLONG(dw);

	const LONGLONG l_offset1 = static_cast<LONGLONG>(GetPosition()); // start of data area
	m_ul_offset_data = l_offset1 + 1;
	const LONGLONG l_offset2 = static_cast<LONGLONG>(ui_count) * 2; // length of data area (in bytes)
	Seek(l_offset2, current); // position pointer

	Read(&w, sizeof(WORD));
	auto check_sum = SWAPWORD(w);

	Read(&w, sizeof(WORD)); // read type
	auto w_type = SWAPWORD(w);
	ASSERT(w_type == m_w_id_);

	// file subtype
	Read(&w, sizeof(WORD)); // file subtype
	w_type = SWAPWORD(w);
	ASSERT(w_type == DT_ASCIITEXT);

	// read comment
	CString cs_comment; // and now text comment
	do
	{
		Read(&ch, sizeof(char));
		if (ch != 0)
			cs_comment += ch;
	}
	while (ch != 0);

	Seek(l_offset1, begin); // position pointer / beginning data
	wave_format->scan_count = 1; // assume 1 channel

	// ---------------- specifics from Syntech A/D card
	wave_format->full_scale_volts = 10.0f; // 10 mv full scale
	wave_format->bin_span = 32768; // 15 bits resolution
	wave_format->bin_zero = wave_format->bin_span / 2; // ?

	wave_format->mode_encoding = OLx_ENC_BINARY;
	wave_format->mode_clock = INTERNAL_CLOCK;
	wave_format->mode_trigger = INTERNAL_TRIGGER;
	wave_format->sampling_rate_per_channel = static_cast<float>(sample_rate);
	wave_format->sample_count = static_cast<long>(ui_count);

	//for (i = 0; i<wave_format->scan_count; i++)
	//{
	constexpr int i = 0;
	wavechan_array->chan_array_add();
	const auto p_chan = wavechan_array->get_p_channel(i);
	p_chan->am_csComment = CString(" "); // channel annotation
	p_chan->am_adchannel = 0; // channel scan list
	p_chan->am_gainAD = 1; // channel gain list
	p_chan->am_csamplifier = CString("syntechAmplifier"); // amplifier type
	p_chan->am_csheadstage = CString("syntechProbe"); // headstage type
	p_chan->am_gainheadstage = 10; // assume headstage gain = 10
	p_chan->am_amplifierchan = 0; // assume 1 channel / amplifier
	p_chan->am_gainpre = gain_pre; // assume gain -pre and -post set before
	p_chan->am_gainpost = gain_post;
	p_chan->am_notchfilt = 0; // assume no notch filter
	p_chan->am_lowpass = 0; // assume not low pass filtering
	p_chan->am_offset = 0.0f; // assume no offset compensation
	p_chan->am_csInputpos = "25";
	p_chan->am_csInputneg = "GND";
	p_chan->am_amplifiergain = static_cast<float>(rec_factor);

	p_chan->am_gaintotal = p_chan->am_amplifiergain * p_chan->am_gainheadstage; // total gain
	p_chan->am_resolutionV = wave_format->full_scale_volts / p_chan->am_gaintotal / wave_format->bin_span;

	// ---------------- ASD -- capture date and time

	const auto strlen = cs_comment.GetLength();
	auto i_char1 = 12;

	// month
	auto dummy = cs_comment.Mid(i_char1, 3);
	const CString cs_month[] =
	{
		_T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"),
		_T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec")
	};

	int i_month;
	for (i_month = 0; i_month < 12; i_month++)
	{
		if (dummy.CompareNoCase(cs_month[i_month]) == 0)
			break;
	}
	i_month++;

	// day
	i_char1 += 4;
	dummy = cs_comment.Mid(i_char1, 2);
	const auto i_day = _ttoi(dummy);

	// time
	i_char1 += 3;
	const auto i_hour = _ttoi(cs_comment.Mid(i_char1, 2));
	i_char1 += 3;
	const auto i_min = _ttoi(cs_comment.Mid(i_char1, 2));
	i_char1 += 3;
	const auto i_sec = _ttoi(cs_comment.Mid(i_char1, 2));
	i_char1 += 3;
	const auto i_year = _ttoi(cs_comment.Mid(i_char1, 4));

	wave_format->acquisition_time = CTime(i_year, i_month, i_day, i_hour, i_min, i_sec);

	// Date  : Thu Nov 01 17:45:24 2001
	// insect -> Jf#8
	// type -> 5th 2
	// stimulus -> Uma 0.05g, 10% EtOH, 20mM NaCl

	constexpr char od = 0xd;
	i_char1 = cs_comment.Find(_T("Pretrigger"));
	i_char1 = cs_comment.Find(od, i_char1) + 2;
	int i_char2 = cs_comment.Find(od, i_char1) + 1;
	wave_format->cs_insect_name = cs_comment.Mid(i_char1, i_char2 - i_char1 - 1);

	i_char1 = i_char2 + 1;
	i_char2 = cs_comment.Find(od, i_char1) + 1;
	wave_format->cs_sensillum = cs_comment.Mid(i_char1, i_char2 - i_char1 - 1);

	wave_format->cs_stimulus = cs_comment.Mid(i_char2 + 1, strlen - 1);

	wave_format->cs_comment.Empty();
	wave_format->cs_concentration.Empty();

	return DOCTYPE_ASDSYNTECH;
}

int CDataFileASD::check_file_type(CString& cs_filename)
{
	Seek(0L, begin);
	auto flag = DOCTYPE_UNKNOWN;
	auto i_len = 32;
	char buf[32];
	auto p_buf = &buf[0];
	do
	{
		Read(p_buf, sizeof(char));
		p_buf++;
		i_len--;
	}
	while (*(p_buf - 1) != 0 && i_len > 0);

	// is it an ASD file?
	if (buf != m_cs_old_string_id_ && buf != m_cs_string_id_)
		return flag;

	// (2) file version number
	WORD w;
	Read(&w, sizeof(WORD));
	auto w_type2 = SWAPWORD(w);
	if (w_type2 != 1)
	{
		AfxMessageBox(_T("Incorrect ID word in ASD file"));
		return flag;
	}

	// browse file and get list of data type chunks
	const auto file_length = GetLength() - 1;
	//auto offset = f->GetPosition();
	DWORD dw;

	while (GetPosition() < file_length)
	{
		// read tag / "new data block"
		Read(&w, sizeof(WORD));
		w_type2 = SWAPWORD(w);
		ASSERT(w_type2 == m_w_id_); // assert tag = 0xAAAA

		// check file type
		Read(&w, sizeof(WORD)); // (4) file version number
		w_type2 = SWAPWORD(w);

		switch (w_type2)
		{
		case DT_WAVE:
			{
				flag = DOCTYPE_ASDSYNTECH;
				m_ul_offset_header = static_cast<LONGLONG>(GetPosition());

				// (1) signal name
				CString cs_name;
				auto ch = ' ';
				while (ch != 0)
				{
					Read(&ch, sizeof(char));
					if (ch != 0) cs_name += ch;
				}

				Read(&dw, sizeof(DWORD));
				const DWORD rec_factor = SWAPLONG(dw);
				auto gain_post = static_cast<short>(rec_factor); // max is 32768
				short gain_pre = 1;
				ASSERT(rec_factor <= 32768);

				Read(&dw, sizeof(DWORD));
				auto sample_rate = static_cast<double>(SWAPLONG(dw)) / 1000.0;

				Read(&dw, sizeof(DWORD));
				const DWORD count = SWAPLONG(dw);

				for (DWORD index = 0; index < count; index++) // (6-7) percent and time
				{
					Read(&w, sizeof(WORD));
					int percent = SWAPWORD(w);
					Read(&dw, sizeof(DWORD));
					auto time = SWAPLONG(dw);
				}

				UINT ui_count = 0; // (8) number of record samples
				Read(&dw, sizeof(DWORD));
				ui_count = SWAPLONG(dw);

				const auto l_offset1 = GetPosition(); // start of data area
				m_ul_offset_data = l_offset1 + 1;
				const auto l_offset2 = static_cast<LONGLONG>(ui_count) * 2; // length of data area (in bytes)
				Seek(l_offset2, current); // position pointer

				Read(&w, sizeof(WORD));
				auto check_sum = SWAPWORD(w);
			}
			break;

		case DT_SPIKE:
		case DT_HISTOGRAM:
		case DT_ASCIITEXT:
		case DT_RICHTEXT:
			{
				WORD w = 0;
				do
				{
					Read(&w, sizeof(WORD));
					WORD w_type = SWAPWORD(w);
				}
				while (w != m_w_id_ && GetPosition() < file_length);
				// 0xAAAA
				if (w == m_w_id_)
				{
					const LONGLONG l_pos = static_cast<LONGLONG>(GetPosition()) - 2;
					Seek(l_pos, begin);
				}
			}
			break;
		default:
			//m_ulOffsetHeader = 0;
			AfxMessageBox(_T("Data type not supported yet"));
			break;
		}
	}

	return flag;
}
