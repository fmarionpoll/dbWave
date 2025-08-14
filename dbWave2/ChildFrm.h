#pragma once

class CChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	void ActivateFrame(int n_cmd_show = -1) override;
	~CChildFrame() override;

	UINT m_view_on =  ID_VIEW_DATABASE;
	int m_previous_view_on = ID_VIEW_DATABASE ;
	int m_cursor_state = 0;
	int m_n_status = 0;

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
protected:
	BOOL m_b_delete_file_{ false };
	BOOL m_b_keep_choice_{ false };

	void replace_view(CRuntimeClass* p_view_class, HMENU h_menu);
	void on_tools_import_files(int i_filter);
	void export_ascii(int option);
	BOOL export_to_excel();
	BOOL export_to_excel_and_build_pivot(int option);
	static void build_excel_pivot(void* po_app, void* p_odata_sheet, CString cs_source_data_address, CString cs_name_sheet,
	                              short xl_consolidation_function, int col2);

protected:
	afx_msg void on_view_cursor_mode_normal();
	afx_msg void on_update_view_cursor_mode_normal(CCmdUI* p_cmd_ui);
	afx_msg void on_view_cursor_mode_measure();
	afx_msg void on_update_view_cursor_mode_measure(CCmdUI* p_cmd_ui);
	afx_msg void on_view_cursor_mode_zoom_in();
	afx_msg void on_update_view_cursor_mode_zoom_in(CCmdUI* p_cmd_ui);

	afx_msg void on_options_browse_mode();
	afx_msg void on_options_print_margins();
	afx_msg void on_options_load_save_options();
	afx_msg void on_tools_export_data_comments();
	afx_msg void on_tools_export_data_as_text();
	afx_msg void on_tools_export_number_of_spikes();
	afx_msg LRESULT on_my_message(WPARAM w_param, LPARAM l_param);
	afx_msg void replace_view_index(UINT n_id);
	afx_msg void on_update_view_menu(CCmdUI* p_cmd_ui);

public:
	afx_msg void on_record_goto();
	afx_msg void on_record_delete();
	afx_msg void on_record_add();
	afx_msg void on_tools_remove_missing_files();
	afx_msg void on_tools_remove_duplicate_files();
	afx_msg void on_tools_restore_deleted_files();
	afx_msg void on_tools_synchronize_source_information_current_file();
	afx_msg void on_tools_synchronize_source_information_all_files();
	afx_msg void on_tools_remove_artefact_files();
	afx_msg void on_tools_check_file_lists_consistency();
	afx_msg void on_tools_import_atl_files();
	//afx_msg void On_Tools_Import_Syntech_aspk_files();
	afx_msg void on_tools_import_data_files();
	afx_msg void on_tools_import_spike_files();
	afx_msg void on_tools_import_database();
	afx_msg void on_tools_copy_all_project_files();
	afx_msg void on_tools_export_data_file();
	afx_msg void on_mdi_activate(BOOL b_activate, CWnd* p_activate_wnd, CWnd* p_deactivate_wnd);
	afx_msg void on_tools_paths_relative();
	afx_msg void on_tools_paths_absolute();
	afx_msg void on_tools_path();
	afx_msg void on_tools_remove_unused();
	afx_msg void on_tools_import();
	afx_msg void on_tools_synchro();
	afx_msg void on_tools_garbage();
	afx_msg void on_tools_compact_database();
	afx_msg void on_tools_cleanup_filenames();

	DECLARE_MESSAGE_MAP()
};
