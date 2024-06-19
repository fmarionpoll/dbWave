#pragma once

#include "Tag.h"

class TagList : public CObject
{
protected:
	CArray<Tag*, Tag*> tag_ptr_array_;
	int m_version_;
	int insert_tag(Tag* tag);

public:
	TagList();
	~TagList() override;

	int add_tag(int val, int reference_channel);
	int add_l_tag(long l_val, int reference_channel); 
	int add_tag(Tag& arg); 
	void copy_tag_list(TagList* p_t_list); 

	int remove_tag(int tag_index); 
	void remove_all_tags();
	int remove_chan_tags(int reference_channel); 

	void set_tag_chan(int tag_index, int channel_index); 
	void set_tag_val(int i_tag, int value); 
	void set_tag_pixel(int tag_index, int pixel_value);
	void set_tag_l_value(int tag_index, long value); 
	void set_tag_comment(int tag_index, const CString& comment);

	int get_channel(int tag_index); 
	int get_value(int i_tag);
	int get_tag_pixel(int tag_index); 
	long get_tag_l_val(int tag_index); 
	CString get_tag_comment(int tag_index); 

	int get_tag_list_size() const;
	Tag* get_tag(int tag_index);

	long Write(CFile* p_data_file);
	BOOL Read(CFile* p_data_file);
};
