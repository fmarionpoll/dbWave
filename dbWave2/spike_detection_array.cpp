#include "stdafx.h"
#include "spike_detection_array.h"

IMPLEMENT_SERIAL(spike_detection_array, CObject, 0)

spike_detection_array::spike_detection_array()
{
	b_changed = FALSE;
	w_version_ = 4;
}

spike_detection_array::~spike_detection_array()
{
	delete_all();
}

void spike_detection_array::delete_all()
{
	auto pos = chan_array_map_.GetStartPosition();
	void* ptr = nullptr;
	WORD w_key;
	while (pos)
	{
		chan_array_map_.GetNextAssoc(pos, w_key, ptr);
		auto p_spk_detect = static_cast<SpikeDetectArray*>(ptr);
		ASSERT_VALID(p_spk_detect);
		delete p_spk_detect;
		p_spk_detect = nullptr;
	}
	chan_array_map_.RemoveAll();
}

// check if array is ok, if not, increase size of array
// create empty CPtrArray if necessary
SpikeDetectArray* spike_detection_array::get_chan_array(const int acq_chan)
{
	void* ptr = nullptr;
	if (!chan_array_map_.Lookup(static_cast<WORD>(acq_chan), ptr))
	{
		const auto p_spk_detect_array = new SpikeDetectArray;
		ptr = p_spk_detect_array;
		chan_array_map_.SetAt(static_cast<WORD>(acq_chan), ptr);
	}
	return static_cast<SpikeDetectArray*>(ptr);
}

void spike_detection_array::set_chan_array(const int acq_chan, SpikeDetectArray* p_spk)
{
	void* ptr = p_spk;
	chan_array_map_.SetAt(static_cast<WORD>(acq_chan), ptr);
}

void spike_detection_array::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		w_version_ = 4;
		ar << w_version_; // w_version_number = 4
		const auto n_arrays = chan_array_map_.GetSize();
		ar << n_arrays;
		auto pos = chan_array_map_.GetStartPosition();
		void* ptr = nullptr;
		WORD w_key;
		while (pos)
		{
			chan_array_map_.GetNextAssoc(pos, w_key, ptr);
			ar << w_key;
			auto* p_spk_detect_array = static_cast<SpikeDetectArray*>(ptr);
			ASSERT_VALID(p_spk_detect_array);
			p_spk_detect_array->Serialize(ar);
		}
	}
	else
	{
		delete_all();
		WORD version;
		ar >> version;
		// version 1 (11-2-96 FMP)
		if (version < 4)
		{
			const auto p_spk_detect_array = new SpikeDetectArray;
			p_spk_detect_array->load(ar, version);
			constexpr WORD w_key = 1;
			void* ptr = p_spk_detect_array;
			chan_array_map_.SetAt(w_key, ptr);
		}
		else if (version >= 4)
		{
			int n_arrays;
			ar >> n_arrays;
			for (auto j = 0; j < n_arrays; j++)
			{
				WORD w_key;
				ar >> w_key;
				auto* p_spk_detect_array = new SpikeDetectArray;
				p_spk_detect_array->Serialize(ar);
				void* ptr = p_spk_detect_array;
				chan_array_map_.SetAt(w_key, ptr);
			}
		}
	}
}


