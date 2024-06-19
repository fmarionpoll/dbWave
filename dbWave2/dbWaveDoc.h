#pragma once

#include "dbTableMain.h"
#include "dbTable.h"
#include "Spikelist.h"
#include "Spikedoc.h"
#include "AcqDataDoc.h"
#include "dbWave_structures.h"
#include "dbSpike.h"

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
	CString		m_current_spike_file_name_;

	BOOL		m_b_call_new = true;
	CStringArray m_names_of_files_to_delete;
	BOOL		m_clean_database_on_exit_ = false;
	BOOL		m_bTranspose = false;

public:
	AcqDataDoc* m_p_dat = nullptr;
	CSpikeDoc* m_p_spk = nullptr;
	HMENU		m_hMyMenu = nullptr;
	CIntervals	m_stimulus_saved;

	CdbTable*	m_pDB = nullptr;
	BOOL		m_validTables = false;
	CDWordArray m_selectedRecords;
	CString		m_dbFilename;
	CString		m_ProposedDataPathName;

	void	import_file_list(CStringArray& file_list, int n_columns = 1, boolean b_header = false);
	BOOL	import_database(const CString& filename) const;

	static BOOL	is_extension_recognized_as_data_file(CString string);
	static BOOL	is_file_present(CString& cs_filename) {
		CFileStatus status;
		return CFile::GetStatus(cs_filename, status);
	}

	CWaveFormat* get_wave_format(CString filename, BOOL is_dat_file);

	BOOL	OnNewDocument(LPCTSTR lpszPathName);
	AcqDataDoc* open_current_data_file();
	AcqDataDoc* get_current_dat_document() const { return m_p_dat; }
	void	close_current_data_file() const;
	CSpikeDoc* open_current_spike_file();
	CSpikeDoc* get_current_spike_file() const { return m_p_spk; }
	Spike* get_spike(const dbSpike& spike_coords);

	void	remove_duplicate_files();
	void	remove_missing_files() const;
	void	remove_false_spk_files() const;
	void	delete_erased_files();

	long	get_db_n_spikes() const;
	void	set_db_n_spikes(long n_spikes) const;
	long	get_db_n_spike_classes() const;
	void	set_db_n_spike_classes(long n_classes) const;
	void	get_max_min_of_all_spikes(BOOL b_all_files, BOOL b_recalculate, short* max, short* min);
	CSize	get_max_min_of_single_spike(BOOL b_all);

	void	set_clean_db_on_exit(BOOL bClear) { m_clean_database_on_exit_ = bClear; }

	void	export_data_ascii_comments(CSharedFile* p_shared_file);
	void	export_number_of_spikes(CSharedFile* p_sf);
	CString export_database_data(int option = 0) const;
	void	export_spk_descriptors(CSharedFile* p_sf, SpikeList* p_spike_list, int k_class) const;
	void	export_datafiles_as_text_files();

	void	synchronize_source_infos(const BOOL b_all);
	BOOL	update_waveformat_from_database(CWaveFormat* p_wave_format) const;
	BOOL	import_data_files_from_another_data_base(const CString& other_data_base_file_name) const;
	BOOL	copy_files_to_directory(const CString& path);

protected:
	sourceData get_wave_format_from_either_file(CString cs_filename);
	void	set_record_file_names(const sourceData* record) const;
	boolean set_record_spk_classes(const sourceData* record) const;
	void	set_record_wave_format(const sourceData* record) const;
	boolean import_file_single(const CString& cs_filename, long& m_id, int i_record, const CStringArray& cs_array, int n_columns,
	                           boolean b_header);
	CString get_full_path_name_without_extension() const;

	static numberIDToText headers_[];
	static BOOL	transpose_file_for_excel(CSharedFile* p_sf);
	static int check_files_can_be_opened(CStringArray& file_names_array, CSharedFile* psf, int n_columns, boolean b_header);
	static int index_2d_array(const int i_row, const int n_columns, boolean bHeader) { return (i_row + (bHeader ? 1 : 0)) * n_columns; }
	static int get_size_2d_array(const CStringArray& cs_array, int nColumns, boolean bHeader) { return cs_array.GetSize() / nColumns - (bHeader ? 1 : 0); }
	static void	remove_row_at(CStringArray& file_name_array, int i_row, int n_columns, boolean b_header);
	static CSharedFile* file_discarded_message(CSharedFile* p_sf, const CString& cs_filename, int i_record);
	static void	get_infos_from_string_array(const sourceData* p_record, const CStringArray& file_names_array, int const i_record, int n_columns, boolean b_header);
	static int	find_column_associated_to_header(const CString& text);
	static void	remove_file_from_disk(const CString& file_name);
	static CString get_path_directory(const CString& full_name);
	static boolean create_directory_if_does_not_exists(const CString& path);
	static bool	binary_file_copy(LPCTSTR psz_source, LPCTSTR psz_dest);
	static void	copy_files_to_directory(CStringArray& files_to_copy_array, const CString& mdb_directory);
	static CString copy_file_to_directory(const LPCTSTR psz_source, const CString& directory);
	static bool is_file_present(const CString& cs_new_name);
	static boolean file_exists(const CString& file_name);

	// Overrides
public:
	static CdbWaveDoc* get_active_mdi_document();

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

	// DAO database functions
	long	db_get_n_records() const { return m_pDB->get_n_records(); }
	CdbTableMain* db_get_recordset() const { return &m_pDB->m_mainTableSet; }
	void	db_delete_current_record();
	CString db_get_current_dat_file_name(BOOL b_test = FALSE);
	CString db_get_current_spk_file_name(BOOL b_test = FALSE);

	CString db_set_current_spike_file_name();
	void	db_set_data_len(long len) const { m_pDB->set_data_length(len); }
	long	db_get_data_len() const;
	void	db_set_current_record_flag(int flag) const;
	int		db_get_current_record_flag() const { return m_pDB->m_mainTableSet.m_flag; }
	void	db_set_paths_relative() const;
	void	db_set_paths_absolute() const;
	void	db_transfer_dat_path_to_spk_path() const;
	void	db_delete_unused_entries() const;

	long	db_get_current_record_position() const;
	long	db_get_current_record_id() const;
	BOOL	db_set_current_record_position(const long i_file) const { return m_pDB->set_index_current_file(i_file); }

	BOOL	db_move_to_id(long record_id) const { return m_pDB->move_to_id(record_id); }
	BOOL	db_move_first() const { return m_pDB->move_to(ID_RECORD_FIRST); }
	BOOL	db_move_next() const { return m_pDB->move_to(ID_RECORD_NEXT); }
	BOOL	db_move_prev() const { return m_pDB->move_to(ID_RECORD_PREV); }
	BOOL	db_move_last() const { return m_pDB->move_to(ID_RECORD_LAST); }
	void	db_refresh_query() const {
		if (m_pDB->m_mainTableSet.IsBOF()) m_pDB->m_mainTableSet.SetFieldNull(nullptr);
		m_pDB->m_mainTableSet.RefreshQuery();
	}

protected:
	BOOL	open_database(LPCTSTR lpszPathName);

	DECLARE_MESSAGE_MAP()
};
