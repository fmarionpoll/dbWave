#include "StdAfx.h"
#include "datafile_CEDSpike2.h"
#include "dataheader_Atlab.H"
#include "ceds64int.h"
#include <Olxdadefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CDataFileFromCEDSpike2, CDataFileX)
const int LENCEDSON = 16;
const char CEDSON64[] = "CED_DATA";

number_id_to_text CDataFileFromCEDSpike2::error_messages_[] = {
	{S64_OK, _T("There was no error")},
	{NO_FILE, _T("Attempt to use when file not open, or use of an invalid file handle, or no spare file handle")},
	{
		NO_BLOCK, _T(
			"Failed to allocate a disk block when writing to the file.The disk is probably full, or there was a disk error.")
	},
	{CALL_AGAIN, _T("This is a long operation, call again.")},
	{NO_ACCESS, _T("This operation was not allowed.")},
	{NO_MEMORY, _T("Out of memory reading a 32 - bit son file.")},
	{NO_CHANNEL, _T("A channel does not exist.")},
	{CHANNEL_USED, _T("Attempt to reuse a channel that already exists.")},
	{CHANNEL_TYPE, _T("The channel cannot be used for this operation.")},
	{PAST_EOF, _T("Read past the end of the file.")},
	{WRONG_FILE, _T("Attempt to open wrong file type.This is not a SON file.")},
	{NO_EXTRA, _T("A request to read user data is outside the extra data region.")},
	{BAD_READ, _T("A read error(disk error was detected.This is an operating system error.")},
	{BAD_WRITE, _T("Something went wrong writing data.This is an operating system error.")},
	{CORRUPT_FILE, _T("The file is bad or an attempt to write corrupted data.")},
	{PAST_SOF, _T("An attempt was made to access data before the start of the file.")},
	{READ_ONLY, _T("Attempt to write to a read only file.")},
	{BAD_PARAM, _T("A bad parameter to a call into the SON library.")},
	{OVER_WRITE, _T("An attempt was made to over - write data when not allowed.")},
	{MORE_DATA, _T("A file is bigger than the header says; maybe not closed correctly.")}
};

CDataFileFromCEDSpike2::CDataFileFromCEDSpike2()
{
	m_b_header_size = 512;
	m_ul_offset_data = m_b_header_size;
	m_ul_offset_header = 0;
	m_id_type = DOCTYPE_SMR;
	m_cs_type = CEDSON64;
}

