
#pragma once

// dbInsectTable.h : header file
//
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4995)

class CdbIndexTable : public CDaoRecordset
{
public:
	CdbIndexTable(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CdbIndexTable)

// Field/Param Data
	CString	m_cs;
	long	m_ID;
	
	CString	m_defaultSQL;
	CString m_DFX_cs;
	CString	m_DFX_ID;

// temp value
	CString	m_defaultName;

// Overrides
public:
	virtual CString GetDefaultDBName();						// Default database name
	virtual CString GetDefaultSQL();						// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);	// RFX support

	// operations
	void	SetNames(CString csdefaultSQL, CString DFX_cs, CString DFX_ID);
	long	GetIDorCreateIDforString (CString cs);
	BOOL	GetIDFromString (CString cs, long& iID);
	BOOL	SeekID(long iID);
	CString GetStringFromID (long iID);
	void	CreateIndextable(CString cstable, CString cs, CString csID, int textSize, CDaoDatabase* pDB);
	int		AddStringsFromCombo(CComboBox* pcombo);
	int		RemoveStringsNotInCombo(CComboBox* pcombo);

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};
