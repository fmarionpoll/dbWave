// dbWdatabase.cpp : implementation file
//
#include "stdafx.h"
#include "dbIndexTable.h"
#include "dbMainTable.h"
#include "dbWdatabase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CdbWdatabase


TABCOL CdbWdatabase::m_desctab[NTABLECOLS] =
{
	//table col header---friendly description for properties--type of field--name of attached table
	CH_ID,				_T("ID"),			_T("ID record"),				FIELD_LONG,			_T(""),				// 0
	CH_ACQDATE,			_T("acq_date"),		_T("acquisition date & time"),	FIELD_DATE,			_T(""),				// 1
	CH_FILENAME,		_T("filename"),		_T("dat file"),					FIELD_TEXT,			_T(""),				// 2
	CH_FILESPK,			_T("filespk"),		_T("spk file"),					FIELD_TEXT,			_T(""),				// 3
	CH_ACQ_COMMENTS,	_T("acq_comment"),	_T("comment"),					FIELD_TEXT,			_T(""),				// 4
	CH_MORE,			_T("more"),			_T("comment"),					FIELD_TEXT,			_T(""),				// 5
	CH_IDINSECT,		_T("insectID"),		_T("ID insect"),				FIELD_LONG,			_T(""),				// 6
	CH_IDSENSILLUM,		_T("sensillumID"),	_T("ID sensillum"),				FIELD_LONG,			_T(""),				// 7
	CH_DATALEN,			_T("datalen"),		_T("data length (n points)"),	FIELD_LONG,			_T(""),				// 8
	CH_NSPIKES,			_T("nspikes"),		_T("n spikes detected"),		FIELD_LONG,			_T(""),				// 9
	CH_NSPIKECLASSES,	_T("nspikeclasses"),_T("n spike classes"),			FIELD_LONG,			_T(""),				// 10
	CH_FLAG,			_T("flag"),			_T("flag"),						FIELD_LONG,			_T(""),				// 11 
	CH_INSECT_ID,		_T("insectname_ID"),_T("insect"),					FIELD_IND_TEXT,		_T("insectname"),	// 12
	CH_SENSILLUM_ID,	_T("sensillumname_ID"),_T("sensillum"),				FIELD_IND_TEXT,		_T("sensillumname"),// 13
	CH_OPERATOR_ID,		_T("operator_ID"),	_T("operator"),					FIELD_IND_TEXT,		_T("operator"),		// 14
	CH_STIM_ID,			_T("stim_ID"),		_T("compound(1)"),				FIELD_IND_TEXT,		_T("stim"),			// 15
	CH_CONC_ID,			_T("conc_ID"),		_T("concentration(1)"),			FIELD_IND_TEXT,		_T("conc"),			// 16
	CH_LOCATION_ID,		_T("location_ID"),	_T("location"),					FIELD_IND_TEXT,		_T("location"),		// 17
	CH_PATH_ID,			_T("path_ID"),		_T("path (*.dat)"),				FIELD_IND_FILEPATH,	_T("path"),			// 18
	CH_PATH2_ID,		_T("path2_ID"),		_T("path (*.spk)"),				FIELD_IND_FILEPATH,	_T("path"),			// 19
	CH_STIM2_ID,		_T("stim2_ID"),		_T("compound(2)"),				FIELD_IND_TEXT,		_T("stim"),			// 20
	CH_CONC2_ID,		_T("conc2_ID"),		_T("concentration(2)"),			FIELD_IND_TEXT,		_T("conc"),			// 21
	CH_STRAIN_ID,		_T("strain_ID"),	_T("insect strain"),			FIELD_IND_TEXT,		_T("strain"),		// 22
	CH_SEX_ID,			_T("sex_ID"),		_T("insect sex"),				FIELD_IND_TEXT,		_T("sex"),			// 23
	CH_REPEAT,			_T("repeat"),		_T("repeat(1)"),				FIELD_LONG,			_T(""),				// 24
	CH_REPEAT2,			_T("repeat2"),		_T("repeat(2)"),				FIELD_LONG,			_T(""),				// 25
	CH_ACQDATE_DAY,		_T("acqdate_day"),	_T("date"),						FIELD_DATE_YMD,		_T(""),				// 26
	CH_ACQDATE_TIME,	_T("acqdate_time"), _T("time"),						FIELD_DATE_HMS,		_T(""),				// 27
	CH_EXPT_ID,			_T("expt_ID"),		_T("Experiment"),				FIELD_IND_TEXT,		_T("expt")			// 28
};


CdbWdatabase::CdbWdatabase()
{
	for (int i = 0; i < NTABLECOLS; i++)
	{
		CString csdummy = m_desctab[i].szTableCol;
		m_mainTableSet.m_desc[i].csColName = csdummy;
		m_mainTableSet.m_desc[i].csColNamewithBrackets = _T("[") + csdummy + _T("]");
		m_mainTableSet.m_desc[i].csColParam = csdummy + _T("Param");
		m_mainTableSet.m_desc[i].csEQUcondition = csdummy + _T("=") + m_mainTableSet.m_desc[i].csColParam;
		m_mainTableSet.m_desc[i].typeLocal = m_desctab[i].propCol;
	}

	// SetNames(CString defaultSQL /* or table name*/, CString DFX_cs, CString DFX_ID) 
	m_stimSet.SetNames		(m_desctab[CH_STIM_ID].szRelTable,		_T("stim"),		_T("stimID"));
	m_concSet.SetNames		(m_desctab[CH_CONC_ID].szRelTable,		_T("conc"),		_T("concID"));
	m_operatorSet.SetNames	(m_desctab[CH_OPERATOR_ID].szRelTable,	_T("operator"), _T("operatorID"));
	m_insectSet.SetNames	(m_desctab[CH_INSECT_ID].szRelTable,	_T("insect"),	_T("insectID"));
	m_locationSet.SetNames	(m_desctab[CH_LOCATION_ID].szRelTable,	_T("type"),		_T("typeID"));
	m_pathSet.SetNames		(m_desctab[CH_PATH_ID].szRelTable,		_T("path"),		_T("pathID"));
	m_sensillumSet.SetNames	(m_desctab[CH_SENSILLUM_ID].szRelTable,	_T("stage"),	_T("stageID"));
	m_sexSet.SetNames		(m_desctab[CH_SEX_ID].szRelTable,		_T("sex"),		_T("sexID"));
	m_strainSet.SetNames	(m_desctab[CH_STRAIN_ID].szRelTable,	_T("strain"),	_T("strainID"));
	m_exptSet.SetNames		(m_desctab[CH_EXPT_ID].szRelTable,		_T("expt"),		_T("exptID"));

	m_mainTableSet.m_strSort = m_desctab[CH_ACQDATE].szTableCol;

	m_databasePath.Empty();
	m_pcurrentSpkFileName = nullptr;
}


