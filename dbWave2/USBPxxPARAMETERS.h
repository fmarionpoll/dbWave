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
