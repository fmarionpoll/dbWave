#include "StdAfx.h"
#include "dbWaveDatabaseIndexTable.h"
#include "dbWaveDataBaseMainTable.h"
#include "dbWaveDatabase.h"

#include "dbWave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


database_column_properties CdbWaveDatabase::m_desctab[NTABLECOLS] =
{
	//table col header---friendly description for properties--type of field--name of attached table
	CH_ID, _T("ID"), _T("ID record"), FIELD_LONG, _T(""), // 0
	CH_ACQDATE, _T("acq_date"), _T("acquisition date & time"), FIELD_DATE, _T(""), // 1
	CH_FILENAME, _T("filename"), _T("dat file"), FIELD_TEXT, _T(""), // 2
	CH_FILESPK, _T("filespk"), _T("spk file"), FIELD_TEXT, _T(""), // 3
	CH_ACQ_COMMENTS, _T("acq_comment"), _T("comment"), FIELD_TEXT, _T(""), // 4
	CH_MORE, _T("more"), _T("comment"), FIELD_TEXT, _T(""), // 5
	CH_IDINSECT, _T("insectID"), _T("ID insect"), FIELD_LONG, _T(""), // 6
	CH_IDSENSILLUM, _T("sensillumID"), _T("ID sensillum"), FIELD_LONG, _T(""), // 7
	CH_DATALEN, _T("datalen"), _T("data length (n points)"), FIELD_LONG, _T(""), // 8
	CH_NSPIKES, _T("nspikes"), _T("n spikes detected"), FIELD_LONG, _T(""), // 9
	CH_NSPIKECLASSES, _T("nspikeclasses"),_T("n spike classes"), FIELD_LONG, _T(""), // 10
	CH_FLAG, _T("flag"), _T("flag"), FIELD_LONG, _T(""), // 11
	CH_INSECT_ID, _T("insectname_ID"),_T("insect"), FIELD_IND_TEXT, _T("insectname"), // 12
	CH_SENSILLUM_ID, _T("sensillumname_ID"),_T("sensillum"), FIELD_IND_TEXT, _T("sensillumname"), // 13
	CH_OPERATOR_ID, _T("operator_ID"), _T("operator"), FIELD_IND_TEXT, _T("operator"), // 14
	CH_STIM_ID, _T("stim_ID"), _T("compound(1)"), FIELD_IND_TEXT, _T("stim"), // 15
	CH_CONC_ID, _T("conc_ID"), _T("concentration(1)"), FIELD_IND_TEXT, _T("conc"), // 16
	CH_LOCATION_ID, _T("location_ID"), _T("location"), FIELD_IND_TEXT, _T("location"), // 17
	CH_PATH_ID, _T("path_ID"), _T("path (*.dat)"), FIELD_IND_FILEPATH, _T("path"), // 18
	CH_PATH2_ID, _T("path2_ID"), _T("path (*.spk)"), FIELD_IND_FILEPATH, _T("path"), // 19
	CH_STIM2_ID, _T("stim2_ID"), _T("compound(2)"), FIELD_IND_TEXT, _T("stim"), // 20
	CH_CONC2_ID, _T("conc2_ID"), _T("concentration(2)"), FIELD_IND_TEXT, _T("conc"), // 21
	CH_STRAIN_ID, _T("strain_ID"), _T("insect strain"), FIELD_IND_TEXT, _T("strain"), // 22
	CH_SEX_ID, _T("sex_ID"), _T("insect sex"), FIELD_IND_TEXT, _T("sex"), // 23
	CH_REPEAT, _T("repeat"), _T("repeat(1)"), FIELD_LONG, _T(""), // 24
	CH_REPEAT2, _T("repeat2"), _T("repeat(2)"), FIELD_LONG, _T(""), // 25
	CH_ACQDATE_DAY, _T("acqdate_day"), _T("date"), FIELD_DATE_YMD, _T(""), // 26
	CH_ACQDATE_TIME, _T("acqdate_time"), _T("time"), FIELD_DATE_HMS, _T(""), // 27
	CH_EXPT_ID, _T("expt_ID"), _T("Experiment"), FIELD_IND_TEXT, _T("expt") // 28
};

CdbWaveDatabase::CdbWaveDatabase() 
{
	for (auto i = 0; i < NTABLECOLS; i++)
	{
		CString dummy = m_desctab[i].header_name;
		m_mainTableSet.m_desc[i].header_name = dummy;
		m_mainTableSet.m_desc[i].dfx_name_with_brackets = _T("[") + dummy + _T("]");
		m_mainTableSet.m_desc[i].csColParam = dummy + _T("Param");
		m_mainTableSet.m_desc[i].csEQUcondition = dummy + _T("=") + m_mainTableSet.m_desc[i].csColParam;
		m_mainTableSet.m_desc[i].typeLocal = m_desctab[i].format_code_number;
	}

	// SetNames(CString defaultSQL /* or table name*/, CString DFX_cs, CString DFX_ID)
	m_stimSet.SetNames(m_desctab[CH_STIM_ID].attached_table, _T("stim"), _T("stimID"));
	m_concSet.SetNames(m_desctab[CH_CONC_ID].attached_table, _T("conc"), _T("concID"));
	m_operatorSet.SetNames(m_desctab[CH_OPERATOR_ID].attached_table, _T("operator"), _T("operatorID"));
	m_insectSet.SetNames(m_desctab[CH_INSECT_ID].attached_table, _T("insect"), _T("insectID"));
	m_locationSet.SetNames(m_desctab[CH_LOCATION_ID].attached_table, _T("type"), _T("typeID"));
	m_pathSet.SetNames(m_desctab[CH_PATH_ID].attached_table, _T("path"), _T("pathID"));
	m_sensillumSet.SetNames(m_desctab[CH_SENSILLUM_ID].attached_table, _T("stage"), _T("stageID"));
	m_sexSet.SetNames(m_desctab[CH_SEX_ID].attached_table, _T("sex"), _T("sexID"));
	m_strainSet.SetNames(m_desctab[CH_STRAIN_ID].attached_table, _T("strain"), _T("strainID"));
	m_exptSet.SetNames(m_desctab[CH_EXPT_ID].attached_table, _T("expt"), _T("exptID"));

	m_mainTableSet.m_strSort = m_desctab[CH_ACQDATE].header_name;
}

