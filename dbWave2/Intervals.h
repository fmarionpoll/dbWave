#pragma once


class CIntervals : public CObject
{
	DECLARE_SERIAL(CIntervals)

	CIntervals();
	~CIntervals() override;
	void Serialize(CArchive& ar) override;
	CIntervals& operator =(const CIntervals& arg);

	long get_at(const int i) { return array_.GetAt(i); }
	void set_at(const int i, const long ii_time) {array_.SetAt(i, ii_time); }
	void set_at_grow(const int i, const long l_value) { array_.SetAtGrow(i, l_value); }
	long get_size() const { return array_.GetSize(); }
	void add_item(const long ii) { array_.Add(ii); n_items++; }
	void remove_all() { array_.RemoveAll(); n_items = 0; }
	void remove_at(const int i) { array_.RemoveAt(i); n_items--; }
	void insert_at(const int i, const long l_value) { array_.InsertAt(i, l_value); }
	int  get_channel() const { return channel; }
	void set_channel(const int chan) { channel = chan; }

	int i_id = 1;
	int channel = 0;
	CString cs_descriptor = _T("stimulus intervals"); 
	int n_items = 0;		
	int n_per_cycle = 1; 
	float channel_sampling_rate = 10000.f;

protected:
	CArray<long, long> array_; // time on, time off (n clock intervals)
	int version_ = 4;
};

