#pragma once
class NiceUnit
{
	static int nice_intervals_[];
	static TCHAR cs_unit_[];
	static int units_power_[];
	static int max_index_;

public:
	static float change_unit(float x_val, CString* x_unit, float* x_scale_factor);
	static int nice_unit(float y);
};

