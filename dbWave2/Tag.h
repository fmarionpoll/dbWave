#pragma once

class CTag : public CObject
{
	DECLARE_SERIAL(CTag)
	// Construction Destruction
public:
	CTag();
	CTag(int refchan);
	CTag(int val, int ref_chan);
	CTag(long lval, int ref_chan);
	CTag(long long lTicks);
	CTag(const CTag& hc);
	~CTag();

	CTag& operator = (const CTag& arg);		// operator redefinition
	void Serialize(CArchive& ar) override;
	long Write(CFile* pdatafile);
	BOOL Read(CFile* pdatafile);

	// Management procedures
public:
	int 	m_refchan;		// channel associated (-1) if none
	int 	m_pixel;
	int 	m_value;		// line val (for horizontal bar)
	long	m_lvalue;		// assoc long val (for vertical bar)
	long long m_lTicks;		// length in ticks
	CString m_csComment;	// comment
};

