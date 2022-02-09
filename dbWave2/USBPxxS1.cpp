#include "StdAfx.h"
#include "USBPxxS1.h"

#include "AcqWaveChan.h"


int CUSBPxxS1::allig_Gain[] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000};
CString CUSBPxxS1::allig_Coupling[] = {_T("DC"), _T("AC")};
CString CUSBPxxS1::allig_ClockSource[] = {_T("Internal"), _T("External")};
CString CUSBPxxS1::allig_PClock[] = {_T("Disabled"), _T("Enabled")};
CString CUSBPxxS1::allig_LPFilterType[] = {
	_T("LPFT_CE"), _T("LPFT_B"), _T("LPFT_L"), _T("LPFT_HC"), _T("LPFT_LP"), _T("LPFT_HLP")
};
CString CUSBPxxS1::allig_HPFilterType[] = {
	_T("HPFT_NONEE"), _T("HPFT_LNE"), _T("HPFT_LEE"), _T("HPFT_BNE"), _T("HPFT_BE")
};


CUSBPxxS1::CUSBPxxS1()
{
	HRESULT hr = CoInitialize(nullptr);
	//TRACE("initialize co HR=%i \n", hr);
}

CUSBPxxS1::~CUSBPxxS1()
{
	if (m_pIUSBP)
	{
		m_pIUSBP->Release();
		m_pIUSBP = nullptr;
	}
	CoUninitialize();
}

HRESULT CUSBPxxS1::Initialize()
{
	HRESULT hr = CoCreateInstance(CLSID_USBPxxS1Ctl, nullptr, CLSCTX_INPROC_SERVER,
	                              IID_IUSBPxxS1Ctl, reinterpret_cast<void**>(&m_pIUSBP));
	//if (SUCCEEDED(hr))
	//{
	//	//TRACE("create instance HR=%i \n", hr);
	//}
	//else
	//{
	//	//TRACE("create instance HR=%i -- error \n", hr);
	//}
	return hr;
}

int CUSBPxxS1::GetAlligatorNumberConnected(CString& message)
{
	VARIANT in_val;
	VARIANT out_val;
	m_pIUSBP->USBPxxS1Command(0, ID_INITIALIZE, nullptr, nullptr);
	m_pIUSBP->USBPxxS1Command(NULL, DCID_GET_TOTAL_CONNECTED_CHANNELS, &in_val, &out_val);
	iDevicesConnected = static_cast<int>(out_val.lVal);
	CString out;
	out.Format(_T("Number of Devices Connected = %d\n"), iDevicesConnected);
	message += out;
	return iDevicesConnected;
}

long CUSBPxxS1::GetAlligatorFirstAvailableHandle(CString& message)
{
	VARIANT in_val;
	VARIANT out_val;
	long handle = 0;
	for (int ic = 0; ic < 10; ic++)
	{
		in_val.lVal = ic;
		m_pIUSBP->USBPxxS1Command(NULL, DCID_GET_CHANNEL_HANDLE, &in_val, &out_val);
		handle = out_val.lVal;
		if (handle > 0)
		{
			CString out;
			out.Format(_T("Device %d handle = %d\n"), ic, handle);
			message += out;
			break;
		}
	}
	return handle;
}

bool CUSBPxxS1::ConnectToFirstAlligator()
{
	HRESULT hr = Initialize();
	CString message;
	if (SUCCEEDED(hr))
	{
		const int iDevices = GetAlligatorNumberConnected(message);
		if (iDevices)
			m_USBP.DeviceHandle = GetAlligatorFirstAvailableHandle(message);
	}
	return SUCCEEDED(hr);
}

bool CUSBPxxS1::IsValidHandle()
{
	if (m_USBP.DeviceHandle == 0)
	{
		ConnectToFirstAlligator();
	}
	return (m_USBP.DeviceHandle > 0);
}

// CUSBPxxS1 properties
//**************************************************************************************
// Read LPFc - LPFc is the low pass filter corner frequency.  The in_val is not used.
//   out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPGF-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1::readLPFC(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_LPFC, &in_val, &out_val);
	pUSBP->LPFc = out_val.fltVal;
	//TRACE(_T("read LPFC =%f \n"), pUSBP->LPFc);
}

