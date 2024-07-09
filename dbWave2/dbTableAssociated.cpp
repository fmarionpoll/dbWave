#include "StdAfx.h"
#include "dbWave.h"
#include "dbTableAssociated.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CdbTableAssociated, CDaoRecordset)

CdbTableAssociated::CdbTableAssociated(CDaoDatabase* p_database)
	: CDaoRecordset(p_database)
{
	m_nFields = 2;
	m_nDefaultType = dbOpenTable;
}

// store strings and add "[" and "]"
void CdbTableAssociated::set_dfx_sql_names(const CString& cs_default_sql, const CString& dfx_cs, const CString& dfx_id)
{
	m_default_sql = _T("[") + cs_default_sql + _T("]");
	m_dfx_cs = _T("[") + dfx_cs + _T("]");
	m_dfx_id = _T("[") + dfx_id + _T("]");
}

CString CdbTableAssociated::GetDefaultDBName()
{
	auto cs = m_default_name;
	if (m_pDatabase->m_pDAODatabase != nullptr)
		cs = m_pDatabase->GetName();

	return cs;
}

CString CdbTableAssociated::GetDefaultSQL()
{
	return m_default_sql;
}

void CdbTableAssociated::DoFieldExchange(CDaoFieldExchange* p_fx)
{
	p_fx->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Text(p_fx, m_dfx_cs, m_cs);
	DFX_Long(p_fx, m_dfx_id, m_id);
}

#ifdef _DEBUG
void CdbTableAssociated::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CdbTableAssociated::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// find string within linked table
// add new record in this linked table with the string if not found
// return index of the string in this table
//
// Parameters (in):
//		cs			input string to find/add in the linked table
//		iID			address of the main table record where assoc table index will be stored
// Parameters (out):
//		iID			assoc table index or -1

long CdbTableAssociated::get_string_in_linked_table(const CString& cs)
{
	// string is empty - return nothing!
	if (cs.IsEmpty())
		return -1;

	// record not found: add a new record - none found or empty
	long i_id = -1;
	if (!get_id_from_string(cs, i_id))
	{
		try
		{
			// add new record: pass the text, ID will be updated automatically
			AddNew();
			SetFieldValue(0, COleVariant(cs, VT_BSTRT));
			Update();
			SetBookmark(GetLastModifiedBookmark());

			COleVariant var_value;
			GetFieldValue(1, var_value);
			i_id = var_value.lVal;
			ASSERT(get_id_from_string(cs, i_id));
		}
		catch (CDaoException* e)
		{
			DisplayDaoException(e, 711);
			e->Delete();
		}
	}
	return i_id;
}

/////////////////////////////////////////////////////////////////////////////
// Parameters (in):
//		cs		CString to find in the table
//		iID		address of the parameter which will receive the record ID (if found)
//		DAO recordset (table) assuming first col = txt and called "NORM_OrderByIndex"
// Parameters (out):
//		BOOL	record found (TRUE) or not (FALSE)
//		iID		record ID (if found in the table) or unchanged (if not found)

BOOL CdbTableAssociated::get_id_from_string(const CString& cs, long& i_id)
{
	if (IsEOF() && IsBOF())
		return FALSE;

	auto b_found = FALSE;
	try
	{
		// seek record
		SetCurrentIndex(_T("NORM_OrderByIndex"));
		COleVariant csVal;
		csVal.SetString(cs, VT_BSTRT);
		b_found = Seek(_T("="), &csVal);
		// record found: get ID
		if (b_found)
		{
			COleVariant var_value1;
			GetFieldValue(1, var_value1);
			i_id = var_value1.lVal;
		}
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 18);
		e->Delete();
	}
	return b_found;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CdbTableAssociated::seek_id(const long i_id)
{
	auto is_found = FALSE;
	COleVariant id = i_id;
	try
	{
		SetCurrentIndex(_T("Primary Key"));
		is_found = Seek(_T("="), &id);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 18);
		e->Delete();
	}
	return is_found;
}

/////////////////////////////////////////////////////////////////////////////

CString CdbTableAssociated::get_string_from_id(const long i_id)
{
	const auto found = seek_id(i_id);
	CString cs;
	if (found)
	{
		COleVariant var_value1;
		GetFieldValue(0, var_value1);
		cs = var_value1.bstrVal;
	}
	return cs;
}

