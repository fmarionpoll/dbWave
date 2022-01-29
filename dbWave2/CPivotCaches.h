// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard
#pragma once
//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotCaches wrapper class

class CPivotCaches : public COleDispatchDriver
{
public:
	CPivotCaches()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotCaches(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotCaches(const CPivotCaches& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotCaches methods
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

	LPDISPATCH get__Default(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPUNKNOWN _NewEnum()
	{
		LPUNKNOWN result;
		InvokeHelper(0xfffffffc, DISPATCH_METHOD, VT_UNKNOWN, &result, nullptr);
		return result;
	}

	LPDISPATCH Add(long SourceType, VARIANT& SourceData)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_I4 VTS_VARIANT;
		InvokeHelper(0xb5, DISPATCH_METHOD, VT_DISPATCH, &result, parms, SourceType, &SourceData);
		return result;
	}

	LPDISPATCH Create(long SourceType, VARIANT& SourceData, VARIANT& Version)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_I4 VTS_VARIANT VTS_VARIANT;
		InvokeHelper(0x768, DISPATCH_METHOD, VT_DISPATCH, &result, parms, SourceType, &SourceData, &Version);
		return result;
	}

	// PivotCaches properties
public:
};
