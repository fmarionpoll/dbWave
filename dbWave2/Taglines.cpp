///////////////////////////////////////////////////////////////////////
// CTag & CTagList : Implementation file
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////
// CTag : base element of CTagList

IMPLEMENT_SERIAL(CTag, CObject, 0 /* schema number*/ )

//---------------------------------------------------------------------
// Constructors

CTag::CTag()
{
	m_refchan=-1;
	m_value=0;
	m_lvalue=-1;
	m_pixel = -1;
}

CTag::CTag(int ref_chan)
{
	m_refchan=ref_chan;
	m_value=0;
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

//---------------------------------------------------------------------
// Destructor

CTag::~CTag()
{
}

void CTag::operator = (const CTag& arg)
{
	m_refchan = arg.m_refchan;
	m_pixel = arg.m_pixel;
	m_value = arg.m_value;
	m_lvalue = arg.m_lvalue;
	m_csComment = arg.m_csComment;
}

void CTag::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{   
		ar << (WORD) m_refchan;
		ar << (WORD) m_pixel;
		ar << (WORD) m_value;
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
	ULONGLONG p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	ULONGLONG p2 = datafile->GetPosition();
	return (long) (p2-p1);
}

BOOL CTag::Read(CFile* datafile)
{
	CArchive ar(datafile, CArchive::load);
	Serialize(ar);
	ar.Close();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Class CTagList implementation
//
CTagList::CTagList()
{
	m_version=1;	
}

CTagList::~CTagList() 
{
	RemoveAllTags();
}

//---------------------------------------------------------------------
int CTagList::InsertTag(CTag* pcur)
{
	return m_array.Add(pcur);
}

int  CTagList::AddTag(CTag& arg)
{
	CTag* pcur = new CTag;
	ASSERT(pcur != NULL);
	*pcur = arg;
	return InsertTag(pcur);
}

int CTagList::AddTag(int val,int refChannel)
{
	CTag* pcur= new CTag(val,refChannel);
	ASSERT(pcur != NULL);
	return InsertTag(pcur);
}

// Add a new cursor, with  value and attached channel
int CTagList::AddLTag(long lval, int refchan)
{
	CTag* pcur= new CTag(lval,refchan);
	ASSERT(pcur != NULL);
	return InsertTag(pcur);
}


//---------------------------------------------------------------------
int CTagList::RemoveTag(int itag)
{   
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	delete pcur;				// delete object pointed at
	m_array.RemoveAt(itag);
	return m_array.GetSize();
}

//---------------------------------------------------------------------
void CTagList::RemoveAllTags()
{   
	int pos=m_array.GetUpperBound();
	if (pos >=0)
	{
		for (int pos=m_array.GetUpperBound(); pos>= 0; pos--)
			delete (CTag*) m_array.GetAt(pos);
		m_array.RemoveAll();
	}
}

//---------------------------------------------------------------------
// remove all elements that are associated with one chan

int CTagList::RemoveChanTags(int refchan)
{
	CTag* pcur;
	for (int i = m_array.GetUpperBound(); i>= 0; i--)
	{
		pcur =  (CTag*)m_array.GetAt(i);
		if (pcur != nullptr && pcur->m_refchan == refchan)
		{
			delete pcur;				// delete object pointed at
			m_array.RemoveAt(i);		// remove item			
		}
	}
	return m_array.GetSize();
}

//---------------------------------------------------------------------

void CTagList::SetTagVal(int itag, int newval)
{   
	if (m_array.GetSize() <= itag)
	{
		for (int i=m_array.GetSize(); i <= itag; i++)
			AddTag(0, 0);
		ASSERT(m_array.GetSize() >= itag);
	}
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_value=newval;
}

int CTagList::GetTagVal(int itag)
{   
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_value;
	else
		return NULL;
}


//---------------------------------------------------------------------

int CTagList::GetTagChan(int itag)
{   
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_refchan;
	else
		return NULL;
}

void CTagList::SetTagChan(int itag, int newchan)
{   
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_refchan=newchan;
}

//---------------------------------------------------------------------

void CTagList::SetTagPix(int itag, int newval)
{   
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_pixel=newval;
}

int CTagList::GetTagPix(int itag)
{   
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_pixel;
	else
		return NULL;
}

//---------------------------------------------------------------------

void CTagList::SetTagLVal(int itag, long longval)
{
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	// if the cursor exist change the m_value
	if (pcur != nullptr)
		pcur->m_lvalue=longval;
}


long CTagList::GetTagLVal(int itag)
{
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	if (pcur != nullptr)
		return pcur->m_lvalue;
	else
		return NULL;
}

//---------------------------------------------------------------------

void CTagList::SetTagComment(int itag, CString comment)
{
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	if (pcur != nullptr)       // if the cursor exist change the m_value
		pcur->m_csComment=comment;
}


CString CTagList::GetTagComment(int itag)
{
	CTag* pcur= (CTag*)m_array.GetAt(itag);
	CString cs;
	if (pcur != nullptr)
		cs= pcur->m_csComment;
	return cs;
}

//---------------------------------------------------------------------
int CTagList::GetNTags()
{
	return m_array.GetSize();
}

//---------------------------------------------------------------------
CTag* CTagList::GetTag(int itag)
{
	if (itag >= 0 && itag < m_array.GetSize()) 
		return (CTag*) m_array.GetAt(itag);
	else
		return nullptr;
}

// copy list into current object; delete old objects
void CTagList::CopyTagList(CTagList* pTList)
{
	if (pTList == this || pTList == nullptr)
		return;

	//CPtrArray 	m_array;                              // array of cursors
	RemoveAllTags();	
	int nbtags = pTList->m_array.GetSize();
	for (int i=0; i<nbtags; i++)
	{
		CTag* ptag = pTList->GetTag(i);
		if (ptag != nullptr)
		{
			CTag* pcur = new CTag;
			ASSERT(pcur != NULL);
			pcur->m_refchan = ptag->m_refchan;
			pcur->m_pixel = ptag->m_pixel;
			pcur->m_value = ptag->m_value;
			pcur->m_lvalue = ptag->m_lvalue;
			InsertTag(pcur);
		}		
	}
	return;
}

long CTagList::Write(CFile* pdatafile)
{	
	long lSize = sizeof(int);
	pdatafile->Write(&m_version, lSize);
	int nelemts = m_array.GetSize();	
	pdatafile->Write(&nelemts, lSize);
	lSize += lSize;

	CTag* ptag;
	for (int i = 0; i < nelemts; i ++)
	{
		ptag = (CTag*) m_array.GetAt(i);
		lSize += ptag->Write(pdatafile);
	}
	return lSize;
}

BOOL CTagList::Read(CFile* pdatafile)
{
	int version;
	pdatafile->Read(&version, sizeof(int));
	int nelemts;
	pdatafile->Read(&nelemts, sizeof(int));	
	CTag* ptag;
	for (int i=0; i<nelemts; i++)	
	{
		ptag = new CTag;
		ASSERT(ptag != NULL);
		ptag->Read(pdatafile);
		m_array.Add(ptag);
	}
	return TRUE;
}