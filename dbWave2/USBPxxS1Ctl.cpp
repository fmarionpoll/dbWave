// CUSBPxxS1Ctl.cpp  : Definition of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#include "StdAfx.h"
#include "USBPxxS1Ctl.h"





int		CUSBPxxS1Ctl::allig_Gain[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };
CString CUSBPxxS1Ctl::allig_Coupling[] = { _T("DC"), _T("AC") };
CString CUSBPxxS1Ctl::allig_ClockSource[] = { _T("Internal"), _T("External") };
CString CUSBPxxS1Ctl::allig_PClock[] = { _T("Disabled"), _T("Enabled") };
CString CUSBPxxS1Ctl::allig_LPFilterType[] = { _T("LPFT_CE"), _T("LPFT_B"), _T("LPFT_L"), _T("LPFT_HC"), _T("LPFT_LP"), _T("LPFT_HLP") };
CString CUSBPxxS1Ctl::allig_HPFilterType[] = { _T("HPFT_NONEE"), _T("HPFT_LNE"), _T("HPFT_LEE"), _T("HPFT_BNE"), _T("HPFT_BE") };

IMPLEMENT_DYNCREATE(CUSBPxxS1Ctl, CWnd)

CUSBPxxS1Ctl::CUSBPxxS1Ctl()
{
	//HRESULT hr = CoInitialize(NULL);
	//if FAILED(hr)
	//{
	//	return;
	//}

	IUSBPxxS1Ctl* pIUSBP = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_USBPxxS1Ctl, NULL, CLSCTX_INPROC_SERVER, IID_IUSBPxxS1Ctl, (void**)&pIUSBP);

	if (SUCCEEDED(hr))
	{
		TRACE("Call to open USBPIAS1 successful.\n");
		//	initialize USB
		pIUSBP->USBPxxS1Command(0, ID_INITIALIZE, 0, 0);

		VARIANT	in_val;
		VARIANT	out_val;

		in_val.lVal = 0;
		pIUSBP->USBPxxS1Command(NULL, DCID_GET_CHANNEL_HANDLE, &in_val, &out_val);
		HANDLE = out_val.lVal;

		//	read serial number
		pIUSBP->USBPxxS1Command(HANDLE, ID_READ_SERIALNUMBER, &in_val, &out_val);
		printf("Serial number = %d\n", out_val.lVal);

		//	read product ID
		pIUSBP->USBPxxS1Command(HANDLE, ID_READ_PRODUCTID, &in_val, &out_val);
		printf("Product ID = %d\n", out_val.lVal);
	}
}

CUSBPxxS1Ctl::~CUSBPxxS1Ctl()
{
	CoUninitialize();
}

void CUSBPxxS1Ctl::InitializeComponent()
{
	if (HANDLE != 0)
	{
		//	get number of devices connected
		VARIANT	in_val;
		VARIANT	out_val;

		USBPxxS1Command(HANDLE, DCID_GET_TOTAL_CONNECTED_CHANNELS, &in_val, &out_val);
		iDevicesConnected = out_val.lVal;
		TRACE("Number of Devices Connected = %d\n", iDevicesConnected);
	}
}

// CUSBPxxS1Ctl properties
//**************************************************************************************
// Read LPFc - LPFc is the low pass filter corner frequency.  The in_val is not used.
//   out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readLPFC(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_LPFC, &in_val, &out_val);
	d->LPFc = out_val.fltVal;
}

//**************************************************************************************
// Read HPFc - HPFc is the high pass filter corner frequency.  The in_val is not used.
//   out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readHPFC(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_HPFC, &in_val, &out_val);
	d->HPFc = out_val.fltVal;
}

//**************************************************************************************
// Read Gain - Gain is the gain index of an array of values where:
//		0 = 1x
//		1 = 2x
//		2 = 5x
//		3 = 10x
//		4 = 20x
//		5 = 50x
//		6 = 100x
//		7 = 200x
//		8 = 500x
//		9 = 1000x
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readGain(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_GAIN, &in_val, &out_val);
	d->indexgain = out_val.lVal;
	d->Gain = allig_Gain[d->indexgain];
}

//**************************************************************************************
// Read Coupling - Coupling is whether the AC couple circuit is engaged or not
//
//		0 = DC coupling
//		1 = AC coupling
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
// csCoupling will containe either DC or AC as a string
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readCoupling(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_COUPLING, &in_val, &out_val);
	d->indexCoupling = out_val.lVal;
	d->csCoupling = allig_Coupling[d->indexCoupling];
}

//**************************************************************************************
// Read Clock Source - The Clock Source is the control for the corner frequency of the
//			low pass filter.  For synchronization of the low pass corner frequency with
//			an external device, the filter clock can be generated by the internal clock
//			generator or an external source.
//
//		0 = Internal
//		1 = External
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//  USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readClocksource(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_CLOCKSOURCE, &in_val, &out_val);
	d->indexClockSource = out_val.lVal;
	d->csClockSource = allig_ClockSource[d->indexClockSource];
}

