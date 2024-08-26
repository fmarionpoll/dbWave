#pragma once
#include "datafile_X.h"

class CDataFileAWAVE : public CDataFileX
{
public:
	// Construction / Destruction
	CDataFileAWAVE();
	CDataFileAWAVE(const CFile* p_file);
	~CDataFileAWAVE() override;

	int check_file_type(CString& cs_file_name) override;
	BOOL read_data_infos(CWaveBuf* p_buf) override;
	BOOL read_hz_tags(TagList* p_hz_tags) override;
	BOOL read_vt_tags(TagList* p_vt_tags) override;
	BOOL write_data_infos(CWaveFormat* pwF, CWaveChanArray* pwC) override;
	BOOL write_hz_tags(TagList* p_tags) override;
	BOOL write_vt_tags(TagList* p_tags) override;

	// I/O operations to be updated by Save
	BOOL init_file() override;
	BOOL data_append_start() override;
	BOOL data_append(short* pBU, UINT bytesLength) override;
	BOOL data_append_stop() override;

	// protected variables
protected:
	CStringA cs_file_desc_; // ASCII chain w. file version
	CMapWordToOb m_struct_map_; // sub-file descriptors
	BOOL b_modified_{};

	// Implementation
protected:
	void delete_map();
	void write_file_map();
	static void report_save_load_exception(LPCTSTR lpsz_path_name, CException* e, BOOL b_saving, UINT n_idp);
};
