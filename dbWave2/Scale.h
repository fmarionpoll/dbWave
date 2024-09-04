#pragma once

class Scale : public CObject
{
public:
	Scale();
	Scale(int n_pixels); 
	DECLARE_SERIAL(Scale)
	void Serialize(CArchive& ar) override;

protected:
	int m_n_pixels_ = 1; 
	int m_n_intervals_ = 0; 
	long m_l_n_data_points_ = 0; 
	CWordArray m_intervals_; 
	CDWordArray m_position_; 

public:
	int set_scale(int n_pixels, long n_data_points);
	int how_many_intervals_fit(int first_pixel, long* l_last);
	int get_which_interval(long l_index);
	int get_n_pixels() const { return m_n_pixels_; }
	int get_n_intervals() const { return m_n_intervals_; }
	long get_n_data_points() const { return m_l_n_data_points_; }
	LPWORD element_at(int i) { return &(m_intervals_[i]); }
	long get_position(int i) { return static_cast<long>(m_position_[i]); }
	int get_interval_size(int i) { return m_intervals_[i]; }
};
