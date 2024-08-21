#include "StdAfx.h"
#include "IntervalPoints.h"


IMPLEMENT_SERIAL(CIntervalPoints, CObject, 0)

CIntervalPoints::CIntervalPoints()
{
	version = 2;
	intervalpoint_array.SetSize(0);
	chrate = 10000.;
}

CIntervalPoints::CIntervalPoints(const CIntervalPoints& arg) : version(0)
{
	const auto n_items = arg.intervalpoint_array.GetSize();
	intervalpoint_array.SetSize(n_items);
	for (auto i = 0; i < n_items; i++)
		intervalpoint_array[i] = arg.intervalpoint_array.GetAt(i);
	chrate = arg.chrate;
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
		intervalpoint_array.Serialize(ar);
		n = 1;
		ar << n;
		ar << chrate;
	}
	else
	{
		int n;
		ar >> n;
		if (n > 0) ar >> l_version;
		n--;
		ar >> n;
		if (n > 0) intervalpoint_array.Serialize(ar);
		n--;
		if (l_version > 1) ar >> n;
		if (n > 0) ar >> chrate;
		n--;
	}
}

void CIntervalPoints::operator =(const CIntervalPoints& arg)
{
	const auto n_items = arg.intervalpoint_array.GetSize();
	intervalpoint_array.SetSize(n_items);
	for (auto i = 0; i < n_items; i++)
		intervalpoint_array[i] = arg.intervalpoint_array.GetAt(i);
	chrate = arg.chrate;
}

CIntervalPoint CIntervalPoints::get_interval_point_at(int i)
{
	return intervalpoint_array[i];
}

void CIntervalPoints::erase_all_data()
{
	intervalpoint_array.RemoveAll();
}

void CIntervalPoints::import_intervals_series(CIntervals* p_intervals, const WORD val_up, const BOOL b_copy_rate)
{
	const auto i_ch_rate = p_intervals->channel_sampling_rate;
	if (b_copy_rate)
		chrate = p_intervals->channel_sampling_rate;
	const double ratio = chrate / i_ch_rate;
	const int n_items = p_intervals->GetSize();
	intervalpoint_array.SetSize(n_items);
	const auto w_up = val_up;
	auto w_state = w_up;
	
	for (auto i = 0; i < n_items; i++)
	{
		constexpr WORD w_low = 0;
		CIntervalPoint dummy;
		dummy.ii = static_cast<long>(p_intervals->GetAt(i) * ratio);
		dummy.w = w_state;
		intervalpoint_array[i] = dummy;
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
	intervalpoint_array.RemoveAll();
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
		if (p_source->GetSize() == 0)
			continue;

		const auto p_transfer = new CIntervalPoints();
		const WORD val_up = static_cast<WORD>(2 << p_source->GetChannel());
		p_transfer->import_intervals_series(p_source, val_up, FALSE);
		intervals_and_word_series_ptr_array[i_series] = p_transfer;
		n_intervals += p_transfer->GetSize();
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
		for (auto j = 0; j < p_transfer->GetSize(); j++)
		{
			auto pt = p_transfer->intervalpoint_array[j];
			BOOL b_found = false;

			// loop over all intervals stored into the local array and merge output state
			for (auto k=0; k < intervalpoint_array.GetSize(); k++)
			{
				if (pt.ii < intervalpoint_array.GetAt(k).ii)
				{
					pt.w = output_state & pt.w; // merge with previous status
					intervalpoint_array.InsertAt(k, pt);
					b_found = true;
					break;
				}
				if (pt.ii == intervalpoint_array.GetAt(k).ii)
				{
					pt.w = intervalpoint_array.GetAt(k).w & pt.w; // merge with current status
					intervalpoint_array.SetAt(k, pt);
					b_found = true;
					break;
				}
				output_state = intervalpoint_array.GetAt(k).w; // update output and continue
			}

			// not found into existing intervals? add new interval
			if (!b_found)
			{
				pt.w = output_state & pt.w;
				intervalpoint_array.Add(pt);
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
	for (auto i = 0; i < intervalpoint_array.GetSize(); i++)
	{
		if ((i_filter & intervalpoint_array[i].w) != i_status)
		{
			i_status = i_filter & intervalpoint_array[i].w;
			p_out->Add(intervalpoint_array[i].ii);
		}
	}
}
