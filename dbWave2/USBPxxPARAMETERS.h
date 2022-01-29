#pragma once

//**************************************************************************************
// All parameters that can be programmed in an individual USBPxxS1 are in the structure
// typedef listed below.  Even though each module type may not have the hardware to
// support the function, all module types firmware is the same.  For instance the
// USBPGF-S1 does not have a high pass filter so HPFc is not functional.
//**************************************************************************************

class USBPxxPARAMETERS : public CObject
{
	DECLARE_SERIAL(USBPxxPARAMETERS);

	float LPFc = 0;
	float HPFc = 3000;

	long DeviceHandle = 0;
	long ChannelNumber = 0;
	long SerialNumber = 0;
	long ProductID = 0;

	long indexgain = 0;
	long indexCoupling = 0;
	long indexClockSource = 0;
	long indexPClock = 0;
	long indexLPFilterType = 0;
	long indexHPFilterType = 0;

	long Gain = 2;
	int RevisionHigh = 0;
	int RevisionLow = 0;

	CString Description;
	CString csCoupling;
	CString csClockSource;
	CString csPClock;
	CString csLPFilterType;
	CString csHPFilterType;

	USBPxxPARAMETERS& operator =(const USBPxxPARAMETERS& arg);
	long Write(CFile* datafile);
	BOOL Read(CFile* datafile);
	void Serialize(CArchive& ar) override;
};
