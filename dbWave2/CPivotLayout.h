// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard

//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotLayout wrapper class

class CPivotLayout : public COleDispatchDriver
{
public:
	CPivotLayout()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotLayout(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotLayout(const CPivotLayout& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotLayout methods
public:
	LPDISPATCH get_Application()
	{
		LPDISPATCH result;
		InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	long get_Creator()
	{
		long result;
		InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	LPDISPATCH get_Parent()
	{
		LPDISPATCH result;
		InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_ColumnFields(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2c9, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPDISPATCH get_DataFields(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2cb, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPDISPATCH get_PageFields(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2ca, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPDISPATCH get_RowFields(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2c8, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPDISPATCH get_HiddenFields(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2c7, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPDISPATCH get_VisibleFields(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2c6, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPDISPATCH get_PivotFields(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2ce, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPDISPATCH get_CubeFields()
	{
		LPDISPATCH result;
		InvokeHelper(0x72f, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_PivotCache()
	{
		LPDISPATCH result;
		InvokeHelper(0x5d8, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_PivotTable()
	{
		LPDISPATCH result;
		InvokeHelper(0x2cc, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	CString get_InnerDetail()
	{
		CString result;
		InvokeHelper(0x2ba, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void put_InnerDetail(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x2ba, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	void AddFields(VARIANT& RowFields, VARIANT& ColumnFields, VARIANT& PageFields, VARIANT& AppendField)
	{
		static BYTE parms[] = VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
		InvokeHelper(0x2c4, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, &RowFields, &ColumnFields, &PageFields,
		             &AppendField);
	}

	// PivotLayout properties
public:
};
