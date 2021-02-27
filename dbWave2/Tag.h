#pragma once

class CTag : public CObject
{
	DECLARE_SERIAL(CTag)
	// Construction Destruction
public:
	CTag();
	CTag(int refchan);
	CTag(int val, int ref_chan);
	CTag(long long lTicks);
	CTag(const CTag& hc);
	~CTag();

	CTag& operator = (const CTag& arg);		// operator redefinition
	void Serialize(CArchive& ar) override;
	long Write(CFile* pdatafile);
	BOOL Read(CFile* pdatafile);

	// Management procedures
public:
	int 		m_refchan	= -1;		// channel associated (-1) if none
	int 		m_pixel		= -1;
	int 		m_value		= 0;		// line val (for horizontal bar)
	long		m_lvalue	= -1;		// assoc long val (for vertical bar)
	long long	m_lTicks	= 0;		// length in ticks
	CString		m_csComment{};			// comment
};

