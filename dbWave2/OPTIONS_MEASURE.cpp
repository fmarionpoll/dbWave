#include "StdAfx.h"
#include "OPTIONS_MEASURE.h"

IMPLEMENT_SERIAL(options_measure, CObject, 0 /* schema number*/)

options_measure::options_measure()
= default;

options_measure::~options_measure()
= default;

options_measure& options_measure::operator =(const options_measure& arg)
{
	if (this != &arg)
	{
		b_changed = arg.b_changed;

		// parameters to measure
		b_extrema = arg.b_extrema; // measure max and min (value, time)
		b_diff_extrema = arg.b_diff_extrema; // difference between extrema (value, time)
		b_diff_data_limits = arg.b_diff_data_limits; // diff between tags/cursors
		b_half_rise_time = arg.b_half_rise_time; // time necessary to reach half of the amplitude
		b_half_recovery = arg.b_half_recovery; // time necessary to regain half of the initial amplitude
		b_data_limits = arg.b_data_limits; // data value at limits
		b_time = arg.b_time;
		b_keep_tags = arg.b_keep_tags; // keep flags after measurement
		b_save_tags = arg.b_save_tags;
		b_set_tags_for_complete_file = arg.b_set_tags_for_complete_file;

		// source data
		b_all_channels = arg.b_all_channels; // measure on all chans (TRUE) or on single chan (FALSE)
		w_source_channel = arg.w_source_channel; // if bAllChans=FALSE, source data chan
		b_all_files = arg.b_all_files; // measure on all files (default=FALSE)

		// limits
		w_option = arg.w_option; // 0=between vertic tags, 1=between horiz lines, 2=relative to stimulus
		w_stimulus_channel = arg.w_stimulus_channel; // if wOption=2: stimulus channel
		w_stimulus_threshold = arg.w_stimulus_threshold; // if wOption=2: stimulus threshold
		f_stimulus_offset = arg.f_stimulus_offset; // if wOption=2: offset interval (default=0)
		w_limit_sup = arg.w_limit_sup; // rectangle limits
		w_limit_inf = arg.w_limit_inf;
		l_limit_left = arg.l_limit_left;
		l_limit_right = arg.l_limit_right;

		// dlg edit VT
		duration = arg.duration; // on/OFF duration (sec)
		period = arg.period; // period (sec)
		n_periods = arg.n_periods; // nb of duplicates
		time_shift = arg.time_shift; // shift tags
	}
	return *this;
}

void options_measure::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) // save data .....................
	{
		ar << w_version;

		// measure options
		WORD wMeasureFlags;
		WORD wMult;
		wMult = 1;
		wMeasureFlags = b_extrema * wMult;
		wMult += wMult;
		wMeasureFlags += b_diff_extrema * wMult;
		wMult += wMult;
		wMeasureFlags += b_half_rise_time * wMult;
		wMult += wMult;
		wMeasureFlags += b_half_recovery * wMult;
		wMult += wMult;
		wMeasureFlags += b_data_limits * wMult;
		wMult += wMult;
		wMeasureFlags += b_diff_data_limits * wMult;
		wMult += wMult;
		wMeasureFlags += b_time * wMult;
		wMult += wMult;
		wMeasureFlags += b_keep_tags * wMult;
		wMult += wMult;
		wMeasureFlags += b_save_tags * wMult;
		wMult += wMult;
		wMeasureFlags += b_set_tags_for_complete_file * wMult;
		ar << wMeasureFlags;

		// source data
		ar << static_cast<WORD>(b_all_channels);
		ar << w_source_channel;
		ar << static_cast<WORD>(b_all_files);

		// limits
		ar << w_option;
		ar << w_stimulus_channel << w_stimulus_threshold << f_stimulus_offset;
		ar << w_limit_sup << w_limit_inf << l_limit_left << l_limit_right;

		// dlg edit VT tags
		ar << duration; // on/OFF duration (sec)
		ar << period; // period (sec)
		ar << n_periods; // nb of duplicates
		ar << time_shift; // shift tags
	}
	else // read data ...................
	{
		WORD version;
		ar >> version;
		// measure options version 1
		WORD wMeasureFlags;
		WORD wMult;
		ar >> wMeasureFlags;
		wMult = 1;
		b_extrema = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_diff_extrema = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_half_rise_time = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_half_recovery = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_data_limits = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_diff_data_limits = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_time = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_keep_tags = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_save_tags = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		b_set_tags_for_complete_file = ((wMeasureFlags & wMult) > 0);

		// source data
		WORD w1;
		ar >> w1;
		b_all_channels = w1;
		ar >> w_source_channel;
		ar >> w1;
		b_all_files = w1;

		// limits
		ar >> w_option;
		ar >> w_stimulus_channel >> w_stimulus_threshold >> f_stimulus_offset;
		ar >> w_limit_sup >> w_limit_inf >> l_limit_left >> l_limit_right;

		// dlg edit VT tags - parameters
		if (version > 1)
		{
			ar >> duration; // on/OFF duration (sec)
			ar >> period; // period (sec)
			ar >> n_periods; // nb of duplicates
			ar >> time_shift; // shift tags
		}
	}
}
