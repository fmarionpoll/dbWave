#pragma once

// SpikeHistView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpikeHistView form view

//#include "CustomTabCtrl.h"

class CSpikeHistView : public CDaoRecordView
{
	DECLARE_DYNCREATE(CSpikeHistView)	
protected:
	CSpikeHistView();           // protected constructor used by dynamic creation
	
// Form Data
public:
	enum { IDD = IDD_VIEWSPKTIMESERIES };
	CComboBox	m_cbHistType;
	float	m_timefirst;
	float	m_timelast;
	int		m_spikeclass;
	int		m_dotheight;
	int		m_rowheight;
	float	m_binISIms;
	int		m_nbinsISI;
	float	m_timebinms;

	CdbWaveDoc*	GetDocument();

// Attributes
protected:
	CStretchControl m_stretch;
	BOOL			m_binit;
	CEditCtrl	mm_timebinms;		// bin size (ms)
	CEditCtrl	mm_binISIms;		// bin size (ms)
	CEditCtrl	mm_nbinsISI;		// nbins ISI

	CEditCtrl	mm_timefirst;		// first abcissa value
	CEditCtrl	mm_timelast;		// last abcissa value
	CEditCtrl	mm_spikeclass;		// selected spike class
	CEditCtrl	mm_dotheight;		// dot height
	CEditCtrl	mm_rowheight;		// row height
	OPTIONS_VIEWSPIKES* m_pvdS;		// histogram options
	OPTIONS_VIEWDATA*  mdPM;		// view data options
	int			m_bhistType;
	SCROLLINFO m_scrollFilePos_infos;

	CSpikeDoc* m_pSpkDoc;			// pointer to document

	long*	m_pPSTH;				// histogram data (pointer to array)
	int		m_sizepPSTH;			// nbins within histogram
	long	m_nPSTH;
	long*	m_pISI;
	int		m_sizepISI;
	long	m_nISI;
	long*	m_parrayISI;
	int		m_sizeparrayISI;

	CRect	m_displayRect;			// display area
	CPoint	m_topleft;				// top position of display area
	BOOL	m_initiated;			// flag / initial settings
	BOOL	m_bmodified;			// flag ON-> compute data
	CBitmap* m_pbitmap;				// temp bitmap used to improve display speed
	int		m_nfiles;				// nb of files used to build histogram
	float	t1000;
	BOOL	m_bPrint;
	CRect	m_commentRect;

	LOGFONT	m_logFont;				// onbegin/onendPrinting
	CFont	m_fontPrint;
	int		m_rectratio;
	float	m_xfirst;
	float	m_xlast;

	LOGFONT	m_logFontDisp;			// onbegin/onendPrinting
	CFont	m_fontDisp;				// display font	

public:

// Operations
// public interface to view
public:

// Overrides
	public:
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);   	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);	    	
	virtual void OnDraw(CDC* pDC);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);

protected:
	void BuildData();
	void GetFileInfos(CString &strComment);
	void DisplayHistogram(CDC* pDC, CRect* pRect);
	void DisplayDot(CDC* pDC, CRect* pRect);
	void DisplayPSTHAutoc(CDC* pDC, CRect* pRect);
	void DisplayStim(CDC* pDC, CRect* pRect, long* lFirst, long* lLast);
	void OnDisplay();
	void ShowControls(int iselect);
	void SelectSpkList(int icursel, BOOL bRefreshInterface=FALSE);

// Implementation
protected:
	long PlotHistog(CDC* pDC, CRect* dispRect, int nbins, long* phistog0, int orientation=0, int btype=0);

	virtual ~CSpikeHistView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnEnChangeTimebin();
	afx_msg void OnEnChangebinISI();
	afx_msg void OnClickAllfiles();
	afx_msg void OnabsoluteTime();
	afx_msg void OnrelativeTime();
	afx_msg void OnClickOneclass();
	afx_msg void OnClickAllclasses();
	afx_msg void OnEnChangeSpikeclass();
	afx_msg void OnEnChangenbins();
	afx_msg void OnEnChangerowheight();
	afx_msg void OnEnChangeDotheight();
	afx_msg void OnFormatHistogram();	
	afx_msg void OnClickCycleHist();
	afx_msg void OnEditCopy();
	afx_msg void OnSelchangeHistogramtype();
	afx_msg void OnEnChangeEditnstipercycle();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeEditlockonstim();
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_tabCtrl;
	afx_msg void OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in dataView.cpp
	inline CdbWaveDoc* CSpikeHistView::GetDocument()
						{ return (CdbWaveDoc*)m_pDocument; }
#endif
