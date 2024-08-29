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
		m_main_table_set.m_desc[i].header_name = dummy;
		m_main_table_set.m_desc[i].dfx_name_with_brackets = _T("[") + dummy + _T("]");
		m_main_table_set.m_desc[i].cs_col_param = dummy + _T("Param");
		m_main_table_set.m_desc[i].cs_equ_condition = dummy + _T("=") + m_main_table_set.m_desc[i].cs_col_param;
		m_main_table_set.m_desc[i].data_code_number = m_column_properties[i].format_code_number;
	}

	set_attached_tables_names();

	m_main_table_set.m_strSort = m_column_properties[CH_ACQDATE].header_name;
}

void CdbTable::set_attached_tables_names()
{
	// Set_DFX_SQL_Names(CString defaultSQL /* or table name*/, CString DFX_cs, CString DFX_ID)
	m_stimulus_set.set_dfx_sql_names(m_column_properties[CH_STIM_ID].attached_table, _T("stim"), _T("stimID"));
	m_concentration_set.set_dfx_sql_names(m_column_properties[CH_CONC_ID].attached_table, _T("conc"), _T("concID"));
	m_operator_set.set_dfx_sql_names(m_column_properties[CH_OPERATOR_ID].attached_table, _T("operator"), _T("operatorID"));
	m_insect_set.set_dfx_sql_names(m_column_properties[CH_INSECT_ID].attached_table, _T("insect"), _T("insectID"));
	m_location_set.set_dfx_sql_names(m_column_properties[CH_LOCATION_ID].attached_table, _T("type"), _T("typeID"));
	m_path_set.set_dfx_sql_names(m_column_properties[CH_PATH_ID].attached_table, _T("path"), _T("pathID"));
	m_sensillum_set.set_dfx_sql_names(m_column_properties[CH_SENSILLUM_ID].attached_table, _T("stage"), _T("stageID"));
	m_sex_set.set_dfx_sql_names(m_column_properties[CH_SEX_ID].attached_table, _T("sex"), _T("sexID"));
	m_strain_set.set_dfx_sql_names(m_column_properties[CH_STRAIN_ID].attached_table, _T("strain"), _T("strainID"));
	m_experiment_set.set_dfx_sql_names(m_column_properties[CH_EXPT_ID].attached_table, _T("expt"), _T("exptID"));

}

