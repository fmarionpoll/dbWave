#pragma once

#include "afxwin.h"
#include "dbTableMain.h"
#include "dbTableAssociated.h"

#define REC_ALL		0
#define REC_CURRENT	1
#define REC_SELECTED 2
#define	COND_EQU	0
#define COND_SEARCH	1
#define COND_NONE	2
#define	CHGE_ID		0
#define	CHGE_TXT	1
#define	CHGE_CLEAR	2


class DlgdbEditField : public CDialogEx
{
	DECLARE_DYNAMIC(DlgdbEditField)

public:
	DlgdbEditField(CWnd* pParent = nullptr);
	~DlgdbEditField() override;

	enum { IDD = IDD_DBEDITRECORDFIELD };

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void DisplayElements();
	void ModifyCurrent();
	void ModifyAll();

	void ModifySelected()
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

	long m_sourceID{ 0 };
	long m_destID{ 0 };
	long m_initialID {-1};

public:
	int m_sourceselect{REC_CURRENT};
	int m_sourcecondition{ COND_EQU };
	int m_destaction{ CHGE_ID };
	BOOL m_bCaseSensitive{ false };

	CString m_csfieldvalue { _T("") };
	CString m_cstextsearch{ _T("") };
	CString m_cstextreplacewith{ _T("") };
	CComboBox m_codictionary{};
	BOOL m_bcodictchanged{ false };
	CComboBox m_cosource;
	int m_first{ -1 };


	CdbMainTable* m_pMainTable{ nullptr }; 
	CString m_csColName; 
	CdbIndexTable* m_pIndexTable{ nullptr }; 
	CArray<long, long>* m_pliIDArray{ nullptr }; 
	BOOL m_bIndexTable{ true }; // TRUE=linked field, FALSE=main field (m_pIndexTable=NULL)
	CdbWaveDoc* m_pdbDoc{nullptr};


	afx_msg void OnBnClickedRadio1();
	BOOL OnInitDialog() override;

	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();
	afx_msg void OnBnClickedRadio6();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeCombo3();

	DECLARE_MESSAGE_MAP()
};
