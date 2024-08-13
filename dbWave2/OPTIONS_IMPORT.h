#pragma once
#include "AcqWaveChanArray.h"
#include <Olxdadefs.h>

class options_import final : public CObject
{
	DECLARE_SERIAL(options_import)
	options_import();
	~options_import() override;
	options_import& operator =(const options_import& arg);
	void Serialize(CArchive& ar) override;

public:
	boolean b_changed{false};
	WORD w_version{6};

	// Varian files
	float gain_fid{10.f};
	float gain_ead{10.f};
	CString path_w_to_ascii{};

	// generic files
	boolean is_single_run{true};
	boolean preview_requested{true};
	boolean is_sapid_3_5{false};
	boolean b_dummy{false};
	boolean discard_duplicate_files{false};
	boolean read_columns{false};
	boolean header_present{false};
	short nb_runs{1};
	short nb_channels{1};
	float sampling_rate{10.f};
	short encoding_mode{OLx_ENC_BINARY};
	short bits_precision{12};
	float voltage_max{10.f};
	float voltage_min{-10.f};
	int skip_n_bytes{5};
	CString title{};
	CWaveChanArray* p_wave_chan_array{nullptr};

	// last selected filter - index
	int n_selected_filter{0};
	// export options
	WORD export_type{2}; // type of export (0=sapid 3.5, 1=txt, 2=excel, ...)
	boolean all_channels{true}; // export all channels or one
	boolean separate_comments{false}; // comments in a separate file
	boolean entire_file{true}; // complete file / interval
	boolean include_time{true}; // include time steps (excel, txt)
	int selected_channel{0}; // one channel
	float time_first{0.f}; // interval first
	float time_last{2.f}; // interval last
	int i_under_sample{1}; // under_sample factor when exporting data

	CString path{}; // path to files to import

	void initialize_wave_chan_array()
	{
		p_wave_chan_array = new (CWaveChanArray);
		ASSERT(p_wave_chan_array != NULL);
		p_wave_chan_array->chan_array_add();
	}
};