//**************************************************************************************
// Read PClock - The PClock is a clock output enable.  The internal clock generator can
//		be used to synchronize other external devices.  Clock frequency is a constant
//		multiple of the low pass filter clock.   Here is a list of filter characteristics
//		and the corresponding filter clock multiple:
//			Cauer Elliptic 50kHz	Fclock = 100 * Fc
//			Bessell Elliptic 66kHz	Fclock = 150 * Fc
//			Butterworth Elliptic 100kHz	Fclock = 50 * Fc
//			Cauer Elliptic 100kHz	Fclock = 50 * Fc
//			Linear Phase 100kHz	Fclock = 50 * Fc
//			Linear Phase 200kHz	Fclock = 25 * Fc
//		The PClock can be enabled or disabled.  This function reads the current status.
//		0 = Disabled
//		1 = Enabled
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//  USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readPClock(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_PCLOCK, &in_val, &out_val);
	d->indexPClock = out_val.lVal;
	d->csPClock = allig_PClock[d->indexPClock];
}
//**************************************************************************************
// Read ChannelNumber - The Channel Number is an identifier.  It does not affect the
//		function of the electronics.
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readChannelNumber(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_CHANNELNUMBER, &in_val, &out_val);
	d->ChannelNumber = out_val.lVal;
}

//**************************************************************************************
// Read description - The Description is an identifier.  It does not affect the
//		function of the electronics.  The description command is used to read a
//		string of characters one character at a time.  The index number is the input
//		value and is placed in the upper integer of the long data variable.
//		The character at the index will be returned.  The string will be NULL
//		terminated.  To retrieve all of the characters in description string,
//		send the ID_READ_DESCRIPTION until a NULL character is returned.
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readDescription(USBPxxPARAMETERS* d)
{
	auto x = 30;
	auto index = 0;
	VARIANT	in_val;
	VARIANT	out_val;

	d->Description.Empty();
	while (x--)
	{
		in_val.lVal = index++ << 16;
		USBPxxS1Command(d->DeviceHandle, ID_READ_DESCRIPTION, &in_val, &out_val);
		const auto packed_character = out_val.lVal;
		const TCHAR tc = packed_character & 0x0000ffff;
		if (tc > static_cast<TCHAR>(0))
			d->Description += tc;
		else
			x = 0;
	}
}

//**************************************************************************************
// Read Low Pass Filter Type - This identifies which filter type is installed
//		in the USBPxx-S1.
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readLowPassFilterType(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_LPFILTERTYPE, &in_val, &out_val);
	d->indexLPFilterType = out_val.lVal;
	d->csLPFilterType = allig_LPFilterType[d->indexLPFilterType];
}

//**************************************************************************************
// Read High Pass Filter Type - This identifies which filter type is installed
//		in the USBPxx-S1.
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readHighPassFilterType(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_HPFILTERTYPE, &in_val, &out_val);
	d->indexHPFilterType = out_val.lVal;
	d->csHPFilterType = allig_HPFilterType[d->indexHPFilterType];
}

//**************************************************************************************
// Read Serial Number - This number is registered with Alligator Technologies to help
//		facilitate customer service.
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readSerialNumber(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_SERIALNUMBER, &in_val, &out_val);
	d->SerialNumber = out_val.lVal;
}

//**************************************************************************************
// Read Product ID - This number is associated with the product name.
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readProductID(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_PRODUCTID, &in_val, &out_val);
	d->ProductID = out_val.lVal;
}

//**************************************************************************************
// Read Revision - This number set of numbers is the Firmware revision code.
//
//  The in_val is not used.
//  out_val will contain the returned value after the call.
//
//  The Revision is a set of numbers that is returned as a long integer.  It needs to
//	be unpacked to have meaning.  Here is structure is defined in the file
//	FirmwareInterfaceDefinition.h as REVISION_TYPE.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readRevision(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(d->DeviceHandle, ID_READ_REVISION, &in_val, &out_val);
	const auto packed = out_val.lVal;
	d->RevisionHigh = packed >> 16 & 0x0000ffff;
	d->RevisionLow = packed & 0x0000ffff;
}

//**************************************************************************************
// Read Devices Connected - This number is a count of USBPxx units connected to the USB bus.
//
//	DeviceHandle is not used.
//	The in_val is not used.
//  out_val will contain the returned value after the call.
//
//	This query asks the USB transport layer how many active units are connected
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
long CUSBPxxS1Ctl::readNumberOfDevicesConnected()
{
	VARIANT	in_val;
	VARIANT	out_val;

	USBPxxS1Command(NULL, DCID_GET_TOTAL_CONNECTED_CHANNELS, &in_val, &out_val);
	const auto packed = out_val.lVal;
	return	packed;
}

