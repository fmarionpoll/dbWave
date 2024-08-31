#pragma once

#include "AcqDataDoc.h"
#include "afxcolorbutton.h"
#include "ChartData.h"


class DlgDataSeriesFormat : public CDialog
{
	// Construction
public:
	DlgDataSeriesFormat(CWnd* p_parent = nullptr); 

	// Dialog Data
	enum { IDD = IDD_DATASERIESFORMAT };

	CListBox m_list_series;
	float m_max_mv { 0.f };
	float m_min_mv { 0.f };
	ChartData* m_p_chart_data_wnd{ nullptr };
	AcqDataDoc* m_pdb_doc{nullptr }; 
	int m_list_index {0};
	int m_y_zero{ 0 };
	int m_y_extent{ 0 };
	float m_mv_per_bin{ 0.f };
	int m_bin_zero{ 2048 };
	CMFCColorButton m_color_button;

protected:
	CPalette* m_p_palette_ {nullptr};

	void get_params(int index);
	void set_params(int index);
	void define_custom_palette();
	void init_colors_button();
	void init_color_button(int i_color);

	// Generated message map functions
	void DoDataExchange(CDataExchange* p_dx) override;
	void OnOK() override;
	void OnCancel() override;
	BOOL OnInitDialog() override;
	afx_msg void on_sel_change_list_series();

public:
	afx_msg void on_bn_clicked_mfc_color_button1();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
