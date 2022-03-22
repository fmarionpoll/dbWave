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
		ar << wversion;
		ar << binzero;
		ar << encoding;
		ar << voltsperbin;
		ar << samprate;
		parm.Serialize(ar);
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
			binzero = w;
		}
		else
			ar >> binzero; // long (instead of word)
		if (version < 3)
		{
			ar >> w;
			parm.extractChan = w;
		}
		ar >> encoding;
		ar >> voltsperbin;
		ar >> samprate;
		if (version < 3)
		{
			ar >> w;
			parm.detectTransform = w;
			ar >> w;
			parm.detectFrom = w;
			ar >> w;
			parm.detectThreshold = w;
			ar >> w; // unused parameter, removed at version 3
			ar >> w;
			parm.extractNpoints = w;
			ar >> w;
			parm.prethreshold = w;
			ar >> w;
			parm.refractory = w;
		}
		else
		{
			parm.Serialize(ar);
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
		wversion = arg.wversion;
		binzero = arg.binzero;
		encoding = arg.encoding;
		samprate = arg.samprate;
		voltsperbin = arg.voltsperbin;
		parm = arg.parm;
		comment = arg.comment;
	}
	return *this;
}
