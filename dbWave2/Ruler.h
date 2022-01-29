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

	BOOL m_is_horizontal; // orientation
	double m_dfirst; // actual lower value displayed
	double m_dlast; // actual higher value displayed
	double m_dscaleinc; // length of the major scale
	double m_dscalefirst; // first major scale
	double m_dscalelast; // last major scale

protected:
	BOOL AdjustScale();
};
