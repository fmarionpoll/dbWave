#pragma once

// sort options :
auto constexpr BY_TIME = 1;
auto constexpr BY_NAME = 2;
auto constexpr MAX_BUFFER_LENGTH_AS_BYTES = 614400;

//  1 s at 10 kHz =  1(s) * 10 000 (data points) * 2 (bytes) * 3 (chans) = 60 000
// 10 s at 10 kHz = 10(s) * 10 000 (data points) * 2 (bytes) * 3 (chans) = 600 000
// with a multiple of 1024 =  614400

#include "WaveBuf.h"
#include "datafile_X.h"
#include "OPTIONS_VIEWDATA.h"

class AcqDataDoc : public CDocument
{
	friend class CDlgImportGenericData;
	DECLARE_DYNCREATE(AcqDataDoc)
public:
	AcqDataDoc();
	~AcqDataDoc() override;

public:
	CString GetDataFileInfos(OPTIONS_VIEWDATA* pVD);
	void ExportDataFile_to_TXTFile(CStdioFile* pdataDest);
	BOOL OnSaveDocument(CString& sz_path_name);
	BOOL OnOpenDocument(CString& sz_path_name);
	BOOL OnNewDocument() override;

	BOOL openAcqFile(CString& szfilename);
protected:
	bool dlgImportDataFile(CString& sz_path_name);
	int importFile(CString& sz_path_name);
	void removeFile(CString file1);
	void renameFile2As1(CString filename_old, CString sz_path_name);

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

public:
	// Data members and functions dealing with CDataFileX and data reading buffer

	CDataFileX* m_pXFile = nullptr;
	int m_pXFileType = DOCTYPE_UNKNOWN;
	int m_lastDocumentType = DOCTYPE_UNKNOWN;
	int m_DOCnbchans = 0; // nb channels / doc
	int m_iOffsetInt = sizeof(short); // offset in bytes
	long m_lDOCchanLength = 0;

	CWaveBuf* m_pWBuf = nullptr;
	long m_lBUFmaxSize = MAX_BUFFER_LENGTH_AS_BYTES; // constant (?) size of the buffer
	long m_lBUFSize = 0; // buffer size (n channels * sizeof(word) * lRWSize
	long m_lBUFchanSize = 0; // n words in buffer / channel
	long m_lBUFchanFirst = 0; // file index of first word in RW buffer
	long m_lBUFchanLast = 0; // file index of last word in RW buffer
	BOOL m_bValidReadBuffer = false; // flag / valid data ; FALSE to force reading data from file
	BOOL m_bValidTransfBuffer = false; // flag to tell that transform buffer is valid (check if really used?)
	BOOL m_bdefined = false;
	BOOL m_bRemoveOffset = true; // transform data read in short values if binary offset encoding

	// use with caution - parameters set by last call to LoadTransfData
	long m_tBUFfirst = 0;
	long m_tBUFlast = 1;
	int m_tBUFtransform = 0;
	int m_tBUFsourcechan = 0;

public:
	long GettBUFfirst() const { return m_tBUFfirst; }
	short BGetVal(int chan, long l_index);
	short* LoadTransfData(long l_first, long l_last, int transform_type, int source_channel);
	BOOL BuildTransfData(int transform_type, int ns);
	BOOL LoadRawData(long* l_first, long* l_last, int span /*, BOOL bImposedReading*/);
	short* LoadRawDataParams(int* n_channels) const;

	// write data
	BOOL WriteHZtags(TagList* ptags);
	BOOL WriteVTtags(TagList* ptags);
	BOOL AcqDoc_DataAppendStart();
	BOOL AcqDoc_DataAppend(short* pBU, UINT uilbytesLength) const;
	BOOL AcqDoc_DataAppendStop();
	void AcqDeleteFile() const;
	void AcqCloseFile() const;
	BOOL AcqSaveDataDescriptors();

	// AwaveFile operations -- could add parameter to create other type
	BOOL CreateAcqFile(CString& csFileName);
	BOOL SaveAs(CString& sznewName, BOOL bCheckOverWrite = TRUE, int iType = 0);

	// helpers
	long GetDOCchanLength() const { return m_lDOCchanLength; }
	long GetDOCchanIndex(int kd) const { return (m_lBUFchanFirst + kd - 1); }
	long GetDOCchanIndexFirst() const { return m_lBUFchanFirst; }
	long GetBUFchanIndex(long lPos) const { return (lPos - m_lBUFchanFirst); }
	long GetBUFchanLength() const { return m_lBUFchanSize; }

	CWaveChanArray* GetpWavechanArray() const { return &m_pWBuf->m_chanArray; }
	CWaveFormat* GetpWaveFormat() const { return &m_pWBuf->m_waveFormat; }
	TagList* GetpHZtags() { return m_pWBuf->GetpHZtags(); }
	TagList* GetpVTtags() { return m_pWBuf->GetpVTtags(); }

	int GetScanCount() const { return m_pWBuf->m_waveFormat.scan_count; }

	short* GetpRawDataBUF() const { return m_pWBuf->getWBAdrRawDataBuf(); }

	short* GetpRawDataElmt(int chan, int index) const
	{
		return m_pWBuf->getWBAdrRawDataElmt(chan, index - m_lBUFchanFirst);
	}

	short* GetpTransfDataBUF() const { return m_pWBuf->getWBAdrTransfData(); }
	short* GetpTransfDataElmt(int index) const { return (m_pWBuf->getWBAdrTransfData() + index); }
	int GetTransfDataSpan(int i) const { return m_pWBuf->GetWBTransformSpan(i); }
	WORD GetTransfDataNTypes() const { return m_pWBuf->GetWBNTypesofTransforms(); }
	CString GetTransfDataName(int i) const { return m_pWBuf->GetWBTransformsAllowed(i); }

	BOOL GetWBVoltsperBin(int chindex, float* VoltsperBin, int mode = 0) const
	{
		return m_pWBuf->GetWBVoltsperBin(chindex, VoltsperBin, mode);
	}

	BOOL IsWBTransformAllowed(int imode) const { return m_pWBuf->IsWBTransformAllowed(imode); }
	BOOL InitWBTransformBuffer() const { return m_pWBuf->InitWBTransformBuffer(); }
	BOOL SetWBTransformSpan(int i, int ispan) const { return m_pWBuf->SetWBTransformSpan(i, ispan); }
	int IsWBSpanChangeAllowed(int i) const { return m_pWBuf->IsWBSpanChangeAllowed(i); }

	void SetbOffsetToData(ULONGLONG ulOffset) const { m_pXFile->m_ulOffsetData = ulOffset; }
	void SetbOffsetToHeader(ULONGLONG ulOffset) const { m_pXFile->m_ulOffsetHeader = ulOffset; }

	ULONGLONG GetbOffsetToData() const { return m_pXFile->m_ulOffsetData; }
	ULONGLONG GetbOffsetToHeader() const { return m_pXFile->m_ulOffsetHeader; }
	int GetbHeaderSize() const { return m_pXFile->m_bHeaderSize; }

	void SetReadingBufferDirty() { m_bValidReadBuffer = FALSE; }

	BOOL AllocBUF();
	BOOL AdjustBUF(int iNumElements);
	void ReadDataInfos();

protected:
	BOOL readDataBlock(long l_first);
	void instanciateDataFileObject(int docType);

	DECLARE_MESSAGE_MAP()
};
