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
} column_properties, *lp_database_column_properties;

// CdbTable command target
#pragma warning(disable : 4995)

class CdbTable : public CDaoDatabase
{
public:
	CdbTable();
	~CdbTable() override;

	static column_properties m_column_properties[];

	// CDaoRecordSets
	CdbTableMain m_mainTableSet;

//protected:
	CdbTableAssociated m_operator_set;
	CdbTableAssociated m_insect_set;
	CdbTableAssociated m_locationSet;
	CdbTableAssociated m_sensillum_set;
	CdbTableAssociated m_stim_set;
	CdbTableAssociated m_conc_set;
	CdbTableAssociated m_sex_set;
	CdbTableAssociated m_strain_set;
	CdbTableAssociated m_expt_set;
	CdbTableAssociated m_path_set;

	//CdbTableAssociated	m_stim2Set; // TODO
	//CdbTableAssociated	m_conc2Set;	// TODO

	CString* m_current_data_filename = nullptr;
	CString* m_p_current_spike_filename = nullptr;
	CString m_database_path = _T("");

	void Attach(CString* cs_data_file_name, CString* cs_spike_file_name);

	// operations
	BOOL CreateMainTable(const CString& cs);
	void CreateAllTables();
	
	BOOL OpenTables();
	void add_column_28(CDaoTableDef& table_def, const CString& cs_table, long l_attr);
	void add_column_26_27(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const;
	void add_column_24_25(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const;
	void add_column_22_23(CDaoTableDef& table_def, const CString& cs_table, long l_attr);
	void add_column_21(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const;
	void add_column_19_20(CDaoTableDef& table_def, const CString& cs_table, long l_attr);

	void OpenAssociatedTable(CdbTableAssociated* p_index_table_set);
	void CloseDatabase();
	void UpdateAllDatabaseTables();

	CString GetDataBasePath();
	void SetDataBasePath();
	void GetCurrentRecord_FileNames();
	CString GetCurrentRecord_DataFileName();
	CString GetCurrentRecord_SpikeFileName();

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

	BOOL MoveToID(long record_id);
	BOOL MoveTo(UINT nIDMoveCommand);
	BOOL MoveToFirstRecord() { return MoveTo(ID_RECORD_FIRST); }
	BOOL MoveToNextRecord() { return MoveTo(ID_RECORD_NEXT); }
	BOOL MoveToPreviousRecord() { return MoveTo(ID_RECORD_PREV); }
	BOOL MoveToLastRecord() { return MoveTo(ID_RECORD_LAST); }
	void SetDataLength(const long data_length) { m_mainTableSet.SetDataLen(data_length); }
	long GetNRecords() { return m_mainTableSet.GetNRecords(); }
	long GetNFields() const { return m_mainTableSet.m_nFields; }

	// get associated table?
	// get list of items in associated tables?

	BOOL SetIndexCurrentFile(long i_file);

	DB_ITEMDESC* GetRecordItemDescriptor(int column_index);
	BOOL GetRecordItemValue(int column_index, DB_ITEMDESC* dbItem_descriptor);
	DB_ITEMDESC* GetRecordItemValue(const int column_index);
	BOOL SetRecordItemValue(int column_index, DB_ITEMDESC* dbItem_descriptor);
	boolean GetRecordValueString(int column_index, CString& output_string);
	boolean GetRecordValueLong(int column_index, long& value);

	BOOL ImportRecordFromDatabase(CdbTable* p_external_dbTable);
	void TransferWaveFormatDataToRecord(const CWaveFormat* p_wave_format);
	void DeleteUnusedEntriesInAccessoryTables();
	void DeleteUnusedEntriesInAttachedTable(CdbTableAssociated* p_index_table, int index_column1, int index_column2);
	static void CompactDataBase(const CString& file_name, const CString& file_name_new);
	boolean IsRecordTimeUnique(const COleDateTime& data_acquisition_date);

protected:
	void create_associated_tables();
	BOOL create_relation_between_associated_table_and_1_column(LPCTSTR lpsz_foreign_table, int column_index, long l_attributes, CdbTableAssociated* p_to_associated_table);
	BOOL create_relation_between_associated_table_and_2_columns(LPCTSTR lpsz_foreign_table, int column_index_1, int column_index_2);
	void set_attached_tables_names();
	boolean create_relations_with_attached_tables(const CString& csTable);
};