void CdbWdatabase::Attach (CString* pstrDataName, CString* pstrSpkName)
{
	m_pcurrentDataFilename = pstrDataName;
	m_pcurrentSpkFileName = pstrSpkName;
}


CdbWdatabase::~CdbWdatabase()
{
	CloseDatabase();
}

// CdbWdatabase member functions

BOOL CdbWdatabase::CreateMainTable(CString csTable)
{
	CDaoTableDef tableDef (this);
	tableDef.Create(csTable);
	
	//first create the main field, and ID number that is incremented automatically
	// when a new record is created. This column is indexed
	CDaoFieldInfo fd0;
	fd0.m_strName				= m_mainTableSet.m_desc[CH_ID].csColName;	// "ID"
	fd0.m_nType					= dbLong;						// Primary
	fd0.m_lSize					= 4;							// Primary
	fd0.m_lAttributes			= dbAutoIncrField;				// Primary
	fd0.m_nOrdinalPosition		= 1;							// Secondary
	fd0.m_bRequired				= TRUE;							// Secondary
	fd0.m_bAllowZeroLength		= FALSE;						// Secondary
	fd0.m_lCollatingOrder		= dbSortGeneral;				// Secondary
	fd0.m_strForeignName		= _T("");						// Secondary
	fd0.m_strSourceField		= _T("");						// Secondary
	fd0.m_strSourceTable		= _T("");						// Secondary
	fd0.m_strValidationRule		= _T("");						// All   
	fd0.m_strValidationText		= _T("");						// All
	fd0.m_strDefaultValue		= _T("");						// All
	tableDef.CreateField(fd0);													// 0 - iID
	
	// then create data fields
	int i=1;
	tableDef.CreateField(m_mainTableSet.m_desc[i].csColName, dbDate, 8,	  0);		// 1 -acq_date

	fd0.m_bAllowZeroLength = TRUE;
	fd0.m_bRequired = FALSE;
	fd0.m_lAttributes = dbVariableField;
	fd0.m_nType = dbText;
	fd0.m_lSize = 255;

	for (i = 2; i <= 4; i++)													// 2 -filename / 3 -filespk // 4 - "acq_comment" 
	{ 
		fd0.m_strName = m_mainTableSet.m_desc[i].csColName;
		fd0.m_nOrdinalPosition = i;
		tableDef.CreateField(fd0);
	}

	i=5;																		// 5 - "more"
	fd0.m_strName			= m_mainTableSet.m_desc[i].csColName;	
	fd0.m_nOrdinalPosition = i;
	fd0.m_nType				= dbMemo;
	fd0.m_lSize				= dbMemo;
	tableDef.CreateField(fd0);
	
	for (i = 6; i <= 25; i++)
		tableDef.CreateField(m_mainTableSet.m_desc[i].csColName, dbLong, 4, 0);		//  6 - insectID to 25 = sex_ID
	for ( i = 26; i <= 27; i++)
		tableDef.CreateField(m_mainTableSet.m_desc[i].csColName, dbDate, 8, 0);		// 26 - acqdate_day / 27 - acqdate_time
	i=28; 
	tableDef.CreateField(m_mainTableSet.m_desc[i].csColName, dbLong, 4, 0);			// 28 - expt_ID

	// create the corresponding indexes
	CDaoIndexFieldInfo indexfield0;
	indexfield0.m_strName = m_mainTableSet.m_desc[CH_IDINSECT].csColName;			// insectID
	indexfield0.m_bDescending = FALSE;

	CDaoIndexInfo indexfd0;
	indexfield0.m_strName	= m_mainTableSet.m_desc[CH_ID].csColName;				// ID
	indexfd0.m_strName		= _T("Primary_Key");
	indexfd0.m_pFieldInfos	= &indexfield0;
	indexfd0.m_bPrimary		= TRUE;
	indexfd0.m_bUnique		= TRUE;
	indexfd0.m_bRequired	= TRUE;
	indexfd0.m_bForeign		= FALSE;
	indexfd0.m_nFields		= 1;
	indexfd0.m_bClustered	= FALSE;
	indexfd0.m_bIgnoreNulls = TRUE; // previously: FALSE;
	tableDef.CreateIndex(indexfd0);
	tableDef.Append();

	// create relations
	long lAttr = dbRelationDontEnforce; //dbRelationUpdateCascade;
	if (!CreateRelationwithAssocTable(csTable, CH_INSECT_ID,	lAttr, &m_insectSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_SENSILLUM_ID,	lAttr, &m_sensillumSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_OPERATOR_ID,	lAttr, &m_operatorSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_LOCATION_ID,	lAttr, &m_locationSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_STRAIN_ID,	lAttr, &m_strainSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_SEX_ID,		lAttr, &m_sexSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_EXPT_ID,		lAttr, &m_exptSet)) return FALSE;
		
	if (!CreateRelationwith2AssocTables(csTable, CH_PATH_ID, CH_PATH2_ID)) return FALSE;
	if (!CreateRelationwith2AssocTables(csTable, CH_STIM_ID, CH_STIM2_ID)) return FALSE;
	if (!CreateRelationwith2AssocTables(csTable, CH_CONC_ID, CH_CONC2_ID)) return FALSE;

	return TRUE;
}


