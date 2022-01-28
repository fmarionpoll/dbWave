#include "stdafx.h"
#include "SCOPECHANNEL.h"

IMPLEMENT_SERIAL(SCOPECHANNEL, CObject, 0 /* schema number*/)

SCOPECHANNEL::SCOPECHANNEL()
{
}

SCOPECHANNEL::~SCOPECHANNEL()
{
}

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
		int version = 1;
		ar << version;
		ar << iextent;
		ar << izero;
	}
	else
	{
		int version;
		ar >> version;
		ar >> iextent;
		ar >> izero;
	}
}
