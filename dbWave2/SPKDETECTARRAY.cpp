#include "stdafx.h"
#include "SPKDETECTARRAY.h"

IMPLEMENT_SERIAL(SPKDETECTARRAY, CObject, 0)

SPKDETECTARRAY::SPKDETECTARRAY()
{
	bChanged = FALSE;
	wversion = 4;
}

SPKDETECTARRAY::~SPKDETECTARRAY()
{
	DeleteAll();
}

// erase all arrays of parmItems (and all parmItems within them)
void SPKDETECTARRAY::DeleteAll()
{
	auto pos = chanArrayMap.GetStartPosition();
	void* ptr = nullptr;
	WORD w_key;
	while (pos)
	{
		chanArrayMap.GetNextAssoc(pos, w_key, ptr);
		auto pspk = static_cast<SpikeDetectArray*>(ptr);
		ASSERT_VALID(pspk);
		delete pspk;
		pspk = nullptr;
	}
	chanArrayMap.RemoveAll();
}

// check if array is ok, if not, increase size of array
// create empty CPtrArray if necessary
SpikeDetectArray* SPKDETECTARRAY::GetChanArray(int acqchan)
{
	void* ptr = nullptr;
	if (!chanArrayMap.Lookup(acqchan, ptr))
	{
		const auto pspk = new SpikeDetectArray;
		ptr = pspk;
		chanArrayMap.SetAt(acqchan, ptr);
	}
	return static_cast<SpikeDetectArray*>(ptr);
}

void SPKDETECTARRAY::SetChanArray(int acqchan, SpikeDetectArray* pspk)
{
	void* ptr = pspk;
	chanArrayMap.SetAt(acqchan, ptr);
}

void SPKDETECTARRAY::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		wversion = 4;
		ar << wversion; // w_version_number = 4
		const auto narrays = chanArrayMap.GetSize();
		ar << narrays;
		auto pos = chanArrayMap.GetStartPosition();
		void* ptr = nullptr;
		WORD w_key;
		while (pos)
		{
			chanArrayMap.GetNextAssoc(pos, w_key, ptr);
			ar << w_key;
			auto* pspk = static_cast<SpikeDetectArray*>(ptr);
			ASSERT_VALID(pspk);
			pspk->Serialize(ar);
		}
	}
	else
	{
		DeleteAll();
		WORD version;
		ar >> version;
		// version 1 (11-2-96 FMP)
		if (version < 4)
		{
			auto parm_items = new SpikeDetectArray;
			parm_items->Serialize_Load(ar, version);
			const WORD w_key = 1;
			void* ptr = parm_items;
			chanArrayMap.SetAt(w_key, ptr);
		}
		else if (version >= 4)
		{
			int narrays;
			ar >> narrays;
			for (auto j = 0; j < narrays; j++)
			{
				WORD w_key;
				ar >> w_key;
				auto* pspk = new SpikeDetectArray;
				pspk->Serialize(ar);
				void* ptr = pspk;
				chanArrayMap.SetAt(w_key, ptr);
			}
		}
	}
}


