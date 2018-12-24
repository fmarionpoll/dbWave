#pragma once

// viewdata.h : header file



#include "RulerBar.h"
#include "ScrollBarEx.h"
//#include "dbWave_constants.h"
#include "afxwin.h"
#include "Lineview.h"
#include "Editctrl.h"


class CViewData : public CDaoRecordView
{
protected:
	DECLARE_DYNCREATE(CViewData)
	CViewData();					// protected constructor used by dynamic creation

// Form Data
public:	
	enum { IDD = IDD_VIEWDATA };
	
	int		m_ichanselected;
	float	m_v1;
	float	m_v2;
	float	m_diff;	
	float	m_timefirst;
	float	m_timelast;

	// subclassed controls within CDaoRecordView
	CLineViewWnd	m_VDlineview;	// data display
	float		m_floatNDigits; 	// 10(000) -> n digits displayed
	CEditCtrl	mm_v1;				// first HZ cursor
	CEditCtrl	mm_v2;				// second HZ cursor
	CEditCtrl	mm_diff;			// difference v1-v2
	CEditCtrl	mm_timefirst;		// first abcissa value
	CEditCtrl	mm_timelast;		// last abcissa value
	CComboBox	m_comboSelectChan;
	CRulerBar	m_ADC_yRulerBar;
	CRulerBar	m_ADC_xRulerBar;

	BOOL		m_bInitComment;	
	CdbWaveDoc*	GetDocument();
	
protected:	
	// parameters related to data display and to document	
	CAcqDataDoc* m_pdatDoc;			// document pointer
	BOOL	m_bvalidDoc;
	float	m_samplingRate;
	int 	m_cursorstate;			// lineview cursor 
	int		m_VBarpixelratio;		// vertical bar pixel ratio
	int		m_HBarpixelratio;		// horizontal bar pixel ratio
	int		m_currentfileindex;

	HICON	m_hBias;
	HICON	m_hZoom;
	int		scan_count;
	float	chrate;
	
	CRect 	m_Margin;				// margins (pixels)
	int		m_file0;				// current file
	long	m_lFirst0;
	long	m_lLast0;
	int		m_npixels0;
		
	int		m_nfiles;				// nb of files in doc
	int 	m_nbrowsperpage;		// USER: nb files/page
	long 	m_lprintFirst;			// file index of first pt
	long 	m_lprintLen{};			// nb pts per line
	float 	m_printFirst{};
	float 	m_printLast{};
	BOOL	m_bIsPrinting;

	// specific printer parameters
	TEXTMETRIC m_tMetric{};			// onbegin/onendPrinting
	LOGFONT	m_logFont{};				// onbegin/onendPrinting
	CFont*	m_pOldFont{};				// onbegin/onendPrinting
	CFont	m_fontPrint;			// onbegin/onendPrinting    

	// page format printing parameters (pixel unit)    
	CRect						m_printRect;
	OPTIONS_VIEWDATA*			mdPM{};		// view data options
	OPTIONS_VIEWDATAMEASURE*	mdMO{};		// measure options

protected:
	void 	PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo);	
	CString ConvertFileIndex(long l_first, long l_last);
	void 	ComputePrinterPageSize();
	CString GetFileInfos();
	CString PrintBars(CDC* p_dc, CRect* rect);
	BOOL	GetFileSeriesIndexFromPage(int page, int &filenumber, long &l_first);
	BOOL	PrintGetNextRow(int &filenumber, long &l_first, long &verylast);
	void	SaveModifiedFile();
	void	UpdateFileParameters(BOOL bUpdateInterface=TRUE);	
	void	UpdateChannelsDisplayParameters();
	void	ChainDialog(WORD iID);
	int		PrintGetNPages();	
		
protected:
	CStretchControl m_stretch;
	BOOL	m_binit;
	BOOL	m_bCommonScale{};

//public:
protected:
	CScrollBarEx	m_filescroll;		// data position within file
	SCROLLINFO		m_filescroll_infos{};	// infos for scrollbar
	int				m_VBarMode{};			// flag V scrollbar state
	CScrollBar 		m_scrolly;			// V scrollbar

protected:
	void	OnFileScroll(UINT nSBCode, UINT nPos);
	void	OnGainScroll(UINT nSBCode, UINT nPos);
	void	UpdateYExtent(int ichan, int yextent);
	void	UpdateYZero(int ichan, int ybias);
	void	OnBiasScroll(UINT nSBCode, UINT nPos);
	void	UpdateGainScroll();
	void	UpdateBiasScroll();
	void	SetVBarMode(short bMode);
	void	UpdateFileScroll();
	void	UpdateLegends(int operation);
	void	UpdateHZtagsVal();
	void	SetCursorAssociatedWindows();
	void	UpdateChannel(int channel);

////////////////////////////////////////////////////////////////////////////////
// conversions Helper functions
protected:
	void MeasureProperties(int item);
	
	// Overrides
public:
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);   	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* p_dc, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

// Implementation
public:
	virtual ~CViewData();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	DECLARE_MESSAGE_MAP()
	// Generated message map functions
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickedBias();
	afx_msg void OnClickedGain();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFormatXscale();
	afx_msg void OnFormatYscale();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnToolsDataseries();
	afx_msg void ADC_OnHardwareChannelsDlg();
	afx_msg void ADC_OnHardwareIntervalsDlg();
	afx_msg void OnCenterCurve();
	afx_msg void OnGainAdjustCurve();
	afx_msg void OnSplitCurves();
	afx_msg void OnFirstFrame();
	afx_msg void OnLastFrame();
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnViewAlldata();
	afx_msg void OnFormatDataseriesattributes();
	afx_msg void OnToolsMeasuremode();
	afx_msg void OnToolsMeasure();
	afx_msg void OnToolsVerticaltags();
	afx_msg void OnToolsHorizontalcursors();
	afx_msg void OnUpdateToolsHorizontalcursors(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolsVerticaltags(CCmdUI* pCmdUI);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg void ADC_OnHardwareDefineexperiment();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnCbnSelchangeCombochan();
};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* CViewData::GetDocument()
   { return (CdbWaveDoc*)m_pDocument; }
#endif

