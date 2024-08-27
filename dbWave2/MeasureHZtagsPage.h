#pragma once
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "Editctrl.h"
#include "OPTIONS_MEASURE.h"


class CMeasureHZtagsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureHZtagsPage)

	// Construction
public:
	CMeasureHZtagsPage();
	~CMeasureHZtagsPage() override;

	// Dialog Data
	enum { IDD = IDD_PROPPAGE3 };

	int m_data_channel {0};
	int m_index {0};
	float m_mv_level {0.0f};
	int m_nb_cursors{0};

	// input parameters
	options_measure* m_p_options_measure{nullptr};
	CdbWaveDoc* m_pdb_doc{nullptr};
	AcqDataDoc* m_p_dat_doc{nullptr};
	ChartData* m_p_chart_data_wnd{nullptr};

	// locals
	CEditCtrl mm_index;
	CEditCtrl mm_data_channel;
	CEditCtrl mm_mv_level;

	// Overrides
	void OnCancel() override;
	void OnOK() override;
	BOOL OnInitDialog() override;

protected:
	void DoDataExchange(CDataExchange* p_dx) override;
	BOOL get_hz_cursor_value(int index);

	// Generated message map functions
	afx_msg void on_center();
	afx_msg void on_adjust();
	afx_msg void on_remove();
	afx_msg void on_en_change_data_channel();
	afx_msg void on_en_change_index();
	afx_msg void on_en_change_mv_level();
	afx_msg void on_delete_all();

	DECLARE_MESSAGE_MAP()
};