//**************************************************************************************
// Read Device Handle - The Handle is used for identifying a connected device
//
//	DeviceHandle is not used.
//	The in_val is set to Device Number.
//  out_val will contain the returned value after the call.
//
//	This query asks the USB transport layer what handle identifies the device
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************

long CUSBPxxS1Ctl::readHandleOfDevice(long device)
{
	VARIANT	in_val;
	VARIANT	out_val;

	in_val.lVal = device;
	USBPxxS1Command(NULL, DCID_GET_CHANNEL_HANDLE, &in_val, &out_val);
	const auto packed = out_val.lVal;
	return	packed;
}

bool CUSBPxxS1Ctl::readAllParameters(long device, USBPxxPARAMETERS* pUSBPxxParms)
{
	devicesConnected = readNumberOfDevicesConnected();
	pUSBPxxParms->DeviceHandle = readHandleOfDevice(device);
	if (pUSBPxxParms->DeviceHandle == NULL)
		return false;

	readLPFC(pUSBPxxParms);
	readHPFC(pUSBPxxParms);
	readGain(pUSBPxxParms);
	readCoupling(pUSBPxxParms);
	readClocksource(pUSBPxxParms);
	readPClock(pUSBPxxParms);
	readChannelNumber(pUSBPxxParms);
	readDescription(pUSBPxxParms);
	readLowPassFilterType(pUSBPxxParms);
	readHighPassFilterType(pUSBPxxParms);
	readSerialNumber(pUSBPxxParms);
	readProductID(pUSBPxxParms);
	readRevision(pUSBPxxParms);
	return true;
}

// CUSBPxxS1Ctl operations

//**************************************************************************************
// Write LPFc - LPFc is the low pass filter corner frequency.  The in_val is used to
//		send the value.  out_val will contain a status the indicates programmed success
//		or if the value was out of range and not used.
//
//	EC_SUCCESSFUL = 0  operation was a success
//	EC_LP_FC_RANGE = 1  LPFc was out of range and not used
//
// Modules where this call is useful:
//  USBPGF-S1
//	USBPBP-S1
//**************************************************************************************

void CUSBPxxS1Ctl::writeLPFC(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	in_val.fltVal = d->LPFc;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_LPFC, &in_val, &out_val);
}

//**************************************************************************************
// Write HPFc - HPFc is the high pass filter corner frequency.  The in_val is used to
//		send the value.  out_val will contain a status the indicates programmed success
//		or if the value was out of range and not used.
//
//	EC_SUCCESSFUL = 0  operation was a success
//	EC_HP_FC_RANGE = 2	high pass value was out of range
//
// Modules where this call is useful:
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************

void CUSBPxxS1Ctl::writeHPFC(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	in_val.fltVal = d->HPFc;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_HPFC, &in_val, &out_val);
}

//**************************************************************************************
// Write Gain - Gain is the gain index of an array of values where:
//		0 = 1x
//		1 = 2x
//		2 = 5x
//		3 = 10x
//		4 = 20x
//		5 = 50x
//		6 = 100x
//		7 = 200x
//		8 = 500x
//		9 = 1000x
//	The in_val is used to send the value.  out_val will contain a status that
//	indicates programmed success or if the value was out of range and not used.
//
//	EC_SUCCESSFUL = 0 operation was a success
//	EC_GAIN_RANGE = 3 gain index out of range
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************

void CUSBPxxS1Ctl::writeGainIndex(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	in_val.lVal = d->indexgain;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_GAIN, &in_val, &out_val);
}

//**************************************************************************************
// Write Coupling - Coupling is whether the AC couple circuit is engaged or not.
//
//		0 = DC coupling
//		1 = AC coupling
//
//		The in_val is used to send the value.  out_val will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail because only the lowest bit is used to set the coupling
//		switch.
//
//	EC_SUCCESSFUL = 0  operation was a success
//
// Modules where this call is useful:
//  USBPGF-S1
//	USBPIA-S1
//**************************************************************************************

void CUSBPxxS1Ctl::writeCouplingIndex(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	in_val.lVal = d->indexCoupling;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_COUPLING, &in_val, &out_val);
}

//**************************************************************************************
// Write Clock Source - The Clock Source is the control for the corner frequency of the
//			low pass filter.  For synchronization of the low pass corner frequency with
//			an external device, the filter clock can be generated by the internal clock
//			generator or an external source.
//
//		0 = Internal
//		1 = External
//
//		The in_val is used to send the value.  out_val will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail because only the lowest bit is used to set the clock source
//		switch.
//
// Modules where this call is useful:
//  USBPGF-S1
//  USBPBP-S1
//**************************************************************************************

