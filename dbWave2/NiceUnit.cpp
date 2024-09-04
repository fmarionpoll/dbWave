#include "stdafx.h"
#include "NiceUnit.h"

TCHAR NiceUnit::cs_unit_[] = { _T("GM  mµpf  ") };
int NiceUnit::units_power_[] = { 9, 6, 0, 0, -3, -6, -9, -12, 0 };
int NiceUnit::max_index_ = 8;
int NiceUnit::nice_intervals_[] = { 1, 5, 10, 20, 25, 30, 40, 50, 75, 100, 200, 250, 300, 400, 500, 0 };

int NiceUnit::nice_unit(const float y)
{
	auto i = 0;
	const auto i_value = static_cast<int>(y);
	do
	{
		if (i_value <= nice_intervals_[i])
		{
			i++;
			break;
		}
		i++;
	} while (nice_intervals_[i] > 0);
	return nice_intervals_[i - 1];
}


float NiceUnit::change_unit(float x_val, CString* x_unit, float* x_scale_factor)
{
	// avoid division by zero error
	if (x_val == 0)
	{
		*x_scale_factor = 1.0f;
		x_unit->SetAt(0, ' ');
		return 0.0f;
	}

	// take absolute value of xVal and save sign
	short i;
	short i_sign = 1;
	if (x_val < 0)
	{
		i_sign = -1;
		x_val = -x_val;
	}
	// get power of 10 of the value
	const auto i_log10_upper = static_cast<short>(log10(x_val)); // log10 of value (upper limit)
	if (i_log10_upper <= 0 && x_val < 1.) // perform extra checking if ip_rec <= 0
		i = 4 - i_log10_upper / 3; // change equation if Units values change
	else
		i = 3 - i_log10_upper / 3; // change equation if Units values change
	if (i > max_index_) // clip to max index
		i = max_index_;
	else if (i < 0) // or clip to min index
		i = 0;
	// return data
	*x_scale_factor = static_cast<float>(pow(10.0f, units_power_[i])); // convert & store
	x_unit->SetAt(0, cs_unit_[i]); // replace character corresponding to unit
	return x_val * i_sign / *x_scale_factor; // return value/scale_factor
}