// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

#include "StdAfx.h"
#include "dtacq32.h"

/////////////////////////////////////////////////////////////////////////////
// CDTAcq32

IMPLEMENT_DYNCREATE(CDTAcq32, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CDTAcq32 properties

long CDTAcq32::GetHDass()
{
	long result;
	GetProperty(0x1, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetHDass(long propVal)
{
	SetProperty(0x1, VT_I4, propVal);
}

double CDTAcq32::GetFrequency()
{
	double result;
	GetProperty(0x2, VT_R8, (void*)&result);
	return result;
}

void CDTAcq32::SetFrequency(double propVal)
{
	SetProperty(0x2, VT_R8, propVal);
}

short CDTAcq32::GetDmaUsage()
{
	short result;
	GetProperty(0x3, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetDmaUsage(short propVal)
{
	SetProperty(0x3, VT_I2, propVal);
}

short CDTAcq32::GetTriggeredScan()
{
	short result;
	GetProperty(0x4, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetTriggeredScan(short propVal)
{
	SetProperty(0x4, VT_I2, propVal);
}

short CDTAcq32::GetChannelType()
{
	short result;
	GetProperty(0x5, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetChannelType(short propVal)
{
	SetProperty(0x5, VT_I2, propVal);
}

short CDTAcq32::GetEncoding()
{
	short result;
	GetProperty(0x6, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetEncoding(short propVal)
{
	SetProperty(0x6, VT_I2, propVal);
}

short CDTAcq32::GetTrigger()
{
	short result;
	GetProperty(0x7, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetTrigger(short propVal)
{
	SetProperty(0x7, VT_I2, propVal);
}

short CDTAcq32::GetClockSource()
{
	short result;
	GetProperty(0x8, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetClockSource(short propVal)
{
	SetProperty(0x8, VT_I2, propVal);
}

double CDTAcq32::GetRetriggerFreq()
{
	double result;
	GetProperty(0x9, VT_R8, (void*)&result);
	return result;
}

void CDTAcq32::SetRetriggerFreq(double propVal)
{
	SetProperty(0x9, VT_R8, propVal);
}

long CDTAcq32::GetClockDivider()
{
	long result;
	GetProperty(0xa, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetClockDivider(long propVal)
{
	SetProperty(0xa, VT_I4, propVal);
}

short CDTAcq32::GetResolution()
{
	short result;
	GetProperty(0xb, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetResolution(short propVal)
{
	SetProperty(0xb, VT_I2, propVal);
}

short CDTAcq32::GetRange()
{
	short result;
	GetProperty(0xc, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetRange(short propVal)
{
	SetProperty(0xc, VT_I2, propVal);
}

float CDTAcq32::GetMinRange()
{
	float result;
	GetProperty(0xd, VT_R4, (void*)&result);
	return result;
}

void CDTAcq32::SetMinRange(float propVal)
{
	SetProperty(0xd, VT_R4, propVal);
}

float CDTAcq32::GetMaxRange()
{
	float result;
	GetProperty(0xe, VT_R4, (void*)&result);
	return result;
}

void CDTAcq32::SetMaxRange(float propVal)
{
	SetProperty(0xe, VT_R4, propVal);
}

short CDTAcq32::GetDataFlow()
{
	short result;
	GetProperty(0xf, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetDataFlow(short propVal)
{
	SetProperty(0xf, VT_I2, propVal);
}

short CDTAcq32::GetWrapMode()
{
	short result;
	GetProperty(0x10, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetWrapMode(short propVal)
{
	SetProperty(0x10, VT_I2, propVal);
}

short CDTAcq32::GetGateType()
{
	short result;
	GetProperty(0x11, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetGateType(short propVal)
{
	SetProperty(0x11, VT_I2, propVal);
}

short CDTAcq32::GetPulseType()
{
	short result;
	GetProperty(0x12, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetPulseType(short propVal)
{
	SetProperty(0x12, VT_I2, propVal);
}

double CDTAcq32::GetPulseWidth()
{
	double result;
	GetProperty(0x13, VT_R8, (void*)&result);
	return result;
}

void CDTAcq32::SetPulseWidth(double propVal)
{
	SetProperty(0x13, VT_R8, propVal);
}

short CDTAcq32::GetCTMode()
{
	short result;
	GetProperty(0x14, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetCTMode(short propVal)
{
	SetProperty(0x14, VT_I2, propVal);
}

short CDTAcq32::GetCascadeMode()
{
	short result;
	GetProperty(0x15, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetCascadeMode(short propVal)
{
	SetProperty(0x15, VT_I2, propVal);
}

long CDTAcq32::GetReserved()
{
	long result;
	GetProperty(0x16, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetReserved(long propVal)
{
	SetProperty(0x16, VT_I4, propVal);
}

short CDTAcq32::GetNumResolutions()
{
	short result;
	GetProperty(0x17, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetNumResolutions(short propVal)
{
	SetProperty(0x17, VT_I2, propVal);
}

short CDTAcq32::GetNumRanges()
{
	short result;
	GetProperty(0x18, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetNumRanges(short propVal)
{
	SetProperty(0x18, VT_I2, propVal);
}

short CDTAcq32::GetNumGains()
{
	short result;
	GetProperty(0x19, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetNumGains(short propVal)
{
	SetProperty(0x19, VT_I2, propVal);
}

short CDTAcq32::GetNumFilters()
{
	short result;
	GetProperty(0x1a, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetNumFilters(short propVal)
{
	SetProperty(0x1a, VT_I2, propVal);
}

long CDTAcq32::GetQueue()
{
	long result;
	GetProperty(0x1b, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetQueue(long propVal)
{
	SetProperty(0x1b, VT_I4, propVal);
}

short CDTAcq32::GetSyncDIOUsage()
{
	short result;
	GetProperty(0x1c, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetSyncDIOUsage(short propVal)
{
	SetProperty(0x1c, VT_I2, propVal);
}

long CDTAcq32::GetDTConnectXferSize()
{
	long result;
	GetProperty(0x1d, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetDTConnectXferSize(long propVal)
{
	SetProperty(0x1d, VT_I4, propVal);
}

long CDTAcq32::GetDTConnectXferCount()
{
	long result;
	GetProperty(0x1e, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetDTConnectXferCount(long propVal)
{
	SetProperty(0x1e, VT_I4, propVal);
}

short CDTAcq32::GetNumSubSystems()
{
	short result;
	GetProperty(0x1f, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetNumSubSystems(short propVal)
{
	SetProperty(0x1f, VT_I2, propVal);
}

short CDTAcq32::GetSubSystem()
{
	short result;
	GetProperty(0x20, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetSubSystem(short propVal)
{
	SetProperty(0x20, VT_I2, propVal);
}

short CDTAcq32::GetSubSysType()
{
	short result;
	GetProperty(0x21, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetSubSysType(short propVal)
{
	SetProperty(0x21, VT_I2, propVal);
}

short CDTAcq32::GetSubSysElement()
{
	short result;
	GetProperty(0x22, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetSubSysElement(short propVal)
{
	SetProperty(0x22, VT_I2, propVal);
}

long CDTAcq32::GetHDev()
{
	long result;
	GetProperty(0x23, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetHDev(long propVal)
{
	SetProperty(0x23, VT_I4, propVal);
}

short CDTAcq32::GetNumBoards()
{
	short result;
	GetProperty(0x24, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetNumBoards(short propVal)
{
	SetProperty(0x24, VT_I2, propVal);
}

long CDTAcq32::GetListSize()
{
	long result;
	GetProperty(0x25, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetListSize(long propVal)
{
	SetProperty(0x25, VT_I4, propVal);
}

OLE_HANDLE CDTAcq32::GetHWnd()
{
	OLE_HANDLE result;
	GetProperty(DISPID_HWND, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetHWnd(OLE_HANDLE propVal)
{
	SetProperty(DISPID_HWND, VT_I2, propVal);
}

CString CDTAcq32::GetBoard()
{
	CString result;
	GetProperty(0x26, VT_BSTR, (void*)&result);
	return result;
}

void CDTAcq32::SetBoard(LPCTSTR propVal)
{
	SetProperty(0x26, VT_BSTR, propVal);
}

CString CDTAcq32::GetCaption()
{
	CString result;
	GetProperty(DISPID_CAPTION, VT_BSTR, (void*)&result);
	return result;
}

void CDTAcq32::SetCaption(LPCTSTR propVal)
{
	SetProperty(DISPID_CAPTION, VT_BSTR, propVal);
}

short CDTAcq32::GetRetriggerMode()
{
	short result;
	GetProperty(0x27, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetRetriggerMode(short propVal)
{
	SetProperty(0x27, VT_I2, propVal);
}

long CDTAcq32::GetLastError()
{
	long result;
	GetProperty(0x28, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetLastError(long propVal)
{
	SetProperty(0x28, VT_I4, propVal);
}

CString CDTAcq32::GetLastErrDescription()
{
	CString result;
	GetProperty(0x29, VT_BSTR, (void*)&result);
	return result;
}

void CDTAcq32::SetLastErrDescription(LPCTSTR propVal)
{
	SetProperty(0x29, VT_BSTR, propVal);
}

CString CDTAcq32::GetDeviceName()
{
	CString result;
	GetProperty(0x2a, VT_BSTR, (void*)&result);
	return result;
}

void CDTAcq32::SetDeviceName(LPCTSTR propVal)
{
	SetProperty(0x2a, VT_BSTR, propVal);
}

short CDTAcq32::GetRetrigger()
{
	short result;
	GetProperty(0x4c, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetRetrigger(short propVal)
{
	SetProperty(0x4c, VT_I2, propVal);
}

long CDTAcq32::GetMultiScanCount()
{
	long result;
	GetProperty(0x4d, VT_I4, (void*)&result);
	return result;
}

void CDTAcq32::SetMultiScanCount(long propVal)
{
	SetProperty(0x4d, VT_I4, propVal);
}

short CDTAcq32::GetPreTrigger()
{
	short result;
	GetProperty(0x4e, VT_I2, (void*)&result);
	return result;
}

void CDTAcq32::SetPreTrigger(short propVal)
{
	SetProperty(0x4e, VT_I2, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CDTAcq32 operations

void CDTAcq32::Config()
{
	InvokeHelper(0x2b, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::Start()
{
	InvokeHelper(0x2c, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::Stop()
{
	InvokeHelper(0x2d, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::Abort()
{
	InvokeHelper(0x2e, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::Pause()
{
	InvokeHelper(0x2f, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::Continue()
{
	InvokeHelper(0x30, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::Flush()
{
	InvokeHelper(0x31, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::Reset()
{
	InvokeHelper(0x32, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::PutSingleValue(long uiChannel, double dGain, long lValue)
{
	static BYTE parms[] =
		VTS_I4 VTS_R8 VTS_I4;
	InvokeHelper(0x33, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		uiChannel, dGain, lValue);
}

long CDTAcq32::GetSingleValue(long uiChannel, double dGain)
{
	long result;
	static BYTE parms[] =
		VTS_I4 VTS_R8;
	InvokeHelper(0x34, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		uiChannel, dGain);
	return result;
}

long CDTAcq32::CTReadEvents()
{
	long result;
	InvokeHelper(0x35, DISPATCH_METHOD, VT_I4, (void*)&result, nullptr);
	return result;
}

void CDTAcq32::MeasureFrequency(double dCountDuration)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x36, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		dCountDuration);
}

void CDTAcq32::EnumBoards()
{
	InvokeHelper(0x37, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::EnumSS()
{
	InvokeHelper(0x38, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::EnumSSCaps(short uiCap)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x39, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		uiCap);
}

void CDTAcq32::ClearError()
{
	InvokeHelper(0x3a, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void CDTAcq32::DTConnectBurst(long TransferSize, long TransferCount)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0x3b, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		TransferSize, TransferCount);
}

short CDTAcq32::GetDevCaps(short olDc)
{
	short result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x3c, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		olDc);
	return result;
}

long CDTAcq32::GetSSCaps(short OlSSc)
{
	long result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x3d, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		OlSSc);
	return result;
}

double CDTAcq32::GetSSCapsEx(short OlSSc)
{
	double result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x3e, DISPATCH_METHOD, VT_R8, (void*)&result, parms,
		OlSSc);
	return result;
}

short CDTAcq32::GetChannelList(short index)
{
	short result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x3f, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, parms,
		index);
	return result;
}

void CDTAcq32::SetChannelList(short index, short nNewValue)
{
	static BYTE parms[] =
		VTS_I2 VTS_I2;
	InvokeHelper(0x3f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		index, nNewValue);
}

double CDTAcq32::GetGainList(short index)
{
	double result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x40, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
		index);
	return result;
}

void CDTAcq32::SetGainList(short index, double newValue)
{
	static BYTE parms[] =
		VTS_I2 VTS_R8;
	InvokeHelper(0x40, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		index, newValue);
}

double CDTAcq32::GetFilterList(short index)
{
	double result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x41, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
		index);
	return result;
}

void CDTAcq32::SetFilterList(short index, double newValue)
{
	static BYTE parms[] =
		VTS_I2 VTS_R8;
	InvokeHelper(0x41, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		index, newValue);
}

short CDTAcq32::GetResolutionValues(short index)
{
	short result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x42, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, parms,
		index);
	return result;
}

double CDTAcq32::GetMinRangeValues(short index)
{
	double result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x43, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
		index);
	return result;
}

double CDTAcq32::GetMaxRangeValues(short index)
{
	double result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x44, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
		index);
	return result;
}

double CDTAcq32::GetGainValues(short index)
{
	double result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x45, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
		index);
	return result;
}

double CDTAcq32::GetFilterValues(short index)
{
	double result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x46, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
		index);
	return result;
}

BOOL CDTAcq32::GetInhibitList(short index)
{
	BOOL result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x47, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, parms,
		index);
	return result;
}

void CDTAcq32::SetInhibitList(short index, BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_I2 VTS_BOOL;
	InvokeHelper(0x47, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		index, bNewValue);
}

long CDTAcq32::GetDIOList(short index)
{
	long result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x48, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, parms,
		index);
	return result;
}

void CDTAcq32::SetDIOList(short index, long nNewValue)
{
	static BYTE parms[] =
		VTS_I2 VTS_I4;
	InvokeHelper(0x48, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		index, nNewValue);
}

CString CDTAcq32::GetSubSystemList(short index)
{
	CString result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x49, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, parms,
		index);
	return result;
}

CString CDTAcq32::GetBoardList(short index)
{
	CString result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x4a, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, parms,
		index);
	return result;
}

short CDTAcq32::GetQueueSize(short uiQueue)
{
	short result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x4b, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, parms,
		uiQueue);
	return result;
}

void CDTAcq32::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}