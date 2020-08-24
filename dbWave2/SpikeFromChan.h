#pragma once

// CSpikeFromChan structure
// parameters used for detection of spikes from data acquisition

class CSpikeFromChan : public CObject
{
	DECLARE_SERIAL(CSpikeFromChan)
public:
	CSpikeFromChan();
public:
	WORD			wversion;		// version number of the structure
	WORD  			encoding;		// data encoding mode
	long  			binzero;		// 2048
	float 			samprate;		// data acq sampling rate (Hz)
	float 			voltsperbin;	// nb volts per bin (data)
	CString			comment;		// spike channel descriptor

	SPKDETECTPARM	parm;			// detection parameters

// Implementation
public:
	virtual ~CSpikeFromChan();
	void Serialize(CArchive& ar) override;

	CSpikeFromChan& operator= (const CSpikeFromChan& arg);	// redefinition operator =
};
