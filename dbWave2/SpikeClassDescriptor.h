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

	int get_class_id() const {return class_id; }
	void set_class_id(const int id) { class_id = id; }
	int get_n_items() const {return n_items; }
	void set_n_items(const int n) { n_items = n; }
	int increment_n_items() { n_items++; return n_items; }
	int decrement_n_items() { if (n_items > 0)  n_items--; return n_items; }

};

