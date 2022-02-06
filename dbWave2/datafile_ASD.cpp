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
	m_idType = DOCTYPE_ASDSYNTECH;
	m_csType = _T("ASDsyntech");
	m_csOldStringID = "AutoSpike 97 Data File";
	m_csStringID = "AutoSpike-32 Data File";
	m_wID = 0xAAAA;
	m_ulOffsetHeader = 29;
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

BOOL CDataFileASD::ReadDataInfos(CWaveBuf* pBuf)
{
	CWaveFormat* wave_format = pBuf->GetpWaveFormat();
	CWaveChanArray* wavechan_array = pBuf->GetpWavechanArray();

	Seek(29, begin); // position pointer
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
	const auto gainpost = static_cast<short>(rec_factor); // max is 32768
	const auto gainpre = 1;
	ASSERT(rec_factor <= 32768);

	Read(&dw, sizeof(DWORD));
	const auto sample_rate = static_cast<double>(SWAPLONG(dw)) / 1000.0;

	Read(&dw, sizeof(DWORD));
	const DWORD count = SWAPLONG(dw);

	for (DWORD index = 0; index < count; index++) // (6-7) percent and time
	{
		Read(&w, sizeof(WORD));
		//int percent = SWAPWORD(w);
		Read(&dw, sizeof(DWORD));
		//auto time = SWAPLONG(dw);
	}

	Read(&dw, sizeof(DWORD));
	const UINT uicount = SWAPLONG(dw);

	const auto l_offset1 = GetPosition(); // start of data area
	m_ulOffsetData = l_offset1 + 1;
	const auto l_offset2 = static_cast<ULONGLONG>(uicount) * 2; // length of data area (in bytes)
	Seek(l_offset2, current); // position pointer

	Read(&w, sizeof(WORD));
	//auto check_sum = SWAPWORD(w);

	Read(&w, sizeof(WORD)); // read type
	auto w_type = SWAPWORD(w);
	ASSERT(w_type == m_wID);

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
	wave_format->fullscale_volts = 10.0f; // 10 mv full scale
	wave_format->binspan = 32768; // 15 bits resolution
	wave_format->binzero = wave_format->binspan / 2; // ?

	wave_format->mode_encoding = OLx_ENC_BINARY;
	wave_format->mode_clock = INTERNAL_CLOCK;
	wave_format->mode_trigger = INTERNAL_TRIGGER;
	wave_format->sampling_rate_per_channel = static_cast<float>(sample_rate);
	wave_format->sample_count = uicount;

	//for (i = 0; i<wave_format->scan_count; i++)
	//{
	int i = 0;
	wavechan_array->ChanArray_add();
	auto pChan = wavechan_array->Get_p_channel(i);
	pChan->am_csComment = CString(" "); // channel annotation
	pChan->am_adchannel = 0; // channel scan list
	pChan->am_gainAD = 1; // channel gain list
	pChan->am_csamplifier = CString("syntechAmplifier"); // amplifier type
	pChan->am_csheadstage = CString("syntechProbe"); // headstage type
	pChan->am_gainheadstage = 10; // assume headstage gain = 10
	pChan->am_amplifierchan = 0; // assume 1 channel / amplifier
	pChan->am_gainpre = gainpre; // assume gain -pre and -post set before
	pChan->am_gainpost = gainpost;
	pChan->am_notchfilt = 0; // assume no notch filter
	pChan->am_lowpass = 0; // assume not low pass filtering
	pChan->am_offset = 0.0f; // assume no offset compensation
	pChan->am_csInputpos = "25";
	pChan->am_csInputneg = "GND";
	pChan->am_gainamplifier = static_cast<float>(rec_factor);

	pChan->am_gaintotal = pChan->am_gainamplifier * pChan->am_gainheadstage; // total gain
	pChan->am_resolutionV = wave_format->fullscale_volts / pChan->am_gaintotal / wave_format->binspan;

	// ---------------- ASD -- capture date and time

	const auto strlen = cs_comment.GetLength();
	auto ichar1 = 12;

	// month
	auto dummy = cs_comment.Mid(ichar1, 3);
	CString csmonth[] =
	{
		_T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"),
		_T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec")
	};

	int imonth;
	for (imonth = 0; imonth < 12; imonth++)
	{
		if (dummy.CompareNoCase(csmonth[imonth]) == 0)
			break;
	}
	imonth++;

	// day
	ichar1 += 4;
	dummy = cs_comment.Mid(ichar1, 2);
	const auto iday = _ttoi(dummy);

	// time
	ichar1 += 3;
	const auto ihour = _ttoi(cs_comment.Mid(ichar1, 2));
	ichar1 += 3;
	const auto imin = _ttoi(cs_comment.Mid(ichar1, 2));
	ichar1 += 3;
	const auto isec = _ttoi(cs_comment.Mid(ichar1, 2));
	ichar1 += 3;
	const auto iyear = _ttoi(cs_comment.Mid(ichar1, 4));

	wave_format->acqtime = CTime(iyear, imonth, iday, ihour, imin, isec);

	// Date  : Thu Nov 01 17:45:24 2001
	// insect -> Jf#8
	// type -> 5th 2
	// stimulus -> Uma 0.05g, 10% EtOH, 20mM NaCl

	const char od = 0xd;
	ichar1 = cs_comment.Find(_T("Pretrigger"));
	ichar1 = cs_comment.Find(od, ichar1) + 2;
	short ichar2 = cs_comment.Find(od, ichar1) + 1;
	wave_format->csInsectname = cs_comment.Mid(ichar1, ichar2 - ichar1 - 1);

	ichar1 = ichar2 + 1;
	ichar2 = cs_comment.Find(od, ichar1) + 1;
	wave_format->csSensillum = cs_comment.Mid(ichar1, ichar2 - ichar1 - 1);

	wave_format->csStimulus = cs_comment.Mid(ichar2 + 1, strlen - 1);

	wave_format->cs_comment.Empty();
	wave_format->csConcentration.Empty();

	return DOCTYPE_ASDSYNTECH;
}

