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
	long m_ii_time = 0;						// occurrence time - multiply by rate to get time in seconds
	int m_class = 0;						// spike class - init to zero at first
	int m_detection_parameters_index = 0;	// spike detection array index
	short m_value_max = 4096;				// spike max (used to scan rapidly to adjust display)
	short m_value_min = 0;					// spike min (used to scan rapidly to adjust display)
	int m_d_max_min = 0;
	int m_offset = 2048;					// offset voltage pt 1
	int y1_ = 0;							// parameter measured and stored
	int y2_ = 0;
	int dt_ = 0;

	short* m_spike_data_buffer = nullptr;	// buffer address
	int m_spike_length = 0;					// length of buffer
	int m_spk_buffer_length = 0;			// n points in the buffer
	boolean m_selected_state = false;
	int m_bin_zero = 2048;					// zero (if 12 bits scale = 0-4095)

public:
	int get_bin_zero() const { return m_bin_zero; }
	void set_bin_zero(int bin_zero) { m_bin_zero = bin_zero; }

	boolean get_selected() const { return m_selected_state; }
	void set_selected(boolean status) { m_selected_state = status; }

	long get_time() const { return m_ii_time; }
	int get_class() const { return m_class; }
	int get_source_channel() const { return m_detection_parameters_index; }
	int get_maximum() const { return m_value_max; }
	int get_minimum() const { return m_value_min; }
	int get_amplitude_offset() const { return m_offset; }

	int get_spike_length() const { return m_spike_length; }
	short* get_p_data(int spike_length);
	short* get_p_data() const;
	short get_value_at_offset(int index) const {return *(m_spike_data_buffer+index);}
	void get_max_min_ex(short* max, short* min, int* d_max_to_min) const;

	void get_max_min(short* max, short* min) const;

	int get_y1() const { return y1_; }
	int get_y2() const { return y2_; }
	int get_dt() const { return dt_; }

	void set_time(long ii) { m_ii_time = ii; }
	void set_class(int cl) { m_class = cl; }

	void SetMaxMinEx(int max, int min, int d_max_min)
	{
		m_value_max = max;
		m_value_min = min;
		m_d_max_min = d_max_min;
	}

	void SetAmplitudeOffset(int offset) { m_offset = offset; }
	void set_y1(int y) { y1_ = y; }
	void set_y2(int y) { y2_ = y; }
	void set_dt(long x) { dt_ = x; }

	void TransferDataToSpikeBuffer(short* source_data, int source_n_channels);

	void measure_max_min_ex(short* value_max, int* index_max, short* value_min, int* index_min, int i_first, int i_last) const;
	void MeasureMaxThenMinEx(short* value_max, int* index_max, short* value_min, int* index_min, int i_first, int i_last) const;
	long MeasureSumEx(int i_first, int i_last) const;

	void OffsetSpikeData(short offset);
	void OffsetSpikeDataToAverageEx(int i_first, int i_last);
	void OffsetSpikeDataToExtremaEx(int i_first, int i_last);
	void CenterSpikeAmplitude(int i_first, int i_last, WORD method = 0);

	// Implementation
public:
	void read_version0(CArchive& ar);
	void Serialize(CArchive& ar) override;

protected:
	void read_version2(CArchive& ar, WORD wVersion);
};
