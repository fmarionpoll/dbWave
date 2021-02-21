#pragma once
#include "dbWaveDoc.h"
#include "Acqdatad.h"
#include "ChartData.h"
#include "Editctrl.h"

/////////////////////////////////////////////////////////////////////////////
// CMeasureHZtagsPage dialog

class CMeasureHZtagsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureHZtagsPage)

	// Construction
public:
	CMeasureHZtagsPage();
	~CMeasureHZtagsPage();

	// Dialog Data
	enum { IDD = IDD_PROPPAGE3 };
	int		m_datachannel;
	int		m_index;
	float	m_mvlevel;
	int		m_nbcursors;

	// input parameters
	OPTIONS_VIEWDATAMEASURE* m_pMO;
	CdbWaveDoc*		m_pdbDoc;
	CAcqDataDoc*	m_pdatDoc;
	CChartDataWnd*	m_pChartDataWnd;

	// locals
	CEditCtrl mm_index;
	CEditCtrl mm_datachannel;
	CEditCtrl mm_mvlevel;

	// Overrides
public:
	virtual void OnCancel();
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

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
	virtual BOOL OnInitDialog();
	afx_msg void OnDeleteAll();
	DECLARE_MESSAGE_MAP()
};
