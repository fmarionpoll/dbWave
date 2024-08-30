#include "StdAfx.h"
#include "options_input.h"


IMPLEMENT_SERIAL(options_input, CObject, 0 /* schema number*/)

options_input::options_input()
= default;

options_input::~options_input()
= default;

options_input& options_input::operator =(const options_input& arg)
{
	if (this != &arg)
	{
		chan_array.Copy( &arg.chan_array);
		wave_format.copy( &arg.wave_format);
		cs_basename = arg.cs_basename;
		cs_pathname = arg.cs_pathname;
		experiment_number = arg.experiment_number;
		cs_a_stimulus.RemoveAll();
		cs_a_stimulus.Append(arg.cs_a_stimulus);
		cs_a_concentration.RemoveAll();
		cs_a_concentration.Append(arg.cs_a_concentration);
		cs_a_stimulus2.RemoveAll();
		cs_a_stimulus2.Append(arg.cs_a_stimulus2);
		cs_a_concentration2.RemoveAll();
		cs_a_concentration2.Append(arg.cs_a_concentration2);
		cs_a_insect.RemoveAll();
		cs_a_insect.Append(arg.cs_a_insect);
		cs_a_location.RemoveAll();
		cs_a_location.Append(arg.cs_a_location);
		cs_a_sensillum.RemoveAll();
		cs_a_sensillum.Append(arg.cs_a_sensillum);
		cs_a_strain.RemoveAll();
		cs_a_strain.Append(arg.cs_a_strain);
		cs_a_sex.RemoveAll();
		cs_a_sex.Append(arg.cs_a_sex);
		cs_a_operator_name.RemoveAll();
		cs_a_operator_name.Append(arg.cs_a_operator_name);
		cs_a_experiment.RemoveAll();
		cs_a_experiment.Append(arg.cs_a_experiment);

		ics_a_stimulus = arg.ics_a_stimulus;
		ics_a_concentration = arg.ics_a_concentration;
		ics_a_stimulus2 = arg.ics_a_stimulus2;
		ics_a_concentration2 = arg.ics_a_concentration2;
		ics_a_experiment = arg.ics_a_experiment;
		i_zoom_cur_sel = arg.i_zoom_cur_sel;

		ics_a_insect = arg.ics_a_insect;
		ics_a_location = arg.ics_a_location;
		ics_a_sensillum = arg.ics_a_sensillum;
		ics_a_strain = arg.ics_a_strain;
		ics_a_sex = arg.ics_a_sex;
		ics_a_operator_name = arg.ics_a_operator_name;
		i_under_sample = arg.i_under_sample;
		b_audible_sound = arg.b_audible_sound;
		b_channel_type = arg.b_channel_type;

		sweep_duration = arg.sweep_duration;
		insect_number = arg.insect_number;
		duration_to_acquire = arg.duration_to_acquire;
	}
	return *this;
}

void options_input::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << w_version;

		ar << static_cast<WORD>(2); // 1 - string parameters
		ar << cs_basename;
		ar << cs_pathname;

		int dummy_n = 0;
		ar << static_cast<WORD>(19); // 2 - int parameters
		serialize_all_int(ar, dummy_n);

		ar << static_cast<WORD>(11); // 3 - CStringArray parameters
		serialize_all_string_arrays(ar, dummy_n);

		ar << static_cast<WORD>(4); // 4 - serialized objects
		chan_array.Serialize(ar);
		wave_format.Serialize(ar);
		ar << sweep_duration;
		ar << duration_to_acquire;
	}
	else
	{
		WORD version;
		ar >> version;
		int n;
		WORD wn;

		// string parameters
		// TODO: check if names (CString) are read ok
		ar >> wn;
		n = wn;
		if (n > 0) ar >> cs_basename;
		n--;
		if (n > 0) ar >> cs_pathname;
		n--;
		CString cs_dummy;
		while (n > 0)
		{
			n--;
			ar >> cs_dummy;
		}

		// int parameters
		ar >> wn;
		n = wn;
		serialize_all_int(ar, n);

		int idummy;
		while (n > 0)
		{
			n--;
			ar >> idummy;
		}

		// CStringArray parameters
		ar >> wn;
		n = wn;
		serialize_all_string_arrays(ar, n);
		while (n > 0)
		{
			CStringArray dummy_array;
			serialize_one_string_array(ar, dummy_array, n);
		}
		// serialized objects
		ar >> wn;
		n = wn;
		if (n > 0) chan_array.Serialize(ar);
		n--;
		if (n > 0) wave_format.Serialize(ar);
		n--;
		if (n > 0) ar >> sweep_duration;
		n--;
		if (n > 0) ar >> duration_to_acquire;
	}
}

int options_input::serialize_all_string_arrays(CArchive& ar, int& n)
{
	serialize_one_string_array(ar, cs_a_stimulus, n);
	serialize_one_string_array(ar, cs_a_concentration, n);
	serialize_one_string_array(ar, cs_a_insect, n);
	serialize_one_string_array(ar, cs_a_location, n);
	serialize_one_string_array(ar, cs_a_sensillum, n);
	serialize_one_string_array(ar, cs_a_strain, n);
	serialize_one_string_array(ar, cs_a_operator_name, n);
	serialize_one_string_array(ar, cs_a_stimulus2, n);
	serialize_one_string_array(ar, cs_a_concentration2, n);
	serialize_one_string_array(ar, cs_a_sex, n);
	serialize_one_string_array(ar, cs_a_experiment, n);
	return n;
}

int options_input::serialize_one_string_array(CArchive& ar, CStringArray& string_array, int& n)
{
	if (ar.IsStoring())
	{
		const int number_items = string_array.GetSize();
		ar << number_items;
		for (int i = 0; i < number_items; i++)
		{
			ar << string_array.GetAt(i);
		}
		n++;
	}
	else
	{
		if (n > 0)
		{
			int number_items = 0;
			ar >> number_items;
			string_array.SetSize(number_items);
			for (int i = 0; i < number_items; i++)
			{
				CString dummy;
				ar >> dummy;
				string_array.SetAt(i, dummy);
			}
		}
		n--;
	}
	return n;
}


int options_input::serialize_all_int(CArchive& ar, int& n)
{
	serialize_one_int(ar, experiment_number, n);
	serialize_one_int(ar,ics_a_stimulus, n);
	serialize_one_int(ar,ics_a_concentration, n);
	serialize_one_int(ar,ics_a_insect, n);
	serialize_one_int(ar,ics_a_location, n);
	serialize_one_int(ar,ics_a_sensillum, n);
	serialize_one_int(ar,ics_a_strain, n);
	serialize_one_int(ar,ics_a_operator_name, n);
	serialize_one_int(ar,i_under_sample, n);
	serialize_one_int(ar,b_audible_sound, n);
	serialize_one_int(ar,b_channel_type, n);
	serialize_one_int(ar,ics_a_stimulus2, n);
	serialize_one_int(ar,ics_a_concentration2, n);
	serialize_one_int(ar,i_zoom_cur_sel, n);
	serialize_one_int(ar,ics_a_sex, n);
	serialize_one_int(ar,ics_a_repeat, n);
	serialize_one_int(ar,ics_a_repeat2, n);
	serialize_one_int(ar,ics_a_experiment, n);
	serialize_one_int(ar,insect_number, n);
	return n;
}

int options_input::serialize_one_int(CArchive& ar, int& value, int& n)
{
	if (ar.IsStoring())
	{
		ar << value;
		n++;
	}
	else
	{
		if (n > 0) 
			ar >> value;
		n--;
	}
	return n;
}

