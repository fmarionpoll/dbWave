// aqcparam.cpp    implementation file

#include "StdAfx.h"
#include "AcqWaveChanArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CWaveChanArray, CObject, 0 /* schema number*/)

CWaveChanArray::CWaveChanArray()
= default;

CWaveChanArray::~CWaveChanArray()
{
	ChanArray_removeAll();
}

long CWaveChanArray::Write(CFile * datafile)
{
	const auto p1 = datafile->GetPosition();
	short array_size = wavechan_ptr_array.GetSize();
	datafile->Write(&array_size, sizeof(short));
	for (auto i = 0; i < array_size; i++)
	{
		auto p_channel = wavechan_ptr_array[i];
		p_channel->Write(datafile);
	}

	const auto p2 = datafile->GetPosition();
	return static_cast<long>(p2 - p1);
}

BOOL CWaveChanArray::Read(CFile * datafile)
{
	short array_size;
	datafile->Read(&array_size, sizeof(short));
	CWaveChan* p_channel;
	auto n = 0;

	// if size = 0, create dummy & empty channel
	if (array_size == 0)
	{
		ChanArray_removeAll();					// erase existing data
		p_channel = new CWaveChan;
		ASSERT(p_channel != NULL);
		wavechan_ptr_array.Add(p_channel);
		return FALSE;
	}

	// same size, load data in the objects already created
	if (wavechan_ptr_array.GetSize() == array_size)
	{
		do
		{
			p_channel = Get_p_channel(n);
			ASSERT(p_channel != NULL);
			if (!p_channel->Read(datafile))
				return FALSE;
			n++;
		} while (n < array_size);
	}
	else
	{
		ChanArray_removeAll();					// erase existing data
		do
		{
			p_channel = new CWaveChan;
			ASSERT(p_channel != NULL);
			if (!p_channel->Read(datafile))
				return FALSE;
			wavechan_ptr_array.Add(p_channel);
			n++;
		} while (n < array_size);
	}
	return TRUE;
}

// this routine must duplicate the content of the array
// otherwise the pointers would point to the same objects and
// one of the array would be undefined when the other is destroyed

CWaveChanArray& CWaveChanArray::operator = (const CWaveChanArray & arg)
{
	if (this != &arg) {
		const auto n_items = arg.wavechan_ptr_array.GetSize();// source size
		ChanArray_removeAll();					// erase existing data
		for (auto i = 0; i < n_items; i++)	// loop over n items
		{
			const auto  p_channel = new CWaveChan(); // create new object
			ASSERT(p_channel != NULL);
			*p_channel = *arg.Get_p_channel(i);

			ChanArray_add(p_channel);		// store pointer into array
		}
	}
	return *this;
}

CWaveChan* CWaveChanArray::Get_p_channel(int i) const
{
	return wavechan_ptr_array.GetAt(i);
}

void CWaveChanArray::ChanArray_removeAll()
{
	for (auto i = 0; i < wavechan_ptr_array.GetSize(); i++)
	{
		const auto p = wavechan_ptr_array[i];
		delete p;
	}
	wavechan_ptr_array.RemoveAll();
}

int CWaveChanArray::ChanArray_add(CWaveChan * arg)
{
	return wavechan_ptr_array.Add(arg);
}

int CWaveChanArray::ChanArray_add()
{
	const auto p = new CWaveChan;
	ASSERT(p != NULL);
	return ChanArray_add(p);
}

void CWaveChanArray::ChanArray_insertAt(const int i)
{
	const auto p = new CWaveChan;
	ASSERT(p != NULL);
	wavechan_ptr_array.InsertAt(i, p, 1);
}

void CWaveChanArray::ChanArray_removeAt(const int i)
{
	const auto p = wavechan_ptr_array[i];
	delete p;
	wavechan_ptr_array.RemoveAt(i);
}

int CWaveChanArray::ChanArray_getSize() const
{
	return wavechan_ptr_array.GetSize();
}

int CWaveChanArray::ChanArray_setSize(const int i)
{
	if (i < wavechan_ptr_array.GetSize())
	{
		for (auto j = wavechan_ptr_array.GetUpperBound(); j >= i; j--)
			ChanArray_removeAt(j);
	}
	else if (i > wavechan_ptr_array.GetSize())
		for (auto j = wavechan_ptr_array.GetSize(); j < i; j++)
			ChanArray_add();
	return wavechan_ptr_array.GetSize();
}

void CWaveChanArray::Serialize(CArchive & ar)
{
	WORD version;
	if (ar.IsStoring())
	{
		version = 1;
		ar << version;				// save version number
		const int n_items = wavechan_ptr_array.GetSize();
		ar << n_items;				// save number of items
		if (n_items > 0)			// loop to save each CWaveChan
		{
			for (auto i = 0; i < n_items; i++)
			{
				auto* p_item = static_cast<CWaveChan*>(wavechan_ptr_array.GetAt(i));
				p_item->Serialize(ar);
			}
		}
	}
	else
	{
		ar >> version;				// get version
		int items; ar >> items;		// get number of items
		if (items > 0)				// loop to read all CWaveChan items
		{
			ChanArray_removeAll();
			for (auto i = 0; i < items; i++)
			{
				auto pItem = new CWaveChan;
				ASSERT(pItem != NULL);
				pItem->Serialize(ar);
				wavechan_ptr_array.Add(pItem);
			}
		}
	}
}