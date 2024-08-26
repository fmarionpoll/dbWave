// CDataFileMCID.cpp : implementation file
//

#include "StdAfx.h"
#include "dataheader_Atlab.H"
#include "datafile_mcid.h"

#include <Olxdadefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CDataFileMCID, CDataFileX)

CDataFileMCID::CDataFileMCID()
{
	m_b_header_size = sizeof(MCID_HEADER);
	m_ul_offset_data = m_b_header_size;
	m_ul_offset_header = 0;
	m_id_type = DOCTYPE_MCID;
	m_cs_type = _T("MCID");
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

BOOL CDataFileMCID::read_data_infos(CWaveBuf* pBuf)
{
	CWaveFormat* p_w_format = pBuf->get_p_wave_format();
	CWaveChanArray* p_array = pBuf->get_p_wave_chan_array();
	MCID_HEADER mc_id_header;
	Seek(0, begin);
	Read(&mc_id_header, sizeof(MCID_HEADER));

	m_b_header_size = sizeof(MCID_HEADER) + mc_id_header.application_header;
	m_ul_offset_data = m_b_header_size;

	p_w_format->scan_count = mc_id_header.number_of_channels; // 4   number of channels in scan list

	for (UINT i = 0; i < mc_id_header.number_of_channels; i++)
	{
		p_array->chan_array_add();
		const auto p_chan = p_array->get_p_channel(i);
		p_chan->am_csComment = CString(" ");
		p_chan->am_resolutionV = mc_id_header.sensitivity[i] / 2000.;
		p_chan->am_amplifiergain = 1. / p_chan->am_resolutionV;

		p_chan->am_adchannel = 0; // channel scan list
		p_chan->am_gainAD = 1; // channel gain list
		p_chan->am_csamplifier = CString(" "); // amplifier type
		p_chan->am_csheadstage = CString(" "); // head stage type
		p_chan->am_gainheadstage = 1; // assume head stage gain = 1
		p_chan->am_amplifierchan = 0; // assume 1 channel / amplifier
		p_chan->am_gainpre = 1; // assume gain -pre and -post = 1
		p_chan->am_gainpost = 1;
		p_chan->am_notchfilt = 0; // assume no notch filter
		p_chan->am_lowpass = 0; // assume not low pass filtering
		p_chan->am_offset = 0.0f; // assume no offset compensation
		p_chan->am_csInputpos = "DC"; // assume input + = DC
		p_chan->am_csInputneg = "GND"; // assume input - = GND
		p_chan->am_gaintotal = p_chan->am_amplifiergain;
	}

	//	tentative
	p_w_format->full_scale_volts = 5.0f; // 10 V full scale
	p_w_format->bin_span = 65536; // 16 bits resolution
	p_w_format->bin_zero = 0;

	p_w_format->mode_encoding = OLx_ENC_BINARY;
	p_w_format->mode_clock = INTERNAL_CLOCK;
	p_w_format->mode_trigger = INTERNAL_TRIGGER;
	p_w_format->sampling_rate_per_channel = static_cast<float>(1. / mc_id_header.sample_interval);
	p_w_format->sample_count = mc_id_header.number_of_samples;

	p_w_format->cs_comment.Empty();
	p_w_format->cs_stimulus.Empty();
	p_w_format->cs_concentration.Empty();
	p_w_format->cs_sensillum.Empty();

	return TRUE;
}

int CDataFileMCID::check_file_type(CString& cs_filename)
{
	constexpr BOOL flag = DOCTYPE_UNKNOWN;
	MCID_HEADER mc_id_header;
	Seek(0, begin);
	Read(&mc_id_header, sizeof(MCID_HEADER));
	m_b_header_size = mc_id_header.application_header;
	m_ul_offset_data = mc_id_header.application_header;

	// test "mcid"
	/*if (tab[8]==166 && tab[9]==166 && tab[10]==166 && tab[11]==166)
		flag = m_idType;*/
	return flag;
}
