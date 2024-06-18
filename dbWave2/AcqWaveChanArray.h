#pragma once
#include "AcqWaveChan.h"


class CWaveChanArray : public CObject
{
	DECLARE_SERIAL(CWaveChanArray);
protected:
	CArray<CWaveChan*, CWaveChan*> wavechan_ptr_array; 

public:
	CWaveChanArray();
	~CWaveChanArray() override;
	long Write(CFile* datafile);
	BOOL Read(CFile* datafile);

	void Copy(const CWaveChanArray* arg);
	CWaveChan* get_p_channel(int i) const;

	int chan_array_add();
	int chan_array_add(CWaveChan* arg);
	int chan_array_set_size(int i);
	void chan_array_insert_at(int i);
	void chan_array_remove_at(int i);
	void chan_array_remove_all();
	int chan_array_get_size() const;
	void Serialize(CArchive& ar) override;
};
