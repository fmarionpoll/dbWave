#pragma once
#include "WaveBuf.h"

// Document types 
constexpr auto DOCTYPE_UNKNOWN = -1; // document type not recognized;
constexpr auto DOCTYPE_AWAVE = 0; // aWave document;
constexpr auto DOCTYPE_ASYST = 1; // ASYST document;
constexpr auto DOCTYPE_ATLAB = 2; // ATLAB document;
constexpr auto DOCTYPE_ASDSYNTECH = 3; // ASD SYNTECH document (data);
constexpr auto DOCTYPE_MCID = 4; // MCI cf A French, Halifax;
constexpr auto DOCTYPE_SMR = 5; // CFS cambridge Filing system;

class CDataFileX : public CFile
{
public:
	CDataFileX();
	~CDataFileX() override;
	DECLARE_DYNCREATE(CDataFileX)

	virtual bool open_data_file(CString& sz_path_name, UINT u_open_flag);
	virtual void close_data_file();
	// Operations
	virtual int check_file_type(CString& cs_filename);
	virtual BOOL read_data_infos(CWaveBuf* pBuf);
	virtual long read_adc_data(long data_index, long nb_points, short* p_buffer, CWaveChanArray* p_array);
	virtual BOOL read_hz_tags(TagList* p_hz_tags);
	virtual BOOL read_vt_tags(TagList* p_vt_tags);

	// I/O operations to be updated by Save
	virtual BOOL init_file();
	virtual BOOL data_append_start();
	virtual BOOL data_append(short* p_bu, UINT ui_bytes_length);
	virtual BOOL data_append_stop();
	virtual BOOL write_data_infos(CWaveFormat* pw_f, CWaveChanArray* pw_c);
	virtual BOOL write_hz_tags(TagList* p_tags);
	virtual BOOL write_vt_tags(TagList* p_tags);

	//data parameters
	UINT m_u_open_flag;
	int m_b_header_size; // data header size
	int m_id_type; // file type 0=unknown; 1=atlab; 2=Asyst; 3=awave; ..
	CStringA m_cs_type;
	ULONGLONG m_ul_offset_data; // file offset of data start
	LONGLONG m_ul_offset_header; // file offset of data header
	ULONGLONG m_ul_bytes_count; // dummy parameter: Data append

	// Implementation
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
protected:
	int is_pattern_present(const char* buf_read, int len_read, const char* buf_pattern, int len_pattern);
};
