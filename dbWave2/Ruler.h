#pragma once

class CRuler : public CObject
{
public:
	CRuler();
	virtual ~CRuler();
	void			SetRange(float* dfirst, float* dlast);
	inline int		GetScaleUnitPixels(int cx) { return (int)(m_dscaleinc * cx / (m_dlast - m_dfirst)); };
	inline double	GetScaleIncrement() { return m_dscaleinc; };
	void			UpdateRange(float* dfirst, float* dlast);

	BOOL	m_bHorizontal;		// orientation
	double	m_dfirst;			// actual lower value displayed
	double	m_dlast;			// actual higher value displayed
	double	m_dscaleinc;		// length of the major scale
	double	m_dscalefirst;		// first major scale
	double	m_dscalelast;		// last major scale

protected:
	BOOL	AdjustScale();
};
