#pragma once
#include "dbWaveDoc.h"
#include "Acqdatad.h"
#include "ChartData.h"
#include "Editctrl.h"

/////////////////////////////////////////////////////////////////////////////
// CMeasureVTtagsPage dialog

class CMeasureVTtagsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureVTtagsPage)

	// Construction
public:
	CMeasureVTtagsPage();
	~CMeasureVTtagsPage();

	// Dialog Data
	enum { IDD = IDD_PROPPAGE2 };
	int		m_index;
	int		m_nbtags;
	int		m_nperiods;
	float	m_period;
	float	m_timesec;
	float	m_timeshift;
	float	m_duration;

	// input parameters
	OPTIONS_VIEWDATAMEASURE* m_pMO;	// init on call
	CdbWaveDoc*		m_pdbDoc;
	CAcqDataDoc*	m_pdatDoc;
	CChartDataWnd*	m_pChartDataWnd;
	float	m_samplingrate;
	float	m_verylast;

	// locals
	CEditCtrl mm_index;
	CEditCtrl mm_fileindex;
	CEditCtrl mm_timesec;
	CEditCtrl mm_duration;
	CEditCtrl mm_period;
	CEditCtrl mm_nperiods;
	CEditCtrl mm_timeshift;

	// Overrides
public:
	virtual void OnCancel();
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	BOOL GetVTtagVal(int index);
	void SetspacedTagsOptions();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnRemove();
	afx_msg void OnEnChangeItem();
	afx_msg void OnEnChangeTimesec();
	afx_msg void OnCheck1();
	afx_msg void OnSetDuplicateMode();
	afx_msg void OnEnChangeDuration();
	afx_msg void OnEnChangePeriod();
	afx_msg void OnEnChangeNperiodsedit();
	afx_msg void OnEnChangeTimeshift();
	afx_msg void OnShiftTags();
	afx_msg void OnAddTags();
	afx_msg void OnDeleteSeries();
	afx_msg void OnDeleteAll();
	DECLARE_MESSAGE_MAP()
};
