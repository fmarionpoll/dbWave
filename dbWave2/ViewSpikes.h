#pragma once

// ViewSpikes.h : header file

#include "SpikeClassListBox.h"

class CViewSpikes : public CDaoRecordView
{
protected:
	DECLARE_DYNCREATE(CViewSpikes)
	CViewSpikes();			// protected constructor used by dynamic creation

// Form Data
public:
	enum { IDD = IDD_VIEWSPIKES };
	float			m_timefirst;
	float			m_timelast;
	int				m_spikeno;
	int				m_spikenoclass;
	float			m_zoom;
	int				m_sourceclass;
	int				m_destclass;
	BOOL			m_bresetzoom;
	BOOL			m_bartefact;
	float			m_jitter_ms;
	CdbWaveDoc*		GetDocument();

// Attributes
protected:
	CSpikeClassListBox m_spkClass;		// listbox of spike classes
	CLineViewWnd	m_sourceView;		// data display
	CEditCtrl		mm_spikeno;
	CEditCtrl		mm_spikenoclass;
	CEditCtrl		mm_timefirst;		// first abcissa value
	CEditCtrl		mm_timelast;		// last abcissa value
	CEditCtrl		mm_zoom;			// zoom factor
	CEditCtrl		mm_sourceclass;
	CEditCtrl		mm_destclass;
	CEditCtrl		mm_jitter_ms;

	int				m_zoominteger;		// zoom length (nb data acq points)
	HICON			m_hBias;
	HICON			m_hZoom;
	float			m_yscaleFactor;		// div factor for y bar 
	int				m_VBarMode;			// flag V scrollbar state
	CScrollBar 		m_scrolly;			// V scrollbar

	CStretchControl m_stretch;			// clamp controls to sides of the formview area
	BOOL			m_binit;

	CdbWaveDoc*		m_dbDoc;			// master source document
	CSpikeDoc*		m_pSpkDoc;			// destination data doc
	CSpikeList*		m_pSpkList;			// temporary spike list	
	CAcqDataDoc*	m_pDataDoc;				// data document pointer
	BOOL			m_bDatDocExists;
	BOOL			m_bSpkDocExists;
	BOOL			m_bInitSourceView;
	int				m_lFirst;
	int				m_lLast;
	SCROLLINFO		m_scrollFilePos_infos;
	CDWordArray		m_DWintervals;		// intervals to highlight spikes
	BOOL			m_baddspikemode;	

	int				m_yWE;		// offset and gain to display spikes
	int				m_yWO;
	int				m_ptVT;
	CRect			m_rectVTtrack;
	float			m_jitter;
	BOOL			m_bdummy;

// Implementation
protected:
	void UpdateLegends(BOOL bFirst);	// update legends
	void UpdateFileParameters();		// update parms when file has changed	
	void SaveCurrentFileParms();		// save spike file if modified
	void SelectSpike(int spikeno);
	void UpdateScrollBar();
	void ZoomOnPresetInterval(int iistart);
	void OnGainScroll(UINT nSBCode, UINT nPos);
	void OnBiasScroll(UINT nSBCode, UINT nPos);
	void UpdateGainScroll();
	void UpdateBiasScroll();
	void SetVBarMode(short bMode);
	BOOL AddSpiketoList(long iitime, BOOL bcheck_if_otheraround);
	void SetAddspikesMode (int mousecursorType);
	void SelectSpkList(int icursel);

// public interface to view
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewData)
	public:
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);   	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);	    	
	//}}AFX_VIRTUAL


// VIEWPRNT.CPP: print view
protected:
	CRect 	m_Margin;				// margins (pixels)
	int		m_file0{};				// current file
	long	m_lFirst0{};
	long	m_lLast0{};
		
	int		m_nfiles{};				// nb of files in doc
	int 	m_nbrowsperpage{};		// USER: nb files/page
	long 	m_lprintFirst{};			// file index of first pt
	long 	m_lprintLen{};			// nb pts per line
	int 	m_printFirst{};
	int 	m_printLast{};
	int		m_maxclasses;
	BOOL	m_bIsPrinting{};
	
	// specific printer parameters
	TEXTMETRIC m_tMetric{};			// onbegin/onendPrinting
	LOGFONT	m_logFont{};				// onbegin/onendPrinting
	CFont*	m_pOldFont{};				// onbegin/onendPrinting
	CFont	m_fontPrint;			// onbegin/onendPrinting    

	// page format printing parameters (pixel unit)    
	CRect						m_printRect;
	OPTIONS_VIEWDATA*			mdPM;			// view data options
	OPTIONS_VIEWDATAMEASURE*	mdMO;			// measure options
	SPKCLASSIF*					m_psC;
	SPKDETECTPARM*				m_pspkDP;		// spike detection parameters

protected:
	void 	PrintFileBottomPage(CDC* pDC, CPrintInfo* pInfo);	
	CString PrintConvertFileIndex(long l_first, long l_last);
	void 	PrintComputePageSize();
	CString PrintGetFileInfos();
	CString PrintBars(CDC* pDC, CRect* rect);
	long 	PrintGetFileSeriesIndexFromPage(int page, int* file);
	float	PrintChangeUnit(float xVal, CString* xUnit, float* xScalefactor);

// Implementation
protected:
	virtual ~CViewSpikes();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatCentercurve();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnToolsEdittransformspikes();
	afx_msg void OnEnChangeNOspike();
	afx_msg void OnEnChangeSpikenoclass();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditCopy();
	afx_msg void OnDestroy();
	afx_msg void OnZoom();
	afx_msg void OnEnChangeZoom();
	afx_msg void OnEnChangeSourceclass();
	afx_msg void OnEnChangeDestclass();
	afx_msg void OnFormatPreviousframe();
	afx_msg void OnFormatNextframe();
	afx_msg void OnGAINbutton();
	afx_msg void OnBIASbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
//	afx_msg void OnExchangeSourceAndDest();
	afx_msg void OnArtefact();
	afx_msg void OnEnChangeJitter();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRecordShiftleft();
	afx_msg void OnRecordShiftright();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonClickedTab(NMHDR* pNMHDR, LRESULT* pResult) ;
	afx_msg void OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	CTabCtrl m_tabCtrl;
};

#ifndef _DEBUG  // debug version in dataView.cpp
	inline CdbWaveDoc* CViewSpikes::GetDocument()
						{ return (CdbWaveDoc*)m_pDocument; }
#endif
