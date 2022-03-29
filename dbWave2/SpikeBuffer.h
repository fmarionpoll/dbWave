#pragma once

// stores data collected from data acquisition files

class CSpikeBuffer : public CObject
{
	DECLARE_SERIAL(CSpikeBuffer)
public:
	CSpikeBuffer();
	CSpikeBuffer(int lenspik);
	~CSpikeBuffer() override;

	// Attributes
protected:
	int m_spike_length = 60;					// length of one spike
public:
	int m_spk_buffer_increment{};
	int m_spk_buffer_length{};				// current buffer length
	int m_next_index{};						// next available index
	int m_last_index{};						// index last free space for spike data
	int m_bin_zero = 2048;					// zero (if 12 bits scale = 0-4095)
	int m_current_filter{};					// ID of last transform
	short* m_spike_data_buffer = nullptr;	// buffer address
	CArray<int, int> m_spike_data_positions;// offsets to individual spike data - used to exchange, add, remove spikes without modifying buffer

	void SetSpikeLength(int spike_length);
	int GetSpikeLength() const { return m_spike_length; }

	// Implementation
public:
	void Serialize(CArchive& ar) override;
	void DeleteAllSpikes();
	short* AllocateSpaceForSpikeAt(int spike_index);
	short* AllocateSpaceForSeveralSpikes(int spike_index);
	BOOL DeleteSpike(int spike_index);
	BOOL ExchangeSpikes(int spk1, int spk2);
	short* GetSpike(int spike_index) const { return (m_spike_data_buffer + m_spike_data_positions[spike_index]); }
};
