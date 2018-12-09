// dbWaveSet.cpp : implementation of the CdbMainTable class
//

#include "StdAfx.h"
#include "dbWave.h"
#include "dbMainTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CdbMainTable implementation

IMPLEMENT_DYNAMIC(CdbMainTable, CDaoRecordset)

CdbMainTable::CdbMainTable(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	m_ID = 0;						// 1
	m_Filedat = _T("");				//
	m_Filespk = _T("");				//
	m_acq_date = (DATE)0;			//
	m_acq_comment = _T("");			// 5
	m_IDinsect = 0;					//
	m_IDsensillum = 0;				//
	m_more = _T("");				//
	m_insect_ID = 0;				//
	m_location_ID = 0;				// 10
	m_operator_ID = 0;				//
	m_sensillum_ID = 0;				//
	m_path_ID = 0;					//
	m_path2_ID = 0;					//
	m_datalen=0;					// 15
	m_nspikes=0;					//
	m_nspikeclasses=0;				//
	m_stim_ID = 0;					//
	m_conc_ID = 0;					//
	m_stim2_ID = 0;					// 20
	m_conc2_ID = 0;					//
	m_sex_ID = 0;					//
	m_strain_ID = 0;				//
	m_flag = 0;						// 24
	m_repeat = 0;					// 25
	m_repeat2 = 0;					// 26
	m_acqdate_day  =(DATE)0;		// 27
	m_acqdate_time =(DATE)0;		// 28
	m_expt_ID = 0;					// 29
	m_nFields = 29;

	m_desc[CH_ID].pdataItem				= &m_ID; 
	m_desc[CH_ACQDATE].pdataItem		= nullptr; //&m_acq_date;
	m_desc[CH_FILENAME].pdataItem		= nullptr; //&m_Filedat;
	m_desc[CH_FILESPK].pdataItem		= nullptr; //&m_Filespk;
	m_desc[CH_ACQ_COMMENTS].pdataItem	= nullptr; //&m_acq_comment;
	m_desc[CH_MORE].pdataItem			= nullptr; //&m_more;
	m_desc[CH_IDINSECT].pdataItem		= &m_IDinsect;
	m_desc[CH_IDSENSILLUM].pdataItem	= &m_IDsensillum;
	m_desc[CH_DATALEN].pdataItem		= &m_datalen;
	m_desc[CH_NSPIKES].pdataItem		= &m_nspikes;
	m_desc[CH_NSPIKECLASSES].pdataItem	= &m_nspikeclasses;
	m_desc[CH_FLAG].pdataItem			= &m_flag;
	m_desc[CH_INSECT_ID].pdataItem		= &m_insect_ID;
	m_desc[CH_SENSILLUM_ID].pdataItem	= &m_sensillum_ID;
	m_desc[CH_OPERATOR_ID].pdataItem	= &m_operator_ID;
	m_desc[CH_STIM_ID].pdataItem		= &m_stim_ID;
	m_desc[CH_CONC_ID].pdataItem		= &m_conc_ID;
	m_desc[CH_LOCATION_ID].pdataItem	= &m_location_ID;
	m_desc[CH_PATH_ID].pdataItem		= &m_path_ID;
	m_desc[CH_PATH2_ID].pdataItem		= &m_path2_ID;
	m_desc[CH_STIM2_ID].pdataItem		= &m_stim2_ID;
	m_desc[CH_CONC2_ID].pdataItem		= &m_conc2_ID;
	m_desc[CH_STRAIN_ID].pdataItem		= &m_strain_ID;
	m_desc[CH_SEX_ID].pdataItem			= &m_sex_ID;
	m_desc[CH_REPEAT].pdataItem			= &m_repeat;
	m_desc[CH_REPEAT2].pdataItem		= &m_repeat2;
	m_desc[CH_ACQDATE_DAY].pdataItem	= nullptr;
	m_desc[CH_ACQDATE_TIME].pdataItem	= nullptr;
	m_desc[CH_EXPT_ID].pdataItem		= &m_expt_ID;

	m_nDefaultType = dbOpenDynaset;
	m_desc[CH_ACQDATE_DAY].otfilterParam1 =DATE(0);		// 16
	m_desc[CH_ACQDATE_TIME].otfilterParam1 = DATE(0);	 // 17
	m_nParams = 18;	

	m_csdefaultSQL = _T("[table]");

	// clear fields
	for (int i=0; i<= m_nFields; i++)
	{
		m_desc[i].bFilter1 = FALSE;
		m_desc[i].csColParam.Empty();
		m_desc[i].lfilterParam1 = 0;
		m_desc[i].csfilterParam1.Empty();
		m_desc[i].bFilter2 = FALSE;
		m_desc[i].lfilterParam2.RemoveAll();
		m_desc[i].otfilterParam2.RemoveAll();
		m_desc[i].csfilterParam2.RemoveAll();
		m_desc[i].pdataItem = nullptr;
		m_desc[i].icol = i;
		m_desc[i].liArray.RemoveAll();
		m_desc[i].tiArray.RemoveAll();
		m_desc[i].csElementsArray.RemoveAll();
		m_desc[i].pComboBox = nullptr;
	}

	m_defaultName.Empty();
	m_bFilterON = false;
}

