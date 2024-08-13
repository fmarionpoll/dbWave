#pragma once

// spike classification parameters

class options_spk_classification final : public CObject
{
	DECLARE_SERIAL(options_spk_classification)

	BOOL b_changed; // flag set TRUE if contents has changed
	WORD w_version {2}; // version number

	// sort with parameters
	int data_transform {0}; // dummy
	int i_parameter {0}; // type of parameter measured
	int shape_t1 {10}; // position of first cursor along spike 
	int shape_t2 {40}; // position of second cursor along spike
	int lower_threshold {0}; // binary value of lower threshold to select spike class
	int upper_threshold {10}; // binary value of upper threshold to select spike class
	int i_xy_left {10}; // position of left cursor on xy display
	int i_xy_right {40}; // position of right cursor on xy display
	int source_class {0}; // source class
	int dest_class {1}; // destination class

	// display parameters for spike_view
	int row_height {100};
	int col_text {-1};
	int col_spikes {100};
	int col_separator {5};
	int v_source_class {0};
	int v_dest_class {1};
	float f_jitter_ms {1.f};
	BOOL b_reset_zoom {true};

	// sort with templates
	int n_int_parameters;
	int hit_rate{50};
	int hit_rate_sort{75};
	int k_left{10};
	int k_right{40};

	int n_float_parameters;
	float k_tolerance{1.96f};
	float mv_min{0.f};
	float mv_max{2.f};

	void* p_template{nullptr};

	options_spk_classification();
	~options_spk_classification() override;
	options_spk_classification& operator =(const options_spk_classification& arg); // operator redefinition
	void Serialize(CArchive& ar) override;
	void create_tpl();
};
