// CUSBPxxS1Ctl.cpp  : Definition of ActiveX Control wrapper class(es) created by Microsoft Visual C++


#include "StdAfx.h"
#include "CUSBPxxS1Ctl.h"

//------------------ class USBPxxPARAMETERS ---------------------------------

IMPLEMENT_SERIAL(USBPxxPARAMETERS, CObject, 0 /* schema number*/)

USBPxxPARAMETERS::USBPxxPARAMETERS()
{
	DeviceHandle = NULL;
	LPFc = 0;
	HPFc = 3000;
	indexgain = 0;
	indexCoupling = 0;
	indexClockSource = 0;
	indexPClock = 0;
	ChannelNumber = 0;
	indexLPFilterType = 0;
	indexHPFilterType = 0;
	SerialNumber = 0;
	ProductID = 0;
	RevisionHigh = 0;
	RevisionLow = 0;
	Gain = 1;
}

USBPxxPARAMETERS::~USBPxxPARAMETERS()
{
}

void USBPxxPARAMETERS::operator=(const USBPxxPARAMETERS & arg)
{
	DeviceHandle = arg.DeviceHandle;
	LPFc = arg.LPFc;
	HPFc = arg.HPFc;
	indexgain = arg.indexgain;
	indexCoupling = arg.indexCoupling;
	indexClockSource = arg.indexClockSource;
	indexPClock = arg.indexPClock;
	ChannelNumber = arg.ChannelNumber;
	indexLPFilterType = arg.indexLPFilterType;
	indexHPFilterType = arg.indexHPFilterType;
	SerialNumber = arg.SerialNumber;
	ProductID = arg.ProductID;
	RevisionHigh = arg.RevisionHigh;
	RevisionLow = arg.RevisionLow;
	Description = arg.Description;
	csCoupling = arg.csCoupling;
	csClockSource = arg.csClockSource;
	csPClock = arg.csPClock;
	csLPFilterType = arg.csLPFilterType;
	csHPFilterType = arg.csHPFilterType;
}

long USBPxxPARAMETERS::Write(CFile * datafile)
{
	ULONGLONG p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	ULONGLONG p2 = datafile->GetPosition();
	return (long)(p2 - p1);
}

BOOL USBPxxPARAMETERS::Read(CFile * datafile)
{
	CArchive ar(datafile, CArchive::load);
	BOOL flag = TRUE;
	try
	{
		Serialize(ar);
	}
	catch (CException* e)
	{
		e->Delete();
		flag = FALSE;
	}
	ar.Close();
	return flag;
}

