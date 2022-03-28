#pragma once
#include "SPKDETECTPARM.h"


// parameters used for detection of spikes from data acquisition

class SpikeFromChan : public CObject
{
	DECLARE_SERIAL(SpikeFromChan)

	WORD wversion = 5;
	WORD encoding = 0;
	long binzero = 0; 
	float samprate = 0.f;
	float voltsperbin = 0.f; 
	CString comment; 
	SPKDETECTPARM parm;

	SpikeFromChan();
	~SpikeFromChan() override;
	void Serialize(CArchive& ar) override;
	SpikeFromChan& operator=(const SpikeFromChan& arg);
};
