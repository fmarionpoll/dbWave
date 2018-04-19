// CUSBPxxS1Ctl.cpp  : Definition of ActiveX Control wrapper class(es) created by Microsoft Visual C++


#include "stdafx.h"
#include "CUSBPxxS1Ctl.h"


/////////////////////////////////////////////////////////////////////////////
// CUSBPxxS1Ctl

IMPLEMENT_DYNCREATE(CUSBPxxS1Ctl, CWnd)



// CUSBPxxS1Ctl properties
//**************************************************************************************
// Read LPFc - LPFc is the low pass filter corner frequency.  The InVal is not used.
//   OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readLPFC(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_LPFC, &InVal, &OutVal);
	d->LPFc = OutVal.fltVal;
}

//**************************************************************************************
// Read HPFc - HPFc is the high pass filter corner frequency.  The InVal is not used.
//   OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readHPFC(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_HPFC, &InVal, &OutVal);
	d->HPFc = OutVal.fltVal;
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
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readGain(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_GAIN, &InVal, &OutVal);
	d->gain = OutVal.lVal;
}

//**************************************************************************************
// Read Coupling - Coupling is whether the AC couple circuit is engaged or not
//
//		0 = DC coupling
//		1 = AC coupling
//
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readCoupling(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_COUPLING, &InVal, &OutVal);
	d->coupling = OutVal.lVal;
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
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//  USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readClocksource(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_CLOCKSOURCE, &InVal, &OutVal);
	d->ClockSource = OutVal.lVal;
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
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//  USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readPClock(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_PCLOCK, &InVal, &OutVal);
	d->PClock = OutVal.lVal;
}
//**************************************************************************************
// Read ChannelNumber - The Channel Number is an identifier.  It does not affect the 
//		function of the electronics.
//
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readChannelNumber(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_CHANNELNUMBER, &InVal, &OutVal);
	d->ChannelNumber = OutVal.lVal;
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
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readDescription(USBPxxPARAMETERS *d)
{
	int	x = 30;
	int index = 0;
	long	PackedCharacter;
	TCHAR	tc;
	VARIANT	InVal;
	VARIANT	OutVal;

	d->Description.Empty();
	while (x--)
	{
		InVal.lVal = index++ << 16;
		USBPxxS1Command(d->DeviceHandle, ID_READ_DESCRIPTION, &InVal, &OutVal);
		PackedCharacter = OutVal.lVal;
		tc = PackedCharacter & 0x0000ffff;
		if (tc > (TCHAR)0)
			d->Description += tc;
		else
			x = 0;
	}
}

//**************************************************************************************
// Read Low Pass Filter Type - This identifies which filter type is installed
//		in the USBPxx-S1.
//
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readLowPassFilterType(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_LPFILTERTYPE, &InVal, &OutVal);
	d->LPFilterType = OutVal.lVal;
}

//**************************************************************************************
// Read High Pass Filter Type - This identifies which filter type is installed
//		in the USBPxx-S1.
//
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readHighPassFilterType(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_HPFILTERTYPE, &InVal, &OutVal);
	d->HPFilterType = OutVal.lVal;
}

//**************************************************************************************
// Read Serial Number - This number is registered with Alligator Technologies to help
//		facilitate customer service.
//
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readSerialNumber(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_SERIALNUMBER, &InVal, &OutVal);
	d->SerialNumber = OutVal.lVal;
}

//**************************************************************************************
// Read Product ID - This number is associated with the product name.
//
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::readProductID(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	USBPxxS1Command(d->DeviceHandle, ID_READ_PRODUCTID, &InVal, &OutVal);
	d->ProductID = OutVal.lVal;
}

//**************************************************************************************
// Read Revision - This number set of numbers is the Firmware revision code.
//
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
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
void CUSBPxxS1Ctl::readRevision(USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;
	long	packed;

	USBPxxS1Command(d->DeviceHandle, ID_READ_REVISION, &InVal, &OutVal);
	packed = OutVal.lVal;
	d->RevisionHigh = packed >> 16 & 0x0000ffff;
	d->RevisionLow = packed & 0x0000ffff;
}

//**************************************************************************************
// Read Devices Connected - This number is a count of USBPxx units connected to the USB bus.
//
//	DeviceHandle is not used.
//	The InVal is not used.
//  OutVal will contain the returned value after the call.
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
	VARIANT	InVal;
	VARIANT	OutVal;
	long	packed;

	USBPxxS1Command(NULL, DCID_GET_TOTAL_CONNECTED_CHANNELS, &InVal, &OutVal);
	packed = OutVal.lVal;
	return	packed;
}

//**************************************************************************************
// Read Device Handle - The Handle is used for identifying a connected device
//
//	DeviceHandle is not used.
//	The InVal is set to Device Number.
//  OutVal will contain the returned value after the call.
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
	VARIANT	InVal;
	VARIANT	OutVal;
	long	packed;

	InVal.lVal = device;
	USBPxxS1Command(NULL, DCID_GET_CHANNEL_HANDLE, &InVal, &OutVal);
	packed = OutVal.lVal;
	return	packed;
}

void CUSBPxxS1Ctl::readAllParameters(long device, USBPxxPARAMETERS* pUSBPxxParms)
{
	pUSBPxxParms->DeviceHandle = readHandleOfDevice(device);

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
	devicesConnected = readNumberOfDevicesConnected();
	deviceNumber = 0;

}

