#pragma once

class OPTIONS_OUTPUTDATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_OUTPUTDATA)
	OPTIONS_OUTPUTDATA();
	~OPTIONS_OUTPUTDATA();
	OPTIONS_OUTPUTDATA& operator = (const OPTIONS_OUTPUTDATA& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL	bChanged;								// flag set TRUE if contents has changed
	WORD	wversion;								// version number

	BOOL	bAllowDA;								// allow DA if present
	BOOL	bPresetWave;							// 0=preset, 1=from file
	CString	csOutputFile;							// output file

	CArray <OUTPUTPARMS, OUTPUTPARMS> outputparms_array;

	double	dDAFrequency_perchan;					// output frequency (per chan)
	int		iDATriggermode;							// 0=synchronous; 1=soft on start; 2=external trigger
	int		iDAnbuffers;							// number of DA buffers
	int		iDAbufferlength;						// length of each buffer
};

