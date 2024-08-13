#include "StdAfx.h"
#include "AcqDataDoc.h"
#include "SpikeFromChan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



SpikeFromChan::SpikeFromChan()
= default;

SpikeFromChan::~SpikeFromChan()
= default;

IMPLEMENT_SERIAL(SpikeFromChan, CObject, 0 /* schema number*/)

void SpikeFromChan::Serialize(CArchive& ar)
{
	// store elements
	if (ar.IsStoring())
	{
		ar << w_version;
		ar << bin_zero;
		ar << encoding;
		ar << volts_per_bin;
		ar << sampling_rate;
		detect_spikes_parameters.Serialize(ar);
		constexpr int n_items = 1;
		ar << n_items;
		ar << comment;
	}
	// load data
	else
	{
		WORD w;
		WORD version;
		ar >> version; // version number
		if (version < 5)
		{
			ar >> w;
			bin_zero = w;
		}
		else
			ar >> bin_zero; // long (instead of word)
		if (version < 3)
		{
			ar >> w;
			detect_spikes_parameters.extract_channel = w;
		}
		ar >> encoding;
		ar >> volts_per_bin;
		ar >> sampling_rate;
		if (version < 3)
		{
			ar >> w;
			detect_spikes_parameters.detect_transform = w;
			ar >> w;
			detect_spikes_parameters.detect_from = w;
			ar >> w;
			detect_spikes_parameters.detect_threshold_bin = w;
			ar >> w; // unused parameter, removed at version 3
			ar >> w;
			detect_spikes_parameters.extract_n_points = w;
			ar >> w;
			detect_spikes_parameters.detect_pre_threshold = w;
			ar >> w;
			detect_spikes_parameters.detect_refractory_period = w;
		}
		else
		{
			detect_spikes_parameters.Serialize(ar);
		}
		if (version > 3)
		{
			int n_items;
			ar >> n_items;
			ar >> comment;
			n_items--;
		}
	}
}

SpikeFromChan& SpikeFromChan::operator =(const SpikeFromChan& arg)
{
	if (&arg != this)
	{
		w_version = arg.w_version;
		bin_zero = arg.bin_zero;
		encoding = arg.encoding;
		sampling_rate = arg.sampling_rate;
		volts_per_bin = arg.volts_per_bin;
		detect_spikes_parameters = arg.detect_spikes_parameters;
		comment = arg.comment;
	}
	return *this;
}
