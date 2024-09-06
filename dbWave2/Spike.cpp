#include "StdAfx.h"
#include "Spike.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


Spike::Spike()
= default;

Spike::Spike(const long time, const int channel)
{
	ii_time_ = time;
	detection_parameters_index_ = channel;
}

Spike::Spike(const long time, const int channel, const int max, const int min, const int offset, const int class_i, const int d_max_min, const int spike_length)
{
	ii_time_ = time;
	detection_parameters_index_ = channel;
	value_min_ = min;
	value_max_ = max;
	offset_ = offset;
	class_id_ = class_i;
	d_max_min_ = d_max_min;
	spike_length_ = spike_length;
	buffer_spike_length_ = spike_length;
}

Spike::Spike(const long time, const int channel, const int offset, const int class_i, const int spike_length)
{
	ii_time_ = time;
	detection_parameters_index_ = channel;
	offset_ = offset;
	class_id_ = class_i;
	spike_length_ = spike_length;
}

Spike::~Spike()
{
	if (buffer_spike_data_ != nullptr)
		free(buffer_spike_data_);
}

IMPLEMENT_SERIAL(Spike, CObject, 0 /* schema number*/)

void Spike::Serialize(CArchive& ar)
{
	WORD w_version = 3;

	if (ar.IsStoring())
	{
		ar << w_version;
		ar << ii_time_;
		ar << class_id_;
		ar << detection_parameters_index_;
		ar << value_max_;
		ar << value_min_;
		ar << offset_;
		ar << d_max_min_;
		ar << static_cast<WORD>(2);
		ar << y1_;
		ar << dt_;

		ar << spike_length_;
		if (spike_length_ > 0)
		{
			const auto n_bytes_int = spike_length_ * sizeof(int);
			const int* p_data = get_p_data(spike_length_);
			ar.Write(p_data, n_bytes_int);
		}
	}
	else
	{
		ar >> w_version;
		if (w_version <= 2)
		{
			read_version2(ar, w_version);
		}
		else
		{
			ar >> ii_time_;
			ar >> class_id_;
			ar >> detection_parameters_index_;
			ar >> value_max_;
			ar >> value_min_;
			ar >> offset_;
			ar >> d_max_min_;
			WORD n_items = 0;
			ar >> n_items;
			ar >> y1_; n_items--;
			ar >> dt_; n_items--;
			ASSERT(n_items == 0);

			ar >> spike_length_;
			if (spike_length_ > 0)
			{
				const auto n_bytes_int = spike_length_ * sizeof(int);
				int* p_data = get_p_data(spike_length_);
				ar.Read(p_data, n_bytes_int);
			}
		}
	}
}

void Spike::read_version2(CArchive& ar, const WORD w_version)
{
	ASSERT(ar.IsStoring() == FALSE);

	WORD w1;
	ar >> ii_time_;
	ar >> w1;
	class_id_ = static_cast<short>(w1);
	ar >> w1;
	detection_parameters_index_ = static_cast<short>(w1);
	ar >> w1;
	value_max_ = static_cast<short>(w1);
	ar >> w1;
	value_min_ = static_cast<short>(w1);
	ar >> w1;
	offset_ = static_cast<short>(w1);
	ar >> w1;
	d_max_min_ = static_cast<short>(w1);
	if (w_version > 1)
	{
		WORD n_items = 0;
		ar >> n_items;
		ar >> y1_;
		n_items--;
		ar >> dt_;
		n_items--;
		ASSERT(n_items == 0);
	}
}

void Spike::read_version0(CArchive& ar)
{
	ASSERT(ar.IsStoring() == FALSE);

	WORD w1;
	ar >> ii_time_;
	ar >> w1; class_id_ = static_cast<int>(w1);
	ar >> w1; detection_parameters_index_ = static_cast<int>(w1);
	ar >> w1; value_max_ = static_cast<short>(w1);
	ar >> w1; value_min_ = static_cast<short>(w1);
	ar >> w1; offset_ = static_cast<short>(w1);
	d_max_min_ = 0;
}

int* Spike::get_p_data(const int spike_length)
{
	constexpr int delta = 0;
	const size_t spike_data_length = sizeof(int) * (spike_length + delta);
	int* p_rw_buffer = buffer_spike_data_;
	
	if (buffer_spike_data_ == nullptr)
		p_rw_buffer = static_cast<int*>(malloc(spike_data_length));
	else if (spike_length != buffer_spike_length_)
		p_rw_buffer = static_cast<int*>(realloc(buffer_spike_data_, spike_data_length));

	buffer_spike_data_ = p_rw_buffer;
	buffer_spike_length_ = spike_length;
	if (buffer_spike_data_ == nullptr)
	{
		printf("Insufficient memory available\n");
		buffer_spike_length_ = 0;
	}

	return buffer_spike_data_;
}

