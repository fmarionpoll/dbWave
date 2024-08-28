#include "stdafx.h"
#include "options_detect_stimulus.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(options_detect_stimulus, CObject, 0)

options_detect_stimulus::options_detect_stimulus()
= default;

options_detect_stimulus::~options_detect_stimulus()
= default;

options_detect_stimulus& options_detect_stimulus::operator =(const options_detect_stimulus & arg)
{
	if (this != &arg)
	{
		//comment=arg.comment;	// CArchive
		n_items = arg.n_items;
		source_chan = arg.source_chan;
		transform_method = arg.transform_method;
		detect_method = arg.detect_method;
		threshold1 = arg.threshold1;
		b_mode = arg.b_mode;
	}
	return *this;
}

void options_detect_stimulus::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << w_version;
		ar << n_items;
		ar << source_chan;
		ar << transform_method;
		ar << detect_method;
		ar << threshold1;
		ar << b_mode;
	}
	else
	{
		WORD version;
		ar >> version;
		// read data (version 1)
		if (version == 1)
		{
			WORD w1;
			ar >> w1;
			n_items = w1;
			ar >> w1;
			source_chan = w1;
			ar >> w1;
			transform_method = w1;
			ar >> w1;
			detect_method = w1;
			ar >> w1;
			threshold1 = w1;
		}
		else if (version > 1)
		{
			ar >> n_items;
			ar >> source_chan;
			ar >> transform_method;
			ar >> detect_method;
			ar >> threshold1;
			ar >> b_mode;
		}
	}
}

