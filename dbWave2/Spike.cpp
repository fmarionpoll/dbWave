#include "StdAfx.h"
#include "Spike.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


Spike::Spike()
= default;

Spike::Spike(const long time, const int channel)
{
	m_ii_time = time;
	m_detection_parameters_index = channel;
}

Spike::Spike(const long time, const int channel, const int max, const int min, const int offset, const int class_i, const int d_max_min, const int spike_length)
{
	m_ii_time = time;
	m_detection_parameters_index = channel;
	m_value_min = min;
	m_value_max = max;
	m_offset = offset;
	m_class_id = class_i;
	m_d_max_min = d_max_min;
	m_spike_length = spike_length;
}

Spike::Spike(const long time, const int channel, const int offset, const int class_i, const int spike_length)
{
	m_ii_time = time;
	m_detection_parameters_index = channel;
	m_offset = offset;
	m_class_id = class_i;
	m_spike_length = spike_length;
}

Spike::~Spike()
{
	delete m_spike_data_buffer;
}

IMPLEMENT_SERIAL(Spike, CObject, 0 /* schema number*/)

void Spike::Serialize(CArchive& ar)
{
	WORD wVersion = 2;

	if (ar.IsStoring())
	{
		ar << wVersion;
		ar << m_ii_time;
		ar << static_cast<WORD>(m_class_id);
		ar << static_cast<WORD>(m_detection_parameters_index);
		ar << static_cast<WORD>(m_value_max);
		ar << static_cast<WORD>(m_value_min);
		ar << static_cast<WORD>(m_offset);
		ar << static_cast<WORD>(m_d_max_min);
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

	ar >> m_ii_time;
	ar >> w1; m_class_id = static_cast<int>(w1);
	ar >> w1; m_detection_parameters_index = static_cast<int>(w1);
	ar >> w1; m_value_max = static_cast<short>(w1);
	ar >> w1; m_value_min = static_cast<short>(w1);
	ar >> w1; m_offset = static_cast<short>(w1);
	ar >> w1; m_d_max_min = static_cast<short>(w1);
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

	ar >> m_ii_time;
	ar >> w1; m_class_id = static_cast<int>(w1);
	ar >> w1; m_detection_parameters_index = static_cast<int>(w1);
	ar >> w1; m_value_max = static_cast<short>(w1);
	ar >> w1; m_value_min = static_cast<short>(w1);
	ar >> w1; m_offset = static_cast<short>(w1);
	m_d_max_min = 0;
}

short* Spike::get_p_data(int spike_length)
{
	constexpr int delta = 0;
	const size_t spike_data_length = sizeof(short) * (spike_length + delta);
	if (m_spike_data_buffer == nullptr)
	{
		m_spike_data_buffer = static_cast<short*>(malloc(spike_data_length));
		m_spk_buffer_length = spike_length;
	}
	else if (spike_length != m_spk_buffer_length)
	{
		delete m_spike_data_buffer;
		m_spike_data_buffer = static_cast<short*>(malloc(spike_data_length));
		m_spk_buffer_length = spike_length;
	}
	return m_spike_data_buffer;
}

short* Spike::get_p_data() const
{
	return m_spike_data_buffer;
}

void Spike::get_max_min_ex(short* max, short* min, int* d_max_to_min) const
{
	*max = m_value_max;
	*min = m_value_min;
	*d_max_to_min = m_d_max_min;
}

void Spike::get_max_min(short* max, short* min) const
{
	*max = m_value_max;
	*min = m_value_min;
}

void Spike::measure_max_min_ex(short* value_max, int* index_max, short* value_min, int* index_min, const int i_first, const int i_last) const
{
	auto lp_buffer = get_p_data() + i_first;
	auto val = *lp_buffer;
	*value_max = val;
	*value_min = val;
	*index_min = *index_max = i_first;
	for (auto i = i_first + 1; i < i_last; i++)
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *value_max)
		{
			*value_max = val;
			*index_max = i;
		}
		else if (val < *value_min)
		{
			*value_min = val;
			*index_min = i;
		}
	}
}

void Spike::MeasureMaxThenMinEx(short* value_max, int* index_max, short* value_min, int* index_min, const int i_first, const int i_last) const
{
	auto lp_buffer = get_p_data() + i_first;
	auto lp_buffer_max = lp_buffer;
	auto val = *lp_buffer;
	*value_max = val;
	*index_max = i_first;

	// first search for max
	for (auto i = i_first + 1; i <= i_last; i++)
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > *value_max)
		{
			*value_max = val;
			*index_max = i;
			lp_buffer_max = lp_buffer;
		}
	}

	// search for min
	lp_buffer = lp_buffer_max;
	*value_min = *value_max;
	*index_min = *index_max;
	for (int i = *index_min + 1; i <= i_last; i++)
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val < *value_min)
		{
			*value_min = val;
			*index_min = i;
		}
	}
}

long Spike::MeasureSumEx(int i_first, int i_last) const
{
	auto lp_b = get_p_data() + i_first;
	long average_value = 0;
	for (auto i = i_first; i <= i_last; i++, lp_b ++)
	{
		const int val = *lp_b;
		average_value += val;
	}
	return average_value;
}

void Spike::TransferDataToSpikeBuffer(short* source_data, const int source_n_channels)
{
	auto lp_dest = get_p_data(get_spike_length());
	for (auto i = get_spike_length(); i > 0; i--, source_data += source_n_channels, lp_dest++)
	{
		*lp_dest = *source_data;
	}
}

void Spike::OffsetSpikeData(const short offset)
{
	auto lp_dest = get_p_data(get_spike_length());
	for (auto i = get_spike_length(); i > 0; i--, lp_dest++)
	{
		*lp_dest += offset; 
	}
	
	m_offset = offset;
	m_value_max -= offset;
	m_value_min -= offset;
}

void Spike::OffsetSpikeDataToAverageEx(int i_first, int i_last)
{
	const long average_value = MeasureSumEx(i_first, i_last);
	const int offset = (average_value / (i_last - i_first + 1)) - m_bin_zero;
	OffsetSpikeData(static_cast<short>(offset));
}

void Spike::OffsetSpikeDataToExtremaEx(int i_first, int i_last)
{
	short value_max, value_min;
	int max_index, min_index;
	measure_max_min_ex(&value_max, &max_index, &value_min, &min_index, i_first, i_last);
	const int offset = (value_max + value_min) / 2 - m_bin_zero;
	OffsetSpikeData(static_cast<short>(offset));
}

void Spike::CenterSpikeAmplitude(const int i_first, const int i_last, const WORD method)
{
	switch (method)
	{
		case 0:
			OffsetSpikeDataToExtremaEx(i_first, i_last);
			break;
		case 1: 
			OffsetSpikeDataToAverageEx(i_first, i_last);
			break;
		default:
			break;
	}

	// change spike offset
	//SetAmplitudeOffset(get_amplitude_offset() + val_first);
	//SetMaxMinEx(max - val_first, min - val_first, d_max_to_min);
	//OffsetSpikeData(val_first);
}
