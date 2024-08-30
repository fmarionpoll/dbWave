#include "StdAfx.h"
#include "output_parameters.h"


IMPLEMENT_SERIAL(output_parameters, CObject, 0 /* schema number*/)

output_parameters::output_parameters()
= default;

output_parameters::~output_parameters()
= default;

output_parameters& output_parameters::operator =(const output_parameters& arg)
{
	if (this != &arg)
	{
		w_version = arg.w_version;
		csFilename = arg.csFilename;
		noise_b_external = arg.noise_b_external;
		b_on = arg.b_on;
		b_digital = arg.b_digital;
		i_chan = arg.i_chan;
		i_waveform = arg.i_waveform;
		m_seq_i_ratio = arg.m_seq_i_ratio;
		m_seq_i_delay = arg.m_seq_i_delay;
		m_seq_i_seed = arg.m_seq_i_seed;
		noise_i_delay = arg.noise_i_delay;
		d_amplitude_max_v = arg.d_amplitude_max_v;
		d_amplitude_min_v = arg.d_amplitude_min_v;
		d_frequency = arg.d_frequency;
		dummy1 = arg.dummy1;
		dummy2 = arg.dummy2;
		noise_d_amplitude_v = arg.noise_d_amplitude_v;
		noise_d_factor = arg.noise_d_factor;
		noise_d_offset_v = arg.noise_d_offset_v;
		stimulus_sequence = arg.stimulus_sequence;
		value = arg.value;
		sti = arg.sti;
	}
	return *this;
}

void output_parameters::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << w_version;

		ar << static_cast<WORD>(1); // CString
		ar << csFilename;

		ar << static_cast<WORD>(3); // BOOL
		ar << noise_b_external;
		ar << b_on;
		ar << b_digital;

		ar << static_cast<WORD>(6); // int
		ar << i_chan;
		ar << i_waveform;
		ar << m_seq_i_ratio;
		ar << m_seq_i_delay;
		ar << m_seq_i_seed;
		ar << noise_i_delay;

		ar << static_cast<WORD>(9); // double
		ar << d_amplitude_max_v;
		ar << d_amplitude_min_v;
		ar << d_frequency;
		ar << dummy1;
		ar << dummy2;
		ar << noise_d_amplitude_v;
		ar << noise_d_factor;
		ar << noise_d_offset_v;
		ar << value;

		ar << static_cast<WORD>(2); // 1 more object
		stimulus_sequence.Serialize(ar);
		sti.Serialize(ar);
	}
	else
	{
		WORD version;
		ar >> version;
		WORD wn;

		// string parameters
		ar >> wn;
		int n = wn;
		if (n > 0) ar >> csFilename;
		n--;
		CString cs_dummy;
		while (n > 0)
		{
			n--;
			ar >> cs_dummy;
		}

		// BOOL parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> noise_b_external;
		n--;
		if (n > 0) ar >> b_on;
		n--;
		if (n > 0) ar >> b_digital;
		n--;
		BOOL bdummy;
		while (n > 0)
		{
			n--;
			ar >> bdummy;
		}

		// int parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> i_chan;
		n--;
		if (n > 0) ar >> i_waveform;
		n--;
		if (n > 0) ar >> m_seq_i_ratio;
		n--;
		if (n > 0) ar >> m_seq_i_delay;
		n--;
		if (n > 0) ar >> m_seq_i_seed;
		n--;
		if (n > 0) ar >> noise_i_delay;
		n--;
		int idummy;
		while (n > 0)
		{
			n--;
			ar >> idummy;
		}

		// double parameters
		ar >> wn;
		n = wn;
		if (n > 0) ar >> d_amplitude_max_v;
		n--;
		if (n > 0) ar >> d_amplitude_min_v;
		n--;
		if (n > 0) ar >> d_frequency;
		n--;
		if (n > 0) ar >> dummy1;
		n--;
		if (n > 0) ar >> dummy2;
		n--;
		if (n > 0) ar >> noise_d_amplitude_v;
		n--;
		if (n > 0) ar >> noise_d_factor;
		n--;
		if (n > 0) ar >> noise_d_offset_v;
		n--;
		if (n > 0) ar >> value;
		n--;
		double ddummy;
		while (n > 0)
		{
			n--;
			ar >> ddummy;
		}

		// other?
		ar >> wn;
		n = wn;
		if (n > 0)
		{
			stimulus_sequence.Serialize(ar);
			n--;
		}
		if (n > 0)
		{
			sti.Serialize(ar);
			n--;
		}
		ASSERT(n == 0);
	}
}
