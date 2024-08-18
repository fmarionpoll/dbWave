#pragma once

#define N_TABLE_COLUMNS	29
#include "AcqWaveFormat.h"

typedef struct
{
	int column_number;
	CString header_name;
	CString description;
	int format_code_number;
	CString attached_table;
} column_properties, * lp_database_column_properties;

// CdbTable command target
#pragma warning(disable : 4995)

class CdbTable : public CDaoDatabase
{
public:
	CdbTable();
	~CdbTable() override;

	static column_properties m_column_properties[];

	// CDaoRecordSets
	CdbTableMain m_main_table_set;

	//protected:
	CdbTableAssociated m_operator_set;
	CdbTableAssociated m_insect_set;
	CdbTableAssociated m_location_set;
	CdbTableAssociated m_sensillum_set;
	CdbTableAssociated m_stimulus_set;
	CdbTableAssociated m_concentration_set;
	CdbTableAssociated m_sex_set;
	CdbTableAssociated m_strain_set;
	CdbTableAssociated m_experiment_set;
	CdbTableAssociated m_path_set;

	//CdbTableAssociated	m_stim2Set; // TODO
	//CdbTableAssociated	m_conc2Set;	// TODO

	CString current_data_filename = {};
	CString current_spike_filename = {};
	CString m_database_path = _T("");

	void attach(const CString& cs_data_file_name, const CString& cs_spike_file_name);

	// operations
	BOOL create_main_table(const CString& cs_table);
	void create_all_tables();

	BOOL open_tables();
	void add_column_28(CDaoTableDef& table_def, const CString& cs_table, long l_attr);
	void add_column_26_27(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const;
	void add_column_24_25(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const;
	void add_column_22_23(CDaoTableDef& table_def, const CString& cs_table, long l_attr);
	void add_column_21(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const;
	void add_column_19_20(CDaoTableDef& table_def, const CString& cs_table, long l_attr);

	void open_associated_table(CdbTableAssociated* p_index_table_set);
	void close_database();
	void update_all_database_tables();

	CString get_data_base_path();
	void set_data_base_path();
	void get_current_record_file_names();
	CString get_current_record_data_file_name();
	CString get_current_record_spike_file_name();

	// operations on main table
	CString get_file_path(int i_id);
	static BOOL is_relative_path(const CString& cs_path) { return (cs_path[0] == '.'); }

	CString get_relative_path_from_string(const CString& cs_path) const;
	long get_relative_path_from_id(long i_id);
	void convert_path_to_relative_path(long i_col_path);
	void set_path_relative();

	CString get_absolute_path_from_string(const CString& cs_path) const;
	long get_absolute_path_from_id(long i_id);
	void convert_to_absolute_path(int i_col_path);
	void set_path_absolute();

	BOOL move_to_id(long record_id);
	BOOL move_to(UINT n_id_move_command);
	BOOL move_to_first_record() { return move_to(ID_RECORD_FIRST); }
	BOOL move_to_next_record() { return move_to(ID_RECORD_NEXT); }
	BOOL move_to_previous_record() { return move_to(ID_RECORD_PREV); }
	BOOL move_to_last_record() { return move_to(ID_RECORD_LAST); }
	void set_data_length(const long data_length) { m_main_table_set.set_data_len(data_length); }
	long get_records_count() { return m_main_table_set.get_records_count(); }
	long get_n_fields() const { return m_main_table_set.m_nFields; }

	// get associated table?
	// get list of items in associated tables?

	BOOL set_index_current_file(long i_file);

	DB_ITEMDESC* get_record_item_descriptor(int column_index);
	BOOL get_record_item_value(const int i_column, DB_ITEMDESC* db_item_descriptor);
	DB_ITEMDESC* get_record_item_value(const int column_index);
	BOOL set_record_item_value(int column_index, DB_ITEMDESC* db_item_descriptor);
	boolean get_record_value_string(int column_index, CString& output_string);
	boolean get_record_value_long(int column_index, long& value);

	BOOL import_record_from_database(CdbTable* p_external_db_table);
	void transfer_wave_format_data_to_record(const CWaveFormat* p_wave_format);
	void delete_unused_entries_in_accessory_tables();
	void delete_unused_entries_in_attached_table(CdbTableAssociated* p_index_table, int index_column1, int index_column2);
	static void compact_data_base(const CString& file_name, const CString& file_name_new);
	boolean is_record_time_unique(const COleDateTime& data_acquisition_date);

protected:
	void create_associated_tables();
	BOOL create_relation_between_associated_table_and_1_column(LPCTSTR lpsz_foreign_table, int column_index, long l_attributes, CdbTableAssociated* p_to_associated_table);
	BOOL create_relation_between_associated_table_and_2_columns(LPCTSTR lpsz_foreign_table, int column_index_1, int column_index_2);
	void set_attached_tables_names();
	boolean create_relations_with_attached_tables(const CString& cs_table);
};
