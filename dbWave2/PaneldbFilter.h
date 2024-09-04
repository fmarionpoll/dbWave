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
	void populate_item_from_table_long(DB_ITEMDESC* p_desc) const;
	void populate_item_from_linked_table(DB_ITEMDESC* p_desc) const;
	void populate_item_from_table_with_date(DB_ITEMDESC* p_desc) const;
	static void insert_alphabetic(const CString& cs, CStringArray& cs_array);
	void build_filter_item_indirection_from_tree(DB_ITEMDESC* p_desc, HTREEITEM start_item) const;
	void build_filter_item_long_from_tree(DB_ITEMDESC* p_desc, HTREEITEM start_item) const;
	void build_filter_item_date_from_tree(DB_ITEMDESC* p_desc, HTREEITEM start_item) const;
	void select_next(BOOL b_next);

	// Implementation
public:
	~PaneldbFilter() override;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lp_create_struct);
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* p_wnd, CPoint point);
	afx_msg void on_update_tree();
	afx_msg void on_apply_filter();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* p_old_wnd);
	afx_msg void on_sort_records();
	afx_msg void on_select_next();
	afx_msg void on_select_previous();

public:
	afx_msg void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint);
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);

	DECLARE_MESSAGE_MAP()
};
