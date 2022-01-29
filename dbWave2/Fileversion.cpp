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
	m_lpVersionData = nullptr;
	m_dwLangCharset = 0;
}

CFileVersion::~CFileVersion()
{
	Close();
}

void CFileVersion::Close()
{
	SAFE_DELETE_ARRAY(m_lpVersionData);
	m_dwLangCharset = 0;
}

BOOL CFileVersion::Open(LPCTSTR lpszModuleName)
{
	ASSERT(_tcslen(lpszModuleName) > 0);
	ASSERT(m_lpVersionData == NULL);

	// Get the version information size for allocate the buffer
	DWORD dw_handle = 0;
	const auto dw_data_size = ::GetFileVersionInfoSize(lpszModuleName, &dw_handle);
	if (dw_data_size == 0)
		return FALSE;

	// Allocate buffer and retrieve version information
	m_lpVersionData = new BYTE[dw_data_size];
	if (!::GetFileVersionInfo(lpszModuleName, 0, dw_data_size, m_lpVersionData))
	{
		Close();
		return FALSE;
	}

	// Retrieve the first language and character-set identifier
	UINT n_query_size;
	DWORD* p_trans_table;
	if (!::VerQueryValue(m_lpVersionData, _T("\\VarFileInfo\\Translation"),
	                     reinterpret_cast<void**>(&p_trans_table), &n_query_size))
	{
		Close();
		return FALSE;
	}

	// Swap the words to have lang-charset in the correct format
	m_dwLangCharset = MAKELONG(HIWORD(p_trans_table[0]), LOWORD(p_trans_table[0]));

	return TRUE;
}

CString CFileVersion::QueryValue(LPCTSTR lpszValueName, DWORD dwLangCharset /* = 0*/)
{
	// Must call Open() first
	ASSERT(m_lpVersionData != NULL);
	if (m_lpVersionData == nullptr)
		return L"";

	// If no lang-charset specified use default
	if (dwLangCharset == 0)
		dwLangCharset = m_dwLangCharset;

	// Query version information value
	UINT n_query_size;
	LPVOID lp_data;
	CString str_value, str_block_name;
	str_block_name.Format(_T("\\StringFileInfo\\%08lx\\%s"),
	                      dwLangCharset, lpszValueName);
	if (::VerQueryValue(m_lpVersionData, str_block_name.GetBuffer(0),
	                    &lp_data, &n_query_size))
		str_value = static_cast<LPCTSTR>(lp_data);

	str_block_name.ReleaseBuffer();

	return str_value;
}

BOOL CFileVersion::GetFixedInfo(VS_FIXEDFILEINFO& vsffi)
{
	// Must call Open() first
	ASSERT(m_lpVersionData != NULL);
	if (m_lpVersionData == nullptr)
		return FALSE;

	UINT nQuerySize;
	VS_FIXEDFILEINFO* p_vsffi;
	if (::VerQueryValue(m_lpVersionData, _T("\\"),
	                    reinterpret_cast<void**>(&p_vsffi), &nQuerySize))
	{
		vsffi = *p_vsffi;
		return TRUE;
	}

	return FALSE;
}

CString CFileVersion::GetFixedFileVersion()
{
	CString str_version;
	VS_FIXEDFILEINFO vsffi;

	if (GetFixedInfo(vsffi))
	{
		str_version.Format(_T("%u,%u,%u,%u"), HIWORD(vsffi.dwFileVersionMS),
		                   LOWORD(vsffi.dwFileVersionMS),
		                   HIWORD(vsffi.dwFileVersionLS),
		                   LOWORD(vsffi.dwFileVersionLS));
	}
	return str_version;
}

CString CFileVersion::GetFixedProductVersion()
{
	CString str_version;
	VS_FIXEDFILEINFO vsffi;

	if (GetFixedInfo(vsffi))
	{
		str_version.Format(_T("%u,%u,%u,%u"), HIWORD(vsffi.dwProductVersionMS),
		                   LOWORD(vsffi.dwProductVersionMS),
		                   HIWORD(vsffi.dwProductVersionLS),
		                   LOWORD(vsffi.dwProductVersionLS));
	}
	return str_version;
}
