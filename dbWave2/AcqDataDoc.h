#pragma once

// sort options :
auto constexpr BY_TIME = 1;
auto constexpr BY_NAME = 2;
auto constexpr MAX_BUFFER_LENGTH_AS_BYTES = 614400;

//  1 s at 10 kHz =  1(s) * 10 000 (data points) * 2 (bytes) * 3 (channels) = 60 000
// 10 s at 10 kHz = 10(s) * 10 000 (data points) * 2 (bytes) * 3 (channels) = 600 000
// with a multiple of 1024 =  614400

#include "WaveBuf.h"
#include "datafile_X.h"
#include "options_view_data.h"

class AcqDataDoc : public CDocument
{
	friend class CDlgImportGenericData;
	DECLARE_DYNCREATE(AcqDataDoc)

	AcqDataDoc();
	~AcqDataDoc() override;

	CString get_data_file_infos(const options_view_data* p_vd) const;
	void export_data_file_to_txt_file(CStdioFile* pdata_dest);
	BOOL save_document(CString& sz_path_name);
	BOOL open_document(CString& sz_path_name);
	BOOL open_acq_file(CString& cs_filename, const CFileStatus& status);
	BOOL OnNewDocument() override;

protected:
	static bool dlg_import_data_file(CString& sz_path_name);
	int import_file(CString& sz_path_name);
	static void remove_file(const CString& file1);
	static void rename_file(const CString& filename_old, const CString& filename_new);

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

public:
	// Data members and functions dealing with CDataFileX and data reading buffer

	CDataFileX* x_file = nullptr;
	int x_file_type = DOCTYPE_UNKNOWN;
	int last_document_type = DOCTYPE_UNKNOWN;
	int doc_n_channels = 0; // nb channels / doc
	int doc_offset_int = sizeof(short); // offset in bytes
	long doc_channel_length = 0;

	CWaveBuf* p_w_buf = nullptr;
	long buf_max_size = MAX_BUFFER_LENGTH_AS_BYTES; // constant (?) size of the buffer
	long buf_size = 0; // buffer size (n channels * sizeof(word) * lRWSize
	long buf_channel_size = 0; // n words in buffer / channel
	long buf_channel_first = 0; // file index of first word in RW buffer
	long buf_channel_last = 0; // file index of last word in RW buffer
	BOOL buf_valid_data = false; // flag / valid data ; FALSE to force reading data from file
	BOOL buf_valid_transform = false; // flag to tell that transform buffer is valid (check if really used?)
	BOOL b_defined = false;
	BOOL b_remove_offset = true; // transform data read in short values if binary offset encoding

	// use with caution - parameters set by last call to LoadTransformedData
protected:
	long buf_first_ = 0;
	long buf_last_ = 1;
	int buf_transform_ = 0;
	int buf_source_channel_ = 0;

public:
	long get_t_buffer_first() const { return buf_first_; }
	short get_value_from_buffer(int channel, long l_index);
	short* load_transformed_data(long l_first, long l_last, int transform_type, int source_channel);
	BOOL build_transformed_data(int transform_type, int source_channel) const;
	BOOL load_raw_data(long* l_first, long* l_last, int span /*, BOOL bImposedReading*/);

	// write data
	BOOL write_hz_tags(TagList* p_tags) const;
	BOOL write_vt_tags(TagList* p_tags) const;
	void acq_delete_file() const;
	void acq_close_file() const;
	BOOL acq_save_data_descriptors() const;

	// AwaveFile operations -- could add parameter to create other type
	BOOL acq_create_file(CString& cs_file_name);
	BOOL save_as(CString& new_name, BOOL b_check_over_write = TRUE, int i_type = 0);

	// helpers
	long get_doc_channel_length() const { return doc_channel_length; }
	long get_doc_channel_index(const int kd) const { return (buf_channel_first + kd - 1); }
	long get_doc_channel_index_first() const { return buf_channel_first; }
	long get_buffer_channel_index(const long l_pos) const { return (l_pos - buf_channel_first); }
	long get_buffer_channel_length() const { return buf_channel_size; }

	CWaveChanArray* get_wave_channels_array() const { return &p_w_buf->channels_array_; }
	CWaveFormat* get_wave_format() const { return &p_w_buf->wave_format_; }
	TagList* get_hz_tags_list() const { return p_w_buf->get_p_hz_tags(); }
	TagList* get_vt_tags_list() const { return p_w_buf->get_p_vt_tags(); }

	int get_scan_count() const { return p_w_buf->wave_format_.scan_count; }
	short* get_raw_data_buffer() const { return p_w_buf->get_pointer_to_raw_data_buffer(); }
	short* get_raw_data_element(const int chan, const int index) const
	{
		return p_w_buf->get_pointer_to_raw_data_element(chan, index - buf_channel_first);
	}

	short* get_transformed_data_buffer() const { return p_w_buf->get_pointer_to_transformed_data_buffer(); }
	short* get_transformed_data_element(const int index) const { return (p_w_buf->get_pointer_to_transformed_data_buffer() + index); }

	static int get_transformed_data_span(const int i) { return CWaveBuf::wb_get_transform_span(i); }
	static int get_transforms_count() { return CWaveBuf::wb_get_n_transforms(); }
	static CString get_transform_name(const int i) { return CWaveBuf::wb_get_transforms_allowed(i); }
	static BOOL set_wb_transform_span(const int i, const int i_span) { return CWaveBuf::wb_set_transform_span(i, i_span); }
	static int is_wb_span_change_allowed(const int i) { return CWaveBuf::wb_is_span_change_allowed(i); }
	static BOOL is_wb_transform_allowed(const int i_mode) { return CWaveBuf::wb_is_transform_allowed(i_mode); }

	BOOL get_volts_per_bin(const int channel, float* volts_per_bin, const int mode = 0) const
	{
		return p_w_buf->get_wb_volts_per_bin(channel, volts_per_bin, mode);
	}

	BOOL init_wb_transform_buffer() const { return p_w_buf->wb_init_transform_buffer(); }

	void set_offset_to_data(const ULONGLONG ul_offset) const { x_file->m_ul_offset_data = ul_offset; }
	void set_offset_to_header(const ULONGLONG ul_offset) const { x_file->m_ul_offset_header = static_cast<LONGLONG>(ul_offset); }
	ULONGLONG get_offset_to_data() const { return x_file->m_ul_offset_data; }
	ULONGLONG get_offset_to_header() const { return x_file->m_ul_offset_header; }
	int get_header_size() const { return x_file->m_b_header_size; }
	void set_reading_buffer_dirty() { buf_valid_data = FALSE; }
	BOOL allocate_buffer();
	BOOL adjust_buffer(const int elements_count);
	void read_data_infos();

protected:
	BOOL read_data_block(long l_first);
	void instantiate_data_file_object(int doc_type);

	DECLARE_MESSAGE_MAP()
};
