#pragma once
#include "TemplateListWnd.h"

// SpikeTemplateView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeTemplates form view

class CViewSpikeTemplates : public CDaoRecordView
{
protected:
	CViewSpikeTemplates();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewSpikeTemplates)

	// Form Data
public:

	enum { IDD = IDD_VIEWSPKTEMPLATES };
	float	m_timefirst;
	float	m_timelast;
	int		m_hitrate;
	float	m_ktolerance;
	int		m_spikenoclass;
	int		m_hitratesort;
	int		m_ifirstsortedclass;
	BOOL	m_ballfiles;

	BOOL	m_ballTempl;
	BOOL	m_ballSort;
	CTabCtrl m_tab1Ctrl;

	CdbWaveDoc* GetDocument();

protected:
	CEditCtrl	mm_hitrate;
	CEditCtrl	mm_hitratesort;
	CEditCtrl	mm_ktolerance;
	CEditCtrl	mm_spikenoclass;
	CEditCtrl	mm_timefirst;			// first abcissa value
	CEditCtrl	mm_timelast;			// last abcissa value
	CEditCtrl	mm_ifirstsortedclass;

	CTemplateListWnd	m_avgList;
	CTemplateListWnd	m_templList;
	CTemplateListWnd	m_avgAllList;

	CSpikeShapeWnd		m_spkForm;		// all spikes in displayspikes
	CStretchControl		m_stretch;		// array of properties associated with controls
	BOOL				m_binit;

	CSpikeDoc* m_pSpkDoc;			// destination data doc
	CSpikeList* m_pSpkList;			// temporary spike list
	int				m_lFirst;
	int				m_lLast;
	OPTIONS_VIEWDATA* mdPM;	// view data options
	OPTIONS_VIEWDATAMEASURE* mdMO;	// measure options
	SPKCLASSIF* m_psC;				// sort parameters
	int				m_ktagleft;			// VT tags
	int				m_ktagright;
	SCROLLINFO		m_scrollFilePos_infos;
	int				m_spikeno;
	BOOL			m_ballclasses;

	// Attributes
public:
	inline void SetViewMouseCursor(int cursormode) { m_spkForm.SetMouseCursorType(cursormode); }

	// Operations
public:

	// Overrides
public:
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

	// Implementation
protected:
	virtual ~CViewSpikeTemplates();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void SaveCurrentSpkFile();
	void UpdateFileParameters();		// reset parameters for new file
	void UpdateTemplates();
	void UpdateLegends();
	void SelectSpike(short spikeno);
	void UpdateScrollBar();
	void SelectSpikeList(int icur);
	void EditSpikeClass(int controlID, int controlItem);
	void DisplayAvg(BOOL ballfiles, CTemplateListWnd* pTPList); //, CImageList* pImList);
	void SortSpikes();
	void UpdateCtrlTab1(int iselect);

public:
	// Generated message map functions
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeclassno();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnFormatCentercurve();
	afx_msg void OnBuildTemplates();
	afx_msg void OnEnChangeHitrate();
	afx_msg void OnEnChangeTolerance();
	afx_msg void Onallclasses();
	afx_msg void Onsingleclass();
	afx_msg void OnEnChangeHitrateSort();
	afx_msg void OnKeydownTemplateList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheck1();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonClickedTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSort();
	afx_msg void OnBnClickedDisplay();
	afx_msg void OnEnChangeIfirstsortedclass();
	CTabCtrl m_tabCtrl;
	afx_msg void OnTcnSelchangeTab2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTab2(NMHDR* pNMHDR, LRESULT* pResult);
};

/////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* CViewSpikeTemplates::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif
