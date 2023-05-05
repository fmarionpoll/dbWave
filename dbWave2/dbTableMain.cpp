#include "StdAfx.h"
#include "dbWave.h"
#include "dbTableMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CdbTableMain, CDaoRecordset)

CdbTableMain::CdbTableMain(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	m_desc[CH_ID].pdataItem					= &m_ID;
	m_desc[CH_IDINSECT].pdataItem			= &m_IDinsect;
	m_desc[CH_IDSENSILLUM].pdataItem		= &m_IDsensillum;
	m_desc[CH_DATALEN].pdataItem			= &m_datalen;
	m_desc[CH_NSPIKES].pdataItem			= &m_nspikes;

	m_desc[CH_NSPIKECLASSES].pdataItem		= &m_nspikeclasses;
	m_desc[CH_FLAG].pdataItem				= &m_flag;
	m_desc[CH_INSECT_ID].pdataItem			= &m_insect_ID;
	m_desc[CH_SENSILLUM_ID].pdataItem		= &m_sensillum_ID;
	m_desc[CH_OPERATOR_ID].pdataItem		= &m_operator_ID;

	m_desc[CH_STIM_ID].pdataItem			= &m_stim_ID;
	m_desc[CH_CONC_ID].pdataItem			= &m_conc_ID;
	m_desc[CH_LOCATION_ID].pdataItem		= &m_location_ID;
	m_desc[CH_PATH_ID].pdataItem			= &m_path_ID;
	m_desc[CH_PATH2_ID].pdataItem			= &m_path2_ID;

	m_desc[CH_STIM2_ID].pdataItem			= &m_stim2_ID;
	m_desc[CH_CONC2_ID].pdataItem			= &m_conc2_ID;
	m_desc[CH_STRAIN_ID].pdataItem			= &m_strain_ID;
	m_desc[CH_SEX_ID].pdataItem				= &m_sex_ID;
	m_desc[CH_REPEAT].pdataItem				= &m_repeat;

	m_desc[CH_REPEAT2].pdataItem			= &m_repeat2;
	m_desc[CH_EXPT_ID].pdataItem			= &m_expt_ID;

	m_desc[CH_ACQDATE].pdataItem			= nullptr; 
	m_desc[CH_FILENAME].pdataItem			= nullptr; 
	m_desc[CH_FILESPK].pdataItem			= nullptr; 
	m_desc[CH_ACQ_COMMENTS].pdataItem		= nullptr;
	m_desc[CH_MORE].pdataItem				= nullptr;
	m_desc[CH_ACQDATE_DAY].pdataItem		= nullptr;
	m_desc[CH_ACQDATE_TIME].pdataItem		= nullptr;
	m_desc[CH_ACQDATE_DAY].date_time_param_single_filter	= static_cast<DATE>(0); 
	m_desc[CH_ACQDATE_TIME].date_time_param_single_filter	= static_cast<DATE>(0);

	// clear fields
	for (int i = 0; i <= m_nFields; i++)
	{
		m_desc[i].b_single_filter = FALSE;
		m_desc[i].l_param_single_filter = 0;
		m_desc[i].b_array_filter = FALSE;
		m_desc[i].pdataItem = nullptr;
		m_desc[i].index = i;
		m_desc[i].pComboBox = nullptr;
		m_desc[i].csColParam.Empty();
		m_desc[i].cs_param_single_filter.Empty();
		m_desc[i].l_param_filter_array.RemoveAll();
		m_desc[i].data_time_array_filter.RemoveAll();
		m_desc[i].cs_array_filter.RemoveAll();
		m_desc[i].liArray.RemoveAll();
		m_desc[i].tiArray.RemoveAll();
		m_desc[i].csElementsArray.RemoveAll();
	}

	m_nFields = 29;
	m_nParams = 18;
	m_nDefaultType = dbOpenDynaset;
}

CdbTableMain::~CdbTableMain()
{
	DeleteDateArray();
}

boolean CdbTableMain::OpenTable(int nOpenType, LPCTSTR lpszSQL, int nOptions)
{
	boolean flag = true;
	try
	{
		Open(nOpenType, lpszSQL, nOptions);
	}
	catch (CDaoException* e)
	{
		AfxMessageBox(_T("Cancel import: ") + e->m_pErrorInfo->m_strDescription);
		e->Delete();
		flag = false;
	}
	return flag;
}

