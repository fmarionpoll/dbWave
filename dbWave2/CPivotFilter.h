// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard
#pragma once
//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotFilter wrapper class

class CPivotFilter : public COleDispatchDriver
{
public:
	CPivotFilter()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotFilter(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotFilter(const CPivotFilter& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotFilter methods
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

	long get_Order()
	{
		long result;
		InvokeHelper(0xc0, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	void put_Order(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0xc0, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	long get_FilterType()
	{
		long result;
		InvokeHelper(0xa7e, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	CString get_Name()
	{
		CString result;
		InvokeHelper(0x6e, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	CString get_Description()
	{
		CString result;
		InvokeHelper(0xda, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void Delete()
	{
		InvokeHelper(0x75, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}

	BOOL get_Active()
	{
		BOOL result;
		InvokeHelper(0x908, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	LPDISPATCH get_PivotField()
	{
		LPDISPATCH result;
		InvokeHelper(0x2db, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_DataField()
	{
		LPDISPATCH result;
		InvokeHelper(0x82b, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_DataCubeField()
	{
		LPDISPATCH result;
		InvokeHelper(0xa7f, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	VARIANT get_Value1()
	{
		VARIANT result;
		InvokeHelper(0xa80, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	VARIANT get_Value2()
	{
		VARIANT result;
		InvokeHelper(0x56c, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	LPDISPATCH get_MemberPropertyField()
	{
		LPDISPATCH result;
		InvokeHelper(0xa81, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	BOOL get_IsMemberPropertyFilter()
	{
		BOOL result;
		InvokeHelper(0xa82, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	// PivotFilter properties
public:
};
