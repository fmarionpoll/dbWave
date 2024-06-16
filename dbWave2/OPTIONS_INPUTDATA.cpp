#include "StdAfx.h"
#include "OPTIONS_INPUTDATA.h"


IMPLEMENT_SERIAL(OPTIONS_INPUTDATA, CObject, 0 /* schema number*/)

OPTIONS_INPUTDATA::OPTIONS_INPUTDATA()
= default;

OPTIONS_INPUTDATA::~OPTIONS_INPUTDATA()
= default;

OPTIONS_INPUTDATA& OPTIONS_INPUTDATA::operator =(const OPTIONS_INPUTDATA& arg)
{
	if (this != &arg)
	{
		chanArray.Copy( &arg.chanArray);
		waveFormat.copy( &arg.waveFormat);
		csBasename = arg.csBasename;
		csPathname = arg.csPathname;
		exptnumber = arg.exptnumber;
		csA_stimulus.RemoveAll();
		csA_stimulus.Append(arg.csA_stimulus);
		csA_concentration.RemoveAll();
		csA_concentration.Append(arg.csA_concentration);
		csA_stimulus2.RemoveAll();
		csA_stimulus2.Append(arg.csA_stimulus2);
		csA_concentration2.RemoveAll();
		csA_concentration2.Append(arg.csA_concentration2);
		csA_insect.RemoveAll();
		csA_insect.Append(arg.csA_insect);
		csA_location.RemoveAll();
		csA_location.Append(arg.csA_location);
		csA_sensillum.RemoveAll();
		csA_sensillum.Append(arg.csA_sensillum);
		csA_strain.RemoveAll();
		csA_strain.Append(arg.csA_strain);
		csA_sex.RemoveAll();
		csA_sex.Append(arg.csA_sex);
		csA_operatorname.RemoveAll();
		csA_operatorname.Append(arg.csA_operatorname);
		csA_expt.RemoveAll();
		csA_expt.Append(arg.csA_expt);

		icsA_stimulus = arg.icsA_stimulus;
		icsA_concentration = arg.icsA_concentration;
		icsA_stimulus2 = arg.icsA_stimulus2;
		icsA_concentration2 = arg.icsA_concentration2;
		icsA_expt = arg.icsA_expt;
		izoomCursel = arg.izoomCursel;

		icsA_insect = arg.icsA_insect;
		icsA_location = arg.icsA_location;
		icsA_sensillum = arg.icsA_sensillum;
		icsA_strain = arg.icsA_strain;
		icsA_sex = arg.icsA_sex;
		icsA_operatorname = arg.icsA_operatorname;
		iundersample = arg.iundersample;
		baudiblesound = arg.baudiblesound;
		bChannelType = arg.bChannelType;

		sweepduration = arg.sweepduration;
		insectnumber = arg.insectnumber;
		duration_to_acquire = arg.duration_to_acquire;
	}
	return *this;
}

void OPTIONS_INPUTDATA::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_wversion;

		ar << static_cast<WORD>(2); // 1 - string parameters
		ar << csBasename;
		ar << csPathname;

		int dummy_n = 0;
		ar << static_cast<WORD>(19); // 2 - int parameters
		serialize_all_int(ar, dummy_n);

		ar << static_cast<WORD>(11); // 3 - CStringArray parameters
		serialize_all_string_arrays(ar, dummy_n);

		ar << static_cast<WORD>(4); // 4 - serialized objects
		chanArray.Serialize(ar);
		waveFormat.Serialize(ar);
		ar << sweepduration;
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
		if (n > 0) ar >> csBasename;
		n--;
		if (n > 0) ar >> csPathname;
		n--;
		CString csdummy;
		while (n > 0)
		{
			n--;
			ar >> csdummy;
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
		if (n > 0) chanArray.Serialize(ar);
		n--;
		if (n > 0) waveFormat.Serialize(ar);
		n--;
		if (n > 0) ar >> sweepduration;
		n--;
		if (n > 0) ar >> duration_to_acquire;
	}
}

int OPTIONS_INPUTDATA::serialize_all_string_arrays(CArchive& ar, int& n)
{
	serialize_one_string_array(ar, csA_stimulus, n);
	serialize_one_string_array(ar, csA_concentration, n);
	serialize_one_string_array(ar, csA_insect, n);
	serialize_one_string_array(ar, csA_location, n);
	serialize_one_string_array(ar, csA_sensillum, n);
	serialize_one_string_array(ar, csA_strain, n);
	serialize_one_string_array(ar, csA_operatorname, n);
	serialize_one_string_array(ar, csA_stimulus2, n);
	serialize_one_string_array(ar, csA_concentration2, n);
	serialize_one_string_array(ar, csA_sex, n);
	serialize_one_string_array(ar, csA_expt, n);
	return n;
}

int OPTIONS_INPUTDATA::serialize_one_string_array(CArchive& ar, CStringArray& string_array, int& n)
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


int OPTIONS_INPUTDATA::serialize_all_int(CArchive& ar, int& n)
{
	serialize_one_int(ar, exptnumber, n);
	serialize_one_int(ar,icsA_stimulus, n);
	serialize_one_int(ar,icsA_concentration, n);
	serialize_one_int(ar,icsA_insect, n);
	serialize_one_int(ar,icsA_location, n);
	serialize_one_int(ar,icsA_sensillum, n);
	serialize_one_int(ar,icsA_strain, n);
	serialize_one_int(ar,icsA_operatorname, n);
	serialize_one_int(ar,iundersample, n);
	serialize_one_int(ar,baudiblesound, n);
	serialize_one_int(ar,bChannelType, n);
	serialize_one_int(ar,icsA_stimulus2, n);
	serialize_one_int(ar,icsA_concentration2, n);
	serialize_one_int(ar,izoomCursel, n);
	serialize_one_int(ar,icsA_sex, n);
	serialize_one_int(ar,icsA_repeat, n);
	serialize_one_int(ar,icsA_repeat2, n);
	serialize_one_int(ar,icsA_expt, n);
	serialize_one_int(ar,insectnumber, n);
	return n;
}

int OPTIONS_INPUTDATA::serialize_one_int(CArchive& ar, int& value, int& n)
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

