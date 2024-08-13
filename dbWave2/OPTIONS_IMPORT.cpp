#include "StdAfx.h"
#include "options_import.h"
#include "include/DataTranslation/Olxdadefs.h"


IMPLEMENT_SERIAL(options_import, CObject, 0)

options_import::options_import()
= default;

options_import::~options_import()
{
	SAFE_DELETE(p_wave_chan_array)
}

options_import& options_import::operator =(const options_import& arg)
{
	if (this != &arg)
	{
		w_version = arg.w_version;
		gain_fid = arg.gain_fid;
		gain_ead = arg.gain_ead;
		path_w_to_ascii = arg.path_w_to_ascii;

		// generic import options
		is_single_run = arg.is_single_run;
		preview_requested = arg.preview_requested;
		is_sapid_3_5 = arg.is_sapid_3_5;
		b_dummy = arg.b_dummy;
		nb_runs = arg.nb_runs;
		nb_channels = arg.nb_channels;
		sampling_rate = arg.sampling_rate;
		encoding_mode = arg.encoding_mode;
		bits_precision = arg.bits_precision;
		voltage_max = arg.voltage_max;
		voltage_min = arg.voltage_min;
		skip_n_bytes = arg.skip_n_bytes;
		title = arg.title;
		p_wave_chan_array->Copy( arg.p_wave_chan_array);
		n_selected_filter = arg.n_selected_filter;

		// export options
		export_type = arg.export_type;
		all_channels = arg.all_channels;
		separate_comments = arg.separate_comments;
		include_time = arg.include_time;
		selected_channel = arg.selected_channel;
		time_first = arg.time_first;
		time_last = arg.time_last;
		entire_file = arg.entire_file;
		i_under_sample = arg.i_under_sample;

		// others
		path = arg.path;
	}
	return *this;
}

void options_import::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		w_version = 6;
		ar << w_version;
		ar << gain_fid;
		ar << gain_ead;

		// generic data import options
		int flag = is_sapid_3_5; // combine flags: update flag, combine
		flag <<= 1;
		flag += preview_requested;
		flag <<= 1;
		flag += is_single_run;
		ar << flag;

		ar << static_cast<WORD>(nb_runs);
		ar << static_cast<WORD>(nb_channels);
		ar << sampling_rate;
		ar << static_cast<WORD>(encoding_mode);
		ar << static_cast<WORD>(bits_precision);
		ar << voltage_max;
		ar << voltage_min;
		ar << skip_n_bytes;
		ar << title;
		if (p_wave_chan_array == nullptr)
			initialize_wave_chan_array();
		p_wave_chan_array->Serialize(ar);
		ar << n_selected_filter;

		// generic data export options
		ar << export_type;
		flag = all_channels;
		flag <<= 1;
		flag += separate_comments;
		flag <<= 1;
		flag += entire_file;
		flag <<= 1;
		flag += include_time;
		ar << flag;

		ar << selected_channel;
		ar << time_first;
		ar << time_last;

		ar << path_w_to_ascii;

		// add extensible options
		int n_types = 2;
		ar << n_types;

		// int
		n_types = 2;
		ar << n_types;
		ar << i_under_sample;
		flag = b_dummy;
		flag <<= 1;
		flag += discard_duplicate_files;
		ar << flag;

		// CStrings
		n_types = 1;
		ar << n_types;
		ar << path;
	}
	else
	{
		WORD version;
		int flag;
		ar >> version;
		ar >> gain_fid;
		ar >> gain_ead;
		if (version >= 2)
		{
			WORD w1;
			ar >> flag;
			// decompose flags: update flag (/2),  get value, mask
			is_single_run = static_cast<boolean>(flag);
			is_single_run &= 0x1;
			flag >>= 1;
			preview_requested = static_cast<boolean>(flag);
			preview_requested &= 0x1;
			flag >>= 1;
			is_sapid_3_5 = static_cast<boolean>(flag);
			is_sapid_3_5 &= 0x1;

			ar >> w1;
			nb_runs = static_cast<short>(w1);
			ar >> w1;
			nb_channels = static_cast<short>(w1);
			ar >> sampling_rate;
			ar >> w1;
			encoding_mode = static_cast<short>(w1);
			ar >> w1;
			bits_precision = static_cast<short>(w1);
			ar >> voltage_max;
			ar >> voltage_min;
			ar >> skip_n_bytes;
			ar >> title;
			if (p_wave_chan_array == nullptr)
				initialize_wave_chan_array();
			p_wave_chan_array->Serialize(ar);
		}
		if (version >= 3)
			ar >> n_selected_filter;
		if (version >= 4)
		{
			ar >> export_type;
			ar >> flag;
			include_time = static_cast<boolean>(flag);
			include_time &= 0x1;
			flag >>= 1;
			entire_file = static_cast<boolean>(flag);
			entire_file &= 0x1;
			flag >>= 1;
			separate_comments = static_cast<boolean>(flag);
			separate_comments &= 0x1;
			flag >>= 1;
			all_channels = static_cast<boolean>(flag);
			all_channels &= 0x1;
			ar >> selected_channel;
			ar >> time_first;
			ar >> time_last;
		}
		if (version >= 5)
			ar >> path_w_to_ascii;
		if (version >= 6)
		{
			int n_types;
			ar >> n_types;
			if (n_types > 0)
			{
				int n_integers;
				ar >> n_integers;
				ar >> i_under_sample;
				n_integers--;
				if (n_integers > 0)
				{
					ar >> flag;
					n_integers--;
					discard_duplicate_files = static_cast<boolean>(flag);
					discard_duplicate_files &= 0x1;
					flag >>= 1;
					b_dummy = static_cast<boolean>(flag);
					b_dummy &= 0x1;
					flag >>= 1;
				}
			}
			n_types--;

			if (n_types > 0)
			{
				int n_strings;
				ar >> n_strings;
				ar >> path;
				n_strings--;
			}
			n_types--;
		}
	}
}
