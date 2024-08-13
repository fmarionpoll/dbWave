#pragma once
#include "options_detect_spikes.h"

class SpikeDetectArray : public CObject
{
	DECLARE_SERIAL(SpikeDetectArray)

	BOOL bChanged = FALSE; // flag set TRUE if contents has changed

	int AddItem(); // add one parameter array item
	int RemoveItem(int i);
	options_detect_spikes* GetItem(int i) { return spkdetectparm_ptr_array.GetAt(i); }
	void SetItem(int i, options_detect_spikes* pSD) { *(spkdetectparm_ptr_array[i]) = *pSD; }
	int GetSize() { return spkdetectparm_ptr_array.GetSize(); }
	void SetSize(int nitems);

protected:
	WORD w_version_number = 4;
	void DeleteArray();
	CArray<options_detect_spikes*, options_detect_spikes*> spkdetectparm_ptr_array; 

public:
	SpikeDetectArray();
	~SpikeDetectArray() override;
	SpikeDetectArray& operator =(const SpikeDetectArray& arg);
	void Serialize(CArchive& ar) override;

	void Serialize_Load(CArchive& ar, WORD wversion);
};

