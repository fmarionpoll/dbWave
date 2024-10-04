#pragma once
#include "PaneldbFilterToolBar.h"
#include "QuadStateTree.h"



class PaneldbFilter : public CDockablePane
{
	// Construction
public:
	PaneldbFilter();
	void AdjustLayout() override;

	// Attributes
protected:
	CQuadStateTree m_wnd_filter_view_;
	PaneldbFilterToolBar m_wnd_tool_bar_;

	CdbWaveDoc* m_p_doc_{nullptr};
	CdbWaveDoc* m_p_doc_old_{nullptr};
	static int m_no_col_[]; // [26] succession of fields that can be filtered
	HTREEITEM m_h_tree_item_[26]{};

	void init_filter_list();
	void fill_combo_with_categories(const CdbTable* p_db) const;

	DB_ITEMDESC* create_tree_category( CdbTable* p_db, int i) ;
	HTREEITEM create_tree_subitem_element(const DB_ITEMDESC* p_desc, int i, int j);
	void create_tree_subitem(const DB_ITEMDESC* p_desc, int i);

	void populate_item_from_table_long(DB_ITEMDESC* p_desc) const;
	void populate_item_from_linked_table(DB_ITEMDESC* p_desc) const;
	void populate_item_from_table_with_date(DB_ITEMDESC* p_desc) const;

	static void insert_alphabetic(const CString& cs, CStringArray& cs_array);
	void build_filter_item_indirection_from_tree(DB_ITEMDESC* p_desc, HTREEITEM start_item) const;
	void build_filter_item_long_from_tree(DB_ITEMDESC* p_desc, HTREEITEM start_item) const;
	void build_filter_item_date_from_tree(DB_ITEMDESC* p_desc, HTREEITEM start_item) const;
	void select_next(boolean b_next);
	void select_next_filter_item(boolean b_next);
	HTREEITEM select_tree_item(int col_requested);


	// Implementation
public:
	~PaneldbFilter() override;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lp_create_struct);
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* p_wnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* p_old_wnd);

	afx_msg void on_update_tree();
	afx_msg void on_apply_filter();

	afx_msg void on_record_sort();
	afx_msg void select_previous_combo_item();
	afx_msg void select_next_combo_item();
	afx_msg void on_update_bn_update_previous(CCmdUI* p_cmd_ui);
	afx_msg void on_update_bn_update_next(CCmdUI* p_cmd_ui);

public:
	afx_msg void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint);
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);

	afx_msg void on_tvn_sel_changed_filter_tree(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_nm_click_filter_tree(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_cbn_sel_change_category();

	DECLARE_MESSAGE_MAP()
};
