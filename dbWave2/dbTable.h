#pragma once

#define NTABLECOLS	29
#include "AcqWaveFormat.h"

typedef struct 
{
	int column_number;
	CString header_name;
	CString description;
	int format_code_number;
	CString attached_table;
} db_column_properties, *lp_database_column_properties;

// CdbTable command target
#pragma warning(disable : 4995)

class CdbTable : public CDaoDatabase
{
public:
	CdbTable();
	~CdbTable() override;

	static db_column_properties m_desctab[];

	// CDaoRecordSets
	CdbTableMain m_mainTableSet;

//protected:
	CdbTableAssociated m_operatorSet;
	CdbTableAssociated m_insectSet;
	CdbTableAssociated m_locationSet;
	CdbTableAssociated m_sensillumSet;
	CdbTableAssociated m_stimSet;
	CdbTableAssociated m_concSet;
	CdbTableAssociated m_sexSet;
	CdbTableAssociated m_strainSet;
	CdbTableAssociated m_exptSet;
	CdbTableAssociated m_pathSet;

	//CdbTableAssociated	m_stim2Set; // TODO
	//CdbTableAssociated	m_conc2Set;	// TODO

public:
	CString* m_current_data_filename = nullptr;
	CString* m_p_current_spike_filename = nullptr;
	CString m_database_path = _T("");

	void Attach(CString* pstrData, CString* pstrSpk);

	// operations
	BOOL CreateMainTable(CString cs);
	void CreateTables();
	BOOL OpenTables();
	void add_column_28(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_column_26_27(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_column_24_25(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_column_22_23(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_column_21(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_column_19_20(CDaoTableDef& table_def, CString cs_table, long l_attr);

	void OpenIndexTable(CdbTableAssociated* p_index_table_set);
	void CloseDatabase();
	void UpdateTables();

	CString GetDataBasePath();
	void SetDataBasePath();
	void GetFilenamesFromCurrentRecord();
	CString GetDatFilenameFromCurrentRecord();
	CString GetSpkFilenameFromCurrentRecord();

	// operations on main table
	BOOL MoveToID(long recordID);
	CString GetFilePath(int ID);
	BOOL IsRelativePath(CString cspath) { return (cspath[0] == '.'); }
	CString GetRelativePathFromString(const CString& csAbsolutePath);
	long GetRelativePathFromID(long iID);
	void ConvertPathtoRelativePath(long icol);
	void ConvertPathTabletoRelativePath();
	CString GetAbsolutePathFromString(CString csPath);
	long GetAbsolutePathFromID(long iID);
	void ConvertPathtoAbsolutePath(int icolpath);
	void ConvertPathTabletoAbsolutePath();
	BOOL MoveRecord(UINT nIDMoveCommand);
	BOOL MoveFirst() { return MoveRecord(ID_RECORD_FIRST); }
	BOOL MoveNext() { return MoveRecord(ID_RECORD_NEXT); }
	BOOL MovePrev() { return MoveRecord(ID_RECORD_PREV); }
	BOOL MoveLast() { return MoveRecord(ID_RECORD_LAST); }
	void SetDataLen(long datalen) { m_mainTableSet.SetDataLen(datalen); }
	long GetNRecords() { return m_mainTableSet.GetNRecords(); }
	long GetNFields() { return m_mainTableSet.m_nFields; }

	// get associated table?
	// get list of items in associated tables?

	BOOL SetIndexCurrentFile(long ifile);
	DB_ITEMDESC* GetRecordItemDescriptor(int icol);
	BOOL GetRecordItemValue(int icol, DB_ITEMDESC* pdesc);
	DB_ITEMDESC* GetRecordItemValue(const int i_column);
	BOOL SetRecordItemValue(int icol, DB_ITEMDESC* pdesc);
	boolean GetRecordValueString(int column_index, CString& output_string);
	boolean GetRecordValueLong(int column_index, long& value);



	BOOL ImportRecordfromDatabase(CdbTable* pdbW);
	void TransferWaveFormatDataToRecord(CWaveFormat* p_wave_format);
	void DeleteUnusedEntriesInAccessoryTables();
	void DeleteUnusedEntriesInAttachedTable(CdbTableAssociated* pIndexTable, int column1, int column2);
	static void CompactDataBase(CString file_name, CString file_name_new);

protected:
	BOOL CreateRelationwithAssocTable(LPCTSTR lpszForeignTable, int icol, long lAttributes, CdbTableAssociated* plink);
	BOOL CreateRelationwith2AssocTables(LPCTSTR lpszForeignTable, int icol1, int icol2);
	void SetAttachedTablesNames();
	boolean CreateRelationsWithAttachedTables(CString csTable);
};
