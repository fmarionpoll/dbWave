#pragma once
#pragma warning(disable : 4995)
#include <afxdao.h>			// MFC DAO database classes

class CdbTableAssociated : public CDaoRecordset
{
public:
	CdbTableAssociated(CDaoDatabase* p_database = nullptr);
	DECLARE_DYNAMIC(CdbTableAssociated)

	// Field/Param Data
	CString m_cs = _T("");
	long m_id = 0;
	CString m_default_sql = _T("[name]");
	CString m_dfx_cs = _T("[cs]");
	CString m_dfx_id = _T("[ID]");
	CString m_default_name = _T("");

public:
	CString GetDefaultDBName() override;
	CString GetDefaultSQL() override;
	void DoFieldExchange(CDaoFieldExchange* p_fx) override; 

	// operations
	void set_dfx_sql_names(const CString& cs_default_sql, const CString& dfx_cs, const CString& dfx_id);
	long get_string_in_linked_table(const CString& cs);
	BOOL get_id_from_string(const CString& cs, long& i_id);
	BOOL seek_id(long i_id);
	CString get_string_from_id(long i_id);
	void create_index_table(const CString& cs_table, const CString& cs_column1, const CString& cs_id_column2, int text_size,
	                      CDaoDatabase* p_database);
	int add_strings_from_combo(const CComboBox* p_combo);
	int remove_strings_not_in_combo(const CComboBox* p_combo);

	// Implementation
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
};
