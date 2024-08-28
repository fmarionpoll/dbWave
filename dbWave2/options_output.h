#pragma once
#include "output_parameters.h"

class options_output final : public CObject
{
	DECLARE_SERIAL(options_output)
	options_output();
	~options_output() override;
	options_output& operator =(const options_output& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL b_changed{false};
	WORD w_version{1};

	BOOL b_allow_output_data{true}; // allow DA if present
	BOOL b_preset_wave{true}; // 0=preset, 1=from file
	CString cs_output_file{}; // output file

	CArray<output_parameters, output_parameters> output_parameters_array{};

	double da_frequency_per_channel{1000.f}; // output frequency (per chan)
	int da_trigger_mode{0}; // 0=synchronous{}; 1=soft on start{}; 2=external trigger
	int da_n_buffers{10}; // number of DA buffers
	int da_buffer_length{10000}; // length of each buffer
};
