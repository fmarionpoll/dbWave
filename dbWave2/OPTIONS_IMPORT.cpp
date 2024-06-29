#include "StdAfx.h"
#include "OPTIONS_IMPORT.h"
#include "include/DataTranslation/Olxdadefs.h"


IMPLEMENT_SERIAL(OPTIONS_IMPORT, CObject, 0)

OPTIONS_IMPORT::OPTIONS_IMPORT()
= default;

OPTIONS_IMPORT::~OPTIONS_IMPORT()
{
	SAFE_DELETE(p_wave_chan_array)
}

OPTIONS_IMPORT& OPTIONS_IMPORT::operator =(const OPTIONS_IMPORT& arg)
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

void OPTIONS_IMPORT::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		w_version = 6;
		ar << w_version;
		ar << gain_fid;
		ar << gain_ead;

		// generic data import options
		int bflag = is_sapid_3_5; // combine flags: update flag, combine
		bflag <<= 1;
		bflag += preview_requested;
		bflag <<= 1;
		bflag += is_single_run;
		ar << bflag;

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
		bflag = all_channels;
		bflag <<= 1;
		bflag += separate_comments;
		bflag <<= 1;
		bflag += entire_file;
		bflag <<= 1;
		bflag += include_time;
		ar << bflag;

		ar << selected_channel;
		ar << time_first;
		ar << time_last;

		ar << path_w_to_ascii;

		// add extensible options
		int ntypes = 2;
		ar << ntypes;

		// int
		ntypes = 2;
		ar << ntypes;
		ar << i_under_sample;
		bflag = b_dummy;
		bflag <<= 1;
		bflag += discard_duplicate_files;
		ar << bflag;

		// CStrings
		ntypes = 1;
		ar << ntypes;
		ar << path;
	}
	else
	{
		WORD version;
		int bflag;
		ar >> version;
		ar >> gain_fid;
		ar >> gain_ead;
		if (version >= 2)
		{
			WORD w1;
			ar >> bflag;
			// decompose flags: update flag (/2),  get value, mask
			is_single_run = bflag;
			is_single_run &= 0x1;
			bflag >>= 1;
			preview_requested = bflag;
			preview_requested &= 0x1;
			bflag >>= 1;
			is_sapid_3_5 = bflag;
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
			ar >> bflag;
			include_time = bflag;
			include_time &= 0x1;
			bflag >>= 1;
			entire_file = bflag;
			entire_file &= 0x1;
			bflag >>= 1;
			separate_comments = bflag;
			separate_comments &= 0x1;
			bflag >>= 1;
			all_channels = bflag;
			all_channels &= 0x1;
			ar >> selected_channel;
			ar >> time_first;
			ar >> time_last;
		}
		if (version >= 5)
			ar >> path_w_to_ascii;
		if (version >= 6)
		{
			int ntypes;
			ar >> ntypes;
			if (ntypes > 0)
			{
				int nints;
				ar >> nints;
				ar >> i_under_sample;
				nints--;
				if (nints > 0)
				{
					ar >> bflag;
					nints--;
					discard_duplicate_files = bflag;
					discard_duplicate_files &= 0x1;
					bflag >>= 1;
					b_dummy = bflag;
					b_dummy &= 0x1;
					bflag >>= 1;
				}
			}
			ntypes--;

			if (ntypes > 0)
			{
				int nstrings;
				ar >> nstrings;
				ar >> path;
				nstrings--;
			}
			ntypes--;
		}
	}
}