CString CdbTableMain::GetDefaultDBName()
{
	auto cs = m_defaultName;
	if (m_pDatabase->m_pDAODatabase != nullptr)
		cs = m_pDatabase->GetName();

	return cs;
}

CString CdbTableMain::GetDefaultSQL()
{
	return m_csdefaultSQL;
}

void CdbTableMain::DoFieldExchange(CDaoFieldExchange* pFX)
{
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Text(pFX, m_desc[CH_FILENAME].dfx_name_with_brackets, m_Filedat);
	DFX_Text(pFX, m_desc[CH_FILESPK].dfx_name_with_brackets, m_Filespk);
	DFX_Text(pFX, m_desc[CH_ACQ_COMMENTS].dfx_name_with_brackets, m_acq_comment);
	DFX_Text(pFX, m_desc[CH_MORE].dfx_name_with_brackets, m_more);
	DFX_Long(pFX, m_desc[CH_ID].dfx_name_with_brackets, m_ID);
	DFX_Long(pFX, m_desc[CH_DATALEN].dfx_name_with_brackets, m_datalen);
	DFX_Long(pFX, m_desc[CH_NSPIKES].dfx_name_with_brackets, m_nspikes);
	DFX_Long(pFX, m_desc[CH_NSPIKECLASSES].dfx_name_with_brackets, m_nspikeclasses);
	DFX_Long(pFX, m_desc[CH_PATH_ID].dfx_name_with_brackets, m_path_ID);
	DFX_Long(pFX, m_desc[CH_PATH2_ID].dfx_name_with_brackets, m_path2_ID);
	DFX_DateTime(pFX, m_desc[CH_ACQDATE].dfx_name_with_brackets, m_table_acq_date);

	int i = CH_IDINSECT;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_IDinsect);
	i = CH_IDSENSILLUM;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_IDsensillum);
	i = CH_LOCATION_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_location_ID);
	i = CH_STIM_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_stim_ID);
	i = CH_CONC_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_conc_ID);
	i = CH_STIM2_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_stim2_ID);
	i = CH_CONC2_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_conc2_ID);
	i = CH_OPERATOR_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_operator_ID);
	i = CH_INSECT_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_insect_ID);
	i = CH_SENSILLUM_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_sensillum_ID);
	i = CH_STRAIN_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_strain_ID);
	i = CH_SEX_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_sex_ID);
	i = CH_FLAG;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_flag);
	i = CH_REPEAT;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_repeat);
	i = CH_REPEAT2;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_repeat2);
	i = CH_ACQDATE_DAY;
	DFX_DateTime(pFX, m_desc[CH_ACQDATE_DAY].dfx_name_with_brackets, m_acqdate_day);
	i = CH_ACQDATE_TIME;
	DFX_DateTime(pFX, m_desc[i].dfx_name_with_brackets, m_acqdate_time);
	i = CH_EXPT_ID;
	DFX_Long(pFX, m_desc[i].dfx_name_with_brackets, m_expt_ID);
	
	pFX->SetFieldType(CDaoFieldExchange::param);
	i = CH_IDINSECT;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); // 1
	i = CH_IDSENSILLUM;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_LOCATION_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_STIM_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_CONC_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); // 5
	i = CH_STIM2_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_CONC2_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_OPERATOR_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_INSECT_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_SENSILLUM_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); // 10
	i = CH_STRAIN_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_SEX_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_FLAG;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_REPEAT;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); //
	i = CH_REPEAT2;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); // 15
	i = CH_ACQDATE_DAY;
	DFX_DateTime(pFX, m_desc[i].csColParam, m_desc[i].date_time_param_single_filter); // 16
	i = CH_ACQDATE_TIME;
	DFX_DateTime(pFX, m_desc[i].csColParam, m_desc[i].date_time_param_single_filter); // 17
	i = CH_EXPT_ID;
	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].l_param_single_filter); // 18
}

/////////////////////////////////////////////////////////////////////////////
// CdbTableMain diagnostics

#ifdef _DEBUG
void CdbTableMain::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CdbTableMain::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG

