#pragma once

#include "SCOPESTRUCT.h"


class OPTIONS_VIEW_DATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_VIEW_DATA)
	OPTIONS_VIEW_DATA();
	~OPTIONS_VIEW_DATA() override;
	OPTIONS_VIEW_DATA& operator =(const OPTIONS_VIEW_DATA& arg);
	void Serialize(CArchive& ar) override;

public:
	boolean b_changed{false}; // flag set TRUE if contents has changed
	WORD m_w_version{5}; // version number

	// print options
	boolean b_acq_comment{true}; // print data acquisition comments
	boolean b_acq_date_time{true}; // data acquisition date and time
	boolean b_channel_comment{true}; // individual data acq channel description
	boolean b_channel_settings{true}; // amplifier's settings (CyberAmp conventions)
	boolean b_doc_name{true}; // file name
	boolean b_frame_rect{false}; // draw a rectangle frame around the data
	boolean b_clip_rect{true}; // clip data to the rectangle
	boolean b_time_scale_bar{true}; // display time scale bar
	boolean b_voltage_scale_bar{true}; // display voltage scale bar
	boolean b_print_selection{true}; // print only selection or whole series
	boolean b_print_spk_bars{true}; // for spike view, print bars or not
	boolean b_filter_data_source{true}; // display source data filtered

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
	boolean b_all_channels{true}; // display all channels
	boolean b_complete_record{true}; // display entire record
	boolean b_multiple_rows{false}; // display one record on consecutive lines
	boolean b_split_curves{true}; // arrange curves so that they are distributed vertically
	boolean b_maximize_gain{true}; // maximize gain
	boolean b_center_curves{true}; // center curves
	boolean b_keep_for_each_file{true}; // keep parameters for each file
	boolean b_display_old_detect_p{false}; // spk detection params: display old (true), keep same
	boolean b_filter_dat{false}; // for db_View_data
	boolean b_detect_while_browse{true}; // for spk_detection_view
	boolean b_goto_record_id{true}; // for dbWaveView
	boolean b_display_file_name{true}; // for dbViewData
	boolean b_set_time_span{false};
	boolean b_set_mv_span{false};
	boolean b_display_all_classes{true};

	// export infos from data files
	boolean b_acq_comments{true}; // global comment
	boolean b_acq_date{true}; // acquisition date
	boolean b_acq_time{true}; // acquisition time
	boolean b_file_size{false}; // file size
	boolean b_acq_channel_comment{false}; // acq channel individual comment
	boolean b_acq_channel_setting{false}; // acq chan individual settings (gain, filter, etc.)
	boolean b_units{true}; // TRUE=mV, FALSE=binary
	boolean b_contours{false}; // TRUE=contours, FALSE=center
	boolean b_graphics{true}; // TRUE=graphics (enh metafile), FALSE=ASCII
	boolean b_data_base_columns{true}; // TRUE= export content of database columns
	boolean b_to_excel{false};

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
	SCOPESTRUCT view_data{};
	SCOPESTRUCT view_spk_detect_filtered{};
	SCOPESTRUCT view_spk_detect_data{};
	SCOPESTRUCT view_spk_detect_spk{};
	SCOPESTRUCT view_spk_detect_bars{};
	SCOPESTRUCT view_ad_continuous{};
	SCOPESTRUCT spk_view_data{};
	SCOPESTRUCT spk_view_spk{};
	SCOPESTRUCT spk_view_bars{};
	SCOPESTRUCT spk_sort1_spk{};
	SCOPESTRUCT spk_sort1_parameters{};
	SCOPESTRUCT spk_sort1_bars{};
	SCOPESTRUCT spk_sort1_hist{};
	SCOPESTRUCT db_view_data{};
	SCOPESTRUCT db_view_bars{};
	SCOPESTRUCT db_view_shape{};

	// splitter settings
	int col0_width{200};
	int row0_height{50};
	boolean b_col0_visible{true};
	boolean b_row0_visible{true};

	// int parameters - spike class
	int spike_class{0};

	// database list view settings
	float t_first{0.f};
	float t_last{0.f};
	float mv_span{0.f};
	int display_mode{1};
};