void CdbWaveDatabase::Attach(CString* pstrDataName, CString* pstrSpkName)
{
	m_current_data_filename = pstrDataName;
	m_p_current_spike_filename = pstrSpkName;
}

CdbWaveDatabase::~CdbWaveDatabase()
{
	CloseDatabase();
}

// CdbWaveDatabase member functions

BOOL CdbWaveDatabase::CreateMainTable(CString csTable)
{
	CDaoTableDef table_def(this);
	table_def.Create(csTable);

	//first create the main field, and ID number that is incremented automatically
	// when a new record is created. This column is indexed
	CDaoFieldInfo fd0;
	fd0.m_strName = m_mainTableSet.m_desc[CH_ID].header_name; // "ID"
	fd0.m_nType = dbLong; // Primary
	fd0.m_lSize = 4;
	fd0.m_lAttributes = dbAutoIncrField;

	fd0.m_nOrdinalPosition = 1; // Secondary
	fd0.m_bRequired = TRUE;
	fd0.m_bAllowZeroLength = FALSE;
	fd0.m_lCollatingOrder = dbSortGeneral;
	fd0.m_strForeignName = _T("");
	fd0.m_strSourceField = _T("");
	fd0.m_strSourceTable = _T("");

	fd0.m_strValidationRule = _T(""); // All
	fd0.m_strValidationText = _T("");
	fd0.m_strDefaultValue = _T("");
	table_def.CreateField(fd0); // 0 - iID

	// then create data fields
	auto i = 1;
	table_def.CreateField(m_mainTableSet.m_desc[i].header_name, dbDate, 8, 0); // 1 -acq_date

	fd0.m_bAllowZeroLength = TRUE;
	fd0.m_bRequired = FALSE;
	fd0.m_lAttributes = dbVariableField;
	fd0.m_nType = dbText;
	fd0.m_lSize = 255;

	for (i = 2; i <= 4; i++) // 2 -filename / 3 -filespk // 4 - "acq_comment"
	{
		fd0.m_strName = m_mainTableSet.m_desc[i].header_name;
		fd0.m_nOrdinalPosition = i;
		table_def.CreateField(fd0);
	}

	i = 5; // 5 - "more"
	fd0.m_strName = m_mainTableSet.m_desc[i].header_name;
	fd0.m_nOrdinalPosition = i;
	fd0.m_nType = dbMemo;
	fd0.m_lSize = dbMemo;
	table_def.CreateField(fd0);

	for (i = 6; i <= 25; i++)
		table_def.CreateField(m_mainTableSet.m_desc[i].header_name, dbLong, 4, 0); //  6 - insectID to 25 = sex_ID
	for (i = 26; i <= 27; i++)
		table_def.CreateField(m_mainTableSet.m_desc[i].header_name, dbDate, 8, 0); // 26 - acqdate_day / 27 - acqdate_time
	i = 28;
	table_def.CreateField(m_mainTableSet.m_desc[i].header_name, dbLong, 4, 0); // 28 - expt_ID

	// create the corresponding indexes
	CDaoIndexFieldInfo index_field0;
	//index_field0.m_strName = m_mainTableSet.m_desc[CH_IDINSECT].header_name;			// insectID
	index_field0.m_bDescending = FALSE;

	CDaoIndexInfo index_fd0;
	index_field0.m_strName = m_mainTableSet.m_desc[CH_ID].header_name; // ID
	index_fd0.m_strName = _T("Primary_Key");
	index_fd0.m_pFieldInfos = &index_field0;
	index_fd0.m_bPrimary = TRUE;
	index_fd0.m_bUnique = TRUE;
	index_fd0.m_bRequired = TRUE;
	index_fd0.m_bForeign = FALSE;
	index_fd0.m_nFields = 1;
	index_fd0.m_bClustered = FALSE;
	index_fd0.m_bIgnoreNulls = TRUE; // previously: FALSE;
	table_def.CreateIndex(index_fd0);
	table_def.Append();

	// create relations
	const long l_attr = dbRelationDontEnforce; //dbRelationUpdateCascade;
	if (!CreateRelationwithAssocTable(csTable, CH_INSECT_ID, l_attr, &m_insectSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_SENSILLUM_ID, l_attr, &m_sensillumSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_OPERATOR_ID, l_attr, &m_operatorSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_LOCATION_ID, l_attr, &m_locationSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_STRAIN_ID, l_attr, &m_strainSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_SEX_ID, l_attr, &m_sexSet)) return FALSE;
	if (!CreateRelationwithAssocTable(csTable, CH_EXPT_ID, l_attr, &m_exptSet)) return FALSE;

	if (!CreateRelationwith2AssocTables(csTable, CH_PATH_ID, CH_PATH2_ID)) return FALSE;
	if (!CreateRelationwith2AssocTables(csTable, CH_STIM_ID, CH_STIM2_ID)) return FALSE;
	if (!CreateRelationwith2AssocTables(csTable, CH_CONC_ID, CH_CONC2_ID)) return FALSE;

	return TRUE;
}

