#include "StdAfx.h"
#include "SCOPECHANNEL.h"

IMPLEMENT_SERIAL(SCOPECHANNEL, CObject, 0 /* schema number*/)

SCOPECHANNEL::SCOPECHANNEL()
= default;

SCOPECHANNEL::~SCOPECHANNEL()
= default;

SCOPECHANNEL& SCOPECHANNEL::operator=(const SCOPECHANNEL& arg)
{
	if (this != &arg)
	{
		iextent = arg.iextent;
		izero = arg.izero;
	}
	return *this;
}

void SCOPECHANNEL::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << version;
		ar << iextent;
		ar << izero;
	}
	else
	{
		ar >> version;
		ar >> iextent;
		ar >> izero;
	}
}
