
#include "StdAfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTag : base element of CTagList

IMPLEMENT_SERIAL(CTag, CObject, 0 /* schema number*/ )

CTag::CTag()
{
	m_refchan=-1;
	m_value=0;
	m_lvalue=-1;
	m_pixel = -1;
}

CTag::CTag(int ref_chan): m_lvalue(0)
{
	m_refchan = ref_chan;
	m_value = 0;
	m_pixel = -1;
}

CTag::CTag(int val,int ref_chan)
{
	m_refchan=ref_chan;
	m_value=val;
	m_lvalue=-1;
	m_pixel = -1;
}

CTag::CTag(long lval,int ref_chan)
{
	m_refchan=ref_chan;	
	m_value=-1;
	m_lvalue=lval;
	m_pixel = -1;	
}

CTag::CTag(const CTag &hc)
{
	m_refchan=hc.m_refchan;
	m_value=hc.m_value;
	m_pixel = hc.m_pixel;
	m_lvalue = hc.m_lvalue;
	m_csComment = hc.m_csComment;
}

CTag::~CTag()
{
}

CTag& CTag::operator = (const CTag& arg)
{
	if (this != &arg) {
		m_refchan = arg.m_refchan;
		m_pixel = arg.m_pixel;
		m_value = arg.m_value;
		m_lvalue = arg.m_lvalue;
		m_csComment = arg.m_csComment;
	}
	return *this;
}

void CTag::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{   
		ar << static_cast<WORD>(m_refchan);
		ar << static_cast<WORD>(m_pixel);
		ar << static_cast<WORD>(m_value);
		ar << m_lvalue;
		ar << m_csComment;
	} 
	else
	{
		WORD w1;
		ar >> w1; m_refchan=w1;
		ar >> w1; m_pixel=w1;
		ar >> w1; m_value=w1;
		ar >> m_lvalue;
		ar >> m_csComment;
	}    	
}

long CTag::Write(CFile* datafile)
{
	const auto p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	const auto p2 = datafile->GetPosition();
	return static_cast<long>(p2 - p1);
}

BOOL CTag::Read(CFile* datafile)
{
	CArchive ar(datafile, CArchive::load);
	Serialize(ar);
	ar.Close();
	return TRUE;
}

// Class CTagList implementation

CTagList::CTagList()
{
	m_version=1;	
}

CTagList::~CTagList() 
{
	RemoveAllTags();
}

int CTagList::InsertTag(CTag* pcur)
{
	return tag_ptr_array.Add(pcur);
}

int  CTagList::AddTag(CTag& arg)
{
	const auto pcur = new CTag;
	ASSERT(pcur != NULL);
	*pcur = arg;
	return InsertTag(pcur);
}

int CTagList::AddTag(int val,int refChannel)
{
	const auto pcur= new CTag(val,refChannel);
	ASSERT(pcur != NULL);
	return InsertTag(pcur);
}

// Add a new cursor, with  value and attached channel
int CTagList::AddLTag(long lval, int refchan)
{
	const auto pcur= new CTag(lval,refchan);
	ASSERT(pcur != NULL);
	return InsertTag(pcur);
}

int CTagList::RemoveTag(int itag)
{   
	const auto pcur= tag_ptr_array.GetAt(itag);
	delete pcur;				// delete object pointed at
	tag_ptr_array.RemoveAt(itag);
	return tag_ptr_array.GetSize();
}

void CTagList::RemoveAllTags()
{   
	const auto pos0=tag_ptr_array.GetUpperBound();
	if (pos0 >=0)
	{
		for (auto pos=pos0; pos>= 0; pos--)
			delete tag_ptr_array.GetAt(pos);
		tag_ptr_array.RemoveAll();
	}
}

int CTagList::RemoveChanTags(int refchan)
{
	for (auto i = tag_ptr_array.GetUpperBound(); i>= 0; i--)
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
		for (auto i=tag_ptr_array.GetSize(); i <= itag; i++)
			AddTag(0, 0);
		ASSERT(tag_ptr_array.GetSize() >= itag);
	}
	const auto pcur= tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_value=newval;
}

int CTagList::GetTagVal(int itag)
{   
	const auto pcur= tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_value;

	return NULL;
}

int CTagList::GetTagChan(int itag)
{   
	const auto pcur= tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_refchan;

	return NULL;
}

void CTagList::SetTagChan(int itag, int newchan)
{   
	const auto pcur= tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_refchan=newchan;
}

void CTagList::SetTagPix(int itag, int newval)
{   
	const auto pcur= tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_pixel=newval;
}

int CTagList::GetTagPix(int itag)
{   
	const auto pcur= tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_pixel;
	return NULL;
}

void CTagList::SetTagLVal(int itag, long longval)
{
	const auto pcur= tag_ptr_array.GetAt(itag);
	// if the cursor exist change the m_value
	if (pcur != nullptr)
		pcur->m_lvalue=longval;
}

long CTagList::GetTagLVal(int itag)
{
	const auto pcur= tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_lvalue;
	return NULL;
}

void CTagList::SetTagComment(int itag, CString comment)
{
	const auto pcur= tag_ptr_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_csComment=comment;
}

CString CTagList::GetTagComment(int itag)
{
	const auto pcur= tag_ptr_array.GetAt(itag);
	CString cs;
	if (pcur != nullptr)
		cs= pcur->m_csComment;
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
	for (auto i=0; i<nbtags; i++)
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
			InsertTag(pcur);
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

	for (auto i = 0; i < nelemts; i ++)
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
	
	for (auto i=0; i<nelemts; i++)	
	{
		auto ptag = new CTag;
		ASSERT(ptag != NULL);
		ptag->Read(pdatafile);
		tag_ptr_array.Add(ptag);
	}
	return TRUE;
}