int CDataFileASD::CheckFileType(CString& cs_filename)
{
	Seek(0L, begin);
	auto flag = DOCTYPE_UNKNOWN;
	auto i_len = 32;
	char buf[32];
	auto pbuf = &buf[0];
	do
	{
		Read(pbuf, sizeof(char));
		pbuf++;
		i_len--;
	}
	while (*(pbuf - 1) != 0 && i_len > 0);

	// is it an ASD file?
	if (buf != m_csOldStringID && buf != m_csStringID)
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
	const auto filelength = GetLength() - 1;
	//auto offset = f->GetPosition();
	DWORD dw;

	while (GetPosition() < filelength)
	{
		// read tag / "new data block"
		Read(&w, sizeof(WORD));
		w_type2 = SWAPWORD(w);
		ASSERT(w_type2 == m_wID); // assert tag = 0xAAAA

		// check file type
		Read(&w, sizeof(WORD)); // (4) file version number
		w_type2 = SWAPWORD(w);

		switch (w_type2)
		{
		case DT_WAVE:
			{
				flag = DOCTYPE_ASDSYNTECH;
				m_ulOffsetHeader = GetPosition();

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
				auto gainpost = static_cast<short>(rec_factor); // max is 32768
				short gainpre = 1;
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

				UINT uicount = 0; // (8) number of record samples
				Read(&dw, sizeof(DWORD));
				uicount = SWAPLONG(dw);

				const auto l_offset1 = GetPosition(); // start of data area
				m_ulOffsetData = l_offset1 + 1;
				const auto l_offset2 = static_cast<ULONGLONG>(uicount) * 2; // length of data area (in bytes)
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
				while (w != m_wID && GetPosition() < filelength);
				// 0xAAAA
				if (w == m_wID)
				{
					const LONGLONG lpos = GetPosition() - 2;
					Seek(lpos, begin);
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
