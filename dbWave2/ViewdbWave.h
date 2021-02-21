#pragma once

#include "DataListCtrl.h"
#include "dbMainTable.h"
#include "ChartData.h"
#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "Editctrl.h"
#include "CSpkListTabCtrl.h"

//class CdbWaveCntrItem;
//class CdbMainTable;
//class CLineViewWnd;
//class CChartSpikeBarWnd;
//class CChartSpikeShapeWnd;

class CViewdbWave : public CDaoRecordView
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CViewdbWave)
	CViewdbWave();

public:
	enum { IDD = IDD_VIEWDBWAVE };
	CdbMainTable*	m_pSet;

	// Attributes
public:
	CDataListCtrl	m_dataListCtrl;
	BOOL			m_bvalidDat;
	BOOL			m_bvalidSpk;
	CdbWaveDoc*		GetDocument();
	CSpkListTabCtrl m_tabCtrl;

	// Overrides
public:
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnInitialUpdate();
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* p_dc, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

	// Implementation
public:
	virtual ~CViewdbWave();

#ifdef _DEBUG
	virtual void		AssertValid() const;
	virtual void		Dump(CDumpContext& dc) const;
#endif

protected:
	BOOL				m_binit;
	BOOL				m_bAddMode;
	BOOL				m_bFilterON;
	int					m_dattransform;
	CEditCtrl			mm_spikeclass;		// selected spike class
	float				m_timefirst;
	float				m_timelast;
	float				m_amplitudespan;
	int					m_spikeclass;
	CEditCtrl			mm_timefirst;		// first abcissa value
	CEditCtrl			mm_timelast;		// last abcissa value
	CEditCtrl			mm_amplitudespan;	// amplitude
	CStretchControl		m_stretch;			// properties for controls
	OPTIONS_VIEWDATA*	m_options_viewdata;


	// internal functions
	void		UpdateControls();
	void		FillListBox();
public:
	void		DeleteRecords();
	
	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRecordPageup();
	afx_msg void OnRecordPagedown();
	afx_msg void OnClickMedianFilter();
	afx_msg void OnItemActivateListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListctrl(NMHDR* pNMHDR, LRESULT* pResult);
public:
	afx_msg void OnLvnColumnclickListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedDisplaySpikes();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnEnChangeAmplitudespan();
	afx_msg void OnBnClickedCheckfilename();
	afx_msg void OnHdnEndtrackListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedRadioallclasses();
	afx_msg void OnBnClickedRadiooneclass();
	afx_msg void OnEnChangeSpikeclass();

	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult);
};

#ifndef _DEBUG  // debug version in dbWaveView.cpp
inline CdbWaveDoc* CViewdbWave::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif
