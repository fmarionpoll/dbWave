
#pragma once

// Document types 
constexpr auto DOCTYPE_UNKNOWN		= -1;	// document type not recognized;
constexpr auto DOCTYPE_AWAVE		= 0;	// aWave document;
constexpr auto DOCTYPE_ASYST		= 1;	// ASYST document;
constexpr auto DOCTYPE_ATLAB		= 2;	// ATLAB document;
constexpr auto DOCTYPE_ASDSYNTECH	= 3;	// ASD SYNTECH document (data);
constexpr auto DOCTYPE_MCID			= 4;	// MCI cf A French, Halifax;
constexpr auto DOCTYPE_SMR			= 5;	// CFS cambridge Filing system;

class CDataFileX : public CFile
{
public:
	CDataFileX();
	~CDataFileX();
	DECLARE_DYNCREATE(CDataFileX)

	// Operations
	virtual int		CheckFileType(CString& cs_filename);
	virtual BOOL	ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray);
	virtual	long	ReadAdcData(long dataIndex, long nbpoints, short* pBuffer, CWaveChanArray* pArray);
	virtual BOOL	ReadHZtags(CTagList* pHZtags);
	virtual BOOL	ReadVTtags(CTagList* pVTtags);

	// I/O operations to be updated by Save
	virtual BOOL	InitFile();
	virtual BOOL	DataAppendStart();
	virtual BOOL	DataAppend(short* pBU, UINT uibytesLength);
	virtual BOOL	DataAppendStop();
	virtual BOOL	WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC);
	virtual BOOL	WriteHZtags(CTagList* ptags);
	virtual BOOL	WriteVTtags(CTagList* ptags);

	//data parameters
	UINT			m_u_open_flag;
	int				m_bHeaderSize;		// data header size
	int				m_idType;			// file type 0=unknown; 1=atlab; 2=Asyst; 3=awave; ..
	CStringA		m_csType;
	ULONGLONG		m_ulOffsetData;		// file offset of data start
	ULONGLONG		m_ulOffsetHeader;	// file offset of data header
	ULONGLONG		m_ulbytescount;		// dummy parameter: Data append

// Implementation
public:
	virtual bool	OpenDataFile(CString& sz_path_name, UINT u_open_flag);
	virtual void	CloseDataFile();

#ifdef _DEBUG
	virtual void	AssertValid() const;
	virtual void	Dump(CDumpContext& dc) const;
#endif
protected:
	int				isPatternPresent(char* bufRead, int lenRead, const char* bufPattern, int lenPattern);

};
