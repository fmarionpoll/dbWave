#pragma once


class CFileVersion
{
public:
	CFileVersion();
	~CFileVersion();
public:
	BOOL open(LPCTSTR lpsz_module_name);
	void close();

	CString query_value(LPCTSTR lpsz_value_name, DWORD dw_lang_charset = 0);
	CString get_file_description() { return query_value(_T("FileDescription")); };
	CString get_file_version() { return query_value(_T("FileVersion")); };
	CString get_internal_name() { return query_value(_T("InternalName")); };
	CString get_company_name() { return query_value(_T("CompanyName")); };
	CString get_legal_copyright() { return query_value(_T("LegalCopyright")); };
	CString get_original_filename() { return query_value(_T("OriginalFilename")); };
	CString get_product_name() { return query_value(_T("ProductName")); };
	CString get_product_version() { return query_value(_T("ProductVersion")); };

	BOOL get_fixed_info(VS_FIXEDFILEINFO& vs_ff_i);
	CString get_fixed_file_version();
	CString get_fixed_product_version();

protected:
	LPBYTE m_lp_version_data_;
	DWORD m_dw_lang_charset_;
};