BOOL CdbWdatabase::CreateRelationwith2AssocTables(LPCTSTR lpszForeignTable, int icol1, int icol2)
{
	try {
		LPCTSTR lpszTable = m_desctab[icol1].szRelTable;

		CDaoRelationInfo rlinfo;
		rlinfo.m_strTable = lpszTable;
		rlinfo.m_strName.Format(_T("%s_%s"), lpszForeignTable, lpszTable);
		rlinfo.m_strForeignTable = lpszForeignTable;
		rlinfo.m_lAttributes = dbRelationDontEnforce;

		CDaoRelationFieldInfo rfield[2];
		rfield[0].m_strName = lpszTable;
		rfield[0].m_strName += _T("ID");
		rfield[0].m_strForeignName = m_mainTableSet.m_desc[icol1].csColName; // path_ID
		rfield[1].m_strName = rfield[0].m_strName;
		rfield[1].m_strForeignName = m_mainTableSet.m_desc[icol2].csColName; // path2_ID
		rlinfo.m_pFieldInfos = &rfield[0];
		rlinfo.m_nFields = 2;
		CreateRelation(rlinfo);
	
		m_mainTableSet.m_desc[icol1].plinkedSet = &m_pathSet;
		m_mainTableSet.m_desc[icol1].csAssocTable = rfield[0].m_strName;
	
		m_mainTableSet.m_desc[icol2].plinkedSet = &m_pathSet;
		m_mainTableSet.m_desc[icol2].csAssocTable = rfield[0].m_strName;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 5); e->Delete();
		return FALSE;
	}
	return TRUE;
}


// insect name: "table_insectname" relates "insect" table with "ID" and "insectname_ID"
BOOL CdbWdatabase::CreateRelationwithAssocTable(LPCTSTR lpszForeignTable, int icol, long lAttributes, CdbIndexTable* plink)
{
	try {
		LPCTSTR lpszTable = m_desctab[icol].szRelTable;
		m_mainTableSet.m_desc[icol].csAssocTable = lpszTable;
		CString csrel;		// unique name of the relation object (max 40 chars)
		csrel.Format(_T("%s_%s"), lpszForeignTable, lpszTable);
		CString szField = lpszTable;
		szField += _T("ID");
		CreateRelation(csrel, lpszTable, lpszForeignTable, lAttributes, szField, m_mainTableSet.m_desc[icol].csColName);
		m_mainTableSet.m_desc[icol].plinkedSet = plink;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 5); e->Delete();
		return FALSE;
	}
	return TRUE;
}


CString CdbWdatabase::GetDataBasePath() {
	CString filename = GetName();
	int ilastslash = filename.ReverseFind('\\');
	m_databasePath = filename.Left(ilastslash+1);
	m_databasePath.MakeLower();
	return m_databasePath;
}


void CdbWdatabase::CreateTables()
{
	GetDataBasePath();

	// create tables with indexes. These tables will contain categories entered by the user
	m_insectSet.CreateIndextable	(_T("insectname"),		_T("insect"),	_T("insectID"),		100, this);
	m_sensillumSet.CreateIndextable	(_T("sensillumname"),	_T("stage"),	_T("stageID"),		100, this);
	m_operatorSet.CreateIndextable	(_T("operator"),		_T("operator"),	_T("operatorID"),	50,  this);
	m_stimSet.CreateIndextable		(_T("stim"),			_T("stim"),		_T("stimID"),		100, this);
	m_concSet.CreateIndextable		(_T("conc"),			_T("conc"),		_T("concID"),		100, this);
	m_locationSet.CreateIndextable	(_T("location"),		_T("type"),		_T("typeID"),		100, this);
	m_pathSet.CreateIndextable		(_T("path"),			_T("path"),		_T("pathID"),		255, this);	
	m_strainSet.CreateIndextable	(_T("strain"),			_T("strain"),	_T("strainID"),		100, this);
	m_sexSet.CreateIndextable		(_T("sex"),				_T("sex"),		_T("sexID"),		10,  this);
	m_exptSet.CreateIndextable		(_T("expt"),			_T("expt"),		_T("exptID"),		100, this);

	CreateMainTable(_T("table"));

	// store links to attached table addresses
	for (int icol = 0; icol < NTABLECOLS; icol++)
		GetRecordItemDescriptor(icol);
}


