#pragma once

#include "Tag.h"

class TagList : public CObject
{
protected:
	CArray<Tag*, Tag*> tag_ptr_array;
	int m_version;
	int insertTag(Tag* pcur);

	// Construction, Destruction
public:
	TagList();
	~TagList() override;

	int AddTag(int val, int refchan); // Add a new tag, with  value and attached channel
	int AddLTag(long lval, int refchan); // Add a new tag, with  value and attached channel
	int AddTag(Tag& arg); // add a tag
	void CopyTagList(TagList* pTList); // copy list into current object; delete old objects

	int RemoveTag(int itag); // remove tag
	void RemoveAllTags(); // remove all tags
	int RemoveChanTags(int refchan); // remove all tags associated with a channel

	void SetTagChan(int itag, int newchan); // get reference channel
	void SetTagVal(int itag, int newval); // change tag value
	void SetTagPix(int itag, int newval); // change tag pixel value
	void SetTagLVal(int itag, long longval); // change tag Lvalue
	void SetTagComment(int itag, CString comment); // change tag comment

	int GetChannel(int itag); // get reference channel
	int GetValue(int itag); // get tag value
	int GetTagPix(int itag); // get tag pixel value
	long GetTagLVal(int itag); // get tag value
	CString GetTagComment(int itag); // get comment

	int GetNTags();
	Tag* GetTag(int itag);

	long Write(CFile* pdatafile);
	BOOL Read(CFile* pdatafile);
};