void CUSBPxxS1Ctl::writeClockSourceIndex(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	in_val.lVal = d->indexClockSource;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_CLOCKSOURCE, &in_val, &out_val);
}

//**************************************************************************************
// Write PClock - The PClock is a clock output enable.  The internal clock generator can
//		be used to synchronize other external devices.  Clock frequency is a constant
//		multiple of the low pass filter clock.   Here is a list of filter characteristics
//		and the corresponding filter clock multiple:
//			Cauer Elliptic 50kHz	Fclock = 100 * Fc
//			Bessell Elliptic 66kHz	Fclock = 150 * Fc
//			Butterworth Elliptic 100kHz	Fclock = 50 * Fc
//			Cauer Elliptic 100kHz	Fclock = 50 * Fc
//			Linear Phase 100kHz	Fclock = 50 * Fc
//			Linear Phase 200kHz	Fclock = 25 * Fc
//		The PClock can be enabled or disabled.
//		0 = Disabled
//		1 = Enabled
//
//		The in_val is used to send the value.  out_val will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail because only the lowest bit is used to set the PClock output
//		enable switch.
//
// Modules where this call is useful:
//  USBPGF-S1
//  USBPBP-S1
//**************************************************************************************

void CUSBPxxS1Ctl::writePClockIndex(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	in_val.lVal = d->indexPClock;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_PCLOCK, &in_val, &out_val);
}

//**************************************************************************************
// Write Channel Number - The Channel Number is an identifier.  It does not affect the
//		function of the electronics.
//
//		The in_val is used to send the value.  out_val will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************

void CUSBPxxS1Ctl::writeChannelNumber(USBPxxPARAMETERS* d)
{
	VARIANT	in_val;
	VARIANT	out_val;

	in_val.lVal = d->ChannelNumber;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_CHANNELNUMBER, &in_val, &out_val);
}

//**************************************************************************************
// Write description - The Description is an identifier.  It does not affect the
//		function of the electronics.  The description command is used to write a
//		string of characters one character at a time.  Pack the offset of the character
//		relative to the beginning of the string in the data packet and issue the
//		ID_WRITE_DESCRIPTION.  Repeat this until all characters in the description
//		string are sent.
//
//		The in_val is used to send the value.  out_val will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************

void  CUSBPxxS1Ctl::writeDescription(USBPxxPARAMETERS* d)
{
	auto index = 0;
	VARIANT	in_val;
	VARIANT	out_val;

	long packed_character;
	auto x = d->Description.GetLength();
	while (x--)
	{
		const auto character = static_cast<unsigned int>(d->Description.GetAt(index));
		packed_character = index++;
		packed_character <<= 16;
		packed_character += character;
		in_val.lVal = packed_character;
		USBPxxS1Command(d->DeviceHandle, ID_WRITE_DESCRIPTION, &in_val, &out_val);
	}
	packed_character = index;
	packed_character <<= 16;
	in_val.lVal = packed_character;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_DESCRIPTION, &in_val, &out_val);
}

// dbWave-specific functions

bool CUSBPxxS1Ctl::SetWaveChanParms(CWaveChan* pchan)
{
	devicesConnected = readNumberOfDevicesConnected();
	auto* pdevice = new USBPxxPARAMETERS();
	pdevice->DeviceHandle = readHandleOfDevice(pchan->am_amplifierchan);
	if (pdevice == nullptr || pdevice->DeviceHandle == NULL)
		return false;

	pdevice->indexgain = ConvertAbsoluteGainToIndexGain(pchan->am_amplifierchan);
	writeGainIndex(pdevice);
	pdevice->HPFc = static_cast<float>(atof(CT2A(pchan->am_csInputpos)));
	writeHPFC(pdevice);
	pdevice->LPFc = pchan->am_lowpass;
	writeLPFC(pdevice);
	return true;
}

bool CUSBPxxS1Ctl::GetWaveChanParms(CWaveChan* pchan)
{
	auto* pdevice = new USBPxxPARAMETERS();
	if (!readAllParameters(pchan->am_amplifierchan, pdevice))
		return false;

	pchan->am_amplifierchan = short(pdevice->ChannelNumber);
	pchan->am_gainpre = 1;
	pchan->am_gainpost = short(pdevice->Gain);
	pchan->am_csInputpos.Format(_T("%.3f"), pdevice->HPFc);
	pchan->am_lowpass = short(pdevice->LPFc);
	return true;
}

// index functions

// gain: convert to index which is inferior of equal to the table value explored from lower to higher values
// array:	int CUSBPxxS1Ctl::allig_Gain[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };

int CUSBPxxS1Ctl::ConvertAbsoluteGainToIndexGain(long gain)
{
	int i;
	const int imax = sizeof(allig_Gain) / sizeof(int);
	for (i = 0; i < imax; i++)
	{
		if (gain <= allig_Gain[i])
			break;
	}
	return i;
}