#pragma once
#include "IntervalsAndLevels.h"

struct CIntervalPoint
{
	long ii;
	WORD w;
};

class CIntervalsAndWordsSeries : public CObject
{
	DECLARE_SERIAL(CIntervalsAndWordsSeries)

	CIntervalsAndWordsSeries();
	CIntervalsAndWordsSeries(const CIntervalsAndWordsSeries& arg);
	~CIntervalsAndWordsSeries() override;
	void operator =(const CIntervalsAndWordsSeries& arg);
	void Serialize(CArchive& ar) override;

	void EraseAllData();
	long GetSize() { return intervalpoint_array.GetSize(); }
	CIntervalPoint GetIntervalPointAt(int i);

	void ImportIntervalsSeries(CIntervalsAndLevels* pIntervals, WORD valUP = 1, BOOL bcopyRate = TRUE);
	void ImportAndMergeIntervalsArrays(CPtrArray* pIntervals);
	void ExportIntervalsSeries(int chan, CIntervalsAndLevels* pOut);

public:
	CArray<CIntervalPoint, CIntervalPoint> intervalpoint_array;
	float chrate;
	int version;
};

