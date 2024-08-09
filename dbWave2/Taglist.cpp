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
		if (tag != nullptr && tag->ref_channel == reference_channel)
		{
			delete tag; // delete object pointed at
			tag_ptr_array_.RemoveAt(i); // remove item
		}
	}
	return tag_ptr_array_.GetSize();
}

void TagList::set_value_int(const int i_tag, const int value)
{
	if (tag_ptr_array_.GetSize() <= i_tag)
	{
		for (auto i = tag_ptr_array_.GetSize(); i <= i_tag; i++)
			add_tag(0, 0);
		ASSERT(tag_ptr_array_.GetSize() >= i_tag);
	}
	const auto p_cur = tag_ptr_array_.GetAt(i_tag);
	if (p_cur != nullptr) 
		p_cur->value_int = value;
}

int TagList::get_value_int(const int i_tag)
{
	const auto tag = tag_ptr_array_.GetAt(i_tag);
	if (tag != nullptr)
		return tag->value_int;
	return NULL;
}

int TagList::get_channel(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		return tag->ref_channel;
	return NULL;
}

void TagList::set_channel(const int tag_index, const int channel_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr) 
		tag->ref_channel = channel_index;
}

void TagList::set_pixel(const int tag_index, const int pixel_value)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr) 
		tag->pixel = pixel_value;
}

int TagList::get_tag_pixel(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		return tag->pixel;
	return NULL;
}

void TagList::set_value_long(const int tag_index, const long value)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		tag->value_long = value;
}

long TagList::get_tag_value_long(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		return tag->value_long;
	return NULL;
}

void TagList::set_comment(const int tag_index, const CString& comment)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	if (tag != nullptr)
		tag->m_cs_comment = comment;
}

CString TagList::get_comment(const int tag_index)
{
	const auto tag = tag_ptr_array_.GetAt(tag_index);
	CString cs;
	if (tag != nullptr)
		cs = tag->m_cs_comment;
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
			tag->ref_channel = p_tag->ref_channel;
			tag->pixel = p_tag->pixel;
			tag->value_int = p_tag->value_int;
			tag->value_long = p_tag->value_long;
			insert_tag(tag);
		}
	}
}

long TagList::write(CFile* p_data_file)
{
	long l_size = sizeof(int);
	p_data_file->Write(&m_version_, l_size);
	const auto n_elements = tag_ptr_array_.GetSize();
	p_data_file->Write(&n_elements, l_size);
	l_size += l_size;

	for (auto i = 0; i < n_elements; i++)
	{
		const auto tag = tag_ptr_array_.GetAt(i);
		l_size += tag->write(p_data_file);
	}
	return l_size;
}

BOOL TagList::read(CFile* p_data_file)
{
	int version;
	p_data_file->Read(&version, sizeof(int));
	int n_elements;
	p_data_file->Read(&n_elements, sizeof(int));

	for (auto i = 0; i < n_elements; i++)
	{
		const auto tag = new Tag;
		ASSERT(tag != NULL);
		tag->read(p_data_file);
		tag_ptr_array_.Add(tag);
	}
	return TRUE;
}

int TagList::hit_vertical_tag_long(const long lx, const long jitter)
{
	auto chit = -1;
	const auto array_size = tag_ptr_array_.GetSize();
	const long max = lx + jitter;
	const long min = lx - jitter;
	for (auto i = 0; i < array_size; i++)
	{
		const auto l_val = get_tag_value_long(i);
		if (l_val <= max && l_val >= min)
		{
			chit = i;
			break;
		}
	}
	return chit;
}

int TagList::hit_vertical_tag_pixel(const int x, const int jitter)
{
	auto chit = -1;
	const auto j = get_tag_list_size();
	const int max = x + jitter;
	const int min = x - jitter;
	for (auto i = 0; i < j; i++)
	{
		const auto val = get_tag_pixel(i);
		if (val <= max && val >= min)
		{
			chit = i;
			break;
		}
	}
	return chit;
}
