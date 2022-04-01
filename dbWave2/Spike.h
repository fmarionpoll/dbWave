#pragma once

// parameters associated to each spike detected 
// stores:  time of occurence, initial data acq chan and a modifiable parameter
// the class
// this basic object is part of a CSpikelist object that stores parameters
// concerning the source document (file, date, acq rate and parms, etc) and
// eventually the source data extracted from the data file
// real data are stored in a separate object managing data buffers

class Spike : public CObject
{
	DECLARE_SERIAL(Spike)

	Spike();
	Spike(long time, int channel);
	Spike(long time, int channel, int max, int min, int offset, int class_i, int d_maxmin);
	~Spike() override;

	// Attributes
private:
	long m_iitime = 0;		// occurence time - multiply by rate to get time in seconds
	int m_class = 0;		// spike class - init to zero at first
	int m_chanparm = 0;		// spike detection array index
	int m_max = 4096;		// spike max (used to scan rapidly to adjust display)
	int m_min = 0;			// spike min (used to scan rapidly to adjust display)
	int m_dmaxmin = 0;
	int m_offset = 2048;	// offset voltage pt 1
	int y1_ = 0;			// parameter measured and stored
	int y2_ = 0;
	int dt_ = 0;

	short* m_spike_data_buffer = nullptr;	// buffer address
	int m_spike_length = 0;					// length of buffer
	int m_current_filter{};					// ID of last transform


	// Operations set/change elements of spikeele
public:
	long get_time() const { return m_iitime; }
	int get_class() const { return m_class; }
	int get_source_channel() const { return m_chanparm; }
	short get_maximum() const { return m_max; }
	short get_minimum() const { return m_min; }
	int get_amplitude_offset() const { return m_offset; }

	int GetSpikeLength() const { return m_spike_length; }
	short* GetpSpikeData(int spike_length);
	short* GetpSpikeData() { return m_spike_data_buffer; }
	short GetSpikeValAt(int index) {return *(m_spike_data_buffer+index);}

	int m_spk_buffer_increment{};
	int m_spk_buffer_length{};				// current buffer length
	int m_next_index{};						// next available index
	int m_last_index{};						// index last free space for spike data
	int m_bin_zero = 2048;					// zero (if 12 bits scale = 0-4095)

	void GetSpikeMaxMin(int* max, int* min, int* d_max_to_min)
	{
		*max = m_max;
		*min = m_min;
		*d_max_to_min = m_dmaxmin;
	}

	void GetSpike(int* max, int* min)
	{
		*max = m_max;
		*min = m_min;
	}

	int get_y1() const { return y1_; }
	int get_y2() const { return y2_; }
	int get_dt() const { return dt_; }

	void set_time(long ii) { m_iitime = ii; }
	void set_class(int cl) { m_class = cl; }

	void SetSpikeMaxMin(int max, int min, int dmaxmin)
	{
		m_max = max;
		m_min = min;
		m_dmaxmin = dmaxmin;
	}

	void SetSpikeAmplitudeOffset(int offset) { m_offset = offset; }
	void set_y1(int y) { y1_ = y; }
	void set_y2(int y) { y2_ = y; }
	void set_dt(long x) { dt_ = x; }

	// Implementation
public:
	void read_version0(CArchive& ar);
	void Serialize(CArchive& ar) override;
	void read_version2(CArchive& ar, WORD wVersion);
};
