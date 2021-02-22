
#pragma once


//////////////////////////////////////////////////////////////////////
// Class  CWaveChanArray                                            //
// table of CWaveChan                                               //
//////////////////////////////////////////////////////////////////////
class CWaveChanArray : public CObject
{
	DECLARE_SERIAL(CWaveChanArray);
protected:
	CArray <CWaveChan*, CWaveChan*>	wavechan_ptr_array;		// array of channels

public:
	CWaveChanArray();
	~CWaveChanArray();
	long write(CFile* datafile);
	BOOL read(CFile* datafile);

	CWaveChanArray& operator = (const CWaveChanArray& arg);
	CWaveChan* get_p_channel(int i) const;

	int		chanArray_add();
	int		chanArray_add(CWaveChan* arg);
	int		chanArray_setSize(int i);
	void	chanArray_insertAt(int i);
	void	chanArray_removeAt(int i);
	void	chanArray_removeAll();
	int		chanArray_getSize() const;
	void	Serialize(CArchive& ar) override;
};
