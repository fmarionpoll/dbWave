#include "StdAfx.h"
#include "options_view_spikes.h"

#include "ColorNames.h"


IMPLEMENT_SERIAL(options_view_spikes, CObject, 0 /* schema number*/)

options_view_spikes::options_view_spikes()
{
	int i = 18;
	i--;
	cr_scale[i] = RGB(35, 31, 28); //RGB(255,255,255);
	i--;
	cr_scale[i] = RGB(213, 45, 31);
	i--;
	cr_scale[i] = RGB(222, 100, 19);
	i--;
	cr_scale[i] = RGB(232, 142, 33);
	i--;
	cr_scale[i] = RGB(243, 196, 0);
	i--;
	cr_scale[i] = RGB(226, 237, 0);
	i--;
	cr_scale[i] = RGB(142, 201, 33);
	i--;
	cr_scale[i] = RGB(64, 174, 53);
	i--;
	cr_scale[i] = RGB(0, 152, 62);
	i--;
	cr_scale[i] = RGB(0, 142, 85);
	i--;
	cr_scale[i] = RGB(0, 142, 123);
	i--;
	cr_scale[i] = RGB(0, 142, 163);
	i--;
	cr_scale[i] = RGB(0, 139, 206);
	i--;
	cr_scale[i] = RGB(0, 117, 190);
	i--;
	cr_scale[i] = RGB(0, 90, 158);
	i--;
	cr_scale[i] = RGB(29, 62, 133);
	i--;
	cr_scale[i] = RGB(49, 20, 105);
	i--;
	cr_scale[i] = RGB(255, 255, 255); //RGB(35,31,28);
	ASSERT(i >= 0);
}

options_view_spikes::~options_view_spikes()
= default;

options_view_spikes& options_view_spikes::operator =(const options_view_spikes& arg)
{
	if (this != &arg)
	{
		time_start = arg.time_start;
		time_end = arg.time_end;
		time_bin = arg.time_bin;
		hist_ampl_v_max = arg.hist_ampl_v_max;
		hist_ampl_v_min = arg.hist_ampl_v_min;
		hist_ampl_n_bins = arg.hist_ampl_n_bins;

		n_bins = arg.n_bins;
		class_nb = arg.class_nb;
		class_nb_2 = arg.class_nb_2;
		b_acq_comments = arg.b_acq_comments;
		b_acq_date = arg.b_acq_date;
		b_acq_ch_settings = arg.b_acq_ch_settings;
		b_spk_comments = arg.b_spk_comments;
		export_data_type = arg.export_data_type;
		spike_class_option = arg.spike_class_option;
		b_artefacts = arg.b_artefacts;
		b_column_header = arg.b_column_header;
		b_total_spikes = arg.b_total_spikes;
		b_absolute_time = arg.b_absolute_time;
		b_export_zero = arg.b_export_zero;
		b_all_channels = arg.b_all_channels;

		height_line = arg.height_line;
		b_display_bars = arg.b_display_bars;
		b_display_shapes = arg.b_display_shapes;
		height_separator = arg.height_separator;
		b_split_classes = arg.b_split_classes;
		b_all_files = arg.b_all_files;
		dot_height = arg.dot_height;
		dot_line_height = arg.dot_line_height;
		b_dot_underline = arg.b_dot_underline;
		n_bins_isi = arg.n_bins_isi;
		bin_isi = arg.bin_isi;
		b_y_max_auto = arg.b_y_max_auto;
		y_max = arg.y_max;

		cr_hist_fill = arg.cr_hist_fill;
		cr_hist_border = arg.cr_hist_border;
		cr_stimulus_fill = arg.cr_stimulus_fill;
		cr_stimulus_border = arg.cr_stimulus_border;
		cr_chart_area = arg.cr_chart_area;
		for (int i = 0; i < 18; i++)
			cr_scale[i] = arg.cr_scale[i];
		f_scale_max = arg.f_scale_max;
		i_stimulus_index = arg.i_stimulus_index;
		b_hist_type = arg.b_hist_type;
		b_cycle_hist = arg.b_cycle_hist;
		n_stimuli_per_cycle = arg.n_stimuli_per_cycle;
	}
	return *this;
}

