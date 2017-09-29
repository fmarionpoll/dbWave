#pragma once

// spikesort1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpikeSort1View form view


class CSpikeSort1View : public CDaoRecordView
{
	DECLARE_DYNCREATE(CSpikeSort1View)
protected:
	CSpikeSort1View();			// protected constructor used by dynamic creation

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
	CSpikeHistWnd	m_spkHist;		// source data
	CSpikeXYpWnd	m_spkXYp;		// bars with spike height
	CSpikeShapeWnd	m_spkForm;		// all spikes in displayspikes
	CSpikeBarWnd	m_spkBarView;	// bars with spike height

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

	CSpikeDoc*	m_pSpkDoc;			// document
	CSpikeList*	m_pSpkList;			// spike list
	SPKCLASSIF*	m_psC;
	OPTIONS_VIEWDATA*  mdPM;		// view data options

	int		m_itaglow;
	int		m_itagup;
	int		m_ixyright;
	int		m_ixyleft;
	
	int		m_spkhistupper;
	int		m_spkhistlower;	
	int		m_spkformtagleft;
	int		m_spkformtagright;
	
	float	m_tunit;				// 1=s, 1000f=ms, 1e6=us
	float	m_vunit;				// 1=V, 1000f=mV, 1e6=uV
	float	m_delta;
	int		m_parmmax;				// max of array m_parm
	int		m_parmmin;				// min of array m_parm
	BOOL	m_bvalidextrema;		// tells if m_parmmax & m_parmmin are valid
	long	m_lFirst;				// display first
	long	m_lLast;				// display last
	BOOL	m_bMeasureDone;			// flag m_parm valid
	int		m_divAmplitudeBy;		// value to adjust changes in amplitude / filter(s)

	CWordArray  m_parm;				// parameter value
	CDWordArray m_parm2;
	CWordArray  m_class;			// spike class
	CWordArray	m_nspkperfile;		// number of spikes per file
	CDWordArray m_iitime;			// time index
	SCROLLINFO	m_scrollFilePos_infos;

// Operations
public:
	inline void SetViewMouseCursor(int cursormode) {
					m_spkXYp.SetMouseCursorType(cursormode);
					m_spkForm.SetMouseCursorType(cursormode);}

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataView)
	public:
	virtual CDaoRecordset* OnGetRecordset();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);   	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);	    	
	//}}AFX_VIRTUAL

	// Implementation
protected:
	virtual ~CSpikeSort1View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void UpdateFileParameters();
	void UpdateLegends();
	void SaveCurrentFileParms();
	BOOL MeasureSpkParm1(BOOL bkeepOldData, int ioption, int currentfile);
	void MeasureSpkParm2(BOOL bkeepOldData, int ioption, int currentfile);
	void MeasureSpkParm4(BOOL bkeepOldData, int ioption, int currentfile);
	void SelectSpike(int spikeno, BOOL bglobal=FALSE);
	int  GlobalIndextoLocal(int index, int* filenb);
	int	 LocalIndextoGlobal(int filenb, int index);	
	void UpdateGain();
	void UpdateScrollBar();
	void SelectSpkList(int icur);
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
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);	
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
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonClickedTab(NMHDR* pNMHDR, LRESULT* pResult) ;
	afx_msg void OnEnChangeEditleft2();
	afx_msg void OnEnChangeEditright2();
	
	DECLARE_MESSAGE_MAP()
	CTabCtrl m_tabCtrl;
	afx_msg void OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in dataView.cpp
	inline CdbWaveDoc* CSpikeSort1View::GetDocument()
						{ return (CdbWaveDoc*)m_pDocument; }
#endif
