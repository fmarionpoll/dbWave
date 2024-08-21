#include "StdAfx.h"
#include "SpikeDetectArray.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_SERIAL(SpikeDetectArray, CObject, 0)

SpikeDetectArray::SpikeDetectArray()
{
	const auto spk_detect_parameters = new options_detect_spikes();
	ASSERT(spk_detect_parameters != NULL);
	spk_detect_options_array_.SetSize(0);
	spk_detect_options_array_.Add(spk_detect_parameters);
}

SpikeDetectArray::~SpikeDetectArray()
{
	delete_array();
}

void SpikeDetectArray::delete_array()
{
	const auto i_size = spk_detect_options_array_.GetSize();
	for (auto i = 0; i < i_size; i++)
	{
		const auto p_parameters_set = spk_detect_options_array_[i];
		delete p_parameters_set;
	}
	spk_detect_options_array_.RemoveAll();
}

void SpikeDetectArray::set_size(const int n_items)
{
	const auto i_size = spk_detect_options_array_.GetSize();
	// delete items
	if (i_size > n_items)
	{
		for (auto i = i_size - 1; i >= n_items; i--)
		{
			auto p_parameters_set = spk_detect_options_array_[i];
			delete p_parameters_set;
		}
		spk_detect_options_array_.SetSize(n_items);
	}
	// add dummy items
	else if (i_size < n_items)
	{
		for (auto i = i_size; i < n_items; i++)
			add_item();
	}
}

int SpikeDetectArray::add_item()
{
	const auto p_parameters_set = new options_detect_spikes();
	ASSERT(p_parameters_set != NULL);
	spk_detect_options_array_.Add(p_parameters_set);
	return spk_detect_options_array_.GetSize();
}

int SpikeDetectArray::remove_item(const int i)
{
	const auto i_size = spk_detect_options_array_.GetSize() - 1;
	if (i > i_size)
		return -1;

	const auto p_parameters_set = spk_detect_options_array_[i];
	delete p_parameters_set;
	spk_detect_options_array_.RemoveAt(i);
	return i_size;
}

SpikeDetectArray& SpikeDetectArray::operator =(const SpikeDetectArray& arg)
{
	if (this != &arg)
	{
		const auto n_items = arg.spk_detect_options_array_.GetSize();
		set_size(n_items);
		for (auto i = 0; i < n_items; i++)
			*(spk_detect_options_array_[i]) = *(arg.spk_detect_options_array_[i]);
	}
	return *this;
}

void SpikeDetectArray::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << w_version_number_;
		const WORD n_items = spk_detect_options_array_.GetSize();
		ar << n_items;
		for (auto i = 0; i < n_items; i++)
			spk_detect_options_array_[i]->Serialize(ar);
	}
	else
	{
		WORD version;
		ar >> version;
		// version 1 (11-2-96 FMP)
		load(ar, version);
	}
}

void SpikeDetectArray::load(CArchive& ar, const WORD w_version)
{
	ASSERT(ar.IsLoading());
	WORD n_items;
	ar >> n_items;
	set_size(n_items);
	for (auto i = 0; i < n_items; i++)
		spk_detect_options_array_[i]->Serialize(ar);
	if (w_version > 1 && w_version < 3)
	{
		int dummy;
		ar >> dummy;
		ar >> dummy;
		ar >> dummy;
		ar >> dummy;
	}
}

