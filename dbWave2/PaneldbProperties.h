// PropertiesWnd.h

#pragma once
#include "dbWaveDoc.h"
#include "PaneldbPropertiesToolBar.h"


class PaneldbProperties : public CDockablePane
{
	DECLARE_DYNAMIC(PaneldbProperties)
public:
	PaneldbProperties();
	~PaneldbProperties() override;
	void AdjustLayout() override;

protected:
	PaneldbPropertiesToolBar m_wnd_tool_bar_;
	CMFCPropertyGridCtrl m_wnd_prop_list_;

	// Implementation
	CdbWaveDoc* m_p_doc_ {nullptr};
	CdbWaveDoc* m_p_doc_old_ {nullptr};
	CFont m_fnt_prop_list_;

	static int m_no_col_[]; // [26] succession of fields
	static int m_prop_col_[N_TABLE_COLUMNS]; // no col (0 to 28), no group (1, 2, 3 or -1 if not displayed) and type ()
	CUIntArray m_type_props_;
	CUIntArray m_i_id_props_;
	CUIntArray m_group_props_;
	int m_wnd_edit_infos_height_ {0};
	BOOL m_b_update_combos_ {FALSE};
	BOOL m_b_changed_property_ {FALSE};

	int init_group_from_table(CMFCPropertyGridProperty* p_group, int i_col0);
	void update_group_prop_from_table(CMFCPropertyGridProperty* p_group) const;
	void init_prop_list();
	void update_prop_list();
	void set_prop_list_font();
	void update_table_from_prop();
	void update_table_from_group_prop(const CMFCPropertyGridProperty* p_group);

	afx_msg int OnCreate(LPCREATESTRUCT lp_create_struct);
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* p_old_wnd);
	afx_msg void OnSettingChange(UINT u_flags, LPCTSTR lpsz_section);

	afx_msg void on_expand_all_properties();
	afx_msg void on_update_expand_all_properties(CCmdUI* p_cmd_ui);
	afx_msg void on_sort_properties();
	afx_msg void on_update_sort_properties(CCmdUI* p_cmd_ui);
	afx_msg void on_bn_clicked_edit_infos();
	afx_msg void on_update_bn_edit_infos(CCmdUI* p_cmd_ui);
	afx_msg void on_bn_clicked_update_infos();
	afx_msg void on_update_bn_update_infos(CCmdUI* p_cmd_ui);
	afx_msg LRESULT on_property_changed(WPARAM, LPARAM);

public:
	afx_msg void OnUpdate(CView* p_sender, LPARAM l_hint, CObject* p_hint);
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);

	DECLARE_MESSAGE_MAP()
};
