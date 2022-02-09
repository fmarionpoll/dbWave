#pragma once

class CRuler : public CObject
{
public:
	CRuler();
	~CRuler() override;

	void SetRange(double dfirst, double dlast);
	int GetScaleUnitPixels(int cx) const
		{ return static_cast<int>(m_length_major_scale * cx / (m_highest_value - m_lowest_value)); }
	double GetScaleIncrement() const
		{ return m_length_major_scale; }
	void UpdateRange(double dfirst, double dlast);

	BOOL m_is_horizontal {true}; 
	double m_lowest_value {0.}; 
	double m_highest_value {0.};
	double m_length_major_scale {0.}; 
	double m_first_major_scale {0.};
	double m_last_major_scale {0.};

protected:
	BOOL AdjustScale();
};
