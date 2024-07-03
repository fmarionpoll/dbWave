#include "StdAfx.h"
#include "Taglist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


TagList::TagList()
{
	m_version_ = 1;
}

TagList::~TagList()
{
	remove_all_tags();
}

int TagList::insert_tag(Tag* tag)
{
	return tag_ptr_array_.Add(tag);
}

int TagList::add_tag(const Tag& arg)
{
	const auto tag = new Tag;
	ASSERT(tag != NULL);
	*tag = arg;
	return insert_tag(tag);
}

int TagList::add_tag(const int val, const int reference_channel)
{
	const auto tag = new Tag(val, reference_channel);
	ASSERT(tag != NULL);
	return insert_tag(tag);
}

// Add a new cursor, with  value and attached channel
int TagList::add_l_tag(const long l_val, const int reference_channel)
{
	const auto tag = new Tag(l_val, reference_channel);
	ASSERT(tag != NULL);
	return insert_tag(tag);
}

int TagList::remove_tag(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	delete tag; 
	tag_ptr_array_.RemoveAt(tag_index);
	return tag_ptr_array_.GetSize();
}

void TagList::remove_all_tags()
{
	const auto pos0 = tag_ptr_array_.GetUpperBound();
	if (pos0 >= 0)
	{
		for (auto pos = pos0; pos >= 0; pos--)
			delete tag_ptr_array_.GetAt(pos);
		tag_ptr_array_.RemoveAll();
	}
}

int TagList::remove_chan_tags(int reference_channel)
{
	for (auto i = tag_ptr_array_.GetUpperBound(); i >= 0; i--)
	{
		const auto tag = tag_ptr_array_.GetAt(i);
		if (tag != nullptr && tag->m_refchan == reference_channel)
		{
			delete tag; // delete object pointed at
			tag_ptr_array_.RemoveAt(i); // remove item
		}
	}
	return tag_ptr_array_.GetSize();
}

void TagList::set_tag_val(const int i_tag, const int value)
{
	if (tag_ptr_array_.GetSize() <= i_tag)
	{
		for (auto i = tag_ptr_array_.GetSize(); i <= i_tag; i++)
			add_tag(0, 0);
		ASSERT(tag_ptr_array_.GetSize() >= i_tag);
	}
	const auto p_cur = tag_ptr_array_.GetAt(i_tag);
	if (p_cur != nullptr) 
		p_cur->m_value = value;
}

int TagList::get_value(const int i_tag)
{
	const auto tag = tag_ptr_array_.GetAt(i_tag);
	if (tag != nullptr)
		return tag->m_value;
	return NULL;
}

int TagList::get_channel(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		return tag->m_refchan;
	return NULL;
}

void TagList::set_tag_chan(const int tag_index, const int channel_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr) 
		tag->m_refchan = channel_index;
}

void TagList::set_tag_pixel(const int tag_index, const int pixel_value)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr) 
		tag->m_pixel = pixel_value;
}

int TagList::get_tag_pixel(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		return tag->m_pixel;
	return NULL;
}

void TagList::set_tag_l_value(const int tag_index, const long value)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		tag->m_lvalue = value;
}

long TagList::get_tag_l_val(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		return tag->m_lvalue;
	return NULL;
}

void TagList::set_tag_comment(const int tag_index, const CString& comment)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		tag->m_csComment = comment;
}

CString TagList::get_tag_comment(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	CString cs;
	if (tag != nullptr)
		cs = tag->m_csComment;
	return cs;
}

int TagList::get_tag_list_size() const
{
	return tag_ptr_array_.GetSize();
}

Tag* TagList::get_tag(const int tag_index)
{
	if (tag_index >= 0 && tag_index < tag_ptr_array_.GetSize())
		return tag_ptr_array_.GetAt(tag_index);
	return nullptr;
}

void TagList::copy_tag_list(TagList* p_t_list)
{
	if (p_t_list == this || p_t_list == nullptr)
		return;

	remove_all_tags();
	const auto n_tags = p_t_list->tag_ptr_array_.GetSize();
	for (auto i = 0; i < n_tags; i++)
	{
		const auto p_tag = p_t_list->get_tag(i);
		if (p_tag != nullptr)
		{
			const auto tag = new Tag;
			ASSERT(tag != NULL);
			tag->m_refchan = p_tag->m_refchan;
			tag->m_pixel = p_tag->m_pixel;
			tag->m_value = p_tag->m_value;
			tag->m_lvalue = p_tag->m_lvalue;
			insert_tag(tag);
		}
	}
}

long TagList::Write(CFile* p_data_file)
{
	long l_size = sizeof(int);
	p_data_file->Write(&m_version_, l_size);
	const auto n_elements = tag_ptr_array_.GetSize();
	p_data_file->Write(&n_elements, l_size);
	l_size += l_size;

	for (auto i = 0; i < n_elements; i++)
	{
		const auto tag = tag_ptr_array_.GetAt(i);
		l_size += tag->Write(p_data_file);
	}
	return l_size;
}

BOOL TagList::Read(CFile* p_data_file)
{
	int version;
	p_data_file->Read(&version, sizeof(int));
	int n_elements;
	p_data_file->Read(&n_elements, sizeof(int));

	for (auto i = 0; i < n_elements; i++)
	{
		const auto tag = new Tag;
		ASSERT(tag != NULL);
		tag->Read(p_data_file);
		tag_ptr_array_.Add(tag);
	}
	return TRUE;
}
