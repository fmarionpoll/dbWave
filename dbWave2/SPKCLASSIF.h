#pragma once

// spike classification parameters

class SPKCLASSIF : public CObject
{
	DECLARE_SERIAL(SPKCLASSIF)
public:
	BOOL b_changed; // flag set TRUE if contents has changed
	WORD w_version; // version number

	// sort with parameters
	int data_transform; // dummy
	int i_parameter; // type of parameter measured
	int i_left; // position of first cursor
	int i_right; // position of second cursor
	int lower_threshold; // binary value of lower
	int upper_threshold; // binary value of upper
	int i_xy_left; // position of left cursor on xy display
	int i_xy_right; // position of right cursor on xy display
	int source_class; // source class
	int dest_class; // destination class

	// display parameters for spike_view
	int row_height;
	int col_text;
	int col_spikes;
	int col_separator;
	int v_source_class;
	int v_dest_class;
	float f_jitter_ms;
	BOOL b_reset_zoom;

	// sort with templates
	int n_int_parameters;
	int hit_rate;
	int hit_rate_sort;
	int k_left;
	int k_right;

	int n_float_parameters;
	float k_tolerance;
	float mv_min; // display limits
	float mv_max;

	void* p_template;

public:
	SPKCLASSIF();
	~SPKCLASSIF() override;
	SPKCLASSIF& operator =(const SPKCLASSIF& arg); // operator redefinition
	void Serialize(CArchive& ar) override;
	void CreateTPL();
};
