#pragma once


class DlgExportData : public CDialog
{
	// Construction
public:
	DlgExportData(CWnd* p_parent = nullptr);
	BOOL DestroyWindow() override;

	options_import iiv_o;
	CdbWaveDoc* m_db_doc {nullptr};
	CString m_file_source;
	CString m_file_dest;
	CString m_file_temp;
	int m_i_current_file{ -1 };
	int m_index_old_selected_file {0};
	BOOL m_b_all_files{ true };

	long mm_l_first {0};
	long mm_l_last { 0 };
	float mm_time_first{ 0 };
	float mm_time_last{ 0 };
	int mm_first_chan{ 0 };
	int mm_last_chan{ 0 };
	int mm_bin_zero{ 0 };
	AcqDataDoc* m_p_dat{ nullptr };

	// Dialog Data
	enum { IDD = IDD_EXPORTDATAFILE };

	CComboBox m_combo_export_as;
	CComboBox m_file_drop_list;
	float m_time_first{ 0.f };
	float m_time_last{ 0.f };
	int m_channel_number{ 0 };
	int m_i_under_sample{ 1 };


protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void update_struct_from_controls();
	void Export();
	BOOL export_data_as_db_wave_file();
	BOOL export_data_as_text_file();
	BOOL export_data_as_sapid_file();
	BOOL export_data_as_excel_file();
	static void save_biff(CFile* fp, int type, int row, int col, const char* data);
	static void save_c_string_biff(CFile* fp, int row, int col, const CString& data);

	BOOL OnInitDialog() override;
	afx_msg void on_single_channel();
	afx_msg void on_all_channels();
	void OnOK() override;
	afx_msg void on_entire_file();
	afx_msg void on_sel_change_export_as();
	afx_msg void on_export();
	afx_msg void on_export_all();

	DECLARE_MESSAGE_MAP()
};
