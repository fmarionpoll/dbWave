#pragma once

#include "dbTableMain.h"
#include "dbTable.h"
#include "Spikelist.h"
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
	CIntervals m_stimsaved;

	// database
	CdbTable* m_pDB = nullptr;
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
	void	ImportFileList(CStringArray& fileList, int nColumns = 1, boolean bHeader = false);
	BOOL	IsExtensionRecognizedAsDataFile(CString string) const;

	BOOL	IsFilePresent(CString csFilename) {
		CFileStatus status;
		return CFile::GetStatus(csFilename, status); }

	CWaveFormat* GetWaveFormat(CString filename, BOOL bIsDatFile);
	BOOL	ExtractFilenamesFromDatabase(LPCSTR filename, CStringArray& filenames);
	BOOL	ImportDatabase(CString& filename);
	BOOL	OnNewDocument(LPCTSTR lpszPathName);
	AcqDataDoc* OpenCurrentDataFile();
	AcqDataDoc* GetCurrent_Dat_Document() const { return m_pDat; }
	void	CloseCurrentDataFile() const;
	CSpikeDoc* open_current_spike_file();
	CSpikeDoc* GetCurrent_Spk_Document() const { return m_pSpk; }

	void	Remove_DuplicateFiles();
	void	Remove_MissingFiles();
	void	Remove_FalseSpkFiles();
	void	Delete_ErasedFiles();

	long	GetDB_n_spikes() const; 
	void	SetDB_n_spikes(long n_spikes) const; 
	long	GetDB_n_spike_classes() const;
	void	SetDB_n_spike_classes(long n_classes) const;
	void	GetAllSpkMaxMin(BOOL bAll, BOOL b_recalculate, short* max, short* min);
	CSize	GetSpkMaxMin_y1(BOOL bAll);

	void	SetClearMdbOnExit(BOOL bClear) { m_bClearMdbOnExit = bClear; }

	void	Export_DataAsciiComments(CSharedFile* p_shared_file);
	void	Export_NumberOfSpikes(CSharedFile* pSF);
	CString Export_DatabaseData(int ioption = 0);
	void	Export_SpkDescriptors(CSharedFile* pSF, SpikeList* p_spike_list, int kclass);
	void	SynchronizeSourceInfos(BOOL bAll);
	BOOL	UpdateWaveFmtFromDatabase(CWaveFormat* p_wave_format) const;

	void	Export_DatafilesAsTXTfiles();
	BOOL	CopyAllFilesToDirectory(const CString& path);
	bool	BinaryFileCopy(LPCTSTR pszSource, LPCTSTR pszDest);
	BOOL	CreateDirectories(const CString& path) const;

protected:
	static numberIDToText headers[];
	BOOL	transpose_file_for_excel(CSharedFile* pSF);
	sourceData get_wave_format_from_either_file(CString cs_filename);
	void	set_record_file_names(sourceData* record);
	boolean set_record_spk_classes(sourceData* record);
	void	set_record_wave_format(sourceData* record);
	boolean import_file_single(CString& cs_filename, long& m_id, int irecord, CStringArray& csArray, int nColumns,
	                         boolean bHeader);
	int		check_files_can_be_opened(CStringArray& filenames, CSharedFile* psf, int nColumns, boolean bHeader);

	int		index_2d_array(int iRow, int nColumns, boolean bHeader) {
		return (iRow + (bHeader ? 1 : 0)) * nColumns; };
	int		get_size_2d_array(CStringArray& csArray, int nColumns, boolean bHeader) {
		return csArray.GetSize() / nColumns - (bHeader ? 1 : 0); }
	void	remove_row_at(CStringArray& filenames, int iRow, int nColumns, boolean bHeader);
	CSharedFile* file_discarded_message(CSharedFile* pSF, CString cs_filename, int irec);
	void	getInfosFromStringArray(sourceData* pRecord, CStringArray& filenames, int irecord, int nColumns,
	                             boolean bHeader);
	int		find_header(CString& text);
	void	remove_file_from_disk(CString file_name);

	// Overrides
public:
	void	Serialize(CArchive& ar) override;

	BOOL	OnNewDocument() override;
	BOOL	OnOpenDocument(LPCTSTR lpszPathName) override;
	BOOL	OnSaveDocument(LPCTSTR lpszPathName) override;
	HMENU	GetDefaultMenu() override; // get menu depending on state
	void	UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint);

	// Implementation
public:
	~CdbWaveDoc() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	// DAO database functions
public:
	long	GetDB_NRecords() const { return m_pDB->GetNRecords(); }
	CdbTableMain* GetDB_Recordset() const { return &m_pDB->m_mainTableSet; }
	void	DBDeleteCurrentRecord();
	CString GetDB_CurrentDatFileName(BOOL bTest = FALSE);
	CString GetDB_CurrentSpkFileName(BOOL bTest = FALSE);
	CString SetDB_CurrentSpikeFileName();
	void	SetDB_DataLen(long len) const { m_pDB->SetDataLen(len); }
	long	GetDB_DataLen();
	void	SetDB_CurrentRecordFlag(int flag) const;
	int		GetDB_CurrentRecordFlag() const { return m_pDB->m_mainTableSet.m_flag; }
	void	SetDB_PathsRelative() const;
	void	SetDB_PathsAbsolute() const;
	void	DBTransferDatPathToSpkPath() const;
	void	DBDeleteUnusedEntries() const;

	long	GetDB_CurrentRecordPosition() const;
	long	GetDB_CurrentRecordID() const;
	BOOL	set_db_current_record_position(long i_file) { return m_pDB->SetIndexCurrentFile(i_file); }
	BOOL	DBMoveToID(long record_id) { return m_pDB->MoveToID(record_id); }
	BOOL	DBMoveFirst() { return m_pDB->MoveRecord(ID_RECORD_FIRST); }
	BOOL	DBMoveNext() { return m_pDB->MoveRecord(ID_RECORD_NEXT); }
	BOOL	DBMovePrev() { return m_pDB->MoveRecord(ID_RECORD_PREV); }
	BOOL	DBMoveLast() { return m_pDB->MoveRecord(ID_RECORD_LAST); }
	void	 DBRefreshQuery() const {
		if (m_pDB->m_mainTableSet.IsBOF()) m_pDB->m_mainTableSet.SetFieldNull(nullptr);
		m_pDB->m_mainTableSet.RefreshQuery();
	}

protected:
	BOOL	OpenDatabase(LPCTSTR lpszPathName);

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
