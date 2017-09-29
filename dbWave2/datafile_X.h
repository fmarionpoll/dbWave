// CDataFileX.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CDataFileX document

#pragma once

// The different type of the document recognized
#define DOCTYPE_AWAVE 		0	// aWave document
#define DOCTYPE_ASYST 		1	// ASYST document
#define DOCTYPE_ATLAB 		2	// ATLAB document
#define DOCTYPE_ASDSYNTECH 	3	// ASD SYNTECH document (data)
#define DOCTYPE_MCID		4	// MCI cf A French, Halifax
#define DOCTYPE_UNKNOWN		-1	// document type not recognized

class CDataFileX : public CFile
{
public:
	CDataFileX();			// constructor used by dynamic creation
	~CDataFileX();
	DECLARE_DYNCREATE(CDataFileX)

// Operations
	virtual BOOL CheckFileType(CFile* pfile, int bSignatureOffset=0);
	virtual BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray);
	virtual	long ReadData(long dataIndex, long nbpoints, short* pBuffer);
	virtual BOOL ReadHZtags(CTagList* pHZtags);
	virtual BOOL ReadVTtags(CTagList* pVTtags);

	// I/O operations to be updated by Save
	virtual BOOL InitFile();
	virtual BOOL DataAppendStart();
	virtual BOOL DataAppend(short* pBU, UINT uibytesLength);
	virtual BOOL DataAppendStop();
	virtual BOOL WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC);
	virtual BOOL WriteHZtags(CTagList* ptags);
	virtual BOOL WriteVTtags(CTagList* ptags);

// helpers
	CString	GetFileType();

//data parameters
	int			m_bHeaderSize;		// data header size
	int			m_idType;			// file type 0=unknown; 1=atlab; 2=Asyst; 3=awave; ..
	CStringA	m_csType;
	ULONGLONG	m_ulOffsetData;		// file offset of data start
	ULONGLONG	m_ulOffsetHeader;	// file offset of data header
	ULONGLONG	m_ulbytescount;		// dummy parameter: Data append

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

};
