#pragma once
#include <afxwin.h>

#include "dbWaveDoc.h"
#include "SpikeClassGridList.h"
#include "SpikeClassGridToolBar.h"

class SpikeClassGrid :
	public CWnd
{
	DECLARE_DYNAMIC(SpikeClassGrid)
public:
	SpikeClassGrid();
	~SpikeClassGrid() override;
	void adjust_layout();

protected:
	SpikeClassGridToolBar property_toolbar_;
	SpikeClassGridList property_list_;

	CFont m_fnt_prop_list_;
	int m_wnd_edit_infos_height_{ 0 };
	BOOL m_b_changed_property_{ FALSE };

	void set_prop_list_font();
	int get_vertical_size_with_all_rows();

public:
	BOOL create_window(CWnd* p_parent);
	BOOL create_grid();
	SpikeClassGridList* get_property_list() { return &property_list_; }
	SpikeClassGridToolBar* get_property_toolbar() { return &property_toolbar_; }
	void update_list(SpikeList* p_spike_list);

protected:
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


	DECLARE_MESSAGE_MAP()
};



