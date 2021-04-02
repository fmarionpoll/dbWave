#pragma once

// SpikeHistView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeHist form view

//#include "CustomTabCtrl.h"

class CViewSpikeHist : public CDaoRecordView
{
	DECLARE_DYNCREATE(CViewSpikeHist)
protected:
				CViewSpikeHist();
	virtual		~CViewSpikeHist() override;
// Form Data
public:
	enum { IDD = IDD_VIEWSPKTIMESERIES };
	CComboBox	m_cbHistType;
	float		m_timefirst = 0.f;
	float		m_timelast = 2.5f;
	int			m_spikeclass = 0;
	int			m_dotheight = 0;
	int			m_rowheight = 0;
	float		m_binISIms = 0.f;
	int			m_nbinsISI = 0;
	float		m_timebinms = 0.f;

	CdbWaveDoc* GetDocument();
	CSpikeDoc*	p_spike_doc_ = nullptr;	

// Attributes
protected:
	CStretchControl m_stretch;
	BOOL		m_binit = false;
	CEditCtrl	mm_timebinms;		// bin size (ms)
	CEditCtrl	mm_binISIms;		// bin size (ms)
	CEditCtrl	mm_nbinsISI;		// nbins ISI

	CEditCtrl	mm_timefirst;		// first abcissa value
	CEditCtrl	mm_timelast;		// last abcissa value
	CEditCtrl	mm_spikeclass;		// selected spike class
	CEditCtrl	mm_dotheight;		// dot height
	CEditCtrl	mm_rowheight;		// row height
	OPTIONS_VIEWSPIKES* m_pvdS = nullptr;		// histogram options
	OPTIONS_VIEWDATA* mdPM = nullptr;			// view data options
	int			m_bhistType = 0;
	SCROLLINFO m_scrollFilePos_infos{};

	long*		m_pPSTH = nullptr;	// histogram data (pointer to array)
	int			m_sizepPSTH = 0;	// nbins within histogram
	long		m_nPSTH = 0;
	long*		m_pISI = nullptr;
	int			m_sizepISI = 0;
	long		m_nISI = 0;
	long*		m_parrayISI = nullptr;
	int			m_sizeparrayISI = 0;

	CRect		m_displayRect = CRect(0, 0, 0, 0);
	CPoint		m_topleft;			// top position of display area
	BOOL		m_initiated = false;// flag / initial settings
	BOOL		m_bmodified = true;	// flag ON-> compute data
	CBitmap*	m_pbitmap = nullptr;// temp bitmap used to improve display speed
	int			m_nfiles = 1;		// nb of files used to build histogram
	const float	t1000 = 1000.f;
	BOOL		m_bPrint = false;
	CRect		m_commentRect;

	LOGFONT		m_logFont{};		// onbegin/onendPrinting
	CFont		m_fontPrint;
	int			m_rectratio = 100;
	float		m_xfirst = 0.f;
	float		m_xlast = 0.f;

	LOGFONT		m_logFontDisp{};	// onbegin/onendPrinting
	CFont		m_fontDisp{};		// display font

	// Overrides
public:
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void DoDataExchange(CDataExchange* pDX); 
	virtual void OnInitialUpdate(); 
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnDraw(CDC* p_dc);
	virtual void OnPrint(CDC* p_dc, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo);

protected:
	void		buildData();
	void		getFileInfos(CString& str_comment);
	void		displayHistogram(CDC* p_dc, CRect* pRect);
	void		displayDot(CDC* p_dc, CRect* pRect);
	void		displayPSTHAutoc(CDC* p_dc, CRect* pRect);
	void		displayStim(CDC* p_dc, CRect* pRect, long* l_first, long* l_last);
	void		buildDataAndDisplay();
	void		showControls(int iselect);
	void		selectSpkList(int icursel, BOOL bRefreshInterface = FALSE);

	// Implementation
protected:
	long		plotHistog(CDC* p_dc, CRect* dispRect, int nbins, long* phistog0, int orientation = 0, int btype = 0);

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

public:
	CTabCtrl m_tabCtrl;
	afx_msg void OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* CViewSpikeHist::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif
