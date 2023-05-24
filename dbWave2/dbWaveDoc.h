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
protected: 
	CdbWaveDoc();
	DECLARE_DYNCREATE(CdbWaveDoc)

protected:
	CString		m_currentDatafileName;
	CString		m_currentSpikefileName;

	BOOL		m_bcallnew = true;
	CStringArray m_csfiles_to_delete;
	BOOL		m_bClearMdbOnExit = false;
	BOOL		m_bTranspose = false;

public:
	AcqDataDoc* m_pDat = nullptr;
	CSpikeDoc*	m_pSpk = nullptr;
	HMENU		m_hMyMenu = nullptr;
	CIntervals	m_stimsaved;

	CdbTable*	m_pDB = nullptr;
	BOOL		m_validTables = false;
	CDWordArray m_selectedRecords;
	CString		m_dbFilename;
	CString		m_ProposedDataPathName;

	void	Import_FileList(CStringArray& fileList, int n_columns = 1, boolean bHeader = false);
	BOOL	Import_Database(CString& filename);

	BOOL	IsExtensionRecognizedAsDataFile(CString string) const;
	BOOL	IsFilePresent(CString csFilename) {
		CFileStatus status;
		return CFile::GetStatus(csFilename, status); }

	CWaveFormat* GetWaveFormat(CString filename, BOOL bIsDatFile);

	BOOL	OnNewDocument(LPCTSTR lpszPathName);
	AcqDataDoc* OpenCurrentDataFile();
	AcqDataDoc* GetCurrent_Dat_Document() const { return m_pDat; }
	void	CloseCurrentDataFile() const;
	CSpikeDoc* Open_Current_Spike_File();
	CSpikeDoc* Get_Current_Spike_File() const { return m_pSpk; }

	void	Remove_DuplicateFiles();
	void	Remove_MissingFiles();
	void	Remove_FalseSpkFiles();
	void	Delete_ErasedFiles();

	long	GetDB_n_spikes() const; 
	void	SetDB_n_spikes(long n_spikes) const; 
	long	GetDB_n_spike_classes() const;
	void	SetDB_n_spike_classes(long n_classes) const;
	void	Get_MaxMin_Of_All_Spikes(BOOL b_all_files, BOOL b_recalculate, short* max, short* min);
	CSize	Get_MaxMin_Of_Single_Spike(BOOL bAll);

	void	SetClearMdbOnExit(BOOL bClear) { m_bClearMdbOnExit = bClear; }

	void	Export_DataAsciiComments(CSharedFile* p_shared_file);
	void	Export_NumberOfSpikes(CSharedFile* pSF);
	CString Export_DatabaseData(int option = 0) const;
	void	Export_SpkDescriptors(CSharedFile* pSF, SpikeList* p_spike_list, int kclass);
	void	Export_DatafilesAsTXTfiles();

	void	SynchronizeSourceInfos(BOOL bAll);
	BOOL	UpdateWaveFmtFromDatabase(CWaveFormat* p_wave_format) const;
	BOOL	Import_Data_Files_From_Another_DataBase(const CString& otherDataBaseFileName) const;
	int		import_records_from_another_data_base(const CString& otherDataBaseFileName, CStringArray& file_list_dat,
	                                          CStringArray& file_list_spk) const;
	BOOL	Copy_Files_To_Directory(const CString& path);