BOOL CdbWdatabase::OpenTables()
{
	GetDataBasePath();

	// check format of table set
	CDaoRecordset recordSet(this);
	CString csTable = _T("table");

	// check for the presence of Table 'table' 
	try 
	{ 
		CDaoFieldInfo fd0;
		recordSet.Open(dbOpenTable, csTable);
		// check if column "filename" is present
		recordSet.GetFieldInfo(m_mainTableSet.m_desc[CH_FILENAME].csColName, fd0);
		// check number of columns
		int nfields = recordSet.GetFieldCount();
		recordSet.Close();
		
		// less columns are present: add missing columns		
		CdbMainTable rs2;				// CDaoRecordSet
		if (nfields < rs2.m_nFields)
		{
			AfxMessageBox(_T("This is a valid dbWave database\nbut its version must be upgraded...\nOK?"));
			// open table definition
			CDaoTableDef tableDef (this);
			CString csRel;
			long lAttr = dbRelationUpdateCascade;
	
			switch (nfields)
			{
			// only 19 fields - create 2 additional fields for secondary stim and conc
			case 19:
				{
				tableDef.Open(csTable);
				csRel = _T("table_Rel1");
				int ipos = csRel.GetLength()-1;
				tableDef.CreateField(m_mainTableSet.m_desc[CH_STIM2_ID].csColName, dbLong, 4, 0); // stim2_ID
				tableDef.CreateField(m_mainTableSet.m_desc[CH_CONC2_ID].csColName, dbLong, 4, 0); // conc2_ID
				csRel.SetAt(ipos, '9');
				CreateRelation(csRel, _T("stim"), csTable, lAttr, _T("stimID"), m_mainTableSet.m_desc[CH_STIM2_ID].csColName); // stim2_ID
				csRel.SetAt(ipos, 'A');
				CreateRelation(csRel, _T("conc"), csTable, lAttr, _T("concID"), m_mainTableSet.m_desc[CH_CONC2_ID].csColName); // conc2_ID
				tableDef.Close();
				}
			// only 21 fields (instead of 22) - add one field for flags
			case 21:
				tableDef.Open(csTable);
				tableDef.CreateField(m_mainTableSet.m_desc[CH_FLAG].csColName, dbLong, 4, 0);		// flag
				tableDef.Close();

			// only 22 fields (instead of 24) - add 2 fields for strain and sex
			case 22:
				{
				tableDef.Open(csTable);
		
				// add fields in the maintable, add the corresponding tables and the relations between the main table and the new index tables
				tableDef.CreateField(m_mainTableSet.m_desc[CH_STRAIN_ID].csColName, dbLong, 4, 0);	// strain_ID
				tableDef.CreateField(m_mainTableSet.m_desc[CH_SEX_ID].csColName, dbLong, 4, 0);		// sex_ID
				m_strainSet.CreateIndextable(_T("strain"), _T("strain"), _T("strainID"), 100, this);
				m_sexSet.CreateIndextable(_T("sex"), _T("sex"), _T("sexID"), 10,  this);
				CreateRelation(_T("table_strain"), _T("strain"), csTable, lAttr, _T("strainID"),m_mainTableSet.m_desc[CH_STRAIN_ID].csColName); // strain_ID
				CreateRelation(_T("table_sex"), _T("sex"), csTable, lAttr, _T("sexID"), m_mainTableSet.m_desc[CH_SEX_ID].csColName);		// sex_ID
				// type -> location
				DeleteRelation(_T("table_type"));									// delete relationship
				tableDef.DeleteField(CH_LOCATION_ID);								// delete the field (index is different because we deleted one field)
				tableDef.CreateField(m_mainTableSet.m_desc[CH_LOCATION_ID].csColName, dbLong, 4, 0);	// locationID
				// stage -> sensillumname
				DeleteRelation(_T("table_stage"));									// delete relationship
				tableDef.DeleteField(CH_SENSILLUM_ID);							// delete field
				tableDef.CreateField(m_mainTableSet.m_desc[CH_SENSILLUM_ID].csColName, dbLong, 4, 0);	// sensillumID
				tableDef.Close();

				// rename table stage into sensillumname
				tableDef.Open(_T("stage"));
				tableDef.SetName(_T("sensillumname"));
				tableDef.Close();
				
				// rename table type into location
				tableDef.Open(_T("type"));
				tableDef.SetName(_T("location"));
				tableDef.Close();

				// rename existing fields into "sensillumname" table (sensillum/sensillumID instead of stage/stageID)
				// rename existing fields into "location" table (location/locationID instead type/typeID)
				// create relations
				tableDef.Open(csTable);
				csRel = _T("table_sensillumname");
				CreateRelation(csRel, _T("sensillumname"), csTable, lAttr, _T("stageID"), m_mainTableSet.m_desc[CH_SENSILLUM_ID].csColName); // sensillumname_ID
				csRel = _T("table_location");
				CreateRelation(csRel, _T("location"), csTable, lAttr, _T("typeID"), m_mainTableSet.m_desc[CH_LOCATION_ID].csColName); //location_ID
				tableDef.Close();
				}
		
				// only 24 fields instead of 26
			case 24:
				tableDef.Open(csTable);
				tableDef.CreateField(m_mainTableSet.m_desc[CH_REPEAT].csColName, dbLong, 4, 0);		// repeatID
				tableDef.CreateField(m_mainTableSet.m_desc[CH_REPEAT2].csColName, dbLong, 4, 0);	// repeat2ID
				tableDef.Close();

				// only 26 instead of 28
			case 26:
				tableDef.Open(csTable);
				tableDef.CreateField(m_mainTableSet.m_desc[CH_ACQDATE_DAY].csColName, dbDate, 8, 0); // acqdate_day
				tableDef.CreateField(m_mainTableSet.m_desc[CH_ACQDATE_TIME].csColName, dbDate, 8, 0); // acqdate_time
				tableDef.Close();

				// only 28 instead of 29: add "exptID" column and 1 table
			case 28:
				tableDef.Open(csTable);
				tableDef.CreateField(m_mainTableSet.m_desc[CH_EXPT_ID].csColName, dbLong, 4, 0);	// expt_ID
				m_exptSet.CreateIndextable (_T("expt"), _T("expt"), _T("exptID"), 100, this);
				CreateRelation(_T("table_expt"), _T("expt"), csTable, lAttr, _T("exptID"), m_mainTableSet.m_desc[CH_EXPT_ID].csColName); // strain_ID
				tableDef.Close();
				break;

			default:
				return FALSE;
				break;
			}
		}
		// if not, we assume it is a not a valid dbWave database
		// checking is OK, close recordset
	}
	catch (CDaoException* e) 
	{
		CString cs = _T("Error in a database operation - contact dbWave support: \n");
		cs += e->m_pErrorInfo->m_strDescription;
		AfxMessageBox(cs); 
		e->Delete(); 
		return FALSE;
	}

	// open tables
	try 
	{ 
		OpenIndexTable(dbOpenTable, &m_stimSet);
		OpenIndexTable(dbOpenTable, &m_concSet);
		OpenIndexTable(dbOpenTable, &m_operatorSet);
		OpenIndexTable(dbOpenTable, &m_insectSet);
		OpenIndexTable(dbOpenTable, &m_locationSet);
		OpenIndexTable(dbOpenTable, &m_pathSet);
		OpenIndexTable(dbOpenTable, &m_sensillumSet);
		OpenIndexTable(dbOpenTable, &m_sexSet);
		OpenIndexTable(dbOpenTable, &m_strainSet);
		OpenIndexTable(dbOpenTable, &m_exptSet);
	}
	catch(CDaoException* e) 
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete(); 
		return FALSE;
	}

	try 
	{
		m_mainTableSet.m_defaultName = GetName();
		m_mainTableSet.Open (dbOpenDynaset, NULL, 0);
	}
	catch(CDaoException* e) 
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete(); 
		return FALSE;
	}

	// store links to attached table addresses
	for (int icol = 0; icol < NTABLECOLS; icol++)
		GetRecordItemDescriptor(icol);

	return TRUE;
}


