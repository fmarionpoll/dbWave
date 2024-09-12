#pragma once
#include "dbTableAssociated.h"


constexpr auto FIELD_IND_TEXT = 1;
constexpr auto FIELD_LONG = 2;
constexpr auto FIELD_TEXT = 3;
constexpr auto FIELD_DATE = 4;
constexpr auto FIELD_IND_FILEPATH = 5;
constexpr auto FIELD_DATE_HMS = 6;
constexpr auto FIELD_DATE_YMD = 7;

// Used for GetItemDescriptors calls to identify database items
using DB_ITEMDESC = struct db_table_column_descriptor
{
	// -----------------descriptor and DFX parameters
	int		index  {0};								// column number in the table
	CString header_name;							// x - "name" of the column in the main table
	CString dfx_name_with_brackets;					// x - "[name]" used by DFX exchange mechanism
	CString cs_col_param;							// x - "nameParam"
	CString cs_equ_condition;						// x - "name  nameParam"
	CString associated_table_name;

	// ------------------filtering
	BOOL	b_single_filter  {false};				// TRUE: only 1 value  selected - stored in l_param_single_filter
	long	l_param_single_filter  {0};
	COleDateTime date_time_param_single_filter;
	CString cs_param_single_filter  {};

	BOOL	b_array_filter  {false};				// TRUE: selected values - stored in l_param_filter_array ;
	CArray<long, long> l_param_filter_array;
	CArray<COleDateTime, COleDateTime> data_time_array_filter;
	CStringArray cs_array_filter;

	// ------------------value of current record parameter
	long l_val  {0};
	CString cs_val;
	COleDateTime o_val;
	BOOL b_is_field_null  {false};

	// helpers
	int data_code_number  {FIELD_LONG};				// field type (FIELD_IND_TEXT, FIELD_LONG, FIELD_TEXT, FIELD_DATE, FIELD_IND_FILEPATH)
	CArray<long, long> li_array;					// different elements of this field (if long or indirection)
	CArray<COleDateTime, COleDateTime> ti_array;	// array storing the different elements of this field (if date(day))
	CStringArray cs_elements_array;					// string version of the list different elements (to compare with filter - either cs_param_single_filter, or csFilterParam2)

	// --------------------temporary pointers
	long* pdata_item  {nullptr};					// x - address of the field in the main table
	CdbTableAssociated* p_linked_set  {nullptr};	// x - address of the linked table (indirection)
	CComboBox* p_combo_box  {nullptr};				// address of combobox associated with the item
};

