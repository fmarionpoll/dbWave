// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard
#pragma once
//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotFields wrapper class

class CPivotFields : public COleDispatchDriver
{
public:
	CPivotFields()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotFields(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotFields(const CPivotFields& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotFields methods
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

	long get_Count()
	{
		long result;
		InvokeHelper(0x76, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	LPDISPATCH Item(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0xaa, DISPATCH_METHOD, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPUNKNOWN _NewEnum()
	{
		LPUNKNOWN result;
		InvokeHelper(0xfffffffc, DISPATCH_METHOD, VT_UNKNOWN, &result, nullptr);
		return result;
	}

	// PivotFields properties
public:
};
