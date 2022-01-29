// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard
#pragma once
//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotFormula wrapper class

class CPivotFormula : public COleDispatchDriver
{
public:
	CPivotFormula()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotFormula(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotFormula(const CPivotFormula& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotFormula methods
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

	void Delete()
	{
		InvokeHelper(0x75, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
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

	long get_Index()
	{
		long result;
		InvokeHelper(0x1e6, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	void put_Index(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0x1e6, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
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

	// PivotFormula properties
public:
};