CdbMainTable::~CdbMainTable()
{
	DeleteDateArray();
}

CString CdbMainTable::GetDefaultDBName()
{
	CString cs = m_defaultName;
	if (m_pDatabase->m_pDAODatabase != nullptr)
		cs = m_pDatabase->GetName();
	
	return cs;
}

CString CdbMainTable::GetDefaultSQL()
{
	return m_csdefaultSQL;
}

void CdbMainTable::DoFieldExchange(CDaoFieldExchange* pFX)
{
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Text(pFX,	m_desc[CH_FILENAME].csColNamewithBrackets,		m_Filedat);
	DFX_Text(pFX,	m_desc[CH_FILESPK].csColNamewithBrackets,		m_Filespk);
	DFX_Text(pFX,	m_desc[CH_ACQ_COMMENTS].csColNamewithBrackets,	m_acq_comment);
	DFX_Text(pFX,	m_desc[CH_MORE].csColNamewithBrackets,			m_more);
	DFX_Long(pFX,	m_desc[CH_ID].csColNamewithBrackets,			m_ID);
	DFX_Long(pFX,	m_desc[CH_IDINSECT].csColNamewithBrackets,		m_IDinsect);
	DFX_Long(pFX,	m_desc[CH_IDSENSILLUM].csColNamewithBrackets,	m_IDsensillum);
	DFX_Long(pFX,	m_desc[CH_DATALEN].csColNamewithBrackets,		m_datalen);
	DFX_Long(pFX,	m_desc[CH_NSPIKES].csColNamewithBrackets,		m_nspikes);
	DFX_Long(pFX,	m_desc[CH_NSPIKECLASSES].csColNamewithBrackets,	m_nspikeclasses);
	DFX_Long(pFX,	m_desc[CH_FLAG].csColNamewithBrackets,			m_flag);
	DFX_Long(pFX,	m_desc[CH_INSECT_ID].csColNamewithBrackets,		m_insect_ID);
	DFX_Long(pFX,	m_desc[CH_SENSILLUM_ID].csColNamewithBrackets,	m_sensillum_ID);
	DFX_Long(pFX,	m_desc[CH_OPERATOR_ID].csColNamewithBrackets,	m_operator_ID);
	DFX_Long(pFX,	m_desc[CH_STIM_ID].csColNamewithBrackets,		m_stim_ID);
	DFX_Long(pFX,	m_desc[CH_CONC_ID].csColNamewithBrackets,		m_conc_ID);
	DFX_Long(pFX,	m_desc[CH_LOCATION_ID].csColNamewithBrackets,	m_location_ID);
	DFX_Long(pFX,	m_desc[CH_PATH_ID].csColNamewithBrackets,		m_path_ID);
	DFX_Long(pFX,	m_desc[CH_PATH2_ID].csColNamewithBrackets,		m_path2_ID);
	DFX_Long(pFX,	m_desc[CH_STIM2_ID].csColNamewithBrackets,		m_stim2_ID);
	DFX_Long(pFX,	m_desc[CH_CONC2_ID].csColNamewithBrackets,		m_conc2_ID);
	DFX_Long(pFX,	m_desc[CH_STRAIN_ID].csColNamewithBrackets,		m_strain_ID);
	DFX_Long(pFX,	m_desc[CH_SEX_ID].csColNamewithBrackets,		m_sex_ID);
	DFX_Long(pFX,	m_desc[CH_REPEAT].csColNamewithBrackets,		m_repeat);
	DFX_Long(pFX,	m_desc[CH_REPEAT2].csColNamewithBrackets,		m_repeat2);
	DFX_Long(pFX,	m_desc[CH_EXPT_ID].csColNamewithBrackets,		m_expt_ID);
	DFX_DateTime(pFX, m_desc[CH_ACQDATE].csColNamewithBrackets,		m_acq_date);
	DFX_DateTime(pFX, m_desc[CH_ACQDATE_DAY].csColNamewithBrackets,	m_acqdate_day);
	DFX_DateTime(pFX, m_desc[CH_ACQDATE_TIME].csColNamewithBrackets,m_acqdate_time);
	

	pFX->SetFieldType(CDaoFieldExchange::param);
	int i;
	i = CH_IDINSECT;		DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); // 1
	i = CH_IDSENSILLUM;		DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_LOCATION_ID;		DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_STIM_ID;			DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_CONC_ID;			DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); // 5
	i = CH_STIM2_ID;		DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_CONC2_ID;		DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_OPERATOR_ID;		DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_INSECT_ID;		DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_SENSILLUM_ID;	DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); // 10
	i = CH_STRAIN_ID;		DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_SEX_ID;			DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_FLAG;			DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_REPEAT;			DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); //
	i = CH_REPEAT2;			DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1); // 15
	i = CH_ACQDATE_DAY;		DFX_DateTime(pFX, m_desc[i].csColParam, m_desc[i].otfilterParam1);	// 16
	i = CH_ACQDATE_TIME;	DFX_DateTime(pFX, m_desc[i].csColParam, m_desc[i].otfilterParam1);	// 17
	i = CH_EXPT_ID;			DFX_Long(pFX, m_desc[i].csColParam, m_desc[i].lfilterParam1);		// 18
}

