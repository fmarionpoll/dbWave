#pragma once
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "OPTIONS_MEASURE.h"

class CMeasureResultsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureResultsPage)

	// Construction
public:
	CMeasureResultsPage();
	~CMeasureResultsPage() override;

	// Dialog Data
	enum { IDD = IDD_PROPPAGE4 };

	CEdit m_c_edit_results;
	CListCtrl m_list_results;

	// input parameters
	ChartData* m_p_chart_data_wnd;
	CdbWaveDoc* m_pdb_doc;
	AcqDataDoc* m_p_dat_doc{};
	options_measure* m_p_options_measure{};
	int m_current_chan{};

protected:
	// locals
	BOOL measure_parameters();
	void measure_from_vertical_tags(int channel);
	void measure_from_horizontal_cursors(int i_chan);
	void measure_from_rectangle(int i_chan);
	void measure_from_stimulus(int i_chan);
	void output_title();
	void measure_within_interval(int channel, int line, long l1, long l2);
	void measure_between_hz(int channel, int line, int v1, int v2);
	void get_max_min(int channel, long l_first, long l_last);

	short m_max_{};
	long m_imax_{};
	short m_min_{};
	long i_min_{};
	short m_first_{};
	short m_last_{};
	TCHAR m_sz_t_[64]{}; // dummy characters buffer
	float m_mv_per_bin_{};
	int m_nb_data_cols_{};
	int m_col_{};
	CString m_cs_title_;

	// Overrides
public:
	void OnOK() override;
	BOOL OnSetActive() override;
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	afx_msg void on_export();
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
