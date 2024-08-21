#include "StdAfx.h"
#include "options_detect_spikes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(options_detect_spikes, CObject, 0 /* schema number*/)

options_detect_spikes::options_detect_spikes()
= default;

options_detect_spikes::~options_detect_spikes()
= default;

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

void options_detect_spikes::read_v5(CArchive& ar, const int version)
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
		// removed at version 3, moved to SPK_detect_ARRAY
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

void options_detect_spikes::read_v6(CArchive& ar)
{
	int n_items;
	ar >> n_items;
	ar >> comment; n_items--;
	ASSERT(n_items == 0);

	ar >> n_items;
	ar >> detect_from; n_items--;
	ar >> compensate_baseline; n_items--;
	ASSERT(n_items == 0);

	ar >> n_items;
	ar >> detect_channel; n_items--;
	ar >> detect_transform; n_items--;
	ar >> detect_threshold_bin; n_items--;
	ar >> extract_n_points; n_items--;
	ar >> detect_pre_threshold; n_items--;
	ar >> detect_refractory_period; n_items--;
	ar >> extract_channel; n_items--;
	ar >> extract_transform; n_items--;
	if (n_items > 0) ar >> detect_what; n_items--;
	if (n_items > 0) ar >> detect_mode; n_items--;

	ar >> n_items;
	ar >> detect_threshold_mv; n_items--;
	ASSERT(n_items == 0);
}

void options_detect_spikes::serialize_v7(CArchive& ar)
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
		int n_items;  
		ar >> n_items;
		ar >> comment; n_items--;
		ASSERT(n_items == 0);
		
		ar >> n_items; // int parameters
		ar >> detect_from; n_items--;
		ar >> compensate_baseline; n_items--;
		ar >> detect_channel; n_items--;
		ar >> detect_transform; n_items--;
		ar >> detect_threshold_bin; n_items--;
		ar >> extract_n_points; n_items--;
		ar >> detect_pre_threshold; n_items--;
		ar >> detect_refractory_period; n_items--;
		ar >> extract_channel; n_items--;
		ar >> extract_transform; n_items--;
		ar >> detect_what; n_items--;
		ar >> detect_mode; n_items--;
		ASSERT(n_items == 0);
		
		ar >> n_items; // float
		ar >> detect_threshold_mv; n_items--;
		ASSERT(n_items == 0);
	}
}

void options_detect_spikes::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		serialize_v7(ar);
	}
	else
	{
		WORD version; ar >> version;
		if (version == 7)
			serialize_v7(ar);
		else if (version > 0 && version < 5)
			read_v5(ar, version);
		else if (version < 7) //== 6)
			read_v6(ar);
		else
		{
			ASSERT(FALSE);
			CString message;
			message.Format(_T("Error: version (%i) not recognized"), version);
			AfxMessageBox(message, MB_OK);
		}
	}
}
