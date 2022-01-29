#include "stdafx.h"
#include "USBPxxPARAMETERS.h"

IMPLEMENT_SERIAL(USBPxxPARAMETERS, CObject, 0 /* schema number*/)


USBPxxPARAMETERS& USBPxxPARAMETERS::operator=(const USBPxxPARAMETERS& arg)
{
	if (this != &arg)
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
	return *this;
}

long USBPxxPARAMETERS::Write(CFile* datafile)
{
	const auto p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	const auto p2 = datafile->GetPosition();
	return static_cast<long>(p2 - p1);
}

BOOL USBPxxPARAMETERS::Read(CFile* datafile)
{
	CArchive ar(datafile, CArchive::load);
	auto flag = TRUE;
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

void USBPxxPARAMETERS::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		const WORD wversion = 1;
		ar << wversion;

		int nitems = 2;
		ar << nitems;
		ar << LPFc;
		ar << HPFc;

		nitems = 10;
		ar << nitems;
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

		nitems = 2;
		ar << nitems;
		ar << RevisionHigh;
		ar << RevisionLow;

		nitems = 6;
		ar << nitems;
		ar << Description;
		ar << csCoupling;
		ar << csClockSource;
		ar << csPClock;
		ar << csLPFilterType;
		ar << csHPFilterType;
	}
	else
	{
		WORD wversion;
		ar >> wversion;
		ASSERT(wversion == 1);

		int nitems;
		ar >> nitems;
		if (nitems > 0) ar >> LPFc;
		nitems--;
		if (nitems > 0) ar >> HPFc;
		nitems--;

		ar >> nitems;
		if (nitems > 0) ar >> DeviceHandle;
		nitems--;
		if (nitems > 0) ar >> indexgain;
		nitems--;
		if (nitems > 0) ar >> indexCoupling;
		nitems--;
		if (nitems > 0) ar >> indexClockSource;
		nitems--;
		if (nitems > 0) ar >> indexPClock;
		nitems--;
		if (nitems > 0) ar >> ChannelNumber;
		nitems--;
		if (nitems > 0) ar >> indexLPFilterType;
		nitems--;
		if (nitems > 0) ar >> indexHPFilterType;
		nitems--;
		if (nitems > 0) ar >> SerialNumber;
		nitems--;
		if (nitems > 0) ar >> ProductID;
		nitems--;

		ar >> nitems;
		if (nitems > 0) ar >> RevisionHigh;
		nitems--;
		if (nitems > 0) ar >> RevisionLow;
		nitems--;

		ar >> nitems;
		if (nitems > 0) ar >> Description;
		nitems--;
		if (nitems > 0) ar >> csCoupling;
		nitems--;
		if (nitems > 0) ar >> csClockSource;
		nitems--;
		if (nitems > 0) ar >> csPClock;
		nitems--;
		if (nitems > 0) ar >> csLPFilterType;
		nitems--;
		if (nitems > 0) ar >> csHPFilterType;
		nitems--;
	}
}
