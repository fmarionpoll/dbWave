#pragma once

constexpr auto MEASURE_VERTICAL = 0;	// vertical tags
constexpr auto MEASURE_HORIZONTAL = 1;	// horizontal lines
constexpr auto MEASURE_RECTANGLE = 3;
constexpr auto MEASURE_STIMULUS = 3;

class options_measure final : public CObject
{
	DECLARE_SERIAL(options_measure)
	options_measure();
	~options_measure() override;
	options_measure& operator =(const options_measure& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL b_changed{false}; // flag set TRUE if contents has changed
	WORD w_version{2}; // version number

	// parameters to measure
	BOOL b_extrema{true}; // 1- measure max and min (value, time)
	BOOL b_diff_extrema{true}; // 2- difference between extrema (value, time)
	BOOL b_half_rise_time{false}; // 3- time necessary to reach half of the amplitude
	BOOL b_half_recovery{false}; // 4- time necessary to regain half of the initial amplitude
	BOOL b_data_limits{true}; // 5- data value at limits
	BOOL b_diff_data_limits{true}; // 6- diff between data limits
	BOOL b_time{false}; // 7- time value of 1, 2, 5
	BOOL b_keep_tags{true}; // 8- don't erase flags after measurement
	BOOL b_save_tags{true}; // 9- set flag "modified" into assoc doc
	BOOL b_set_tags_for_complete_file{false}; // 10- dialog box option

	// source data
	BOOL b_all_channels{false}; // measure on all channels (TRUE) or on single chan (FALSE)
	WORD w_source_channel{0}; // if b_All_Channels=FALSE, source data chan
	BOOL b_all_files{false}; // measure on all files (default=FALSE)

	// limits
	WORD w_option{2}; // MEASURE_VERTICAL=vertical tags, MEASURE_HORIZONTAL=horizontal lines; MEASURE_RECTANGLE=rectangle; MEASURE_STIMULUS=relative to stimulus
	WORD w_stimulus_channel{0}; // if wOption=2: stimulus channel
	WORD w_stimulus_threshold{2048}; // if wOption=2: stimulus threshold
	float f_stimulus_offset{0.f}; // if wOption=2: offset interval (default=0)
	WORD w_limit_sup{0}; // rectangle limits
	WORD w_limit_inf{1};
	long l_limit_left{0};
	long l_limit_right{1};

	// HZ tags options
	float duration{.8f}; // on/OFF duration (sec)
	float period{3.f}; // period (sec)
	int n_periods{10}; // nb of duplicates
	float time_shift{0.f}; // shift tags
};
