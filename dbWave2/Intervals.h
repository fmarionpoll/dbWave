#pragma once


class CIntervals : public CObject
{
	DECLARE_SERIAL(CIntervals)

public:
	CIntervals();
	~CIntervals() override;
	void Serialize(CArchive& ar) override;

	CIntervals& operator =(const CIntervals& arg);

	long GetTimeIntervalAt(int i) { return intervalsArray.GetAt(i); }
	void SetTimeIntervalAt(int i, long iitime) { intervalsArray.SetAt(i, iitime); }
	long GetSize() const { return intervalsArray.GetSize(); }
	int  GetChannel() { return channel; }
	void SetChannel(int chan) { channel = chan; }
	void AddTimeInterval(long ii) { intervalsArray.Add(ii); }

public:
	CArray<long, long> intervalsArray ; // time on, time off (n clock intervals)
	int iID = 1;
	int channel = 0;
	CString cs_descriptor = _T("stimulus intervals"); 
	int n_items = 0;		
	int n_per_cycle = 1; 
	float channel_sampling_rate = 10000.f;

protected:
	int version = 4;
};

