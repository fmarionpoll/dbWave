#pragma once
#include "datafile_X.h"
#include "dbWave_structures.h"
#include "WaveBuf.h"

constexpr auto CHANTYPE_unused = 0;
constexpr auto CHANTYPE_Adc = 1;
constexpr auto CHANTYPE_EventFall = 2;
constexpr auto CHANTYPE_EventRise = 3;
constexpr auto CHANTYPE_EventBoth = 4;
constexpr auto CHANTYPE_Marker = 5;
constexpr auto CHANTYPE_WaveMark = 6;
constexpr auto CHANTYPE_RealMark = 7;
constexpr auto CHANTYPE_TextMark = 8;
constexpr auto CHANTYPE_RealWave = 9;

constexpr int S64_OK = 0;
constexpr int NO_FILE = -1;
constexpr int NO_BLOCK = -2;
constexpr int CALL_AGAIN = -3;
constexpr int NO_ACCESS = -5;
constexpr int NO_MEMORY = -8;
constexpr int NO_CHANNEL = -9;
constexpr int CHANNEL_USED = -10;
constexpr int CHANNEL_TYPE = -11;
constexpr int PAST_EOF = -12;
constexpr int WRONG_FILE = -13;
constexpr int NO_EXTRA = -14;
constexpr int BAD_READ = -17;
constexpr int BAD_WRITE = -18;
constexpr int CORRUPT_FILE = -19;
constexpr int PAST_SOF = -20;
constexpr int READ_ONLY = -21;
constexpr int BAD_PARAM = -22;
constexpr int OVER_WRITE = -23;
constexpr int MORE_DATA = -24;

struct TTimeDate // bit compatible with TSONTimeDate
{
	uint8_t uc_hun; //!< hundreths of a second, 0-99
	uint8_t uc_sec; //!< seconds, 0-59
	uint8_t uc_min; //!< minutes, 0-59
	uint8_t uc_hour; //!< hour - 24 hour clock, 0-23
	uint8_t uc_day; //!< day of month, 1-31
	uint8_t uc_mon; //!< month of year, 1-12
	uint16_t w_year; //!< year 1980-65535! 0 means unset.

	//! Sets the contents to 0
	void clear()
	{
		uc_hun = uc_sec = uc_min = uc_hour = uc_day = uc_mon = 0;
		w_year = 0;
	}
};

class CDataFileFromCEDSpike2 :
	public CDataFileX
{
public:
	CDataFileFromCEDSpike2();
	DECLARE_DYNCREATE(CDataFileFromCEDSpike2)
	~CDataFileFromCEDSpike2() override;

	// Operations
public:
	int check_file_type(CString& cs_filename) override;
	BOOL read_data_infos(CWaveBuf* p_buf) override;
	bool open_data_file(CString& sz_path_name, UINT u_open_flag) override;
	void close_data_file() override;
	long read_adc_data(long l_first, long nb_points_all_channels, short* p_buffer, CWaveChanArray* p_array) override;

	// Implementation
public:
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

protected:
	static number_id_to_text error_messages_[];
	int m_n_fid_ = -1;
	boolean b_relocate_if_start_with_gap_ = false;
	boolean b_remove_gaps_ = true;
	long long ticks_per_sample_ = -1;
	long long ll_file_offset_ = 0;

	void read_channel_parameters(CWaveChan* p_chan, int ced_chan);
	CString read_file_comment(int n_ind) const;
	CString read_channel_comment(int ced_chan);
	CString read_channel_title(int ced_chan);
	long read_channel_data(const CWaveChan* p_chan, short* p_data, long long ll_first, long long ll_n_values);
	static long relocate_channel_data(short* p_buffer, long long t_from, long long t_first, int n_values_read, long long ticks_per_sample);
	static CString get_error_message(int flag);
	void read_event_fall(int ced_chan, CWaveBuf* p_buf);
	void convert_vt_tags_ticks_to_ad_intervals(CWaveBuf* p_buf, int ced_chan);
};
