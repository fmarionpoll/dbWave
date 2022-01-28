#pragma once
#include "SCOPECHANNEL.h"

class SCOPESTRUCT : public CObject
{
	DECLARE_SERIAL(SCOPESTRUCT)
	SCOPESTRUCT();
	~SCOPESTRUCT();
	SCOPESTRUCT& operator = (const SCOPESTRUCT& arg);
	void Serialize(CArchive& ar) override;

public:
	WORD		wversion;
	int			iID;
	int			iXCells;
	int			iYCells;
	int			iXTicks;
	int			iYTicks;
	int			iXTickLine;
	int			iYTickLine;
	COLORREF	crScopeFill;
	COLORREF	crScopeGrid;
	BOOL		bDrawframe;
	BOOL		bClipRect;
	float		xScaleUnitValue;
	float		yScaleUnitValue;
	float		xScaleSpan_s;
	float		yScaleSpan_v;

	CArray <SCOPECHANNEL, SCOPECHANNEL> channels;
};

