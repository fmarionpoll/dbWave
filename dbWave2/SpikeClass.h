#pragma once


class SpikeClass : public CObject
{
	DECLARE_SERIAL(SpikeClass)

protected:
	int m_n_classes = 0;					// Number of different classes of spikes
	int m_spike_size = 0;					// size of spike
	long m_buffer_size = 0;					// GetRectSize of the buffer
	short* m_class_buffer = nullptr;		// pointer to the buffer
	int m_EArraySize = 0;
	int* m_elements_array = nullptr;		// array of class elemt
	BOOL SizeNclasses(int n_classes, int spike_size);

public:
	void EraseData();

	SpikeClass();
	SpikeClass(int SpikeSize);
	~SpikeClass() override;
	void Serialize(CArchive& ar) override;
};