BOOL CdbWaveDatabase::CreateRelationwith2AssocTables(const LPCTSTR lpsz_foreign_table, const int column_first,
                                                  const int column_last)
{
	try
	{
		const LPCTSTR lpsz_table = m_desctab[column_first].attached_table;
		CDaoRelationInfo rl_info;
		rl_info.m_strTable = lpsz_table;
		rl_info.m_strName.Format(_T("%s_%s"), lpsz_foreign_table, lpsz_table);
		rl_info.m_strForeignTable = lpsz_foreign_table;
		rl_info.m_lAttributes = dbRelationDontEnforce;

		CDaoRelationFieldInfo r_field[2];
		r_field[0].m_strName = lpsz_table;
		r_field[0].m_strName += _T("ID");
		r_field[0].m_strForeignName = m_mainTableSet.m_desc[column_first].header_name; // path_ID
		r_field[1].m_strName = r_field[0].m_strName;
		r_field[1].m_strForeignName = m_mainTableSet.m_desc[column_last].header_name; // path2_ID
		rl_info.m_pFieldInfos = &r_field[0];
		rl_info.m_nFields = 2;
		CreateRelation(rl_info);

		m_mainTableSet.m_desc[column_first].plinkedSet = &m_pathSet;
		m_mainTableSet.m_desc[column_first].associated_table_name = r_field[0].m_strName;

		m_mainTableSet.m_desc[column_last].plinkedSet = &m_pathSet;
		m_mainTableSet.m_desc[column_last].associated_table_name = r_field[0].m_strName;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 5);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

// insect name: "table_insectname" relates "insect" table with "ID" and "insectname_ID"
BOOL CdbWaveDatabase::CreateRelationwithAssocTable(const LPCTSTR lpsz_foreign_table, const int column,
                                                const long l_attributes, CdbIndexTable* p_link)
{
	try
	{
		const LPCTSTR lpsz_table = m_desctab[column].attached_table;
		m_mainTableSet.m_desc[column].associated_table_name = lpsz_table;
		CString cs_rel; // unique name of the relation object (max 40 chars)
		cs_rel.Format(_T("%s_%s"), lpsz_foreign_table, lpsz_table);
		CString sz_field = lpsz_table;
		sz_field += _T("ID");
		CreateRelation(cs_rel, lpsz_table, lpsz_foreign_table, l_attributes, sz_field,
		               m_mainTableSet.m_desc[column].header_name);
		m_mainTableSet.m_desc[column].plinkedSet = p_link;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 5);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

CString CdbWaveDatabase::GetDataBasePath()
{
	return m_database_path;
}

void CdbWaveDatabase::SetDataBasePath()
{
	auto filename = GetName();
	const auto last_slash = filename.ReverseFind('\\');
	m_database_path = filename.Left(last_slash + 1);
	m_database_path.MakeLower();
}

void CdbWaveDatabase::CreateTables()
{
	SetDataBasePath();

	// create tables with indexes. These tables will contain categories entered by the user
	m_insectSet.CreateIndextable(_T("insectname"), _T("insect"), _T("insectID"), 100, this);
	m_sensillumSet.CreateIndextable(_T("sensillumname"), _T("stage"), _T("stageID"), 100, this);
	m_operatorSet.CreateIndextable(_T("operator"), _T("operator"), _T("operatorID"), 50, this);
	m_stimSet.CreateIndextable(_T("stim"), _T("stim"), _T("stimID"), 100, this);
	m_concSet.CreateIndextable(_T("conc"), _T("conc"), _T("concID"), 100, this);
	m_locationSet.CreateIndextable(_T("location"), _T("type"), _T("typeID"), 100, this);
	m_pathSet.CreateIndextable(_T("path"), _T("path"), _T("pathID"), 255, this);
	m_strainSet.CreateIndextable(_T("strain"), _T("strain"), _T("strainID"), 100, this);
	m_sexSet.CreateIndextable(_T("sex"), _T("sex"), _T("sexID"), 10, this);
	m_exptSet.CreateIndextable(_T("expt"), _T("expt"), _T("exptID"), 100, this);

	CreateMainTable(_T("table"));

	// store links to attached table addresses
	for (auto column = 0; column < NTABLECOLS; column++)
		GetRecordItemDescriptor(column);
}

BOOL CdbWaveDatabase::OpenTables()
{
	SetDataBasePath();

	// check format of table set
	CDaoRecordset record_set(this);
	const CString cs_table = _T("table");

	// check for the presence of Table 'table'
	try
	{
		CDaoFieldInfo fd0;
		record_set.Open(dbOpenTable, cs_table);
		// check if column "filename" is present
		record_set.GetFieldInfo(m_mainTableSet.m_desc[CH_FILENAME].header_name, fd0);
		// check number of columns
		const int field_count = record_set.GetFieldCount();
		record_set.Close();

		// less columns are present? add missing columns
		CdbMainTable rs2; // CDaoRecordSet
		if (field_count < rs2.m_nFields)
		{
			AfxMessageBox(_T("This is a valid dbWave database\nbut its version must be upgraded...\nOK?"));
			// open table definition
			CDaoTableDef table_def(this);
			CString cs_rel;
			constexpr long l_attr = dbRelationUpdateCascade;

			switch (field_count)
			{
			case 19:
				add_column_19_20(table_def, cs_table, l_attr);
			case 21:
				add_column_21(table_def, cs_table, l_attr);
			case 22:
				add_column_22_23(table_def, cs_table, l_attr);
			case 24:
				add_column_24_25(table_def, cs_table, l_attr);
			case 26:
				add_column_26_27(table_def, cs_table, l_attr);
			case 28:
				add_column_28(table_def, cs_table, l_attr);
				break;

			default:
				return FALSE;
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
		OpenIndexTable(&m_stimSet);
		OpenIndexTable(&m_concSet);
		OpenIndexTable(&m_operatorSet);
		OpenIndexTable(&m_insectSet);
		OpenIndexTable(&m_locationSet);
		OpenIndexTable(&m_pathSet);
		OpenIndexTable(&m_sensillumSet);
		OpenIndexTable(&m_sexSet);
		OpenIndexTable(&m_strainSet);
		OpenIndexTable(&m_exptSet);
	}
	catch (CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
		return FALSE;
	}

	m_mainTableSet.m_defaultName = GetName();
	if (!m_mainTableSet.OpenTable(dbOpenDynaset, nullptr, 0))
		return FALSE;

	// store links to attached table addresses
	for (auto column = 0; column < NTABLECOLS; column++)
		GetRecordItemDescriptor(column);

	return TRUE;
}

void CdbWaveDatabase::add_column_28(CDaoTableDef& table_def, CString cs_table, long l_attr)
{
	table_def.Open(cs_table);
	table_def.CreateField(m_mainTableSet.m_desc[CH_EXPT_ID].header_name, dbLong, 4, 0); // expt_ID
	m_exptSet.CreateIndextable(_T("expt"), _T("expt"), _T("exptID"), 100, this);
	CreateRelation(_T("table_expt"), _T("expt"), cs_table, l_attr, _T("exptID"),
		m_mainTableSet.m_desc[CH_EXPT_ID].header_name); // strain_ID
	table_def.Close();
}

void CdbWaveDatabase::add_column_26_27(CDaoTableDef& table_def, CString cs_table, long l_attr)
{
	table_def.Open(cs_table);
	table_def.CreateField(m_mainTableSet.m_desc[CH_ACQDATE_DAY].header_name, dbDate, 8, 0); // acqdate_day
	table_def.CreateField(m_mainTableSet.m_desc[CH_ACQDATE_TIME].header_name, dbDate, 8, 0); // acqdate_time
	table_def.Close();
}

void CdbWaveDatabase::add_column_24_25(CDaoTableDef& table_def, CString cs_table, long l_attr)
{
	table_def.Open(cs_table);
	table_def.CreateField(m_mainTableSet.m_desc[CH_REPEAT].header_name, dbLong, 4, 0); // repeatID
	table_def.CreateField(m_mainTableSet.m_desc[CH_REPEAT2].header_name, dbLong, 4, 0); // repeat2ID
	table_def.Close();
}

void CdbWaveDatabase::add_column_22_23(CDaoTableDef& table_def, CString cs_table, long l_attr)
{
	table_def.Open(cs_table);

	// add fields in the maintable, add the corresponding tables and the relations between the main table and the new index tables
	table_def.CreateField(m_mainTableSet.m_desc[CH_STRAIN_ID].header_name, dbLong, 4, 0); // strain_ID
	table_def.CreateField(m_mainTableSet.m_desc[CH_SEX_ID].header_name, dbLong, 4, 0); // sex_ID
	m_strainSet.CreateIndextable(_T("strain"), _T("strain"), _T("strainID"), 100, this);
	m_sexSet.CreateIndextable(_T("sex"), _T("sex"), _T("sexID"), 10, this);
	CreateRelation(_T("table_strain"), _T("strain"), cs_table, l_attr, _T("strainID"),
		m_mainTableSet.m_desc[CH_STRAIN_ID].header_name); // strain_ID
	CreateRelation(_T("table_sex"), _T("sex"), cs_table, l_attr, _T("sexID"),
		m_mainTableSet.m_desc[CH_SEX_ID].header_name); // sex_ID
// type -> location
	DeleteRelation(_T("table_type")); // delete relationship
	table_def.DeleteField(CH_LOCATION_ID);
	// delete the field (index is different because we deleted one field)
	table_def.CreateField(m_mainTableSet.m_desc[CH_LOCATION_ID].header_name, dbLong, 4, 0); // locationID
	// stage -> sensillumname
	DeleteRelation(_T("table_stage")); // delete relationship
	table_def.DeleteField(CH_SENSILLUM_ID); // delete field
	table_def.CreateField(m_mainTableSet.m_desc[CH_SENSILLUM_ID].header_name, dbLong, 4, 0);
	// sensillumID
	table_def.Close();

	// rename table stage into sensillumname
	table_def.Open(_T("stage"));
	table_def.SetName(_T("sensillumname"));
	table_def.Close();

	// rename table type into location
	table_def.Open(_T("type"));
	table_def.SetName(_T("location"));
	table_def.Close();

	// rename existing fields into "sensillumname" table (sensillum/sensillumID instead of stage/stageID)
	// rename existing fields into "location" table (location/locationID instead type/typeID)
	// create relations
	table_def.Open(cs_table);
	CString cs_rel = _T("table_sensillumname");
	CreateRelation(cs_rel, _T("sensillumname"), cs_table, l_attr, _T("stageID"),
		m_mainTableSet.m_desc[CH_SENSILLUM_ID].header_name); // sensillumname_ID
	cs_rel = _T("table_location");
	CreateRelation(cs_rel, _T("location"), cs_table, l_attr, _T("typeID"),
		m_mainTableSet.m_desc[CH_LOCATION_ID].header_name); //location_ID
	table_def.Close();
}

void CdbWaveDatabase::add_column_21(CDaoTableDef& table_def, CString cs_table, long l_attr)
{
	table_def.Open(cs_table);
	table_def.CreateField(m_mainTableSet.m_desc[CH_FLAG].header_name, dbLong, 4, 0); // flag
	table_def.Close();
}

void CdbWaveDatabase::add_column_19_20(CDaoTableDef& table_def, CString cs_table, long l_attr)
{
	table_def.Open(cs_table);
	CString cs_rel = _T("table_Rel1");
	const auto i_pos = cs_rel.GetLength() - 1;
	table_def.CreateField(m_mainTableSet.m_desc[CH_STIM2_ID].header_name, dbLong, 4, 0); // stim2_ID
	table_def.CreateField(m_mainTableSet.m_desc[CH_CONC2_ID].header_name, dbLong, 4, 0); // conc2_ID
	cs_rel.SetAt(i_pos, '9');
	CreateRelation(cs_rel, _T("stim"), cs_table, l_attr, _T("stimID"),
		m_mainTableSet.m_desc[CH_STIM2_ID].header_name); // stim2_ID
	cs_rel.SetAt(i_pos, 'A');
	CreateRelation(cs_rel, _T("conc"), cs_table, l_attr, _T("concID"),
		m_mainTableSet.m_desc[CH_CONC2_ID].header_name); // conc2_ID
	table_def.Close();
}

void CdbWaveDatabase::OpenIndexTable(CdbIndexTable* p_index_table_set)
{
	p_index_table_set->m_defaultName = GetName();
	p_index_table_set->Open(dbOpenTable, nullptr, 0);
}

void CdbWaveDatabase::CloseDatabase()
{
	if (IsOpen())
	{
		// closing the workspace does not close the recordsets
		// so first close them
		if (m_mainTableSet.IsOpen()) m_mainTableSet.Close();
		if (m_operatorSet.IsOpen()) m_operatorSet.Close();
		if (m_insectSet.IsOpen()) m_insectSet.Close();
		if (m_locationSet.IsOpen()) m_locationSet.Close();
		if (m_sensillumSet.IsOpen()) m_sensillumSet.Close();
		if (m_pathSet.IsOpen()) m_pathSet.Close();
		if (m_stimSet.IsOpen()) m_stimSet.Close();
		if (m_concSet.IsOpen()) m_concSet.Close();
		if (m_sexSet.IsOpen()) m_sexSet.Close();
		if (m_strainSet.IsOpen()) m_strainSet.Close();
		if (m_exptSet.IsOpen()) m_exptSet.Close();

		// close the workspace
		m_pWorkspace->Close();
		Close();
	}
}

void CdbWaveDatabase::UpdateTables()
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

BOOL CdbWaveDatabase::MoveToID(const long record_id)
{
	CString str;
	str.Format(_T("ID=%li"), record_id);
	try
	{
		if (!m_mainTableSet.FindFirst(str))
			return FALSE;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 36);
		e->Delete();
	}
	GetFilenamesFromCurrentRecord();
	return TRUE;
}

CString CdbWaveDatabase::GetFilePath(const int i_id)
{
	auto cs_path = m_pathSet.GetStringFromID(i_id);
	if (IsRelativePath(cs_path))
		cs_path = m_database_path + cs_path;
	return cs_path;
}

CString CdbWaveDatabase::GetRelativePathFromString(const CString& cs_path)
{
	TCHAR sz_out[MAX_PATH] = _T("");
	if (cs_path.IsEmpty())
		return cs_path;

	const auto flag = PathRelativePathTo(sz_out, m_database_path, FILE_ATTRIBUTE_DIRECTORY, cs_path,
	                                     FILE_ATTRIBUTE_DIRECTORY);
	CString cs_out = sz_out;
	if (!flag)
		cs_out.Empty();

	return cs_out;
}

long CdbWaveDatabase::GetRelativePathFromID(const long i_id)
{
	long inew_id = -1;
	const auto cs_path = m_pathSet.GetStringFromID(i_id);
	if (!IsRelativePath(cs_path))
	{
		auto cs_relative_path = GetRelativePathFromString(cs_path);
		if (!cs_relative_path.IsEmpty())
		{
			inew_id = m_pathSet.GetStringInLinkedTable(cs_relative_path);
		}
	}
	else
		inew_id = i_id;
	return inew_id;
}

void CdbWaveDatabase::ConvertPathtoRelativePath(const long i_col_path)
{
	COleVariant var_value;
	m_mainTableSet.GetFieldValue(i_col_path, var_value);
	const auto path_id = var_value.lVal;
	const auto i_id = GetRelativePathFromID(path_id);
	if (i_id != path_id && i_id != -1)
	{
		m_mainTableSet.Edit();
		var_value.lVal = i_id;
		m_mainTableSet.SetFieldValue(i_col_path, var_value.lVal);
		m_mainTableSet.Update();
	}
}

void CdbWaveDatabase::ConvertPathTabletoRelativePath()
{
	const auto cs_origin = GetDataBasePath();
	if (m_mainTableSet.IsBOF() && m_mainTableSet.IsEOF())
		return;

	ASSERT(m_mainTableSet.CanBookmark());
	try
	{
		const auto ol = m_mainTableSet.GetBookmark();
		m_mainTableSet.MoveFirst();
		const auto i_col_path = CH_PATH_ID - 1;
		// m_mainTableSet.GetColumnIndex(m_mainTableSet.m_desc[CH_PATH_ID].header_name);
		const auto i_col_path2 = CH_PATH2_ID - 1;
		//  m_mainTableSet.GetColumnIndex(m_mainTableSet.m_desc[CH_PATH2_ID].header_name);

		while (!m_mainTableSet.IsEOF())
		{
			ConvertPathtoRelativePath(i_col_path);
			ConvertPathtoRelativePath(i_col_path2);
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

CString CdbWaveDatabase::GetAbsolutePathFromString(CString cs_path)
{
	TCHAR sz_out[MAX_PATH] = _T("");
	if (cs_path.IsEmpty())
		return cs_path;

	const auto cs_relative = m_database_path + cs_path;
	const auto flag = PathCanonicalize(sz_out, cs_relative);
	CString cs_out = sz_out;
	if (!flag)
		cs_out.Empty();
	return cs_out;
}

long CdbWaveDatabase::GetAbsolutePathFromID(const long i_id)
{
	long new_id = -1;
	const auto cs_path = m_pathSet.GetStringFromID(i_id);
	if (IsRelativePath(cs_path))
	{
		auto cs_absolute_path = GetAbsolutePathFromString(cs_path);
		if (!cs_absolute_path.IsEmpty())
		{
			new_id = m_pathSet.GetStringInLinkedTable(cs_absolute_path);
		}
	}
	else
		new_id = i_id;
	return new_id;
}

void CdbWaveDatabase::ConvertPathtoAbsolutePath(const int i_col_path)
{
	COleVariant var_value;
	m_mainTableSet.GetFieldValue(i_col_path, var_value);
	const auto path_id = var_value.lVal;
	const auto i_id = GetAbsolutePathFromID(path_id);
	if (i_id != path_id && i_id != -1)
	{
		m_mainTableSet.Edit();
		var_value.lVal = i_id;
		m_mainTableSet.SetFieldValue(i_col_path, var_value.lVal);
		m_mainTableSet.Update();
	}
}

void CdbWaveDatabase::ConvertPathTabletoAbsolutePath()
{
	const auto cs_origin = GetDataBasePath();
	if (m_mainTableSet.IsBOF() && m_mainTableSet.IsEOF())
		return;

	ASSERT(m_mainTableSet.CanBookmark());
	try
	{
		const auto ol = m_mainTableSet.GetBookmark();
		m_mainTableSet.MoveFirst();
		const auto col_path = CH_PATH_ID - 1;
		// m_mainTableSet.GetColumnIndex(m_mainTableSet.m_desc[CH_PATH_ID].header_name);
		const auto col_path2 = CH_PATH2_ID - 1;
		//  m_mainTableSet.GetColumnIndex(m_mainTableSet.m_desc[CH_PATH2_ID].header_name);

		while (!m_mainTableSet.IsEOF())
		{
			ConvertPathtoAbsolutePath(col_path);
			ConvertPathtoAbsolutePath(col_path2);
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

void CdbWaveDatabase::GetFilenamesFromCurrentRecord()
{
	*m_current_data_filename = GetDatFilenameFromCurrentRecord();
	*m_p_current_spike_filename = GetSpkFilenameFromCurrentRecord();
}

CString CdbWaveDatabase::GetDatFilenameFromCurrentRecord()
{
	CString filename;
	filename.Empty();
	if (!m_mainTableSet.IsFieldNull(&m_mainTableSet.m_Filedat) && !m_mainTableSet.m_Filedat.IsEmpty())
	{
		filename = GetFilePath(m_mainTableSet.m_path_ID) + '\\' + m_mainTableSet.m_Filedat;
	}
	return filename;
}

CString CdbWaveDatabase::GetSpkFilenameFromCurrentRecord()
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

BOOL CdbWaveDatabase::MoveRecord(UINT nIDMoveCommand)
{
	UpdateTables();
	auto flag = TRUE;
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

BOOL CdbWaveDatabase::SetIndexCurrentFile(long ifile)
{
	// save any pending edit or add operation
	UpdateTables();
	const int record_count = m_mainTableSet.GetRecordCount();
	if (ifile < 0 || record_count < 1)
		return FALSE;

	auto flag = TRUE;
	try
	{
		if (ifile >= record_count)
			ifile = record_count - 1;
		if (ifile != m_mainTableSet.GetAbsolutePosition())
			m_mainTableSet.SetAbsolutePosition(ifile);
		GetFilenamesFromCurrentRecord();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 13);
		e->Delete();
		flag = FALSE;
	}
	return flag;
}

// fill in DB_ITEMDESC structure
// input:
// iCH = column of m_psz... (see CH_.. definitions in dbMainTable
// pointer to DB_ITEMDESC structure
// output:
// DB_ITEMDESC filled
// flag = FALSE if the column is not found

DB_ITEMDESC* CdbWaveDatabase::GetRecordItemDescriptor(int icol)
{
	const auto p_desc = &m_mainTableSet.m_desc[icol];
	p_desc->index = icol;

	switch (icol)
	{
	case CH_ID:
		p_desc->pdataItem = &m_mainTableSet.m_ID;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_ACQDATE:
		p_desc->pdataItem = nullptr; //&m_acq_date;
		ASSERT(p_desc->typeLocal == FIELD_DATE);
		break;
	case CH_FILENAME:
	case CH_FILESPK:
	case CH_ACQ_COMMENTS:
	case CH_MORE:
		p_desc->pdataItem = nullptr; //&m_more;
		ASSERT(p_desc->typeLocal == FIELD_TEXT);
		break;
	case CH_IDINSECT:
		p_desc->pdataItem = &m_mainTableSet.m_IDinsect;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_IDSENSILLUM:
		p_desc->pdataItem = &m_mainTableSet.m_IDsensillum;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_DATALEN:
		p_desc->pdataItem = &m_mainTableSet.m_datalen;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_NSPIKES:
		p_desc->pdataItem = &m_mainTableSet.m_nspikes;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_NSPIKECLASSES:
		p_desc->pdataItem = &m_mainTableSet.m_nspikeclasses;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_FLAG:
		p_desc->pdataItem = &m_mainTableSet.m_flag;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_INSECT_ID:
		p_desc->pdataItem = &m_mainTableSet.m_insect_ID;
		p_desc->plinkedSet = &m_insectSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_SENSILLUM_ID:
		p_desc->pdataItem = &m_mainTableSet.m_sensillum_ID;
		p_desc->plinkedSet = &m_sensillumSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_OPERATOR_ID:
		p_desc->pdataItem = &m_mainTableSet.m_operator_ID;
		p_desc->plinkedSet = &m_operatorSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_STIM_ID:
		p_desc->pdataItem = &m_mainTableSet.m_stim_ID;
		p_desc->plinkedSet = &m_stimSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_CONC_ID:
		p_desc->pdataItem = &m_mainTableSet.m_conc_ID;
		p_desc->plinkedSet = &m_concSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_LOCATION_ID:
		p_desc->pdataItem = &m_mainTableSet.m_location_ID;
		p_desc->plinkedSet = &m_locationSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_PATH_ID:
		p_desc->pdataItem = &m_mainTableSet.m_path_ID;
		p_desc->plinkedSet = &m_pathSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_FILEPATH);
		break;
	case CH_PATH2_ID:
		p_desc->pdataItem = &m_mainTableSet.m_path2_ID;
		p_desc->plinkedSet = &m_pathSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_FILEPATH);
		break;
	case CH_STIM2_ID:
		p_desc->pdataItem = &m_mainTableSet.m_stim2_ID;
		p_desc->plinkedSet = &m_stimSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_CONC2_ID:
		p_desc->pdataItem = &m_mainTableSet.m_conc2_ID;
		p_desc->plinkedSet = &m_concSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_STRAIN_ID:
		p_desc->pdataItem = &m_mainTableSet.m_strain_ID;
		p_desc->plinkedSet = &m_strainSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_SEX_ID:
		p_desc->pdataItem = &m_mainTableSet.m_sex_ID;
		p_desc->plinkedSet = &m_sexSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;
	case CH_REPEAT:
		p_desc->pdataItem = &m_mainTableSet.m_repeat;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_REPEAT2:
		p_desc->pdataItem = &m_mainTableSet.m_repeat2;
		ASSERT(p_desc->typeLocal == FIELD_LONG);
		break;
	case CH_ACQDATE_DAY:
		p_desc->pdataItem = nullptr; //&m_acqdate_day;
		ASSERT(p_desc->typeLocal == FIELD_DATE_YMD);
		break;
	case CH_ACQDATE_TIME:
		p_desc->pdataItem = nullptr; //&m_acqdate_time;
		ASSERT(p_desc->typeLocal == FIELD_DATE_HMS);
		break;
	case CH_EXPT_ID:
		p_desc->pdataItem = &m_mainTableSet.m_expt_ID;
		p_desc->plinkedSet = &m_exptSet;
		ASSERT(p_desc->typeLocal == FIELD_IND_TEXT);
		break;

	// if it comes here, the program must have crashed because pdesc is not defined...
	default:
		return nullptr;
	}
	////////////

	return p_desc;
}

BOOL CdbWaveDatabase::GetRecordItemValue(const int i_channel, DB_ITEMDESC* p_desc)
{
	auto flag = TRUE;
	COleVariant var_value;
	m_mainTableSet.GetFieldValue(m_mainTableSet.m_desc[i_channel].header_name, var_value);
	switch (i_channel)
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
		p_desc->lVal = var_value.lVal;
		p_desc->csVal = m_mainTableSet.m_desc[i_channel].plinkedSet->GetStringFromID(var_value.lVal);
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
		p_desc->lVal = var_value.lVal;
		if (var_value.vt == VT_NULL)
			p_desc->lVal = 0;
		break;

	case CH_ACQDATE:
	case CH_ACQDATE_DAY:
	case CH_ACQDATE_TIME:
		p_desc->oVal = var_value.date;
		break;

	case CH_FILENAME:
	case CH_FILESPK:
	case CH_ACQ_COMMENTS:
	case CH_MORE:
		m_mainTableSet.GetFieldValue(m_mainTableSet.m_desc[i_channel].header_name, var_value);
		p_desc->csVal = V_BSTRT(&var_value);
		break;

	case CH_EXPT_ID:
		p_desc->lVal = var_value.lVal;
		p_desc->csVal = m_mainTableSet.m_desc[i_channel].plinkedSet->GetStringFromID(var_value.lVal);
		if (p_desc->csVal.IsEmpty())
		{
			auto cs = p_desc->csVal = GetName();
			const auto left = cs.ReverseFind(_T('\\'));
			const auto right = cs.ReverseFind(_T('.'));
			p_desc->csVal = cs.Mid(left + 1, right - left - 1);
		}
		break;
	default:
		flag = FALSE;
		break;
	}

	return flag;
}

BOOL CdbWaveDatabase::SetRecordItemValue(const int ich, DB_ITEMDESC* p_desc)
{
	auto flag = TRUE;
	long dummy_id;
	COleVariant var_value;

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
		var_value = p_desc->lVal;
		m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[ich].header_name, var_value);
		break;

	case CH_ACQDATE_DAY:
	case CH_ACQDATE_TIME:
	case CH_ACQDATE:
		var_value = p_desc->oVal;
		m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[ich].header_name, var_value);
		break;

	case CH_FILENAME:
	case CH_FILESPK:
	case CH_ACQ_COMMENTS:
	case CH_MORE:
		var_value = p_desc->csVal;
		m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[ich].header_name, var_value);
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
		dummy_id = m_mainTableSet.m_desc[ich].plinkedSet->GetStringInLinkedTable(p_desc->csVal);
		if (dummy_id >= 0)
		{
			var_value.lVal = dummy_id;
			m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[ich].header_name, var_value.lVal);
		}
		break;

	default:
		flag = FALSE;
		break;
	}
	////////////
	return flag;
}

BOOL CdbWaveDatabase::ImportRecordfromDatabase(CdbWaveDatabase* p_w_database)
{
	// variables
	DB_ITEMDESC desc;
	const auto flag = TRUE;
	// insert new record
	m_mainTableSet.AddNew();

	// copy each field of the source database into current
	for (auto i = 1; i < m_mainTableSet.m_nFields; i++)
	{
		p_w_database->GetRecordItemValue(i, &desc);
		SetRecordItemValue(i, &desc);
	}
	m_mainTableSet.Update();
	return flag;
}

void CdbWaveDatabase::TransferWaveFormatDataToRecord(CWaveFormat* p_wf)
{
	// set time -- o_time
	COleDateTime o_time;
	auto t = p_wf->acqtime;
	o_time.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_acq_date), FALSE);
	m_mainTableSet.m_acq_date = o_time;
	m_mainTableSet.m_acqdate_time = o_time;
	o_time.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), 0, 0, 0);
	m_mainTableSet.m_acqdate_day = o_time;

	// set insect ID, sensillumID, repeat and repeat2, moreComment
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_IDinsect), FALSE);
	m_mainTableSet.m_IDinsect = p_wf->insectID;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_IDsensillum), FALSE);
	m_mainTableSet.m_IDsensillum = p_wf->sensillumID;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_repeat), FALSE);
	m_mainTableSet.m_repeat = p_wf->repeat;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_repeat2), FALSE);
	m_mainTableSet.m_repeat2 = p_wf->repeat2;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_more), FALSE);
	m_mainTableSet.m_more = p_wf->csMoreComment;

	// set type, stimulus and concentrations
	m_mainTableSet.m_operator_ID = m_operatorSet.GetStringInLinkedTable(p_wf->csOperator);
	m_mainTableSet.m_insect_ID = m_insectSet.GetStringInLinkedTable(p_wf->csInsectname);
	m_mainTableSet.m_location_ID = m_locationSet.GetStringInLinkedTable(p_wf->csLocation);
	m_mainTableSet.m_expt_ID = m_exptSet.GetStringInLinkedTable(p_wf->cs_comment);
	m_mainTableSet.m_sensillum_ID = m_sensillumSet.GetStringInLinkedTable(p_wf->csSensillum);
	m_mainTableSet.m_stim_ID = m_stimSet.GetStringInLinkedTable(p_wf->csStimulus);
	m_mainTableSet.m_conc_ID = m_concSet.GetStringInLinkedTable(p_wf->csConcentration);
	m_mainTableSet.m_stim2_ID = m_stimSet.GetStringInLinkedTable(p_wf->csStimulus2);
	m_mainTableSet.m_conc2_ID = m_concSet.GetStringInLinkedTable(p_wf->csConcentration2);
	m_mainTableSet.m_sex_ID = m_sexSet.GetStringInLinkedTable(p_wf->csSex);
	m_mainTableSet.m_strain_ID = m_strainSet.GetStringInLinkedTable(p_wf->csStrain);
	m_mainTableSet.m_expt_ID = m_exptSet.GetStringInLinkedTable(p_wf->cs_comment);
	m_mainTableSet.m_flag = p_wf->flag;
}

