#pragma once

// viewspkd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeDetection form view

#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "./Controls/cdxCRotBevelLine.h"
#include "CSpkListTabCtrl.h"

class CViewSpikeDetection : public CDaoRecordView
{
protected:
	DECLARE_DYNCREATE(CViewSpikeDetection)
	CViewSpikeDetection();

	// Form Data
public:
	enum { IDD = IDD_VIEWSPKDETECTION };
	CRulerBar		m_xspkdscale;
	CComboBox		m_CBdetectWhat;
	CComboBox		m_CBdetectChan;
	CComboBox		m_CBtransform;
	CComboBox		m_CBtransform2;
	float			m_timefirst = 0.f;
	float			m_timelast = 0.f;
	CString			m_datacomments{};
	int				m_spikeno = -1;
	BOOL			m_bartefact = false;
	float			m_thresholdval = 0.f;
	int				m_ichanselected = 0;
	int				m_ichanselected2 = 0;
	cdxCRotBevelLine	m_bevel1;
	cdxCRotBevelLine	m_bevel2;
	cdxCRotBevelLine	m_bevel3;

	CdbWaveDoc*		GetDocument();

	// form variables
protected:
	int				m_scancount_doc = -1;			// number of channels in the data document
	CSpikeDoc*		p_spike_doc_ = nullptr;			// destination data doc
	CSpikeList*		p_spikelist_ = nullptr;			// temporary spike list
	CDWordArray 	m_DWintervals;					// intervals to draw detected spikes

	CChartDataWnd	m_ChartDataWnd_Detect;
	CChartDataWnd	m_ChartDataWnd_Source;
	CChartSpikeBarWnd	m_displaySpk_BarView;		// bars with spike height
	CChartSpikeShapeWnd	m_displaySpk_Shape;			// all spikes in displayspikes

	CScrollBarEx	m_filescroll;					// data position within file
	SCROLLINFO		m_filescroll_infos{};			// infos for scrollbar

	CEditCtrl		mm_spikeno;						// spike selected
	CEditCtrl		mm_thresholdval;				// value of threshold
	CEditCtrl		mm_timefirst;					// first abcissa value
	CEditCtrl		mm_timelast;					// last abcissa value
	CEditCtrl		mm_spkWndDuration;				// duration of the spike shape window
	CEditCtrl		mm_spkWndAmplitude;				// amplitude of the spike shape window
	CEditCtrl		mm_ichanselected;				// channel selected
	CEditCtrl		mm_ichanselected2;

	int				m_zoominteger = 0;				// zoom length (nb data acq points)
	SPKDETECTARRAY* m_pArrayFromApp = nullptr;
	CSpkDetectArray	m_parmsCurrent;
	SPKDETECTPARM* m_pDetectParms = nullptr;
	int 			m_iDetectParms = 0;				// index spk detect parm currently selected / array

	OPTIONS_VIEWDATA* options_viewdata = nullptr;	// browse options
	OPTIONS_VIEWDATAMEASURE* mdMO = nullptr;		// measure options

	float 		m_samplingRate = 0.f;				// data sampling rate
	BOOL		m_bValidThreshold = false;			// flag: TRUE=threshold should be evaluated
	BOOL		m_bDetected = false;
	TCHAR		m_szbuf[64]{};

	// Operations
protected:
	void		DetectAll(BOOL bAll);				// detect from current set of parms or from all
	int			DetectMethod1(WORD schan);			// spike detection, method 1 / m_spkD chan schan
	int			DetectStim1(int i);					// stimulus detection
	void		SerializeWindowsState(BOOL bSave, int itab = -1);

	// changing size of client moves children
	CStretchControl m_stretch;
	BOOL		m_binit = false;
	int			m_cursorstate = 0;
public:
	inline void	SetViewMouseCursor(int cursormode) {
					m_displaySpk_BarView.SetMouseCursorType(cursormode);
					m_displaySpk_Shape.SetMouseCursorType(cursormode);
					m_ChartDataWnd_Detect.SetMouseCursorType(cursormode);
					m_ChartDataWnd_Source.SetMouseCursorType(cursormode);
	}

