#pragma once
#include "ChartData.h"


class DlgDataViewOrdinates : public CDialog
{
public:
	DlgDataViewOrdinates(CWnd* p_parent = nullptr); 

	// Dialog Data
	enum { IDD = IDD_ORDINATES };

	CComboBox m_chan_select;
	int m_i_unit{ -1 };
	float m_x_center{ 0.f };
	float m_x_max{ 0.f };
	float m_x_min{ 0.f };

	ChartData* m_p_chart_data_wnd{ nullptr }; 
	int m_n_chan_max{ 0 };
	int m_channel{ 0 }; 
	BOOL m_b_changed{ false }; 

protected:
	float m_p10_{ 0.f }; 
	float m_volts_per_pixel_{ 0.f }; 
	float m_volts_per_bin_{ 0.f }; 
	CWordArray m_settings_; 

	// Implementation
protected:
	void load_chan_list_data(int i_list); 
	void save_chan_list_data(int index_list) const; 
	void change_units(int new_scale, BOOL b_new); 
	void DoDataExchange(CDataExchange* p_dx) override;

	// Generated message map functions
	void OnOK() override;
	BOOL OnInitDialog() override;
	afx_msg void on_sel_change_vert_units();
	afx_msg void on_sel_change_chan_select();
	afx_msg void on_kill_focus_vert_mx_mi();
	void OnCancel() override;
	afx_msg void on_kill_focus_vert_center();

	DECLARE_MESSAGE_MAP()
};
