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
	void display_elements() const;
	void modify_current();
	void modify_all();
	void modify_selected()
	{
		const auto i_edit = m_pMainTable->GetEditMode();
		if (i_edit != dbEditNone)
			m_pMainTable->Update();

		const auto bookmark_current = m_pMainTable->GetBookmark();

		const auto u_selected_count = m_pdbDoc->selected_records.GetSize();
		ASSERT(u_selected_count > 0);

		for (auto i = 0; i < u_selected_count; i++)
		{
			const long n_item = m_pdbDoc->selected_records.GetAt(i);
			m_pMainTable->SetAbsolutePosition(n_item);
			modify_current();
		}
		m_pMainTable->SetBookmark(bookmark_current);
	}

	long m_source_id_{ 0 };
	long m_dest_id_{ 0 };
	long m_initial_id_ {-1};

public:
	int m_source_select{REC_CURRENT};
	int m_source_condition{ COND_EQU };
	int m_dest_action{ CHGE_ID };
	BOOL m_b_case_sensitive{ false };

	CString m_cs_field_value { _T("") };
	CString m_cs_text_search{ _T("") };
	CString m_cs_text_replace_with{ _T("") };
	CComboBox m_co_dictionary{};
	BOOL m_b_co_dictionary_changed{ false };
	CComboBox m_co_source;
	int m_first{ -1 };

	CdbTableMain* m_pMainTable{ nullptr }; 
	CString m_csColName; 
	CdbTableAssociated* m_pIndexTable{ nullptr }; 
	CArray<long, long>* m_pliIDArray{ nullptr }; 
	BOOL m_bIndexTable{ true }; // TRUE=linked field, FALSE=main field (m_pIndexTable=NULL)
	CdbWaveDoc* m_pdbDoc {nullptr};

	BOOL OnInitDialog() override;
	afx_msg void OnBnClickedRadio1();
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
