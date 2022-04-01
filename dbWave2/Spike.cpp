#include "StdAfx.h"
#include "Spike.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


Spike::Spike()
= default;

Spike::Spike(const long time, const int channel)
{
	m_iitime = time;
	m_chanparm = channel;
}

Spike::Spike(const long time, const int channel, const int max, const int min, const int offset, const int class_i, int d_maxmin)
{
	m_iitime = time;
	m_chanparm = channel;
	m_min = min;
	m_max = max;
	m_offset = offset;
	m_class = class_i;
	m_dmaxmin = d_maxmin;
}

Spike::~Spike()
= default;

IMPLEMENT_SERIAL(Spike, CObject, 0 /* schema number*/)

void Spike::Serialize(CArchive& ar)
{
	WORD wVersion = 2;

	if (ar.IsStoring())
	{
		ar << wVersion;
		ar << m_iitime;
		ar << static_cast<WORD>(m_class);
		ar << static_cast<WORD>(m_chanparm);
		ar << static_cast<WORD>(m_max);
		ar << static_cast<WORD>(m_min);
		ar << static_cast<WORD>(m_offset);
		ar << static_cast<WORD>(m_dmaxmin);
		ar << static_cast<WORD>(2);
		ar << y1_;
		ar << dt_;
	}
	else
	{
		ar >> wVersion;
		if (wVersion <= 2)
			read_version2(ar, wVersion);
	}
}

void Spike::read_version2(CArchive& ar, WORD wVersion)
{
	WORD w1;

	ar >> m_iitime;
	ar >> w1; m_class = static_cast<int>(w1);
	ar >> w1; m_chanparm = static_cast<int>(w1);
	ar >> w1; m_max = static_cast<short>(w1);
	ar >> w1; m_min = static_cast<short>(w1);
	ar >> w1; m_offset = static_cast<short>(w1);
	ar >> w1; m_dmaxmin = static_cast<short>(w1);
	if (wVersion > 1)
	{
		WORD n_items = 0;
		ar >> n_items;
		ar >> y1_; n_items--;
		ar >> dt_; n_items--;
		ASSERT(n_items == 0);
	}
}


void Spike::read_version0(CArchive& ar)
{
	WORD w1;

	ASSERT(ar.IsStoring() == FALSE);

	ar >> m_iitime;
	ar >> w1; m_class = static_cast<int>(w1);
	ar >> w1; m_chanparm = static_cast<int>(w1);
	ar >> w1; m_max = static_cast<short>(w1);
	ar >> w1; m_min = static_cast<short>(w1);
	ar >> w1; m_offset = static_cast<short>(w1);
	m_dmaxmin = 0;
}

short* Spike::GetpSpikeData(int spike_length)
{
	int delta = 6;
	if (m_spike_data_buffer == nullptr)
	{
		m_spike_data_buffer = static_cast<short*>(malloc(sizeof(short) * (spike_length + delta)));
		m_spk_buffer_length = spike_length;
	}
	if (spike_length != m_spk_buffer_length)
	{
		m_spike_data_buffer = static_cast<short*>(malloc(sizeof(short) * (spike_length + delta)));
		m_spk_buffer_length = spike_length;
	}
	return m_spike_data_buffer;
}
