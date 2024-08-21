#pragma once
#include "Intervals.h"

struct CIntervalPoint
{
	long ii;
	WORD w;
};

class CIntervalPoints : public CObject
{
	DECLARE_SERIAL(CIntervalPoints)

	CIntervalPoints();
	CIntervalPoints(const CIntervalPoints& arg);
	~CIntervalPoints() override;
	void operator =(const CIntervalPoints& arg);
	void Serialize(CArchive& ar) override;

	void erase_all_data();
	long GetSize() { return intervalpoint_array.GetSize(); }
	CIntervalPoint get_interval_point_at(int i);

	void import_intervals_series(CIntervals* p_intervals, WORD val_up = 1, BOOL b_copy_rate = TRUE);
	void import_and_merge_intervals_arrays(const CPtrArray* p_source_intervals);
	void export_intervals_series(int chan, CIntervals* p_out);

public:
	CArray<CIntervalPoint, CIntervalPoint> intervalpoint_array;
	float chrate;
	int version;
};

