#include "StdAfx.h"
#include "Tag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(Tag, CObject, 1 /* schema number*/)

Tag::Tag()
= default;

Tag::Tag(const int ref_chan)
{
	ref_channel = ref_chan;
}

Tag::Tag(const int val, const int ref_chan)
{
	ref_channel = ref_chan;
	value_int = val;
}

Tag::Tag(const long l_val, const int ref_chan)
{
	ref_channel = ref_chan;
	value_long = l_val;
}

Tag::Tag(const long long ticks)
{
	l_ticks = ticks;
}

Tag::Tag(const Tag& hc)
{
	ref_channel = hc.ref_channel;
	value_int = hc.value_int;
	pixel = hc.pixel;
	value_long = hc.value_long;
	m_cs_comment = hc.m_cs_comment;
}

Tag::~Tag()
= default;

Tag& Tag::operator =(const Tag& arg)
{
	if (this != &arg)
	{
		ref_channel = arg.ref_channel;
		pixel = arg.pixel;
		value_int = arg.value_int;
		value_long = arg.value_long;
		l_ticks = arg.l_ticks;
		m_cs_comment = arg.m_cs_comment;
	}
	return *this;
}

void Tag::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << static_cast<WORD>(ref_channel);
		ar << static_cast<WORD>(pixel);
		ar << static_cast<WORD>(value_int);
		ar << value_long;
		ar << m_cs_comment;
	}
	else
	{
		const int n_version = ar.GetObjectSchema();
		WORD w1;
		ar >> w1;
		ref_channel = w1;
		ar >> w1;
		pixel = w1;
		ar >> w1;
		value_int = w1;
		ar >> value_long;
		ar >> m_cs_comment;
	}
}

long Tag::write(CFile* p_data_file)
{
	const auto p1 = p_data_file->GetPosition();
	CArchive ar(p_data_file, CArchive::store);
	Serialize(ar);
	ar.Close();
	const auto p2 = p_data_file->GetPosition();
	return static_cast<long>(p2 - p1);
}

BOOL Tag::read(CFile* p_data_file)
{
	CArchive ar(p_data_file, CArchive::load);
	Serialize(ar);
	ar.Close();
	return TRUE;
}


