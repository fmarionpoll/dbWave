// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard
#pragma once
//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotCell wrapper class

class CPivotCell : public COleDispatchDriver
{
public:
	CPivotCell()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotCell(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotCell(const CPivotCell& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotCell methods
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

	long get_PivotCellType()
	{
		long result;
		InvokeHelper(0x88d, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	LPDISPATCH get_PivotTable()
	{
		LPDISPATCH result;
		InvokeHelper(0x2cc, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_DataField()
	{
		LPDISPATCH result;
		InvokeHelper(0x82b, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_PivotField()
	{
		LPDISPATCH result;
		InvokeHelper(0x2db, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_PivotItem()
	{
		LPDISPATCH result;
		InvokeHelper(0x2e4, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_RowItems()
	{
		LPDISPATCH result;
		InvokeHelper(0x88e, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_ColumnItems()
	{
		LPDISPATCH result;
		InvokeHelper(0x88f, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_Range()
	{
		LPDISPATCH result;
		InvokeHelper(0xc5, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	CString get_Dummy18()
	{
		CString result;
		InvokeHelper(0x8f7, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	long get_CustomSubtotalFunction()
	{
		long result;
		InvokeHelper(0x891, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	LPDISPATCH get_PivotRowLine()
	{
		LPDISPATCH result;
		InvokeHelper(0xa71, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	LPDISPATCH get_PivotColumnLine()
	{
		LPDISPATCH result;
		InvokeHelper(0xa72, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	void AllocateChange()
	{
		InvokeHelper(0xb70, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}

	void DiscardChange()
	{
		InvokeHelper(0xb71, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}

	VARIANT get_DataSourceValue()
	{
		VARIANT result;
		InvokeHelper(0xb72, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	long get_CellChanged()
	{
		long result;
		InvokeHelper(0xb73, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	CString get_MDX()
	{
		CString result;
		InvokeHelper(0x84b, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	// PivotCell properties
public:
};
