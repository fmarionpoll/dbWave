#include "StdAfx.h"
#include "IntervalsAndWordsSeries.h"


// -------------------------------------------------------
//
//IMPLEMENT_SERIAL(CIntervalPoint, CObject, 0 )
//
//CIntervalPoint::CIntervalPoint()
//{
//	ii = 0;
//	w = 0;
//}
//
//CIntervalPoint::CIntervalPoint(const CIntervalPoint& pt)
//{
//	ii = pt.ii;
//	w = pt.w;
//}
//
//CIntervalPoint::~CIntervalPoint()
//{
//}
//
//void CIntervalPoint::Serialize(CArchive & ar)
//{
//	if (ar.IsStoring())
//	{
//		int n = 2;
//		ar << n;
//		ar << ii;
//		ar << w;
//	}
//	else
//	{
//		int n;
//		ar >> n;
//		if (n>0) ar >> ii;	n--;
//		if (n>0) ar >> w;	n--;
//	}
//}
//
//void CIntervalPoint::operator = (const CIntervalPoint & arg)
//{
//	ii = arg.ii;
//	w = arg.w;
//}

// --------------------------------------------------------

IMPLEMENT_SERIAL(CIntervalsAndWordsSeries, CObject, 0)

CIntervalsAndWordsSeries::CIntervalsAndWordsSeries()
{
	version = 2;
	intervalpoint_array.SetSize(0);
	chrate = 10000.;
}

CIntervalsAndWordsSeries::CIntervalsAndWordsSeries(const CIntervalsAndWordsSeries& arg) : version(0)
{
	const auto nitems = arg.intervalpoint_array.GetSize();
	intervalpoint_array.SetSize(nitems);
	for (auto i = 0; i < nitems; i++)
		intervalpoint_array[i] = arg.intervalpoint_array.GetAt(i);
	chrate = arg.chrate;
}

CIntervalsAndWordsSeries::~CIntervalsAndWordsSeries()
{
}

void CIntervalsAndWordsSeries::Serialize(CArchive& ar)
{
	auto lversion = 2;
	if (ar.IsStoring())
	{
		auto n = 1;
		ar << n;
		ar << lversion;
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
		if (n > 0) ar >> lversion;
		n--;
		ar >> n;
		if (n > 0) intervalpoint_array.Serialize(ar);
		n--;
		if (lversion > 1) ar >> n;
		if (n > 0) ar >> chrate;
		n--;
	}
}

void CIntervalsAndWordsSeries::operator =(const CIntervalsAndWordsSeries& arg)
{
	const auto nitems = arg.intervalpoint_array.GetSize();
	intervalpoint_array.SetSize(nitems);
	for (auto i = 0; i < nitems; i++)
		intervalpoint_array[i] = arg.intervalpoint_array.GetAt(i);
	chrate = arg.chrate;
}

CIntervalPoint CIntervalsAndWordsSeries::GetIntervalPointAt(int i)
{
	return intervalpoint_array[i];
}

void CIntervalsAndWordsSeries::EraseAllData()
{
	intervalpoint_array.RemoveAll();
}

void CIntervalsAndWordsSeries::ImportIntervalsSeries(CIntervalsAndLevels* pIntervals, WORD valUP, BOOL bcopyRate)
{
	const auto ichrate = pIntervals->channel_sampling_rate;
	if (bcopyRate)
		chrate = pIntervals->channel_sampling_rate;
	const double ratio = chrate / ichrate;
	const int nitems = pIntervals->GetSize();
	intervalpoint_array.SetSize(nitems);
	const WORD w_low = 0;
	const auto w_up = valUP;
	auto w_state = w_up;
	CIntervalPoint dummy{};
	for (auto i = 0; i < nitems; i++)
	{
		dummy.ii = static_cast<long>(pIntervals->GetTimeIntervalAt(i) * ratio);
		dummy.w = w_state;
		intervalpoint_array[i] = dummy;
		if (w_state == w_low)
			w_state = w_up;
		else
			w_state = w_low;
	}
}

// combine up to 8 chans stored into CIntervalsAndLevels(s).
// in the resulting CIntervaAndWordsSeries, each bit is coding for a channel
// the channel number is sotred in the CIntervalsAndLevels (parameter "channel")
// 1) create separate CIntervalsAndWordsSeries objects with bits set
// 2) merge the series

void CIntervalsAndWordsSeries::ImportAndMergeIntervalsArrays(CPtrArray* pSourceIntervals)
{
	intervalpoint_array.RemoveAll();
	auto nseries = pSourceIntervals->GetSize();
	if (nseries > 8)
		nseries = 8;
	auto nintervals = 0;
	CArray<CIntervalsAndWordsSeries*, CIntervalsAndWordsSeries*> intervalsandwordseries_ptr_array;
	intervalsandwordseries_ptr_array.SetSize(8);

	// (1) transform series into CIntervalsAndWordSeries
	auto iseries = 0;
	for (auto i = 0; i < nseries; i++)
	{
		// transform this series if not empty
		auto* p_source = static_cast<CIntervalsAndLevels*>(pSourceIntervals->GetAt(i));
		if (p_source->GetSize() == 0)
			continue;

		auto pTransf = new CIntervalsAndWordsSeries();
		const WORD val_up = 2 << p_source->GetChannel();
		pTransf->ImportIntervalsSeries(p_source, val_up, FALSE);
		intervalsandwordseries_ptr_array[iseries] = pTransf;
		nintervals += pTransf->GetSize();
		iseries++;
	}

	// (2) now ptrInter contains 8 channels with a list of ON/OFF states (with the proper bit set)
	// merge the 8 channels

	WORD output_state = 0;
	for (auto i = 0; i < iseries; i++)
	{
		auto p_transf = intervalsandwordseries_ptr_array.GetAt(i);
		if (p_transf == nullptr)
			continue;

		// loop over all intervals stored in this transformed series
		auto k = 0;
		for (auto j = 0; j < p_transf->GetSize(); j++)
		{
			auto pt = p_transf->intervalpoint_array[j];
			BOOL b_found = false;

			// loop over all intervals stored into the local array and merge output state
			for (k; k < intervalpoint_array.GetSize(); k++)
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

	// (3) delete pTransf objects

	for (auto i = 0; i < 8; i++)
	{
		const auto p_interv = intervalsandwordseries_ptr_array.GetAt(i);
		if (p_interv != nullptr)
			delete p_interv;
	}
	intervalsandwordseries_ptr_array.RemoveAll();
}

void CIntervalsAndWordsSeries::ExportIntervalsSeries(int chan, CIntervalsAndLevels* pOut)
{
	const WORD ifilter = 2 << chan;
	WORD istatus = 0;
	for (auto i = 0; i < intervalpoint_array.GetSize(); i++)
	{
		if ((ifilter & intervalpoint_array[i].w) != istatus)
		{
			istatus = ifilter & intervalpoint_array[i].w;
			pOut->AddTimeInterval(intervalpoint_array[i].ii);
		}
	}
}
