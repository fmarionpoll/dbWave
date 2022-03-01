#pragma once

#pragma warning(disable : 4995)

class CdbTableAssociated : public CDaoRecordset
{
public:
	CdbTableAssociated(CDaoDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CdbTableAssociated)

	// Field/Param Data
	CString m_cs = _T("");
	long m_ID = 0;
	CString m_defaultSQL = _T("[name]");
	CString m_DFX_cs = _T("[cs]");
	CString m_DFX_ID = _T("[ID]");
	CString m_defaultName = _T("");

public:
	CString GetDefaultDBName() override;
	CString GetDefaultSQL() override;
	void DoFieldExchange(CDaoFieldExchange* pFX) override; 

	// operations
	void SetNames(CString csdefaultSQL, CString DFX_cs, CString DFX_ID);
	long GetStringInLinkedTable(const CString& cs);
	BOOL GetIDFromString(CString cs, long& iID);
	BOOL SeekID(long iID);
	CString GetStringFromID(long iID);
	void CreateIndextable(const CString& cstable, const CString& cs, const CString& csID, int textSize,
	                      CDaoDatabase* p_database);
	int AddStringsFromCombo(CComboBox* pcombo);
	int RemoveStringsNotInCombo(CComboBox* pcombo);

	// Implementation
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
};
