#pragma once
#include "dbWaveDoc.h"

#define GERT		 0
#define ASCIISYNTECH 1
#define ATFFILE		 2

// CImportFilesDlg dialog

class DlgImportFiles : public CDialog
{
	DECLARE_DYNAMIC(DlgImportFiles)

public:
	DlgImportFiles(CWnd* p_parent = nullptr);
	~DlgImportFiles() override;

	// Dialog Data
	enum { IDD = IDD_IMPORTFILES };

	// values passed by caller
	CStringArray* m_p_file_name_array{ nullptr };
	CStringArray* m_p_converted_files{ nullptr };
	CdbWaveDoc* m_pdb_doc{ nullptr };
	int		m_option{ 0 };
	BOOL	m_b_read_header{ true };

	// Implementation
protected:
	BOOL	m_b_convert_{ false };
	CString m_ext_{};
	int		m_n_current_{ 0 };
	int		m_n_files_{ 0 };
	CString m_file_from_{};
	CString m_file_to_{};

	int		m_scan_count_{ 0 };
	double	m_x_inst_gain_{ 0. };
	double	m_x_rate_{ 0. };
	double	m_d_span_[16]{};
	double	m_d_bin_val_[16]{};

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support
	BOOL OnInitDialog() override;
	void update_dlg_items();
	BOOL get_acquisition_parameters(const AcqDataDoc* p_to);
	BOOL get_experiment_parameters(const AcqDataDoc* p_to) const;
	BOOL import_atf_file();

public:
	afx_msg void OnDestroy();
	afx_msg void on_bn_clicked_cancel();
	afx_msg void adc_on_bn_clicked_start_stop();

	DECLARE_MESSAGE_MAP()
};
