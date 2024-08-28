#pragma once


#define EXPORT_PSTH			0
#define EXPORT_ISI			1
#define EXPORT_AUTOCORR		2
#define EXPORT_LATENCY		3
#define EXPORT_AVERAGE		4
#define EXPORT_INTERV		5
#define EXPORT_EXTREMA		6
#define EXPORT_AMPLIT		7
#define EXPORT_HISTAMPL		8
#define EXPORT_SPIKEPOINTS	9
#include "ColorNames.h"

class options_view_spikes final : public CObject
{
	DECLARE_SERIAL(options_view_spikes)
	options_view_spikes();
	~options_view_spikes() override;
	options_view_spikes& operator =(const options_view_spikes& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL b_changed{false}; // flag set TRUE if contents has changed
	WORD w_version{3}; // version number
	int i_chan{0}; // data channel from which we have detected spikes

	float time_start{0.f}; // interval definition
	float time_end{2.f}; //
	float time_bin{.1f}; // bin size (sec)
	float hist_ampl_v_max{1.f};
	float hist_ampl_v_min{0.f};

	int hist_ampl_n_bins{20};
	int n_bins{20}; // number of bins
	int class_nb{0}; // class nb
	int class_nb_2{1};
	int i_stimulus_index{0};
	int export_data_type{0}; // export 0=psth, 1=isi, 2=auto_corr, 3=intervals, 4=extrema, 5=max-min, 6=hist ampl
	int spike_class_option{0}; // spike class: -1(one:selected){}; 0(all){}; 1(all:split)

	BOOL b_acq_comments{true}; // source data comments
	BOOL b_acq_date{true}; // source data time and date
	BOOL b_acq_ch_settings{false}; // source data acq chan settings
	BOOL b_spk_comments{false}; // spike file comments
	BOOL b_all_files{false}; // export number of spikes / interval
	BOOL b_export_zero{false}; // when exporting histogram, export zero (true) or empty cell (false)
	BOOL b_export_pivot{false}; // when exporting to excel, export pivot or not
	BOOL b_export_to_excel{false}; // export to Excel (1/0)
	BOOL b_artefacts{true}; // eliminate artefacts (class < 0)
	BOOL b_column_header{true}; // column headers ON/OFF
	BOOL b_total_spikes{true}; // total nb of spikes
	BOOL b_absolute_time{true}; // TRUE= absolute spk time, FALSE=relative to first stimulus
	BOOL b_incr_flag_on_save{false}; // increment database flag when spike file is saved
	BOOL b_spike_detect_threshold{true}; // spike detection threshold

	int n_comment_columns{1}; // number of comment columns in the table exported to excel

	// print parameters
	int height_line{130}; 
	int height_separator{20}; // separator height between classes
	BOOL b_display_bars{true};
	BOOL b_display_shapes{true}; 
	BOOL b_split_classes{true};
	BOOL b_all_channels{true}; // all spike channels (FALSE: only current)

	// histogram and dot display
	int dot_height{3};
	int dot_line_height{5};
	BOOL b_dot_underline{false};
	int n_bins_isi{100};
	float bin_isi{2.f};
	BOOL b_y_max_auto{true};
	float y_max{1.f};

	COLORREF cr_hist_fill{col_black};
	COLORREF cr_hist_border{col_black};
	COLORREF cr_stimulus_fill{col_blue};
	COLORREF cr_stimulus_border{col_blue};
	COLORREF cr_chart_area{col_white};
	COLORREF cr_scale[18]{};

	float f_scale_max{100.f};
	int b_hist_type{0}; // type of histogram requested (PSTH, ISI, ...)
	BOOL b_cycle_hist{false}; // cycle histogram (TRUE/FALSE), default=FALSE
	int n_stimuli_per_cycle{1}; 
};
