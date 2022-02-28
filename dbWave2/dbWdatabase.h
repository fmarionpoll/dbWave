#pragma once

#define NTABLECOLS	29
#include "AcqWaveFormat.h"

typedef struct _TABCOL
{
	int iID;
	TCHAR* szTableCol;
	TCHAR* szDescriptor;
	int propCol;
	TCHAR* szRelTable;
} TABCOL, *LPTABCOL;

// CdbWdatabase command target
#pragma warning(disable : 4995)

class CdbWdatabase : public CDaoDatabase
{
public:
	CdbWdatabase();
	~CdbWdatabase() override;

	static TABCOL m_desctab[];

	// CDaoRecordSets
	CdbMainTable m_mainTableSet;

	CdbIndexTable m_operatorSet;
	CdbIndexTable m_insectSet;
	CdbIndexTable m_locationSet;
	CdbIndexTable m_sensillumSet;
	CdbIndexTable m_stimSet;
	CdbIndexTable m_concSet;
	CdbIndexTable m_sexSet;
	CdbIndexTable m_strainSet;
	CdbIndexTable m_exptSet;
	CdbIndexTable m_pathSet;

	//CdbIndexTable	m_stim2Set; // TODO
	//CdbIndexTable	m_conc2Set;	// TODO

	// parent
	CString* m_pcurrentDataFilename;
	CString* m_pcurrentSpkFileName;
	// local
	CString m_databasePath;

	void Attach(CString* pstrData, CString* pstrSpk);

	// operations
	BOOL CreateMainTable(CString cs);
	void CreateTables();
	BOOL OpenTables();
	void add_28(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_26_27(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_24_25(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_22_23(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_21(CDaoTableDef& table_def, CString cs_table, long l_attr);
	void add_19_20(CDaoTableDef& table_def, CString cs_table, long l_attr);

	void OpenIndexTable(CdbIndexTable* pIndexTableSet);
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
	BOOL SetRecordItemValue(int icol, DB_ITEMDESC* pdesc);
	BOOL ImportRecordfromDatabase(CdbWdatabase* pdbW);
	void TransferWaveFormatDataToRecord(CWaveFormat* p_wave_format);
	void DeleteUnusedEntriesInAccessoryTables();
	void DeleteUnusedEntriesInAttachedTable(CdbIndexTable* pIndexTable, int column1, int column2);

protected:
	BOOL CreateRelationwithAssocTable(LPCTSTR lpszForeignTable, int icol, long lAttributes, CdbIndexTable* plink);
	BOOL CreateRelationwith2AssocTables(LPCTSTR lpszForeignTable, int icol1, int icol2);
};
