#pragma once
class SpikeClassDescriptor : public CObject
{
	DECLARE_SERIAL(SpikeClassDescriptor)

	int id_number = 0;
	int n_items = 0;

	SpikeClassDescriptor();                                     // simple constructor
    ~SpikeClassDescriptor() override;                           // destructor
    SpikeClassDescriptor(int number, int items);
    SpikeClassDescriptor(const SpikeClassDescriptor& other);    // copy constructor
	void Serialize(CArchive& ar) override;
	SpikeClassDescriptor& operator=(const SpikeClassDescriptor& arg); // copy assignment operator

};

