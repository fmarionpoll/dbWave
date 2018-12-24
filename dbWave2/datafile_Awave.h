// AwaveFile.h 
// Definition of Awave files and I/O functions
#pragma once

///////////////////////////////////////////////////////////////////

// Proprietary data file
class CDataFileAWAVE : public CDataFileX
{
public:
	// Construction / Destruction
	CDataFileAWAVE();
	CDataFileAWAVE(CFile* file);
	virtual ~CDataFileAWAVE();

	BOOL CheckFileType(CFile* pfile) override;
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray) override;
	BOOL ReadHZtags(CTagList* pHZtags) override;
	BOOL ReadVTtags(CTagList* pVTtags) override;
	BOOL WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC) override;
	BOOL WriteHZtags(CTagList* ptags) override;
	BOOL WriteVTtags(CTagList* ptags) override;

	// I/O operations to be updated by Save
	BOOL InitFile() override;	
	BOOL DataAppendStart() override;
	BOOL DataAppend(short* pBU, UINT bytesLength) override;
	BOOL DataAppendStop() override;
	
	// protected variables
protected:
	CStringA		m_csFiledesc;	// ASCII chain w. file version
	CMapWordToOb	m_structMap;	// subfile descriptors
	BOOL			m_bmodified{};

	// Implementation
protected:	
	void DeleteMap();
	void WriteFileMap();
	void ReportSaveLoadException(LPCTSTR filename, CException* e, BOOL bSaving, UINT nIDP);
};