void CdbWdatabase::OpenIndexTable(int nOpenType, CdbIndexTable * pIndexTableSet) {
	pIndexTableSet->m_defaultName = GetName();
	pIndexTableSet->Open(dbOpenTable, NULL, 0);
}


void CdbWdatabase::CloseDatabase()
{
	if(IsOpen())
	{
		// closing the workspace does not close the recordsets
		// so first close them
		if (m_mainTableSet.IsOpen())		m_mainTableSet.Close();
		if (m_operatorSet.IsOpen())		m_operatorSet.Close();
		if (m_insectSet.IsOpen())		m_insectSet.Close();
		if (m_locationSet.IsOpen())		m_locationSet.Close();
		if (m_sensillumSet.IsOpen())	m_sensillumSet.Close();
		if (m_pathSet.IsOpen())			m_pathSet.Close();
		if (m_stimSet.IsOpen())			m_stimSet.Close();
		if (m_concSet.IsOpen())			m_concSet.Close();
		if (m_sexSet.IsOpen())			m_sexSet.Close();
		if (m_strainSet.IsOpen())		m_strainSet.Close();
		if (m_exptSet.IsOpen())			m_exptSet.Close();

		// close the workspace
		m_pWorkspace->Close();
		Close();
	}
}


void CdbWdatabase::UpdateTables()
{
	// For each table, call Update() if GetEditMode ()
	//	= dbEditInProgress (Edit has been called) 
	//	= dbEditAdd (AddNew has been called)
	// Update() is necessary before calling Moveto to complete AddNew or Edit operations

	if (m_mainTableSet.GetEditMode() != dbEditNone)
		m_mainTableSet.Update();
	if (m_operatorSet.GetEditMode() != dbEditNone)
		m_operatorSet.Update();
	if (m_insectSet.GetEditMode() != dbEditNone)
		m_insectSet.Update();
	if (m_locationSet.GetEditMode() != dbEditNone)
		m_locationSet.Update();
	if (m_sensillumSet.GetEditMode() != dbEditNone)
		m_sensillumSet.Update();
	if (m_pathSet.GetEditMode() != dbEditNone)
		m_pathSet.Update();	
	if (m_sexSet.GetEditMode() != dbEditNone)
		m_sexSet.Update();	
	if (m_strainSet.GetEditMode() != dbEditNone)
		m_strainSet.Update();
	if (m_exptSet.GetEditMode() != dbEditNone)
		m_exptSet.Update();
}


BOOL CdbWdatabase::MoveToID(long recordID)
{
	CString str;
	str.Format(_T("ID=%li"), recordID);
	try
	{
		if (!m_mainTableSet.FindFirst(str))
			return FALSE;
	}
	catch(CDaoException* e)
	{
		DisplayDaoException(e, 36); 
		e->Delete();
	}
	GetFilenamesFromCurrentRecord();
	return TRUE;
}


CString CdbWdatabase::GetFilePath(int iID) 
{
	CString cspath = m_pathSet.GetStringFromID(iID);
	if (IsRelativePath(cspath))
		cspath = m_databasePath + cspath;
	return cspath;
}


CString CdbWdatabase::GetRelativePathFromString(CString csPath) 
{
	TCHAR szOut[MAX_PATH] = _T("");
	if (csPath.IsEmpty())
		return csPath;

	BOOL flag = PathRelativePathTo(szOut, m_databasePath, FILE_ATTRIBUTE_DIRECTORY, csPath, FILE_ATTRIBUTE_DIRECTORY);
	CString csOut = szOut;
	if (!flag)
		csOut.Empty();
	
	return csOut;
}

long CdbWdatabase::GetRelativePathFromID(long iID) 
{
	long inewID = -1;
	CString cspath = m_pathSet.GetStringFromID(iID);
	if (!IsRelativePath(cspath))
	{
		CString csRelativePath = GetRelativePathFromString(cspath);
		if (!csRelativePath.IsEmpty())
		{
			inewID = m_pathSet.GetIDorCreateIDforString(csRelativePath);
		 }
	}
	else
		inewID = iID;
	return inewID;
}


void CdbWdatabase::ConvertPathtoRelativePath(long icolpath)
{
	COleVariant varValue;
	m_mainTableSet.GetFieldValue(icolpath, varValue);
	long pathID = varValue.lVal;
	long iID = GetRelativePathFromID(pathID);
	if (iID != pathID && iID != -1)
	{
		m_mainTableSet.Edit();
		varValue.lVal = iID;
		m_mainTableSet.SetFieldValue(icolpath, varValue.lVal);
		m_mainTableSet.Update();
	}
}


void CdbWdatabase::ConvertPathTabletoRelativePath() 
{
	const CString csOrigin = GetDataBasePath();
	if (m_mainTableSet.IsBOF() && m_mainTableSet.IsEOF())
		return;

	ASSERT(m_mainTableSet.CanBookmark());
	try
	{
		COleVariant ol = m_mainTableSet.GetBookmark();
		m_mainTableSet.MoveFirst();
		int icolpath = CH_PATH_ID - 1; // m_mainTableSet.GetColumnIndex(m_mainTableSet.m_desc[CH_PATH_ID].csColName);
		int icolpath2 = CH_PATH2_ID - 1; //  m_mainTableSet.GetColumnIndex(m_mainTableSet.m_desc[CH_PATH2_ID].csColName);

		while (!m_mainTableSet.IsEOF())
		{
			ConvertPathtoRelativePath(icolpath);
			ConvertPathtoRelativePath(icolpath2);
			m_mainTableSet.MoveNext();
		}
		m_mainTableSet.SetBookmark(ol);
	}
	catch (CDaoException* e) 
	{ 
		DisplayDaoException(e, 9); 
		e->Delete(); 
	}
}


CString CdbWdatabase::GetAbsolutePathFromString(CString csPath)
{
	TCHAR szOut[MAX_PATH] = _T("");
	if (csPath.IsEmpty())
		return csPath;

	CString csRelative = m_databasePath + csPath;
	BOOL flag = PathCanonicalize(szOut, csRelative);
	CString csOut = szOut;
	if (!flag)
		csOut.Empty();
	return csOut;
}


