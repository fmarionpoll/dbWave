// CUSBPxxS1Ctl.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

#include "ImportAlligatorDefinitions.h"

//**************************************************************************************
// All of the parameters that can be programmed in an individual USBPxxS1 is in the structure
// typedef listed below.  Even though each module type may not have the hardware to 
// support the function, all module types firmware is the same.  For instance the 
// USBPGF-S1 does not have a high pass filter so HPFc is not functional.
//**************************************************************************************
typedef struct
{
	long	DeviceHandle;
	float	LPFc;
	float	HPFc;
	long	gain;
	long	coupling;
	long	ClockSource;
	long	PClock;
	long	ChannelNumber;
	long	LPFilterType;
	long	HPFilterType;
	long	SerialNumber;
	long	ProductID;
	int		RevisionHigh;
	int		RevisionLow;
	CString Description;
}	USBPxxPARAMETERS;

/////////////////////////////////////////////////////////////////////////////
// CUSBPxxS1Ctl

class CUSBPxxS1Ctl : public CWnd
{
protected:
	DECLARE_DYNCREATE(CUSBPxxS1Ctl)

public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x9A90F3D9, 0x6EEA, 0x4735, { 0x87, 0xCB, 0xE2, 0xDC, 0x7A, 0xF1, 0x6E, 0xC6 } };
		return clsid;
	}

	virtual BOOL Create(LPCTSTR lpszClassName, 
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, 
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = nullptr)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID);	}

	BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, 
		const RECT& rect, CWnd* pParentWnd, UINT nID, 
		CFile* pPersist = nullptr, BOOL bStorage = FALSE,
		BSTR bstrLicKey = nullptr)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID, 
		pPersist, bStorage, bstrLicKey); }

// Attributes
public:
	USBPxxPARAMETERS	device1;
	long	devicesConnected;
	long	deviceNumber;

// Operations
public:
	void USBPxxS1Command(long Handle, long CmdID, VARIANT * DataInPtr, VARIANT * DataOutPtr)
	{
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT ;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Handle, CmdID, DataInPtr, DataOutPtr);
	}

// functions
	void	readLPFC(USBPxxPARAMETERS *d);
	void	readHPFC(USBPxxPARAMETERS *d);
	void	readGain(USBPxxPARAMETERS *d);
	void	readCoupling(USBPxxPARAMETERS *d);
	void	readClocksource(USBPxxPARAMETERS *d);
	void	readPClock(USBPxxPARAMETERS *d);
	void	readChannelNumber(USBPxxPARAMETERS *d);
	void	readDescription(USBPxxPARAMETERS *d);
	void	readLowPassFilterType(USBPxxPARAMETERS *d);
	void	readHighPassFilterType(USBPxxPARAMETERS *d);
	void	readSerialNumber(USBPxxPARAMETERS *d);
	void	readProductID(USBPxxPARAMETERS *d);
	void	readRevision(USBPxxPARAMETERS *d);
	long	readNumberOfDevicesConnected();
	long	readHandleOfDevice(long device);
	void	readAllParameters(long device, USBPxxPARAMETERS *d);
	
	void	writeLPFC(USBPxxPARAMETERS *d);
	void	writeHPFC(USBPxxPARAMETERS *d);
	void	writeGain(USBPxxPARAMETERS *d);
	void	writeCoupling(USBPxxPARAMETERS *d);
	void	writeClockSource(USBPxxPARAMETERS *d);
	void	writePClock(USBPxxPARAMETERS *d);
	void	writeChannelNumber(USBPxxPARAMETERS *d);
	void	writeDescription(USBPxxPARAMETERS *d);

	// dbWave-specific functions
	void	SetWaveChanParms(CWaveChan* pChan);
	void	GetWaveChanParms(CWaveChan* pChan);
	void	InitializeDriver(LPDISPATCH pDisp);
};
