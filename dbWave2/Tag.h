#pragma once

class Tag : public CObject
{
	DECLARE_SERIAL(Tag)

	Tag();
	Tag(int refchan);
	Tag(int val, int ref_chan);
	Tag(long long lTicks);
	Tag(const Tag& hc);
	~Tag();

	Tag& operator =(const Tag& arg);
	void Serialize(CArchive& ar);
	long Write(CFile* pdatafile);
	BOOL Read(CFile* pdatafile);

	// Management procedures
public:
	int m_refchan = -1;			// channel associated (-1) if none
	int m_pixel = -1;
	int m_value = 0;			// line value (for horizontal bar)
	long m_lvalue = -1;			// assoc long value (for vertical bar)
	long long m_lTicks = 0;		// length in ticks
	CString m_csComment{};		// comment
};
