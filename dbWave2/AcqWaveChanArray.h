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
	CWaveChan* Get_p_channel(int i) const;

	int ChanArray_add();
	int ChanArray_add(CWaveChan* arg);
	int ChanArray_setSize(int i);
	void ChanArray_insertAt(int i);
	void ChanArray_removeAt(int i);
	void ChanArray_removeAll();
	int ChanArray_getSize() const;
	void Serialize(CArchive& ar) override;
};
