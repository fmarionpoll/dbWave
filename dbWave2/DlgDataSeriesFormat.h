#pragma once

#include "AcqDataDoc.h"
#include "afxcolorbutton.h"
#include "ChartData.h"


class DlgDataSeriesFormat : public CDialog
{
	// Construction
public:
	DlgDataSeriesFormat(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_DATASERIESFORMAT };

	CListBox m_listseries;
	float m_maxmv { 0.f };
	float m_minmv { 0.f };
	ChartData* m_pChartDataWnd{ nullptr };
	AcqDataDoc* m_pdbDoc{nullptr }; 
	int m_listindex {0};
	int m_yzero{ 0 };
	int m_yextent{ 0 };
	float m_mVperbin{ 0.f };
	int m_binzero{ 2048 };
	CMFCColorButton m_colorbutton;

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	void GetParams(int index);
	void SetParams(int index);

	// Implementation
protected:
	// Generated message map functions
	void OnOK() override;
	void OnCancel() override;
	BOOL OnInitDialog() override;
	afx_msg void OnSelchangeListseries();

	DECLARE_MESSAGE_MAP()
};