void options_view_spikes::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << w_version;

		ar << time_start << time_end << time_bin;
		ar << static_cast<WORD>(n_bins);
		ar << static_cast<WORD>(class_nb); // classnb2 is added to the series of int parameters
		WORD w_e, w_m;

		w_m = 1;
		w_e = b_acq_comments * w_m; //1
		w_m += w_m;
		w_e += b_acq_date * w_m; //2
		w_m += w_m;
		w_e += b_acq_comments * w_m; //4
		w_m += w_m;
		w_e += b_acq_date * w_m; //8
		w_m += w_m;
		w_e += b_acq_ch_settings * w_m; //16
		w_m += w_m;
		w_e += b_spk_comments * w_m; //32
		w_m += w_m;
		w_e += b_incr_flag_on_save * w_m; //64
		w_m += w_m;
		w_e += b_export_zero * w_m; //128
		w_m += w_m;
		w_e += b_export_pivot * w_m; //256
		w_m += w_m;
		w_e += b_export_to_excel * w_m; //512	empty slot
		w_m += w_m;
		w_e += b_artefacts * w_m; //1024
		w_m += w_m;
		w_e += b_column_header * w_m; //2048
		w_m += w_m;
		w_e += b_total_spikes * w_m; //4096
		w_m += w_m;
		w_e += b_absolute_time * w_m; //8192
		w_m += w_m;
		w_e += b_all_files * w_m; //16384
		w_m += w_m;
		w_e += b_dot_underline * w_m; //32768
		ar << w_e;

		// int parameters
		WORD w_int = 11;
		ar << w_int; // set to nb of following lines
		ar << static_cast<WORD>(height_line); // 1
		ar << static_cast<WORD>(height_separator); // 2
		ar << static_cast<WORD>(dot_height); // 3
		ar << static_cast<WORD>(dot_line_height); // 4
		ar << static_cast<WORD>(n_bins_isi); // 5
		ar << static_cast<WORD>(i_stimulus_index); // 6
		ar << static_cast<WORD>(b_hist_type); // 7
		ar << n_stimuli_per_cycle; // 8
		ar << export_data_type; // 9
		ar << spike_class_option; // 10
		ar << class_nb_2; // 11

		WORD wnb = 0;
		w_m = 1;
		w_e = b_display_bars * w_m;
		wnb++; // 1
		w_m += w_m;
		w_e += b_display_shapes * w_m;
		wnb++; // 2: 2
		w_m += w_m;
		w_e += b_split_classes * w_m;
		wnb++; // 3: 4
		w_m += w_m;
		w_e += b_y_max_auto * w_m;
		wnb++; // 4: 8
		w_m += w_m;
		w_e += b_cycle_hist * w_m;
		wnb++; // 5: 16
		w_m += w_m;
		w_e += b_all_channels * w_m;
		wnb++; // 6: 32
		ar << wnb; // set to nb of bool parameters
		ar << w_e; // set compressed BOOL data

		//float parameters or 32 bits values
		WORD w_n_float = 29;
		ar << w_n_float;
		ar << hist_ampl_v_max; // 1
		ar << hist_ampl_v_min; // 2
		ar << hist_ampl_n_bins; // 3
		ar << bin_isi; // 4
		ar << y_max; // 5
		ar << cr_hist_fill; // 6
		ar << cr_hist_border; // 7
		ar << cr_stimulus_fill; // 8
		ar << cr_stimulus_border; // 9
		ar << cr_chart_area; // 10
		for (unsigned long i : cr_scale)
			ar << i;
		ar << f_scale_max; // 29
	}
	else
	{
		ar >> w_version;

		WORD w1;
		ar >> time_start >> time_end >> time_bin;
		ar >> w1;
		n_bins = w1;
		ar >> w1;
		class_nb = w1;

		// print options
		WORD w_e;
		WORD w_m = 1;
		ar >> w_e;
		w_m = 1;
		b_acq_comments = ((w_e & w_m) != 0); //1
		w_m += w_m;
		b_acq_date = ((w_e & w_m) != 0); //2
		w_m += w_m;
		b_acq_comments = ((w_e & w_m) != 0); //4
		w_m += w_m;
		b_acq_date = ((w_e & w_m) != 0); //8
		w_m += w_m;
		b_acq_ch_settings = ((w_e & w_m) != 0); //16
		w_m += w_m;
		b_spk_comments = ((w_e & w_m) != 0); //32
		w_m += w_m;
		b_incr_flag_on_save = ((w_e & w_m) != 0); //64
		// this parameter does not belong to print options but to export options
		w_m += w_m;
		b_export_zero = ((w_e & w_m) > 0); //128
		w_m += w_m;
		b_export_pivot = ((w_e & w_m) > 0); //256
		w_m += w_m;
		b_export_to_excel = ((w_e & w_m) != 0); //512

		w_m += w_m;
		b_artefacts = ((w_e & w_m) != 0); //1024
		w_m += w_m;
		b_column_header = ((w_e & w_m) != 0); //2048
		w_m += w_m;
		b_total_spikes = ((w_e & w_m) != 0); //4096
		w_m += w_m;
		b_absolute_time = ((w_e & w_m) != 0); //8192
		w_m += w_m;
		b_all_files = ((w_e & w_m) != 0); //16384
		w_m += w_m;
		b_dot_underline = ((w_e & w_m) != 0); //32768

		// int parameters
		int nb;
		ar >> w1;
		nb = w1; // number of int parameters
		ar >> w1;
		height_line = w1;
		nb--; // 1
		ar >> w1;
		height_separator = w1;
		nb--; // 2
		if (nb > 0)
		{
			ar >> w1;
			dot_height = w1;
			nb--;
		} // 3
		if (nb > 0)
		{
			ar >> w1;
			dot_line_height = w1;
			nb--;
		} // 4
		if (nb > 0)
		{
			ar >> w1;
			n_bins_isi = w1;
			nb--;
		} // 5
		if (nb > 0)
		{
			ar >> w1;
			i_stimulus_index = w1;
			nb--;
		} // 6
		if (nb > 0)
		{
			ar >> w1;
			b_hist_type = w1;
			nb--;
		} // 7
		if (nb > 0)
		{
			ar >> n_stimuli_per_cycle;
			nb--;
		} // 8
		if (nb > 0)
		{
			ar >> export_data_type;
			nb--;
		} // 9
		if (nb > 0)
		{
			ar >> spike_class_option;
			nb--;
		} // 10
		if (nb > 0)
		{
			ar >> class_nb_2;
			nb--;
		}
		else { class_nb_2 = class_nb; } // 11
		while (nb > 0)
		{
			ar >> w1;
			nb--;
		}

		ar >> w1;
		nb = w1; // number of bool parameters
		ar >> w_e; // data -> decompress
		// version 1 (20-4-96): 3 parameters
		w_m = 1;
		b_display_bars = ((w_e & w_m) > 0);
		nb--; // 1
		w_m += w_m;
		b_display_shapes = ((w_e & w_m) > 0);
		nb--; // 2:2
		w_m += w_m;
		b_split_classes = ((w_e & w_m) > 0);
		nb--; // 3:4
		if (nb > 0)
		{
			w_m += w_m;
			b_y_max_auto = ((w_e & w_m) > 0);
			nb--;
		} // 4:8
		if (nb > 0)
		{
			w_m += w_m;
			b_cycle_hist = ((w_e & w_m) > 0);
			nb--;
		} // 5:16
		if (nb > 0)
		{
			w_m += w_m;
			b_all_channels = ((w_e & w_m) > 0);
			nb--;
		} // 6:32
		while (nb > 0)
		{
			ar >> w_e;
			nb--;
		}

		if (w_version > 2)
		{
			ar >> w1;
			nb = w1;
			ar >> hist_ampl_v_max;
			nb--;
			ar >> hist_ampl_v_min;
			nb--;
			ar >> hist_ampl_n_bins;
			nb--;
			if (nb > 0)
			{
				ar >> bin_isi;
				nb--;
			}
			if (nb > 0)
			{
				ar >> y_max;
				nb--;
			}
			if (nb > 0)
			{
				ar >> cr_hist_fill;
				nb--;
			}
			if (nb > 0)
			{
				ar >> cr_hist_border;
				nb--;
			}
			if (nb > 0)
			{
				ar >> cr_stimulus_fill;
				nb--;
			}
			if (nb > 0)
			{
				ar >> cr_stimulus_border;
				nb--;
			}
			if (nb > 0)
			{
				ar >> cr_chart_area;
				nb--;
			}
			COLORREF cr_dummy;
			for (int i = 0; i < 18; i++)
				//if (nb > 0) {ar >> crScale[i]; nb--;}
				if (nb > 0)
				{
					ar >> cr_dummy;
					nb--;
				}
			if (nb > 0)
			{
				ar >> f_scale_max;
				nb--;
			}
		}
	}
}
