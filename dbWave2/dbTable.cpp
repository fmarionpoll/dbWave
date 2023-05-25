#include "StdAfx.h"
#include "dbTableAssociated.h"
#include "dbTableMain.h"
#include "dbTable.h"

#include "dbTableColumnDescriptor.h"
#include "dbWave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


column_properties CdbTable::m_column_properties[N_TABLE_COLUMNS] =
{
	//table col header---friendly description for properties--type of field--name of attached table
	{COL_ID, _T("ID"), _T("ID record"), FIELD_LONG, _T("")}, // 0
	{COL_ACQDATE, _T("acq_date"), _T("acquisition date & time"), FIELD_DATE, _T("")}, // 1
	{COL_FILENAME, _T("filename"), _T("dat file"), FIELD_TEXT, _T("") }, // 2
	{COL_FILESPK, _T("filespk"), _T("spk file"), FIELD_TEXT, _T("")}, // 3
	{COL_ACQ_COMMENTS, _T("acq_comment"), _T("comment"), FIELD_TEXT, _T("")}, // 4
	{COL_MORE, _T("more"), _T("comment"), FIELD_TEXT, _T("")}, // 5
	{COL_IDINSECT, _T("insectID"), _T("ID insect"), FIELD_LONG, _T("")}, // 6
	{COL_IDSENSILLUM, _T("sensillumID"), _T("ID sensillum"), FIELD_LONG, _T("")}, // 7
	{COL_DATALEN, _T("datalen"), _T("data length (n points)"), FIELD_LONG, _T("")}, // 8
	{COL_NSPIKES, _T("nspikes"), _T("n spikes detected"), FIELD_LONG, _T("")}, // 9
	{COL_NSPIKECLASSES, _T("nspikeclasses"),_T("n spike classes"), FIELD_LONG, _T("")}, // 10
	{COL_FLAG, _T("flag"), _T("flag"), FIELD_LONG, _T("")}, // 11
	{COL_INSECT_ID, _T("insectname_ID"),_T("insect"), FIELD_IND_TEXT, _T("insectname")}, // 12
	{COL_SENSILLUM_ID, _T("sensillumname_ID"),_T("sensillum"), FIELD_IND_TEXT, _T("sensillumname")}, // 13
	{COL_OPERATOR_ID, _T("operator_ID"), _T("operator"), FIELD_IND_TEXT, _T("operator")}, // 14
	{COL_STIM_ID, _T("stim_ID"), _T("compound(1)"), FIELD_IND_TEXT, _T("stim")}, // 15
	{COL_CONC_ID, _T("conc_ID"), _T("concentration(1)"), FIELD_IND_TEXT, _T("conc")}, // 16
	{COL_LOCATION_ID, _T("location_ID"), _T("location"), FIELD_IND_TEXT, _T("location")}, // 17
	{COL_PATH_ID, _T("path_ID"), _T("path (*.dat)"), FIELD_IND_FILEPATH, _T("path")}, // 18
	{COL_PATH2_ID, _T("path2_ID"), _T("path (*.spk)"), FIELD_IND_FILEPATH, _T("path")}, // 19
	{COL_STIM2_ID, _T("stim2_ID"), _T("compound(2)"), FIELD_IND_TEXT, _T("stim")}, // 20
	{COL_CONC2_ID, _T("conc2_ID"), _T("concentration(2)"), FIELD_IND_TEXT, _T("conc")}, // 21
	{COL_STRAIN_ID, _T("strain_ID"), _T("insect strain"), FIELD_IND_TEXT, _T("strain")}, // 22
	{COL_SEX_ID, _T("sex_ID"), _T("insect sex"), FIELD_IND_TEXT, _T("sex")}, // 23
	{COL_REPEAT, _T("repeat"), _T("repeat(1)"), FIELD_LONG, _T("")}, // 24
	{COL_REPEAT2, _T("repeat2"), _T("repeat(2)"), FIELD_LONG, _T("")}, // 25
	{COL_ACQDATE_DAY, _T("acqdate_day"), _T("date"), FIELD_DATE_YMD, _T("")}, // 26
	{COL_ACQDATE_TIME, _T("acqdate_time"), _T("time"), FIELD_DATE_HMS, _T("")}, // 27
	{COL_EXPT_ID, _T("expt_ID"), _T("Experiment"), FIELD_IND_TEXT, _T("expt")} // 28
};

CdbTable::CdbTable() 
{
	for (auto i = 0; i < N_TABLE_COLUMNS; i++)
	{
		CString dummy = m_column_properties[i].header_name;
		m_mainTableSet.m_desc[i].header_name = dummy;
		m_mainTableSet.m_desc[i].dfx_name_with_brackets = _T("[") + dummy + _T("]");
		m_mainTableSet.m_desc[i].csColParam = dummy + _T("Param");
		m_mainTableSet.m_desc[i].csEQUcondition = dummy + _T("=") + m_mainTableSet.m_desc[i].csColParam;
		m_mainTableSet.m_desc[i].data_code_number = m_column_properties[i].format_code_number;
	}

	set_attached_tables_names();
	
	m_mainTableSet.m_strSort = m_column_properties[CH_ACQDATE].header_name;
}

