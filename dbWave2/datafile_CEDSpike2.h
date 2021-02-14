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

class CDataFileFromCEDSpike2 :
    public CDataFileX
{
public:
	CDataFileFromCEDSpike2();
	DECLARE_DYNCREATE(CDataFileFromCEDSpike2)
	virtual ~CDataFileFromCEDSpike2();

	// Operations
public:
	int  CheckFileType(CFile* file) override;
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray) override;
	bool isOpened(CString& sz_path_name) override;
	bool openDataFile(CString& sz_path_name, UINT u_open_flag) override;
	void closeDataFile() override;

	// Implementation
public:
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CStringA		m_csFiledesc;
	CWaveFormat*	m_pWFormat;
	CWaveChanArray* m_pArray;
	int				m_nFid;

	void addAdcChannelFromCEDFile(int nChan, CWaveChanArray* pArray);
};