void CdbTableAssociated::create_index_table(const CString& cs_table, const CString& cs_column1, const CString& cs_id_column2,
                                          const int text_size, CDaoDatabase* p_database)
{
	set_dfx_sql_names(cs_table, cs_column1, cs_id_column2); // change name of table, col1, col2

	CDaoTableDef tb(p_database);
	tb.Create(cs_table);

	// create first field in the table
	CDaoFieldInfo fd0;
	fd0.m_strName = cs_column1;
	fd0.m_nType = dbText; // Primary
	fd0.m_lSize = text_size; // Primary
	fd0.m_lAttributes = dbVariableField; // Primary
	fd0.m_nOrdinalPosition = 2; // Secondary
	fd0.m_bRequired = FALSE; // Secondary
	fd0.m_bAllowZeroLength = FALSE; // Secondary
	fd0.m_lCollatingOrder = dbSortGeneral;
	fd0.m_strForeignName = _T(""); // Secondary
	fd0.m_strSourceField = _T(""); // Secondary
	fd0.m_strSourceTable = _T(""); // Secondary
	fd0.m_strValidationRule = _T(""); // All
	fd0.m_strValidationText = _T(""); // All
	fd0.m_strDefaultValue = _T(""); // All
	tb.CreateField(fd0);

	// create first index
	CDaoIndexFieldInfo index_field0;
	index_field0.m_strName = cs_column1;
	index_field0.m_bDescending = FALSE;

	CDaoIndexInfo index_fd0;
	index_fd0.m_pFieldInfos = &index_field0;
	index_fd0.m_strName = _T("NORM_OrderByIndex");
	index_fd0.m_nFields = 1;
	index_fd0.m_bPrimary = FALSE;
	index_fd0.m_bUnique = TRUE;
	index_fd0.m_bClustered = FALSE;
	index_fd0.m_bIgnoreNulls = FALSE;
	index_fd0.m_bRequired = FALSE;
	index_fd0.m_bForeign = FALSE;
	tb.CreateIndex(index_fd0);

	// create second field
	fd0.m_strName = cs_id_column2;
	fd0.m_nType = dbLong; // Primary
	fd0.m_lSize = 4; // Primary
	fd0.m_lAttributes = dbAutoIncrField; // Primary
	fd0.m_nOrdinalPosition = 2; // Secondary
	fd0.m_bRequired = TRUE; // Secondary
	tb.CreateField(fd0);

	// create first index
	index_fd0.m_pFieldInfos = &index_field0;
	index_field0.m_strName = cs_id_column2;
	index_field0.m_bDescending = FALSE;

	index_fd0.m_strName = _T("Primary Key");
	index_fd0.m_pFieldInfos = &index_field0;
	index_fd0.m_bPrimary = TRUE;
	index_fd0.m_bRequired = TRUE;
	tb.CreateIndex(index_fd0);

	tb.Append();
}

int CdbTableAssociated::add_strings_from_combo(const CComboBox* p_combo)
{
	const auto n_items = p_combo->GetCount();
	auto n_added = 0;
	try
	{
		CString cs;
		long i_id;
		for (auto i = 0; i < n_items; i++)
		{
			p_combo->GetLBText(i, cs);
			if (!get_id_from_string(cs, i_id))
			{
				i_id = get_string_in_linked_table(cs);
				n_added++;
			}
		}
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 31);
		e->Delete();
	}
	return n_added;
}

int CdbTableAssociated::remove_strings_not_in_combo(const CComboBox* p_combo)
{
	if (IsBOF() && IsEOF())
		return 0;

	auto n_deleted = 0;
	COleVariant var_value0;
	MoveFirst();
	while (!IsEOF())
	{
		GetFieldValue(0, var_value0);
		CString cs = var_value0.bstrVal;
		const auto i = p_combo->FindStringExact(0, cs);
		if (CB_ERR == i)
		{
			try
			{
				Delete();
				n_deleted++;
			}
			catch (CDaoException* e)
			{
				DisplayDaoException(e, 31);
				e->Delete();
			}
		}
		MoveNext();
	}
	return n_deleted;
}