void CdbTable::set_attached_tables_names()
{
	// Set_DFX_SQL_Names(CString defaultSQL /* or table name*/, CString DFX_cs, CString DFX_ID)
	m_stim_set.Set_DFX_SQL_Names(m_column_properties[CH_STIM_ID].attached_table, _T("stim"), _T("stimID"));
	m_conc_set.Set_DFX_SQL_Names(m_column_properties[CH_CONC_ID].attached_table, _T("conc"), _T("concID"));
	m_operator_set.Set_DFX_SQL_Names(m_column_properties[CH_OPERATOR_ID].attached_table, _T("operator"), _T("operatorID"));
	m_insect_set.Set_DFX_SQL_Names(m_column_properties[CH_INSECT_ID].attached_table, _T("insect"), _T("insectID"));
	m_locationSet.Set_DFX_SQL_Names(m_column_properties[CH_LOCATION_ID].attached_table, _T("type"), _T("typeID"));
	m_path_set.Set_DFX_SQL_Names(m_column_properties[CH_PATH_ID].attached_table, _T("path"), _T("pathID"));
	m_sensillum_set.Set_DFX_SQL_Names(m_column_properties[CH_SENSILLUM_ID].attached_table, _T("stage"), _T("stageID"));
	m_sex_set.Set_DFX_SQL_Names(m_column_properties[CH_SEX_ID].attached_table, _T("sex"), _T("sexID"));
	m_strain_set.Set_DFX_SQL_Names(m_column_properties[CH_STRAIN_ID].attached_table, _T("strain"), _T("strainID"));
	m_expt_set.Set_DFX_SQL_Names(m_column_properties[CH_EXPT_ID].attached_table, _T("expt"), _T("exptID"));

}

