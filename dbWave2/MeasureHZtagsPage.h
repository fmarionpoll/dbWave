#pragma once
#include "dbWaveDoc.h"
#include "AcqDataDoc.h"
#include "Editctrl.h"
#include "OPTIONS_VIEW_DATA_MEASURE.h"


class CMeasureHZtagsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureHZtagsPage)

	// Construction
public:
	CMeasureHZtagsPage();
	~CMeasureHZtagsPage() override;

	// Dialog Data
	enum { IDD = IDD_PROPPAGE3 };

	int m_datachannel;
	int m_index;
	float m_mvlevel;
	int m_nbcursors;

	// input parameters
	OPTIONS_VIEW_DATA_MEASURE* m_pMO;
	CdbWaveDoc* m_pdbDoc;
	AcqDataDoc* m_pdatDoc;
	ChartData* m_pChartDataWnd;

	// locals
	CEditCtrl mm_index;
	CEditCtrl mm_datachannel;
	CEditCtrl mm_mvlevel;

	// Overrides
public:
	void OnCancel() override;
	void OnOK() override;
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	BOOL GetHZcursorVal(int index);

	// Implementation
protected:
	// Generated message map functions
	afx_msg void OnCenter();
	afx_msg void OnAdjust();
	afx_msg void OnRemove();
	afx_msg void OnEnChangeDatachannel();
	afx_msg void OnEnChangeIndex();
	afx_msg void OnEnChangeMvlevel();
	BOOL OnInitDialog() override;
	afx_msg void OnDeleteAll();

	DECLARE_MESSAGE_MAP()
};