CDataFileFromCEDSpike2::~CDataFileFromCEDSpike2()
{
	close_data_file();
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

bool CDataFileFromCEDSpike2::open_data_file(CString& sz_path_name, UINT u_open_flag)
{
	m_n_fid_ = S64Open(CT2A(sz_path_name), -1);
	if (m_n_fid_ <= 0)
	{
		CString message = CString("OpenCEDDataFile error: ") + get_error_message(m_n_fid_);
		AfxMessageBox(message, MB_OK);
		return false;
	}
	return true;
}

void CDataFileFromCEDSpike2::close_data_file()
{
	if (m_n_fid_ >= 0)
	{
		const int flag = S64Close(m_n_fid_);
		if (flag < 0)
			AfxMessageBox(CString("CloseCEDDataFile error: ") + get_error_message(flag), MB_OK);
		m_n_fid_ = -1;
	}
}

int CDataFileFromCEDSpike2::check_file_type(CString& cs_fileName)
{
	cs_fileName.MakeLower();
	if (cs_fileName.Find(_T("smr")) != -1)
	{
		return DOCTYPE_SMR;
	}
	return DOCTYPE_UNKNOWN;
}

BOOL CDataFileFromCEDSpike2::read_data_infos(CWaveBuf* p_buf)
{
	CWaveFormat* p_w_format = p_buf->get_p_wave_format();
	CWaveChanArray* p_array = p_buf->get_p_wave_chan_array();

	//	tentative
	p_w_format->full_scale_volts = 5.0f; // 10 V full scale
	p_w_format->bin_span = 65536; // 16 bits resolution
	p_w_format->bin_zero = 0;
	p_w_format->mode_encoding = OLx_ENC_BINARY;
	p_w_format->mode_clock = INTERNAL_CLOCK;
	p_w_format->mode_trigger = INTERNAL_TRIGGER;
	p_w_format->cs_ad_card_name = "CED1401";

	// get comments
	p_w_format->cs_comment.Empty();
	p_w_format->cs_stimulus.Empty();
	p_w_format->cs_concentration.Empty();
	p_w_format->cs_sensillum.Empty();

	// get global data and n channels
	double time_base = S64GetTimeBase(m_n_fid_);
	int lowest_free_chan = S64GetFreeChan(m_n_fid_);
	const int max_chan = S64MaxChans(m_n_fid_);

	TTimeDate array_get_time_date{};
	const int flag = S64TimeDate(m_n_fid_, reinterpret_cast<long long*>(&array_get_time_date), nullptr, -1);
	if (flag < 0)
	{
		const CString message = CString("Error reading date ") + get_error_message(flag) + CString(" \n");
		AfxMessageBox(message, MB_OK);
		return false;
	}
	p_w_format->acquisition_time = CTime(
		array_get_time_date.w_year, array_get_time_date.uc_mon, array_get_time_date.uc_day,
		array_get_time_date.uc_hour, array_get_time_date.uc_min, array_get_time_date.uc_sec);
	p_w_format->scan_count = 0;
	p_array->chan_array_remove_all();
	int adc_chan = -1;

	for (int ced_chan = 1; ced_chan < max_chan; ced_chan++)
	{
		// lowestFreeChan
		const int chan_type = S64ChanType(m_n_fid_, ced_chan);
		if (chan_type == 0)
			continue;

		CString descriptor;
		switch (chan_type)
		{
		case CHANTYPE_Adc:
			{
				descriptor = "Adc data";
				const int i = p_array->chan_array_add();
				const auto p_chan = p_array->get_p_channel(i);
				read_channel_parameters(p_chan, ced_chan);
				p_w_format->scan_count++;
				p_w_format->sampling_rate_per_channel = static_cast<float>(1.0 / (static_cast<double>(p_chan->am_CEDticksPerSample) * S64GetTimeBase(m_n_fid_)));
				p_w_format->sample_count = static_cast<long>(p_chan->am_CEDmaxTimeInTicks / p_chan->am_CEDticksPerSample);
				adc_chan = ced_chan;
			}
			break;
		case CHANTYPE_EventFall:
			descriptor = "Event on falling edge";
			read_event_fall(ced_chan, p_buf);
			break;
		case CHANTYPE_EventRise:
			descriptor = "Event on rising edge";
			break;
		case CHANTYPE_EventBoth:
			descriptor = "Event on both edges";
			break;
		case CHANTYPE_Marker:
			descriptor = "Marker data";
			break;
		case CHANTYPE_WaveMark:
			descriptor = "WaveMark data";
			break;
		case CHANTYPE_RealMark:
			descriptor = "RealMark data";
			break;
		case CHANTYPE_TextMark:
			descriptor = "TextMark data";
			break;
		case CHANTYPE_RealWave:
			descriptor = "RealWave data";
			break;
		case CHANTYPE_unused:
		default:
			descriptor = "unused channel";
			break;
		}
	}
	if (adc_chan >= 0)
		convert_vt_tags_ticks_to_ad_intervals(p_buf, adc_chan);

	TagList* p_tags = p_buf->get_p_vt_tags();
	if (p_tags != nullptr && p_tags->get_tag_list_size() > 0 && p_w_format->scan_count > 0)
	{
		for (int i = 0; i < p_array->chan_array_get_size(); i++)
		{
			const auto p_chan = p_array->get_p_channel(i);
			if (ticks_per_sample_ != p_chan->am_CEDticksPerSample)
				ticks_per_sample_ = p_chan->am_CEDticksPerSample;
		}
		const Tag* p_tag = p_tags->get_tag(0);
		ll_file_offset_ = p_tag->l_ticks / ticks_per_sample_;
		const int new_length = p_w_format->sample_count - static_cast<int>(ll_file_offset_);
		p_w_format->sample_count = new_length;
	}

	return TRUE;
}

void CDataFileFromCEDSpike2::read_channel_parameters(CWaveChan* p_chan, int ced_chan)
{
	p_chan->am_CEDchanID = ced_chan;
	p_chan->am_csamplifier.Empty(); // amplifier type
	p_chan->am_csheadstage.Empty(); // head_stage type
	p_chan->am_csComment.Empty(); // channel comment
	p_chan->am_csComment = read_channel_comment(ced_chan);

	p_chan->am_adchannel = 0; // channel scan list
	p_chan->am_gainAD = 1; // channel gain list
	p_chan->am_gainheadstage = 1; // assume head_stage gain = 1
	p_chan->am_amplifierchan = 0; // assume 1 channel / amplifier
	p_chan->am_gainpre = 1; // assume gain -pre = 1
	p_chan->am_gainpost = 1; // assume gain -post = 1
	p_chan->am_notchfilt = 0; // assume no notch filter
	p_chan->am_lowpass = 0; // assume not low pass filtering
	p_chan->am_offset = 0.0f; // assume no offset compensation
	p_chan->am_csInputpos = "DC"; // assume input + = DC
	p_chan->am_csInputneg = "GND"; // assume input - = GND

	int flag = S64GetChanOffset(m_n_fid_, ced_chan, &p_chan->am_CEDoffset);
	flag = S64GetChanScale(m_n_fid_, ced_chan, &p_chan->am_CEDscale);

	p_chan->am_amplifiergain = p_chan->am_CEDscale * 10.;
	p_chan->am_resolutionV = 2.5 / p_chan->am_amplifiergain / 65536;
	p_chan->am_gaintotal = p_chan->am_amplifiergain;

	p_chan->am_CEDticksPerSample = S64ChanDivide(m_n_fid_, ced_chan);
	p_chan->am_CEDmaxTimeInTicks = S64ChanMaxTime(m_n_fid_, ced_chan);
}

CString CDataFileFromCEDSpike2::read_channel_comment(const int ced_chan)
{
	const int size_comment = S64GetChanComment(m_n_fid_, ced_chan, nullptr, -1);
	CString comment;
	if (size_comment > 0)
	{
		const auto buffer = new char[size_comment];
		int flag = S64GetChanComment(m_n_fid_, ced_chan, buffer, 0);
		comment = CString(buffer);
		delete[] buffer;
	}
	return comment;
}

CString CDataFileFromCEDSpike2::read_channel_title(const int ced_chan)
{
	const int size_comment = S64GetChanTitle(m_n_fid_, ced_chan, nullptr, -1);
	CString comment;
	if (size_comment > 0)
	{
		const auto buffer = new char[size_comment];
		int flag = S64GetChanTitle(m_n_fid_, ced_chan, buffer, 0);
		comment = CString(buffer);
		delete[] buffer;
	}
	return comment;
}

CString CDataFileFromCEDSpike2::read_file_comment(const int n_ind) const
{
	const int size_comment = S64GetFileComment(m_n_fid_, n_ind, nullptr, -1);
	CString comment;
	if (size_comment > 0)
	{
		const auto buffer = new char[size_comment];
		int flag = S64GetFileComment(m_n_fid_, n_ind, buffer, 0);
		comment = CString(buffer);
		delete[] buffer;
	}
	return comment;
}

long CDataFileFromCEDSpike2::read_adc_data(const long l_first, const long nb_points_all_channels, short* p_buffer, CWaveChanArray* p_array)
{
	const int scan_count = p_array->chan_array_get_size();
	const long long ll_data_n_values = nb_points_all_channels / scan_count / sizeof(short);
	int n_values_read = -1;

	const long long ll_First = l_first + ll_file_offset_;

	for (int channel = 0; channel < scan_count; channel++)
	{
		const CWaveChan* p_chan = p_array->get_p_channel(channel);
		// TODO: create channel buffer
		const size_t number_bytes = static_cast<int>(ll_data_n_values) * sizeof(short);
		memset(p_buffer, 0, number_bytes);
		n_values_read = read_channel_data(p_chan, p_buffer, ll_First, ll_data_n_values);
	}
	// TODO: combine channels buffers to build interleaved data 
	return n_values_read;
}

long CDataFileFromCEDSpike2::read_channel_data(const CWaveChan* p_chan, short* p_data, const long long ll_first, const long long ll_n_values)
{
	const int chan_id = p_chan->am_CEDchanID;
	const long long ticks_per_sample = S64ChanDivide(m_n_fid_, chan_id);
	const long long t_up_to = (ll_first + ll_n_values - 1) * ticks_per_sample;
	int number_of_values_read = 0;

	while (number_of_values_read < ll_n_values)
	{
		constexpr int n_mask = 0;
		const int n_max = static_cast<int>(ll_n_values) - number_of_values_read;
		const long long t_from = (ll_first + number_of_values_read) * ticks_per_sample;
		if (t_from >= t_up_to)
			break;

		short* p_buffer = p_data + number_of_values_read;
		long long t_first{};
		const int n_values_read = S64ReadWaveS(m_n_fid_, chan_id, p_buffer, n_max, t_from, t_up_to, &t_first, n_mask);

		if (n_values_read <= 0)
			break;

		number_of_values_read += n_values_read;
		if (t_first > t_from)
		{
			if (b_relocate_if_start_with_gap_)
				relocate_channel_data(p_buffer, t_from, t_first, n_values_read, ticks_per_sample);
			number_of_values_read += static_cast<long>(t_first / ticks_per_sample);
		}
	}
	return number_of_values_read;
}

long CDataFileFromCEDSpike2::relocate_channel_data(short* p_buffer, long long t_from, long long t_first, int n_values_read,
                                                  long long ticks_per_sample)
{
	const long offset = static_cast<long>((t_first - t_from) / ticks_per_sample);
	size_t count = n_values_read * sizeof(short);
	memmove(p_buffer + offset, p_buffer, count);

	count = (offset - 1) * sizeof(short);
	memset(p_buffer, 0, count);
	return offset;
}

CString CDataFileFromCEDSpike2::get_error_message(const int flag)
{
	constexpr int n_items = std::size(error_messages_);
	ASSERT(n_items == 20);
	CString error_msg = _T("error not found");
	for (const auto& error_message : error_messages_)
	{
		if (flag == error_message.value)
		{
			error_msg = error_message.cs_text;
			break;
		}
	}
	return error_msg;
}

void CDataFileFromCEDSpike2::read_event_fall(const int ced_chan, CWaveBuf* p_buf)
{
	TagList* p_tags = p_buf->get_p_vt_tags();
	p_tags->remove_all_tags();
	int n_read = 0;
	long long data = -1;
	do
	{
		n_read = S64ReadEvents(m_n_fid_, ced_chan, &data, 1, data + 1, -1, 0);
		if (n_read > 0)
			p_tags->add_tag(Tag(data));
	}
	while (n_read > 0);
}

void CDataFileFromCEDSpike2::convert_vt_tags_ticks_to_ad_intervals(CWaveBuf* p_buf, const int ced_chan)
{
	TagList* p_tags = p_buf->get_p_vt_tags();
	const int n_tags = p_tags->get_tag_list_size();
	if (n_tags > 0)
	{
		const long long ticks_per_sample = S64ChanDivide(m_n_fid_, ced_chan);
		for (int i = 0; i < n_tags; i++)
		{
			Tag* p_tag = p_tags->get_tag(i);
			p_tag->value_long = static_cast<long>(p_tag->l_ticks / ticks_per_sample);
			p_tag->ref_channel = ced_chan;
		}
	}
}
