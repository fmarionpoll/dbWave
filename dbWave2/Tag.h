#pragma once

class Tag : public CObject
{
	DECLARE_SERIAL(Tag)

	Tag();
	Tag(int ref_chan);
	Tag(int val, int ref_chan);
	Tag(long l_val, int ref_chan);
	Tag(long long ticks);
	Tag(const Tag& hc);
	~Tag();

	Tag& operator =(const Tag& arg);
	void Serialize(CArchive& ar) override;
	long write(CFile* p_data_file);
	BOOL read(CFile* p_data_file);

	//data
	int ref_channel = -1;		// channel associated (-1) if none
	int pixel = -1;
	int old_pixel = -1;			// value stored (<0: none)
	int value_int = 0;			// line value (for horizontal bar)
	long value_long = -1;		// assoc long value (for vertical bar)
	long long l_ticks = 0;		// length in ticks
	double value_mv = 0.;
	CString m_cs_comment{};		// comment
};
