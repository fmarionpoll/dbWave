#include "StdAfx.h"
#include "OPTIONS_OUTPUTDATA.h"


IMPLEMENT_SERIAL(OPTIONS_OUTPUTDATA, CObject, 0 /* schema number*/)

OPTIONS_OUTPUTDATA::OPTIONS_OUTPUTDATA()
= default;

OPTIONS_OUTPUTDATA::~OPTIONS_OUTPUTDATA()
= default;

OPTIONS_OUTPUTDATA& OPTIONS_OUTPUTDATA::operator =(const OPTIONS_OUTPUTDATA& arg)
{
	if (this != &arg)
	{
		w_version = arg.w_version;
		b_allow_output_data = arg.b_allow_output_data;
		cs_output_file = arg.cs_output_file;
		b_preset_wave = arg.b_preset_wave;
		da_buffer_length = arg.da_buffer_length;
		da_n_buffers = arg.da_n_buffers;
		da_trigger_mode = arg.da_trigger_mode;
		da_frequency_per_channel = arg.da_frequency_per_channel;

		const int n_channels = arg.output_parms_array.GetSize();
		output_parms_array.SetSize(n_channels);
		for (int i = 0; i < n_channels; i++)
			output_parms_array[i] = arg.output_parms_array[i];
	}
	return *this;
}

void OPTIONS_OUTPUTDATA::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << w_version;

		ar << static_cast<WORD>(1); // CString
		ar << cs_output_file;

		ar << static_cast<WORD>(2); // BOOL
		ar << b_allow_output_data;
		ar << b_preset_wave;

		ar << static_cast<WORD>(3); // int
		ar << da_buffer_length;
		ar << da_n_buffers;
		ar << da_trigger_mode;

		ar << static_cast<WORD>(1); // double
		ar << da_frequency_per_channel;

		const int n_channels = output_parms_array.GetSize();
		ar << static_cast<WORD>(n_channels);
		for (int i = 0; i < n_channels; i++)
			output_parms_array.GetAt(i).Serialize(ar);

		ar << static_cast<WORD>(0); // no more ...
	}
	else
	{
		WORD version;
		ar >> version;
		WORD wn;

		// string parameters
		ar >> wn;
		int n = wn;
		if (n > 0) ar >> cs_output_file;
		n--;
		CString cs_dummy;
		while (n > 0)
		{
			n--;
			ar >> cs_dummy;
		}

		// BOOL parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> b_allow_output_data;
		n--;
		if (n > 0) ar >> b_preset_wave;
		n--;
		BOOL b_dummy;
		while (n > 0)
		{
			n--;
			ar >> b_dummy;
		}

		// int parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> da_buffer_length;
		n--;
		if (n > 0) ar >> da_n_buffers;
		n--;
		if (n > 0) ar >> da_trigger_mode;
		n--;
		int i_dummy;
		while (n > 0)
		{
			n--;
			ar >> i_dummy;
		}

		// double parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> da_frequency_per_channel;
		n--;
		double d_dummy;
		while (n > 0)
		{
			n--;
			ar >> d_dummy;
		}

		// output_parms
		ar >> wn;
		n = wn;
		if (n > output_parms_array.GetSize())
			output_parms_array.SetSize(n);
		for (int i = 0; i < n; i++)
			output_parms_array.GetAt(i).Serialize(ar);

		// other?
		ar >> wn;
		n = wn;
		ASSERT(n == 0);
	}
}
