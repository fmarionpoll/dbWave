#include "StdAfx.h"
#include "SpikeDetectArray.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_SERIAL(SpikeDetectArray, CObject, 0)

SpikeDetectArray::SpikeDetectArray()
{
	const auto spk_detect_parameters = new SPKDETECTPARM;
	ASSERT(spk_detect_parameters != NULL);
	spkdetectparm_ptr_array.SetSize(0);
	spkdetectparm_ptr_array.Add(spk_detect_parameters);
}

SpikeDetectArray::~SpikeDetectArray()
{
	DeleteArray();
}

void SpikeDetectArray::DeleteArray()
{
	const auto isize = spkdetectparm_ptr_array.GetSize();
	for (auto i = 0; i < isize; i++)
	{
		const auto pparm = spkdetectparm_ptr_array[i];
		delete pparm;
	}
	spkdetectparm_ptr_array.RemoveAll();
}

void SpikeDetectArray::SetSize(int nitems)
{
	const auto isize = spkdetectparm_ptr_array.GetSize();
	// delete items
	if (isize > nitems)
	{
		for (auto i = isize - 1; i >= nitems; i--)
		{
			auto pparm = spkdetectparm_ptr_array[i];
			delete pparm;
		}
		spkdetectparm_ptr_array.SetSize(nitems);
	}
	// add dummy items
	else if (isize < nitems)
	{
		for (auto i = isize; i < nitems; i++)
			AddItem();
	}
}

// insert one parameter array item
int SpikeDetectArray::AddItem()
{
	const auto pparm = new SPKDETECTPARM;
	ASSERT(pparm != NULL);
	spkdetectparm_ptr_array.Add(pparm);
	return spkdetectparm_ptr_array.GetSize();
}

// delete one parameter array item
// return isize left
int SpikeDetectArray::RemoveItem(int ichan)
{
	const auto isize = spkdetectparm_ptr_array.GetSize() - 1;
	if (ichan > isize)
		return -1;

	const auto pparm = spkdetectparm_ptr_array[ichan];
	delete pparm;
	spkdetectparm_ptr_array.RemoveAt(ichan);
	return isize;
}

SpikeDetectArray& SpikeDetectArray::operator =(const SpikeDetectArray& arg)
{
	if (this != &arg)
	{
		const auto n_items = arg.spkdetectparm_ptr_array.GetSize();
		SetSize(n_items);
		for (auto i = 0; i < n_items; i++)
			*(spkdetectparm_ptr_array[i]) = *(arg.spkdetectparm_ptr_array[i]);
	}
	return *this;
}

void SpikeDetectArray::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << w_version_number;
		const WORD n_items = spkdetectparm_ptr_array.GetSize();
		ar << n_items;
		for (auto i = 0; i < n_items; i++)
			spkdetectparm_ptr_array[i]->Serialize(ar);
	}
	else
	{
		WORD version;
		ar >> version;
		// version 1 (11-2-96 FMP)
		Serialize_Load(ar, version);
	}
}

void SpikeDetectArray::Serialize_Load(CArchive& ar, WORD wversion)
{
	ASSERT(ar.IsLoading());
	WORD n_items;
	ar >> n_items;
	SetSize(n_items);
	for (auto i = 0; i < n_items; i++)
		spkdetectparm_ptr_array[i]->Serialize(ar);
	if (wversion > 1 && wversion < 3)
	{
		int dummy;
		ar >> dummy;
		ar >> dummy;
		ar >> dummy;
		ar >> dummy;
	}
}