boolean CdbTable::create_relations_with_attached_tables(const CString& csTable)
{
	// create relations
	const long l_attr = dbRelationDontEnforce; //dbRelationUpdateCascade;
	if (!create_relation_between_associated_table_and_1_column(csTable, CH_INSECT_ID, l_attr, &m_insect_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(csTable, CH_SENSILLUM_ID, l_attr, &m_sensillum_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(csTable, CH_OPERATOR_ID, l_attr, &m_operator_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(csTable, CH_LOCATION_ID, l_attr, &m_locationSet)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(csTable, CH_STRAIN_ID, l_attr, &m_strain_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(csTable, CH_SEX_ID, l_attr, &m_sex_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(csTable, CH_EXPT_ID, l_attr, &m_expt_set)) return FALSE;

	if (!create_relation_between_associated_table_and_2_columns(csTable, CH_PATH_ID, CH_PATH2_ID)) return FALSE;
	if (!create_relation_between_associated_table_and_2_columns(csTable, CH_STIM_ID, CH_STIM2_ID)) return FALSE;
	if (!create_relation_between_associated_table_and_2_columns(csTable, CH_CONC_ID, CH_CONC2_ID)) return FALSE;

	return true;
}

void CdbTable::Attach(CString* cs_data_file_name, CString* cs_spike_file_name)
{
	m_current_data_filename = cs_data_file_name;
	m_p_current_spike_filename = cs_spike_file_name;
}

CdbTable::~CdbTable()
{
	CloseDatabase();
}

BOOL CdbTable::CreateMainTable(const CString& csTable)
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

	return create_relations_with_attached_tables(csTable);
}

BOOL CdbTable::create_relation_between_associated_table_and_2_columns(const LPCTSTR lpsz_foreign_table, const int column_index_1,
                                                  const int column_index_2)
{
	try
	{
		const LPCTSTR lpsz_table = m_column_properties[column_index_1].attached_table;
		CDaoRelationInfo rl_info;
		rl_info.m_strTable = lpsz_table;
		rl_info.m_strName.Format(_T("%s_%s"), lpsz_foreign_table, lpsz_table);
		rl_info.m_strForeignTable = lpsz_foreign_table;
		rl_info.m_lAttributes = dbRelationDontEnforce;

		CDaoRelationFieldInfo r_field[2];
		r_field[0].m_strName = lpsz_table;
		r_field[0].m_strName += _T("ID");
		r_field[0].m_strForeignName = m_mainTableSet.m_desc[column_index_1].header_name; // path_ID
		r_field[1].m_strName = r_field[0].m_strName;
		r_field[1].m_strForeignName = m_mainTableSet.m_desc[column_index_2].header_name; // path2_ID
		rl_info.m_pFieldInfos = &r_field[0];
		rl_info.m_nFields = 2;
		CreateRelation(rl_info);

		m_mainTableSet.m_desc[column_index_1].plinkedSet = &m_path_set;
		m_mainTableSet.m_desc[column_index_1].associated_table_name = r_field[0].m_strName;

		m_mainTableSet.m_desc[column_index_2].plinkedSet = &m_path_set;
		m_mainTableSet.m_desc[column_index_2].associated_table_name = r_field[0].m_strName;
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
BOOL CdbTable::create_relation_between_associated_table_and_1_column(const LPCTSTR lpsz_foreign_table, const int column_index,
                                                const long l_attributes, CdbTableAssociated* p_to_associated_table)
{
	try
	{
		const LPCTSTR lpsz_table = m_column_properties[column_index].attached_table;
		m_mainTableSet.m_desc[column_index].associated_table_name = lpsz_table;
		CString cs_rel; // unique name of the relation object (max 40 chars)
		cs_rel.Format(_T("%s_%s"), lpsz_foreign_table, lpsz_table);
		CString sz_field = lpsz_table;
		sz_field += _T("ID");
		CreateRelation(cs_rel, lpsz_table, lpsz_foreign_table, l_attributes, sz_field,
		               m_mainTableSet.m_desc[column_index].header_name);
		m_mainTableSet.m_desc[column_index].plinkedSet = p_to_associated_table;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 5);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

CString CdbTable::GetDataBasePath()
{
	return m_database_path;
}

void CdbTable::SetDataBasePath()
{
	const auto filename = GetName();
	const auto last_slash = filename.ReverseFind('\\');
	m_database_path = filename.Left(last_slash + 1);
	m_database_path.MakeLower();
}

void CdbTable::create_associated_tables()
{
	// create tables with indexes. These tables will contain categories entered by the user
	m_insect_set.CreateIndextable(_T("insectname"), _T("insect"), _T("insectID"), 100, this);
	m_sensillum_set.CreateIndextable(_T("sensillumname"), _T("stage"), _T("stageID"), 100, this);
	m_operator_set.CreateIndextable(_T("operator"), _T("operator"), _T("operatorID"), 50, this);
	m_stim_set.CreateIndextable(_T("stim"), _T("stim"), _T("stimID"), 100, this);
	m_conc_set.CreateIndextable(_T("conc"), _T("conc"), _T("concID"), 100, this);
	m_locationSet.CreateIndextable(_T("location"), _T("type"), _T("typeID"), 100, this);
	m_path_set.CreateIndextable(_T("path"), _T("path"), _T("pathID"), 255, this);
	m_strain_set.CreateIndextable(_T("strain"), _T("strain"), _T("strainID"), 100, this);
	m_sex_set.CreateIndextable(_T("sex"), _T("sex"), _T("sexID"), 10, this);
	m_expt_set.CreateIndextable(_T("expt"), _T("expt"), _T("exptID"), 100, this);
}

void CdbTable::CreateAllTables()
{
	SetDataBasePath();
	create_associated_tables();
	CreateMainTable(_T("table"));

	// store links to attached table addresses
	for (auto column = 0; column < N_TABLE_COLUMNS; column++)
		GetRecordItemDescriptor(column);
}

BOOL CdbTable::OpenTables()
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
		CdbTableMain rs2; // CDaoRecordSet
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
		OpenAssociatedTable(&m_stim_set);
		OpenAssociatedTable(&m_conc_set);
		OpenAssociatedTable(&m_operator_set);
		OpenAssociatedTable(&m_insect_set);
		OpenAssociatedTable(&m_locationSet);
		OpenAssociatedTable(&m_path_set);
		OpenAssociatedTable(&m_sensillum_set);
		OpenAssociatedTable(&m_sex_set);
		OpenAssociatedTable(&m_strain_set);
		OpenAssociatedTable(&m_expt_set);
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
	for (auto column = 0; column < N_TABLE_COLUMNS; column++)
		GetRecordItemDescriptor(column);

	return TRUE;
}

void CdbTable::add_column_28(CDaoTableDef& table_def, const CString& cs_table, const long l_attr)
{
	table_def.Open(cs_table);
	table_def.CreateField(m_mainTableSet.m_desc[CH_EXPT_ID].header_name, dbLong, 4, 0); 
	m_expt_set.CreateIndextable(_T("expt"), _T("expt"), _T("exptID"), 100, this);
	CreateRelation(_T("table_expt"), _T("expt"), cs_table, l_attr, _T("exptID"),
		m_mainTableSet.m_desc[CH_EXPT_ID].header_name); 
	table_def.Close();
}

void CdbTable::add_column_26_27(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const
{
	table_def.Open(cs_table);
	table_def.CreateField(m_mainTableSet.m_desc[CH_ACQDATE_DAY].header_name, dbDate, 8, 0);
	table_def.CreateField(m_mainTableSet.m_desc[CH_ACQDATE_TIME].header_name, dbDate, 8, 0); 
	table_def.Close();
}

void CdbTable::add_column_24_25(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const
{
	table_def.Open(cs_table);
	table_def.CreateField(m_mainTableSet.m_desc[CH_REPEAT].header_name, dbLong, 4, 0); 
	table_def.CreateField(m_mainTableSet.m_desc[CH_REPEAT2].header_name, dbLong, 4, 0); 
	table_def.Close();
}

void CdbTable::add_column_22_23(CDaoTableDef& table_def, const CString& cs_table, long l_attr)
{
	table_def.Open(cs_table);

	// add fields in the main table, add the corresponding tables and the relations between the main table and the new index tables
	table_def.CreateField(m_mainTableSet.m_desc[CH_STRAIN_ID].header_name, dbLong, 4, 0); // strain_ID
	table_def.CreateField(m_mainTableSet.m_desc[CH_SEX_ID].header_name, dbLong, 4, 0); // sex_ID
	m_strain_set.CreateIndextable(_T("strain"), _T("strain"), _T("strainID"), 100, this);
	m_sex_set.CreateIndextable(_T("sex"), _T("sex"), _T("sexID"), 10, this);
	CreateRelation(_T("table_strain"), _T("strain"), cs_table, l_attr, _T("strainID"),
		m_mainTableSet.m_desc[CH_STRAIN_ID].header_name); // strain_ID
	CreateRelation(_T("table_sex"), _T("sex"), cs_table, l_attr, _T("sexID"),
		m_mainTableSet.m_desc[CH_SEX_ID].header_name); // sex_ID
// type -> location
	DeleteRelation(_T("table_type")); // delete relationship
	table_def.DeleteField(CH_LOCATION_ID);
	// delete the field (index is different because we deleted one field)
	table_def.CreateField(m_mainTableSet.m_desc[CH_LOCATION_ID].header_name, dbLong, 4, 0); // locationID
	// stage -> sensillum name
	DeleteRelation(_T("table_stage")); // delete relationship
	table_def.DeleteField(CH_SENSILLUM_ID); // delete field
	table_def.CreateField(m_mainTableSet.m_desc[CH_SENSILLUM_ID].header_name, dbLong, 4, 0);
	// sensillumID
	table_def.Close();

	// rename table stage into sensillum name
	table_def.Open(_T("stage"));
	table_def.SetName(_T("sensillumname"));
	table_def.Close();

	// rename table type into location
	table_def.Open(_T("type"));
	table_def.SetName(_T("location"));
	table_def.Close();

	// rename existing fields into "sensillum name" table (sensillum /sensillumID instead of stage/stageID)
	// rename existing fields into "location" table (location/locationID instead type/typeID)
	// create relations
	table_def.Open(cs_table);
	CString cs_rel = _T("table_sensillumname");
	CreateRelation(cs_rel, _T("sensillumname"), cs_table, l_attr, _T("stageID"),
		m_mainTableSet.m_desc[CH_SENSILLUM_ID].header_name); // sensillum name_ID
	cs_rel = _T("table_location");
	CreateRelation(cs_rel, _T("location"), cs_table, l_attr, _T("typeID"),
		m_mainTableSet.m_desc[CH_LOCATION_ID].header_name); //location_ID
	table_def.Close();
}

void CdbTable::add_column_21(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const
{
	table_def.Open(cs_table);
	table_def.CreateField(m_mainTableSet.m_desc[CH_FLAG].header_name, dbLong, 4, 0); 
	table_def.Close();
}

void CdbTable::add_column_19_20(CDaoTableDef& table_def, const CString& cs_table, long l_attr)
{
	table_def.Open(cs_table);
	CString cs_rel = _T("table_Rel1");
	const auto i_pos = cs_rel.GetLength() - 1;
	table_def.CreateField(m_mainTableSet.m_desc[CH_STIM2_ID].header_name, dbLong, 4, 0); 
	table_def.CreateField(m_mainTableSet.m_desc[CH_CONC2_ID].header_name, dbLong, 4, 0);
	cs_rel.SetAt(i_pos, '9');
	CreateRelation(cs_rel, _T("stim"), cs_table, l_attr, _T("stimID"),
		m_mainTableSet.m_desc[CH_STIM2_ID].header_name); 
	cs_rel.SetAt(i_pos, 'A');
	CreateRelation(cs_rel, _T("conc"), cs_table, l_attr, _T("concID"),
		m_mainTableSet.m_desc[CH_CONC2_ID].header_name);
	table_def.Close();
}

void CdbTable::OpenAssociatedTable(CdbTableAssociated* p_index_table_set)
{
	p_index_table_set->m_defaultName = GetName();
	p_index_table_set->Open(dbOpenTable, nullptr, 0);
}

void CdbTable::CloseDatabase()
{
	if (IsOpen())
	{
		// closing the workspace does not close the recordsets
		// so first close them
		if (m_mainTableSet.IsOpen()) m_mainTableSet.Close();
		if (m_operator_set.IsOpen()) m_operator_set.Close();
		if (m_insect_set.IsOpen()) m_insect_set.Close();
		if (m_locationSet.IsOpen()) m_locationSet.Close();
		if (m_sensillum_set.IsOpen()) m_sensillum_set.Close();
		if (m_path_set.IsOpen()) m_path_set.Close();
		if (m_stim_set.IsOpen()) m_stim_set.Close();
		if (m_conc_set.IsOpen()) m_conc_set.Close();
		if (m_sex_set.IsOpen()) m_sex_set.Close();
		if (m_strain_set.IsOpen()) m_strain_set.Close();
		if (m_expt_set.IsOpen()) m_expt_set.Close();

		// close the workspace
		m_pWorkspace->Close();
		Close();
	}
}

void CdbTable::UpdateAllDatabaseTables()
{
	// For each table, call Update() if GetEditMode ()
	//	= dbEditInProgress (Edit has been called)
	//	= dbEditAdd (AddNew has been called)
	if (m_mainTableSet.GetEditMode() != dbEditNone)
		m_mainTableSet.Update();
	if (m_operator_set.GetEditMode() != dbEditNone)
		m_operator_set.Update();
	if (m_insect_set.GetEditMode() != dbEditNone)
		m_insect_set.Update();
	if (m_locationSet.GetEditMode() != dbEditNone)
		m_locationSet.Update();
	if (m_sensillum_set.GetEditMode() != dbEditNone)
		m_sensillum_set.Update();
	if (m_path_set.GetEditMode() != dbEditNone)
		m_path_set.Update();
	if (m_sex_set.GetEditMode() != dbEditNone)
		m_sex_set.Update();
	if (m_strain_set.GetEditMode() != dbEditNone)
		m_strain_set.Update();
	if (m_expt_set.GetEditMode() != dbEditNone)
		m_expt_set.Update();
}

CString CdbTable::get_file_path(const int i_id)
{
	auto cs_path = m_path_set.GetStringFromID(i_id);
	if (is_relative_path(cs_path)) {
		cs_path = cs_path.Right(cs_path.GetLength() - 2);
		cs_path = m_database_path + cs_path;
	}
	return cs_path;
}

CString CdbTable::get_relative_path_from_string(const CString& cs_path) const
{
	char sz_out[MAX_PATH] = "";
	if (cs_path.IsEmpty())
		return cs_path;

	const CStringA str_from(m_database_path);
	const LPCSTR psz_from = str_from;
	const CStringA str_to(cs_path);
	const LPCSTR psz_to = str_to;
	const auto flag = PathRelativePathToA(sz_out, 
									psz_from, 
									FILE_ATTRIBUTE_DIRECTORY, 
									psz_to, 
									FILE_ATTRIBUTE_DIRECTORY);
	CString cs_out (sz_out);
	if (!flag)
		cs_out.Empty();

	return cs_out;
}

long CdbTable::get_relative_path_from_id(const long i_id)
{
	long new_id = -1;
	const auto cs_path = m_path_set.GetStringFromID(i_id);
	
	if (!is_relative_path(cs_path))
	{
		const auto cs_relative_path = get_relative_path_from_string(cs_path);
		if (!cs_relative_path.IsEmpty())
		{
			new_id = m_path_set.GetStringInLinkedTable(cs_relative_path);
		}
	}
	else
		new_id = i_id;
	return new_id;
}

void CdbTable::convert_path_to_relative_path(const long i_col_path)
{
	COleVariant var_value;
	m_mainTableSet.GetFieldValue(i_col_path, var_value);
	const auto path_id = var_value.lVal;

	const auto i_id = get_relative_path_from_id(path_id);
	if (i_id != path_id && i_id != -1)
	{
		m_mainTableSet.Edit();
		var_value.lVal = i_id;
		m_mainTableSet.SetFieldValue(i_col_path, var_value.lVal);
		m_mainTableSet.Update();
	}
}

void CdbTable::set_path_relative()
{
	ASSERT(m_mainTableSet.CanBookmark()); 
	if (m_mainTableSet.IsBOF() && m_mainTableSet.IsEOF())
		return;
	
	try
	{
		const auto ol = m_mainTableSet.GetBookmark();
		m_mainTableSet.MoveFirst();
		const auto i_col_path = m_mainTableSet.GetColumnIndex(_T("path_ID"));
		const auto i_col_path2 = m_mainTableSet.GetColumnIndex(_T("path2_ID"));
		
		while (!m_mainTableSet.IsEOF())
		{
			convert_path_to_relative_path(i_col_path);
			convert_path_to_relative_path(i_col_path2);
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

CString CdbTable::get_absolute_path_from_string(const CString& cs_path) const
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

long CdbTable::get_absolute_path_from_id(const long i_id)
{
	long new_id = -1;
	const auto cs_path = m_path_set.GetStringFromID(i_id);
	if (is_relative_path(cs_path))
	{
		const auto cs_absolute_path = get_absolute_path_from_string(cs_path);
		if (!cs_absolute_path.IsEmpty())
		{
			new_id = m_path_set.GetStringInLinkedTable(cs_absolute_path);
		}
	}
	else
		new_id = i_id;
	return new_id;
}

void CdbTable::convert_to_absolute_path(const int i_col_path)
{
	COleVariant var_value;
	m_mainTableSet.GetFieldValue(i_col_path, var_value);
	const auto path_id = var_value.lVal;

	const auto i_id = get_absolute_path_from_id(path_id);
	if (i_id != path_id && i_id != -1)
	{
		m_mainTableSet.Edit();
		var_value.lVal = i_id;
		m_mainTableSet.SetFieldValue(i_col_path, var_value.lVal);
		m_mainTableSet.Update();
	}
}

void CdbTable::set_path_absolute()
{
	ASSERT(m_mainTableSet.CanBookmark());
	if (m_mainTableSet.IsBOF() && m_mainTableSet.IsEOF())
		return;

	try
	{
		const auto ol = m_mainTableSet.GetBookmark();
		m_mainTableSet.MoveFirst();
		const auto col_path = m_mainTableSet.GetColumnIndex(_T("path_ID"));
		const auto col_path2 = m_mainTableSet.GetColumnIndex(_T("path2_ID"));
		
		while (!m_mainTableSet.IsEOF())
		{
			convert_to_absolute_path(col_path);
			convert_to_absolute_path(col_path2);
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

void CdbTable::GetCurrentRecord_FileNames()
{
	*m_current_data_filename = GetCurrentRecord_DataFileName();
	*m_p_current_spike_filename = GetCurrentRecord_SpikeFileName();
}

CString CdbTable::GetCurrentRecord_DataFileName()
{
	CString filename;
	filename.Empty();
	if (!m_mainTableSet.IsFieldNull(&m_mainTableSet.m_Filedat) && !m_mainTableSet.m_Filedat.IsEmpty())
	{
		filename = get_file_path(m_mainTableSet.m_path_ID) + '\\' + m_mainTableSet.m_Filedat;
	}
	return filename;
}

CString CdbTable::GetCurrentRecord_SpikeFileName()
{
	CString filename;
	filename.Empty();

	// set current spk document
	if (!m_mainTableSet.IsFieldNull(&m_mainTableSet.m_Filespk) && !m_mainTableSet.m_Filespk.IsEmpty())
	{
		if (m_mainTableSet.IsFieldNull(&m_mainTableSet.m_path2_ID))
		{
			m_mainTableSet.Edit();
			m_mainTableSet.m_path2_ID = m_mainTableSet.m_path_ID;
			m_mainTableSet.Update();
		}
		filename = get_file_path(m_mainTableSet.m_path2_ID) + '\\' + m_mainTableSet.m_Filespk;
	}
	return filename;
}

BOOL CdbTable::MoveToID(const long record_id)
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
	GetCurrentRecord_FileNames();
	return TRUE;
}

BOOL CdbTable::MoveTo(UINT nIDMoveCommand)
{
	UpdateAllDatabaseTables();
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
	GetCurrentRecord_FileNames();
	return flag;
}

BOOL CdbTable::SetIndexCurrentFile(long i_file)
{
	// save any pending edit or add operation
	UpdateAllDatabaseTables();
	const int record_count = m_mainTableSet.GetRecordCount();
	if (i_file < 0 || record_count < 1)
		return FALSE;

	auto flag = TRUE;
	try
	{
		if (i_file >= record_count)
			i_file = record_count - 1;
		if (i_file != m_mainTableSet.GetAbsolutePosition())
			m_mainTableSet.SetAbsolutePosition(i_file);
		GetCurrentRecord_FileNames();
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

DB_ITEMDESC* CdbTable::GetRecordItemDescriptor(int column_index)
{
	const auto p_desc = &m_mainTableSet.m_desc[column_index];
	p_desc->index = column_index;

	switch (column_index)
	{
	case CH_ID:
		p_desc->pdataItem = &m_mainTableSet.m_ID;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_ACQDATE:
		p_desc->pdataItem = nullptr; //&m_acq_date;
		ASSERT(p_desc->data_code_number == FIELD_DATE);
		break;
	case CH_FILENAME:
	case CH_FILESPK:
	case CH_ACQ_COMMENTS:
	case CH_MORE:
		p_desc->pdataItem = nullptr; //&m_more;
		ASSERT(p_desc->data_code_number == FIELD_TEXT);
		break;
	case CH_IDINSECT:
		p_desc->pdataItem = &m_mainTableSet.m_IDinsect;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_IDSENSILLUM:
		p_desc->pdataItem = &m_mainTableSet.m_IDsensillum;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_DATALEN:
		p_desc->pdataItem = &m_mainTableSet.m_datalen;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_NSPIKES:
		p_desc->pdataItem = &m_mainTableSet.m_nspikes;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_NSPIKECLASSES:
		p_desc->pdataItem = &m_mainTableSet.m_nspikeclasses;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_FLAG:
		p_desc->pdataItem = &m_mainTableSet.m_flag;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_INSECT_ID:
		p_desc->pdataItem = &m_mainTableSet.m_insect_ID;
		p_desc->plinkedSet = &m_insect_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_SENSILLUM_ID:
		p_desc->pdataItem = &m_mainTableSet.m_sensillum_ID;
		p_desc->plinkedSet = &m_sensillum_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_OPERATOR_ID:
		p_desc->pdataItem = &m_mainTableSet.m_operator_ID;
		p_desc->plinkedSet = &m_operator_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_STIM_ID:
		p_desc->pdataItem = &m_mainTableSet.m_stim_ID;
		p_desc->plinkedSet = &m_stim_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_CONC_ID:
		p_desc->pdataItem = &m_mainTableSet.m_conc_ID;
		p_desc->plinkedSet = &m_conc_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_LOCATION_ID:
		p_desc->pdataItem = &m_mainTableSet.m_location_ID;
		p_desc->plinkedSet = &m_locationSet;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_PATH_ID:
		p_desc->pdataItem = &m_mainTableSet.m_path_ID;
		p_desc->plinkedSet = &m_path_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_FILEPATH);
		break;
	case CH_PATH2_ID:
		p_desc->pdataItem = &m_mainTableSet.m_path2_ID;
		p_desc->plinkedSet = &m_path_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_FILEPATH);
		break;
	case CH_STIM2_ID:
		p_desc->pdataItem = &m_mainTableSet.m_stim2_ID;
		p_desc->plinkedSet = &m_stim_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_CONC2_ID:
		p_desc->pdataItem = &m_mainTableSet.m_conc2_ID;
		p_desc->plinkedSet = &m_conc_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_STRAIN_ID:
		p_desc->pdataItem = &m_mainTableSet.m_strain_ID;
		p_desc->plinkedSet = &m_strain_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_SEX_ID:
		p_desc->pdataItem = &m_mainTableSet.m_sex_ID;
		p_desc->plinkedSet = &m_sex_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_REPEAT:
		p_desc->pdataItem = &m_mainTableSet.m_repeat;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_REPEAT2:
		p_desc->pdataItem = &m_mainTableSet.m_repeat2;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_ACQDATE_DAY:
		p_desc->pdataItem = nullptr; 
		ASSERT(p_desc->data_code_number == FIELD_DATE_YMD);
		break;
	case CH_ACQDATE_TIME:
		p_desc->pdataItem = nullptr; 
		ASSERT(p_desc->data_code_number == FIELD_DATE_HMS);
		break;
	case CH_EXPT_ID:
		p_desc->pdataItem = &m_mainTableSet.m_expt_ID;
		p_desc->plinkedSet = &m_expt_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;

	// if it comes here, the program must have crashed because pdesc is not defined...
	default:
		return nullptr;
	}

	return p_desc;
}

DB_ITEMDESC* CdbTable::GetRecordItemValue(const int column_index)
{
	const auto p_desc = &m_mainTableSet.m_desc[column_index];
	p_desc->index = column_index;
	GetRecordItemValue(column_index, p_desc);
	return p_desc;
}

BOOL CdbTable::GetRecordItemValue(const int i_column, DB_ITEMDESC* dbItem_descriptor)
{
	auto flag = TRUE;
	COleVariant var_value;
	m_mainTableSet.GetFieldValue(m_mainTableSet.m_desc[i_column].header_name, var_value);
	const int data_code_number = m_column_properties[i_column].format_code_number;

	switch (data_code_number)
	{
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			dbItem_descriptor->lVal = var_value.lVal;
			dbItem_descriptor->csVal = m_mainTableSet.m_desc[i_column].plinkedSet->GetStringFromID(var_value.lVal);
			if (i_column == CH_EXPT_ID && dbItem_descriptor->csVal.IsEmpty() )
			{
				const auto cs = dbItem_descriptor->csVal = GetName();
				const auto left = cs.ReverseFind(_T('\\'));
				const auto right = cs.ReverseFind(_T('.'));
				dbItem_descriptor->csVal = cs.Mid(left + 1, right - left - 1);
			}
			break;
		case FIELD_LONG:
			dbItem_descriptor->lVal = var_value.lVal;
			if (var_value.vt == VT_NULL)
				dbItem_descriptor->lVal = 0;
			break;
		case FIELD_TEXT:
			m_mainTableSet.GetFieldValue(m_mainTableSet.m_desc[i_column].header_name, var_value);
			dbItem_descriptor->csVal = V_BSTRT(&var_value);
			break;
		case FIELD_DATE:
		case FIELD_DATE_HMS:
		case FIELD_DATE_YMD:
			dbItem_descriptor->oVal = var_value.date;
			break;
		default:
			flag = FALSE;
			break;
	}

	return flag;
}

BOOL CdbTable::SetRecordItemValue(const int column_index, DB_ITEMDESC* dbItem_descriptor)
{
	auto flag = TRUE;
	const int data_code_number = m_column_properties[column_index].format_code_number;
	switch (data_code_number) {
		case FIELD_IND_TEXT:
		case FIELD_IND_FILEPATH:
			{
				long dummy_id = m_mainTableSet.m_desc[column_index].plinkedSet->GetStringInLinkedTable(dbItem_descriptor->csVal);
				if (dummy_id >= 0)
				{
					COleVariant var_value;
					var_value.lVal = dummy_id;
					m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[column_index].header_name, var_value.lVal);
				}
			}
			break;
		case FIELD_LONG:
			m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[column_index].header_name, dbItem_descriptor->lVal);
			break;
		case FIELD_TEXT:
			{
				COleVariant var_value = dbItem_descriptor->csVal;
				m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[column_index].header_name, var_value);
			}
			break;
		case FIELD_DATE:
		case FIELD_DATE_HMS:
		case FIELD_DATE_YMD:
			m_mainTableSet.SetFieldValue(m_mainTableSet.m_desc[column_index].header_name, dbItem_descriptor->oVal);
			break;
		default:
			flag = FALSE;
			break;
	}

	return flag;
}

BOOL CdbTable::ImportRecordFromDatabase(CdbTable* p_external_dbTable)
{
	// variables
	DB_ITEMDESC desc;
	constexpr auto flag = TRUE;
	// insert new record
	m_mainTableSet.AddNew();

	// copy each field of the source database into current
	for (auto i = 1; i < m_mainTableSet.m_nFields; i++)
	{
		p_external_dbTable->GetRecordItemValue(i, &desc);
		SetRecordItemValue(i, &desc);
	}
	m_mainTableSet.Update();
	return flag;
}

void CdbTable::TransferWaveFormatDataToRecord(const CWaveFormat* p_wave_format)
{
	// set time -- o_time
	COleDateTime o_time;
	auto t = p_wave_format->acqtime;
	o_time.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_table_acq_date), FALSE);
	m_mainTableSet.m_table_acq_date = o_time;
	m_mainTableSet.m_acqdate_time = o_time;
	o_time.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), 0, 0, 0);
	m_mainTableSet.m_acqdate_day = o_time;

	// set insect ID, sensillumID, repeat and repeat2, moreComment
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_IDinsect), FALSE);
	m_mainTableSet.m_IDinsect = p_wave_format->insectID;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_IDsensillum), FALSE);
	m_mainTableSet.m_IDsensillum = p_wave_format->sensillumID;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_repeat), FALSE);
	m_mainTableSet.m_repeat = p_wave_format->repeat;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_repeat2), FALSE);
	m_mainTableSet.m_repeat2 = p_wave_format->repeat2;
	m_mainTableSet.SetFieldNull(&(m_mainTableSet.m_more), FALSE);
	m_mainTableSet.m_more = p_wave_format->csMoreComment;

	// set type, stimulus and concentrations
	m_mainTableSet.m_operator_ID = m_operator_set.GetStringInLinkedTable(p_wave_format->csOperator);
	m_mainTableSet.m_insect_ID = m_insect_set.GetStringInLinkedTable(p_wave_format->csInsectname);
	m_mainTableSet.m_location_ID = m_locationSet.GetStringInLinkedTable(p_wave_format->csLocation);
	m_mainTableSet.m_expt_ID = m_expt_set.GetStringInLinkedTable(p_wave_format->cs_comment);
	m_mainTableSet.m_sensillum_ID = m_sensillum_set.GetStringInLinkedTable(p_wave_format->csSensillum);
	m_mainTableSet.m_stim_ID = m_stim_set.GetStringInLinkedTable(p_wave_format->csStimulus);
	m_mainTableSet.m_conc_ID = m_conc_set.GetStringInLinkedTable(p_wave_format->csConcentration);
	m_mainTableSet.m_stim2_ID = m_stim_set.GetStringInLinkedTable(p_wave_format->csStimulus2);
	m_mainTableSet.m_conc2_ID = m_conc_set.GetStringInLinkedTable(p_wave_format->csConcentration2);
	m_mainTableSet.m_sex_ID = m_sex_set.GetStringInLinkedTable(p_wave_format->csSex);
	m_mainTableSet.m_strain_ID = m_strain_set.GetStringInLinkedTable(p_wave_format->csStrain);
	m_mainTableSet.m_expt_ID = m_expt_set.GetStringInLinkedTable(p_wave_format->cs_comment);
	m_mainTableSet.m_flag = p_wave_format->flag;
}