// CUSBPxxS1Ctl operations

//**************************************************************************************
// Write LPFc - LPFc is the low pass filter corner frequency.  The InVal is used to
//		send the value.  OutVal will contain a status the indicates programmed success
//		or if the value was out of range and not used.
//
//	EC_SUCCESSFUL = 0  operation was a success
//	EC_LP_FC_RANGE = 1  LPFc was out of range and not used
//
// Modules where this call is useful:
//  USBPGF-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::writeLPFC (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.fltVal = d->LPFc;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_LPFC, &InVal, &OutVal);
}

//**************************************************************************************
// Write HPFc - HPFc is the high pass filter corner frequency.  The InVal is used to
//		send the value.  OutVal will contain a status the indicates programmed success
//		or if the value was out of range and not used.
//
//	EC_SUCCESSFUL = 0  operation was a success
//	EC_HP_FC_RANGE = 2	high pass value was out of range
//
// Modules where this call is useful:
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::writeHPFC (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.fltVal = d->HPFc;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_HPFC, &InVal, &OutVal);
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
//	The InVal is used to send the value.  OutVal will contain a status that
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
void CUSBPxxS1Ctl::writeGain (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->gain;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_GAIN,  &InVal, &OutVal);
}

//**************************************************************************************
// Write Coupling - Coupling is whether the AC couple circuit is engaged or not.
//
//		0 = DC coupling
//		1 = AC coupling
//
//		The InVal is used to send the value.  OutVal will contain a status the indicates
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
void CUSBPxxS1Ctl::writeCoupling (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->coupling;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_COUPLING,  &InVal, &OutVal);
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
//		The InVal is used to send the value.  OutVal will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail because only the lowest bit is used to set the clock source
//		switch.
//
// Modules where this call is useful:
//  USBPGF-S1
//  USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::writeClockSource (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->ClockSource;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_CLOCKSOURCE,  &InVal, &OutVal);
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
//		The InVal is used to send the value.  OutVal will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail because only the lowest bit is used to set the PClock output
//		enable switch.
//
// Modules where this call is useful:
//  USBPGF-S1
//  USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::writePClock (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->PClock;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_PCLOCK,  &InVal, &OutVal);
}

//**************************************************************************************
// Write Channel Number - The Channel Number is an identifier.  It does not affect the
//		function of the electronics.
//
//		The InVal is used to send the value.  OutVal will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1Ctl::writeChannelNumber (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->ChannelNumber;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_CHANNELNUMBER,  &InVal, &OutVal);
}

//**************************************************************************************
// Write description - The Description is an identifier.  It does not affect the
//		function of the electronics.  The description command is used to write a
//		string of characters one character at a time.  Pack the offset of the character
//		relative to the beginning of the string in the data packet and issue the
//		ID_WRITE_DESCRIPTION.  Repeat this until all characters in the description
//		string are sent.
//
//		The InVal is used to send the value.  OutVal will contain a status the indicates
//		programmed success or if the value was out of range and not used.  This
//		function cannot fail.
//
// Modules where this call is useful:
//  USBPIA-S1
//  USBPGF-S1
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void  CUSBPxxS1Ctl::writeDescription(USBPxxPARAMETERS *d)
{
	int	x = USBPxxS1_DESCRIPTION_SIZE_LIMIT;
	int index = 0;
	long	PackedCharacter;
	unsigned int	Character;
	VARIANT	InVal;
	VARIANT	OutVal;

	PackedCharacter = 0;
	x = d->Description.GetLength();
	while(x--)
	{
		Character = (unsigned int)d->Description.GetAt(index);
		PackedCharacter = index++;
		PackedCharacter <<= 16;
		PackedCharacter += Character;
		InVal.lVal = PackedCharacter;
		USBPxxS1Command(d->DeviceHandle, ID_WRITE_DESCRIPTION,  &InVal, &OutVal);
	}
	PackedCharacter = index;
	PackedCharacter <<= 16;
	InVal.lVal = PackedCharacter;
	USBPxxS1Command(d->DeviceHandle, ID_WRITE_DESCRIPTION,  &InVal, &OutVal);
}

void CUSBPxxS1Ctl::SetWaveChanParms(CWaveChan * pchan)
{
	USBPxxPARAMETERS device;

	device.ChannelNumber = pchan->am_amplifierchan;
	device.DeviceHandle = readHandleOfDevice(pchan->am_amplifierchan);

	device.gain = pchan->am_amplifierchan;
	writeGain(&device);
	device.HPFc = atof(CT2A(pchan->am_csInputpos));
	writeHPFC(&device);
	device.LPFc = pchan->am_lowpass;
	writeLPFC(&device);
}

void CUSBPxxS1Ctl::GetWaveChanParms(CWaveChan * pchan)
{
	USBPxxPARAMETERS device;
	readAllParameters(0, &device);
	pchan->am_amplifierchan = short(device.ChannelNumber);
	pchan->am_amplifierchan = short(device.gain);
	pchan->am_csInputpos.Format(_T("%f.3"), device.HPFc);
	pchan->am_lowpass = short(device.LPFc);
}
