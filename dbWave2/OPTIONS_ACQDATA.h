#pragma once
#include <Olxdadefs.h>
#include "AcqWaveChanArray.h"
#include "AcqWaveFormat.h"


class OPTIONS_ACQDATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_ACQDATA)
	OPTIONS_ACQDATA();
	~OPTIONS_ACQDATA() override;
	OPTIONS_ACQDATA& operator =(const OPTIONS_ACQDATA& arg);
	void Serialize(CArchive& ar) override;
	int serialize_all_string_arrays(CArchive& ar, int& n);

public:
	BOOL bChanged{false};
	WORD m_wversion{2};

	CString csBasename{};
	CString csPathname{};
	int exptnumber{0};
	int insectnumber{0};
	int iundersample{1};
	BOOL baudiblesound{0};
	BOOL bChannelType{OLx_CHNT_DIFFERENTIAL};

	CStringArray csA_stimulus{};
	CStringArray csA_concentration{};
	CStringArray csA_stimulus2{};
	CStringArray csA_concentration2{};
	CStringArray csA_insect{};
	CStringArray csA_location{};
	CStringArray csA_sensillum{};
	CStringArray csA_strain{};
	CStringArray csA_operatorname{};
	CStringArray csA_sex{};
	CStringArray csA_expt{};

	int icsA_stimulus{0};
	int icsA_concentration{0};
	int icsA_stimulus2{0};
	int icsA_concentration2{0};
	int icsA_insect{0};
	int icsA_location{0};
	int icsA_sensillum{0};
	int icsA_strain{0};
	int icsA_operatorname{0};
	int icsA_sex{0};
	int icsA_repeat{0};
	int icsA_repeat2{0};
	int icsA_expt{0};
	int izoomCursel{0};

	CWaveFormat waveFormat{};
	CWaveChanArray chanArray{};
	float sweepduration{2.f};
	float duration_to_acquire{ 2. };

private:
	int save_string_array(CArchive& ar, const CStringArray& string_array, int& n);
	int serialize_one_string_array(CArchive& ar, CStringArray& string_array, int& n);
	int serialize_all_int(CArchive& ar, int& n);
	int serialize_one_int(CArchive& ar, int& value, int& n);
};