void CdbTable::DeleteUnusedEntriesInAccessoryTables()
{
	DeleteUnusedEntriesInAttachedTable(&m_operator_set, CH_OPERATOR_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_insect_set, CH_INSECT_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_locationSet, CH_LOCATION_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_sensillum_set, CH_SENSILLUM_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_stim_set, CH_STIM_ID, CH_STIM2_ID);
	DeleteUnusedEntriesInAttachedTable(&m_conc_set, CH_CONC_ID, CH_CONC2_ID);
	DeleteUnusedEntriesInAttachedTable(&m_sex_set, CH_SEX_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_strain_set, CH_STRAIN_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_expt_set, CH_EXPT_ID, -1);
	DeleteUnusedEntriesInAttachedTable(&m_path_set, CH_PATH_ID, CH_PATH2_ID);
}

void CdbTable::DeleteUnusedEntriesInAttachedTable(CdbTableAssociated* p_index_table, const int index_column1, const int index_column2)
{
	if (p_index_table->IsBOF() && p_index_table->IsEOF())
		return;

	p_index_table->MoveFirst();
	while (!p_index_table->IsEOF())
	{
		COleVariant var_value1;
		p_index_table->GetFieldValue(1, var_value1);
		const auto id_current = var_value1.lVal;
		const auto flag1 = m_mainTableSet.FindIDinColumn(id_current, index_column1);
		auto flag2 = FALSE;
		if (index_column2 >= 0)
			flag2 = m_mainTableSet.FindIDinColumn(id_current, index_column2);
		if (flag1 == FALSE && flag2 == FALSE)
		{
			p_index_table->Delete();
		}
		p_index_table->MoveNext();
	}
}

