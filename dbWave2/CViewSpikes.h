#pragma once

// ViewSpikes.h : header file

#include "SpikeClassListBox.h"
#include "CSpkListTabCtrl.h"
#include "CViewDao.h"

class CViewSpikes : public CViewDAO
{
protected:
	DECLARE_DYNCREATE(CViewSpikes)
	CViewSpikes();

// Form Data
public:
	enum { IDD = IDD_VIEWSPIKES };
	float			m_timefirst = 0.f;
	float			m_timelast = 0.f;
	int				m_spikeno = -1;
	int				m_spikenoclass = 0;
	float			m_zoom = .2f;
	int				m_sourceclass = 0;
	int				m_destclass = 1;
	BOOL			m_bresetzoom = true;
	BOOL			m_bartefact = false;
	BOOL			m_bKeepSameClass = false;
	float			m_jitter_ms = 1.f;
	CSpkListTabCtrl	m_tabCtrl;

	// Attributes
protected:
	CSpikeClassListBox m_spkClassListBox;	// listbox of spike classes
	int				m_maxclasses = 1;
	
	CChartDataWnd	m_ChartDataWnd;			// data display
	CEditCtrl		mm_spikeno;
	CEditCtrl		mm_spikenoclass;
	CEditCtrl		mm_timefirst;			// first abcissa value
	CEditCtrl		mm_timelast;			// last abcissa value
	CEditCtrl		mm_zoom;				// zoom factor
	CEditCtrl		mm_sourceclass;
	CEditCtrl		mm_destclass;
	CEditCtrl		mm_jitter_ms;

	int				m_zoominteger = 1;		// zoom length (nb data acq points)
	HICON			m_hBias = nullptr;
	HICON			m_hZoom = nullptr;
	float			m_yscaleFactor = 0.f;	// div factor for y bar
	int				m_VBarMode = 0;			// flag V scrollbar state
	CScrollBar 		m_scrolly;				// V scrollbar

	CSpikeDoc*		m_pSpkDoc = nullptr;	// destination data doc
	CSpikeList*		m_pSpkList = nullptr;	// temporary spike list
	CAcqDataDoc*	m_pDataDoc = nullptr;	// data document pointer

	BOOL			m_bInitSourceView = true;
	int				m_lFirst = 0;
	int				m_lLast = -1;
	SCROLLINFO		m_scrollFilePos_infos{};
	CDWordArray		m_DWintervals;			// intervals to highlight spikes
	BOOL			m_baddspikemode = false;

	int				m_yWE = 1;				// offset and gain to display spikes
	int				m_yWO = 0;
	int				m_ptVT = -1;
	CRect			m_rectVTtrack = CRect(0, 0, 0, 0);
	float			m_jitter = 0.f;
	BOOL			m_bdummy = true;

	// Implementation
protected:
	void UpdateFileParameters(BOOL bUpdateInterface = TRUE);
	void UpdateLegends(BOOL bUpdateInterface);	
	void UpdateDataFile(BOOL bUpdateInterface);
	void UpdateSpikeFile(BOOL bUpdateInterface);
	void AdjustYZoomToMaxMin(BOOL bForceSearchMaxMin);
	void SaveCurrentFileParms();
	void SelectSpike(int spikeno);
	void DefineSubClassedItems();
	void DefineStretchParameters();
	void UpdateScrollBar();
	void ZoomOnPresetInterval(int iistart);
	void OnGainScroll(UINT nSBCode, UINT nPos);
	void OnBiasScroll(UINT nSBCode, UINT nPos);
	void UpdateGainScroll();
	void UpdateBiasScroll();
	void SetVBarMode(short bMode);
	BOOL AddSpiketoList(long iitime, BOOL bcheck_if_otheraround);
	void SetAddspikesMode(int mousecursorType);
	void SelectSpkList(int icursel);

	// public interface to view
public:
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
	//}}AFX_VIRTUAL

	// page format printing parameters (pixel unit)
	OPTIONS_VIEWDATA*	options_viewdata = nullptr;	// view data options
	OPTIONS_VIEWDATAMEASURE* mdMO = nullptr;	// measure options
	SPKCLASSIF*			m_psC = nullptr;
	SPKDETECTPARM*		m_pspkDP = nullptr;		// spike detection parameters

protected:
	void 	PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo);
	CString PrintConvertFileIndex(long l_first, long l_last);
	void 	PrintComputePageSize();
	CString PrintGetFileInfos();
	CString PrintBars(CDC* p_dc, CRect* rect);
	long 	PrintGetFileSeriesIndexFromPage(int page, int* file);
	float	PrintChangeUnit(float xVal, CString* xUnit, float* xScalefactor);

	// Implementation
protected:
	virtual ~CViewSpikes();

	// Generated message map functions
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
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
	afx_msg void OnZoom();
	afx_msg void OnEnChangeZoom();
	afx_msg void OnEnChangeSourceclass();
	afx_msg void OnEnChangeDestclass();
	afx_msg void OnFormatPreviousframe();
	afx_msg void OnFormatNextframe();
	afx_msg void OnGAINbutton();
	afx_msg void OnBIASbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnArtefact();
	afx_msg void OnEnChangeJitter();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHScrollLeft();
	afx_msg void OnHScrollRight();
	afx_msg void OnBnClickedSameclass();
	
public:
	afx_msg void OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
