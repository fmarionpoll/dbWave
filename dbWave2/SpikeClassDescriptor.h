#pragma once
class SpikeClassDescriptor : public CObject
{
	DECLARE_SERIAL(SpikeClassDescriptor)
protected:
	int class_id_  {0};
	int n_items_  {0};
	CString descriptor_{};

public:
	SpikeClassDescriptor(); 
    ~SpikeClassDescriptor() override;                         
    SpikeClassDescriptor(int number, int items, const CString& descriptor);
    SpikeClassDescriptor(const SpikeClassDescriptor& other);

	void Serialize(CArchive& ar) override;
	SpikeClassDescriptor& operator=(const SpikeClassDescriptor& arg);

	int get_class_id() const {return class_id_; }
	void set_class_id(const int id) { class_id_ = id; }

	int get_class_n_items() const {return n_items_; }
	void set_class_n_items(const int n) { n_items_ = n; }

	CString get_class_descriptor()  { return descriptor_.IsEmpty() ? _T(" "): descriptor_; }
	void set_class_descriptor(const CString& desc) { descriptor_ = desc; }

	int increment_n_items() { n_items_++; return n_items_; }
	int decrement_n_items() { if (n_items_ > 0)  n_items_--; return n_items_; }
};

