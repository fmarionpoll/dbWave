#pragma once
#include "SCOPECHANNEL.h"

class SCOPESTRUCT : public CObject
{
	DECLARE_SERIAL(SCOPESTRUCT)
	SCOPESTRUCT();
	~SCOPESTRUCT() override;
	SCOPESTRUCT& operator =(const SCOPESTRUCT& arg);
	void Serialize(CArchive& ar) override;

public:
	WORD wversion{2};
	int iID{0};
	int iXCells{0};
	int iYCells{0};
	int iXTicks{0};
	int iYTicks{0};
	int iXTickLine{0};
	int iYTickLine{0};
	COLORREF crScopeFill{RGB(255, 255, 255)};
	COLORREF crScopeGrid{RGB(255, 255, 255)};
	BOOL bDrawframe{true};
	BOOL bClipRect{false};
	float xScaleUnitValue{0.f};
	float yScaleUnitValue{0.f};
	float xScaleSpan_s{0.f};
	float yScaleSpan_v{0.f};

	CArray<SCOPECHANNEL, SCOPECHANNEL> channels{};
};
