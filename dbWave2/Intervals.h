#pragma once


class CIntervals : public CObject
{
	DECLARE_SERIAL(CIntervals)

public:
	CIntervals();
	~CIntervals() override;
	void Serialize(CArchive& ar) override;

	CIntervals& operator =(const CIntervals& arg);

	long GetAt(int i) { return array.GetAt(i); }
	void SetAt(int i, long ii_time) {array.SetAt(i, ii_time); }
	void SetAtGrow(int i, long value) { array.SetAtGrow(i, value); }
	long GetSize() const { return array.GetSize(); }
	void Add(long ii) { array.Add(ii); n_items++; }
	void RemoveAll() { array.RemoveAll(); n_items = 0; }
	void RemoveAt(int i) { array.RemoveAt(i); n_items--; }
	void InsertAt(int i, long value) { array.InsertAt(i, value); }

	int  GetChannel() { return channel; }
	void SetChannel(int chan) { channel = chan; }

public:
	int iID = 1;
	int channel = 0;
	CString cs_descriptor = _T("stimulus intervals"); 
	int n_items = 0;		
	int n_per_cycle = 1; 
	float channel_sampling_rate = 10000.f;

protected:
	CArray<long, long> array; // time on, time off (n clock intervals)
	int version = 4;
};

