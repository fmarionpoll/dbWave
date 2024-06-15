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
	boolean data_file_present = false;
	boolean spike_file_present = false;
	CWaveFormat* p_wave_format = nullptr;
};

class CdbWaveDoc : public COleDocument
{
protected:
	CdbWaveDoc();
	DECLARE_DYNCREATE(CdbWaveDoc)

protected:
	CString		m_current_datafile_name_;
	CString		m_current_spikefile_name_;

	BOOL		m_bcallnew = true;
	CStringArray m_names_of_files_to_delete;
	BOOL		m_clean_database_on_exit_ = false;
	BOOL		m_bTranspose = false;

public:
	AcqDataDoc* m_pDat = nullptr;
	CSpikeDoc* m_pSpk = nullptr;
	HMENU		m_hMyMenu = nullptr;
	CIntervals	m_stimsaved;

	CdbTable*	m_pDB = nullptr;
	BOOL		m_validTables = false;
	CDWordArray m_selectedRecords;
	CString		m_dbFilename;
	CString		m_ProposedDataPathName;

	void	import_file_list(CStringArray& fileList, int n_columns = 1, boolean bHeader = false);
	BOOL	import_database(CString& filename);

	BOOL	is_extension_recognized_as_data_file(CString string) const;
	BOOL	is_file_present(CString csFilename) {
		CFileStatus status;
		return CFile::GetStatus(csFilename, status);
	}

	CWaveFormat* get_wave_format(CString filename, BOOL bIsDatFile);

	BOOL	OnNewDocument(LPCTSTR lpszPathName);
	AcqDataDoc* open_current_data_file();
	AcqDataDoc* get_current_dat_document() const { return m_pDat; }
	void	close_current_data_file() const;
	CSpikeDoc* open_current_spike_file();
	CSpikeDoc* get_current_spike_file() const { return m_pSpk; }

	void	remove_duplicate_files();
	void	remove_missing_files();
	void	remove_false_spk_files();
	void	delete_erased_files();

	long	get_db_n_spikes() const;
	void	set_db_n_spikes(long n_spikes) const;
	long	get_db_n_spike_classes() const;
	void	set_db_n_spike_classes(long n_classes) const;
	void	get_max_min_of_all_spikes(BOOL b_all_files, BOOL b_recalculate, short* max, short* min);
	CSize	get_max_min_of_single_spike(BOOL bAll);

	void	set_clean_db_on_exit(BOOL bClear) { m_clean_database_on_exit_ = bClear; }

	void	export_data_ascii_comments(CSharedFile* p_shared_file);
	void	export_number_of_spikes(CSharedFile* pSF);
	CString export_database_data(int option = 0) const;
	void	export_spk_descriptors(CSharedFile* pSF, SpikeList* p_spike_list, int kclass);
	void	export_datafiles_as_text_files();

	void	synchronize_source_infos(const BOOL b_all);
	BOOL	update_waveformat_from_database(CWaveFormat* p_wave_format) const;
	BOOL	import_data_files_from_another_data_base(const CString& otherDataBaseFileName) const;
	BOOL	copy_files_to_directory(const CString& path);

protected:
	static numberIDToText headers[];

	BOOL	transpose_file_for_excel(CSharedFile* pSF);
	sourceData get_wave_format_from_either_file(CString cs_filename);
	void	set_record_file_names(sourceData* record);
	boolean set_record_spk_classes(sourceData* record);
	void	set_record_wave_format(sourceData* record);
	boolean import_file_single(CString& cs_filename, long& m_id, int irecord, CStringArray& csArray, int nColumns,
		boolean bHeader);
	int		check_files_can_be_opened(CStringArray& file_names_array, CSharedFile* psf, int nColumns, boolean bHeader);

	static int index_2d_array(int iRow, int nColumns, boolean bHeader) { return (iRow + (bHeader ? 1 : 0)) * nColumns; }

	static int get_size_2d_array(const CStringArray& cs_array, int nColumns, boolean bHeader) { return cs_array.GetSize() / nColumns - (bHeader ? 1 : 0); }
	void	remove_row_at(CStringArray& file_name_array, int iRow, int nColumns, boolean bHeader);
	CSharedFile* file_discarded_message(CSharedFile* pSF, CString cs_filename, int irec);
	void	get_infos_from_string_array(const sourceData* pRecord, const CStringArray& file_names_array, int const irecord, int nColumns, boolean bHeader);
	int		find_column_associated_to_header(const CString& text);
	void	remove_file_from_disk(CString file_name);
	CString get_full_path_name_without_extension() const;
	static CString get_path_directory(const CString& full_name);
	boolean	create_directory_if_does_not_exists(const CString& path) const;

	bool	binary_file_copy(LPCTSTR pszSource, LPCTSTR pszDest) const;
	void	copy_files_to_directory(CStringArray& files_to_copy_array, CString mdb_directory) const;
	CString copy_file_to_directory(const LPCTSTR pszSource, const CString& directory) const;
	bool is_file_present(const CString& cs_new_name) const;
	boolean	file_exists(const CString& file_name) const;

	// Overrides
public:
	void	Serialize(CArchive& ar) override;
	BOOL	OnNewDocument() override;
	BOOL	OnOpenDocument(LPCTSTR lpszPathName) override;
	BOOL	OnSaveDocument(LPCTSTR lpszPathName) override;
	HMENU	GetDefaultMenu() override; // get menu depending on state
	void	update_all_views_db_wave(CView* pSender, LPARAM lHint, CObject* pHint);

	~CdbWaveDoc() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	static CdbWaveDoc* get_active_mdi_document();

	// DAO database functions
	long	db_get_n_records() const { return m_pDB->GetNRecords(); }
	CdbTableMain* db_get_recordset() const { return &m_pDB->m_mainTableSet; }
	void	db_delete_current_record();
	CString db_get_current_dat_file_name(BOOL b_test = FALSE);
	CString db_get_current_spk_file_name(BOOL b_test = FALSE);
	CString db_set_current_spike_file_name();
	void	db_set_data_len(long len) const { m_pDB->SetDataLength(len); }
	long	db_get_data_len();
	void	db_set_current_record_flag(int flag) const;
	int		db_get_current_record_flag() const { return m_pDB->m_mainTableSet.m_flag; }
	void	db_set_paths_relative() const;
	void	db_set_paths_absolute() const;
	void	db_transfer_dat_path_to_spk_path() const;
	void	db_delete_unused_entries() const;

	long	db_get_current_record_position() const;
	long	db_get_current_record_id() const;
	BOOL	db_set_current_record_position(long i_file) const { return m_pDB->SetIndexCurrentFile(i_file); }
	BOOL	db_move_to_id(long record_id) const { return m_pDB->MoveToID(record_id); }
	BOOL	db_move_first() const { return m_pDB->MoveTo(ID_RECORD_FIRST); }
	BOOL	db_move_next() const { return m_pDB->MoveTo(ID_RECORD_NEXT); }
	BOOL	db_move_prev() const { return m_pDB->MoveTo(ID_RECORD_PREV); }
	BOOL	db_move_last() const { return m_pDB->MoveTo(ID_RECORD_LAST); }
	void	db_refresh_query() const {
		if (m_pDB->m_mainTableSet.IsBOF()) m_pDB->m_mainTableSet.SetFieldNull(nullptr);
		m_pDB->m_mainTableSet.RefreshQuery();
	}

protected:
	BOOL	open_database(LPCTSTR lpszPathName);

	DECLARE_MESSAGE_MAP()
};
