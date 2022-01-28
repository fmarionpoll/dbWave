// CUSBPxxS1Ctl.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once
#include <objbase.h>
#include "ImportAlligatorDefinitions.h"
#include "USBPxxPARAMETERS.h"
//#import "./include/Alligator/win32/USBPxxS1COM.dll" 
#import "USBPxxS1COM.dll" no_namespace raw_interfaces_only raw_native_types named_guids


class CUSBPxxS1Ctl : public CWnd
{
protected:
	DECLARE_DYNCREATE(CUSBPxxS1Ctl)

	CUSBPxxS1Ctl();
	~CUSBPxxS1Ctl() override;


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
	long		devicesConnected = 0;
	long		HANDLE = 0;
	long		iDevicesConnected = 0;
	long		iGain = 0;
	int			iInput = 0, ic = 0;
	int			ivGn[10] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };

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
	void InitializeComponent();
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
	bool	SetWaveChanParms(CWaveChan* pchan);
	bool	GetWaveChanParms(CWaveChan* pchan);
	int		ConvertAbsoluteGainToIndexGain(long gain);
};
