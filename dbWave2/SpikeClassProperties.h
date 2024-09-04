#pragma once
class SpikeClassProperties : public CObject
{
	DECLARE_SERIAL(SpikeClassProperties)
protected:
	int class_id_  {0};
	int n_items_  {0};
	CString class_text_{ _T("class 0") };

public:
	static constexpr int nb_descriptors{ 7 };
	static CString class_descriptor[nb_descriptors];

	SpikeClassProperties(); 
    ~SpikeClassProperties() override;                         
    SpikeClassProperties(int number, int items, const CString& descriptor);
    SpikeClassProperties(const SpikeClassProperties& other);

	void Serialize(CArchive& ar) override;
	SpikeClassProperties& operator=(const SpikeClassProperties& arg);

	int get_class_id() const {return class_id_; }
	void set_class_id(const int id) { class_id_ = id; }

	int get_class_n_items() const {return n_items_; }
	void set_class_n_items(const int n) { n_items_ = n; }

	CString get_class_text()  { return class_text_; }
	void set_class_text(const CString& desc) { class_text_ = desc; }

	int increment_n_items() { n_items_++; return n_items_; }
	int decrement_n_items() { if (n_items_ > 0)  n_items_--; return n_items_; }

	static CString get_class_default_descriptor_string(int class_id);
};

