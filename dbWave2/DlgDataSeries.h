#pragma once
#include "ChartData.h"

class DlgDataSeries : public CDialog
{
	// Construction
public:
	DlgDataSeries(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DATASERIES };

	CButton m_deleteseries;
	CButton m_defineseries;
	CComboBox m_transform;
	CComboBox m_ordinates;
	CListBox m_listseries;

	CString m_name{ _T("")};
	int m_ispan{ 0 };
	ChartData* m_pChartDataWnd{ nullptr}; 
	AcqDataDoc* m_pdbDoc{ nullptr }; 
	int m_listindex{ 0 };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void OnSelchangeListseries();
	afx_msg void OnClickedDeleteseries();
	afx_msg void OnClickedDefineseries();
	void OnOK() override;
	void OnCancel() override;
	afx_msg void OnSelchangeTransform();
	afx_msg void OnEnChangeEdit1();

	DECLARE_MESSAGE_MAP()
};
