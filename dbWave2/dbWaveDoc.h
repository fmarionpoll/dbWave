#pragma once

#include "dbMainTable.h"
#include "dbWdatabase.h"
#include "Spikelist.h"		// list of spikes
#include "Spikedoc.h"
#include "AcqDataDoc.h"
#include "dbWave_structures.h"

struct sourceData
{
	CString cs_dat_file{};
	CString cs_spk_file{};
	CString cs_path{};
	int i_last_backslash_position = -1;
	boolean b_dat_present = false;
	boolean b_spik_present = false;
	CWaveFormat* p_wave_format = nullptr;
};

class CdbWaveDoc : public COleDocument
{
protected: // create from serialization only
	CdbWaveDoc();
	DECLARE_DYNCREATE(CdbWaveDoc)

	// Attributes
public:
	AcqDataDoc* m_pDat = nullptr;
	CSpikeDoc* m_pSpk = nullptr;
	HMENU m_hMyMenu = nullptr;
	CIntervalsAndLevels m_stimsaved;

	// database
	CdbWdatabase* m_pDB = nullptr;
	BOOL m_validTables = false;
	CDWordArray m_selectedRecords;
	CString m_dbFilename;
	CString m_ProposedDataPathName;

protected:
	CString m_currentDatafileName;
	CString m_currentSpikefileName;

	BOOL m_bcallnew = true;
	CStringArray m_csfiles_to_delete;
	BOOL m_bClearMdbOnExit = false;
	BOOL m_bTranspose = false;

	// Operations
public:
	void ImportFileList(CStringArray& fileList, int nColumns = 1, boolean bHeader = false);
	BOOL IsExtensionRecognizedAsDataFile(CString string) const;

	BOOL IsFilePresent(CString csFilename)
	{
		CFileStatus status;
		return CFile::GetStatus(csFilename, status);
	}

	CWaveFormat* GetWaveFormat(CString filename, BOOL bIsDatFile);
	BOOL ExtractFilenamesFromDatabase(LPCSTR filename, CStringArray& filenames);
	BOOL ImportDatabase(CString& filename);
	BOOL OnNewDocument(LPCTSTR lpszPathName);
	AcqDataDoc* OpenCurrentDataFile();
	AcqDataDoc* GetCurrent_Dat_Document() const { return m_pDat; }
	void CloseCurrentDataFile();
	CSpikeDoc* OpenCurrentSpikeFile();
	CSpikeDoc* GetCurrent_Spk_Document() const { return m_pSpk; }

	void Remove_DuplicateFiles();
	void Remove_MissingFiles();
	void Remove_FalseSpkFiles();
	void Delete_ErasedFiles();

	long GetDB_nbspikes(); 
	void SetDB_nbspikes(long nspikes); 
	long GetDB_nbspikeclasses();
	void SetDB_nbspikeclasses(long nclasses);
	void GetAllSpkMaxMin(BOOL bAll, BOOL bRecalc, int* max, int* min);
	CSize GetSpkMaxMin_y1(BOOL bAll);

	void SetClearMdbOnExit(BOOL bClear) { m_bClearMdbOnExit = bClear; }

	void Export_DataAsciiComments(CSharedFile* p_shared_file);
	void Export_NumberOfSpikes(CSharedFile* pSF);
	CString Export_DatabaseData(int ioption = 0);
	void Export_SpkDescriptors(CSharedFile* pSF, CSpikeList* p_spike_list, int kclass);
	void SynchronizeSourceInfos(BOOL bAll);
	BOOL UpdateWaveFmtFromDatabase(CWaveFormat* p_wave_format) const;

	void Export_DatafilesAsTXTfiles();
	BOOL CopyAllFilesintoDirectory(const CString& path);
	bool BinaryFileCopy(LPCTSTR pszSource, LPCTSTR pszDest);
	BOOL CreateDirectories(CString path);

protected:
	static numberIDToText headers[];
	BOOL transposeFileForExcel(CSharedFile* pSF);
	sourceData getWaveFormatFromEitherFile(CString cs_filename);
	void setRecordFileNames(sourceData* record);
	boolean setRecordSpkClasses(sourceData* record);
	void setRecordWaveFormat(sourceData* record);
	boolean importFileSingle(CString& cs_filename, long& m_id, int irecord, CStringArray& csArray, int nColumns,
	                         boolean bHeader);
	int checkFilesCanbeOpened(CStringArray& filenames, CSharedFile* psf, int nColumns, boolean bHeader);

	int index2DArray(int iRow, int nColumns, boolean bHeader)
	{
		return (iRow + (bHeader ? 1 : 0)) * nColumns;
	};

	int getSize2DArray(CStringArray& csArray, int nColumns, boolean bHeader)
	{
		return csArray.GetSize() / nColumns - (bHeader ? 1 : 0);
	}

	void removeRowAt(CStringArray& filenames, int iRow, int nColumns, boolean bHeader);
	CSharedFile* fileDiscardedMessage(CSharedFile* pSF, CString cs_filename, int irec);
	void getInfosFromStringArray(sourceData* pRecord, CStringArray& filenames, int irecord, int nColumns,
	                             boolean bHeader);
	int findHeader(CString& text);
	void remove_file_from_disk(CString file_name);

	// Overrides
public:
	void Serialize(CArchive& ar) override;

	BOOL OnNewDocument() override;
	BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
	BOOL OnSaveDocument(LPCTSTR lpszPathName) override;
	HMENU GetDefaultMenu() override; // get menu depending on state
	void UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint);

	// Implementation
public:
	~CdbWaveDoc() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	// DAO database functions
public:
	long GetDB_NRecords() { return m_pDB->GetNRecords(); }
	CDaoRecordset* GetDB_Recordset() { return &m_pDB->m_mainTableSet; }

	void DBRefreshQuery()
	{
		if (m_pDB->m_mainTableSet.IsBOF()) m_pDB->m_mainTableSet.SetFieldNull(nullptr);
		m_pDB->m_mainTableSet.RefreshQuery();
	}

	void DBDeleteCurrentRecord();
	CString GetDB_CurrentDatFileName(BOOL bTest = FALSE);
	CString GetDB_CurrentSpkFileName(BOOL bTest = FALSE);
	CString SetDB_CurrentSpikeFileName();
	void SetDB_DataLen(long len) { m_pDB->SetDataLen(len); }
	long GetDB_DataLen();
	void SetDB_CurrentRecordFlag(int flag);
	int GetDB_CurrentRecordFlag() { return m_pDB->m_mainTableSet.m_flag; }
	void SetDB_PathsRelative_to_DataBaseFile();
	void SetDB_PathsAbsolute();
	void DBTransferDatPathToSpkPath();
	void DBDeleteUnusedEntries();

	long GetDB_CurrentRecordPosition();
	long GetDB_CurrentRecordID();
	BOOL SetDB_CurrentRecordPosition(long ifile) { return m_pDB->SetIndexCurrentFile(ifile); }
	BOOL DBMoveToID(long recordID) { return m_pDB->MoveToID(recordID); }
	BOOL DBMoveFirst() { return m_pDB->MoveRecord(ID_RECORD_FIRST); }
	BOOL DBMoveNext() { return m_pDB->MoveRecord(ID_RECORD_NEXT); }
	BOOL DBMovePrev() { return m_pDB->MoveRecord(ID_RECORD_PREV); }
	BOOL DBMoveLast() { return m_pDB->MoveRecord(ID_RECORD_LAST); }

protected:
	BOOL OpenDatabase(LPCTSTR lpszPathName);

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
