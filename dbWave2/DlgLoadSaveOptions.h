#pragma once


class DlgLoadSaveOptions : public CDialog
{
	// Construction
public:
	DlgLoadSaveOptions(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_OPTIONSLOADSAVE };

	CComboBox m_cb_name_list;
	CString m_ddx_comment{ _T("") };
	CStringArray* p_parameter_files{ nullptr };
	CString* p_comment{ nullptr };

	CStringArray p_files;
	CString m_current_file;
	int m_current_selected{ 0 };
	BOOL m_b_file_name_changed{ false };
	BOOL m_b_comment_changed{ false };

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	int parameter_find_string(const CString& filename) const;
	void update_file_list();

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;
	afx_msg void on_sel_change_name_list();
	afx_msg void on_kill_focus_name_list();
	afx_msg void on_en_change_comment();
	afx_msg void on_edit_change_name_list();
	afx_msg void on_load();
	afx_msg void on_save();
	afx_msg void on_kill_focus_comment();

	DECLARE_MESSAGE_MAP()
};
