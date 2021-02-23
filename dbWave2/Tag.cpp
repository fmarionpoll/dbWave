#include "StdAfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTag : base element of CTagList

IMPLEMENT_SERIAL(CTag, CObject, 0 /* schema number*/)

CTag::CTag()
{
}

CTag::CTag(int ref_chan) 
{
	m_refchan = ref_chan;
}

CTag::CTag(int val, int ref_chan)
{
	m_refchan = ref_chan;
	m_value = val;
}

CTag::CTag(long long lTicks)
{
	m_lTicks = lTicks;
}

CTag::CTag(const CTag& hc)
{
	m_refchan = hc.m_refchan;
	m_value = hc.m_value;
	m_pixel = hc.m_pixel;
	m_lvalue = hc.m_lvalue;
	m_csComment = hc.m_csComment;
}

CTag::~CTag()
{
}

CTag& CTag::operator = (const CTag& arg)
{
	if (this != &arg) {
		m_refchan = arg.m_refchan;
		m_pixel = arg.m_pixel;
		m_value = arg.m_value;
		m_lvalue = arg.m_lvalue;
		m_lTicks = arg.m_lTicks;
		m_csComment = arg.m_csComment;
	}
	return *this;
}

void CTag::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << static_cast<WORD>(m_refchan);
		ar << static_cast<WORD>(m_pixel);
		ar << static_cast<WORD>(m_value);
		ar << m_lvalue;
		ar << m_csComment;
	}
	else
	{
		WORD w1;
		ar >> w1; m_refchan = w1;
		ar >> w1; m_pixel = w1;
		ar >> w1; m_value = w1;
		ar >> m_lvalue;
		ar >> m_csComment;
	}
}

long CTag::Write(CFile* datafile)
{
	const auto p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	const auto p2 = datafile->GetPosition();
	return static_cast<long>(p2 - p1);
}

BOOL CTag::Read(CFile* datafile)
{
	CArchive ar(datafile, CArchive::load);
	Serialize(ar);
	ar.Close();
	return TRUE;
}

