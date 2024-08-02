#pragma once

class Spike : public CObject
{
	DECLARE_SERIAL(Spike)

	Spike();
	Spike(long time, int channel);
	Spike(long time, int channel, short offset, int class_i, int spike_length);
	Spike(long time, int channel, short max, short min, short offset, int class_i, short d_max_min, int spike_length);
	~Spike() override;

	// Attributes
private:
	long m_ii_time_ = 0;					// occurrence time - multiply by rate to get time in seconds
	int m_class_id_ = 0;					// spike class - init to zero at first
	int m_detection_parameters_index_ = 0;	// spike detection array index
	short m_value_max_ = 4096;				// spike max (used to scan rapidly to adjust display)
	short m_value_min_ = 0;					// spike min (used to scan rapidly to adjust display)
	short m_d_max_min_ = 0;
	short m_offset_ = 2048;					// offset voltage pt 1
	int y1_ = 0;							// parameter measured and stored
	int y2_ = 0;
	int dt_ = 0;

	short* m_spike_data_buffer_ = nullptr;	// buffer address
	int m_spike_length_ = 0;				// length of buffer
	int m_spk_buffer_length_ = 0;			// n points in the buffer
	boolean m_selected_state_ = false;
	short m_bin_zero_ = 2048;				// zero (2048 if 12 bits scale = 0-4095)

public:
	short	get_bin_zero() const { return m_bin_zero_; }
	void	set_bin_zero(const short bin_zero) { m_bin_zero_ = bin_zero; }

	boolean get_selected() const { return m_selected_state_; }
	void	set_selected(const boolean status) { m_selected_state_ = status; }

	long	get_time() const { return m_ii_time_; }
	int		get_class_id() const { return m_class_id_; }
	int		get_source_channel() const { return m_detection_parameters_index_; }
	short	get_maximum() const { return m_value_max_; }
	short	get_minimum() const { return m_value_min_; }
	short	get_amplitude_offset() const { return m_offset_; }

	int		get_spike_length() const { return m_spike_length_; }
	void	set_spike_length(const int spike_length) { m_spike_length_ = spike_length; }
	short*	get_p_data(int spike_length);
	short*	get_p_data() const;
	short	get_value_at_offset(const int index) const {return *(m_spike_data_buffer_+index);}

	int		get_y1() const { return y1_; }
	void	set_y1(const int y) { y1_ = y; }

	int		get_y2() const { return y2_; }
	void	set_y2(const int y) { y2_ = y; }

	int		get_dt() const { return dt_; }
	void	set_dt(const long x) { dt_ = x; }

	void	set_time(const long ii) { m_ii_time_ = ii; }
	void	set_class_id(const int cl) { m_class_id_ = cl; }

	void	get_max_min(short* max, short* min) const;
	void	get_max_min_ex(short* max, short* min, int* d_max_to_min) const;
	void	set_max_min_ex(const short max, const short min, const short d_max_min) {
				m_value_max_ = max;
				m_value_min_ = min;
				m_d_max_min_ = d_max_min; }

	void	set_amplitude_offset(const short offset) { m_offset_ = offset; }

	void	transfer_data_to_spike_buffer(short* source_data, int source_n_channels, int spike_length);

	void	measure_max_min_ex(short* value_max, int* index_max, short* value_min, int* index_min, int i_first, int i_last) const;
	void	measure_max_then_min_ex(short* value_max, int* index_max, short* value_min, int* index_min, int i_first, int i_last) const;
	long	measure_sum_ex(int i_first, int i_last) const;

	void	offset_spike_data(short offset);
	void	offset_spike_data_to_average_ex(int i_first, int i_last);
	void	offset_spike_data_to_extrema_ex(int i_first, int i_last);
	void	center_spike_amplitude(int i_first, int i_last, WORD method = 0);

	// Implementation
	void	read_version0(CArchive& ar);
	void	Serialize(CArchive& ar) override;

protected:
	void	read_version2(CArchive& ar, WORD w_version);
};
