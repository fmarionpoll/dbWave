#include "StdAfx.h"
#include "OPTIONS_ACQDATA.h"


IMPLEMENT_SERIAL(OPTIONS_ACQDATA, CObject, 0 /* schema number*/)

OPTIONS_ACQDATA::OPTIONS_ACQDATA()
= default;

OPTIONS_ACQDATA::~OPTIONS_ACQDATA()
= default;

OPTIONS_ACQDATA& OPTIONS_ACQDATA::operator =(const OPTIONS_ACQDATA& arg)
{
	if (this != &arg)
	{
		chanArray = arg.chanArray;
		waveFormat = arg.waveFormat;
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
	}
	return *this;
}

void OPTIONS_ACQDATA::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_wversion;

		ar << static_cast<WORD>(2); // 1 - string parameters
		ar << csBasename;
		ar << csPathname;

		ar << static_cast<WORD>(19); // 2 - int parameters
		ar << exptnumber; // 1
		ar << icsA_stimulus; // 2
		ar << icsA_concentration; // 3
		ar << icsA_insect; // 4
		ar << icsA_location; // 5
		ar << icsA_sensillum; // 6
		ar << icsA_strain; // 7
		ar << icsA_operatorname; // 8
		ar << iundersample; // 9
		ar << baudiblesound; // 10
		ar << bChannelType; // 11
		ar << icsA_stimulus2; // 12
		ar << icsA_concentration2; // 13
		ar << izoomCursel; // 14
		ar << icsA_sex; // 15
		ar << icsA_repeat; // 16
		ar << icsA_repeat2; // 17
		ar << icsA_expt; // 18
		ar << insectnumber; // 19

		ar << static_cast<WORD>(11); // 3 - CStringArray parameters
		int nsize;
		int i;
		nsize = csA_stimulus.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_stimulus.GetAt(i); }
		nsize = csA_concentration.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_concentration.GetAt(i); }
		nsize = csA_insect.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_insect.GetAt(i); }
		nsize = csA_location.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_location.GetAt(i); }
		nsize = csA_sensillum.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_sensillum.GetAt(i); }
		nsize = csA_strain.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_strain.GetAt(i); }
		nsize = csA_operatorname.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_operatorname.GetAt(i); }
		nsize = csA_stimulus2.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_stimulus2.GetAt(i); }
		nsize = csA_concentration2.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_concentration2.GetAt(i); }
		nsize = csA_sex.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_sex.GetAt(i); }
		nsize = csA_expt.GetSize();
		ar << nsize;
		for (i = 0; i < nsize; i++) { ar << csA_expt.GetAt(i); }

		ar << static_cast<WORD>(3); // 4 - serialized objects
		chanArray.Serialize(ar);
		waveFormat.Serialize(ar);
		ar << sweepduration;
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
		if (n > 0) ar >> exptnumber;
		n--;
		if (n > 0) ar >> icsA_stimulus;
		n--;
		if (n > 0) ar >> icsA_concentration;
		n--;
		if (n > 0) ar >> icsA_insect;
		n--;
		if (n > 0) ar >> icsA_location;
		n--;
		if (n > 0) ar >> icsA_sensillum;
		n--;
		if (n > 0) ar >> icsA_strain;
		n--;
		if (n > 0) ar >> icsA_operatorname;
		n--;
		if (n > 0) ar >> iundersample;
		n--;
		if (n > 0) ar >> baudiblesound;
		n--;
		if (n > 0) ar >> bChannelType;
		n--;
		if (n > 0) ar >> icsA_stimulus2;
		n--;
		if (n > 0) ar >> icsA_concentration2;
		n--;
		if (n > 0) ar >> izoomCursel;
		n--;
		if (n > 0) ar >> icsA_sex;
		n--;
		if (n > 0) ar >> icsA_repeat;
		n--;
		if (n > 0) ar >> icsA_repeat2;
		n--;
		if (n > 0) ar >> icsA_expt;
		n--;
		if (n > 0) ar >> insectnumber;
		n--;
		int idummy;
		while (n > 0)
		{
			n--;
			ar >> idummy;
		}

		// CStringArray parameters
		ar >> wn;
		n = wn;
		int nsize;
		int i;
		CString dummy;
		if (n > 0)
		{
			ar >> nsize;
			csA_stimulus.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_stimulus.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_concentration.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_concentration.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_insect.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_insect.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_location.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_location.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_sensillum.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_sensillum.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_strain.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_strain.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_operatorname.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_operatorname.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_stimulus2.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_stimulus2.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_concentration2.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_concentration2.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_sex.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_sex.SetAt(i, dummy);
			}
		}
		n--;
		if (n > 0)
		{
			ar >> nsize;
			csA_expt.SetSize(nsize);
			for (i = 0; i < nsize; i++)
			{
				ar >> dummy;
				csA_expt.SetAt(i, dummy);
			}
		}
		n--;

		while (n > 0)
		{
			n--;
			ar >> nsize;
			for (i = 0; i < nsize; i++) { ar >> dummy; }
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
	}
}
