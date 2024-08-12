#pragma once


class SpikeClass : public CObject
{
	DECLARE_SERIAL(SpikeClass)

protected:
	int m_n_classes_ = 0;					// Number of different classes of spikes
	int m_spike_size_ = 0;					// size of spike
	long m_buffer_size_ = 0;				// GetRectSize of the buffer
	short* m_class_buffer_ = nullptr;		// pointer to the buffer
	int m_e_array_size_ = 0;
	int* m_elements_array_ = nullptr;		// array of class element
	BOOL size_n_classes(int n_classes, int spike_size);

public:
	void erase_data();

	SpikeClass();
	SpikeClass(int spike_size);
	~SpikeClass() override;
	void Serialize(CArchive& ar) override;
};