boolean CdbTable::create_relations_with_attached_tables(const CString& cs_table)
{
	// create relations
	constexpr long l_attr = dbRelationDontEnforce; //dbRelationUpdateCascade;
	if (!create_relation_between_associated_table_and_1_column(cs_table, CH_INSECT_ID, l_attr, &m_insect_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(cs_table, CH_SENSILLUM_ID, l_attr, &m_sensillum_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(cs_table, CH_OPERATOR_ID, l_attr, &m_operator_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(cs_table, CH_LOCATION_ID, l_attr, &m_location_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(cs_table, CH_STRAIN_ID, l_attr, &m_strain_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(cs_table, CH_SEX_ID, l_attr, &m_sex_set)) return FALSE;
	if (!create_relation_between_associated_table_and_1_column(cs_table, CH_EXPT_ID, l_attr, &m_experiment_set)) return FALSE;

	if (!create_relation_between_associated_table_and_2_columns(cs_table, CH_PATH_ID, CH_PATH2_ID)) return FALSE;
	if (!create_relation_between_associated_table_and_2_columns(cs_table, CH_STIM_ID, CH_STIM2_ID)) return FALSE;
	if (!create_relation_between_associated_table_and_2_columns(cs_table, CH_CONC_ID, CH_CONC2_ID)) return FALSE;

	return true;
}

void CdbTable::attach(const CString& cs_data_file_name, const CString& cs_spike_file_name)
{
	current_data_filename = cs_data_file_name;
	current_spike_filename = cs_spike_file_name;
}

CdbTable::~CdbTable()
{
	close_database();
}

BOOL CdbTable::create_main_table(const CString& cs_table)
{
	CDaoTableDef table_def(this);
	table_def.Create(cs_table);

	//first create the main field, and ID number that is incremented automatically
	// when a new record is created. This column is indexed
	CDaoFieldInfo fd0;
	fd0.m_strName = m_main_table_set.m_desc[CH_ID].header_name; // "ID"
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
	table_def.CreateField(m_main_table_set.m_desc[i].header_name, dbDate, 8, 0); // 1 -acq_date

	fd0.m_bAllowZeroLength = TRUE;
	fd0.m_bRequired = FALSE;
	fd0.m_lAttributes = dbVariableField;
	fd0.m_nType = dbText;
	fd0.m_lSize = 255;

	for (i = 2; i <= 4; i++) // 2 -filename / 3 -file_spk // 4 - "acq_comment"
	{
		fd0.m_strName = m_main_table_set.m_desc[i].header_name;
		fd0.m_nOrdinalPosition = static_cast<short>(i);
		table_def.CreateField(fd0);
	}

	i = 5; // 5 - "more"
	fd0.m_strName = m_main_table_set.m_desc[i].header_name;
	fd0.m_nOrdinalPosition = i;
	fd0.m_nType = dbMemo;
	fd0.m_lSize = dbMemo;
	table_def.CreateField(fd0);

	for (i = 6; i <= 25; i++)
		table_def.CreateField(m_main_table_set.m_desc[i].header_name, dbLong, 4, 0); //  6 - insectID to 25 = sex_ID
	for (i = 26; i <= 27; i++)
		table_def.CreateField(m_main_table_set.m_desc[i].header_name, dbDate, 8, 0); // 26 - acq_date_day / 27 - acq_date_time
	i = 28;
	table_def.CreateField(m_main_table_set.m_desc[i].header_name, dbLong, 4, 0); // 28 - experiment_ID

	// create the corresponding indexes
	CDaoIndexFieldInfo index_field0;
	index_field0.m_bDescending = FALSE;

	CDaoIndexInfo index_fd0;
	index_field0.m_strName = m_main_table_set.m_desc[CH_ID].header_name; // ID
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

	return create_relations_with_attached_tables(cs_table);
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
		r_field[0].m_strForeignName = m_main_table_set.m_desc[column_index_1].header_name; // path_ID
		r_field[1].m_strName = r_field[0].m_strName;
		r_field[1].m_strForeignName = m_main_table_set.m_desc[column_index_2].header_name; // path2_ID
		rl_info.m_pFieldInfos = &r_field[0];
		rl_info.m_nFields = 2;
		CreateRelation(rl_info);

		m_main_table_set.m_desc[column_index_1].p_linked_set = &m_path_set;
		m_main_table_set.m_desc[column_index_1].associated_table_name = r_field[0].m_strName;

		m_main_table_set.m_desc[column_index_2].p_linked_set = &m_path_set;
		m_main_table_set.m_desc[column_index_2].associated_table_name = r_field[0].m_strName;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 5);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

// insect name: "table_insect_name" relates "insect" table with "ID" and "insect_name_ID"
BOOL CdbTable::create_relation_between_associated_table_and_1_column(const LPCTSTR lpsz_foreign_table, const int column_index,
	const long l_attributes, CdbTableAssociated* p_to_associated_table)
{
	try
	{
		const LPCTSTR lpsz_table = m_column_properties[column_index].attached_table;
		m_main_table_set.m_desc[column_index].associated_table_name = lpsz_table;
		CString cs_rel; // unique name of the relation object (max 40 chars)
		cs_rel.Format(_T("%s_%s"), lpsz_foreign_table, lpsz_table);
		CString sz_field = lpsz_table;
		sz_field += _T("ID");
		CreateRelation(cs_rel, lpsz_table, lpsz_foreign_table, l_attributes, sz_field,
			m_main_table_set.m_desc[column_index].header_name);
		m_main_table_set.m_desc[column_index].p_linked_set = p_to_associated_table;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 5);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

CString CdbTable::get_data_base_path()
{
	return m_database_path;
}

void CdbTable::set_data_base_path()
{
	const auto filename = GetName();
	const auto last_slash = filename.ReverseFind('\\');
	m_database_path = filename.Left(last_slash + 1);
	m_database_path.MakeLower();
}

void CdbTable::create_associated_tables()
{
	// create tables with indexes. These tables will contain categories entered by the user
	m_insect_set.create_index_table(_T("insectname"), _T("insect"), _T("insectID"), 100, this);
	m_sensillum_set.create_index_table(_T("sensillumname"), _T("stage"), _T("stageID"), 100, this);
	m_operator_set.create_index_table(_T("operator"), _T("operator"), _T("operatorID"), 50, this);
	m_stimulus_set.create_index_table(_T("stim"), _T("stim"), _T("stimID"), 100, this);
	m_concentration_set.create_index_table(_T("conc"), _T("conc"), _T("concID"), 100, this);
	m_location_set.create_index_table(_T("location"), _T("type"), _T("typeID"), 100, this);
	m_path_set.create_index_table(_T("path"), _T("path"), _T("pathID"), 255, this);
	m_strain_set.create_index_table(_T("strain"), _T("strain"), _T("strainID"), 100, this);
	m_sex_set.create_index_table(_T("sex"), _T("sex"), _T("sexID"), 10, this);
	m_experiment_set.create_index_table(_T("expt"), _T("expt"), _T("exptID"), 100, this);
}

void CdbTable::create_all_tables()
{
	set_data_base_path();
	create_associated_tables();
	create_main_table(_T("table"));

	// store links to attached table addresses
	for (auto column = 0; column < N_TABLE_COLUMNS; column++)
		get_record_item_descriptor(column);
}

BOOL CdbTable::open_tables()
{
	set_data_base_path();

	// check format of table set
	CDaoRecordset record_set(this);
	const CString cs_table = _T("table");

	// check for the presence of Table 'table'
	try
	{
		CDaoFieldInfo fd0;
		record_set.Open(dbOpenTable, cs_table);
		// check if column "filename" is present
		record_set.GetFieldInfo(m_main_table_set.m_desc[CH_FILENAME].header_name, fd0);
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
		open_associated_table(&m_stimulus_set);
		open_associated_table(&m_concentration_set);
		open_associated_table(&m_operator_set);
		open_associated_table(&m_insect_set);
		open_associated_table(&m_location_set);
		open_associated_table(&m_path_set);
		open_associated_table(&m_sensillum_set);
		open_associated_table(&m_sex_set);
		open_associated_table(&m_strain_set);
		open_associated_table(&m_experiment_set);
	}
	catch (CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
		return FALSE;
	}

	m_main_table_set.m_default_name = GetName();
	if (!m_main_table_set.open_table(dbOpenDynaset, nullptr, 0))
		return FALSE;

	// store links to attached table addresses
	for (auto column = 0; column < N_TABLE_COLUMNS; column++)
		get_record_item_descriptor(column);

	return TRUE;
}

void CdbTable::add_column_28(CDaoTableDef& table_def, const CString& cs_table, const long l_attr)
{
	table_def.Open(cs_table);
	table_def.CreateField(m_main_table_set.m_desc[CH_EXPT_ID].header_name, dbLong, 4, 0);
	m_experiment_set.create_index_table(_T("expt"), _T("expt"), _T("exptID"), 100, this);
	CreateRelation(_T("table_expt"), _T("expt"), cs_table, l_attr, _T("exptID"),
		m_main_table_set.m_desc[CH_EXPT_ID].header_name);
	table_def.Close();
}

void CdbTable::add_column_26_27(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const
{
	table_def.Open(cs_table);
	table_def.CreateField(m_main_table_set.m_desc[CH_ACQDATE_DAY].header_name, dbDate, 8, 0);
	table_def.CreateField(m_main_table_set.m_desc[CH_ACQDATE_TIME].header_name, dbDate, 8, 0);
	table_def.Close();
}

void CdbTable::add_column_24_25(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const
{
	table_def.Open(cs_table);
	table_def.CreateField(m_main_table_set.m_desc[CH_REPEAT].header_name, dbLong, 4, 0);
	table_def.CreateField(m_main_table_set.m_desc[CH_REPEAT2].header_name, dbLong, 4, 0);
	table_def.Close();
}

void CdbTable::add_column_22_23(CDaoTableDef& table_def, const CString& cs_table, long l_attr)
{
	table_def.Open(cs_table);

	// add fields in the main table, add the corresponding tables and the relations between the main table and the new index tables
	table_def.CreateField(m_main_table_set.m_desc[CH_STRAIN_ID].header_name, dbLong, 4, 0); // strain_ID
	table_def.CreateField(m_main_table_set.m_desc[CH_SEX_ID].header_name, dbLong, 4, 0); // sex_ID
	m_strain_set.create_index_table(_T("strain"), _T("strain"), _T("strainID"), 100, this);
	m_sex_set.create_index_table(_T("sex"), _T("sex"), _T("sexID"), 10, this);
	CreateRelation(_T("table_strain"), _T("strain"), cs_table, l_attr, _T("strainID"),
		m_main_table_set.m_desc[CH_STRAIN_ID].header_name); // strain_ID
	CreateRelation(_T("table_sex"), _T("sex"), cs_table, l_attr, _T("sexID"),
		m_main_table_set.m_desc[CH_SEX_ID].header_name); // sex_ID
// type -> location
	DeleteRelation(_T("table_type")); // delete relationship
	table_def.DeleteField(CH_LOCATION_ID);
	// delete the field (index is different because we deleted one field)
	table_def.CreateField(m_main_table_set.m_desc[CH_LOCATION_ID].header_name, dbLong, 4, 0); // locationID
	// stage -> sensillum name
	DeleteRelation(_T("table_stage")); // delete relationship
	table_def.DeleteField(CH_SENSILLUM_ID); // delete field
	table_def.CreateField(m_main_table_set.m_desc[CH_SENSILLUM_ID].header_name, dbLong, 4, 0);
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
		m_main_table_set.m_desc[CH_SENSILLUM_ID].header_name); // sensillum name_ID
	cs_rel = _T("table_location");
	CreateRelation(cs_rel, _T("location"), cs_table, l_attr, _T("typeID"),
		m_main_table_set.m_desc[CH_LOCATION_ID].header_name); //location_ID
	table_def.Close();
}

void CdbTable::add_column_21(CDaoTableDef& table_def, const CString& cs_table, long l_attr) const
{
	table_def.Open(cs_table);
	table_def.CreateField(m_main_table_set.m_desc[CH_FLAG].header_name, dbLong, 4, 0);
	table_def.Close();
}

void CdbTable::add_column_19_20(CDaoTableDef& table_def, const CString& cs_table, long l_attr)
{
	table_def.Open(cs_table);
	CString cs_rel = _T("table_Rel1");
	const auto i_pos = cs_rel.GetLength() - 1;
	table_def.CreateField(m_main_table_set.m_desc[CH_STIM2_ID].header_name, dbLong, 4, 0);
	table_def.CreateField(m_main_table_set.m_desc[CH_CONC2_ID].header_name, dbLong, 4, 0);
	cs_rel.SetAt(i_pos, '9');
	CreateRelation(cs_rel, _T("stim"), cs_table, l_attr, _T("stimID"),
		m_main_table_set.m_desc[CH_STIM2_ID].header_name);
	cs_rel.SetAt(i_pos, 'A');
	CreateRelation(cs_rel, _T("conc"), cs_table, l_attr, _T("concID"),
		m_main_table_set.m_desc[CH_CONC2_ID].header_name);
	table_def.Close();
}

void CdbTable::open_associated_table(CdbTableAssociated* p_index_table_set)
{
	p_index_table_set->m_default_name = GetName();
	p_index_table_set->Open(dbOpenTable, nullptr, 0);
}

void CdbTable::close_database()
{
	if (IsOpen())
	{
		// closing the workspace does not close the recordsets
		// so first close them
		if (m_main_table_set.IsOpen()) m_main_table_set.Close();
		if (m_operator_set.IsOpen()) m_operator_set.Close();
		if (m_insect_set.IsOpen()) m_insect_set.Close();
		if (m_location_set.IsOpen()) m_location_set.Close();
		if (m_sensillum_set.IsOpen()) m_sensillum_set.Close();
		if (m_path_set.IsOpen()) m_path_set.Close();
		if (m_stimulus_set.IsOpen()) m_stimulus_set.Close();
		if (m_concentration_set.IsOpen()) m_concentration_set.Close();
		if (m_sex_set.IsOpen()) m_sex_set.Close();
		if (m_strain_set.IsOpen()) m_strain_set.Close();
		if (m_experiment_set.IsOpen()) m_experiment_set.Close();

		// close the workspace
		m_pWorkspace->Close();
		Close();
	}
}

void CdbTable::update_all_database_tables()
{
	// For each table, call Update() if GetEditMode ()
	//	= dbEditInProgress (Edit has been called)
	//	= dbEditAdd (AddNew has been called)
	if (m_main_table_set.GetEditMode() != dbEditNone)
		m_main_table_set.Update();
	if (m_operator_set.GetEditMode() != dbEditNone)
		m_operator_set.Update();
	if (m_insect_set.GetEditMode() != dbEditNone)
		m_insect_set.Update();
	if (m_location_set.GetEditMode() != dbEditNone)
		m_location_set.Update();
	if (m_sensillum_set.GetEditMode() != dbEditNone)
		m_sensillum_set.Update();
	if (m_path_set.GetEditMode() != dbEditNone)
		m_path_set.Update();
	if (m_sex_set.GetEditMode() != dbEditNone)
		m_sex_set.Update();
	if (m_strain_set.GetEditMode() != dbEditNone)
		m_strain_set.Update();
	if (m_experiment_set.GetEditMode() != dbEditNone)
		m_experiment_set.Update();
}

CString CdbTable::get_file_path(const int i_id)
{
	auto cs_path = m_path_set.get_string_from_id(i_id);
	if (is_relative_path(cs_path))
		cs_path = m_database_path + cs_path.Right(cs_path.GetLength()-2);
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
	CString cs_out(sz_out);
	if (!flag)
		cs_out.Empty();

	return cs_out;
}

long CdbTable::get_relative_path_from_id(const long i_id)
{
	long new_id = -1;
	const auto cs_path = m_path_set.get_string_from_id(i_id);

	if (!is_relative_path(cs_path))
	{
		const auto cs_relative_path = get_relative_path_from_string(cs_path);
		if (!cs_relative_path.IsEmpty())
		{
			new_id = m_path_set.get_string_in_linked_table(cs_relative_path);
		}
	}
	else
		new_id = i_id;
	return new_id;
}

void CdbTable::convert_path_to_relative_path(const long i_col_path)
{
	COleVariant var_value;
	m_main_table_set.GetFieldValue(i_col_path, var_value);
	const auto path_id = var_value.lVal;

	const auto i_id = get_relative_path_from_id(path_id);
	if (i_id != path_id && i_id != -1)
	{
		m_main_table_set.Edit();
		var_value.lVal = i_id;
		m_main_table_set.SetFieldValue(i_col_path, var_value.lVal);
		m_main_table_set.Update();
	}
}

void CdbTable::set_path_relative()
{
	ASSERT(m_main_table_set.CanBookmark());
	if (m_main_table_set.IsBOF() && m_main_table_set.IsEOF())
		return;

	try
	{
		const auto ol = m_main_table_set.GetBookmark();
		m_main_table_set.MoveFirst();
		const auto i_col_path = m_main_table_set.get_column_index(_T("path_ID"));
		const auto i_col_path2 = m_main_table_set.get_column_index(_T("path2_ID"));

		while (!m_main_table_set.IsEOF())
		{
			convert_path_to_relative_path(i_col_path);
			convert_path_to_relative_path(i_col_path2);
			m_main_table_set.MoveNext();
		}
		m_main_table_set.SetBookmark(ol);
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
	const auto cs_path = m_path_set.get_string_from_id(i_id);
	if (is_relative_path(cs_path))
	{
		const auto cs_absolute_path = get_absolute_path_from_string(cs_path);
		if (!cs_absolute_path.IsEmpty())
		{
			new_id = m_path_set.get_string_in_linked_table(cs_absolute_path);
		}
	}
	else
		new_id = i_id;
	return new_id;
}

void CdbTable::convert_to_absolute_path(const int i_col_path)
{
	COleVariant var_value;
	m_main_table_set.GetFieldValue(i_col_path, var_value);
	const auto path_id = var_value.lVal;

	const auto i_id = get_absolute_path_from_id(path_id);
	if (i_id != path_id && i_id != -1)
	{
		m_main_table_set.Edit();
		var_value.lVal = i_id;
		m_main_table_set.SetFieldValue(i_col_path, var_value.lVal);
		m_main_table_set.Update();
	}
}

void CdbTable::set_path_absolute()
{
	ASSERT(m_main_table_set.CanBookmark());
	if (m_main_table_set.IsBOF() && m_main_table_set.IsEOF())
		return;

	try
	{
		const auto ol = m_main_table_set.GetBookmark();
		m_main_table_set.MoveFirst();
		const auto col_path = m_main_table_set.get_column_index(_T("path_ID"));
		const auto col_path2 = m_main_table_set.get_column_index(_T("path2_ID"));

		while (!m_main_table_set.IsEOF())
		{
			convert_to_absolute_path(col_path);
			convert_to_absolute_path(col_path2);
			m_main_table_set.MoveNext();
		}
		m_main_table_set.SetBookmark(ol);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 9);
		e->Delete();
	}
}

void CdbTable::get_current_record_file_names()
{
	current_data_filename = get_current_record_data_file_name();
	current_spike_filename = get_current_record_spike_file_name();
}

CString CdbTable::get_current_record_data_file_name()
{
	CString filename;
	filename.Empty();
	if (!m_main_table_set.IsFieldNull(&m_main_table_set.m_file_dat) && !m_main_table_set.m_file_dat.IsEmpty())
	{
		filename = get_file_path(m_main_table_set.m_path_id) + '\\' + m_main_table_set.m_file_dat;
	}
	return filename;
}

CString CdbTable::get_current_record_spike_file_name()
{
	CString filename;
	filename.Empty();

	// set current spk document
	if (!m_main_table_set.IsFieldNull(&m_main_table_set.m_file_spk) && !m_main_table_set.m_file_spk.IsEmpty())
	{
		if (m_main_table_set.IsFieldNull(&m_main_table_set.m_path2_id))
		{
			m_main_table_set.Edit();
			m_main_table_set.m_path2_id = m_main_table_set.m_path_id;
			m_main_table_set.Update();
		}
		filename = get_file_path(m_main_table_set.m_path2_id) + '\\' + m_main_table_set.m_file_spk;
	}
	return filename;
}

BOOL CdbTable::move_to_id(const long record_id)
{
	CString str;
	str.Format(_T("ID=%li"), record_id);
	try
	{
		if (!m_main_table_set.FindFirst(str))
			return FALSE;
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 36);
		e->Delete();
	}
	get_current_record_file_names();
	return TRUE;
}

BOOL CdbTable::move_to(UINT n_id_move_command)
{
	update_all_database_tables();
	auto flag = TRUE;
	switch (n_id_move_command)
	{
	case ID_RECORD_PREV:
		m_main_table_set.MovePrev();
		if (!m_main_table_set.IsBOF())
			break;
		// Fall through to reset to first record
		flag = FALSE;

	case ID_RECORD_FIRST:
		m_main_table_set.MoveFirst();
		break;

	case ID_RECORD_NEXT:
		m_main_table_set.MoveNext();
		if (!m_main_table_set.IsEOF())
			break;
		// Fall through to reset to last record
		flag = FALSE;

	case ID_RECORD_LAST:
		m_main_table_set.MoveLast();
		break;

	default:
		// Unexpected case value
		ASSERT(FALSE);
		break;
	}
	get_current_record_file_names();
	return flag;
}

BOOL CdbTable::set_index_current_file(long i_file)
{
	// save any pending edit or add operation
	update_all_database_tables();
	const int record_count = m_main_table_set.GetRecordCount();
	if (i_file < 0 || record_count < 1)
		return FALSE;

	auto flag = TRUE;
	try
	{
		if (i_file >= record_count)
			i_file = record_count - 1;
	/*	if (i_file != m_mainTableSet.GetAbsolutePosition()) */
		m_main_table_set.SetAbsolutePosition(i_file);
		get_current_record_file_names();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 13);
		e->Delete();
		flag = FALSE;
	}
	return flag;
}

// fill in DB_ITEM_DESC structure
// input:
// iCH = column of m_psz... (see CH_.. definitions in dbMainTable)
// pointer to DB_ITEM_DESC structure
// output:
// DB_ITEM_DESC filled
// flag = FALSE if the column is not found

DB_ITEMDESC* CdbTable::get_record_item_descriptor(const int column_index)
{
	const auto p_desc = &m_main_table_set.m_desc[column_index];
	p_desc->index = column_index;

	switch (column_index)
	{
	case CH_ID:
		p_desc->pdata_item = &m_main_table_set.m_id;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_ACQDATE:
		p_desc->pdata_item = nullptr;
		ASSERT(p_desc->data_code_number == FIELD_DATE);
		break;
	case CH_FILENAME:
	case CH_FILESPK:
	case CH_ACQ_COMMENTS:
	case CH_MORE:
		p_desc->pdata_item = nullptr;
		ASSERT(p_desc->data_code_number == FIELD_TEXT);
		break;
	case CH_IDINSECT:
		p_desc->pdata_item = &m_main_table_set.m_id_insect;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_IDSENSILLUM:
		p_desc->pdata_item = &m_main_table_set.m_id_sensillum;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_DATALEN:
		p_desc->pdata_item = &m_main_table_set.m_data_len;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_NSPIKES:
		p_desc->pdata_item = &m_main_table_set.m_n_spikes;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_NSPIKECLASSES:
		p_desc->pdata_item = &m_main_table_set.m_n_spike_classes;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_FLAG:
		p_desc->pdata_item = &m_main_table_set.m_flag;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_INSECT_ID:
		p_desc->pdata_item = &m_main_table_set.m_insect_id;
		p_desc->p_linked_set = &m_insect_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_SENSILLUM_ID:
		p_desc->pdata_item = &m_main_table_set.m_sensillum_id;
		p_desc->p_linked_set = &m_sensillum_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_OPERATOR_ID:
		p_desc->pdata_item = &m_main_table_set.m_operator_id;
		p_desc->p_linked_set = &m_operator_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_STIM_ID:
		p_desc->pdata_item = &m_main_table_set.m_stimulus_id;
		p_desc->p_linked_set = &m_stimulus_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_CONC_ID:
		p_desc->pdata_item = &m_main_table_set.m_concentration_id;
		p_desc->p_linked_set = &m_concentration_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_LOCATION_ID:
		p_desc->pdata_item = &m_main_table_set.m_location_id;
		p_desc->p_linked_set = &m_location_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_PATH_ID:
		p_desc->pdata_item = &m_main_table_set.m_path_id;
		p_desc->p_linked_set = &m_path_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_FILEPATH);
		break;
	case CH_PATH2_ID:
		p_desc->pdata_item = &m_main_table_set.m_path2_id;
		p_desc->p_linked_set = &m_path_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_FILEPATH);
		break;
	case CH_STIM2_ID:
		p_desc->pdata_item = &m_main_table_set.m_stimulus2_id;
		p_desc->p_linked_set = &m_stimulus_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_CONC2_ID:
		p_desc->pdata_item = &m_main_table_set.m_concentration2_id;
		p_desc->p_linked_set = &m_concentration_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_STRAIN_ID:
		p_desc->pdata_item = &m_main_table_set.m_strain_id;
		p_desc->p_linked_set = &m_strain_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_SEX_ID:
		p_desc->pdata_item = &m_main_table_set.m_sex_id;
		p_desc->p_linked_set = &m_sex_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;
	case CH_REPEAT:
		p_desc->pdata_item = &m_main_table_set.m_repeat;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_REPEAT2:
		p_desc->pdata_item = &m_main_table_set.m_repeat2;
		ASSERT(p_desc->data_code_number == FIELD_LONG);
		break;
	case CH_ACQDATE_DAY:
		p_desc->pdata_item = nullptr;
		ASSERT(p_desc->data_code_number == FIELD_DATE_YMD);
		break;
	case CH_ACQDATE_TIME:
		p_desc->pdata_item = nullptr;
		ASSERT(p_desc->data_code_number == FIELD_DATE_HMS);
		break;
	case CH_EXPT_ID:
		p_desc->pdata_item = &m_main_table_set.m_experiment_id;
		p_desc->p_linked_set = &m_experiment_set;
		ASSERT(p_desc->data_code_number == FIELD_IND_TEXT);
		break;

		// if it comes here, the program must have crashed because p_desc is not defined...
	default:
		return nullptr;
	}

	return p_desc;
}

DB_ITEMDESC* CdbTable::get_record_item_value(const int column_index)
{
	const auto p_desc = &m_main_table_set.m_desc[column_index];
	p_desc->index = column_index;
	get_record_item_value(column_index, p_desc);
	return p_desc;
}

BOOL CdbTable::get_record_item_value(const int i_column, DB_ITEMDESC* db_item_descriptor)
{
	auto flag = TRUE;
	COleVariant var_value;
	m_main_table_set.GetFieldValue(m_main_table_set.m_desc[i_column].header_name, var_value);
	const int data_code_number = m_column_properties[i_column].format_code_number;

	switch (data_code_number)
	{
	case FIELD_IND_TEXT:
	case FIELD_IND_FILEPATH:
		db_item_descriptor->l_val = var_value.lVal;
		db_item_descriptor->cs_val = m_main_table_set.m_desc[i_column].p_linked_set->get_string_from_id(var_value.lVal);
		if (i_column == CH_EXPT_ID && db_item_descriptor->cs_val.IsEmpty())
		{
			const auto cs = db_item_descriptor->cs_val = GetName();
			const auto left = cs.ReverseFind(_T('\\'));
			const auto right = cs.ReverseFind(_T('.'));
			db_item_descriptor->cs_val = cs.Mid(left + 1, right - left - 1);
		}
		break;
	case FIELD_LONG:
		db_item_descriptor->l_val = var_value.lVal;
		if (var_value.vt == VT_NULL)
			db_item_descriptor->l_val = 0;
		break;
	case FIELD_TEXT:
		m_main_table_set.GetFieldValue(m_main_table_set.m_desc[i_column].header_name, var_value);
		db_item_descriptor->cs_val = V_BSTRT(&var_value);
		break;
	case FIELD_DATE:
	case FIELD_DATE_HMS:
	case FIELD_DATE_YMD:
		db_item_descriptor->o_val = var_value.date;
		break;
	default:
		flag = FALSE;
		break;
	}

	return flag;
}

BOOL CdbTable::set_record_item_value(const int column_index, DB_ITEMDESC* db_item_descriptor)
{
	auto flag = TRUE;
	const int data_code_number = m_column_properties[column_index].format_code_number;
	switch (data_code_number) {
	case FIELD_IND_TEXT:
	case FIELD_IND_FILEPATH:
	{
		long dummy_id = m_main_table_set.m_desc[column_index].p_linked_set->get_string_in_linked_table(db_item_descriptor->cs_val);
		if (dummy_id >= 0)
		{
			COleVariant var_value;
			var_value.lVal = dummy_id;
			m_main_table_set.SetFieldValue(m_main_table_set.m_desc[column_index].header_name, var_value.lVal);
		}
	}
	break;
	case FIELD_LONG:
		m_main_table_set.SetFieldValue(m_main_table_set.m_desc[column_index].header_name, db_item_descriptor->l_val);
		break;
	case FIELD_TEXT:
	{
		COleVariant var_value = db_item_descriptor->cs_val;
		m_main_table_set.SetFieldValue(m_main_table_set.m_desc[column_index].header_name, var_value);
	}
	break;
	case FIELD_DATE:
	case FIELD_DATE_HMS:
	case FIELD_DATE_YMD:
		m_main_table_set.SetFieldValue(m_main_table_set.m_desc[column_index].header_name, db_item_descriptor->o_val);
		break;
	default:
		flag = FALSE;
		break;
	}

	return flag;
}

BOOL CdbTable::import_record_from_database(CdbTable* p_external_db_table)
{
	// variables
	DB_ITEMDESC desc;
	constexpr auto flag = TRUE;
	// insert new record
	m_main_table_set.AddNew();

	// copy each field of the source database into current
	for (auto i = 1; i < m_main_table_set.m_nFields; i++)
	{
		p_external_db_table->get_record_item_value(i, &desc);
		set_record_item_value(i, &desc);
	}
	m_main_table_set.Update();
	return flag;
}

void CdbTable::transfer_wave_format_data_to_record(const CWaveFormat* p_wave_format)
{
	// set time -- o_time
	COleDateTime o_time;
	const auto t = p_wave_format->acquisition_time;
	o_time.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	m_main_table_set.SetFieldNull(&(m_main_table_set.m_table_acq_date), FALSE);
	m_main_table_set.m_table_acq_date = o_time;
	m_main_table_set.m_acq_date_time = o_time;
	o_time.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), 0, 0, 0);
	m_main_table_set.m_acq_date_day = o_time;

	// set insect ID, sensillumID, repeat and repeat2, moreComment
	m_main_table_set.SetFieldNull(&(m_main_table_set.m_id_insect), FALSE);
	m_main_table_set.m_id_insect = p_wave_format->insect_id;
	m_main_table_set.SetFieldNull(&(m_main_table_set.m_id_sensillum), FALSE);
	m_main_table_set.m_id_sensillum = p_wave_format->sensillum_id;
	m_main_table_set.SetFieldNull(&(m_main_table_set.m_repeat), FALSE);
	m_main_table_set.m_repeat = p_wave_format->repeat;
	m_main_table_set.SetFieldNull(&(m_main_table_set.m_repeat2), FALSE);
	m_main_table_set.m_repeat2 = p_wave_format->repeat2;
	m_main_table_set.SetFieldNull(&(m_main_table_set.m_more), FALSE);
	m_main_table_set.m_more = p_wave_format->cs_more_comment;

	// set type, stimulus and concentrations
	m_main_table_set.m_operator_id = m_operator_set.get_string_in_linked_table(p_wave_format->cs_operator);
	m_main_table_set.m_insect_id = m_insect_set.get_string_in_linked_table(p_wave_format->cs_insect_name);
	m_main_table_set.m_location_id = m_location_set.get_string_in_linked_table(p_wave_format->cs_location);
	m_main_table_set.m_experiment_id = m_experiment_set.get_string_in_linked_table(p_wave_format->cs_comment);
	m_main_table_set.m_sensillum_id = m_sensillum_set.get_string_in_linked_table(p_wave_format->cs_sensillum);
	m_main_table_set.m_stimulus_id = m_stimulus_set.get_string_in_linked_table(p_wave_format->cs_stimulus);
	m_main_table_set.m_concentration_id = m_concentration_set.get_string_in_linked_table(p_wave_format->cs_concentration);
	m_main_table_set.m_stimulus2_id = m_stimulus_set.get_string_in_linked_table(p_wave_format->cs_stimulus2);
	m_main_table_set.m_concentration2_id = m_concentration_set.get_string_in_linked_table(p_wave_format->cs_concentration2);
	m_main_table_set.m_sex_id = m_sex_set.get_string_in_linked_table(p_wave_format->cs_sex);
	m_main_table_set.m_strain_id = m_strain_set.get_string_in_linked_table(p_wave_format->cs_strain);
	m_main_table_set.m_experiment_id = m_experiment_set.get_string_in_linked_table(p_wave_format->cs_comment);
	m_main_table_set.m_flag = p_wave_format->flag;
}

void CdbTable::delete_unused_entries_in_accessory_tables()
{
	delete_unused_entries_in_attached_table(&m_operator_set, CH_OPERATOR_ID, -1);
	delete_unused_entries_in_attached_table(&m_insect_set, CH_INSECT_ID, -1);
	delete_unused_entries_in_attached_table(&m_location_set, CH_LOCATION_ID, -1);
	delete_unused_entries_in_attached_table(&m_sensillum_set, CH_SENSILLUM_ID, -1);
	delete_unused_entries_in_attached_table(&m_stimulus_set, CH_STIM_ID, CH_STIM2_ID);
	delete_unused_entries_in_attached_table(&m_concentration_set, CH_CONC_ID, CH_CONC2_ID);
	delete_unused_entries_in_attached_table(&m_sex_set, CH_SEX_ID, -1);
	delete_unused_entries_in_attached_table(&m_strain_set, CH_STRAIN_ID, -1);
	delete_unused_entries_in_attached_table(&m_experiment_set, CH_EXPT_ID, -1);
	delete_unused_entries_in_attached_table(&m_path_set, CH_PATH_ID, CH_PATH2_ID);
}

void CdbTable::delete_unused_entries_in_attached_table(CdbTableAssociated* p_index_table, const int index_column1, const int index_column2)
{
	if (p_index_table->IsBOF() && p_index_table->IsEOF())
		return;

	p_index_table->MoveFirst();
	while (!p_index_table->IsEOF())
	{
		COleVariant var_value1;
		p_index_table->GetFieldValue(1, var_value1);
		const auto id_current = var_value1.lVal;
		const auto flag1 = m_main_table_set.find_id_in_column(id_current, index_column1);
		auto flag2 = FALSE;
		if (index_column2 >= 0)
			flag2 = m_main_table_set.find_id_in_column(id_current, index_column2);
		if (flag1 == FALSE && flag2 == FALSE)
		{
			p_index_table->Delete();
		}
		p_index_table->MoveNext();
	}
}

boolean CdbTable::is_record_time_unique(const COleDateTime& data_acquisition_date)
{
	boolean unique = true;

	ASSERT(m_main_table_set.CanBookmark());
	if (m_main_table_set.IsBOF() && m_main_table_set.IsEOF())
		return unique;

	try
	{
		m_main_table_set.MoveFirst();
		const auto ol = m_main_table_set.GetBookmark();

		while (!m_main_table_set.IsEOF())
		{
			if (data_acquisition_date == m_main_table_set.m_table_acq_date)
			{
				unique = false;
				break;
			}
			m_main_table_set.MoveNext();
		}
		m_main_table_set.SetBookmark(ol);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 9);
		e->Delete();
	}
	return unique;
}

void CdbTable::compact_data_base(const CString& file_name, const CString& file_name_new)
{
	// compact database and save new file
	CDaoWorkspace::CompactDatabase(file_name, file_name_new, dbLangGeneral, 0);
}

boolean CdbTable::get_record_value_string(int column_index, CString& output_string)
{
	bool b_changed = false;
	DB_ITEMDESC desc;
	if (get_record_item_value(column_index, &desc))
	{
		b_changed = (output_string.Compare(desc.cs_val) != 0);
		output_string = desc.cs_val;
	}
	return b_changed;
}

boolean CdbTable::get_record_value_long(int column_index, long& value)
{
	DB_ITEMDESC desc;
	get_record_item_value(column_index, &desc);
	const boolean b_changed = value != desc.l_val;
	value = desc.l_val;
	return b_changed;
}