long CdbWdatabase::GetAbsolutePathFromID(long iID)
{
	long inewID = -1;
	CString cspath = m_pathSet.GetStringFromID(iID);
	if (IsRelativePath(cspath))
	{
		CString csAbsolutePath = GetAbsolutePathFromString(cspath);
		if (!csAbsolutePath.IsEmpty())
		{
			inewID = m_pathSet.GetIDorCreateIDforString(csAbsolutePath);
		}
	}
	else
		inewID = iID;
	return inewID;
}

void CdbWdatabase::ConvertPathtoAbsolutePath(int icolpath)
{
	COleVariant varValue;
	m_mainTableSet.GetFieldValue(icolpath, varValue);
	long pathID = varValue.lVal;
	long iID = GetAbsolutePathFromID(pathID);
	if (iID != pathID && iID != -1)
	{
		m_mainTableSet.Edit();
		varValue.lVal = iID;
		m_mainTableSet.SetFieldValue(icolpath, varValue.lVal);
		m_mainTableSet.Update();
	}
}


void CdbWdatabase::ConvertPathTabletoAbsolutePath() 
{
	const CString csOrigin = GetDataBasePath();
	if (m_mainTableSet.IsBOF() && m_mainTableSet.IsEOF())
		return;

	ASSERT(m_mainTableSet.CanBookmark());
	try
	{
		COleVariant ol = m_mainTableSet.GetBookmark();
		m_mainTableSet.MoveFirst();
		int icolpath = CH_PATH_ID - 1; // m_mainTableSet.GetColumnIndex(m_mainTableSet.m_desc[CH_PATH_ID].csColName);
		int icolpath2 = CH_PATH2_ID - 1; //  m_mainTableSet.GetColumnIndex(m_mainTableSet.m_desc[CH_PATH2_ID].csColName);

		while (!m_mainTableSet.IsEOF())
		{
			ConvertPathtoAbsolutePath(icolpath);
			ConvertPathtoAbsolutePath(icolpath2);
			m_mainTableSet.MoveNext();
		}
		m_mainTableSet.SetBookmark(ol);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 9);
		e->Delete();
	}
}


void CdbWdatabase::GetFilenamesFromCurrentRecord()
{
	*m_pcurrentDataFilename = GetDatFilenameFromCurrentRecord();
	*m_pcurrentSpkFileName = GetSpkFilenameFromCurrentRecord();
}


CString CdbWdatabase::GetDatFilenameFromCurrentRecord()
{
	CString filename;
	filename.Empty();
	if (!m_mainTableSet.IsFieldNull(&m_mainTableSet.m_Filedat) && !m_mainTableSet.m_Filedat.IsEmpty())
	{
		filename = GetFilePath(m_mainTableSet.m_path_ID) + '\\' + m_mainTableSet.m_Filedat;
	}
	return filename;
}


CString CdbWdatabase::GetSpkFilenameFromCurrentRecord()
{
	CString filename;
	filename.Empty();

	// set current spkdocument
	if (!m_mainTableSet.IsFieldNull(&m_mainTableSet.m_Filespk) && !m_mainTableSet.m_Filespk.IsEmpty())
	{
		if (m_mainTableSet.IsFieldNull(&m_mainTableSet.m_path2_ID))
		{
			m_mainTableSet.Edit();
			m_mainTableSet.m_path2_ID = m_mainTableSet.m_path_ID;
			m_mainTableSet.Update();
		}
		filename = GetFilePath(m_mainTableSet.m_path2_ID) + '\\' + m_mainTableSet.m_Filespk;
	}
	return filename;
}


BOOL CdbWdatabase::MoveRecord (UINT nIDMoveCommand)
{
	UpdateTables();
	BOOL flag = TRUE;
	switch (nIDMoveCommand)
	{
		case ID_RECORD_PREV:
			m_mainTableSet.MovePrev();
			if (!m_mainTableSet.IsBOF())
				break;
			// Fall through to reset to first record
			flag = FALSE;

		case ID_RECORD_FIRST:
			m_mainTableSet.MoveFirst();
			break;

		case ID_RECORD_NEXT:
			m_mainTableSet.MoveNext();
			if (!m_mainTableSet.IsEOF())
				break;
			// Fall through to reset to last record
			flag = FALSE;

		case ID_RECORD_LAST:
			m_mainTableSet.MoveLast();
			break;

		default:
			// Unexpected case value
			ASSERT(FALSE);			
			break;
	}
	GetFilenamesFromCurrentRecord();
	return flag;
}


BOOL CdbWdatabase::SetIndexCurrentFile(long ifile)
{
	// save any pending edit or add operation
	UpdateTables();
	int nbrecords = m_mainTableSet.GetRecordCount();
	if (ifile < 0 || nbrecords < 1)
		return FALSE;

	BOOL flag = TRUE;
	try
	{
		if (ifile >= nbrecords)
			ifile = nbrecords -1;
		if (ifile != m_mainTableSet.GetAbsolutePosition())
			m_mainTableSet.SetAbsolutePosition(ifile);
		GetFilenamesFromCurrentRecord();
	}
	catch(CDaoException* e) {DisplayDaoException(e, 13); e->Delete(); flag=FALSE;}
	return flag;
}

// fill in DB_ITEMDESC structure 
// input:
// iCH = column of m_psz... (see CH_.. definitions in dbMainTable
// pointer to DB_ITEMDESC structure
// output:
// DB_ITEMDESC filled
// flag = FALSE if the column is not found