	// Implementation
protected:
	void		AlignDisplayToCurrentSpike();
	void		HighlightSpikes(BOOL flag = TRUE);
	void		OnFileScroll(UINT nSBCode, UINT nPos);
	void		SaveCurrentSpikeFile();
	void		SelectSpikeNo(int spikeno, BOOL bMultipleSelection);
	BOOL		CheckDetectionSettings();
	void		UpdateCB();
	void		UpdateLegendDetectionWnd();
	void		UpdateCombosDetectChanAndTransforms();
	void		DefineStretchParameters();
	void		DefineSubClassedItems();
	void		UpdateDetectionParameters();
	void		UpdateDetectionControls();
	void		UpdateDetectionSettings(int iSelParms);
	void		UpdateFileParameters(BOOL bUpdateInterface = TRUE);
	void		UpdateDataFile(BOOL bUpdateInterface);
	void		UpdateSpikeFile(BOOL bUpdateInterface = TRUE);
	void		UpdateFileScroll();
	void		UpdateLegends();
	void		UpdateSpkShapeWndScale(BOOL bSetFromControls = TRUE);
	void		UpdateVTtags();
	void		UpdateSpikeDisplay();

	// public interface to view
public:
	// Overrides
	CDaoRecordset* OnGetRecordset() override;
	BOOL		PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL		OnMove(UINT nIDMoveCommand) override;
protected:
	void		OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void		DoDataExchange(CDataExchange* pDX) override;
	void		OnInitialUpdate() override;
	BOOL		OnPreparePrinting(CPrintInfo* pInfo) override;
	void		OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void		OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	void		OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void		OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	// Implementation
public:
	~CViewSpikeDetection() override;
#ifdef _DEBUG
	void		AssertValid() const override;
	void		Dump(CDumpContext& dc) const override;
#endif

	// print view
protected:
	CRect		m_Margin;				// margins (pixels)
	int			m_file0 = 0;			// current file
	long		m_lFirst0 = 0;
	long		m_lLast0 = 0;
	int			m_npixels0 = 0;
	int			m_nfiles = 0;			// nb of files in doc
	int 		m_nbrowsperpage = 0;	// USER: nb files/page
	long		m_lprintFirst = 0;		// file index of first pt
	long 		m_lprintLen = 0;		// nb pts per line
	float		m_printFirst = 0;
	float 		m_printLast = 0;
	BOOL		m_bIsPrinting = 0;
	CRect		m_rData;
	CRect		m_rSpike;

	// specific printer parameters
	TEXTMETRIC	m_tMetric{};			// onbegin/onendPrinting
	LOGFONT		m_logFont{};			// onbegin/onendPrinting
	CFont*		m_pOldFont = nullptr;	// onbegin/onendPrinting
	CFont		m_fontPrint;			// onbegin/onendPrinting
	CRect		m_printRect;

protected:
	void 		PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo);
	CString		PrintConvertFileIndex(long l_first, long l_last);
	void 		PrintComputePageSize();

	CString		PrintGetFileInfos();
	CString		PrintDataBars(CDC* p_dc, CChartDataWnd* pDataChartWnd, CRect* prect);
	CString		PrintSpkShapeBars(CDC* p_dc, CRect* prect, BOOL bAll);
	void		PrintCreateFont();
	BOOL		PrintGetFileSeriesIndexFromPage(int page, int& filenumber, long& l_first);
	BOOL		PrintGetNextRow(int& filenumber, long& l_first, long& very_last);
	int			PrintGetNPages();
	void		PrintDataCartridge(CDC* p_dc, CChartDataWnd* pDataChartWnd, CRect* prect, BOOL bComments, BOOL bBars);

	// gain and bias setting: data and functions
	HICON		m_hBias = nullptr;
	HICON		m_hZoom = nullptr;
	CScrollBar 	m_scrolly;				// V scrollbar
	float		m_yscaleFactor = 0.f;	// div factor for y bar
	int			m_VBarMode = 0;			// flag V scrollbar state

	HICON		m_hBias2 = nullptr;
	HICON		m_hZoom2 = nullptr;
	CScrollBar 	m_scrolly2;				// V scrollbar
	float		m_yscaleFactor2 = 0.f;	// div factor for y bar
	int			m_VBarMode2 = 0;		// flag V scrollbar state

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
	CSpkListTabCtrl	m_tabCtrl;
	afx_msg void OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult);
};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* CViewSpikeDetection::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif
