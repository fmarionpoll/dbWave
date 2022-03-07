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

	void EraseAllData();
	long GetSize() { return intervalpoint_array.GetSize(); }
	CIntervalPoint GetIntervalPointAt(int i);

	void ImportIntervalsSeries(CIntervals* pIntervals, WORD valUP = 1, BOOL bcopyRate = TRUE);
	void ImportAndMergeIntervalsArrays(CPtrArray* pIntervals);
	void ExportIntervalsSeries(int chan, CIntervals* pOut);

public:
	CArray<CIntervalPoint, CIntervalPoint> intervalpoint_array;
	float chrate;
	int version;
};

