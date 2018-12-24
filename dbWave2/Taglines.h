#pragma once

////////////////////////////////////////////////////////////////////////////////
// 	Taglines.h
//  CTag: Specification file
//
//

class CTag : public CObject
{
	DECLARE_SERIAL(CTag)
// Construction Destruction
public:
	CTag();
	CTag(int refchan);
	CTag(int val, int ref_chan);
	CTag(long lval, int ref_chan);
	CTag(const CTag &hc);					// Constructeur par copie
	~CTag();

	CTag& operator = (const CTag& arg);		// operator redefinition
	//CTag &operator = (CTag &arg);			// 
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
	long Write(CFile* pdatafile);
	BOOL Read(CFile* pdatafile);
	
// Management procedures
public:
	int 	m_refchan;						// channel associated (-1) if none
	int 	m_pixel;
	int 	m_value;						// line val
	long	m_lvalue;						// assoc long val (for vertical bar)	
	CString m_csComment;					// comment
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTagList

class CTagList : public CObject
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Channel tags operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
protected:
	CArray <CTag*, CTag*> tag_ptr_array;                    // array of tags
	int			m_version;
	int			InsertTag(CTag* pcur);

// Construction, Destruction
public :     
	CTagList();
	virtual ~CTagList();

	int		AddTag(int val, int refchan);		// Add a new tag, with  value and attached channel
	int		AddLTag(long lval, int refchan);	// Add a new tag, with  value and attached channel   	
	int		AddTag(CTag& arg);					// add a tag
	void	CopyTagList(CTagList* pTList);		// copy list into current object; delete old objects

	int		RemoveTag(int itag);				// remove tag
	void	RemoveAllTags();					// remove all tags
	int		RemoveChanTags(int refchan);		// remove all tags associated with a channel	

	void	SetTagChan(int itag, int newchan);	// get reference channel	
	void	SetTagVal(int itag, int newval);	// change tag value
	void	SetTagPix(int itag, int newval);	// change tag pixel value
	void	SetTagLVal(int itag, long longval);// change tag Lvalue	
	void	SetTagComment(int itag, CString comment); // change tag comment

	int		GetTagChan(int itag);				// get reference channel	
	int		GetTagVal(int itag);				// get tag value
	int		GetTagPix(int itag);				// get tag pixel value
	long	GetTagLVal(int itag);				// get tag value
	CString GetTagComment(int itag);		// get comment
	
	int 	GetNTags();
	CTag* 	GetTag(int itag);
	
	long	Write(CFile* pdatafile);
	BOOL	Read(CFile* pdatafile);
};