void CdbWaveDatabase::DeleteUnusedEntriesInAccessoryTables()
{
	DeleteUnusedEntriesInAttachedTable(&m_operatorSet, CH_OPERATOR_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_insectSet, CH_INSECT_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_locationSet, CH_LOCATION_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_sensillumSet, CH_SENSILLUM_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_stimSet, CH_STIM_ID, CH_STIM2_ID);
	DeleteUnusedEntriesInAttachedTable(&m_concSet, CH_CONC_ID, CH_CONC2_ID);
	DeleteUnusedEntriesInAttachedTable(&m_sexSet, CH_SEX_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_strainSet, CH_STRAIN_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_exptSet, CH_EXPT_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_pathSet, CH_PATH_ID, CH_PATH2_ID);
}

void CdbWaveDatabase::DeleteUnusedEntriesInAttachedTable(CdbIndexTable* p_index_table, const int first_column,
                                                      const int last_column)
{
	COleVariant var_value0, var_value1;

	if (p_index_table->IsBOF() && p_index_table->IsEOF())
		return;

	p_index_table->MoveFirst();
	while (!p_index_table->IsEOF())
	{
		p_index_table->GetFieldValue(1, var_value1);
		const auto id_current = var_value1.lVal;
		const auto flag1 = m_mainTableSet.FindIDinColumn(id_current, first_column);
		auto flag2 = FALSE;
		if (last_column >= 0)
			flag2 = m_mainTableSet.FindIDinColumn(id_current, last_column);
		if (flag1 == FALSE && flag2 == FALSE)
		{
			p_index_table->Delete();
		}
		p_index_table->MoveNext();
	}
}
