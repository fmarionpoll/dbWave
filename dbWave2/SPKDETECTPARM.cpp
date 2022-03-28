#include "StdAfx.h"
#include "SPKDETECTPARM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(SPKDETECTPARM, CObject, 0 /* schema number*/)

SPKDETECTPARM::SPKDETECTPARM() : bChanged(0)
{
	wversion = 7; // version 6 (Aug 19 2005 FMP)
	detectChan = 0; // source channel
	detectTransform = 13; // detect from data transformed - i = transform method cf AcqDataDoc
	detectFrom = 0; // detection method 0=data, 1=tags
	extractChan = 0;
	extractTransform = 13;
	compensateBaseline = FALSE;
	detectThreshold = 0; // value of threshold 1
	extractNpoints = 60; // spike length (n data pts)
	prethreshold = 20; // offset spike npts before threshold
	refractory = 20; // re-start detection n pts after threshold
	detectThresholdmV = 0.5f; // detection threshold in mV
	detectWhat = DETECT_SPIKES; // detect spikes, 1=detect stimulus
	detectMode = MODE_ON_OFF; // if sti, = ON/OFF
}

SPKDETECTPARM::~SPKDETECTPARM()
{
}

SPKDETECTPARM& SPKDETECTPARM::operator =(const SPKDETECTPARM& arg)
{
	if (this != &arg)
	{
		comment = arg.comment;
		detectFrom = arg.detectFrom;
		detectChan = arg.detectChan;
		detectTransform = arg.detectTransform;
		detectThreshold = arg.detectThreshold;
		compensateBaseline = arg.compensateBaseline;
		extractChan = arg.extractChan;
		extractTransform = arg.extractTransform;
		extractNpoints = arg.extractNpoints;
		prethreshold = arg.prethreshold;
		refractory = arg.refractory;
		detectThresholdmV = arg.detectThresholdmV;
		detectWhat = arg.detectWhat;
		detectMode = arg.detectMode;
	}
	return *this;
}

void SPKDETECTPARM::ReadVersionlessthan6(CArchive& ar, int version)
{
	long lw;
	WORD wi;
	ar >> comment; // CString
	ar >> lw;
	detectChan = lw; // long
	ar >> lw;
	detectTransform = lw; // long
	ar >> wi;
	detectFrom = wi; // WORD
	ar >> wi;
	compensateBaseline = wi; // WORD
	ar >> lw;
	detectThreshold = lw; // long
	if (version < 3) // dummy reading (threshold2: removed version 3)
		ar >> lw;
	ar >> lw;
	extractNpoints = lw; // long
	ar >> lw;
	prethreshold = lw; // long
	ar >> lw;
	refractory = lw; // long
	if (version < 3) // dummy reading (threshold adjust method)
	{
		// removed at version 3, moved to SPKdetectARRAY
		ar >> lw; // (int)
		float fw;
		ar >> fw;
		ar >> lw;
		ar >> lw;
	}
	if (version > 1) // added at version 2
	{
		ar >> lw;
		extractChan = lw; // long
		ar >> lw;
		extractTransform = lw; // long
	}
}

void SPKDETECTPARM::ReadVersion6(CArchive& ar)
{
	int nitems;
	ar >> nitems;
	ar >> comment;
	nitems--;
	ASSERT(nitems == 0);

	ar >> nitems;
	ar >> detectFrom;
	nitems--;
	ar >> compensateBaseline;
	nitems--;
	ASSERT(nitems == 0);

	ar >> nitems;
	ar >> detectChan;
	nitems--;
	ar >> detectTransform;
	nitems--;
	ar >> detectThreshold;
	nitems--;
	ar >> extractNpoints;
	nitems--;
	ar >> prethreshold;
	nitems--;
	ar >> refractory;
	nitems--;
	ar >> extractChan;
	nitems--;
	ar >> extractTransform;
	nitems--;
	if (nitems > 0) ar >> detectWhat;
	nitems--;
	if (nitems > 0) ar >> detectMode;
	nitems--;

	ar >> nitems;
	ar >> detectThresholdmV;
	nitems--;
	ASSERT(nitems == 0);
}

void SPKDETECTPARM::ReadVersion7(CArchive& ar)
{
	int nitems;
	// string parameters
	ar >> nitems;
	ar >> comment;
	nitems--;
	ASSERT(nitems == 0);
	// int parameters
	ar >> nitems;
	ar >> detectFrom;
	nitems--;
	ar >> compensateBaseline;
	nitems--;
	ar >> detectChan;
	nitems--;
	ar >> detectTransform;
	nitems--;
	ar >> detectThreshold;
	nitems--;
	ar >> extractNpoints;
	nitems--;
	ar >> prethreshold;
	nitems--;
	ar >> refractory;
	nitems--;
	ar >> extractChan;
	nitems--;
	ar >> extractTransform;
	nitems--;
	ar >> detectWhat;
	nitems--;
	ar >> detectMode;
	nitems--;

	ASSERT(nitems == 0);
	// float
	ar >> nitems;
	ar >> detectThresholdmV;
	nitems--;
	ASSERT(nitems == 0);
}

void SPKDETECTPARM::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// version 7 (Aug 19 2005 FMP)
		ar << wversion;
		int nitems = 1;
		ar << nitems;
		ar << comment;

		nitems = 12;
		ar << nitems;
		ar << detectFrom;
		ar << compensateBaseline;
		ar << detectChan;
		ar << detectTransform;
		ar << detectThreshold;
		ar << extractNpoints;
		ar << prethreshold;
		ar << refractory;
		ar << extractChan;
		ar << extractTransform;
		ar << detectWhat;
		ar << detectMode;

		nitems = 1;
		ar << nitems;
		ar << detectThresholdmV;
	}
	else
	{
		WORD version;
		ar >> version;
		if (version != wversion)
		{
			if (version > 0 && version < 5)
				ReadVersionlessthan6(ar, version);
			else if (version < 7) //== 6)
				ReadVersion6(ar);
			else
			{
				ASSERT(FALSE);
				CString message;
				message.Format(_T("Error: version (%i) not recognized"), version);
				AfxMessageBox(message, MB_OK);
			}
		}
		else
			ReadVersion7(ar);
	}
}
