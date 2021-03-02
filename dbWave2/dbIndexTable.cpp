
#include "StdAfx.h"
//#include <afxconv.h>           // For LPTSTR -> LPSTR macros
#include "dbWave.h"
#include "dbIndexTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CdbIndexTable, CDaoRecordset)

CdbIndexTable::CdbIndexTable(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	m_cs = _T("");
	m_ID = 0;
	m_nFields = 2;

	m_nDefaultType = dbOpenTable;
	m_defaultSQL = _T("[name]");
	m_DFX_cs = _T("[cs]");
	m_DFX_ID = _T("[ID]");

	m_defaultName.Empty();
}

// store strings and add "[" and "]"
void CdbIndexTable::SetNames(CString defaultSQL, CString DFX_cs, CString DFX_ID)
{
	m_defaultSQL = _T("[") + defaultSQL + _T("]");
	m_DFX_cs = _T("[") + DFX_cs + _T("]");
	m_DFX_ID = _T("[") + DFX_ID + _T("]");
}

CString CdbIndexTable::GetDefaultDBName()
{
	auto cs = m_defaultName;
	if (m_pDatabase->m_pDAODatabase != nullptr)
		cs = m_pDatabase->GetName();

	return cs;
}

CString CdbIndexTable::GetDefaultSQL()
{
	return m_defaultSQL;
}

void CdbIndexTable::DoFieldExchange(CDaoFieldExchange* pFX)
{
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Text(pFX, m_DFX_cs, m_cs);
	DFX_Long(pFX, m_DFX_ID, m_ID);
}

#ifdef _DEBUG
void CdbIndexTable::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CdbIndexTable::Dump(CDumpContext& dc) const
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