/////////////////////////////////////////////////////////////////////////////
// CdbMainTable diagnostics

#ifdef _DEBUG
void CdbMainTable::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CdbMainTable::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG

// TODO: remove
BOOL CdbMainTable::SetLongValue (long iID, CString cscolname)
{
	try {
		Edit();
		SetFieldValue(cscolname, COleVariant(iID, VT_I4));
		Update();
	}
	catch(CDaoException* e) 
		{DisplayDaoException(e, 24); e->Delete(); return FALSE;}
	return TRUE;
}

BOOL CdbMainTable::SetValueNull (CString cscolname)
{
	try {
		Edit();
		SetFieldValueNull(cscolname);
		Update();
	}
	catch(CDaoException* e) 
		{DisplayDaoException(e, 24); e->Delete(); return FALSE;}
	return TRUE;
}

void CdbMainTable::GetAcqdateArray(CPtrArray * pacqdate)
{
	long nrecords = GetNRecords();
	if (0 == nrecords)
		return;

	pacqdate->SetSize(nrecords);
	MoveFirst();
	while (!IsEOF())
	{
		COleDateTime* ptime = new COleDateTime;
		*ptime = m_acq_date;
		pacqdate->Add(ptime);

		MoveNext();
	}
}


BOOL CdbMainTable::CheckIfAcqDateTimeIsUnique(COleDateTime * ptimeNewVal)
{
	if (IsBOF() && IsEOF())
		return TRUE;

	MoveFirst();
	while (!IsEOF())
	{
		if (*ptimeNewVal == m_acq_date)
			return FALSE;
		MoveNext();
	}
	return TRUE;
}


