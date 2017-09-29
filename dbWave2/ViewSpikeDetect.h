#pragma once

// viewspkd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpikeDetectionView form view

#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "Controls\cdxCRotBevelLine.h"

class CSpikeDetectionView : public CDaoRecordView
{

protected:
	DECLARE_DYNCREATE(CSpikeDetectionView)
	CSpikeDetectionView();

// Form Data
public:
	enum { IDD = IDD_VIEWSPKDETECTION };
	CRulerBar		m_xspkdscale;
	CComboBox		m_CBdetectWhat;
	CComboBox		m_CBdetectChan;
	CComboBox		m_CBtransform;
	CComboBox		m_CBtransform2;
	float			m_timefirst;
	float			m_timelast;
	CString			m_datacomments;
	int				m_spikeno;
	BOOL			m_bartefact;
	float			m_thresholdval;
	int				m_ichanselected;
	int				m_ichanselected2;
	cdxCRotBevelLine	m_bevel1;
	cdxCRotBevelLine	m_bevel2;
	cdxCRotBevelLine	m_bevel3;

	CdbWaveDoc*		GetDocument();
	
// form variables
protected:
	CdbWaveDoc*		m_dbDoc;			// master source document
	int				m_scancount_doc;	// number of channels in the data document
	CSpikeDoc*		m_pspkDocVSD;		// destination data doc
	CSpikeList*		m_pSpkListVSD;		// temporary spike list	
	CDWordArray 	m_DWintervals;		// intervals to draw detected spikes

	CLineViewWnd	m_displayDetect;	// source data
	CLineViewWnd	m_displayData;
	CSpikeBarWnd	m_spkBarView;		// bars with spike height	
	CSpikeShapeWnd	m_spkShapeView;		// all spikes in displayspikes
	CScrollBarEx	m_filescroll;		// data position within file

	SCROLLINFO		m_filescroll_infos;	// infos for scrollbar
	CEditCtrl		mm_spikeno;			// spike selected
	CEditCtrl		mm_thresholdval;	// value of threshold
	CEditCtrl		mm_timefirst;		// first abcissa value
	CEditCtrl		mm_timelast;		// last abcissa value
	CEditCtrl		mm_spkWndDuration;	// duration of the spike shape window
	CEditCtrl		mm_spkWndAmplitude;	// amplitude of the spike shape window
	CEditCtrl		mm_ichanselected;	// channel selected
	CEditCtrl		mm_ichanselected2;

	int				m_zoominteger;		// zoom length (nb data acq points)
	SPKDETECTARRAY*	m_pArrayFromApp;
	CSpkDetectArray	m_parmsCurrent;
	SPKDETECTPARM*	m_pDetectParms;
	int 			m_iDetectParms;		// index spk detect parm currently selected / array

	OPTIONS_VIEWDATA*  mdPM;			// browse options
	OPTIONS_VIEWDATAMEASURE* mdMO;		// measure options

	float 			m_samplingRate;		// data sampling rate
	BOOL			m_bValidThreshold;	// flag: TRUE=threshold should be evaluated
	BOOL			m_bDetected;
	TCHAR			m_szbuf[64];

// Operations
protected:
	void			DetectAll(BOOL bAll);			// detect from current set of parms or from all
	int				DetectMethod1(WORD schan);		// spike detection, method 1 / m_spkD chan schan
	int				DetectStim1(int i);				// stimulus detection
	void			SerializeWindowsState(BOOL bSave, int itab = -1);

// CStretchControl: changing size of client moves children
	CStretchControl m_stretch;
	BOOL			m_binit;

////////////////////////////////////////////////////////////////////////////////
// cursor
protected:
	int				m_cursorstate;
public:
	inline void		SetViewMouseCursor(int cursormode) { 
						m_spkBarView.SetMouseCursorType(cursormode);
						m_spkShapeView.SetMouseCursorType(cursormode);
						m_displayDetect.SetMouseCursorType(cursormode);
						m_displayData.SetMouseCursorType(cursormode);}

// Implementation
protected:
	void			AlignDisplayToCurrentSpike();			// align display to current spike
	void			HighlightSpikes(BOOL flag = TRUE);
	void			OnFileScroll(UINT nSBCode, UINT nPos);	// scroll data file horizontally	
	void			SaveCurrentFileParms();
	void			SelectSpikeNo(int spikeno, BOOL bMultipleSelection);		// select spike no
	BOOL			CheckDetectionSettings();
	void			UpdateCB();
	void			UpdateDetectionParameters();
	void			UpdateDetectionControls();
	void			UpdateDetectionSettings(int iSelParms);
	void			UpdateFileParameters(BOOL bUpdateInterface=TRUE);
	void			UpdateDataFile(BOOL bUpdateInterface);
	void			UpdateSpikeFile(BOOL bUpdateInterface=TRUE);
	