// TODO: remove
BOOL CdbTableMain::SetLongValue(long iID, CString cscolname)
{
	try
	{
		Edit();
		SetFieldValue(cscolname, COleVariant(iID, VT_I4));
		Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 24);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CdbTableMain::SetValueNull(const CString cscolname)
{
	try
	{
		Edit();
		SetFieldValueNull(cscolname);
		Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 24);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}

void CdbTableMain::GetAcqdateArray(CPtrArray* pacqdate)
{
	const auto nrecords = GetNRecords();
	if (0 == nrecords)
		return;

	pacqdate->SetSize(nrecords);
	MoveFirst();
	while (!IsEOF())
	{
		const auto ptime = new COleDateTime;
		*ptime = m_table_acq_date;
		pacqdate->Add(ptime);

		MoveNext();
	}
}

BOOL CdbTableMain::CheckIfAcqDateTimeIsUnique(COleDateTime* ptimeNewVal)
{
	if (IsBOF() && IsEOF())
		return TRUE;

	MoveFirst();
	while (!IsEOF())
	{
		if (*ptimeNewVal == m_table_acq_date)
			return FALSE;
		MoveNext();
	}
	return TRUE;
}

void CdbTableMain::GetMaxIDs()
{
	max_insectID = -1;
	max_sensillumID = -1;
	max_ID = -1;
	const auto nrecords = GetNRecords();
	if (0 == nrecords)
		return;

	MoveFirst();
	while (!IsEOF())
	{
		if (m_IDinsect > max_insectID)
			max_insectID = m_IDinsect;

		if (m_ID > max_ID)
			max_ID = m_ID;

		if (m_IDsensillum > max_sensillumID)
			max_sensillumID = m_IDsensillum;

		MoveNext();
	}
}

BOOL CdbTableMain::FindIDinColumn(long iID, int icolumn)
{
	CString cs; // to construct insect and sensillum number (for example)
	CString str; // to store FindFirst filter
	const auto cscolhead = m_desc[icolumn].header_name;
	str.Format(_T("%s=%li"), (LPCTSTR)cscolhead, iID);
	auto flag = FALSE;

	try
	{
		const auto var_current_pos = GetBookmark();
		MoveFirst();
		flag = FindFirst(str);
		SetBookmark(var_current_pos);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 19);
		e->Delete();
	}

	return flag;
}

int CdbTableMain::GetColumnIndex(CString csName)
{
	CDaoFieldInfo fieldInfo;
	GetFieldInfo(csName, fieldInfo, AFX_DAO_SECONDARY_INFO);

	return fieldInfo.m_nOrdinalPosition;
}

void CdbTableMain::RefreshQuery()
{
	if (CanRestart())
		Requery();
	else
	{
		Close();
		Open(dbOpenDynaset);
	}
}

void CdbTableMain::BuildFilters()
{
	m_strFilter.Empty();
	for (auto ifield = 0; ifield < m_nFields; ifield++)
	{
		if (m_desc[ifield].b_array_filter == TRUE)
		{
			if (!m_strFilter.IsEmpty())
				m_strFilter += _T(" AND ");
			CString cs;
			cs.Format(_T("%s IN ("), (LPCTSTR)m_desc[ifield].dfx_name_with_brackets);
			m_strFilter += cs;
			switch (m_desc[ifield].data_code_number)
			{
			case FIELD_IND_TEXT:
			case FIELD_IND_FILEPATH:
			case FIELD_LONG:
				{
					int i = 0;
					cs.Format(_T("%i"), m_desc[ifield].l_param_filter_array.GetAt(i));
					m_strFilter += cs;
					for (i = 1; i < m_desc[ifield].l_param_filter_array.GetSize(); i++)
					{
						cs.Format(_T(", %i"), m_desc[ifield].l_param_filter_array.GetAt(i));
						m_strFilter += cs;
					}
				}
				break;

			case FIELD_DATE_YMD:
				{
					auto i = 0;
					auto o_time = m_desc[ifield].data_time_array_filter.GetAt(i);
					cs = o_time.Format(_T(" #%m/%d/%y#"));
					m_strFilter += cs;
					for (i = 1; i < m_desc[ifield].data_time_array_filter.GetSize(); i++)
					{
						o_time = m_desc[ifield].data_time_array_filter.GetAt(i);
						cs = o_time.Format(_T(", #%m/%d/%y#"));
						m_strFilter += cs;
					}
				}
				break;

			default:
				ASSERT(false);
				break;
			}
			m_strFilter += _T(")");
		}
		else if (m_desc[ifield].b_single_filter)
		{
			if (!m_strFilter.IsEmpty())
				m_strFilter += " AND ";
			m_strFilter += m_desc[ifield].csEQUcondition;
		}
	}
	m_bFilterON = !m_strFilter.IsEmpty();
}

