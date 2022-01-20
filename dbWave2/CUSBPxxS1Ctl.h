// CUSBPxxS1Ctl.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

#include "ImportAlligatorDefinitions.h"

//**************************************************************************************
// All parameters that can be programmed in an individual USBPxxS1 are in the structure
// typedef listed below.  Even though each module type may not have the hardware to
// support the function, all module types firmware is the same.  For instance the
// USBPGF-S1 does not have a high pass filter so HPFc is not functional.
//**************************************************************************************
class USBPxxPARAMETERS : public CObject
{
	DECLARE_SERIAL(USBPxxPARAMETERS);

public:
	float	LPFc;
	float	HPFc;

	long	DeviceHandle;
	long	ChannelNumber;
	long	SerialNumber;
	long	ProductID;

	long	indexgain;
	long	indexCoupling;
	long	indexClockSource;
	long	indexPClock;
	long	indexLPFilterType;
	long	indexHPFilterType;

	long	Gain;
	int		RevisionHigh;
	int		RevisionLow;

	CString Description;
	CString csCoupling;
	CString csClockSource;
	CString csPClock;
	CString csLPFilterType;
	CString csHPFilterType;

public:
	USBPxxPARAMETERS();								// constructor
	~USBPxxPARAMETERS();							// destructor
	USBPxxPARAMETERS& operator = (const USBPxxPARAMETERS& arg);	// operator redefinition
	long Write(CFile* datafile);
	BOOL Read(CFile* datafile);
	void Serialize(CArchive& ar) override;
};

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
		LPCTSTR lpszWindowName, DWORD dw_style,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = nullptr)
	{
		return CreateControl(GetClsid(), lpszWindowName, dw_style, rect, pParentWnd, nID);
	}

	BOOL Create(LPCTSTR lpszWindowName, DWORD dw_style,
		const RECT& rect, CWnd* pParentWnd, UINT nID,
		CFile* pPersist = nullptr, BOOL bStorage = FALSE,
		BSTR bstrLicKey = nullptr)
	{
		return CreateControl(GetClsid(), lpszWindowName, dw_style, rect, pParentWnd, nID, pPersist, bStorage, bstrLicKey);
	}

	// Attributes
	long	devicesConnected{};

	// Operations
	void USBPxxS1Command(long Handle, long CmdID, VARIANT* DataInPtr, VARIANT* DataOutPtr)
	{
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Handle, CmdID, DataInPtr, DataOutPtr);
	}

	// constants
private:
	static int		allig_Gain[];
	static CString	allig_Coupling[];
	static CString	allig_ClockSource[];
	static CString	allig_PClock[];
	static CString	allig_LPFilterType[];
	static CString	allig_HPFilterType[];

	// functions
public:
	void	readLPFC(USBPxxPARAMETERS* d);
	void	readHPFC(USBPxxPARAMETERS* d);
	void	readGain(USBPxxPARAMETERS* d);
	void	readCoupling(USBPxxPARAMETERS* d);
	void	readClocksource(USBPxxPARAMETERS* d);
	void	readPClock(USBPxxPARAMETERS* d);
	void	readChannelNumber(USBPxxPARAMETERS* d);
	void	readDescription(USBPxxPARAMETERS* d);
	void	readLowPassFilterType(USBPxxPARAMETERS* d);
	void	readHighPassFilterType(USBPxxPARAMETERS* d);
	void	readSerialNumber(USBPxxPARAMETERS* d);
	void	readProductID(USBPxxPARAMETERS* d);
	void	readRevision(USBPxxPARAMETERS* d);
	long	readNumberOfDevicesConnected();
	long	readHandleOfDevice(long device);
	bool	readAllParameters(long device, USBPxxPARAMETERS* d);

	void	writeLPFC(USBPxxPARAMETERS* d);
	void	writeHPFC(USBPxxPARAMETERS* d);
	void	writeGainIndex(USBPxxPARAMETERS* d);
	void	writeCouplingIndex(USBPxxPARAMETERS* d);
	void	writeClockSourceIndex(USBPxxPARAMETERS* d);
	void	writePClockIndex(USBPxxPARAMETERS* d);
	void	writeChannelNumber(USBPxxPARAMETERS* d);
	void	writeDescription(USBPxxPARAMETERS* d);

	// dbWave-specific functions
	bool	SetWaveChanParms(long device, CWaveChan* pchan);
	bool	GetWaveChanParms(long device, CWaveChan* pchan);
	int		ConvertAbsoluteGainToIndexGain(long gain);
};
