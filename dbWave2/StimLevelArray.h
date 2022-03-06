#pragma once


class CIntervalsAndLevels : public CObject
{
	DECLARE_SERIAL(CIntervalsAndLevels)

public:
	CIntervalsAndLevels();
	~CIntervalsAndLevels() override;
	void Serialize(CArchive& ar) override;

	CIntervalsAndLevels& operator =(const CIntervalsAndLevels& arg);

	long GetiiTime(int i) { return intervalsArray.GetAt(i); };
	void SetiiTime(int i, long iitime) { intervalsArray.SetAt(i, iitime); };
	long GetSize() { return intervalsArray.GetSize(); }
	int GetChan() { return ichan; }
	void SetChan(int chan) { ichan = chan; }
	void AddInterval(long ii) { intervalsArray.Add(ii); }

public:
	CArray<long, long> intervalsArray; // time on, time off (n clock intervals)
	int iID; // ID number of the array
	int ichan;
	CString csDescriptor; // descriptor of the array
	int nitems;		// number of on/off events
	int npercycle; // repeat sequence parameter
	float chrate; // n clock cycles per second
protected:
	int version;
};

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
