#pragma once

#include "options_scope_struct.h"


class options_view_data final : public CObject
{
	DECLARE_SERIAL(options_view_data)
	options_view_data();
	~options_view_data() override;
	options_view_data& operator =(const options_view_data& arg);
	void Serialize(CArchive& ar) override;

	BOOL b_changed{false}; // flag set TRUE if contents has changed
	WORD m_w_version{5}; // version number

	// print options
	BOOL b_acq_comment{true}; // print data acquisition comments
	BOOL b_acq_date_time{true}; // data acquisition date and time
	BOOL b_channel_comment{true}; // individual data acq channel description
	BOOL b_channel_settings{true}; // amplifier's settings (CyberAmp conventions)
	BOOL b_doc_name{true}; // file name
	BOOL b_frame_rect{false}; // draw a rectangle frame around the data
	BOOL b_clip_rect{true}; // clip data to the rectangle
	BOOL b_time_scale_bar{true}; // display time scale bar
	BOOL b_voltage_scale_bar{true}; // display voltage scale bar
	BOOL b_print_selection{true}; // print only selection or whole series
	BOOL b_print_spk_bars{true}; // for spike view, print bars or not
	BOOL b_filter_data_source{true}; // display source data filtered

	// print parameters
	int horizontal_resolution{1}; // horizontal printer resolution
	int vertical_resolution{1}; // vertical printer resolution
	int height_doc{300}; // height of one document
	int width_doc{1500}; // width of one document
	int left_page_margin{100}; // page left margin (pixels)
	int bottom_page_margin{40}; // bottom
	int right_page_margin{40}; // right
	int top_page_margin{100}; // top
	int height_separator{40}; // vertical space between consecutive displays
	int text_separator{40}; // space between left margin of display and text
	int font_size{40}; // font size (pixels)
	int spike_height{0}; //
	int spike_width{0}; //

	// how data are displayed from file to file
	BOOL b_all_channels{true}; // display all channels
	BOOL b_complete_record{true}; // display entire record
	BOOL b_multiple_rows{false}; // display one record on consecutive lines
	BOOL b_split_curves{true}; // arrange curves so that they are distributed vertically
	BOOL b_maximize_gain{true}; // maximize gain
	BOOL b_center_curves{true}; // center curves
	BOOL b_keep_for_each_file{true}; // keep parameters for each file
	BOOL b_display_old_detect_p{false}; // spk detection params: display old (true), keep same
	BOOL b_filter_dat{false}; // for db_View_data
	BOOL b_detect_while_browse{true}; // for spk_detection_view
	BOOL b_goto_record_id{true}; // for dbWaveView
	BOOL b_display_file_name{true}; // for dbViewData
	BOOL b_set_time_span{false};
	BOOL b_set_mv_span{false};
	BOOL b_display_all_classes{true};

	// export infos from data files
	BOOL b_acq_comments{true}; // global comment
	BOOL b_acq_date{true}; // acquisition date
	BOOL b_acq_time{true}; // acquisition time
	BOOL b_file_size{false}; // file size
	BOOL b_acq_channel_comment{false}; // acq channel individual comment
	BOOL b_acq_channel_setting{false}; // acq chan individual settings (gain, filter, etc.)
	BOOL b_units{true}; // TRUE=mV, FALSE=binary
	BOOL b_contours{false}; // TRUE=contours, FALSE=center
	BOOL b_graphics{true}; // TRUE=graphics (enh metafile), FALSE=ASCII
	BOOL b_data_base_columns{true}; // TRUE= export content of database columns
	BOOL b_to_excel{false};

	int hz_resolution{600}; // int item #1: horizontal size of exported data (points)
	int vt_resolution{400}; // vertical size of exported data (points)
	int unit_mode{0}; // provisional
	int n_filter_index{0}; // index list item in file_open dialog m_n_filter_index

	// overview list parameters
#define N_COLUMNS	11
	CUIntArray i_column_width{};
	int i_row_height{50};

	// save scope_screen item
	int n_scope_items{0};
	options_scope_struct view_data{};
	options_scope_struct view_spk_detect_filtered{};
	options_scope_struct view_spk_detect_data{};
	options_scope_struct view_spk_detect_spk{};
	options_scope_struct view_spk_detect_bars{};
	options_scope_struct view_ad_continuous{};
	options_scope_struct spk_view_data{};
	options_scope_struct spk_view_spk{};
	options_scope_struct spk_view_bars{};
	options_scope_struct spk_sort1_spk{};
	options_scope_struct spk_sort1_parameters{};
	options_scope_struct spk_sort1_bars{};
	options_scope_struct spk_sort1_hist{};
	options_scope_struct db_view_data{};
	options_scope_struct db_view_bars{};
	options_scope_struct db_view_shape{};

	// splitter settings
	int col0_width{200};
	int row0_height{50};
	BOOL b_col0_visible{true};
	BOOL b_row0_visible{true};

	// int parameters - spike class
	int spike_class{0};

	// database list view settings
	float t_first{0.f};
	float t_last{0.f};
	float mv_span{0.f};
	int display_mode{1};
};
