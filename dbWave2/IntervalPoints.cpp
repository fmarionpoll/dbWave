#include "StdAfx.h"
#include "IntervalPoints.h"


IMPLEMENT_SERIAL(CIntervalPoints, CObject, 0)

CIntervalPoints::CIntervalPoints()
{
	version = 2;
	interval_point_array.SetSize(0);
	ch_rate = 10000.;
}

CIntervalPoints::CIntervalPoints(const CIntervalPoints& arg) : version(0)
{
	const auto n_items = arg.interval_point_array.GetSize();
	interval_point_array.SetSize(n_items);
	for (auto i = 0; i < n_items; i++)
		interval_point_array[i] = arg.interval_point_array.GetAt(i);
	ch_rate = arg.ch_rate;
}

CIntervalPoints::~CIntervalPoints()
= default;

void CIntervalPoints::Serialize(CArchive& ar)
{
	auto l_version = 2;
	if (ar.IsStoring())
	{
		auto n = 1;
		ar << n;
		ar << l_version;
		n = 1;
		ar << n;
		interval_point_array.Serialize(ar);
		n = 1;
		ar << n;
		ar << ch_rate;
	}
	else
	{
		int n;
		ar >> n;
		if (n > 0) ar >> l_version;
		n--;
		ar >> n;
		if (n > 0) interval_point_array.Serialize(ar);
		n--;
		if (l_version > 1) ar >> n;
		if (n > 0) ar >> ch_rate;
		n--;
	}
}

CIntervalPoints& CIntervalPoints::operator =(const CIntervalPoints& arg)
{
	if (this != &arg)
	{
		const auto n_items = arg.interval_point_array.GetSize();
		interval_point_array.SetSize(n_items);
		for (auto i = 0; i < n_items; i++)
			interval_point_array[i] = arg.interval_point_array.GetAt(i);
		ch_rate = arg.ch_rate;
	}
	return *this;
}

CIntervalPoint CIntervalPoints::get_interval_point_at(int i)
{
	return interval_point_array[i];
}

void CIntervalPoints::erase_all_data()
{
	interval_point_array.RemoveAll();
}

void CIntervalPoints::import_intervals_series(CIntervals* p_intervals, const WORD val_up, const BOOL b_copy_rate)
{
	const auto i_ch_rate = p_intervals->channel_sampling_rate;
	if (b_copy_rate)
		ch_rate = p_intervals->channel_sampling_rate;
	const double ratio = ch_rate / i_ch_rate;
	const int n_items = p_intervals->get_size();
	interval_point_array.SetSize(n_items);
	const auto w_up = val_up;
	auto w_state = w_up;
	
	for (auto i = 0; i < n_items; i++)
	{
		constexpr WORD w_low = 0;
		CIntervalPoint dummy;
		dummy.ii = static_cast<long>(p_intervals->get_at(i) * ratio);
		dummy.w = w_state;
		interval_point_array[i] = dummy;
		if (w_state == w_low)
			w_state = w_up;
		else
			w_state = w_low;
	}
}

// combine up to 8 channels stored into CIntervals(s).
// in the resulting CIntervalAndWordsSeries, each bit is coding for a channel
// the channel number is sorted in the CIntervals (parameter "channel")
// 1) create separate CIntervalPoints objects with bits set
// 2) merge the series

void CIntervalPoints::import_and_merge_intervals_arrays(const CPtrArray* p_source_intervals)
{
	interval_point_array.RemoveAll();
	auto n_series = p_source_intervals->GetSize();
	if (n_series > 8)
		n_series = 8;
	auto n_intervals = 0;
	CArray<CIntervalPoints*, CIntervalPoints*> intervals_and_word_series_ptr_array;
	intervals_and_word_series_ptr_array.SetSize(8);

	// (1) transform series into CIntervalsAndWordSeries
	auto i_series = 0;
	for (auto i = 0; i < n_series; i++)
	{
		// transform this series if not empty
		auto* p_source = static_cast<CIntervals*>(p_source_intervals->GetAt(i));
		if (p_source->get_size() == 0)
			continue;

		const auto p_transfer = new CIntervalPoints();
		const WORD val_up = static_cast<WORD>(2 << p_source->get_channel());
		p_transfer->import_intervals_series(p_source, val_up, FALSE);
		intervals_and_word_series_ptr_array[i_series] = p_transfer;
		n_intervals += p_transfer->get_size();
		i_series++;
	}

	// (2) now ptrInter contains 8 channels with a list of ON/OFF states (with the proper bit set)
	// merge the 8 channels

	WORD output_state = 0;
	for (auto i = 0; i < i_series; i++)
	{
		const auto p_transfer = intervals_and_word_series_ptr_array.GetAt(i);
		if (p_transfer == nullptr)
			continue;

		// loop over all intervals stored in this transformed series
		for (auto j = 0; j < p_transfer->get_size(); j++)
		{
			auto pt = p_transfer->interval_point_array[j];
			BOOL b_found = false;

			// loop over all intervals stored into the local array and merge output state
			for (auto k=0; k < interval_point_array.GetSize(); k++)
			{
				if (pt.ii < interval_point_array.GetAt(k).ii)
				{
					pt.w = output_state & pt.w; // merge with previous status
					interval_point_array.InsertAt(k, pt);
					b_found = true;
					break;
				}
				if (pt.ii == interval_point_array.GetAt(k).ii)
				{
					pt.w = interval_point_array.GetAt(k).w & pt.w; // merge with current status
					interval_point_array.SetAt(k, pt);
					b_found = true;
					break;
				}
				output_state = interval_point_array.GetAt(k).w; // update output and continue
			}

			// not found into existing intervals? add new interval
			if (!b_found)
			{
				pt.w = output_state & pt.w;
				interval_point_array.Add(pt);
			}
		}
	}

	// (3) delete pTransfer objects

	for (auto i = 0; i < 8; i++)
	{
		const auto p_intervals = intervals_and_word_series_ptr_array.GetAt(i);
		delete p_intervals;
	}
	intervals_and_word_series_ptr_array.RemoveAll();
}

void CIntervalPoints::export_intervals_series(const int chan, CIntervals* p_out)
{
	const WORD i_filter = static_cast<WORD>(2 << chan);
	WORD i_status = 0;
	for (auto i = 0; i < interval_point_array.GetSize(); i++)
	{
		if ((i_filter & interval_point_array[i].w) != i_status)
		{
			i_status = i_filter & interval_point_array[i].w;
			p_out->add_item(interval_point_array[i].ii);
		}
	}
}
