#include "StdAfx.h"
#include "datafile_X.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Wrapper to generic data files
// Object subclassed for specific data files of other types
// One routine is common to all: reading data from files assuming that data
// are 16 bits numbers and that channels are interleaved

IMPLEMENT_DYNCREATE(CDataFileX, CObject)

CDataFileX::CDataFileX()
{
	m_b_header_size = 0;
	m_ul_offset_data = 0;
	m_ul_offset_header = 0;
	m_ul_bytes_count = 0;
	m_id_type = DOCTYPE_UNKNOWN;
	m_cs_type = _T("UNKNOWN");
	UINT m_u_open_flag = modeReadWrite | shareDenyNone | typeBinary;
}

CDataFileX::~CDataFileX()
{
}

#ifdef _DEBUG

void CDataFileX::AssertValid() const
{
	CObject::AssertValid();
}

void CDataFileX::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc << "header_size = " << m_b_header_size;
	dc << "header_offset = " << m_ul_offset_header;
	dc << "data_offset = " << m_ul_offset_data;
}
#endif //_DEBUG

long CDataFileX::read_adc_data(const long data_index, const long nb_points, short* p_buffer, CWaveChanArray* p_array)
{
	// seek and read CFile
	const LONGLONG l_off = (static_cast<LONGLONG>(data_index) * sizeof(short)) + m_ul_offset_data;
	Seek(l_off, begin);
	const long l_size = Read(p_buffer, nb_points);
	// adjust dependent parameters
	return l_size / sizeof(short);
}

int CDataFileX::check_file_type(CString& cs_filename)
{
	return DOCTYPE_UNKNOWN;
}

BOOL CDataFileX::read_data_infos(CWaveBuf* pBuf)
{
	return TRUE;
}

BOOL CDataFileX::read_hz_tags(TagList* p_hz_tags)
{
	return FALSE;
}

BOOL CDataFileX::read_vt_tags(TagList* p_vt_tags)
{
	return FALSE;
}

BOOL CDataFileX::init_file()
{
	return FALSE;
}

BOOL CDataFileX::data_append_start()
{
	m_ul_bytes_count = 0;
	return FALSE;
}

BOOL CDataFileX::data_append(short* p_bu, UINT ui_bytes_length)
{
	return FALSE;
}

BOOL CDataFileX::data_append_stop()
{
	return FALSE;
}

BOOL CDataFileX::write_data_infos(CWaveFormat* pw_f, CWaveChanArray* pw_c)
{
	return FALSE;
}

BOOL CDataFileX::write_hz_tags(TagList* p_tags)
{
	return FALSE;
}

BOOL CDataFileX::write_vt_tags(TagList* p_tags)
{
	return FALSE;
}

int CDataFileX::is_pattern_present(const char* buf_read, const int len_read, const char* buf_pattern, const int len_pattern)
{
	const std::string needle(buf_pattern, len_pattern - 1);
	std::string haystack(buf_read, len_read);
	const std::size_t n = haystack.find(needle);
	int flag = DOCTYPE_UNKNOWN;
	if (n != std::string::npos)
	{
		flag = m_id_type;
		m_cs_type = buf_pattern;
	}
	return flag;
}

bool CDataFileX::open_data_file(CString& sz_path_name, const UINT u_open_flag)
{
	CFileException error{};
	const bool flag = Open(sz_path_name, u_open_flag, &error);
	if (flag == 0)
		Abort();
	return flag;
}

void CDataFileX::close_data_file()
{
	if (m_hFile != hFileNull)
		Close();
}