int* Spike::get_p_data() const
{
	return buffer_spike_data_;
}

void Spike::get_max_min_ex(int* max, int* min, int* d_max_to_min) const
{
	*max = value_max_;
	*min = value_min_;
	*d_max_to_min = d_max_min_;
}

void Spike::get_max_min(int* max, int* min) const
{
	*max = value_max_;
	*min = value_min_;
}

void Spike::measure_amplitude_min_to_max(const int t1, const int t2)
{
	auto lp_buffer = get_p_data(get_spike_length());
	lp_buffer += t1;
	auto val = *lp_buffer;
	auto max = val;
	auto min = val;
	auto min_index = t1;
	auto max_index = t1;
	for (auto i = t1 + 1; i <= t2; i++)
	{
		lp_buffer++;
		val = *lp_buffer;
		if (val > max)
		{
			max = val;
			max_index = i;
		}
		else if (val < min)
		{
			min = val;
			min_index = i;
		}
	}

	set_max_min_ex(max, min, min_index - max_index);
	set_y1(max - min);
}

void Spike::measure_max_min_ex(int* value_max, int* index_max, int* value_min, int* index_min, const int t1, const int t2) const
{
	auto lp_buffer = get_p_data() + t1;
	auto val = *lp_buffer;
	*value_max = val;
	*value_min = val;
	*index_min = *index_max = t1;
	for (auto i = t1 + 1; i < t2; i++)
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

void Spike::measure_max_then_min_ex(int* value_max, int* index_max, int* value_min, int* index_min, const int t1, const int t2) const
{
	auto lp_buffer = get_p_data() + t1;
	auto lp_buffer_max = lp_buffer;
	auto val = *lp_buffer;
	*value_max = val;
	*index_max = t1;
	*value_min = val;
	*index_min = t1;

	// first search for max
	for (auto i = t1 + 1; i <= t2; i++)
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

	// then search min after the max
	lp_buffer = lp_buffer_max;
	*value_min = *value_max;
	*index_min = *index_max;
	for (int i = *index_min + 1; i <= t2; i++)
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

long Spike::measure_sum_ex(const int i_first, const int i_last) const
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

void Spike::transfer_data_to_spike_buffer(short* source_data, const int source_n_channels, const int spike_length)
{
	spike_length_ = spike_length;
	auto lp_dest = get_p_data(spike_length);
	for (auto i = spike_length_; i > 0; i--, source_data += source_n_channels, lp_dest++)
	{
		const short val = *source_data;
		*lp_dest = static_cast<int>(val);
	}
}

void Spike::offset_spike_data(const int offset)
{
	auto lp_dest = get_p_data(spike_length_);
	for (auto i = get_spike_length(); i > 0; i--, lp_dest++)
		*lp_dest += offset; 
	
	offset_ = offset;
	value_max_ += offset;
	value_min_ += offset;
}

void Spike::offset_spike_data_to_average_ex(const int i_first, const int i_last)
{
	const long average_value = measure_sum_ex(i_first, i_last);
	const int offset = (average_value / (i_last - i_first + 1)) - bin_zero_;
	offset_spike_data(static_cast<int>(-offset));
}

void Spike::offset_spike_data_to_extrema_ex(const int i_first, const int i_last)
{
	int value_max, value_min;
	int max_index, min_index;
	measure_max_min_ex(&value_max, &max_index, &value_min, &min_index, i_first, i_last);
	const int offset = (value_max + value_min) / 2 - bin_zero_;
	offset_spike_data(offset);
}

void Spike::center_spike_amplitude(const int i_first, const int i_last, const WORD method)
{
	switch (method)
	{
		case 0:
			offset_spike_data_to_extrema_ex(i_first, i_last);
			break;
		case 1: 
			offset_spike_data_to_average_ex(i_first, i_last);
			break;
		default:
			break;
	}

	// change spike offset
	//SetAmplitudeOffset(get_amplitude_offset() + val_first);
	//SetMaxMinEx(max - val_first, min - val_first, d_max_to_min);
	//OffsetSpikeData(val_first);
}