void CdbTableMain::ClearFilters()
{
	for (auto i = 0; i < m_nFields; i++)
		m_desc[i].b_single_filter = FALSE;
	m_strFilter.Empty();
	m_bFilterON = FALSE;
}

void CdbTableMain::SetDataLen(long datalen)
{
	try
	{
		Edit();
		m_datalen = datalen;
		Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 12);
		e->Delete();
	}
}

long CdbTableMain::GetNRecords()
{
	CWaitCursor wait;
	long nrecords = 0;
	if (IsBOF() && IsEOF())
		return nrecords;

	ASSERT(CanBookmark());
	try
	{
		const auto ol = GetBookmark();
		MoveLast();
		MoveFirst();
		nrecords = GetRecordCount();
		SetBookmark(ol);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 9);
		e->Delete();
	}

	return nrecords;
}

void CdbTableMain::AddDaytoDateArray(COleDateTime& o_time)
{
	auto b_flag = FALSE;
	COleDateTime day_time;
	day_time.SetDateTime(o_time.GetYear(), o_time.GetMonth(), o_time.GetDay(), 0, 0, 0);

	for (auto i = 0; i < m_desc[CH_ACQDATE_DAY].tiArray.GetSize(); i++)
	{
		auto io_time = m_desc[CH_ACQDATE_DAY].tiArray.GetAt(i);
		// element already exist? -- assume this is the most frequent case
		if (day_time == io_time)
		{
			b_flag = TRUE;
			break;
		}
		// insert element before current?
		if (day_time < io_time)
		{
			m_desc[CH_ACQDATE_DAY].tiArray.InsertAt(i, day_time);
			b_flag = TRUE;
			break;
		}
		// element greater than current, loop to next
	}

	// no element found, add one at the end of the array
	if (!b_flag)
	{
		m_desc[CH_ACQDATE_DAY].tiArray.Add(day_time);
	}
}

// Add element only if new and insert it so that the array is sorted (low to high value)
void CdbTableMain::AddtoliArray(int icol)
{
	COleVariant var_value;
	GetFieldValue(m_desc[icol].header_name, var_value);
	int l_val = var_value.lVal;
	if (var_value.vt == VT_NULL)
		l_val = 0;

	auto pli_array = &m_desc[icol].liArray;
	auto b_flag = FALSE;
	// value is greater than current value -> loop forwards

	for (auto i = 0; i <= pli_array->GetUpperBound(); i++)
	{
		// element already exist? -- assume this is the most frequent case
		const auto ic_id = pli_array->GetAt(i);
		if (l_val == ic_id)
		{
			b_flag = TRUE;
			break;
		}
		// insert element before current?
		if (l_val < ic_id)
		{
			pli_array->InsertAt(i, l_val);
			b_flag = TRUE;
			break;
		}
		// element greater than current, loop to next
	}
	// no element found, add one at the end of the array
	if (!b_flag)
		pli_array->Add(l_val);
}

void CdbTableMain::AddtoIDArray(CUIntArray* puiIDArray, long iID)
{
	auto b_flag = FALSE;
	// value is greater than current value -> loop forwards
	const auto ui_id = static_cast<UINT>(iID);

	for (auto i = 0; i <= puiIDArray->GetUpperBound(); i++)
	{
		// element already exist? -- assume this is the most frequent case
		const auto uc_id = puiIDArray->GetAt(i);
		if (ui_id == uc_id)
		{
			b_flag = TRUE;
			break;
		}
		// insert element before current?
		if (ui_id < uc_id)
		{
			puiIDArray->InsertAt(i, ui_id);
			b_flag = TRUE;
			break;
		}
		// element greater than current, loop to next
	}
	// no element found, add one at the end of the array
	if (!b_flag)
		puiIDArray->Add(ui_id);
}

