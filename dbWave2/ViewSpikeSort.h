#pragma once
#include "spikehistp.h"
#include "spikexyp.h"
#include "spikeshape.h"
#include "dbWaveDoc.h"
#include "Spikebar.h"

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
	
	float	m_t1;
	float	m_t2;
	float	m_lower;
	float	m_upper;
	int		m_sourceclass;
	int		m_destinationclass;
	float	m_timeFirst;
	float	m_timeLast;
	float	m_mVMax;
	float	m_mVMin;
	BOOL	m_bAllfiles;
	int		m_spikeno;
	int		m_spikenoclass;
	float	m_txyright;
	float	m_txyleft;

	CdbWaveDoc*	GetDocument();

// Attributes
protected:
	CStretchControl m_stretch;		// array of properties associated with controls
	BOOL			m_binit;
	CSpikeHistWnd	spk_hist_wnd_;	// spike histogram
	CSpikeXYpWnd	spk_xy_wnd_;	// bars with spike height
	CSpikeShapeWnd	spk_shape_wnd_;	// spike shapes
	CSpikeBarWnd	spk_bar_wnd_;	// bars with spike height

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

	CSpikeDoc*	m_pSpkDoc;	
	CSpikeList*	m_pSpkList;	
	SPKCLASSIF*	m_psC{};
	OPTIONS_VIEWDATA*  mdPM{};

	int		m_itaglow{};
	int		m_itagup{};
	int		m_ixyright{};
	int		m_ixyleft{};
	
	int		m_spkhistupper{};
	int		m_spkhistlower{};	
	int		m_spkformtagleft{};
	int		m_spkformtagright{};
	
	float	m_tunit;				// 1=s, 1000f=ms, 1e6=us
	float	m_vunit;				// 1=V, 1000f=mV, 1e6=uV
	float	m_delta{};
	int		m_parmmax{};			// max of array m_measure_y1
	int		m_parmmin{};			// min of array m_measure_y1
	BOOL	m_bvalidextrema;		// tells if m_parmmax & m_parmmin are valid
	long	m_lFirst{};				// display first
	long	m_lLast{};				// display last
	BOOL	m_bMeasureDone;			// flag m_measure_y1 valid
	int		m_divAmplitudeBy;		// value to adjust changes in amplitude / filter(s)

	CArray<int, int>	m_measure_y1_;		// measure: amplitude t2-t1, val at t1, etc
	CArray<long, long>	m_measure_y2_;		// measure of a second parameter
	CArray<int, int>	m_measure_class_;	// spike class
	CArray<int, int>	m_nspkperfile_;		// number of spikes per file
	CArray<long, long>	m_measure_t_;		// time index
	SCROLLINFO	m_scroll_file_pos_infos_{};

// Operations
public:
	void SetViewMouseCursor(int cursormode) {
					spk_xy_wnd_.SetMouseCursorType(cursormode);
					spk_shape_wnd_.SetMouseCursorType(cursormode);}

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewData)
public:
	CDaoRecordset* OnGetRecordset() override;
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void OnInitialUpdate() override; // called first time after construct
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;	    	
	//}}AFX_VIRTUAL

	// Implementation
protected:
	virtual ~CViewSpikeSort();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	void UpdateFileParameters();
	void UpdateLegends();
	void SaveCurrentFileParms();
	BOOL MeasureSpkParm1(BOOL bkeepOldData, int ioption, int currentfile);
	void MeasureSpkParm2(BOOL bkeepOldData, int ioption, int currentfile);
	void MeasureSpkParm4(BOOL bkeepOldData, int ioption, int currentfile);
	void SelectSpike(int spikeno);
	int  GlobalIndextoLocal(int index_global, int* filenb);
	int LocalIndextoGlobal(int filenb, int index_local) {return index_local + m_nspkperfile_[filenb];};
	int SelectFileFromGlobalSpikeIndex(int global_index);
	void UpdateGain();
	void UpdateScrollBar();
	void SelectSpkList(int icursel);
	void ActivateMode4();

	// Generated message map functions
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeSourceclass();
	afx_msg void OnEnChangeDestinationclass();
	afx_msg void OnSelchangeParameter();
	afx_msg void OnEnChangelower();
	afx_msg void OnEnChangeupper();
	afx_msg void OnEnChangeT1();
	afx_msg void OnEnChangeT2();
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
	afx_msg void OnEnChangetimeFirst();
	afx_msg void OnEnChangetimeLast();
	afx_msg void OnEnChangemVMin();
	afx_msg void OnEnChangemVMax();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeNOspike();
	afx_msg void OnEnChangeSpikenoclass();
	afx_msg void OnEnChangeEditleft2();
	afx_msg void OnEnChangeEditright2();
	
	DECLARE_MESSAGE_MAP()
	CTabCtrl m_tabCtrl;
	afx_msg void OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in dataView.cpp
	inline CdbWaveDoc* CViewSpikeSort::GetDocument()
						{ return (CdbWaveDoc*)m_pDocument; }
#endif
