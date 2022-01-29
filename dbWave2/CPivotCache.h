// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard
#pragma once
//#import "C:\\Program Files (x86)\\Microsoft Office\\Office14\\EXCEL.EXE" no_namespace
#include "ImportExcelDefinitions.h"

// CPivotCache wrapper class

class CPivotCache : public COleDispatchDriver
{
public:
	CPivotCache()
	{
	} // Calls COleDispatchDriver default constructor
	CPivotCache(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch)
	{
	}

	CPivotCache(const CPivotCache& dispatchSrc) : COleDispatchDriver(dispatchSrc)
	{
	}

	// Attributes
public:
	// Operations
public:
	// PivotCache methods
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

	BOOL get_BackgroundQuery()
	{
		BOOL result;
		InvokeHelper(0x593, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_BackgroundQuery(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x593, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	VARIANT get_Connection()
	{
		VARIANT result;
		InvokeHelper(0x598, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	void put_Connection(VARIANT& newValue)
	{
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x598, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, &newValue);
	}

	BOOL get_EnableRefresh()
	{
		BOOL result;
		InvokeHelper(0x5c5, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_EnableRefresh(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x5c5, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	long get_Index()
	{
		long result;
		InvokeHelper(0x1e6, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	long get_MemoryUsed()
	{
		long result;
		InvokeHelper(0x174, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	BOOL get_OptimizeCache()
	{
		BOOL result;
		InvokeHelper(0x594, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_OptimizeCache(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x594, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	long get_RecordCount()
	{
		long result;
		InvokeHelper(0x5c6, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	void Refresh()
	{
		InvokeHelper(0x589, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}

	DATE get_RefreshDate()
	{
		DATE result;
		InvokeHelper(0x2b8, DISPATCH_PROPERTYGET, VT_DATE, &result, nullptr);
		return result;
	}

	CString get_RefreshName()
	{
		CString result;
		InvokeHelper(0x2b9, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	BOOL get_RefreshOnFileOpen()
	{
		BOOL result;
		InvokeHelper(0x5c7, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_RefreshOnFileOpen(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x5c7, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	VARIANT get_Sql()
	{
		VARIANT result;
		InvokeHelper(0x5c8, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	void put_Sql(VARIANT& newValue)
	{
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x5c8, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, &newValue);
	}

	BOOL get_SavePassword()
	{
		BOOL result;
		InvokeHelper(0x5c9, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_SavePassword(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x5c9, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	VARIANT get_SourceData()
	{
		VARIANT result;
		InvokeHelper(0x2ae, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	void put_SourceData(VARIANT& newValue)
	{
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x2ae, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, &newValue);
	}

	VARIANT get_CommandText()
	{
		VARIANT result;
		InvokeHelper(0x725, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	void put_CommandText(VARIANT& newValue)
	{
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x725, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, &newValue);
	}

	long get_CommandType()
	{
		long result;
		InvokeHelper(0x726, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	void put_CommandType(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0x726, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	long get_QueryType()
	{
		long result;
		InvokeHelper(0x727, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	BOOL get_MaintainConnection()
	{
		BOOL result;
		InvokeHelper(0x728, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_MaintainConnection(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x728, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	long get_RefreshPeriod()
	{
		long result;
		InvokeHelper(0x729, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	void put_RefreshPeriod(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0x729, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	LPDISPATCH get_Recordset()
	{
		LPDISPATCH result;
		InvokeHelper(0x48d, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	void putref_Recordset(LPDISPATCH newValue)
	{
		static BYTE parms[] = VTS_DISPATCH;
		InvokeHelper(0x48d, DISPATCH_PROPERTYPUTREF, VT_EMPTY, nullptr, parms, newValue);
	}

	void ResetTimer()
	{
		InvokeHelper(0x72a, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}

	VARIANT get_LocalConnection()
	{
		VARIANT result;
		InvokeHelper(0x72b, DISPATCH_PROPERTYGET, VT_VARIANT, &result, nullptr);
		return result;
	}

	void put_LocalConnection(VARIANT& newValue)
	{
		static BYTE parms[] = VTS_VARIANT;
		InvokeHelper(0x72b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, &newValue);
	}

	LPDISPATCH CreatePivotTable(VARIANT& TableDestination, VARIANT& TableName, VARIANT& ReadData,
	                            VARIANT& DefaultVersion)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
		InvokeHelper(0x72c, DISPATCH_METHOD, VT_DISPATCH, &result, parms, &TableDestination, &TableName, &ReadData,
		             &DefaultVersion);
		return result;
	}

	BOOL get_UseLocalConnection()
	{
		BOOL result;
		InvokeHelper(0x72d, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_UseLocalConnection(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x72d, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	LPDISPATCH get_ADOConnection()
	{
		LPDISPATCH result;
		InvokeHelper(0x81a, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	BOOL get_IsConnected()
	{
		BOOL result;
		InvokeHelper(0x81b, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void MakeConnection()
	{
		InvokeHelper(0x81c, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}

	BOOL get_OLAP()
	{
		BOOL result;
		InvokeHelper(0x81d, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	long get_SourceType()
	{
		long result;
		InvokeHelper(0x2ad, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	long get_MissingItemsLimit()
	{
		long result;
		InvokeHelper(0x81e, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	void put_MissingItemsLimit(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0x81e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	CString get_SourceConnectionFile()
	{
		CString result;
		InvokeHelper(0x81f, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	void put_SourceConnectionFile(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x81f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	CString get_SourceDataFile()
	{
		CString result;
		InvokeHelper(0x820, DISPATCH_PROPERTYGET, VT_BSTR, &result, nullptr);
		return result;
	}

	long get_RobustConnect()
	{
		long result;
		InvokeHelper(0x821, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	void put_RobustConnect(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0x821, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	void SaveAsODC(LPCTSTR ODCFileName, VARIANT& Description, VARIANT& Keywords)
	{
		static BYTE parms[] = VTS_BSTR VTS_VARIANT VTS_VARIANT;
		InvokeHelper(0x822, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, ODCFileName, &Description, &Keywords);
	}

	LPDISPATCH get_WorkbookConnection()
	{
		LPDISPATCH result;
		InvokeHelper(0x9f0, DISPATCH_PROPERTYGET, VT_DISPATCH, &result, nullptr);
		return result;
	}

	long get_Version()
	{
		long result;
		InvokeHelper(0x188, DISPATCH_PROPERTYGET, VT_I4, &result, nullptr);
		return result;
	}

	BOOL get_UpgradeOnRefresh()
	{
		BOOL result;
		InvokeHelper(0x9f1, DISPATCH_PROPERTYGET, VT_BOOL, &result, nullptr);
		return result;
	}

	void put_UpgradeOnRefresh(BOOL newValue)
	{
		static BYTE parms[] = VTS_BOOL;
		InvokeHelper(0x9f1, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}

	// PivotCache properties
public:
};
