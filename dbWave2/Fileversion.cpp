// FileVersion.cpp: implementation of the CFileVersion class.
// by Manuel Laflamme

#include "StdAfx.h"
#include "Fileversion.h"

#pragma comment(lib, "version")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFileVersion::CFileVersion()
{
	m_lp_version_data_ = nullptr;
	m_dw_lang_charset_ = 0;
}

CFileVersion::~CFileVersion()
{
	close();
}

void CFileVersion::close()
{
	SAFE_DELETE_ARRAY(m_lp_version_data_);
	m_dw_lang_charset_ = 0;
}

BOOL CFileVersion::open(const LPCTSTR lpsz_module_name)
{
	ASSERT(_tcslen(lpsz_module_name) > 0);
	ASSERT(m_lp_version_data_ == NULL);

	// Get the version information size for allocate the buffer
	DWORD dw_handle = 0;
	const auto dw_data_size = ::GetFileVersionInfoSize(lpsz_module_name, &dw_handle);
	if (dw_data_size == 0)
		return FALSE;

	// Allocate buffer and retrieve version information
	m_lp_version_data_ = new BYTE[dw_data_size];
	if (!::GetFileVersionInfo(lpsz_module_name, 0, dw_data_size, m_lp_version_data_))
	{
		close();
		return FALSE;
	}

	// Retrieve the first language and character-set identifier
	UINT n_query_size;
	DWORD* p_trans_table;
	if (!::VerQueryValue(m_lp_version_data_, _T("\\VarFileInfo\\Translation"),
	                     reinterpret_cast<void**>(&p_trans_table), &n_query_size))
	{
		close();
		return FALSE;
	}

	// Swap the words to have lang-charset in the correct format
	m_dw_lang_charset_ = MAKELONG(HIWORD(p_trans_table[0]), LOWORD(p_trans_table[0]));

	return TRUE;
}

CString CFileVersion::query_value(const LPCTSTR lpsz_value_name, DWORD dw_lang_charset /* = 0*/)
{
	// Must call Open() first
	ASSERT(m_lp_version_data_ != NULL);
	if (m_lp_version_data_ == nullptr)
		return L"";

	// If no lang-charset specified use default
	if (dw_lang_charset == 0)
		dw_lang_charset = m_dw_lang_charset_;

	// Query version information value
	UINT n_query_size;
	LPVOID lp_data;
	CString str_value, str_block_name;
	str_block_name.Format(_T("\\StringFileInfo\\%08lx\\%s"),
	                      dw_lang_charset, lpsz_value_name);
	if (::VerQueryValue(m_lp_version_data_, str_block_name.GetBuffer(0),
	                    &lp_data, &n_query_size))
		str_value = static_cast<LPCTSTR>(lp_data);

	str_block_name.ReleaseBuffer();

	return str_value;
}

BOOL CFileVersion::get_fixed_info(VS_FIXEDFILEINFO& vs_ff_i)
{
	// Must call Open() first
	ASSERT(m_lp_version_data_ != NULL);
	if (m_lp_version_data_ == nullptr)
		return FALSE;

	UINT n_query_size;
	VS_FIXEDFILEINFO* p_vs_ff_i;
	if (::VerQueryValue(m_lp_version_data_, _T("\\"),
	                    reinterpret_cast<void**>(&p_vs_ff_i), &n_query_size))
	{
		vs_ff_i = *p_vs_ff_i;
		return TRUE;
	}

	return FALSE;
}

CString CFileVersion::get_fixed_file_version()
{
	CString str_version;
	VS_FIXEDFILEINFO vs_ff_i;

	if (get_fixed_info(vs_ff_i))
	{
		str_version.Format(_T("%u,%u,%u,%u"), HIWORD(vs_ff_i.dwFileVersionMS),
		                   LOWORD(vs_ff_i.dwFileVersionMS),
		                   HIWORD(vs_ff_i.dwFileVersionLS),
		                   LOWORD(vs_ff_i.dwFileVersionLS));
	}
	return str_version;
}

CString CFileVersion::get_fixed_product_version()
{
	CString str_version;
	VS_FIXEDFILEINFO vs_ff_i;

	if (get_fixed_info(vs_ff_i))
	{
		str_version.Format(_T("%u,%u,%u,%u"), HIWORD(vs_ff_i.dwProductVersionMS),
		                   LOWORD(vs_ff_i.dwProductVersionMS),
		                   HIWORD(vs_ff_i.dwProductVersionLS),
		                   LOWORD(vs_ff_i.dwProductVersionLS));
	}
	return str_version;
}
