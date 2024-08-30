#pragma once

#include "Editctrl.h"
#include "ChartData.h"
#include "AcqDataDoc.h"
#include "options_import.h"

class DlgImportGenericData : public CDialog
{
	friend class AcqDataDoc;

	// Construction
public:
	DlgImportGenericData(CWnd* p_parent = nullptr);

	options_import* options_import{ nullptr };
	BOOL b_convert{ false };
	CStringArray* m_p_file_name_array{ nullptr };
	CEditCtrl mm_nb_ad_channels;
	CEditCtrl mm_ad_channel_chan;
	CEditCtrl mm_skip_n_bytes;
	AcqDataDoc m_acq_data_file;
	ChartData m_chart_data_wnd;
	CString m_file_source;
	CString m_file_dest;
	CString m_file_old{ _T("")};
	BOOL m_b_changed{ false };
	BOOL m_b_import_all{ false };

	// Dialog Data
	enum { IDD = IDD_IMPORTGENERICDATAFILE };

	CComboBox m_file_drop_list;
	CComboBox m_combo_precision;
	CString m_ad_channel_comment{ _T("")};
	float m_ad_channel_gain{ 0.f };
	UINT m_ad_channel_chan{ 0 };
	UINT m_nb_ad_channels{ 0 };
	UINT m_nb_runs{ 0 };
	float m_sampling_rate{ 0.f };
	UINT m_skip_n_bytes{ 0 };
	float m_voltage_max{ 0.f };
	float m_voltage_min{ 0.f };
	CString m_cs_file_title{ _T("")};
	BOOL m_b_preview_on{ false };

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV supportTUAL

	// Implementation
protected:
	void update_controls_from_struct();
	void update_struct_from_controls();
	void enable_run_parameters();
	void update_preview();
	void update_wave_descriptors(const AcqDataDoc* p_df);
	void set_file_names(int index);

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void on_sel_change_precision();
	afx_msg void on_en_change_number_of_channels();
	afx_msg void on_multiple_runs();
	afx_msg void on_single_run();
	afx_msg void on_en_change_channel_no();
	afx_msg void on_delta_pos_spin_nb_channels(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_delta_pos_spin_no_chan(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_import();
	afx_msg void on_import_all();
	afx_msg void on_set_preview();
	afx_msg void on_sel_change_file_list();
	afx_msg void on_offset_binary();
	afx_msg void on_twos_complement();
	afx_msg void on_en_change_skip_n_bytes();
	afx_msg void on_delta_pos_skip_n_bytes(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_sapid3_5();
	afx_msg void on_en_change_channel_comment();

	DECLARE_MESSAGE_MAP()
};
