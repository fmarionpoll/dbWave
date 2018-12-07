// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "dtplot32.h"

/////////////////////////////////////////////////////////////////////////////
// CDTPlot32

IMPLEMENT_DYNCREATE(CDTPlot32, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CDTPlot32 properties

unsigned long CDTPlot32::GetBackColor()
{
	unsigned long result;
	GetProperty(0x3, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetBackColor(unsigned long propVal)
{
	SetProperty(0x3, VT_I4, propVal);
}

unsigned long CDTPlot32::GetForeColor()
{
	unsigned long result;
	GetProperty(0x4, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetForeColor(unsigned long propVal)
{
	SetProperty(0x4, VT_I4, propVal);
}

float CDTPlot32::GetYMin()
{
	float result;
	GetProperty(0x5, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetYMin(float propVal)
{
	SetProperty(0x5, VT_R4, propVal);
}

float CDTPlot32::GetYMax()
{
	float result;
	GetProperty(0x6, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetYMax(float propVal)
{
	SetProperty(0x6, VT_R4, propVal);
}

float CDTPlot32::GetXStart()
{
	float result;
	GetProperty(0x7, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetXStart(float propVal)
{
	SetProperty(0x7, VT_R4, propVal);
}

float CDTPlot32::GetXLength()
{
	float result;
	GetProperty(0x8, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetXLength(float propVal)
{
	SetProperty(0x8, VT_R4, propVal);
}

long CDTPlot32::GetLineStyle()
{
	long result;
	GetProperty(0x9, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetLineStyle(long propVal)
{
	SetProperty(0x9, VT_I4, propVal);
}

short CDTPlot32::GetLineWidth()
{
	short result;
	GetProperty(0xa, VT_I2, (void*)&result);
	return result;
}

void CDTPlot32::SetLineWidth(short propVal)
{
	SetProperty(0xa, VT_I2, propVal);
}

long CDTPlot32::GetDataType()
{
	long result;
	GetProperty(0xb, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetDataType(long propVal)
{
	SetProperty(0xb, VT_I4, propVal);
}

short CDTPlot32::GetNumChannels()
{
	short result;
	GetProperty(0xc, VT_I2, (void*)&result);
	return result;
}

void CDTPlot32::SetNumChannels(short propVal)
{
	SetProperty(0xc, VT_I2, propVal);
}

float CDTPlot32::GetXScale()
{
	float result;
	GetProperty(0xd, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetXScale(float propVal)
{
	SetProperty(0xd, VT_R4, propVal);
}

unsigned long CDTPlot32::GetMarkerColor()
{
	unsigned long result;
	GetProperty(0xe, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerColor(unsigned long propVal)
{
	SetProperty(0xe, VT_I4, propVal);
}

BOOL CDTPlot32::GetMarkerV1On()
{
	BOOL result;
	GetProperty(0xf, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerV1On(BOOL propVal)
{
	SetProperty(0xf, VT_BOOL, propVal);
}

float CDTPlot32::GetMarkerV1Pos()
{
	float result;
	GetProperty(0x10, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerV1Pos(float propVal)
{
	SetProperty(0x10, VT_R4, propVal);
}

BOOL CDTPlot32::GetMarkerV2On()
{
	BOOL result;
	GetProperty(0x11, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerV2On(BOOL propVal)
{
	SetProperty(0x11, VT_BOOL, propVal);
}

float CDTPlot32::GetMarkerV2Pos()
{
	float result;
	GetProperty(0x12, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerV2Pos(float propVal)
{
	SetProperty(0x12, VT_R4, propVal);
}

BOOL CDTPlot32::GetMarkerH1On()
{
	BOOL result;
	GetProperty(0x13, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerH1On(BOOL propVal)
{
	SetProperty(0x13, VT_BOOL, propVal);
}

float CDTPlot32::GetMarkerH1Pos()
{
	float result;
	GetProperty(0x14, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerH1Pos(float propVal)
{
	SetProperty(0x14, VT_R4, propVal);
}

BOOL CDTPlot32::GetMarkerH2On()
{
	BOOL result;
	GetProperty(0x15, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerH2On(BOOL propVal)
{
	SetProperty(0x15, VT_BOOL, propVal);
}

float CDTPlot32::GetMarkerH2Pos()
{
	float result;
	GetProperty(0x16, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetMarkerH2Pos(float propVal)
{
	SetProperty(0x16, VT_R4, propVal);
}

unsigned long CDTPlot32::GetGridColor()
{
	unsigned long result;
	GetProperty(0x17, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetGridColor(unsigned long propVal)
{
	SetProperty(0x17, VT_I4, propVal);
}

long CDTPlot32::GetGridStyle()
{
	long result;
	GetProperty(0x18, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetGridStyle(long propVal)
{
	SetProperty(0x18, VT_I4, propVal);
}

BOOL CDTPlot32::GetGridAutoScale()
{
	BOOL result;
	GetProperty(0x19, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetGridAutoScale(BOOL propVal)
{
	SetProperty(0x19, VT_BOOL, propVal);
}

BOOL CDTPlot32::GetGridXOn()
{
	BOOL result;
	GetProperty(0x1a, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetGridXOn(BOOL propVal)
{
	SetProperty(0x1a, VT_BOOL, propVal);
}

float CDTPlot32::GetGridXStart()
{
	float result;
	GetProperty(0x1b, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetGridXStart(float propVal)
{
	SetProperty(0x1b, VT_R4, propVal);
}

float CDTPlot32::GetGridXSpacing()
{
	float result;
	GetProperty(0x1c, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetGridXSpacing(float propVal)
{
	SetProperty(0x1c, VT_R4, propVal);
}

BOOL CDTPlot32::GetGridYOn()
{
	BOOL result;
	GetProperty(0x1d, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetGridYOn(BOOL propVal)
{
	SetProperty(0x1d, VT_BOOL, propVal);
}

float CDTPlot32::GetGridYStart()
{
	float result;
	GetProperty(0x1e, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetGridYStart(float propVal)
{
	SetProperty(0x1e, VT_R4, propVal);
}

float CDTPlot32::GetGridYSpacing()
{
	float result;
	GetProperty(0x1f, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetGridYSpacing(float propVal)
{
	SetProperty(0x1f, VT_R4, propVal);
}

BOOL CDTPlot32::GetUpdateMode()
{
	BOOL result;
	GetProperty(0x20, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetUpdateMode(BOOL propVal)
{
	SetProperty(0x20, VT_BOOL, propVal);
}

BOOL CDTPlot32::GetXAutoScale()
{
	BOOL result;
	GetProperty(0x1, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetXAutoScale(BOOL propVal)
{
	SetProperty(0x1, VT_BOOL, propVal);
}

BOOL CDTPlot32::GetYAutoScale()
{
	BOOL result;
	GetProperty(0x2, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetYAutoScale(BOOL propVal)
{
	SetProperty(0x2, VT_BOOL, propVal);
}

BOOL CDTPlot32::GetStripChartMode()
{
	BOOL result;
	GetProperty(0x21, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetStripChartMode(BOOL propVal)
{
	SetProperty(0x21, VT_BOOL, propVal);
}

long CDTPlot32::GetStripChartSize()
{
	long result;
	GetProperty(0x22, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetStripChartSize(long propVal)
{
	SetProperty(0x22, VT_I4, propVal);
}

OLE_HANDLE CDTPlot32::GetHWnd()
{
	OLE_HANDLE result;
	GetProperty(DISPID_HWND, VT_I2, (void*)&result);
	return result;
}

void CDTPlot32::SetHWnd(OLE_HANDLE propVal)
{
	SetProperty(DISPID_HWND, VT_I2, propVal);
}

long CDTPlot32::GetBuffer()
{
	long result;
	GetProperty(0x23, VT_I4, (void*)&result);
	return result;
}

void CDTPlot32::SetBuffer(long propVal)
{
	SetProperty(0x23, VT_I4, propVal);
}

float CDTPlot32::GetMouseXPos()
{
	float result;
	GetProperty(0x24, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetMouseXPos(float propVal)
{
	SetProperty(0x24, VT_R4, propVal);
}

float CDTPlot32::GetMouseYPos()
{
	float result;
	GetProperty(0x25, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetMouseYPos(float propVal)
{
	SetProperty(0x25, VT_R4, propVal);
}

short CDTPlot32::GetBorderStyle()
{
	short result;
	GetProperty(DISPID_BORDERSTYLE, VT_I2, (void*)&result);
	return result;
}

void CDTPlot32::SetBorderStyle(short propVal)
{
	SetProperty(DISPID_BORDERSTYLE, VT_I2, propVal);
}

float CDTPlot32::GetSinglePoint()
{
	float result;
	GetProperty(0x26, VT_R4, (void*)&result);
	return result;
}

void CDTPlot32::SetSinglePoint(float propVal)
{
	SetProperty(0x26, VT_R4, propVal);
}

BOOL CDTPlot32::GetForceRepaint()
{
	BOOL result;
	GetProperty(0x27, VT_BOOL, (void*)&result);
	return result;
}

void CDTPlot32::SetForceRepaint(BOOL propVal)
{
	SetProperty(0x27, VT_BOOL, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CDTPlot32 operations

unsigned long CDTPlot32::GetPalette(short index)
{
	unsigned long result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x28, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, parms,
		index);
	return result;
}

void CDTPlot32::SetPalette(short index, unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I2 VTS_I4;
	InvokeHelper(0x28, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 index, newValue);
}

float CDTPlot32::GetMarkerV1Data(short index)
{
	float result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x29, DISPATCH_PROPERTYGET, VT_R4, (void*)&result, parms,
		index);
	return result;
}

void CDTPlot32::SetMarkerV1Data(short index, float newValue)
{
	static BYTE parms[] =
		VTS_I2 VTS_R4;
	InvokeHelper(0x29, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 index, newValue);
}

float CDTPlot32::GetMarkerV2Data(short index)
{
	float result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x2a, DISPATCH_PROPERTYGET, VT_R4, (void*)&result, parms,
		index);
	return result;
}

void CDTPlot32::SetMarkerV2Data(short index, float newValue)
{
	static BYTE parms[] =
		VTS_I2 VTS_R4;
	InvokeHelper(0x2a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 index, newValue);
}

void CDTPlot32::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}
