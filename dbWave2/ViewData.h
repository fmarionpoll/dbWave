#pragma once

#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "afxwin.h"
#include "ChartData.h"
#include "Editctrl.h"

class CViewData : public CDaoRecordView
{
protected:
	DECLARE_DYNCREATE(CViewData)
	CViewData();					// protected constructor used by dynamic creation

// Form Data
public:
	enum { IDD = IDD_VIEWDATA };

	int			m_ichanselected = 0;
	float		m_v1			= 0.;
	float		m_v2			= 0.;
	float		m_diff			= 0.;
	float		m_timefirst		= 0.;
	float		m_timelast		= 0.;

	// subclassed controls within CDaoRecordView
	CChartDataWnd m_ChartDataWnd{};	// data display
	float		m_floatNDigits = 1000.;  // 10(000) -> n digits displayed
	CEditCtrl	mm_v1;				// first HZ cursor
	CEditCtrl	mm_v2;				// second HZ cursor
	CEditCtrl	mm_diff;			// difference v1-v2
	CEditCtrl	mm_timefirst;		// first abcissa value
	CEditCtrl	mm_timelast;		// last abcissa value
	CComboBox	m_comboSelectChan;
	CRulerBar	m_ADC_yRulerBar;
	CRulerBar	m_ADC_xRulerBar;

	BOOL		m_bInitComment = true;
	CdbWaveDoc* GetDocument();

protected:
	// parameters related to data display and to document
	CAcqDataDoc* m_pdatDoc		= nullptr;
	BOOL		m_bvalidDoc		= false;
	float		m_samplingRate	= 1.;
	int 		m_cursorstate	= 0;		// cursor = system arrow
	int			m_VBarpixelratio = 30;		// vertical bar pixel ratio
	int			m_HBarpixelratio = 10;		// horizontal bar pixel ratio
	int			m_currentfileindex = 0;

	HICON		m_hBias			= nullptr;
	HICON		m_hZoom			= nullptr;
	int			scan_count		= 0;
	float		chrate			= 0.;

	CRect 		m_Margin;					// margins (pixels)
	int			m_file0			= 0;		// current file
	long		m_lFirst0		= 0;
	long		m_lLast0		= 0;
	int			m_npixels0		= 0;

	int			m_nfiles		= 0;		// nb of files in doc
	int 		m_nbrowsperpage = 0;		// USER: nb files/page
	long 		m_lprintFirst	= 0;		// file index of first pt
	long 		m_lprintLen		= 0;		// nb pts per line
	float 		m_printFirst	= 0.;
	float 		m_printLast		= 0.;
	BOOL		m_bIsPrinting	= false;

	// specific printer parameters
	TEXTMETRIC	m_tMetric{};			// onbegin/onendPrinting
	LOGFONT		m_logFont{};			// onbegin/onendPrinting
	CFont*		m_pOldFont		= nullptr; 	// onbegin/onendPrinting
	CFont		m_fontPrint;			// onbegin/onendPrinting

	// page format printing parameters (pixel unit)
	CRect		m_printRect;
	OPTIONS_VIEWDATA* options_viewdata	= nullptr;
	OPTIONS_VIEWDATAMEASURE* mdMO		= nullptr;		// measure options

protected:
	void 	PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo);
	CString ConvertFileIndex(long l_first, long l_last);
	void 	ComputePrinterPageSize();
	CString GetFileInfos();
	CString PrintBars(CDC* p_dc, CRect* rect);
	BOOL	GetFileSeriesIndexFromPage(int page, int& filenumber, long& l_first);
	BOOL	PrintGetNextRow(int& filenumber, long& l_first, long& verylast);
	void	SaveModifiedFile();
	void	UpdateFileParameters(BOOL bUpdateInterface = TRUE);
	void	UpdateChannelsDisplayParameters();
	void	ChainDialog(WORD iID);
	int		PrintGetNPages();

	CStretchControl m_stretch;
	BOOL		m_binit			= false;
	BOOL		m_bCommonScale	= false;

	//public:
protected:
	CScrollBarEx m_filescroll;			// data position within file
	SCROLLINFO	m_filescroll_infos{};	// infos for scrollbar
	int			m_VBarMode{};			// flag V scrollbar state
	CScrollBar 	m_scrolly;				// V scrollbar

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
	void	MeasureProperties(int item);

	// Overrides
public:
	CDaoRecordset* OnGetRecordset() override;
	BOOL	PreCreateWindow(CREATESTRUCT& cs)  override;
	BOOL	OnMove(UINT nIDMoveCommand)  override;
protected:
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void	DoDataExchange(CDataExchange* pDX) override;
	void	OnInitialUpdate() override;
	BOOL	OnPreparePrinting(CPrintInfo* pInfo) override;
	void	OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void	OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	void	OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void	OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	void	DefineSubClassedItems();
	void	DefineStretchParameters();

	// Implementation
public:
	virtual ~CViewData();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

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
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif
