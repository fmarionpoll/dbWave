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
	int add_tag(const Tag& arg); 
	void copy_tag_list(TagList* p_t_list); 

	int remove_tag(int tag_index); 
	void remove_all_tags();
	int remove_chan_tags(int reference_channel); 

	int get_tag_list_size() const;
	Tag* get_tag(int tag_index);

	void set_channel(int tag_index, int channel_index);
	int get_channel(int tag_index);

	void set_value_int(int i_tag, int value);
	int get_value_int(int i_tag);

	void set_pixel(int tag_index, int pixel_value);
	int get_tag_pixel(int tag_index);

	void set_value_long(int tag_index, long value); 
	long get_tag_value_long(int tag_index);

	void set_comment(int tag_index, const CString& comment);
	CString get_comment(int tag_index); 

	long write(CFile* p_data_file);
	BOOL read(CFile* p_data_file);

	int hit_vertical_tag_long(long lx, long jitter);
	int hit_vertical_tag_pixel(int x, int jitter);
};
