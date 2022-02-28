#pragma once

constexpr auto CH_ID = 0;
constexpr auto CH_ACQDATE = 1;
constexpr auto CH_FILENAME = 2;
constexpr auto CH_FILESPK = 3;
constexpr auto CH_ACQ_COMMENTS = 4;

constexpr auto CH_MORE = 5;
constexpr auto CH_IDINSECT = 6;
constexpr auto CH_IDSENSILLUM = 7;
constexpr auto CH_DATALEN = 8;
constexpr auto CH_NSPIKES = 9;

constexpr auto CH_NSPIKECLASSES = 10;
constexpr auto CH_FLAG = 11;
constexpr auto CH_INSECT_ID = 12;
constexpr auto CH_SENSILLUM_ID = 13;
constexpr auto CH_OPERATOR_ID = 14;

constexpr auto CH_STIM_ID = 15;
constexpr auto CH_CONC_ID = 16;
constexpr auto CH_LOCATION_ID = 17;
constexpr auto CH_PATH_ID = 18;
constexpr auto CH_PATH2_ID = 19;

constexpr auto CH_STIM2_ID = 20;
constexpr auto CH_CONC2_ID = 21;
constexpr auto CH_STRAIN_ID = 22;
constexpr auto CH_SEX_ID = 23;
constexpr auto CH_REPEAT = 24;

constexpr auto CH_REPEAT2 = 25;
constexpr auto CH_ACQDATE_DAY = 26;
constexpr auto CH_ACQDATE_TIME = 27;
constexpr auto CH_EXPT_ID = 28;

constexpr auto FIELD_IND_TEXT = 1;
constexpr auto FIELD_LONG = 2;
constexpr auto FIELD_TEXT = 3;
constexpr auto FIELD_DATE = 4;
constexpr auto FIELD_IND_FILEPATH = 5;
constexpr auto FIELD_DATE_HMS = 6;
constexpr auto FIELD_DATE_YMD = 7;

constexpr int NCOLUMNS = 30;

#include "dbWaveDatabaseIndexTable.h"

// Used for GetItemDescriptors calls to identify database items
using DB_ITEMDESC = struct db_item_descriptor
{
	// -----------------descriptor and DFX parameters
	int		index = 0;					// column number in the table
	CString header_name;				// x - "name" of the column in the main table
	CString dfx_name_with_brackets;		// x - "[name]" used by DFX exchange mechanism
	CString csColParam;					// x - "nameParam"
	CString csEQUcondition;				// x - "name = nameParam"
	CString associated_table_name;

	// ------------------filtering
	BOOL	bFilter1 = false;			// TRUE: only 1 value  selected - stored in lfilterParam1
	long	lfilterParam1 = 0;
	COleDateTime otfilterParam1;
	CString csfilterParam1 = nullptr;

	BOOL	bFilter2 = false;			// TRUE: selected values - stored in lfilterParam2 ;
	CArray<long, long> lfilterParam2;
	CArray<COleDateTime, COleDateTime> otfilterParam2;
	CStringArray csfilterParam2;

	// ------------------value of current record parameter
	long lVal = 0;
	CString csVal;
	COleDateTime oVal;
	BOOL bIsFieldNull = false;

	// helpers
	int typeLocal = FIELD_LONG;			// field type (FIELD_IND_TEXT, FIELD_LONG, FIELD_TEXT, FIELD_DATE, FIELD_IND_FILEPATH)
	CArray<long, long> liArray;			// different elements of this field (if long or indirection)
	CArray<COleDateTime, COleDateTime> tiArray; // array storing the different elements of this field (if date(day))
	CStringArray csElementsArray;		// string version of the list different elements (to compare with filter - either csfilterParam1, or csFilterParam2)

	// --------------------temporary pointers
	long* pdataItem = nullptr;			// x - address of the field in the main table
	CdbIndexTable* plinkedSet = nullptr;// x - address of the linked table (indirection)
	CComboBox* pComboBox = nullptr;		// address of combobox associated with the item
};


#pragma warning(disable : 4995)

class CdbMainTable : public CDaoRecordset
{
public:
	CdbMainTable(CDaoDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CdbMainTable)

	// Field/Param Data - these parameters are equivalent to column headers in a table, where each row is a record

	long m_ID = 0;					// 1
	CString m_Filedat = _T("");		// 2
	CString m_Filespk = _T("");		// 3
	COleDateTime m_acq_date = static_cast<DATE>(0); // 4
	CString m_acq_comment = _T(""); // 5
	long m_IDinsect = 0;			// 6
	long m_IDsensillum = 0;			// 7
	CString m_more = _T("");		// 8
	long m_insect_ID = 0;			// 9
	long m_location_ID = 0;			// 10
	long m_operator_ID = 0;			// 11
	long m_sensillum_ID = 0;		// 12
	long m_path_ID = 0;				// 13
	long m_path2_ID = 0;			// 14 xxxxxx suppress this parameter, leave a dummy instead ?
	long m_datalen = 0;				// 15
	long m_nspikes = 0;				// 16
	long m_nspikeclasses = 0;		// 17
	long m_stim_ID = 0;				// 18
	long m_conc_ID = 0;				// 19
	long m_stim2_ID = 0;			// 20
	long m_conc2_ID = 0;			// 21
	long m_flag = 0;				// 22
	long m_strain_ID = 0;			// 23
	long m_sex_ID = 0;				// 24
	long m_repeat = 0;				// 25
	long m_repeat2 = 0;				// 26
	COleDateTime m_acqdate_day = static_cast<DATE>(0); // 27
	COleDateTime m_acqdate_time = static_cast<DATE>(0); // 28 xxxxxxx suppress this parameter, leave dummy ?
	long m_expt_ID = 0;				// 29

	// filter parameters
	BOOL m_bFilterON = false;
	DB_ITEMDESC m_desc[NCOLUMNS];
	CString m_csdefaultSQL = _T("[table]");

	// temp values
	long max_insectID = 0;
	long max_sensillumID = 0;
	long max_ID = 0;
	CString m_defaultName = _T("");

public:
	void BuildAndSortIDArrays();
	void CopyPathToPath2();
	void AddCurrentRecordtoIDArrays();
	boolean OpenTable(int nOpenType = AFX_DAO_USE_DEFAULT_TYPE, LPCTSTR lpszSQL = nullptr, int nOptions = 0);

protected:
	void AddtoliArray(int icol);
	void AddtoIDArray(CUIntArray* pmIDArray, long uiID);
	void AddDaytoDateArray(COleDateTime& o_time);
	void DeleteDateArray();

	// Overrides
public:
	CString GetDefaultDBName() override;
	CString GetDefaultSQL() override; // default SQL for Recordset
	void DoFieldExchange(CDaoFieldExchange* pFX) override; // RFX support

	// Implementation
	~CdbMainTable() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
	long GetNRecords();
	BOOL SetLongValue(long iID, CString cscolname);
	BOOL SetValueNull(CString cscolname);
	void GetAcqdateArray(CPtrArray* pacqdate);
	BOOL CheckIfAcqDateTimeIsUnique(COleDateTime* ptime);
	void GetMaxIDs();
	BOOL FindIDinColumn(long iID, int icolumn);
	int GetColumnIndex(CString csName);

	void BuildFilters();
	void ClearFilters();
	void RefreshQuery();
	void SetDataLen(long datalen);

	void SetFilterSingleItem(DB_ITEMDESC* pdesc)
	{
		m_desc[pdesc->index].bFilter1 = pdesc->bFilter1;
		m_desc[pdesc->index].lfilterParam1 = pdesc->lfilterParam1;
	}
};
