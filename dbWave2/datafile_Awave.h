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

	BOOL CheckFileType(CFile* pfile, int bSignatureOffset=0);
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray);
	BOOL ReadHZtags(CTagList* pHZtags);
	BOOL ReadVTtags(CTagList* pVTtags);
	BOOL WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC);
	BOOL WriteHZtags(CTagList* ptags);
	BOOL WriteVTtags(CTagList* ptags);

	// I/O operations to be updated by Save
	BOOL InitFile();	
	BOOL DataAppendStart();
	BOOL DataAppend(short* pBU, UINT bytesLength);
	BOOL DataAppendStop();
	
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
