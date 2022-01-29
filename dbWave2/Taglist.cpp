#include "StdAfx.h"
#include "Taglist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Class CTagList implementation

CTagList::CTagList()
{
	m_version = 1;
}

CTagList::~CTagList()
{
	RemoveAllTags();
}

int CTagList::insertTag(CTag* pcur)
{
	return tag_ptr_array.Add(pcur);
}

int  CTagList::AddTag(CTag& arg)
{
	const auto pcur = new CTag;
	ASSERT(pcur != NULL);
	*pcur = arg;
	return insertTag(pcur);
}

int CTagList::AddTag(int val, int refChannel)
{
	const auto pcur = new CTag(val, refChannel);
	ASSERT(pcur != NULL);
	return insertTag(pcur);
}

// Add a new cursor, with  value and attached channel
int CTagList::AddLTag(long lval, int refchan)
{
	const auto pcur = new CTag(lval, refchan);
	ASSERT(pcur != NULL);
	return insertTag(pcur);
}

int CTagList::RemoveTag(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	delete pcur;				// delete object pointed at
	tag_ptr_array.RemoveAt(itag);
	return tag_ptr_array.GetSize();
}

void CTagList::RemoveAllTags()
{
	const auto pos0 = tag_ptr_array.GetUpperBound();
	if (pos0 >= 0)
	{
		for (auto pos = pos0; pos >= 0; pos--)
			delete tag_ptr_array.GetAt(pos);
		tag_ptr_array.RemoveAll();
	}
}

int CTagList::RemoveChanTags(int refchan)
{
	for (auto i = tag_ptr_array.GetUpperBound(); i >= 0; i--)
	{
		const auto pcur = tag_ptr_array.GetAt(i);
		if (pcur != nullptr && pcur->m_refchan == refchan)
		{
			delete pcur;				// delete object pointed at
			tag_ptr_array.RemoveAt(i);		// remove item
		}
	}
	return tag_ptr_array.GetSize();
}

void CTagList::SetTagVal(int itag, int newval)
{
	if (tag_ptr_array.GetSize() <= itag)
	{
		for (auto i = tag_ptr_array.GetSize(); i <= itag; i++)
			AddTag(0, 0);
		ASSERT(tag_ptr_array.GetSize() >= itag);
	}
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_value = newval;
}

int CTagList::GetValue(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_value;

	return NULL;
}

int CTagList::GetChannel(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_refchan;

	return NULL;
}

void CTagList::SetTagChan(int itag, int newchan)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_refchan = newchan;
}

void CTagList::SetTagPix(int itag, int newval)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_pixel = newval;
}

int CTagList::GetTagPix(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_pixel;
	return NULL;
}

void CTagList::SetTagLVal(int itag, long longval)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	// if the cursor exist change the m_value
	if (pcur != nullptr)
		pcur->m_lvalue = longval;
}

long CTagList::GetTagLVal(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_lvalue;
	return NULL;
}

void CTagList::SetTagComment(int itag, CString comment)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_csComment = comment;
}

CString CTagList::GetTagComment(int itag)
{
	const auto pcur = tag_ptr_array.GetAt(itag);
	CString cs;
	if (pcur != nullptr)
		cs = pcur->m_csComment;
	return cs;
}

int CTagList::GetNTags()
{
	return tag_ptr_array.GetSize();
}

CTag* CTagList::GetTag(int itag)
{
	if (itag >= 0 && itag < tag_ptr_array.GetSize())
		return tag_ptr_array.GetAt(itag);
	return nullptr;
}

void CTagList::CopyTagList(CTagList* pTList)
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
			const auto pcur = new CTag;
			ASSERT(pcur != NULL);
			pcur->m_refchan = ptag->m_refchan;
			pcur->m_pixel = ptag->m_pixel;
			pcur->m_value = ptag->m_value;
			pcur->m_lvalue = ptag->m_lvalue;
			insertTag(pcur);
		}
	}
}

long CTagList::Write(CFile* pdatafile)
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

BOOL CTagList::Read(CFile* pdatafile)
{
	int version;
	pdatafile->Read(&version, sizeof(int));
	int nelemts;
	pdatafile->Read(&nelemts, sizeof(int));

	for (auto i = 0; i < nelemts; i++)
	{
		auto ptag = new CTag;
		ASSERT(ptag != NULL);
		ptag->Read(pdatafile);
		tag_ptr_array.Add(ptag);
	}
	return TRUE;
}