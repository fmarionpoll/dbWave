#pragma once
#include "datafile_X.h"

class CDataFileAWAVE : public CDataFileX
{
public:
	// Construction / Destruction
	CDataFileAWAVE();
	CDataFileAWAVE(CFile* file);
	~CDataFileAWAVE() override;

	int CheckFileType(CString& cs_filename) override;
	BOOL ReadDataInfos(CWaveBuf* pBuf) override;
	BOOL ReadHZtags(TagList* pHZtags) override;
	BOOL ReadVTtags(TagList* pVTtags) override;
	BOOL WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC) override;
	BOOL WriteHZtags(TagList* ptags) override;
	BOOL WriteVTtags(TagList* ptags) override;

	// I/O operations to be updated by Save
	BOOL InitFile() override;
	BOOL DataAppendStart() override;
	BOOL DataAppend(short* pBU, UINT bytesLength) override;
	BOOL DataAppendStop() override;

	// protected variables
protected:
	CStringA m_csFiledesc; // ASCII chain w. file version
	CMapWordToOb m_structMap; // subfile descriptors
	BOOL m_bmodified{};

	// Implementation
protected:
	void DeleteMap();
	void WriteFileMap();
	void ReportSaveLoadException(LPCTSTR filename, CException* e, BOOL bSaving, UINT nIDP);
};
