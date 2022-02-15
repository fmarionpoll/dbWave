#pragma once

#include <vector>
#include "AcqWaveChan.h"
#include "ImportAlligatorDefinitions.h"
#include "USBPxxPARAMETERS.h"
#import "USBPxxS1COM.dll" no_namespace raw_interfaces_only raw_native_types named_guids


class CUSBPxxS1 : public CObject
{
public:
	CUSBPxxS1();
	~CUSBPxxS1() override;

protected:
	// Attributes
	long devicesConnected = 0;
	long iDevicesConnected = 0;
	USBPxxPARAMETERS m_USBP{};
	IUSBPxxS1Ctl* m_pIUSBP = nullptr;

	// constants
private:
	static int allig_Gain[];
	static CString allig_Coupling[];
	static CString allig_ClockSource[];
	static CString allig_PClock[];
	static CString allig_LPFilterType[];
	static CString allig_HPFilterType[];

	// functions
public:
	HRESULT Initialize();
	int GetAlligatorNumberConnected(CString& message);
	long GetAlligatorFirstAvailableHandle(CString& message);
	bool ConnectToFirstAlligator();
	bool IsValidHandle();

	void readLPFC(USBPxxPARAMETERS* pUSBP);
	void readHPFC(USBPxxPARAMETERS* pUSBP);
	void readGain(USBPxxPARAMETERS* pUSBP);
	void readCoupling(USBPxxPARAMETERS* pUSBP);
	void readClocksource(USBPxxPARAMETERS* pUSBP);
	void readPClock(USBPxxPARAMETERS* pUSBP);
	void readChannelNumber(USBPxxPARAMETERS* pUSBP);
	void readDescription(USBPxxPARAMETERS* pUSBP);
	void readLowPassFilterType(USBPxxPARAMETERS* pUSBP);
	void readHighPassFilterType(USBPxxPARAMETERS* pUSBP);
	void readSerialNumber(USBPxxPARAMETERS* pUSBP);
	void readProductID(USBPxxPARAMETERS* pUSBP);
	void readRevision(USBPxxPARAMETERS* pUSBP);
	long readNumberOfDevicesConnected();
	long readHandleOfDevice(long device);
	bool readAllParameters(USBPxxPARAMETERS* pUSBP);

	void writeLPFC(USBPxxPARAMETERS* pUSBP);
	void writeHPFC(USBPxxPARAMETERS* pUSBP);
	void writeGainIndex(USBPxxPARAMETERS* pUSBP);
	void writeCouplingIndex(USBPxxPARAMETERS* pUSBP);
	void writeClockSourceIndex(USBPxxPARAMETERS* pUSBP);
	void writePClockIndex(USBPxxPARAMETERS* pUSBP);
	void writeChannelNumber(USBPxxPARAMETERS* pUSBP);
	void writeDescription(USBPxxPARAMETERS* pUSBP);

	void writeGainIndex(int gainIndex);
	void writeHPFC(CString highPass);
	void writeLPFC(int lowPass);

	// dbWave-specific functions
	bool SetWaveChanParms(CWaveChan* pchan);
	bool GetWaveChanParms(CWaveChan* pchan);
	int ConvertAbsoluteGainToIndexGain(int gain);
	std::vector<int> GetGainsAvailable();
};