boolean CdbTable::IsRecordTimeUnique(const COleDateTime& data_acquisition_date)
{
	boolean unique = true;

	ASSERT(m_mainTableSet.CanBookmark());
	if (m_mainTableSet.IsBOF() && m_mainTableSet.IsEOF())
		return unique;

	try
	{
		m_mainTableSet.MoveFirst();
		const auto ol = m_mainTableSet.GetBookmark();

		while (!m_mainTableSet.IsEOF())
		{
			if (data_acquisition_date == m_mainTableSet.m_table_acq_date)
			{
				unique = false;
				break;
			}
			m_mainTableSet.MoveNext();
		}
		m_mainTableSet.SetBookmark(ol);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 9);
		e->Delete();
	}
	return unique;
}

void CdbTable::CompactDataBase(const CString& file_name, const CString& file_name_new)
{
	// compact database and save new file
	CDaoWorkspace::CompactDatabase(file_name, file_name_new, dbLangGeneral, 0);
}

boolean CdbTable::GetRecordValueString(int column_index, CString& output_string)
{
	bool b_changed = false;
	DB_ITEMDESC desc;
	if (GetRecordItemValue(column_index, &desc))
	{
		b_changed = (output_string.Compare(desc.csVal) != 0);
		output_string = desc.csVal;
	}
	return b_changed;
}

boolean CdbTable::GetRecordValueLong(int column_index, long& value)
{
	DB_ITEMDESC desc;
	GetRecordItemValue(column_index, &desc);
	const boolean b_changed = value != desc.lVal;
	value = desc.lVal;
	return b_changed;
}
