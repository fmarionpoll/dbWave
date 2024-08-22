
#include "StdAfx.h"
#include "Intervals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CIntervals, CObject, 0)

CIntervals::CIntervals()
{
	array_.SetSize(0); // time on, time off
}

CIntervals::~CIntervals()
{
	array_.RemoveAll();
}

CIntervals& CIntervals::operator =(const CIntervals& arg)
{
	if (this != &arg)
	{
		i_id	= arg.i_id;
		channel = arg.channel;
		cs_descriptor = arg.cs_descriptor; 
		n_items = arg.n_items;
		n_per_cycle = arg.n_per_cycle;
		channel_sampling_rate = arg.channel_sampling_rate;

		array_.SetSize(arg.array_.GetSize());
		for (auto i = 0; i < arg.array_.GetSize(); i++)
			array_.SetAt(i, arg.array_.GetAt(i)); 
	}
	return *this;
}

void CIntervals::Serialize(CArchive& ar)
{
	auto i_version = 2;
	if (ar.IsStoring())
	{
		auto n = 4;
		ar << n;
		ar << i_id;
		ar << n_items;
		ar << n_per_cycle;
		ar << i_version;

		n = 1;
		ar << n;
		ar << cs_descriptor;

		n = 1;
		ar << n;
		array_.Serialize(ar);

		n = 1;
		ar << n;
		ar << channel_sampling_rate;
	}
	else
	{
		int n;
		ar >> n;
		ar >> i_id;
		n--;
		ar >> n_items;
		n--;
		n_per_cycle = 1;
		if (n > 0) ar >> n_per_cycle;
		n--;

		if (n > 0)
		{
			ar >> i_version;
			n--;
			ASSERT(i_version == 2);

			ar >> n;
			if (n > 0) ar >> cs_descriptor;
			n--;
			ar >> n;
			if (n > 0) array_.Serialize(ar);
			n--;
			if (i_version > 1)
				ar >> n;
			if (n > 0) ar >> channel_sampling_rate;
			n--;
		}
		else // old version
		{
			ar >> n;
			ar >> cs_descriptor; 
			ar >> n;
			array_.Serialize(ar);
		}
	}
}

