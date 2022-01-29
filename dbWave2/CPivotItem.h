// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard
#pragma once
//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotItem wrapper class

class CPivotItem : public COleDispatchDriver
{
public:
	CPivotItem()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotItem(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotItem(const CPivotItem& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotItem methods
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

	VARIANT get_ChildItems(VARIANT& Index)
	{
		VARIANT result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2da, DISPATCH_PROPERTYGET, VT_VARIANT, &result, parms, &Index);
		return result;
	}

	LPDISPATCH get_DataRange()
	{
		LPDISPATCH result;
		InvokeHelper(0x2d0, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	CString get__Default()
	{
		CString result;
		InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void put__Default(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x0, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	LPDISPATCH get_LabelRange()
	{
		LPDISPATCH result;
		InvokeHelper(0x2cf, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	CString get_Name()
	{
		CString result;
		InvokeHelper(0x6e, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void put_Name(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x6e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	LPDISPATCH get_ParentItem()
	{
		LPDISPATCH result;
		InvokeHelper(0x2e5, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	BOOL get_ParentShowDetail()
	{
		BOOL result;
		InvokeHelper(0x2e3, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	long get_Position()
	{
		long result;
		InvokeHelper(0x85, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	void put_Position(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0x85, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	BOOL get_ShowDetail()
	{
		BOOL result;
		InvokeHelper(0x249, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_ShowDetail(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x249, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	VARIANT get_SourceName()
	{
		VARIANT result;
		InvokeHelper(0x2d1, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	CString get_Value()
	{
		CString result;
		InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void put_Value(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x6, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	BOOL get_Visible()
	{
		BOOL result;
		InvokeHelper(0x22e, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_Visible(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x22e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	void Delete()
	{
		InvokeHelper(0x75, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}

	BOOL get_IsCalculated()
	{
		BOOL result;
		InvokeHelper(0x5e8, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	long get_RecordCount()
	{
		long result;
		InvokeHelper(0x5c6, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	CString get_Formula()
	{
		CString result;
		InvokeHelper(0x105, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void put_Formula(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x105, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	CString get_Caption()
	{
		CString result;
		InvokeHelper(0x8b, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void put_Caption(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x8b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	BOOL get_DrilledDown()
	{
		BOOL result;
		InvokeHelper(0x73a, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_DrilledDown(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x73a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	CString get_StandardFormula()
	{
		CString result;
		InvokeHelper(0x824, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void put_StandardFormula(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x824, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	CString get_SourceNameStandard()
	{
		CString result;
		InvokeHelper(0x864, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void DrillTo(LPCTSTR Field)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0xa14, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Field);
	}

	// PivotItem properties
public:
};
