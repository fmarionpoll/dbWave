#pragma once
#include "OUTPUTPARMS.h"

class OPTIONS_OUTPUTDATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_OUTPUTDATA)
	OPTIONS_OUTPUTDATA();
	~OPTIONS_OUTPUTDATA() override;
	OPTIONS_OUTPUTDATA& operator =(const OPTIONS_OUTPUTDATA& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL b_changed{false};
	WORD wversion{1};

	BOOL bAllowDA{true}; // allow DA if present
	BOOL bPresetWave{true}; // 0=preset, 1=from file
	CString csOutputFile{}; // output file

	CArray<OUTPUTPARMS, OUTPUTPARMS> outputparms_array{};

	double dDAFrequency_perchan{1000.f}; // output frequency (per chan)
	int iDATriggermode{0}; // 0=synchronous{}; 1=soft on start{}; 2=external trigger
	int iDAnbuffers{10}; // number of DA buffers
	int iDAbufferlength{10000}; // length of each buffer
};
