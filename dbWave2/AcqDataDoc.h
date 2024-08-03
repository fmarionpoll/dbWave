#pragma once

// sort options :
auto constexpr BY_TIME = 1;
auto constexpr BY_NAME = 2;
auto constexpr MAX_BUFFER_LENGTH_AS_BYTES = 614400;

//  1 s at 10 kHz =  1(s) * 10 000 (data points) * 2 (bytes) * 3 (chans) = 60 000
// 10 s at 10 kHz = 10(s) * 10 000 (data points) * 2 (bytes) * 3 (chans) = 600 000
// with a multiple of 1024 =  614400

#include "WaveBuf.h"
#include "datafile_X.h"
#include "OPTIONS_VIEWDATA.h"

class AcqDataDoc : public CDocument
{
	friend class CDlgImportGenericData;
	DECLARE_DYNCREATE(AcqDataDoc)

	AcqDataDoc();
	~AcqDataDoc() override;

	CString get_data_file_infos(const OPTIONS_VIEWDATA* pVD) const;
	void export_data_file_to_txt_file(CStdioFile* pdataDest);
	BOOL save_document(CString& sz_path_name);
	BOOL open_document(CString& sz_path_name);
	BOOL OnNewDocument() override;

	BOOL open_acq_file(CString& cs_filename);
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

	CDataFileX* m_pXFile = nullptr;
	int m_pXFileType = DOCTYPE_UNKNOWN;
	int m_lastDocumentType = DOCTYPE_UNKNOWN;
	int m_DOCnbchans = 0; // nb channels / doc
	int m_iOffsetInt = sizeof(short); // offset in bytes
	long m_lDOCchanLength = 0;

	CWaveBuf* m_pWBuf = nullptr;
	long m_lBUFmaxSize = MAX_BUFFER_LENGTH_AS_BYTES; // constant (?) size of the buffer
	long m_lBUFSize = 0; // buffer size (n channels * sizeof(word) * lRWSize
	long m_lBUFchanSize = 0; // n words in buffer / channel
	long m_lBUFchanFirst = 0; // file index of first word in RW buffer
	long m_lBUFchanLast = 0; // file index of last word in RW buffer
	BOOL m_bValidReadBuffer = false; // flag / valid data ; FALSE to force reading data from file
	BOOL m_bValidTransfBuffer = false; // flag to tell that transform buffer is valid (check if really used?)
	BOOL m_bdefined = false;
	BOOL m_bRemoveOffset = true; // transform data read in short values if binary offset encoding

	// use with caution - parameters set by last call to LoadTransformedData
	long m_tBUFfirst = 0;
	long m_tBUFlast = 1;
	int m_tBUFtransform = 0;
	int m_tBUFsourcechan = 0;

public:
	long get_t_buffer_first() const { return m_tBUFfirst; }
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
	long get_doc_channel_length() const { return m_lDOCchanLength; }
	long get_doc_channel_index(int kd) const { return (m_lBUFchanFirst + kd - 1); }
	long get_doc_channel_index_first() const { return m_lBUFchanFirst; }
	long get_buffer_channel_index(long lPos) const { return (lPos - m_lBUFchanFirst); }
	long get_buffer_channel_length() const { return m_lBUFchanSize; }

	CWaveChanArray* get_wave_channels_array() const { return &m_pWBuf->m_chanArray; }
	CWaveFormat* get_wave_format() const { return &m_pWBuf->m_waveFormat; }
	TagList* get_hz_tags_list() const { return m_pWBuf->GetpHZtags(); }
	TagList* get_vt_tags_list() const { return m_pWBuf->GetpVTtags(); }

	int get_scan_count() const { return m_pWBuf->m_waveFormat.scan_count; }
	short* get_raw_data_buffer() const { return m_pWBuf->get_pointer_to_raw_data_buffer(); }
	short* get_raw_data_element(const int chan, const int index) const
	{
		return m_pWBuf->get_pointer_to_raw_data_element(chan, index - m_lBUFchanFirst);
	}

	short* get_transformed_data_buffer() const { return m_pWBuf->get_pointer_to_transformed_data_buffer(); }
	short* get_transformed_data_element(const int index) const { return (m_pWBuf->get_pointer_to_transformed_data_buffer() + index); }

	static int get_transformed_data_span(const int i) { return CWaveBuf::GetWBTransformSpan(i); }
	static int get_transforms_count() { return CWaveBuf::GetWBNTypesofTransforms(); }
	static CString get_transform_name(const int i) { return CWaveBuf::GetWBTransformsAllowed(i); }
	static BOOL set_wb_transform_span(const int i, const int i_span) { return CWaveBuf::SetWBTransformSpan(i, i_span); }
	static int is_wb_span_change_allowed(const int i) { return CWaveBuf::IsWBSpanChangeAllowed(i); }
	static BOOL is_wb_transform_allowed(const int i_mode) { return CWaveBuf::IsWBTransformAllowed(i_mode); }

	BOOL get_volts_per_bin(const int channel, float* volts_per_bin, const int mode = 0) const
	{
		return m_pWBuf->GetWBVoltsperBin(channel, volts_per_bin, mode);
	}

	BOOL init_wb_transform_buffer() const { return m_pWBuf->InitWBTransformBuffer(); }

	void set_offset_to_data(ULONGLONG ulOffset) const { m_pXFile->m_ulOffsetData = ulOffset; }
	void set_offset_to_header(ULONGLONG ulOffset) const { m_pXFile->m_ulOffsetHeader = static_cast<LONGLONG>(ulOffset); }
	ULONGLONG get_offset_to_data() const { return m_pXFile->m_ulOffsetData; }
	ULONGLONG get_offset_to_header() const { return m_pXFile->m_ulOffsetHeader; }
	int get_header_size() const { return m_pXFile->m_bHeaderSize; }
	void set_reading_buffer_dirty() { m_bValidReadBuffer = FALSE; }
	BOOL allocate_buffer();
	BOOL adjust_buffer(const int elements_count);
	void read_data_infos();

protected:
	BOOL read_data_block(long l_first);
	void instantiate_data_file_object(int doc_type);

	DECLARE_MESSAGE_MAP()
};