//**************************************************************************************
// Read HPFc - HPFc is the high pass filter corner frequency.  The in_val is not used.
//   out_val will contain the returned value after the call.
//
// Modules where this call is useful:
//  USBPHP-S1
//	USBPBP-S1
//**************************************************************************************
void CUSBPxxS1::readHPFC(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_HPFC, &in_val, &out_val);
	pUSBP->HPFc = out_val.fltVal;
	//TRACE(_T("read HPFC =%f \n"), pUSBP->HPFc);
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
void CUSBPxxS1::readGain(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_GAIN, &in_val, &out_val);
	pUSBP->indexgain = out_val.lVal;
	pUSBP->Gain = allig_Gain[pUSBP->indexgain];
	//TRACE(_T("read Gain =%i [%i] \n"), pUSBP->Gain, pUSBP->indexgain);
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
void CUSBPxxS1::readCoupling(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_COUPLING, &in_val, &out_val);
	pUSBP->indexCoupling = out_val.lVal;
	pUSBP->csCoupling = allig_Coupling[pUSBP->indexCoupling];
	//TRACE(_T("read coupling =%s \n"), pUSBP->csCoupling);
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
void CUSBPxxS1::readClocksource(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_CLOCKSOURCE, &in_val, &out_val);
	pUSBP->indexClockSource = out_val.lVal;
	pUSBP->csClockSource = allig_ClockSource[pUSBP->indexClockSource];
	//TRACE(_T("read clock source =%s \n"), pUSBP->csClockSource);
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
void CUSBPxxS1::readPClock(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_PCLOCK, &in_val, &out_val);
	pUSBP->indexPClock = out_val.lVal;
	pUSBP->csPClock = allig_PClock[pUSBP->indexPClock];
	//TRACE(_T("read P Clock =%s \n"), pUSBP->indexPClock);
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
void CUSBPxxS1::readChannelNumber(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_CHANNELNUMBER, &in_val, &out_val);
	pUSBP->ChannelNumber = out_val.lVal;

	//TRACE(_T("read channel number =%i \n"), pUSBP->ChannelNumber);
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
void CUSBPxxS1::readDescription(USBPxxPARAMETERS* pUSBP)
{
	auto x = 30;
	auto index = 0;
	VARIANT in_val;
	VARIANT out_val;

	pUSBP->Description.Empty();
	while (x--)
	{
		in_val.lVal = index++ << 16;
		m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_DESCRIPTION, &in_val, &out_val);
		const auto packed_character = out_val.lVal;
		const TCHAR tc = packed_character & 0x0000ffff;
		if (tc > static_cast<TCHAR>(0))
			pUSBP->Description += tc;
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
void CUSBPxxS1::readLowPassFilterType(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_LPFILTERTYPE, &in_val, &out_val);
	pUSBP->indexLPFilterType = out_val.lVal;
	pUSBP->csLPFilterType = allig_LPFilterType[pUSBP->indexLPFilterType];
	//TRACE(_T("read low pass filter type =%s \n"), pUSBP->csLPFilterType);
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
void CUSBPxxS1::readHighPassFilterType(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_HPFILTERTYPE, &in_val, &out_val);
	pUSBP->indexHPFilterType = out_val.lVal;
	pUSBP->csHPFilterType = allig_HPFilterType[pUSBP->indexHPFilterType];
	//TRACE(_T("read high pass filter type =%s \n"), pUSBP->csHPFilterType);
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
void CUSBPxxS1::readSerialNumber(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_SERIALNUMBER, &in_val, &out_val);
	pUSBP->SerialNumber = out_val.lVal;
	//TRACE(_T("read serial number =%i \n"), pUSBP->SerialNumber);
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
void CUSBPxxS1::readProductID(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_PRODUCTID, &in_val, &out_val);
	pUSBP->ProductID = out_val.lVal;
	//TRACE(_T("read product ID =%i \n"), pUSBP->ProductID);
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
void CUSBPxxS1::readRevision(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_READ_REVISION, &in_val, &out_val);
	const auto packed = out_val.lVal;
	pUSBP->RevisionHigh = packed >> 16 & 0x0000ffff;
	pUSBP->RevisionLow = packed & 0x0000ffff;
	//TRACE(_T("read Revision =%i %i \n"), pUSBP->RevisionHigh, pUSBP->RevisionLow);
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
long CUSBPxxS1::readNumberOfDevicesConnected()
{
	VARIANT in_val;
	VARIANT out_val;

	m_pIUSBP->USBPxxS1Command(NULL, DCID_GET_TOTAL_CONNECTED_CHANNELS, &in_val, &out_val);
	const auto packed = out_val.lVal;
	return packed;
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

long CUSBPxxS1::readHandleOfDevice(long device)
{
	VARIANT in_val;
	VARIANT out_val;

	in_val.lVal = device;
	m_pIUSBP->USBPxxS1Command(NULL, DCID_GET_CHANNEL_HANDLE, &in_val, &out_val);
	const auto packed = out_val.lVal;
	//TRACE(_T("read handle =%i \n"), packed);
	return packed;
}

bool CUSBPxxS1::readAllParameters(USBPxxPARAMETERS* pUSBPxxParms)
{
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

// CUSBPxxS1 operations

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

void CUSBPxxS1::writeLPFC(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	in_val.fltVal = pUSBP->LPFc;
	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_LPFC, &in_val, &out_val);
	//TRACE(_T("write LPFC =%f \n"), pUSBP->LPFc);
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

void CUSBPxxS1::writeHPFC(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	in_val.fltVal = pUSBP->HPFc;
	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_HPFC, &in_val, &out_val);
	//TRACE(_T("write HPFC =%f \n"), pUSBP->HPFc);
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

void CUSBPxxS1::writeGainIndex(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	in_val.lVal = pUSBP->indexgain;
	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_GAIN, &in_val, &out_val);
	//TRACE(_T("write gain index =%i \n"), in_val.lVal);
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

void CUSBPxxS1::writeCouplingIndex(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	in_val.lVal = pUSBP->indexCoupling;
	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_COUPLING, &in_val, &out_val);
	//TRACE(_T("write coupling index =%i \n"), in_val.lVal);
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

void CUSBPxxS1::writeClockSourceIndex(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	in_val.lVal = pUSBP->indexClockSource;
	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_CLOCKSOURCE, &in_val, &out_val);
	//TRACE(_T("write clock source index =%i \n"), in_val.lVal);
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

void CUSBPxxS1::writePClockIndex(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	in_val.lVal = pUSBP->indexPClock;
	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_PCLOCK, &in_val, &out_val);
	//TRACE(_T("write clock index =%i \n"), in_val.lVal);
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

void CUSBPxxS1::writeChannelNumber(USBPxxPARAMETERS* pUSBP)
{
	VARIANT in_val;
	VARIANT out_val;

	in_val.lVal = pUSBP->ChannelNumber;
	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_CHANNELNUMBER, &in_val, &out_val);
	//TRACE(_T("write channel number =%i \n"), in_val.lVal);
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

void CUSBPxxS1::writeDescription(USBPxxPARAMETERS* pUSBP)
{
	auto index = 0;
	VARIANT in_val;
	VARIANT out_val;

	long packed_character;
	auto x = pUSBP->Description.GetLength();
	while (x--)
	{
		const auto character = static_cast<unsigned int>(pUSBP->Description.GetAt(index));
		packed_character = index++;
		packed_character <<= 16;
		packed_character += character;
		in_val.lVal = packed_character;
		m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_DESCRIPTION, &in_val, &out_val);
	}
	packed_character = index;
	packed_character <<= 16;
	in_val.lVal = packed_character;
	m_pIUSBP->USBPxxS1Command(pUSBP->DeviceHandle, ID_WRITE_DESCRIPTION, &in_val, &out_val);
}

void CUSBPxxS1::writeGainIndex(int gainIndex)
{
	m_USBP.indexgain = gainIndex;
	m_USBP.Gain = allig_Gain[m_USBP.indexgain];
	writeGainIndex(&m_USBP);
}

void CUSBPxxS1::writeHPFC(CString highPass)
{
	m_USBP.HPFc = static_cast<float>(atof(CT2A(highPass)));
	writeHPFC(&m_USBP);
}

void CUSBPxxS1::writeLPFC(int lowPass)
{
	m_USBP.LPFc = static_cast<float>(lowPass);
	writeLPFC(&m_USBP);
}

// dbWave-specific functions

bool CUSBPxxS1::SetWaveChanParms(CWaveChan* pchan)
{
	if (m_USBP.DeviceHandle == 0)
		return false;

	writeGainIndex(ConvertAbsoluteGainToIndexGain(pchan->am_gainpre));
	writeLPFC(pchan->am_lowpass);
	writeHPFC(pchan->am_csInputpos);

	return true;
}

bool CUSBPxxS1::GetWaveChanParms(CWaveChan* pchan)
{
	if (m_USBP.DeviceHandle == 0)
		return false;

	readAllParameters(&m_USBP);
	pchan->am_amplifierchan = static_cast<short>(m_USBP.ChannelNumber);
	pchan->am_gainpre = static_cast<short>(m_USBP.Gain);
	pchan->am_gainpost = 1;
	pchan->am_csInputpos.Format(_T("%.3f \n"), m_USBP.HPFc);
	pchan->am_lowpass = static_cast<short>(m_USBP.LPFc);
	return true;
}

// index functions

// gain: convert to index which is inferior of equal to the table value explored from lower to higher values
// array:	int CUSBPxxS1::allig_Gain[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };

int CUSBPxxS1::ConvertAbsoluteGainToIndexGain(int gain)
{
	int i;
	constexpr int max = std::size(allig_Gain);
	for (i = 0; i < max; i++)
	{
		if (gain <= allig_Gain[i])
			break;
	}
	return i;
}

std::vector<int> CUSBPxxS1::GetGainsAvailable()
{
	std::vector<int> numbers(allig_Gain, allig_Gain + sizeof(allig_Gain) / sizeof(int));
	return numbers;
}
