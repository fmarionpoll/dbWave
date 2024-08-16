#pragma once
#include "datafile_X.h"

class CDataFileASD :
	public CDataFileX
{
public:
	CDataFileASD();
	DECLARE_DYNCREATE(CDataFileASD)
	~CDataFileASD() override;

	// Operations
public:
	BOOL read_data_infos(CWaveBuf* p_buf) override;
	int check_file_type(CString& cs_filename) override;

protected:
	CString m_cs_old_string_id_;
	CString m_cs_string_id_;
	WORD m_w_id_;

	// Implementation
public:
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
};
