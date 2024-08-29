#pragma once
#include "ChartData.h"

class DlgDataSeries : public CDialog
{
	// Construction
public:
	DlgDataSeries(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DATASERIES };

	CButton m_delete_series;
	CButton m_define_series;
	CComboBox m_transform;
	CComboBox m_ordinates;
	CListBox m_list_series;

	CString m_name{ _T("")};
	int m_i_span{ 0 };
	ChartData* m_p_chart_data_wnd{ nullptr}; 
	AcqDataDoc* m_pdb_doc{ nullptr }; 
	int m_list_index{ 0 };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;
	afx_msg void on_sel_change_list_series();
	afx_msg void on_clicked_delete_series();
	afx_msg void on_clicked_define_series();
	afx_msg void on_sel_change_transform();
	afx_msg void on_en_change_edit1();

	DECLARE_MESSAGE_MAP()
};