DB_ITEMDESC* CdbWdatabase::GetRecordItemDescriptor(int icol)
{
	DB_ITEMDESC* pdesc = &m_mainTableSet.m_desc[icol];
	pdesc->icol = icol;

	switch (icol)
	{
	case CH_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_ID;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_ACQDATE:
		pdesc->pdataItem	= NULL; //&m_acq_date;
		ASSERT(pdesc->typeLocal == FIELD_DATE);
		break; 
	case CH_FILENAME:
	case CH_FILESPK:
	case CH_ACQ_COMMENTS:
	case CH_MORE:
		pdesc->pdataItem	= NULL; //&m_more;
		ASSERT(pdesc->typeLocal == FIELD_TEXT);
		break;
	case CH_IDINSECT:
		pdesc->pdataItem	= &m_mainTableSet.m_IDinsect;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_IDSENSILLUM:
		pdesc->pdataItem	= &m_mainTableSet.m_IDsensillum;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_DATALEN:
		pdesc->pdataItem	= &m_mainTableSet.m_datalen;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_NSPIKES:
		pdesc->pdataItem	= &m_mainTableSet.m_nspikes;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_NSPIKECLASSES:
		pdesc->pdataItem	= &m_mainTableSet.m_nspikeclasses;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_FLAG:
		pdesc->pdataItem	= &m_mainTableSet.m_flag;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_INSECT_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_insect_ID;
		pdesc->plinkedSet	= &m_insectSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_SENSILLUM_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_sensillum_ID;
		pdesc->plinkedSet	= &m_sensillumSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_OPERATOR_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_operator_ID;
		pdesc->plinkedSet	= &m_operatorSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_STIM_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_stim_ID;
		pdesc->plinkedSet	= &m_stimSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_CONC_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_conc_ID;
		pdesc->plinkedSet	= &m_concSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_LOCATION_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_location_ID;
		pdesc->plinkedSet	= &m_locationSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_PATH_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_path_ID;
		pdesc->plinkedSet	= &m_pathSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_FILEPATH);
		break;
	case CH_PATH2_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_path2_ID;
		pdesc->plinkedSet	= &m_pathSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_FILEPATH);
		break;
	case CH_STIM2_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_stim2_ID;
		pdesc->plinkedSet	= &m_stimSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_CONC2_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_conc2_ID;
		pdesc->plinkedSet	= &m_concSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_STRAIN_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_strain_ID;
		pdesc->plinkedSet	= &m_strainSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_SEX_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_sex_ID;
		pdesc->plinkedSet	= &m_sexSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_REPEAT:
		pdesc->pdataItem	= &m_mainTableSet.m_repeat;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_REPEAT2:
		pdesc->pdataItem	= &m_mainTableSet.m_repeat2;
		ASSERT(pdesc->typeLocal == FIELD_LONG);
		break;
	case CH_ACQDATE_DAY:
		pdesc->pdataItem	= NULL; //&m_acqdate_day;
		ASSERT(pdesc->typeLocal == FIELD_DATE_YMD);
		break;
	case CH_ACQDATE_TIME:
		pdesc->pdataItem	= NULL; //&m_acqdate_time;
		ASSERT(pdesc->typeLocal == FIELD_DATE_HMS);
		break;
	case CH_EXPT_ID:
		pdesc->pdataItem	= &m_mainTableSet.m_expt_ID;
		pdesc->plinkedSet	= &m_exptSet;
		ASSERT(pdesc->typeLocal == FIELD_IND_TEXT);
		break;

		// if it comes here, the program must have crashed because pdesc is not defined...
	default:
		return NULL;
		break;
	}
	////////////

	return pdesc;
}


BOOL CdbWdatabase::GetRecordItemValue(int ich, DB_ITEMDESC* pdesc)
{
	BOOL flag = TRUE;
	COleVariant varValue;
	m_mainTableSet.GetFieldValue(m_mainTableSet.m_desc[ich].csColName, varValue);
	switch (ich)
	{
	case CH_INSECT_ID:
	case CH_SENSILLUM_ID:
	case CH_OPERATOR_ID:
	case CH_STIM_ID:
	case CH_CONC_ID:
	case CH_LOCATION_ID:
	case CH_PATH_ID:
	case CH_PATH2_ID:
	case CH_STIM2_ID:
	case CH_CONC2_ID:
	case CH_STRAIN_ID:
	case CH_SEX_ID:
		pdesc->lVal = varValue.lVal;
		pdesc->csVal = m_mainTableSet.m_desc[ich].plinkedSet->GetStringFromID(varValue.lVal);
		break;

	case CH_NSPIKES:
	case CH_NSPIKECLASSES:
	case CH_IDSENSILLUM:
	case CH_IDINSECT:
	case CH_ID:
	case CH_DATALEN:
	case CH_FLAG:
	case CH_REPEAT2:
	case CH_REPEAT:
		pdesc->lVal = varValue.lVal;
		if (varValue.vt == VT_NULL)
			pdesc->lVal = 0;
		break;

	case CH_ACQDATE:
	case CH_ACQDATE_DAY:
	case CH_ACQDATE_TIME:
		pdesc->oVal = varValue.date;
		break;

	case CH_FILENAME:
	case CH_FILESPK:
	case CH_ACQ_COMMENTS:
	case CH_MORE:
		m_mainTableSet.GetFieldValue(m_mainTableSet.m_desc[ich].csColName, varValue);
		pdesc->csVal = V_BSTRT(&varValue);
		break;

	case CH_EXPT_ID:
		pdesc->lVal = varValue.lVal;
		pdesc->csVal = m_mainTableSet.m_desc[ich].plinkedSet->GetStringFromID(varValue.lVal);
		if (pdesc->csVal.IsEmpty())
		{
			CString cs = pdesc->csVal = GetName();
			int left = cs.ReverseFind(_T('\\'));
			int right = cs.ReverseFind(_T('.'));
			pdesc->csVal = cs.Mid(left + 1, right - left - 1);
		}
		break;
	default:
		flag = FALSE;
		break;
	}
	////////////
	return flag;
}


