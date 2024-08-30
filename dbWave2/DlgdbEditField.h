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
	DlgdbEditField(CWnd* p_parent = nullptr);
	~DlgdbEditField() override;

	enum { IDD = IDD_DBEDITRECORDFIELD };

protected:
	void DoDataExchange(CDataExchange* p_dx) override;
	void display_elements() const;
	void modify_current();
	void modify_all();
	void modify_selected()
	{
		const auto i_edit = m_p_main_table->GetEditMode();
		if (i_edit != dbEditNone)
			m_p_main_table->Update();

		const auto bookmark_current = m_p_main_table->GetBookmark();

		const auto u_selected_count = m_pdb_doc->selected_records.GetSize();
		ASSERT(u_selected_count > 0);

		for (auto i = 0; i < u_selected_count; i++)
		{
			const long n_item = static_cast<long>(m_pdb_doc->selected_records.GetAt(i));
			m_p_main_table->SetAbsolutePosition(n_item);
			modify_current();
		}
		m_p_main_table->SetBookmark(bookmark_current);
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

	CdbTableMain* m_p_main_table{ nullptr }; 
	CString m_cs_col_name; 
	CdbTableAssociated* m_p_index_table{ nullptr }; 
	CArray<long, long>* m_pli_id_array{ nullptr }; 
	BOOL m_b_index_table{ true }; // TRUE=linked field, FALSE=main field (m_pIndexTable=NULL)
	CdbWaveDoc* m_pdb_doc {nullptr};

	BOOL OnInitDialog() override;
	afx_msg void OnSize(UINT n_type, int cx, int cy);

	afx_msg void on_bn_clicked_radio1();
	afx_msg void on_bn_clicked_radio2();
	afx_msg void on_bn_clicked_radio3();
	afx_msg void on_bn_clicked_radio4();
	afx_msg void on_bn_clicked_radio5();
	afx_msg void on_bn_clicked_radio6();
	afx_msg void on_bn_clicked_button1();
	afx_msg void on_bn_clicked_ok();
	afx_msg void on_cbn_sel_change_combo3();

	DECLARE_MESSAGE_MAP()
};
