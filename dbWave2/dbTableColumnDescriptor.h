#pragma once
#include "dbTableAssociated.h"
#include "dbTableMain.h"

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

