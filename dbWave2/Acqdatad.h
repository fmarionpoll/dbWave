// acqdatad.h : header file
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Declaration of constants

// sort options :
#define		BY_TIME		1
#define		BY_NAME		2

/////////////////////////////////////////////////////////////////////////////
// CAcqDataDoc document

#include "dataheader_Atlab.h"
#include "wavebuf.h"		// data acquisition buffer
#include "datafile_X.h"
#include "awavepar.h"		// user parameters
#include "subfileitem.h"
#include "datafile_Awave.h"	// awave data acquisition file

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
	BOOL 			OnSaveDocument (CString &szPathName);
	BOOL 			OnOpenDocument (CString &szPathName);
	BOOL 			OnNewDocument();

protected:
	BOOL			OpenAcqFile(CString &szfilename);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
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
	CTagList		m_HZtags;			// list of horizontal cursors
	CTagList		m_VTtags;			// list of vertical tags
	
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
	inline CTagList*	GetpHZtags() {return &m_HZtags;}
	inline CTagList*	GetpVTtags() {return &m_VTtags;}

// Operations
public:
	int				CheckFileType(CFile* file);
	int 			CheckFileTypeFromName(CString& szPathName);
	int  			BGetVal(int chan, long lIndex);
	short* 			LoadTransfData (long lFirst, long lLast, int transformType, int zbsourcechan);
	BOOL			BuildTransfData(int transformType, int ns);
	BOOL			LoadRawData(long* lFirst, long* lLast, int nspan /*, BOOL bImposedReading*/);
	short*			LoadRawDataParams(int* nchans);

	// write data
	BOOL			WriteHZtags(CTagList* ptags);
	BOOL			WriteVTtags(CTagList* ptags);
	BOOL			AcqDoc_DataAppendStart();
	BOOL			AcqDoc_DataAppend(short* pBU, UINT uilbytesLength);
	BOOL			AcqDoc_DataAppendStop();
	void			AcqDeleteFile();
	void			AcqCloseFile();
	BOOL			AcqSaveDataDescriptors();

	// AwaveFile operations -- could add parameter to create other type
	BOOL			CreateAcqFile(CString &csFileName);
	BOOL			SaveAs(CString &sznewName, BOOL bCheckOverWrite=TRUE, int iType=0);

// helpers
	inline long		GetDOCchanLength() const				{return m_lDOCchanLength;}
	inline long		GetDOCchanIndex (int kd) const			{return (m_lBUFchanFirst + kd -1);}
	inline long		GetDOCchanIndexFirst() const			{return m_lBUFchanFirst;}
	inline long		GetBUFchanIndex (long lPos) const		{return (lPos-m_lBUFchanFirst);}
	inline long		GetBUFchanLength() const				{return m_lBUFchanSize;}

	inline CWaveChanArray*	GetpWavechanArray()				{return &m_pWBuf->m_chanArray;}
	inline CWaveFormat*		GetpWaveFormat()				{return &m_pWBuf->m_waveFormat;}
	inline int		GetScanCount()							{return m_pWBuf->m_waveFormat.scan_count;}
	
	inline short*	GetpRawDataBUF()						{return m_pWBuf->GetWBAdrRawDataBuf();}
	inline short*	GetpRawDataElmt(int chan, int index)	{return m_pWBuf->GetWBAdrRawDataElmt(chan, index-m_lBUFchanFirst);}
	inline short*	GetpTransfDataBUF()						{return m_pWBuf->GetWBAdrTransfData();}
	inline short*	GetpTransfDataElmt(int index)			{return (m_pWBuf->GetWBAdrTransfData() + index);}
	inline int		GetTransfDataSpan(int i) const			{return m_pWBuf->GetWBTransformSpan(i);}
	inline WORD		GetTransfDataNTypes() const				{return m_pWBuf->GetWBNTypesofTransforms();}
	inline CString	GetTransfDataName(int i) const			{return m_pWBuf->GetWBTransformsAllowed(i);}
	inline BOOL		GetWBVoltsperBin(int zbchanIndex, float* VoltsperBin, int mode = 0)
															{return m_pWBuf->GetWBVoltsperBin(zbchanIndex, VoltsperBin, mode);}
	inline BOOL		IsWBTransformAllowed(int imode) const	{return m_pWBuf->IsWBTransformAllowed(imode);}
	inline BOOL		InitWBTransformMode(int imode) const	{return m_pWBuf->InitWBTransformMode(imode);}
	inline BOOL		SetWBTransformSpan(int i, int ispan) const {return m_pWBuf->SetWBTransformSpan(i, ispan);}
	inline int		IsWBSpanChangeAllowed(int i) const		{return m_pWBuf->IsWBSpanChangeAllowed(i);}

	inline void		SetbOffsetToData(ULONGLONG ulOffset)	{m_pXFile->m_ulOffsetData=ulOffset;}
	inline void		SetbOffsetToHeader(ULONGLONG ulOffset)	{m_pXFile->m_ulOffsetHeader=ulOffset;}

	inline ULONGLONG GetbOffsetToData() const				{return m_pXFile->m_ulOffsetData;}
	inline ULONGLONG GetbOffsetToHeader() const				{return m_pXFile->m_ulOffsetHeader;}
	inline int		GetbHeaderSize() const					{return m_pXFile->m_bHeaderSize;}

	inline  void	SetReadingBufferDirty()					{m_bValidReadBuffer = FALSE;}

	BOOL			AllocBUF();
	BOOL			AdjustBUF(int iNumElements);
	void			ReadDataInfos();

protected:
	long			m_lDOCchanLength;	// n words in file / channels
	BOOL			ReadDataBlock(long lFirst);
};


