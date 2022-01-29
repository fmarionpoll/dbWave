#include "StdAfx.h"
#include "SCOPESTRUCT.h"


IMPLEMENT_SERIAL(SCOPESTRUCT, CObject, 0 /* schema number*/)

SCOPESTRUCT::SCOPESTRUCT()
= default;

SCOPESTRUCT::~SCOPESTRUCT()
= default;

SCOPESTRUCT& SCOPESTRUCT::operator =(const SCOPESTRUCT& arg)
{
	if (this != &arg)
	{
		iID = arg.iID;
		iXCells = arg.iXCells;
		iYCells = arg.iYCells;
		iXTicks = arg.iXTicks;
		iYTicks = arg.iYTicks;
		iXTickLine = arg.iXTickLine;
		iYTickLine = arg.iYTickLine;
		crScopeFill = arg.crScopeFill;
		crScopeGrid = arg.crScopeGrid;
		bDrawframe = arg.bDrawframe;
		xScaleUnitValue = arg.xScaleUnitValue;
		yScaleUnitValue = arg.yScaleUnitValue;

		channels.SetSize(arg.channels.GetSize());
		for (int i = 0; i < arg.channels.GetSize(); i++)
			channels[i] = arg.channels[i];
	}
	return *this;
}

void SCOPESTRUCT::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		wversion = 3;
		ar << wversion;
		const int nints = 8;
		ar << nints;
		ar << iID;
		ar << iXCells;
		ar << iYCells;
		ar << iXTicks;
		ar << iYTicks;
		ar << iXTickLine;
		ar << iYTickLine;
		ar << bDrawframe;
		const int ncolor_items = 2;
		ar << ncolor_items;
		ar << crScopeFill;
		ar << crScopeGrid;
		// save scale values
		const int nfloats = 2;
		ar << nfloats;
		ar << xScaleUnitValue;
		ar << yScaleUnitValue;
	}
	else
	{
		WORD wwversion;
		ar >> wwversion;
		int nints;
		ar >> nints;
		ar >> iID;
		nints--;
		ar >> iXCells;
		nints--;
		ar >> iYCells;
		nints--;
		ar >> iXTicks;
		nints--;
		ar >> iYTicks;
		nints--;
		ar >> iXTickLine;
		nints--;
		ar >> iYTickLine;
		nints--;
		ar >> bDrawframe;
		nints--;
		ASSERT(nints == 0);

		int ncolor_items;
		ar >> ncolor_items;
		ar >> crScopeFill;
		ncolor_items--;
		ar >> crScopeGrid;
		ncolor_items--;
		ASSERT(ncolor_items == 0);

		if (wwversion > 1)
		{
			int nfloats;
			ar >> nfloats;
			ar >> xScaleUnitValue;
			nfloats--;
			ar >> yScaleUnitValue;
			nfloats--;
			while (nfloats > 0)
			{
				float x;
				ar >> x;
				nfloats--;
			}
		}
	}
}