long CdbIndexTable::GetIDorCreateIDforString(const CString& cs)
{
	// string is empty - return nothing!
	if (cs.IsEmpty())
		return -1;

	// record not found: add a new record - none found or empty
	long i_id = -1;
	if (!GetIDFromString(cs, i_id))
	{
		try {
			// add new record: pass the text, ID will be updated automatically
			AddNew();
			SetFieldValue(0, COleVariant(cs, VT_BSTRT));
			Update();
			SetBookmark(GetLastModifiedBookmark());

			COleVariant var_value;
			GetFieldValue(1, var_value);
			i_id = var_value.lVal;
			ASSERT(GetIDFromString(cs, i_id));
		}
		catch (CDaoException* e) { DisplayDaoException(e, 711); e->Delete(); }
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

BOOL CdbIndexTable::GetIDFromString(CString cs, long& i_id)
{
	if (IsEOF() && IsBOF())
		return FALSE;

	auto bfound = FALSE;
	try {
		// seek record
		SetCurrentIndex(_T("NORM_OrderByIndex"));
		COleVariant csVal;
		csVal.SetString(cs, VT_BSTRT);
		bfound = Seek(_T("="), &csVal);
		// record found: get ID
		if (bfound)
		{
			COleVariant var_value1;
			GetFieldValue(1, var_value1);
			i_id = var_value1.lVal;
		}
	}
	catch (CDaoException* e) { DisplayDaoException(e, 18); e->Delete(); }
	return bfound;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CdbIndexTable::SeekID(long iID)
{
	auto bfound = FALSE;
	// find record with this ID and make it current
	COleVariant id = iID;
	try
	{
		SetCurrentIndex(_T("Primary Key"));
		bfound = Seek(_T("="), &id);
	}
	catch (CDaoException* e) {
		DisplayDaoException(e, 18);
		e->Delete();
	}
	return bfound;
}

/////////////////////////////////////////////////////////////////////////////

CString CdbIndexTable::GetStringFromID(long iID)
{
	auto bfound = SeekID(iID);
	CString cs;
	if (bfound)
	{
		COleVariant var_value1;
		GetFieldValue(0, var_value1);
		cs = var_value1.bstrVal;
	}
	return cs;
}

void CdbIndexTable::CreateIndextable(const CString& cstablename, const CString& cscol1, const CString& csIDcol2, int textSize, CDaoDatabase* p_db)
{
	SetNames(cstablename, cscol1, csIDcol2);		// change name of table, col1, col2

	CDaoTableDef tb(p_db);
	tb.Create(cstablename);

	// create first field in the table
	CDaoFieldInfo fd0;
	fd0.m_strName = cscol1;
	fd0.m_nType = dbText;					// Primary
	fd0.m_lSize = textSize;					// Primary
	fd0.m_lAttributes = dbVariableField;	// Primary
	fd0.m_nOrdinalPosition = 2;				// Secondary
	fd0.m_bRequired = FALSE;				// Secondary
	fd0.m_bAllowZeroLength = FALSE;			// Secondary
	fd0.m_lCollatingOrder = dbSortGeneral;
	fd0.m_strForeignName = _T("");			// Secondary
	fd0.m_strSourceField = _T("");			// Secondary
	fd0.m_strSourceTable = _T("");			// Secondary
	fd0.m_strValidationRule = _T("");		// All
	fd0.m_strValidationText = _T("");		// All
	fd0.m_strDefaultValue = _T("");			// All
	tb.CreateField(fd0);

	// create first index
	CDaoIndexFieldInfo indexfield0;
	indexfield0.m_strName = cscol1;
	indexfield0.m_bDescending = FALSE;

	CDaoIndexInfo indexfd0;
	indexfd0.m_pFieldInfos = &indexfield0;
	indexfd0.m_strName = _T("NORM_OrderByIndex");
	indexfd0.m_nFields = 1;
	indexfd0.m_bPrimary = FALSE;
	indexfd0.m_bUnique = TRUE;
	indexfd0.m_bClustered = FALSE;
	indexfd0.m_bIgnoreNulls = FALSE;
	indexfd0.m_bRequired = FALSE;
	indexfd0.m_bForeign = FALSE;
	tb.CreateIndex(indexfd0);

	// create second field
	fd0.m_strName = csIDcol2;
	fd0.m_nType = dbLong;					// Primary
	fd0.m_lSize = 4;						// Primary
	fd0.m_lAttributes = dbAutoIncrField;	// Primary
	fd0.m_nOrdinalPosition = 2;				// Secondary
	fd0.m_bRequired = TRUE;					// Secondary
	tb.CreateField(fd0);

	// create first index
	indexfd0.m_pFieldInfos = &indexfield0;
	indexfield0.m_strName = csIDcol2;
	indexfield0.m_bDescending = FALSE;

	indexfd0.m_strName = _T("Primary Key");
	indexfd0.m_pFieldInfos = &indexfield0;
	indexfd0.m_bPrimary = TRUE;
	indexfd0.m_bRequired = TRUE;
	tb.CreateIndex(indexfd0);

	tb.Append();
}

int	CdbIndexTable::AddStringsFromCombo(CComboBox* pcombo)
{
	const auto nitems = pcombo->GetCount();
	auto nadded = 0;
	long i_id;
	CString cs;
	try {
		for (auto i = 0; i < nitems; i++)
		{
			pcombo->GetLBText(i, cs);
			if (!GetIDFromString(cs, i_id))
			{
				i_id = GetIDorCreateIDforString(cs);
				nadded++;
			}
		}
	}
	catch (CDaoException* e) { DisplayDaoException(e, 31); e->Delete(); }
	return nadded;
}

int CdbIndexTable::RemoveStringsNotInCombo(CComboBox* pcombo)
{
	if (IsBOF() && IsEOF())
		return 0;

	auto ndeleted = 0;
	COleVariant var_value0;
	MoveFirst();
	while (!IsEOF())
	{
		GetFieldValue(0, var_value0);
		CString cs = var_value0.bstrVal;
		const auto i = pcombo->FindStringExact(0, cs);
		if (CB_ERR == i)
		{
			try {
				Delete();
				ndeleted++;
			}
			catch (CDaoException* e) { DisplayDaoException(e, 31); e->Delete(); }
		}
		MoveNext();
	}
	return ndeleted;
}