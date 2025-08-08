#pragma once

// CdbEditRecordDlg dialog

class CdbWaveDoc;
class CdbTableMain;

class DlgdbEditRecord : public CDialog
{
	DECLARE_DYNAMIC(DlgdbEditRecord)

public:
	DlgdbEditRecord(CWnd* p_parent = nullptr); 
	~DlgdbEditRecord() override;

	CdbWaveDoc* m_pdb_doc{ nullptr };
	CdbTableMain* m_p_set{ nullptr };
	BOOL show_idc_next{ true };
	BOOL show_idc_previous{ true };

	// Dialog Data
	enum { IDD = IDD_EDITRECORD };

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support
	void populate_controls();
	static void populate_combo_with_text(CDaoRecordset& linked_table_set, CComboBox& combo, int i_id);
	static void populate_combo_with_numbers(CComboBox& combo, CArray<long, long>* p_id_array, const long& l_var);
	void update_set_from_combo(CDaoRecordset& linked_table_set, CComboBox& combo, long& id_set) const;
	void edit_change_item_indirect_field(int idc);
	void edit_change_item_main_field(int idc);
	DB_ITEMDESC* get_item_descriptors(int idc);
	void update_database_from_dialog();

public:
	BOOL OnInitDialog() override;
protected:
	void OnOK() override;
public:
	CComboBox m_ctl_experiment;
	CComboBox m_ctl_insect_id;
	CComboBox m_ctl_sensillum_id;
	CComboBox m_ctl_stimulus;
	CComboBox m_ctl_concentration;
	CComboBox m_ctl_stimulus2;
	CComboBox m_ctl_concentration2;
	CComboBox m_ctl_insect;
	CComboBox m_ctl_sensillum;
	CComboBox m_ctl_location;
	CComboBox m_ctl_operator;
	CComboBox m_ctl_path_dat;
	CComboBox m_ctl_path_spk;
	CComboBox m_ctl_strain;
	CComboBox m_ctl_sex;
	CComboBox m_ctl_repeat;
	CComboBox m_ctl_repeat2;
	CComboBox m_ctl_flag;

	CString m_cs_more{ _T("") };
	CString m_cs_name_dat{ _T("") };
	CString m_cs_name_spk{ _T("") };

	afx_msg void on_bn_clicked_button_insect_id();
	afx_msg void on_bn_clicked_button_sensillum_id();
	afx_msg void on_bn_clicked_button_stimulus();
	afx_msg void on_bn_clicked_button_concentration();
	afx_msg void on_bn_clicked_button_stimulus2();
	afx_msg void on_bn_clicked_button_concentration2();
	afx_msg void on_bn_clicked_button_insect_name();
	afx_msg void on_bn_clicked_button_strain();
	afx_msg void on_bn_clicked_button_sex();
	afx_msg void on_bn_clicked_button_sensillum();
	afx_msg void on_bn_clicked_button_location();
	afx_msg void on_bn_clicked_button_operator();
	afx_msg void on_bn_clicked_button5();
	afx_msg void on_bn_clicked_button1();
	afx_msg void on_bn_clicked_synchro_single();
	afx_msg void on_bn_clicked_synchro_all();
	afx_msg void on_bn_clicked_previous();
	afx_msg void on_bn_clicked_next();
	afx_msg void on_bn_clicked_button_repeat();
	afx_msg void on_bn_clicked_button_repeat2();
	afx_msg void on_bn_clicked_button_flag();
	afx_msg void on_bn_clicked_button_expt2();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
};
