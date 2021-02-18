#pragma once
#include "datafile_X.h"

constexpr auto CHANTYPE_unused		= 0;
constexpr auto CHANTYPE_Adc			= 1;
constexpr auto CHANTYPE_EventFall	= 2;
constexpr auto CHANTYPE_EventRise	= 3;
constexpr auto CHANTYPE_EventBoth	= 4;
constexpr auto CHANTYPE_Marker		= 5;
constexpr auto CHANTYPE_WaveMark	= 6;
constexpr auto CHANTYPE_RealMark	= 7;
constexpr auto CHANTYPE_TextMark	= 8;
constexpr auto CHANTYPE_RealWave	= 9;


struct TTimeDate            // bit compatible with TSONTimeDate
{
	uint8_t ucHun;          //!< hundreths of a second, 0-99
	uint8_t ucSec;          //!< seconds, 0-59
	uint8_t ucMin;          //!< minutes, 0-59
	uint8_t ucHour;         //!< hour - 24 hour clock, 0-23
	uint8_t ucDay;          //!< day of month, 1-31
	uint8_t ucMon;          //!< month of year, 1-12
	uint16_t wYear;         //!< year 1980-65535! 0 means unset.

	//! Sets the contents to 0
	void clear() { ucHun = ucSec = ucMin = ucHour = ucDay = ucMon = 0; wYear = 0; }
};


class CDataFileFromCEDSpike2 :
    public CDataFileX
{
public:
	CDataFileFromCEDSpike2();
	DECLARE_DYNCREATE(CDataFileFromCEDSpike2)
	virtual ~CDataFileFromCEDSpike2();

	// Operations
public:
	int  CheckFileType(CString& cs_filename) override;
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray) override;
	bool isOpened(CString& sz_path_name) override;
	bool openDataFile(CString& sz_path_name, UINT u_open_flag) override;
	void closeDataFile() override;
	long ReadAdcData(long dataIndex, long nbpoints, short* pBuffer, CWaveChanArray* pArray) override;

	// Implementation
public:
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CStringA		m_csFiledesc;
	CWaveFormat*	m_pWFormat		= nullptr;
	CWaveChanArray* m_pArray		= nullptr;
	int				m_nFid			=-1;

	CWaveChan*	getAdcChannel(int nChan, CWaveChanArray* pArray);
	CString		getFileComment(int nInd);
	CString		getChannelComment(int nChan);
	long		ReadOneChanAdcData(CWaveChan* pChan, short* pData, long long llDataIndex, long long llDataNValues);

};

