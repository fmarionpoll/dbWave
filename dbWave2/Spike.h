#pragma once

class Spike : public CObject
{
	DECLARE_SERIAL(Spike)

	Spike();
	Spike(long time, int channel);
	Spike(long time, int channel, int offset, int class_i, int spike_length);
	Spike(long time, int channel, int max, int min, int offset, int class_i, int d_max_min, int spike_length);
	~Spike() override;

	// Attributes
private:
	long ii_time_ {0};					// occurrence time in ticks
	int class_id_ {0};					// spike class - init to zero at first
	int detection_parameters_index_ {0};	// spike detection array index
	int value_max_ {4096};				// spike max (used to scan rapidly to adjust display)
	int value_min_ {0};					// spike min (used to scan rapidly to adjust display)
	int d_max_min_ {0};
	int offset_ {2048};					// offset voltage pt 1
	int y1_ {0};						// parameter measured and stored
	int y2_ {0};
	int dt_ {0};

	int* buffer_spike_data_ {nullptr};	// buffer address
	int buffer_spike_length_ {0};		// n points in the buffer
	int spike_length_ {0};				// length of buffer
	boolean selected_state_ {false};
	int bin_zero_ {2048};				// zero (2048 if 12 bits scale = 0-4095)

public:
	int		get_bin_zero() const { return bin_zero_; }
	void	set_bin_zero(const short bin_zero) { bin_zero_ = bin_zero; }

	boolean get_selected() const { return selected_state_; }
	void	set_selected(const boolean status) { selected_state_ = status; }

	long	get_time() const { return ii_time_; }
	int		get_class_id() const { return class_id_; }
	int		get_source_channel() const { return detection_parameters_index_; }
	int		get_maximum() const { return value_max_; }
	int		get_minimum() const { return value_min_; }
	int		get_amplitude_offset() const { return offset_; }

	int		get_spike_length() const { return spike_length_; }
	void	set_spike_length(const int spike_length) { spike_length_ = spike_length; }
	int*	get_p_data(int spike_length);
	int*	get_p_data() const;
	int		get_value_at_offset(const int index) const {return *(buffer_spike_data_+index);}

	int		get_y1() const { return y1_; }
	void	set_y1(const int y) { y1_ = y; }

	int		get_y2() const { return y2_; }
	void	set_y2(const int y) { y2_ = y; }

	int		get_dt() const { return dt_; }
	void	set_dt(const long x) { dt_ = x; }

	void	set_time(const long ii) { ii_time_ = ii; }
	void	set_class_id(const int cl) { class_id_ = cl; }

	void	get_max_min(int* max, int* min) const;
	void	get_max_min_ex(int* max, int* min, int* d_max_to_min) const;
	void	set_max_min_ex(const int max, const int min, const int d_max_min) {
				value_max_ = max;
				value_min_ = min;
				d_max_min_ = d_max_min;
	}

	void	set_amplitude_offset(const int offset) { offset_ = offset; }

	void	transfer_data_to_spike_buffer(short* source_data, const int source_n_channels, const int spike_length);
	void	measure_amplitude_min_to_max(int t1, int t2);
	void	measure_max_min_ex(int* value_max, int* index_max, int* value_min, int* index_min, int t1, int t2) const;
	void	measure_max_then_min_ex(int* value_max, int* index_max, int* value_min, int* index_min, int t1, int t2) const;
	long	measure_sum_ex(int i_first, int i_last) const;

	void	offset_spike_data(int offset);
	void	offset_spike_data_to_average_ex(int i_first, int i_last);
	void	offset_spike_data_to_extrema_ex(int i_first, int i_last);
	void	center_spike_amplitude(int i_first, int i_last, WORD method = 0);

	// Implementation
	void	Serialize(CArchive& ar) override;
	void	read_version0(CArchive& ar);

protected:
	void	read_version2(CArchive& ar, WORD w_version);
};
