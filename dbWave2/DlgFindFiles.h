#pragma once
#include "afxeditbrowsectrl.h"
#include "dbWaveDoc.h"

class DlgFindFiles : public CDialog
{
	// Construction
public:
	DlgFindFiles(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_FINDFILES };

	CComboBox m_file_ext;
	CString m_path{ _T("")};
	CMFCEditBrowseCtrl m_mfc_browse_control;
	int m_n_found{ 0 };
	int m_sel_init{ 0 };
	CStringArray* m_p_file_names{ nullptr };
	BOOL m_i_option{ 0 };
	BOOL m_b_any_format{ false };
	CdbWaveDoc* m_pdbDoc{ nullptr };

	// Overrides
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	CString m_search_string_;
	BOOL m_b_subtree_search_{ false };
	CStringArray m_p_path_;
	CString m_cs_root_search_;

	void traverse_directory(const CString& path);
	void display_n_found() const;
	void find_files(const CString& path);

	// Generated message map functions
	void OnOK() override;
	BOOL OnInitDialog() override;

	afx_msg void on_search();

	DECLARE_MESSAGE_MAP()
};
