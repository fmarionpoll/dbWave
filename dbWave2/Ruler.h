#pragma once

class Ruler : public CObject
{
public:
	Ruler();
	~Ruler() override;

	void set_range(double d_first, double d_last);
	int get_scale_unit_pixels(const int cx) const
		{ return static_cast<int>(m_length_major_scale * cx / (m_highest_value - m_lowest_value)); }
	double get_scale_increment() const
		{ return m_length_major_scale; }
	void update_range(double d_first, double d_last);

	BOOL m_is_horizontal {true}; 
	double m_lowest_value {0.}; 
	double m_highest_value {0.};
	double m_length_major_scale {0.}; 
	double m_first_major_scale {0.};
	double m_last_major_scale {0.};

protected:
	BOOL adjust_scale();
};