	void			UpdateFileScroll();					// adjust horiz position of scroll bar
	void			UpdateLegends();
	void			UpdateSpkShapeWndScale(BOOL bSetFromControls=TRUE);
	void			UpdateVTtags();
	void			UpdateSpikeDisplay();
	
// public interface to view
public:
	// Overrides
	public:
	virtual			CDaoRecordset* OnGetRecordset();
	virtual BOOL	PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL	OnMove(UINT nIDMoveCommand);
	protected:
	virtual void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);   	
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void	OnInitialUpdate(); // called first time after construct
	virtual BOOL	OnPreparePrinting(CPrintInfo* pInfo);
	virtual void	OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void	OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void	OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void	OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

// Implementation
public:
	virtual ~CSpikeDetectionView();
#ifdef _DEBUG
	virtual void	AssertValid() const;
	virtual void	Dump(CDumpContext& dc) const;
#endif

////////////////////////////////////////////////////////////////////////////////
// print view
protected:
	CRect			m_Margin;			// margins (pixels)
	int				m_file0;			// current file
	long			m_lFirst0;
	long			m_lLast0;
	int				m_npixels0;
	int				m_nfiles;			// nb of files in doc
	int 			m_nbrowsperpage;	// USER: nb files/page
	long			m_lprintFirst;		// file index of first pt
	long 			m_lprintLen;		// nb pts per line
	float			m_printFirst;
	float 			m_printLast;
	BOOL			m_bIsPrinting;
	CRect			m_rData;
	CRect			m_rSpike;

	// specific printer parameters
	TEXTMETRIC		m_tMetric;			// onbegin/onendPrinting
	LOGFONT			m_logFont;			// onbegin/onendPrinting
	CFont*			m_pOldFont;			// onbegin/onendPrinting
	CFont			m_fontPrint;		// onbegin/onendPrinting    

	// page format printing parameters (pixel unit)    
	CRect			m_printRect;

protected:
	void 		PrintFileBottomPage(CDC* pDC, CPrintInfo* pInfo);
	CString		PrintConvertFileIndex(long lFirst, long lLast);
	void 		PrintComputePageSize();
	
	CString		PrintGetFileInfos();
	CString		PrintDataBars(CDC* pDC, CLineViewWnd* pLineViewWnd, CRect* prect);
	CString		PrintSpkShapeBars(CDC* pDC, CRect* prect, BOOL bAll);
	void		PrintCreateFont();
	BOOL		PrintGetFileSeriesIndexFromPage(int page, int &filenumber, long &lFirst);
	BOOL		PrintGetNextRow(int &filenumber, long &lFirst, long &veryLast);
	int			PrintGetNPages();
	void		PrintDataCartridge (CDC* pDC, CLineViewWnd* plineViewWnd, CRect* prect, BOOL bComments, BOOL bBars);

	// gain and bias setting: data and functions
	HICON		m_hBias;
	HICON		m_hZoom;
	CScrollBar 	m_scrolly;			// V scrollbar
	float		m_yscaleFactor;		// div factor for y bar 
	int			m_VBarMode;			// flag V scrollbar state

	HICON		m_hBias2;
	HICON		m_hZoom2;
	CScrollBar 	m_scrolly2;			// V scrollbar
	float		m_yscaleFactor2;	// div factor for y bar 
	int			m_VBarMode2;		// flag V scrollbar state

	void		OnGainScroll(UINT nSBCode, UINT nPos, int iID);
	void		OnBiasScroll(UINT nSBCode, UINT nPos, int iID);
	void		UpdateGainScroll(int iID);
	void		UpdateBiasScroll(int iID);
	void		SetVBarMode(short bMode, int iID);
	void		UpdateTabs();

	// Generated message map functions

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMeasureAll();
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFirstFrame();
	afx_msg void OnLastFrame();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSelchangeDetectchan();
	afx_msg void OnSelchangeTransform();
	afx_msg void OnSelchangeDetectMode();
	afx_msg void OnClear();
	afx_msg void OnEnChangeSpikeno();
	afx_msg void OnArtefact();
	afx_msg void OnFormatXscale();
	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatYscaleCentercurve();
	afx_msg void OnFormatYscaleGainadjust();
	afx_msg void OnFormatSplitcurves();
	afx_msg void OnEnChangeThresholdval();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnToolsDetectionparameters();
	afx_msg void OnToolsEdittransformspikes();
	afx_msg void OnToolsDataseries();
	afx_msg void OnEditCopy();
	afx_msg void OnFileSave();
	afx_msg void OnDestroy();
	afx_msg void OnMeasure();

	afx_msg void OnBnClickedBiasbutton();
	afx_msg void OnBnClickedGainbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeSpkWndAmplitude();
	afx_msg void OnEnChangeSpkWndLength();
	afx_msg void OnBnClickedLocatebttn();


	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedClearall();
	afx_msg void OnToolsEditstimulus();
	afx_msg void OnEnChangeChanselected();
	afx_msg void OnBnClickedGain2();
	afx_msg void OnBnClickedBias2();
	afx_msg void OnEnChangeChanselected2();
	afx_msg void OnCbnSelchangeTransform2();

	DECLARE_MESSAGE_MAP()
	CTabCtrl m_tabCtrl;
	afx_msg void OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* CSpikeDetectionView::GetDocument()
   { return (CdbWaveDoc*)m_pDocument; }
#endif
