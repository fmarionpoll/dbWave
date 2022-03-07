// StimLevelArray.cpp : implementation file

#include "StdAfx.h"
#include "Intervals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CIntervals, CObject, 0)

CIntervals::CIntervals()
{
	intervalsArray.SetSize(0); // time on, time off
}

CIntervals::~CIntervals()
{
	intervalsArray.RemoveAll();
}

CIntervals& CIntervals::operator =(const CIntervals& arg)
{
	if (this != &arg)
	{
		iID = arg.iID; // ID number of the array
		cs_descriptor = arg.cs_descriptor; // descriptor of the array
		n_items = arg.n_items; // number of on/off events
		intervalsArray.SetSize(arg.intervalsArray.GetSize());

		for (auto i = 0; i < arg.intervalsArray.GetSize(); i++)
			intervalsArray.SetAt(i, arg.intervalsArray.GetAt(i)); // time on, time off
		n_per_cycle = arg.n_per_cycle;
		channel_sampling_rate = arg.channel_sampling_rate;
	}
	return *this;
}

void CIntervals::Serialize(CArchive& ar)
{
	auto iversion = 2;
	if (ar.IsStoring())
	{
		auto n = 4;
		ar << n;
		ar << iID;
		ar << n_items;
		ar << n_per_cycle;
		ar << iversion;

		n = 1;
		ar << n;
		ar << cs_descriptor;

		n = 1;
		ar << n;
		intervalsArray.Serialize(ar);

		n = 1;
		ar << n;
		ar << channel_sampling_rate;
	}
	else
	{
		int n;
		ar >> n;
		ar >> iID;
		n--;
		ar >> n_items;
		n--;
		n_per_cycle = 1;
		if (n > 0) ar >> n_per_cycle;
		n--;

		if (n > 0)
		{
			ar >> iversion;
			n--;
			ASSERT(iversion == 2);

			ar >> n;
			if (n > 0) ar >> cs_descriptor;
			n--;
			ar >> n;
			if (n > 0) intervalsArray.Serialize(ar);
			n--;
			if (iversion > 1)
				ar >> n;
			if (n > 0) ar >> channel_sampling_rate;
			n--;
		}
		else // old version
		{
			ar >> n;
			ar >> cs_descriptor; // descriptor of the array
			ar >> n;
			intervalsArray.Serialize(ar);
		}
	}
}

