#pragma once
#include "options_detect_spikes.h"


// parameters used for detection of spikes from data acquisition

class SpikeFromChan : public CObject
{
	DECLARE_SERIAL(SpikeFromChan)

	WORD w_version  {5};
	WORD encoding  {0};
	long bin_zero  {0}; 
	float sampling_rate  {0.f};
	float volts_per_bin  {0.f}; 
	CString comment {}; 
	options_detect_spikes detect_spikes_parameters;

	SpikeFromChan();
	~SpikeFromChan() override;
	void Serialize(CArchive& ar) override;
	SpikeFromChan& operator=(const SpikeFromChan& arg);
};
