#include "stdafx.h"
#include "OPTIONS_OUTPUTDATA.h"

//------------------ class OPTIONS_OUTPUTDATA ---------------------------------

IMPLEMENT_SERIAL(OPTIONS_OUTPUTDATA, CObject, 0 /* schema number*/)

OPTIONS_OUTPUTDATA::OPTIONS_OUTPUTDATA()
{
	bChanged = FALSE;						// flag set TRUE if contents has changed
	wversion = 1;							// version number
	bAllowDA = TRUE;
	bPresetWave = TRUE;
	iDAbufferlength = 10000;
	iDAnbuffers = 10;
	iDATriggermode = 0;
	dDAFrequency_perchan = 1000.;
	outputparms_array.SetSize(10);
}

OPTIONS_OUTPUTDATA::~OPTIONS_OUTPUTDATA()
= default;

OPTIONS_OUTPUTDATA& OPTIONS_OUTPUTDATA::operator = (const OPTIONS_OUTPUTDATA & arg)
{
	if (this != &arg) {
		wversion = arg.wversion;
		bAllowDA = arg.bAllowDA;
		csOutputFile = arg.csOutputFile;;
		bPresetWave = arg.bPresetWave;
		iDAbufferlength = arg.iDAbufferlength;
		iDAnbuffers = arg.iDAnbuffers;
		iDATriggermode = arg.iDATriggermode;
		dDAFrequency_perchan = arg.dDAFrequency_perchan;
		int nchannels = arg.outputparms_array.GetSize();
		outputparms_array.SetSize(nchannels);

		for (int i = 0; i < nchannels; i++) {
			outputparms_array[i] = arg.outputparms_array[i];
		}
	}
	return *this;
}

void OPTIONS_OUTPUTDATA::Serialize(CArchive & ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;

		ar << (WORD)1;			// CString
		ar << csOutputFile;

		ar << (WORD)2;			// BOOL
		ar << bAllowDA;
		ar << bPresetWave;

		ar << (WORD)3;			// int
		ar << iDAbufferlength;
		ar << iDAnbuffers;
		ar << iDATriggermode;

		ar << (WORD)1;			// double
		ar << dDAFrequency_perchan;

		int nchannels = outputparms_array.GetSize(); // OUTPUT_PARMS
		ar << (WORD)nchannels;
		for (int i = 0; i < nchannels; i++)
		{
			outputparms_array.GetAt(i).Serialize(ar);
		}
		ar << (WORD)0;			// no more ...
	}
	else
	{
		WORD version;  ar >> version;
		int n;
		WORD wn;

		// cstring parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> csOutputFile;			n--;
		CString csdummy; while (n > 0) { n--; ar >> csdummy; }

		// BOOL parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> bAllowDA;				n--;
		if (n > 0) ar >> bPresetWave;			n--;
		BOOL bdummy; while (n > 0) { n--; ar >> bdummy; }

		// int parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> iDAbufferlength;		n--;
		if (n > 0) ar >> iDAnbuffers;			n--;
		if (n > 0) ar >> iDATriggermode;		n--;
		int idummy; while (n > 0) { n--; ar >> idummy; }

		// double parameters
		ar >> wn; n = wn;
		if (n > 0) ar >> dDAFrequency_perchan;	n--;
		double ddummy; while (n > 0) { n--; ar >> ddummy; }

		// output_parms
		ar >> wn; n = wn;
		if (n > outputparms_array.GetSize())
			outputparms_array.SetSize(n);
		for (int i = 0; i < n; i++) {
			outputparms_array.GetAt(i).Serialize(ar);
		}

		// other?
		ar >> wn; n = wn;
		ASSERT(n == 0);
	}
}
