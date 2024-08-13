#include "StdAfx.h"
#include "OPTIONS_SCOPE_CHANNEL.h"

IMPLEMENT_SERIAL(options_scope_channel, CObject, 0 /* schema number*/)

options_scope_channel::options_scope_channel()
= default;

options_scope_channel::~options_scope_channel()
= default;

options_scope_channel& options_scope_channel::operator=(const options_scope_channel& arg)
{
	if (this != &arg)
	{
		i_extent = arg.i_extent;
		i_zero = arg.i_zero;
	}
	return *this;
}

void options_scope_channel::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << version;
		ar << i_extent;
		ar << i_zero;
	}
	else
	{
		ar >> version;
		ar >> i_extent;
		ar >> i_zero;
	}
}
