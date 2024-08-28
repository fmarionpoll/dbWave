#pragma once
#include "options_detect_spikes.h"

class SpikeDetectArray : public CObject
{
	DECLARE_SERIAL(SpikeDetectArray)

	BOOL b_changed {FALSE}; // set TRUE if contents has changed

	int add_item(); // add one parameter array item
	int remove_item(int i);
	options_detect_spikes* get_item(const int i) { return spk_detect_options_array_.GetAt(i); }
	void set_item(const int i, const options_detect_spikes* p_sd) { *(spk_detect_options_array_[i]) = *p_sd; }
	int get_size() const { return spk_detect_options_array_.GetSize(); }
	void set_size(int n_items);

protected:
	WORD w_version_number_ {4};
	void delete_array();
	CArray<options_detect_spikes*, options_detect_spikes*> spk_detect_options_array_; 

public:
	SpikeDetectArray();
	~SpikeDetectArray() override;
	SpikeDetectArray& operator =(const SpikeDetectArray& arg);
	void Serialize(CArchive& ar) override;

	void load(CArchive& ar, WORD w_version);
};

