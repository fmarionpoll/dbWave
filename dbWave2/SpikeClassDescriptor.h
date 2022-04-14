#pragma once
class SpikeClassDescriptor : public CObject
{
	DECLARE_SERIAL(SpikeClassDescriptor)
protected:
	int class_id = 0;
	int n_items = 0;

public:
	SpikeClassDescriptor(); 
    ~SpikeClassDescriptor() override;                         
    SpikeClassDescriptor(int number, int items);
    SpikeClassDescriptor(const SpikeClassDescriptor& other);    
	void Serialize(CArchive& ar) override;
	SpikeClassDescriptor& operator=(const SpikeClassDescriptor& arg);

	int GetClassID() const {return class_id; }
	void SetClassID(int id) { class_id = id; }
	int GetNItems() const {return n_items; }
	void SetNItems(int n) { n_items = n; }

};

