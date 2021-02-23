#pragma once

#include "afxwin.h"
#include "dbMainTable.h"
#include "dbIndexTable.h"

#define REC_ALL		0
#define REC_CURRENT	1
#define REC_SELECTED 2
#define	COND_EQU	0
#define COND_SEARCH	1
#define COND_NONE	2
#define	CHGE_ID		0
#define	CHGE_TXT	1
#define	CHGE_CLEAR	2

// CdbEditFieldDlg dialog
class CdbWaveDoc;

class CDlgdbEditField : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgdbEditField)

public:
	CDlgdbEditField(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgdbEditField();

	// Dialog Data
	enum { IDD = IDD_DBEDITRECORDFIELD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void	DisplayElements();
	void	ModifyCurrent();
	void	ModifyAll();
	void	ModifySelected()
	{
		const auto iedit = m_pMainTable->GetEditMode();
		if (iedit != dbEditNone)
			m_pMainTable->Update();

		const auto bookmark_current = m_pMainTable->GetBookmark();

		const auto u_selected_count = m_pdbDoc->m_selectedRecords.GetSize();
		ASSERT(u_selected_count > 0);

		for (auto i = 0; i < u_selected_count; i++)
		{
			const long n_item = m_pdbDoc->m_selectedRecords.GetAt(i);
			m_pMainTable->SetAbsolutePosition(n_item);
			ModifyCurrent();
		}
		m_pMainTable->SetBookmark(bookmark_current);
	}

	long	m_sourceID;
	long	m_destID{};
	long	m_initialID;
	DECLARE_MESSAGE_MAP()
public:
	int		m_sourceselect;
	int		m_sourcecondition;
	int		m_destaction;
	BOOL	m_bCaseSensitive;

	CString		m_csfieldvalue;
	CString		m_cstextsearch;
	CString		m_cstextreplacewith;
	CComboBox	m_codictionary;
	BOOL		m_bcodictchanged;
	CComboBox	m_cosource;
	int			m_first;

	//////////////////////////////////
	// parameters passed
	CdbMainTable* m_pMainTable{};		// address of main table
	CString			m_csColName;		// name of the column
	CdbIndexTable* m_pIndexTable{};		// address secondary table
	CArray<long, long>* m_pliIDArray{};		// address uiArray storing IDs
	BOOL			m_bIndexTable;		// TRUE=linked field, FALSE=main field (m_pIndexTable=NULL)
	CdbWaveDoc* m_pdbDoc{};
	/////////////////////////////////

	afx_msg void OnBnClickedRadio1();
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();
	afx_msg void OnBnClickedRadio6();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeCombo3();
};
