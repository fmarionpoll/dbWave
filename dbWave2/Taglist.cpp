#include "StdAfx.h"
#include "Taglist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


TagList::TagList()
{
	m_version = 1;
}

TagList::~TagList()
{
	RemoveAllTags();
}

int TagList::insertTag(Tag* pcur)
{
	return tag_ptr_array.Add(pcur);
}

int TagList::AddTag(Tag& arg)
{
	const auto pcur = new Tag;
	ASSERT(pcur != NULL);
	*pcur = arg;
	return insertTag(pcur);
}

int TagList::AddTag(int val, int refChannel)
{
	const auto pcur = new Tag(val, refChannel);
	ASSERT(pcur != NULL);
	return insertTag(pcur);
}

// Add a new cursor, with  value and attached channel
int TagList::AddLTag(long lval, int refchan)
{
	const auto pcur = new Tag(lval, refchan);
	ASSERT(pcur != NULL);
	return insertTag(pcur);
}

int TagList::RemoveTag(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	delete pcur; // delete object pointed at
	tag_ptr_array.RemoveAt(itag);
	return tag_ptr_array.GetSize();
}

void TagList::RemoveAllTags()
{
	const auto pos0 = tag_ptr_array.GetUpperBound();
	if (pos0 >= 0)
	{
		for (auto pos = pos0; pos >= 0; pos--)
			delete tag_ptr_array.GetAt(pos);
		tag_ptr_array.RemoveAll();
	}
}

int TagList::RemoveChanTags(int refchan)
{
	for (auto i = tag_ptr_array.GetUpperBound(); i >= 0; i--)
	{
		const auto pcur = tag_ptr_array.GetAt(i);
		if (pcur != nullptr && pcur->m_refchan == refchan)
		{
			delete pcur; // delete object pointed at
			tag_ptr_array.RemoveAt(i); // remove item
		}
	}
	return tag_ptr_array.GetSize();
}

void TagList::SetTagVal(int itag, int newval)
{
	if (tag_ptr_array.GetSize() <= itag)
	{
		for (auto i = tag_ptr_array.GetSize(); i <= itag; i++)
			AddTag(0, 0);
		ASSERT(tag_ptr_array.GetSize() >= itag);
	}
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr) // if the cursor exist change the m_value
		pcur->m_value = newval;
}

int TagList::GetValue(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_value;

	return NULL;
}

int TagList::GetChannel(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_refchan;

	return NULL;
}

void TagList::SetTagChan(int itag, int newchan)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr) // if the cursor exist change the m_value
		pcur->m_refchan = newchan;
}

void TagList::SetTagPix(int itag, int newval)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr) // if the cursor exist change the m_value
		pcur->m_pixel = newval;
}

int TagList::GetTagPix(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_pixel;
	return NULL;
}

void TagList::SetTagLVal(int itag, long longval)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	// if the cursor exist change the m_value
	if (pcur != nullptr)
		pcur->m_lvalue = longval;
}

long TagList::GetTagLVal(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_lvalue;
	return NULL;
}

void TagList::SetTagComment(int itag, CString comment)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr) // if the cursor exist change the m_value
		pcur->m_csComment = comment;
}

CString TagList::GetTagComment(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	CString cs;
	if (pcur != nullptr)
		cs = pcur->m_csComment;
	return cs;
}

int TagList::GetNTags()
{
	return tag_ptr_array.GetSize();
}

Tag* TagList::GetTag(int itag)
{
	if (itag >= 0 && itag < tag_ptr_array.GetSize())
		return tag_ptr_array.GetAt(itag);
	return nullptr;
}

void TagList::CopyTagList(TagList* pTList)
{
	if (pTList == this || pTList == nullptr)
		return;

	//CPtrArray 	m_array;                              // array of cursors
	RemoveAllTags();
	const auto nbtags = pTList->tag_ptr_array.GetSize();
	for (auto i = 0; i < nbtags; i++)
	{
		const auto ptag = pTList->GetTag(i);
		if (ptag != nullptr)
		{
			const auto pcur = new Tag;
			ASSERT(pcur != NULL);
			pcur->m_refchan = ptag->m_refchan;
			pcur->m_pixel = ptag->m_pixel;
			pcur->m_value = ptag->m_value;
			pcur->m_lvalue = ptag->m_lvalue;
			insertTag(pcur);
		}
	}
}

long TagList::Write(CFile* pdatafile)
{
	long l_size = sizeof(int);
	pdatafile->Write(&m_version, l_size);
	auto nelemts = tag_ptr_array.GetSize();
	pdatafile->Write(&nelemts, l_size);
	l_size += l_size;

	for (auto i = 0; i < nelemts; i++)
	{
		auto ptag = tag_ptr_array.GetAt(i);
		l_size += ptag->Write(pdatafile);
	}
	return l_size;
}

BOOL TagList::Read(CFile* pdatafile)
{
	int version;
	pdatafile->Read(&version, sizeof(int));
	int nelemts;
	pdatafile->Read(&nelemts, sizeof(int));

	for (auto i = 0; i < nelemts; i++)
	{
		auto ptag = new Tag;
		ASSERT(ptag != NULL);
		ptag->Read(pdatafile);
		tag_ptr_array.Add(ptag);
	}
	return TRUE;
}
