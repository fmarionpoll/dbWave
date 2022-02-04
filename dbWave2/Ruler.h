#pragma once

class CRuler : public CObject
{
public:
	CRuler();
	~CRuler() override;
	void SetRange(double dfirst, double dlast);
	int GetScaleUnitPixels(int cx) const;
	double GetScaleIncrement() const;
	void UpdateRange(double dfirst, double dlast);

	BOOL m_is_horizontal {true}; // orientation
	double m_dfirst {0.}; // actual lower value displayed
	double m_dlast {0.}; // actual higher value displayed
	double m_dscaleinc {0.}; // length of the major scale
	double m_dscalefirst {0.}; // first major scale
	double m_dscalelast {0.}; // last major scale

protected:
	BOOL AdjustScale();
};
