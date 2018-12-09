#pragma once

// dbWaveView.h : interface of the CdbWaveView class
//
/////////////////////////////////////////////////////////////////////////////

#include "DataListCtrl.h"
#include "Editctrl.h"

//class CdbWaveCntrItem;
class CdbMainTable;
class CLineViewWnd;
class CSpikeBarWnd;
class CSpikeShapeWnd;
class CDataListCtrl;

class CdbWaveView : public CDaoRecordView
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CdbWaveView)
	CdbWaveView();

public:
	enum { IDD = IDD_VIEWDBWAVE };
	CdbMainTable* m_pSet;

// Attributes
public:
	CdbWaveDoc*		GetDocument();
	CDataListCtrl	m_dataListCtrl;
	BOOL			m_bvalidDat;
	BOOL			m_bvalidSpk;

// Operations
	
// Overrides
public:
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
protected:
	virtual void DoDataExchange(CDataExchange* pDX); 
	virtual void OnInitialUpdate(); 
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

// Implementation
public:
	virtual ~CdbWaveView();

#ifdef _DEBUG
	virtual void		AssertValid() const;
	virtual void		Dump(CDumpContext& dc) const;
#endif

protected:
	BOOL				m_bAddMode;
	BOOL				m_bFilterON;
	int					m_dattransform;
	CEditCtrl			mm_timefirst;		// first abcissa value
	CEditCtrl			mm_timelast;		// last abcissa value
	CEditCtrl			mm_amplitudespan;	// amplitude
	float				m_timefirst;
	float				m_timelast;
	float				m_amplitudespan;
	CEditCtrl			mm_spikeclass;		// selected spike class
	int					m_spikeclass;
	CStretchControl		m_stretch;			// properties for controls
	BOOL				m_binit;
	OPTIONS_VIEWDATA*	mdPM;
	CTabCtrl			m_tabCtrl;

	// internal functions
	void		UpdateControls();
	void		FillListBox();
public:
	void		DeleteRecords();
	void		InitctrlTab();
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
	afx_msg void OnLvnColumnclickListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnEnChangeAmplitudespan();
	afx_msg void OnBnClickedCheckfilename();
	afx_msg void OnHdnEndtrackListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedRadioallclasses();
	afx_msg void OnBnClickedRadiooneclass();
	afx_msg void OnEnChangeSpikeclass();
	
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in dbWaveView.cpp
	inline CdbWaveDoc* CdbWaveView::GetDocument()
	{ 
		return (CdbWaveDoc*)m_pDocument; 
	}
#endif
