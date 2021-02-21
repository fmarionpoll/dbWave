#pragma once
#include "ChartSpikeHist.h"
#include "ChartSpikeXY.h"
#include "ChartSpikeShape.h"
#include "dbWaveDoc.h"
#include "ChartSpikeBar.h"
#include "ChartSpikeHistVert.h"
#include "CSpkListTabCtrl.h"

// spikesort1.h : header file

class CViewSpikeSort : public CDaoRecordView
{
	DECLARE_DYNCREATE(CViewSpikeSort)
protected:
	CViewSpikeSort();			// protected constructor used by dynamic creation

// Form Data
public:
	enum { IDD = IDD_VIEWSPKSORT1 };
	CComboBox		m_CBparameter;

	float	m_t1 = 0.f;
	float	m_t2 = 0.f;
	float	m_lower = 0.f;
	float	m_upper = 0.f;
	int		m_sourceclass = 0;
	int		m_destinationclass = 0;
	float	m_timeFirst = 0.f;
	float	m_timeLast = 0.f;
	float	m_mVMax = 0.f;
	float	m_mVMin = 0.f;
	BOOL	m_bAllfiles = false;
	int		m_spikeno = -1;
	int		m_spikenoclass = 0;
	float	m_txyright = 1.f;
	float	m_txyleft = 0.f;
	float   m_mVbin = .1f;

	CdbWaveDoc* GetDocument();

	// Attributes
protected:
	CStretchControl m_stretch{};
	BOOL			m_binit = false;

	CChartSpikeHist	yhistogram_wnd_;	// spike histogram
	CChartSpikeXYWnd	xygraph_wnd_;		// bars with spike height
	CChartSpikeShapeWnd	spikeshape_wnd_;	// spike shapes
	CChartSpikeBarWnd	spikebars_wnd_;		// bars with spike height

	CEditCtrl	mm_t1;
	CEditCtrl	mm_t2;
	CEditCtrl	mm_lower;
	CEditCtrl	mm_upper;
	CEditCtrl	mm_sourceclass;
	CEditCtrl	mm_destinationclass;
	CEditCtrl	mm_timeFirst;
	CEditCtrl	mm_timeLast;
	CEditCtrl	mm_mVMin;
	CEditCtrl	mm_mVMax;
	CEditCtrl	mm_spikeno;
	CEditCtrl	mm_spikenoclass;
	CEditCtrl	mm_txyright;
	CEditCtrl	mm_txyleft;
	CEditCtrl   mm_mVbin;

	CSpikeDoc* m_pSpkDoc = nullptr;
	CSpikeList* m_pSpkList = nullptr;
	SPKCLASSIF* m_psC{};
	OPTIONS_VIEWDATA* m_pOptionsViewData{};
	CSpkListTabCtrl	 m_tabCtrl{};

	int		m_itaglow{};
	int		m_itagup{};
	int		m_ixyright{};
	int		m_ixyleft{};

	int		m_spkhistupper{};
	int		m_spkhistlower{};
	int		m_spkformtagleft{};
	int		m_spkformtagright{};

	float	m_tunit = 1000.f;		// 1=s, 1000f=ms, 1e6=us
	float	m_vunit = 1000.f;		// 1=V, 1000f=mV, 1e6=uV
	float	m_delta{};
	int		m_parmmax{};			// max of array m_measure_y1
	int		m_parmmin{};			// min of array m_measure_y1
	BOOL	m_bvalidextrema = false; // tells if m_parmmax & m_parmmin are valid
	long	m_lFirst{};				// display first
	long	m_lLast{};				// display last
	BOOL	m_bMeasureDone = false;	// flag m_measure_y1 valid
	int		m_divAmplitudeBy = 1;	// value to adjust changes in amplitude / filter(s)

	SCROLLINFO	m_scroll_file_pos_infos_{};

	// Operations
public:
	void SetViewMouseCursor(int cursormode) {
		xygraph_wnd_.SetMouseCursorType(cursormode);
		spikeshape_wnd_.SetMouseCursorType(cursormode);
	}

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewData)
public:
	CDaoRecordset* OnGetRecordset() override;
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void DefineSubClassedItems();
	void DefineStretchParameters();
	void OnInitialUpdate() override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;
	//}}AFX_VIRTUAL

	// Implementation
protected:
	virtual ~CViewSpikeSort();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
# endif

	void UpdateFileParameters();
	void UpdateSpikeFile();
	void UpdateLegends();
	void SaveCurrentFileParms();

	void SelectSpikeFromCurrentList(int spikeno);
	void UpdateGain();
	void UpdateScrollBar();
	
	void SelectSpkList(int icursel);
	void ActivateMode4();
	void BuildHistogram();
	void UnflagAllSpikes();

	// Generated message map functions
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeParameter();
	afx_msg void OnSort();
	afx_msg LRESULT OnMyMessage(WPARAM code, LPARAM lParam);
	afx_msg void OnMeasure();
	afx_msg void OnFormatAlldata();

	afx_msg void OnFormatCentercurve();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnToolsEdittransformspikes();
	afx_msg void OnSelectAllFiles();
	afx_msg void OnToolsAlignspikes();
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnEnChangelower();
	afx_msg void OnEnChangeupper();
	afx_msg void OnEnChangeT1();
	afx_msg void OnEnChangeT2();
	afx_msg void OnEnChangeSourceclass();
	afx_msg void OnEnChangeDestinationclass();
	afx_msg void OnEnChangetimeFirst();
	afx_msg void OnEnChangetimeLast();
	afx_msg void OnEnChangemVMin();
	afx_msg void OnEnChangemVMax();
	afx_msg void OnEnChangeNOspike();
	afx_msg void OnEnChangeSpikenoclass();
	afx_msg void OnEnChangeEditleft2();
	afx_msg void OnEnChangeEditright2();
	afx_msg void OnEnChangeNBins();

	afx_msg void OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* CViewSpikeSort::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif
