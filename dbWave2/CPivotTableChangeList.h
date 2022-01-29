// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard

//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotTableChangeList wrapper class

class CPivotTableChangeList : public COleDispatchDriver
{
public:
	CPivotTableChangeList()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotTableChangeList(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotTableChangeList(const CPivotTableChangeList& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotTableChangeList methods
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

	LPDISPATCH get__Default(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	LPUNKNOWN get__NewEnum()
	{
		LPUNKNOWN result;
		InvokeHelper(0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, &result, nullptr);
		return result;
	}

	LPDISPATCH get_Item(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0xaa, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, parms, &Index);
		return result;
	}

	long get_Count()
	{
		long result;
		InvokeHelper(0x76, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	LPDISPATCH Add(LPCTSTR Tuple, double Value, VARIANT& AllocationValue, VARIANT& AllocationMethod,
	               VARIANT& AllocationWeightExpression)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_BSTR VTS_R8 VTS_VARIANT VTS_VARIANT VTS_VARIANT;
		InvokeHelper(0xb5, DISPATCH_METHOD, VT_DISPATCH, &result, parms, Tuple, Value, &AllocationValue,
		             &AllocationMethod, &AllocationWeightExpression);
		return result;
	}

	// PivotTableChangeList properties
public:
};
