#pragma once
#include <Olxdadefs.h>
#include "AcqWaveChanArray.h"
#include "AcqWaveFormat.h"


class options_input final : public CObject
{
	DECLARE_SERIAL(options_input)
	options_input();
	~options_input() override;
	options_input& operator =(const options_input& arg);
	void Serialize(CArchive& ar) override;
	int serialize_all_string_arrays(CArchive& ar, int& n);

public:
	BOOL b_changed{false};
	WORD w_version{2};

	CString cs_basename{};
	CString cs_pathname{};
	int experiment_number{0};
	int insect_number{0};
	int i_under_sample{1};
	BOOL b_audible_sound{0};
	BOOL b_channel_type{OLx_CHNT_DIFFERENTIAL};

	CStringArray cs_a_stimulus{};
	CStringArray cs_a_concentration{};
	CStringArray cs_a_stimulus2{};
	CStringArray cs_a_concentration2{};
	CStringArray cs_a_insect{};
	CStringArray cs_a_location{};
	CStringArray cs_a_sensillum{};
	CStringArray cs_a_strain{};
	CStringArray cs_a_operator_name{};
	CStringArray cs_a_sex{};
	CStringArray cs_a_experiment{};

	int ics_a_stimulus{0};
	int ics_a_concentration{0};
	int ics_a_stimulus2{0};
	int ics_a_concentration2{0};
	int ics_a_insect{0};
	int ics_a_location{0};
	int ics_a_sensillum{0};
	int ics_a_strain{0};
	int ics_a_operator_name{0};
	int ics_a_sex{0};
	int ics_a_repeat{0};
	int ics_a_repeat2{0};
	int ics_a_experiment{0};
	int i_zoom_cur_sel{0};

	CWaveFormat wave_format{};
	CWaveChanArray chan_array{};
	float sweep_duration{2.f};
	float duration_to_acquire{ 2. };

private:
	int serialize_one_string_array(CArchive& ar, CStringArray& string_array, int& n);
	int serialize_all_int(CArchive& ar, int& n);
	int serialize_one_int(CArchive& ar, int& value, int& n);
};
