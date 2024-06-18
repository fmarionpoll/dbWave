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
	chan_array_remove_all();
}

long CWaveChanArray::Write(CFile* datafile)
{
	const auto p1 = datafile->GetPosition();
	const int array_size = wavechan_ptr_array.GetSize();
	datafile->Write(&array_size, sizeof(short));
	for (auto i = 0; i < array_size; i++)
	{
		const auto p_channel = wavechan_ptr_array[i];
		p_channel->Write(datafile);
	}

	const auto p2 = datafile->GetPosition();
	return static_cast<long>(p2 - p1);
}

BOOL CWaveChanArray::Read(CFile* datafile)
{
	short array_size;
	datafile->Read(&array_size, sizeof(short));
	CWaveChan* p_channel;
	auto n = 0;

	// if size = 0, create dummy & empty channel
	if (array_size == 0)
	{
		chan_array_remove_all(); // erase existing data
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
			p_channel = get_p_channel(n);
			ASSERT(p_channel != NULL);
			if (!p_channel->Read(datafile))
				return FALSE;
			n++;
		}
		while (n < array_size);
	}
	else
	{
		chan_array_remove_all(); 
		do
		{
			p_channel = new CWaveChan;
			ASSERT(p_channel != NULL);
			if (!p_channel->Read(datafile))
				return FALSE;
			wavechan_ptr_array.Add(p_channel);
			n++;
		}
		while (n < array_size);
	}
	return TRUE;
}

void CWaveChanArray::Copy(const CWaveChanArray* arg)
{
	const auto n_items = arg->wavechan_ptr_array.GetSize(); 
	chan_array_remove_all(); 
	for (auto i = 0; i < n_items; i++) 
	{
		const auto p_channel = new CWaveChan();
		ASSERT(p_channel != NULL);
		p_channel ->Copy( arg->get_p_channel(i));
		chan_array_add(p_channel);
	}
}

CWaveChan* CWaveChanArray::get_p_channel(int i) const
{
	return wavechan_ptr_array.GetAt(i);
}

void CWaveChanArray::chan_array_remove_all()
{
	for (auto i = 0; i < wavechan_ptr_array.GetSize(); i++)
	{
		const auto p = wavechan_ptr_array[i];
		delete p;
	}
	wavechan_ptr_array.RemoveAll();
}

int CWaveChanArray::chan_array_add(CWaveChan* arg)
{
	return wavechan_ptr_array.Add(arg);
}

int CWaveChanArray::chan_array_add()
{
	const auto p = new CWaveChan;
	ASSERT(p != NULL);
	return chan_array_add(p);
}

void CWaveChanArray::chan_array_insert_at(const int i)
{
	const auto p = new CWaveChan;
	ASSERT(p != NULL);
	wavechan_ptr_array.InsertAt(i, p, 1);
}

void CWaveChanArray::chan_array_remove_at(const int i)
{
	const auto p = wavechan_ptr_array[i];
	delete p;
	wavechan_ptr_array.RemoveAt(i);
}

int CWaveChanArray::chan_array_get_size() const
{
	return wavechan_ptr_array.GetSize();
}

int CWaveChanArray::chan_array_set_size(const int i)
{
	if (i < wavechan_ptr_array.GetSize())
	{
		for (auto j = wavechan_ptr_array.GetUpperBound(); j >= i; j--)
			chan_array_remove_at(j);
	}
	else if (i > wavechan_ptr_array.GetSize())
		for (auto j = wavechan_ptr_array.GetSize(); j < i; j++)
			chan_array_add();
	return wavechan_ptr_array.GetSize();
}

void CWaveChanArray::Serialize(CArchive& ar)
{
	WORD version;
	if (ar.IsStoring())
	{
		version = 1;
		ar << version; 
		const int n_items = wavechan_ptr_array.GetSize();
		ar << n_items; 
		if (n_items > 0)
		{
			for (auto i = 0; i < n_items; i++)
			{
				auto* p_item = wavechan_ptr_array.GetAt(i);
				p_item->Serialize(ar);
			}
		}
	}
	else
	{
		ar >> version;
		int items;
		ar >> items;
		if (items > 0) 
		{
			chan_array_remove_all();
			for (auto i = 0; i < items; i++)
			{
				const auto p_item = new CWaveChan;
				ASSERT(p_item != NULL);
				p_item->Serialize(ar);
				wavechan_ptr_array.Add(p_item);
			}
		}
	}
}