BOOL CdbWdatabase::SetRecordItemValue(int ich, DB_ITEMDESC* pdesc)
{
	BOOL flag = TRUE;
	long dummyID;
	COleVariant varValue;

	switch (ich)
	{
	case CH_ID:
	case CH_IDINSECT:
	case CH_IDSENSILLUM:
	case CH_DATALEN:
	case CH_NSPIKES:
	case CH_NSPIKECLASSES:
	case CH_FLAG:
	case CH_REPEAT:
	case CH_REPEAT2:
		varValue = pdesc->lVal;
		m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[ich].csColName, varValue);
		break;

	case CH_ACQDATE_DAY:
	case CH_ACQDATE_TIME:
	case CH_ACQDATE:
		varValue = pdesc->oVal;
		m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[ich].csColName, varValue);
		break;

	case CH_FILENAME:
	case CH_FILESPK:
	case CH_ACQ_COMMENTS:
	case CH_MORE:
		varValue = pdesc->csVal;
		m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[ich].csColName, varValue);
		break;

	case CH_INSECT_ID:
	case CH_SENSILLUM_ID:
	case CH_OPERATOR_ID:
	case CH_STIM_ID:
	case CH_CONC_ID:
	case CH_LOCATION_ID:
	case CH_PATH_ID:
	case CH_PATH2_ID:
	case CH_STIM2_ID:
	case CH_CONC2_ID:
	case CH_STRAIN_ID:
	case CH_SEX_ID:
	case CH_EXPT_ID:
		dummyID = ((CdbIndexTable*)m_mainTableSet.m_desc[ich].plinkedSet)->GetIDorCreateIDforString(pdesc->csVal);
		if (dummyID >= 0)
		{
			varValue.lVal = dummyID;
			m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[ich].csColName, varValue.lVal);
		}
		break;

	default:
		flag = FALSE;
		break;
	}
	////////////
	return flag;
}


BOOL CdbWdatabase::ImportRecordfromDatabase(CdbWdatabase* pdbW)
{
	// variables
	DB_ITEMDESC desc;
	BOOL flag = TRUE;
	// insert new record
	m_mainTableSet.AddNew();

	// copy each field of the source database into current
	for (int i=1; i< m_mainTableSet.m_nFields; i++)
	{
		pdbW->GetRecordItemValue(i, &desc);
		SetRecordItemValue(i, &desc);
	}
	m_mainTableSet.Update();
	return flag;
}


void CdbWdatabase::TransferWaveFormatDataToRecord(CWaveFormat * pWF)
{
	// set time -- oTime
	COleDateTime oTime;
	CTime t = pWF->acqtime;
	oTime.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_acq_date), FALSE);
	m_mainTableSet.m_acq_date = oTime;
	m_mainTableSet.m_acqdate_time = oTime;
	oTime.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), 0, 0, 0);
	m_mainTableSet.m_acqdate_day = oTime;

	// set insect ID, sensillumID, repeat and repeat2, moreComment
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_IDinsect), FALSE);
	m_mainTableSet.m_IDinsect = pWF->insectID;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_IDsensillum), FALSE);
	m_mainTableSet.m_IDsensillum = pWF->sensillumID;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_repeat), FALSE);
	m_mainTableSet.m_repeat = pWF->repeat;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_repeat2), FALSE);
	m_mainTableSet.m_repeat2 = pWF->repeat2;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_more), FALSE);
	m_mainTableSet.m_more = pWF->csMoreComment;

	// set type, stimulus and concentrations
	m_mainTableSet.m_operator_ID	= m_operatorSet.GetIDorCreateIDforString(pWF->csOperator);
	m_mainTableSet.m_insect_ID		= m_insectSet.GetIDorCreateIDforString(pWF->csInsectname);
	m_mainTableSet.m_location_ID	= m_locationSet.GetIDorCreateIDforString(pWF->csLocation);
	m_mainTableSet.m_expt_ID		= m_exptSet.GetIDorCreateIDforString(pWF->csComment);
	m_mainTableSet.m_sensillum_ID	= m_sensillumSet.GetIDorCreateIDforString(pWF->csSensillum);
	m_mainTableSet.m_stim_ID		= m_stimSet.GetIDorCreateIDforString(pWF->csStimulus);
	m_mainTableSet.m_conc_ID		= m_concSet.GetIDorCreateIDforString(pWF->csConcentration);
	m_mainTableSet.m_stim2_ID		= m_stimSet.GetIDorCreateIDforString(pWF->csStimulus2);
	m_mainTableSet.m_conc2_ID		= m_concSet.GetIDorCreateIDforString(pWF->csConcentration2);
	m_mainTableSet.m_sex_ID			= m_sexSet.GetIDorCreateIDforString(pWF->csSex);
	m_mainTableSet.m_strain_ID		= m_strainSet.GetIDorCreateIDforString(pWF->csStrain);
	m_mainTableSet.m_expt_ID		= m_exptSet.GetIDorCreateIDforString(pWF->csComment);
	m_mainTableSet.m_flag = 0;
}

void CdbWdatabase::DeleteUnusedEntriesInAccessoryTables()
{
	DeleteUnusedEntriesInAttachedTable(&m_operatorSet,	CH_OPERATOR_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_insectSet,	CH_INSECT_ID,	-1);
	DeleteUnusedEntriesInAttachedTable(&m_locationSet,	CH_LOCATION_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_sensillumSet, CH_SENSILLUM_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_stimSet,		CH_STIM_ID, CH_STIM2_ID);
	DeleteUnusedEntriesInAttachedTable(&m_concSet,		CH_CONC_ID, CH_CONC2_ID);
	DeleteUnusedEntriesInAttachedTable(&m_sexSet,		CH_SEX_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_strainSet,	CH_STRAIN_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_exptSet,		CH_EXPT_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_pathSet,		CH_PATH_ID, CH_PATH2_ID);
}

void CdbWdatabase::DeleteUnusedEntriesInAttachedTable(CdbIndexTable* pIndexTable, int column1, int column2)
{
	COleVariant varValue0, varValue1;

	if(pIndexTable->IsBOF() && pIndexTable->IsEOF())
		return ;

	pIndexTable->MoveFirst();
	while (!pIndexTable->IsEOF())
	{
		// pIndexTable->GetFieldValue(0, varValue0);
		pIndexTable->GetFieldValue(1, varValue1);
		long iIDcurrent = varValue1.lVal;
		BOOL flag1 = m_mainTableSet.FindIDinColumn(iIDcurrent, column1);
		BOOL flag2 = FALSE;
		if (column2 >= 0)
			flag2 = m_mainTableSet.FindIDinColumn(iIDcurrent, column2);
		if (flag1 == FALSE && flag2 == FALSE) {
			pIndexTable->Delete();
		}
		pIndexTable->MoveNext();
	}
}

