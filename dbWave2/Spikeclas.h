#pragma once


class SpikeClass : public CObject
{
	DECLARE_SERIAL(SpikeClass)

protected:
	int m_NClass = -1; // Number of different classes of spikes
	int m_SpikeSize = 0; // size of spike
	long m_BufferSize = 0; // GetRectSize of the buffer
	short* m_pRWBuffer = nullptr; // pointer to the buffer
	int m_EArraySize = 0;
	int* m_pEArray = nullptr; // array of class elemt
	BOOL SizeNclasses(int nclasses, int spikesize);

public:
	void EraseData();

	SpikeClass();
	SpikeClass(int SpikeSize);
	~SpikeClass() override;
	void Serialize(CArchive& ar) override;
};