void CdbMainTable::GetMaxIDs()
{
	max_insectID = -1;
	max_sensillumID = -1;
	max_ID = -1;
	long nrecords = GetNRecords();
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


BOOL CdbMainTable::FindIDinColumn(long iID, int icolumn)
{
	CString cs;		// to construct insect and sensillum number (for example)
	CString str;	// to store FindFirst filter	
	CString cscolhead = m_desc[icolumn].csColName;
	str.Format(_T("%s=%li"), (LPCTSTR)cscolhead, iID);
	BOOL flag = FALSE;

	try {
		COleVariant varCurrentPos = GetBookmark();
		MoveFirst();
		flag = FindFirst(str);
		SetBookmark(varCurrentPos);
	}
	catch (CDaoException* e) {
		DisplayDaoException(e, 19);
		e->Delete();
	}

	return flag;
}

int CdbMainTable::GetColumnIndex(CString csName)
{
	CDaoFieldInfo fieldInfo;
	GetFieldInfo(csName, fieldInfo, AFX_DAO_SECONDARY_INFO);
	
	return fieldInfo.m_nOrdinalPosition;
}


void CdbMainTable::RefreshQuery()
{
	if (CanRestart())
		Requery();
	else
	{
		Close();
		Open(dbOpenDynaset); 
	}
}

void CdbMainTable::BuildFilters()
{
	m_strFilter.Empty();
	for (int ifield =0; ifield < m_nFields; ifield++)
	{
		if (m_desc[ifield].bFilter2 == TRUE)
		{
			if (!m_strFilter.IsEmpty())
				m_strFilter += _T(" AND ");
			CString cs;
			cs.Format(_T("%s IN ("), (LPCTSTR) m_desc[ifield].csColNamewithBrackets);
			m_strFilter += cs;
			switch (m_desc[ifield].typeLocal)
			{
			case FIELD_IND_TEXT:
			case FIELD_IND_FILEPATH:
			case FIELD_LONG:
				{
					int i = 0;
					cs.Format(_T("%i"), m_desc[ifield].lfilterParam2.GetAt(i));
					m_strFilter += cs;
					for (i = 1; i < m_desc[ifield].lfilterParam2.GetSize(); i++)
					{
						cs.Format(_T(", %i"), m_desc[ifield].lfilterParam2.GetAt(i));
						m_strFilter += cs;
					}
				}
				break;

			case FIELD_DATE_YMD:
				{
					int i = 0;
					COleDateTime oTime = m_desc[ifield].otfilterParam2.GetAt(i);
					cs = oTime.Format(_T(" #%m/%d/%y#"));
					m_strFilter += cs;
					for (i = 1; i < m_desc[ifield].otfilterParam2.GetSize(); i++)
					{
						oTime = m_desc[ifield].otfilterParam2.GetAt(i);
						cs = oTime.Format(_T(", #%m/%d/%y#"));
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
		else if (m_desc[ifield].bFilter1)
		{
			if(!m_strFilter.IsEmpty())
				m_strFilter += " AND ";
			m_strFilter += m_desc[ifield].csEQUcondition;
		}
	}
	m_bFilterON = !m_strFilter.IsEmpty();
}

void CdbMainTable::ClearFilters()
{
	for (int i = 0; i < m_nFields; i++)
		m_desc[i].bFilter1 = FALSE;
	m_strFilter.Empty();
	m_bFilterON =FALSE;
}

void CdbMainTable::SetDataLen(long datalen)
{
	try 
	{
		Edit();
		m_datalen = datalen;
		Update();
	}
	catch(CDaoException* e) {DisplayDaoException(e, 12); e->Delete();}
}

long CdbMainTable::GetNRecords()
{
	CWaitCursor wait;
	long nrecords = 0;
	if (IsBOF() && IsEOF())
		return nrecords;

	ASSERT(CanBookmark());
	COleVariant ol;
	try 
	{
		ol = GetBookmark();
		MoveLast();
		MoveFirst();
		nrecords = GetRecordCount( );
		SetBookmark(ol);
	}
	catch (CDaoException* e) { DisplayDaoException(e, 9); e->Delete(); }
	
	return nrecords;
}

void CdbMainTable::AddDaytoDateArray(COleDateTime &oTime)
{
	BOOL bFlag = FALSE;
	COleDateTime dayTime;
	COleDateTime ioTime;
	dayTime.SetDateTime(oTime.GetYear(), oTime.GetMonth(), oTime.GetDay(), 0, 0, 0);

	for (int i = 0; i < m_desc[CH_ACQDATE_DAY].tiArray.GetSize(); i++)
	{
		ioTime = m_desc[CH_ACQDATE_DAY].tiArray.GetAt(i);
		// element already exist? -- assume this is the most frequent case
		if (dayTime == ioTime)
		{
			bFlag = TRUE;
			break;
		}
		// insert element before current?
		if (dayTime < ioTime)
		{
			m_desc[CH_ACQDATE_DAY].tiArray.InsertAt(i, dayTime);
			bFlag = TRUE;
			break;
		}
		// element greater than current, loop to next
	}

	// no element found, add one at the end of the array
	if (!bFlag)
	{
		m_desc[CH_ACQDATE_DAY].tiArray.Add(dayTime);
	}
}

// Add element only if new and insert it so that the array is sorted (low to high value)
void CdbMainTable::AddtoliArray(int icol)
{
	COleVariant varValue;
	GetFieldValue(m_desc[icol].csColName, varValue);
	int lVal = varValue.lVal;
	if (varValue.vt == VT_NULL)
		lVal = 0;

	CArray<long, long>* pliArray = &m_desc[icol].liArray;
	BOOL bFlag = FALSE;
	// val is greater than current val -> loop forwards

	for (int i = 0; i <= pliArray->GetUpperBound(); i++)
	{
		// element already exist? -- assume this is the most frequent case
		long icID = pliArray->GetAt(i);
		if (lVal == icID)
		{
			bFlag = TRUE;
			break;
		}
		// insert element before current?
		if (lVal < icID)
		{
			pliArray->InsertAt(i, lVal);
			bFlag = TRUE;
			break;
		}
		// element greater than current, loop to next
	}
	// no element found, add one at the end of the array
	if (!bFlag)
		pliArray->Add(lVal);
}

void CdbMainTable::AddtoIDArray(CUIntArray* puiIDArray, long iID)
{
	BOOL bFlag = FALSE;			
	// val is greater than current val -> loop forwards
	UINT uiID = (UINT) iID;

	for (int i = 0; i <= puiIDArray->GetUpperBound(); i++)
	{
		// element already exist? -- assume this is the most frequent case
		UINT ucID = puiIDArray->GetAt(i);
		if (uiID == ucID)
		{
			bFlag = TRUE;
			break;
		}
		// insert element before current?
		if (uiID < ucID)
		{
			puiIDArray->InsertAt(i, uiID);
			bFlag = TRUE;
			break;
		}
		// element greater than current, loop to next
	}
	// no element found, add one at the end of the array
	if (!bFlag)
		puiIDArray->Add(uiID);
}

void CdbMainTable::AddCurrentRecordtoIDArrays()
{
	COleVariant varValue;

	AddtoliArray(CH_IDINSECT);
	AddtoliArray(CH_IDSENSILLUM);
	AddtoliArray(CH_REPEAT);
	AddtoliArray(CH_REPEAT2);
	AddtoliArray(CH_FLAG);

	// look for date
	GetFieldValue(m_desc[CH_ACQDATE_DAY].csColName, varValue);
	if (varValue.vt == VT_NULL)
	{
		// transfer date from field 1 and copy date and time in 2 separate columns
		GetFieldValue(m_desc[CH_ACQDATE].csColName, varValue);
		if (varValue.vt != VT_NULL)
		{
			COleDateTime oTime = varValue.date;
			COleDateTime acqdate_day;
			acqdate_day.SetDateTime(oTime.GetYear(), oTime.GetMonth(), oTime.GetDay(), 0, 0, 0);
			COleDateTimeSpan acqdate_time = oTime - acqdate_day;

			Edit();
			varValue = acqdate_time;
			SetFieldValue(CH_ACQDATE_TIME, varValue.date);
			SetFieldValue(CH_ACQDATE_DAY, acqdate_day);
			Update();
		}
	}

	if (varValue.vt != VT_NULL)
	{
		COleDateTime oTime = varValue.date;
		AddDaytoDateArray(oTime);
	}
}

void CdbMainTable::DeleteDateArray()
{
	if (m_desc[CH_ACQDATE_DAY].tiArray.GetSize() > 0)
		m_desc[CH_ACQDATE_DAY].tiArray.RemoveAll();
}

// loop over the entire database and save descriptors
void CdbMainTable::BuildAndSortIDArrays()
{
	if (IsBOF())
		return;
	// save current position into a bookmark	
	COleVariant bookmarkCurrent;
	bookmarkCurrent = GetBookmark();

	// ID arrays will be ordered incrementally
	int nrecords = GetNRecords();
	if (!m_desc[CH_IDINSECT].bFilter1 && !m_desc[CH_IDINSECT].bFilter2)
		m_desc[CH_IDINSECT].liArray.SetSize(0, nrecords);		// resize array to the max possible
	if (!m_desc[CH_IDSENSILLUM].bFilter1 && !m_desc[CH_IDSENSILLUM].bFilter2)
		m_desc[CH_IDSENSILLUM].liArray.SetSize(0, nrecords);	// size of the array not known beforehand
	if (!m_desc[CH_REPEAT].bFilter1 && !m_desc[CH_REPEAT].bFilter2)
		m_desc[CH_REPEAT].liArray.SetSize(0, nrecords);
	if (!m_desc[CH_REPEAT2].bFilter1 && !m_desc[CH_REPEAT2].bFilter2)
		m_desc[CH_REPEAT2].liArray.SetSize(0, nrecords);
	if (!m_desc[CH_FLAG].bFilter1 && !m_desc[CH_FLAG].bFilter2)
		m_desc[CH_FLAG].liArray.SetSize(0, nrecords);
	if (nrecords == 0)
		return;

	// TODO check if date is selected, may be we should prevent deleting this array
	DeleteDateArray();

	// go to first record and browse the table
	MoveFirst();
	int i = 0;
	while(!IsEOF()) 
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
	SetBookmark(bookmarkCurrent);
}

// loop over the entire database and copy field m_math into m_path2
void CdbMainTable::CopyPathToPath2()
{
	if (IsBOF())
		return;
	// save current position into a bookmark	
	COleVariant bookmarkCurrent;
	bookmarkCurrent = GetBookmark();

	// go to first record and browse the table
	MoveFirst();
	int i = 0;
	while (!IsEOF())
	{
		Edit();
		m_path2_ID = m_path_ID;
		Update();
		MoveNext();
		i++;
	}

	// restore current record
	SetBookmark(bookmarkCurrent);
}


		
