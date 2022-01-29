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
	uint8_t ucHun; //!< hundreths of a second, 0-99
	uint8_t ucSec; //!< seconds, 0-59
	uint8_t ucMin; //!< minutes, 0-59
	uint8_t ucHour; //!< hour - 24 hour clock, 0-23
	uint8_t ucDay; //!< day of month, 1-31
	uint8_t ucMon; //!< month of year, 1-12
	uint16_t wYear; //!< year 1980-65535! 0 means unset.

	//! Sets the contents to 0
	void clear()
	{
		ucHun = ucSec = ucMin = ucHour = ucDay = ucMon = 0;
		wYear = 0;
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
	int CheckFileType(CString& cs_filename) override;
	BOOL ReadDataInfos(CWaveBuf* pBuf) override;
	bool OpenDataFile(CString& sz_path_name, UINT u_open_flag) override;
	void CloseDataFile() override;
	long ReadAdcData(long dataIndex, long nbpoints, short* pBuffer, CWaveChanArray* pArray) override;

	// Implementation
public:
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

protected:
	static numberIDToText errorMessages[];
	int m_nFid = -1;
	boolean m_bRelocate_if_StartWithGap = false;
	boolean m_bRemoveGaps = true;
	long long m_ticksPerSample = -1;
	long long m_llFileOffset = 0;

	void read_ChannelParameters(CWaveChan* pChan, int cedChan);
	CString read_FileComment(int nInd);
	CString read_ChannelComment(int cedChan);
	CString read_ChannelTitle(int cedChan);
	long read_ChannelData(CWaveChan* pChan, short* pData, long long llDataIndex, long long llDataNValues);
	long relocate_ChannelData(short* pBuffer, long long tFrom, long long tFirst, int nValuesRead,
	                          long long ticksPerSample);
	CString getErrorMessage(int flag);
	void read_EventFall(int cedChan, CWaveBuf* pBuf);
	void convert_VTtags_Ticks_to_ADintervals(CWaveBuf* pBuf, int cedChan);
};
