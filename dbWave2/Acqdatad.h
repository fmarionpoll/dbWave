// acqdatad.h : header file

#pragma once

// sort options :
#define		BY_TIME		1
#define		BY_NAME		2

#include "WaveBuf.h"		// data acquisition buffer
#include "datafile_X.h"
#include "Awavepar.h"		// user parameters

class CAcqDataDoc : public CDocument
{
	friend class CImportGenericDataDlg;
	DECLARE_DYNCREATE(CAcqDataDoc)
public:
	CAcqDataDoc();		// constructor used by dynamic creation
	virtual ~CAcqDataDoc();

// Operations
//-----------
public:
	CString			GetDataFileInfos(OPTIONS_VIEWDATA* pVD);
	void			ExportDataFile_to_TXTFile(CStdioFile* pdataDest);
	
	// Implementation
	//---------------
	BOOL 			OnSaveDocument (CString &sz_path_name) ;
	BOOL 			OnOpenDocument (CString &sz_path_name) ;
	BOOL 			OnNewDocument() override;

protected:
	BOOL			OpenAcqFile(CString &szfilename);

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

// Generated message map functions
//--------------------------------
protected:
	DECLARE_MESSAGE_MAP()

// -------------------------------
// Data members and functions dealing with CDataFileX and data reading buffer
// -----------------------------------

	CDataFileX*		m_pXFile;			// data file /* CDataFileX* */
	CWaveBuf*		m_pWBuf;			// CWaveBuffer
	CTagList		m_hz_tags{};		// list of horizontal cursors
	CTagList		m_vt_tags{};		// list of vertical tags
	
	long			m_lBUFmaxSize;		// constant (?) size of the buffer
	long			m_lBUFchanFirst;	// file index of first word in RW buffer
	long			m_lBUFchanLast;		// file index of last word in RW buffer
	int				m_DOCnbchans;		// nb channels / doc
	int				m_iOffsetInt;		// offset in bytes
	BOOL			m_bValidReadBuffer;	// flag / valid data into buffer
										// set to FALSE to force reading data from file
	BOOL			m_bValidTransfBuffer;	// flag to tell that transform buffer is valid (check if really used?)
	BOOL			m_bdefined;			// file that remain open
	long			m_lBUFSize;			// buffer size (n channels * sizeof(word) * lRWSize
	long			m_lBUFchanSize;		// n words in buffer / channel
	BOOL			m_bRemoveOffset;	// transform data read in short values if binary offset encoding

	// use with caution - parameters set by last call to LoadTransfData
	long			m_tBUFfirst;
	long			m_tBUFlast;
	int				m_tBUFtransform;
	int				m_tBUFsourcechan;

public:
	inline long			GettBUFfirst() const {return m_tBUFfirst;}
	inline CTagList*	GetpHZtags() {return &m_hz_tags;}
	inline CTagList*	GetpVTtags() {return &m_vt_tags;}

// Operations
public:
	int		CheckFileType(CFile* file) const;
	int 	CheckFileTypeFromName(CString& sz_path_name);
	int  	BGetVal(int chan, long l_index);
	short* 	LoadTransfData (long l_first, long l_last, int transform_type, int source_channel);
	BOOL	BuildTransfData(int transform_type, int ns);
	BOOL	LoadRawData(long* l_first, long* l_last, int span /*, BOOL bImposedReading*/);
	short*	LoadRawDataParams(int* n_channels);

	// write data
	BOOL	WriteHZtags(CTagList* ptags);
	BOOL	WriteVTtags(CTagList* ptags);
	BOOL	AcqDoc_DataAppendStart();
	BOOL	AcqDoc_DataAppend(short* pBU, UINT uilbytesLength) const;
	BOOL	AcqDoc_DataAppendStop();
	void	AcqDeleteFile() const;
	void	AcqCloseFile() const;
	BOOL	AcqSaveDataDescriptors();

	// AwaveFile operations -- could add parameter to create other type
	BOOL	CreateAcqFile(CString &csFileName);
	BOOL	SaveAs(CString &sznewName, BOOL bCheckOverWrite=TRUE, int iType=0);

// helpers
	long	GetDOCchanLength() const				{return m_lDOCchanLength;}
	long	GetDOCchanIndex (int kd) const			{return (m_lBUFchanFirst + kd -1);}
	long	GetDOCchanIndexFirst() const			{return m_lBUFchanFirst;}
	long	GetBUFchanIndex (long lPos) const		{return (lPos-m_lBUFchanFirst);}
	long	GetBUFchanLength() const				{return m_lBUFchanSize;}

	CWaveChanArray*	GetpWavechanArray() const {return &m_pWBuf->m_chanArray;}
	CWaveFormat*		GetpWaveFormat() const {return &m_pWBuf->m_waveFormat;}
	int		GetScanCount() const {return m_pWBuf->m_waveFormat.scan_count;}
	
	short*	GetpRawDataBUF() const {return m_pWBuf->GetWBAdrRawDataBuf();}
	short*	GetpRawDataElmt(int chan, int index) const {return m_pWBuf->GetWBAdrRawDataElmt(chan, index-m_lBUFchanFirst);}
	short*	GetpTransfDataBUF() const {return m_pWBuf->GetWBAdrTransfData();}
	short*	GetpTransfDataElmt(int index) const {return (m_pWBuf->GetWBAdrTransfData() + index);}
	int		GetTransfDataSpan(int i) const			{return m_pWBuf->GetWBTransformSpan(i);}
	WORD	GetTransfDataNTypes() const				{return m_pWBuf->GetWBNTypesofTransforms();}
	CString	GetTransfDataName(int i) const			{return m_pWBuf->GetWBTransformsAllowed(i);}
	BOOL	GetWBVoltsperBin(int chindex, float* VoltsperBin, int mode = 0) const {return m_pWBuf->GetWBVoltsperBin(chindex, VoltsperBin, mode);}
	BOOL	IsWBTransformAllowed(int imode) const	{return m_pWBuf->IsWBTransformAllowed(imode);}
	BOOL	InitWBTransformBuffer() const	{return m_pWBuf->InitWBTransformBuffer();}
	BOOL	SetWBTransformSpan(int i, int ispan) const {return m_pWBuf->SetWBTransformSpan(i, ispan);}
	int		IsWBSpanChangeAllowed(int i) const		{return m_pWBuf->IsWBSpanChangeAllowed(i);}

	void	SetbOffsetToData(ULONGLONG ulOffset) const {m_pXFile->m_ulOffsetData=ulOffset;}
	void	SetbOffsetToHeader(ULONGLONG ulOffset) const {m_pXFile->m_ulOffsetHeader=ulOffset;}

	ULONGLONG GetbOffsetToData() const				{return m_pXFile->m_ulOffsetData;}
	ULONGLONG GetbOffsetToHeader() const				{return m_pXFile->m_ulOffsetHeader;}
	int		GetbHeaderSize() const					{return m_pXFile->m_bHeaderSize;}

	 void	SetReadingBufferDirty()					{m_bValidReadBuffer = FALSE;}

	BOOL	AllocBUF();
	BOOL	AdjustBUF(int iNumElements);
	void	ReadDataInfos();

protected:
	long	m_lDOCchanLength;	// n words in file / channels
	BOOL	ReadDataBlock(long l_first);
};


