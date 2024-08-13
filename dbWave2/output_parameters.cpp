#include "StdAfx.h"
#include "output_parameters.h"


IMPLEMENT_SERIAL(output_parameters, CObject, 0 /* schema number*/)

output_parameters::output_parameters()
= default;

output_parameters::~output_parameters()
= default;

output_parameters& output_parameters::operator =(const output_parameters& arg)
{
	if (this != &arg)
	{
		wversion = arg.wversion;
		csFilename = arg.csFilename;
		noise_bExternal = arg.noise_bExternal;
		bON = arg.bON;
		bDigital = arg.bDigital;
		iChan = arg.iChan;
		iWaveform = arg.iWaveform;
		mseq_iRatio = arg.mseq_iRatio;
		mseq_iDelay = arg.mseq_iDelay;
		mseq_iSeed = arg.mseq_iSeed;
		noise_iDelay = arg.noise_iDelay;
		dAmplitudeMaxV = arg.dAmplitudeMaxV;
		dAmplitudeMinV = arg.dAmplitudeMinV;
		dFrequency = arg.dFrequency;
		dummy1 = arg.dummy1;
		dummy2 = arg.dummy2;
		noise_dAmplitV = arg.noise_dAmplitV;
		noise_dFactor = arg.noise_dFactor;
		noise_dOffsetV = arg.noise_dOffsetV;
		stimulussequence = arg.stimulussequence;
		value = arg.value;
		sti = arg.sti;
	}
	return *this;
}

void output_parameters::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << wversion;

		ar << static_cast<WORD>(1); // CString
		ar << csFilename;

		ar << static_cast<WORD>(3); // BOOL
		ar << noise_bExternal;
		ar << bON;
		ar << bDigital;

		ar << static_cast<WORD>(6); // int
		ar << iChan;
		ar << iWaveform;
		ar << mseq_iRatio;
		ar << mseq_iDelay;
		ar << mseq_iSeed;
		ar << noise_iDelay;

		ar << static_cast<WORD>(9); // double
		ar << dAmplitudeMaxV;
		ar << dAmplitudeMinV;
		ar << dFrequency;
		ar << dummy1;
		ar << dummy2;
		ar << noise_dAmplitV;
		ar << noise_dFactor;
		ar << noise_dOffsetV;
		ar << value;

		ar << static_cast<WORD>(2); // 1 more object
		stimulussequence.Serialize(ar);
		sti.Serialize(ar);
	}
	else
	{
		WORD version;
		ar >> version;
		WORD wn;

		// string parameters
		ar >> wn;
		int n = wn;
		if (n > 0) ar >> csFilename;
		n--;
		CString csdummy;
		while (n > 0)
		{
			n--;
			ar >> csdummy;
		}

		// BOOL parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> noise_bExternal;
		n--;
		if (n > 0) ar >> bON;
		n--;
		if (n > 0) ar >> bDigital;
		n--;
		BOOL bdummy;
		while (n > 0)
		{
			n--;
			ar >> bdummy;
		}

		// int parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> iChan;
		n--;
		if (n > 0) ar >> iWaveform;
		n--;
		if (n > 0) ar >> mseq_iRatio;
		n--;
		if (n > 0) ar >> mseq_iDelay;
		n--;
		if (n > 0) ar >> mseq_iSeed;
		n--;
		if (n > 0) ar >> noise_iDelay;
		n--;
		int idummy;
		while (n > 0)
		{
			n--;
			ar >> idummy;
		}

		// double parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> dAmplitudeMaxV;
		n--;
		if (n > 0) ar >> dAmplitudeMinV;
		n--;
		if (n > 0) ar >> dFrequency;
		n--;
		if (n > 0) ar >> dummy1;
		n--;
		if (n > 0) ar >> dummy2;
		n--;
		if (n > 0) ar >> noise_dAmplitV;
		n--;
		if (n > 0) ar >> noise_dFactor;
		n--;
		if (n > 0) ar >> noise_dOffsetV;
		n--;
		if (n > 0) ar >> value;
		n--;
		double ddummy;
		while (n > 0)
		{
			n--;
			ar >> ddummy;
		}

		// other?
		ar >> wn;
		n = wn;
		if (n > 0)
		{
			stimulussequence.Serialize(ar);
			n--;
		}
		if (n > 0)
		{
			sti.Serialize(ar);
			n--;
		}
		ASSERT(n == 0);
	}
}
