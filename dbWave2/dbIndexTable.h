#pragma once

#pragma warning(disable : 4995)

class CdbIndexTable : public CDaoRecordset
{
public:
	CdbIndexTable(CDaoDatabase* pDatabase = nullptr);
	DECLARE_DYNAMIC(CdbIndexTable)

	// Field/Param Data
	CString m_cs;
	long m_ID;

	CString m_defaultSQL;
	CString m_DFX_cs;
	CString m_DFX_ID;

	// temp value
	CString m_defaultName;

	// Overrides
public:
	CString GetDefaultDBName() override; // Default database name
	CString GetDefaultSQL() override; // Default SQL for Recordset
	void DoFieldExchange(CDaoFieldExchange* pFX) override; // RFX support

	// operations
	void SetNames(CString csdefaultSQL, CString DFX_cs, CString DFX_ID);
	long GetIDorCreateIDforString(const CString& cs);
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
