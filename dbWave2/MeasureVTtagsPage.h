#pragma once
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "Editctrl.h"
#include "OPTIONS_MEASURE.h"


class CMeasureVTtagsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureVTtagsPage)

	// Construction
public:
	CMeasureVTtagsPage();
	~CMeasureVTtagsPage() override;

	// Dialog Data
	enum { IDD = IDD_PROPPAGE2 };

	int m_index {0};
	int m_nb_tags {0};
	int m_n_periods {0};
	float m_period {0};
	float m_time_sec {0.0f};
	float m_time_shift {0.0f};
	float m_duration {0.0f};

	// input parameters
	options_measure* m_p_options_measure {nullptr}; 
	CdbWaveDoc* m_pdb_doc {nullptr};
	AcqDataDoc* m_p_dat_doc {nullptr};
	ChartData* m_p_chart_data_wnd {nullptr};
	float m_sampling_rate {0.0f};
	float m_very_last {0.0f};

	// locals
	CEditCtrl mm_index;
	CEditCtrl mm_file_index;
	CEditCtrl mm_time_sec;
	CEditCtrl mm_duration;
	CEditCtrl mm_period;
	CEditCtrl mm_n_periods;
	CEditCtrl mm_time_shift;

	// Overrides
public:
	void OnCancel() override;
	void OnOK() override;
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
	BOOL get_vt_tag_value(int index);
	void set_spaced_tags_options() const;

	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void on_remove();
	afx_msg void on_en_change_item();
	afx_msg void on_en_change_time_sec();
	afx_msg void on_check1();
	afx_msg void on_set_duplicate_mode();
	afx_msg void on_en_change_duration();
	afx_msg void on_en_change_period();
	afx_msg void on_en_change_n_periods_edit();
	afx_msg void on_en_change_time_shift();
	afx_msg void on_shift_tags();
	afx_msg void on_add_tags();
	afx_msg void on_delete_series();
	afx_msg void on_delete_all();

	DECLARE_MESSAGE_MAP()
};