void USBPxxPARAMETERS::Serialize(CArchive & ar)
{
	if (ar.IsStoring())
	{
		WORD wversion = 1;
		ar << wversion;

		int nitems = 2; ar << nitems;
		ar << LPFc;
		ar << HPFc;

		nitems = 10; ar << nitems;
		ar << DeviceHandle;
		ar << indexgain;
		ar << indexCoupling;
		ar << indexClockSource;
		ar << indexPClock;
		ar << ChannelNumber;
		ar << indexLPFilterType;
		ar << indexHPFilterType;
		ar << SerialNumber;
		ar << ProductID;

		nitems = 2; ar << nitems; 
		ar << RevisionHigh;
		ar << RevisionLow;

		nitems = 6; ar << nitems;
		ar << Description;
		ar << csCoupling;
		ar << csClockSource ;
		ar << csPClock;
		ar << csLPFilterType;
		ar << csHPFilterType;
	}
	else
	{
		WORD wversion;
		ar >> wversion;
		ASSERT(wversion == 1);

		int nitems;  ar >> nitems;
		if (nitems > 0) ar >> LPFc; nitems--;
		if (nitems > 0) ar >> HPFc; nitems--;

		ar >> nitems;
		if (nitems > 0) ar >> DeviceHandle; nitems--;
		if (nitems > 0) ar >> indexgain; nitems--;
		if (nitems > 0) ar >> indexCoupling; nitems--;
		if (nitems > 0) ar >> indexClockSource; nitems--;
		if (nitems > 0) ar >> indexPClock; nitems--;
		if (nitems > 0) ar >> ChannelNumber; nitems--;
		if (nitems > 0) ar >> indexLPFilterType; nitems--;
		if (nitems > 0) ar >> indexHPFilterType; nitems--;
		if (nitems > 0) ar >> SerialNumber; nitems--;
		if (nitems > 0) ar >> ProductID; nitems--;

		ar >> nitems;
		if (nitems > 0) ar >> RevisionHigh; nitems--;
		if (nitems > 0) ar >> RevisionLow; nitems--;

		ar >> nitems;
		if (nitems > 0) ar >> Description; nitems--;
		if (nitems > 0) ar >> csCoupling; nitems--;
		if (nitems > 0) ar >> csClockSource; nitems--;
		if (nitems > 0) ar >> csPClock; nitems--;
		if (nitems > 0) ar >> csLPFilterType; nitems--;
		if (nitems > 0) ar >> csHPFilterType; nitems--;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CUSBPxxS1Ctl

int		CUSBPxxS1Ctl::allig_Gain[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };
CString CUSBPxxS1Ctl::allig_Coupling[] = { _T("DC"), _T("AC") };
CString CUSBPxxS1Ctl::allig_ClockSource[] = { _T("Internal"), _T("External") };
CString CUSBPxxS1Ctl::allig_PClock[] = { _T("Disabled"), _T("Enabled") };
CString CUSBPxxS1Ctl::allig_LPFilterType[] = { _T("LPFT_CE"), _T("LPFT_B"), _T("LPFT_L"), _T("LPFT_HC"), _T("LPFT_LP"), _T("LPFT_HLP") };
CString CUSBPxxS1Ctl::allig_HPFilterType[] = { _T("HPFT_NONEE"), _T("HPFT_LNE"), _T("HPFT_LEE"), _T("HPFT_BNE"), _T("HPFT_BE") };

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
	d->indexgain = OutVal.lVal;
	d->Gain = allig_Gain[d->indexgain];
}

//**************************************************************************************
// Read Coupling - Coupling is whether the AC couple circuit is engaged or not
//
//		0 = DC coupling
//		1 = AC coupling
//
//  The InVal is not used.
//  OutVal will contain the returned value after the call.
// csCoupling will containe either DC or AC as a string
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
	d->indexCoupling = OutVal.lVal;
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
	d->indexClockSource = OutVal.lVal;
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
	d->indexPClock = OutVal.lVal;
	d->csPClock = allig_PClock[d->indexPClock];
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
	d->indexLPFilterType = OutVal.lVal;
	d->csLPFilterType = allig_LPFilterType[d->indexLPFilterType];
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
	d->indexHPFilterType = OutVal.lVal;
	d->csHPFilterType = allig_HPFilterType[d->indexHPFilterType];
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

void CUSBPxxS1Ctl::readAllParameters(long handle, USBPxxPARAMETERS* pUSBPxxParms)
{
	//pUSBPxxParms->DeviceHandle = readHandleOfDevice(device);
	pUSBPxxParms->DeviceHandle = handle;

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
	//deviceNumber = 0;
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
void CUSBPxxS1Ctl::writeGainIndex (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->indexgain;
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
void CUSBPxxS1Ctl::writeCouplingIndex (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->indexCoupling;
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
void CUSBPxxS1Ctl::writeClockSourceIndex (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->indexClockSource;
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
void CUSBPxxS1Ctl::writePClockIndex (USBPxxPARAMETERS *d)
{
	VARIANT	InVal;
	VARIANT	OutVal;

	InVal.lVal = d->indexPClock;
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


// dbWave-specific functions

BOOL CUSBPxxS1Ctl::SetWaveChanParms(CWaveChan * pchan, USBPxxPARAMETERS* pdevice)
{
	//pdevice->DeviceHandle = readHandleOfDevice(pchan->am_amplifierchan);

	if (pdevice == nullptr || pdevice->DeviceHandle == NULL)
		return FALSE;
	pdevice->ChannelNumber = pchan->am_amplifierchan;
	
	pdevice->indexgain = ConvertAbsoluteGainToIndexGain(pchan->am_amplifierchan);
	writeGainIndex(pdevice);
	pdevice->HPFc = (float) atof(CT2A(pchan->am_csInputpos));
	writeHPFC(pdevice);
	pdevice->LPFc = pchan->am_lowpass;
	writeLPFC(pdevice);
	return TRUE;
}

BOOL CUSBPxxS1Ctl::GetWaveChanParms(CWaveChan * pchan, USBPxxPARAMETERS* pdevice)
{
	if (pdevice == nullptr || pdevice->DeviceHandle == NULL)
		return FALSE;

	readAllParameters(pdevice->DeviceHandle, pdevice);
	pchan->am_amplifierchan = short(pdevice->ChannelNumber);
	pchan->am_gainpre = 1;
	pchan->am_gainpost = short(pdevice->Gain);
	pchan->am_csInputpos.Format(_T("%.3f"), pdevice->HPFc);
	pchan->am_lowpass = short(pdevice->LPFc);
	return TRUE;
}

// index functions

// gain: convert to index which is inferior of equal to the table value explored from lower to higher values
// array:	int CUSBPxxS1Ctl::allig_Gain[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };

int CUSBPxxS1Ctl::ConvertAbsoluteGainToIndexGain(long gain)
{
	int i;
	int imax = sizeof(allig_Gain)/sizeof(int);
	for (i = 0; i < imax; i++)
	{
		if (gain <= allig_Gain[i])
			break;
	}
	return i;
}
