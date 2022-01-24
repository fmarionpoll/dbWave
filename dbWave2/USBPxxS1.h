// CUSBPxxS1.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once
#include <objbase.h>
#include "ImportAlligatorDefinitions.h"
#include "USBPxxPARAMETERS.h"
//#import "./include/Alligator/win32/USBPxxS1COM.dll" 
#import "USBPxxS1COM.dll" \
		no_namespace raw_interfaces_only raw_native_types named_guids


class CUSBPxxS1 : public CObject
{
public:
	CUSBPxxS1();
	~CUSBPxxS1() override;

protected:
	// Attributes
	long		devicesConnected	= 0;
	long		HANDLE				= 0;
	long		iDevicesConnected	= 0;
	USBPxxPARAMETERS m_USBP{};

	IUSBPxxS1Ctl*	m_pIUSBP = nullptr;

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
	HRESULT Initialize();
	int		GetAlligatorNumberConnected(CString& message);
	long	GetAlligatorFirstAvailableHandle(CString& message);
	bool	ConnectToFirstAlligator();

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