protected:
	static numberIDToText headers[];

	BOOL	transpose_file_for_excel(CSharedFile* pSF);
	sourceData get_wave_format_from_either_file(CString cs_filename);
	void	set_record_file_names(const sourceData* record) const;
	boolean set_record_spk_classes(sourceData* record);
	void	set_record_wave_format(sourceData* record);
	boolean import_file_single(CString& cs_filename, long& m_id, int irecord, CStringArray& csArray, int nColumns,
	                         boolean bHeader);
	int		check_files_can_be_opened(CStringArray& file_names_array, CSharedFile* psf, int nColumns, boolean bHeader);

	static int index_2d_array(int iRow, int nColumns, boolean bHeader) {return (iRow + (bHeader ? 1 : 0)) * nColumns; }

	static int get_size_2d_array(const CStringArray& cs_array, int nColumns, boolean bHeader) {return cs_array.GetSize() / nColumns - (bHeader ? 1 : 0); }
	void	remove_row_at(CStringArray& file_name_array, int iRow, int nColumns, boolean bHeader);
	CSharedFile* file_discarded_message(CSharedFile* pSF, CString cs_filename, int irec);
	void	getInfosFromStringArray(const sourceData* pRecord, const CStringArray& file_names_array, int i_record, int nColumns, boolean bHeader);
	int		find_column_associated_to_header(const CString& text);
	void	remove_file_from_disk(CString file_name);
	CString get_full_path_name_without_extension() const;
	static CString get_path_directory(const CString& full_name);
	CString get_destination_name(const CString& source_file, const CString& directory) const;
	boolean	create_directory_if_does_not_exists(const CString& path) const;
	
	bool	binary_file_copy(LPCTSTR pszSource, LPCTSTR pszDest) const;
	void	copy_files_to_directory(CStringArray& files_to_copy, const CString& mdb_directory) const;
	boolean is_file_present(const CString& cs_new_name) const;
	
	// Overrides
public:
	void	Serialize(CArchive& ar) override;
	BOOL	OnNewDocument() override;
	BOOL	OnOpenDocument(LPCTSTR lpszPathName) override;
	BOOL	OnSaveDocument(LPCTSTR lpszPathName) override;
	HMENU	GetDefaultMenu() override; // get menu depending on state
	void	UpdateAllViews_dbWave(CView* pSender, LPARAM lHint, CObject* pHint);

	~CdbWaveDoc() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	static CdbWaveDoc* get_active_mdi_document();

	// DAO database functions
	long	DB_GetNRecords() const { return m_pDB->GetNRecords(); }
	CdbTableMain* DB_GetRecordset() const { return &m_pDB->m_mainTableSet; }
	void	DB_DeleteCurrentRecord();
	CString DB_GetCurrentDatFileName(BOOL bTest = FALSE);
	CString DB_GetCurrentSpkFileName(BOOL bTest = FALSE);
	CString DB_SetCurrentSpikeFileName();
	void	DB_SetDataLen(long len) const { m_pDB->SetDataLength(len); }
	long	DB_GetDataLen();
	void	DB_SetCurrentRecordFlag(int flag) const;
	int		DB_GetCurrentRecordFlag() const { return m_pDB->m_mainTableSet.m_flag; }
	void	DB_SetPathsRelative() const;
	void	DB_SetPathsAbsolute() const;
	void	DBTransferDatPathToSpkPath() const;
	void	DBDeleteUnusedEntries() const;

	long	DB_GetCurrentRecordPosition() const;
	long	DB_GetCurrentRecordID() const;
	BOOL	DB_SetCurrentRecordPosition(long i_file) { return m_pDB->SetIndexCurrentFile(i_file); }
	BOOL	DB_MoveToID(long record_id) { return m_pDB->MoveToID(record_id); }
	BOOL	DB_MoveFirst() { return m_pDB->MoveTo(ID_RECORD_FIRST); }
	BOOL	DB_MoveNext() { return m_pDB->MoveTo(ID_RECORD_NEXT); }
	BOOL	DB_MovePrev() { return m_pDB->MoveTo(ID_RECORD_PREV); }
	BOOL	DBMoveLast() { return m_pDB->MoveTo(ID_RECORD_LAST); }
	void	DB_RefreshQuery() const { if (m_pDB->m_mainTableSet.IsBOF()) m_pDB->m_mainTableSet.SetFieldNull(nullptr);
								m_pDB->m_mainTableSet.RefreshQuery(); }

protected:
	BOOL	OpenDatabase(LPCTSTR lpszPathName);

	DECLARE_MESSAGE_MAP()
};