void CdbTableMain::AddCurrentRecordtoIDArrays()
{
	COleVariant var_value;

	AddtoliArray(CH_IDINSECT);
	AddtoliArray(CH_IDSENSILLUM);
	AddtoliArray(CH_REPEAT);
	AddtoliArray(CH_REPEAT2);
	AddtoliArray(CH_FLAG);

	// look for date
	GetFieldValue(m_desc[CH_ACQDATE_DAY].header_name, var_value);
	if (var_value.vt == VT_NULL)
	{
		// transfer date from field 1 and copy date and time in 2 separate columns
		GetFieldValue(m_desc[CH_ACQDATE].header_name, var_value);
		if (var_value.vt != VT_NULL)
		{
			COleDateTime o_time = var_value.date;
			COleDateTime acqdate_day;
			acqdate_day.SetDateTime(o_time.GetYear(), o_time.GetMonth(), o_time.GetDay(), 0, 0, 0);
			const auto acqdate_time = o_time - acqdate_day;

			Edit();
			var_value = acqdate_time;
			SetFieldValue(CH_ACQDATE_TIME, var_value.date);
			SetFieldValue(CH_ACQDATE_DAY, acqdate_day);
			Update();
		}
	}

	if (var_value.vt != VT_NULL)
	{
		COleDateTime o_time = var_value.date;
		AddDaytoDateArray(o_time);
	}
}

void CdbTableMain::DeleteDateArray()
{
	if (m_desc[CH_ACQDATE_DAY].tiArray.GetSize() > 0)
		m_desc[CH_ACQDATE_DAY].tiArray.RemoveAll();
}

// loop over the entire database and save descriptors
void CdbTableMain::BuildAndSortIDArrays()
{
	if (IsBOF())
		return;
	const auto bookmark_current = GetBookmark();

	// ID arrays will be ordered incrementally
	const int nrecords = GetNRecords();
	if (!m_desc[CH_IDINSECT].b_single_filter && !m_desc[CH_IDINSECT].b_array_filter)
		m_desc[CH_IDINSECT].liArray.SetSize(0, nrecords); // resize array to the max possible
	if (!m_desc[CH_IDSENSILLUM].b_single_filter && !m_desc[CH_IDSENSILLUM].b_array_filter)
		m_desc[CH_IDSENSILLUM].liArray.SetSize(0, nrecords); // size of the array not known beforehand
	if (!m_desc[CH_REPEAT].b_single_filter && !m_desc[CH_REPEAT].b_array_filter)
		m_desc[CH_REPEAT].liArray.SetSize(0, nrecords);
	if (!m_desc[CH_REPEAT2].b_single_filter && !m_desc[CH_REPEAT2].b_array_filter)
		m_desc[CH_REPEAT2].liArray.SetSize(0, nrecords);
	if (!m_desc[CH_FLAG].b_single_filter && !m_desc[CH_FLAG].b_array_filter)
		m_desc[CH_FLAG].liArray.SetSize(0, nrecords);
	if (nrecords == 0)
		return;

	// TODO check if date is selected, may be we should prevent deleting this array
	DeleteDateArray();

	// go to first record and browse the table
	MoveFirst();
	auto i = 0;
	while (!IsEOF())
	{
		AddCurrentRecordtoIDArrays();
		MoveNext();
		i++;
	}

	// resize arrays
	m_desc[CH_IDINSECT].liArray.FreeExtra();
	m_desc[CH_IDSENSILLUM].liArray.FreeExtra();
	m_desc[CH_REPEAT].liArray.FreeExtra();
	m_desc[CH_REPEAT2].liArray.FreeExtra();
	m_desc[CH_FLAG].liArray.FreeExtra();
	// restore current record
	SetBookmark(bookmark_current);
}

// loop over the entire database and copy field m_path into m_path2
void CdbTableMain::CopyPathToPath2()
{
	if (IsBOF())
		return;
	const auto bookmark_current = GetBookmark();

	// go to first record and browse the table
	MoveFirst();
	auto i = 0;
	while (!IsEOF())
	{
		Edit();
		m_path2_ID = m_path_ID;
		Update();
		MoveNext();
		i++;
	}

	// restore current record
	SetBookmark(bookmark_current);
}
