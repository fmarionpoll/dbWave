#include "StdAfx.h"
#include "SPKDETECTPARM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(SPKDETECTPARM, CObject, 0 /* schema number*/)

SPKDETECTPARM::SPKDETECTPARM() : bChanged(0)
{
	wversion = 7; // version 6 (Aug 19 2005 FMP)
	detect_channel = 0; // source channel
	detect_transform = 13; // detect from data transformed - i = transform method cf AcqDataDoc
	detectFrom = 0; // detection method 0=data, 1=tags
	extract_channel = 0;
	extract_transform = 13;
	compensateBaseline = FALSE;
	detect_threshold = 0; // value of threshold 1
	extract_n_points = 60; // spike length (n data pts)
	detect_pre_threshold = 20; // offset spike npts before threshold
	detect_refractory_period = 20; // re-start detection n pts after threshold
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
		detect_channel = arg.detect_channel;
		detect_transform = arg.detect_transform;
		detect_threshold = arg.detect_threshold;
		compensateBaseline = arg.compensateBaseline;
		extract_channel = arg.extract_channel;
		extract_transform = arg.extract_transform;
		extract_n_points = arg.extract_n_points;
		detect_pre_threshold = arg.detect_pre_threshold;
		detect_refractory_period = arg.detect_refractory_period;
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
	detect_channel = lw; // long
	ar >> lw;
	detect_transform = lw; // long
	ar >> wi;
	detectFrom = wi; // WORD
	ar >> wi;
	compensateBaseline = wi; // WORD
	ar >> lw;
	detect_threshold = lw; // long
	if (version < 3) // dummy reading (threshold2: removed version 3)
		ar >> lw;
	ar >> lw;
	extract_n_points = lw; // long
	ar >> lw;
	detect_pre_threshold = lw; // long
	ar >> lw;
	detect_refractory_period = lw; // long
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
		extract_channel = lw; // long
		ar >> lw;
		extract_transform = lw; // long
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
	ar >> detect_channel;
	nitems--;
	ar >> detect_transform;
	nitems--;
	ar >> detect_threshold;
	nitems--;
	ar >> extract_n_points;
	nitems--;
	ar >> detect_pre_threshold;
	nitems--;
	ar >> detect_refractory_period;
	nitems--;
	ar >> extract_channel;
	nitems--;
	ar >> extract_transform;
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
	ar >> detect_channel;
	nitems--;
	ar >> detect_transform;
	nitems--;
	ar >> detect_threshold;
	nitems--;
	ar >> extract_n_points;
	nitems--;
	ar >> detect_pre_threshold;
	nitems--;
	ar >> detect_refractory_period;
	nitems--;
	ar >> extract_channel;
	nitems--;
	ar >> extract_transform;
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
		ar << detect_channel;
		ar << detect_transform;
		ar << detect_threshold;
		ar << extract_n_points;
		ar << detect_pre_threshold;
		ar << detect_refractory_period;
		ar << extract_channel;
		ar << extract_transform;
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
