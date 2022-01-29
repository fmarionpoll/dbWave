#pragma once

class CDataSeriesDlg : public CDialog
{
	// Construction
public:
	CDataSeriesDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DATASERIES };

	CButton m_deleteseries;
	CButton m_defineseries;
	CComboBox m_transform;
	CComboBox m_ordinates;
	CListBox m_listseries;
	CString m_name;
	int m_ispan;
	ChartData* m_pChartDataWnd; // data contours
	AcqDataDoc* m_pdbDoc; // data document
	int m_listindex; // same as lineview:Chanlist

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
