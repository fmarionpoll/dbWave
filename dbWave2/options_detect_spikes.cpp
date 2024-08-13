#include "StdAfx.h"
#include "options_detect_spikes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(options_detect_spikes, CObject, 0 /* schema number*/)

options_detect_spikes::options_detect_spikes() : b_changed(0)
{
	w_version = 7; // version 6 (Aug 19 2005 FMP)
	detect_channel = 0; // source channel
	detect_transform = 13; // detect from data transformed - i = transform method cf AcqDataDoc
	detect_from = 0; // detection method 0=data, 1=tags
	extract_channel = 0;
	extract_transform = 13;
	compensate_baseline = FALSE;
	detect_threshold_bin = 0; // value of threshold 1
	extract_n_points = 60; // spike length (n data pts)
	detect_pre_threshold = 20; // offset spike npts before threshold
	detect_refractory_period = 20; // re-start detection n pts after threshold
	detect_threshold_mv = 0.5f; // detection threshold in mV
	detect_what = DETECT_SPIKES; // detect spikes, 1=detect stimulus
	detect_mode = MODE_ON_OFF; // if sti, = ON/OFF
}

options_detect_spikes::~options_detect_spikes()
{
}

options_detect_spikes& options_detect_spikes::operator =(const options_detect_spikes& arg)
{
	if (this != &arg)
	{
		comment = arg.comment;
		detect_from = arg.detect_from;
		detect_channel = arg.detect_channel;
		detect_transform = arg.detect_transform;
		detect_threshold_bin = arg.detect_threshold_bin;
		compensate_baseline = arg.compensate_baseline;
		extract_channel = arg.extract_channel;
		extract_transform = arg.extract_transform;
		extract_n_points = arg.extract_n_points;
		detect_pre_threshold = arg.detect_pre_threshold;
		detect_refractory_period = arg.detect_refractory_period;
		detect_threshold_mv = arg.detect_threshold_mv;
		detect_what = arg.detect_what;
		detect_mode = arg.detect_mode;
	}
	return *this;
}

void options_detect_spikes::Read_v5(CArchive& ar, int version)
{
	long lw;
	WORD wi;
	ar >> comment; // CString
	ar >> lw;
	detect_channel = lw; // long
	ar >> lw;
	detect_transform = lw; // long
	ar >> wi;
	detect_from = wi; // WORD
	ar >> wi;
	compensate_baseline = wi; // WORD
	ar >> lw;
	detect_threshold_bin = lw; // long
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

void options_detect_spikes::Read_v6(CArchive& ar)
{
	int nitems;
	ar >> nitems;
	ar >> comment; nitems--;
	ASSERT(nitems == 0);

	ar >> nitems;
	ar >> detect_from; nitems--;
	ar >> compensate_baseline; nitems--;
	ASSERT(nitems == 0);

	ar >> nitems;
	ar >> detect_channel; nitems--;
	ar >> detect_transform; nitems--;
	ar >> detect_threshold_bin; nitems--;
	ar >> extract_n_points; nitems--;
	ar >> detect_pre_threshold; nitems--;
	ar >> detect_refractory_period; nitems--;
	ar >> extract_channel; nitems--;
	ar >> extract_transform; nitems--;
	if (nitems > 0) ar >> detect_what; nitems--;
	if (nitems > 0) ar >> detect_mode; nitems--;

	ar >> nitems;
	ar >> detect_threshold_mv; nitems--;
	ASSERT(nitems == 0);
}

void options_detect_spikes::Serialize_v7(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// version 7 (Aug 19 2005 FMP)
		ar << w_version;
		int n_items = 1;
		ar << n_items;
		ar << comment;

		n_items = 12;
		ar << n_items;
		ar << detect_from;
		ar << compensate_baseline;
		ar << detect_channel;
		ar << detect_transform;
		ar << detect_threshold_bin;
		ar << extract_n_points;
		ar << detect_pre_threshold;
		ar << detect_refractory_period;
		ar << extract_channel;
		ar << extract_transform;
		ar << detect_what;
		ar << detect_mode;

		n_items = 1;
		ar << n_items;
		ar << detect_threshold_mv;
	}
	else
	{
		int nitems;  // string parameters
		ar >> nitems;
		ar >> comment; nitems--;
		ASSERT(nitems == 0);
		
		ar >> nitems; // int parameters
		ar >> detect_from; nitems--;
		ar >> compensate_baseline; nitems--;
		ar >> detect_channel; nitems--;
		ar >> detect_transform; nitems--;
		ar >> detect_threshold_bin; nitems--;
		ar >> extract_n_points; nitems--;
		ar >> detect_pre_threshold; nitems--;
		ar >> detect_refractory_period; nitems--;
		ar >> extract_channel; nitems--;
		ar >> extract_transform; nitems--;
		ar >> detect_what; nitems--;
		ar >> detect_mode; nitems--;
		ASSERT(nitems == 0);
		
		ar >> nitems; // float
		ar >> detect_threshold_mv; nitems--;
		ASSERT(nitems == 0);
	}
}

void options_detect_spikes::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		Serialize_v7(ar);
	}
	else
	{
		WORD version; ar >> version;
		if (version == 7)
			Serialize_v7(ar);
		else if (version > 0 && version < 5)
			Read_v5(ar, version);
		else if (version < 7) //== 6)
			Read_v6(ar);
		else
		{
			ASSERT(FALSE);
			CString message;
			message.Format(_T("Error: version (%i) not recognized"), version);
			AfxMessageBox(message, MB_OK);
		}
	}
}
