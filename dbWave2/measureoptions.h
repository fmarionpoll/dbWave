#pragma once

// CMeasureOptionsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMeasureOptionsPage dialog

class CMeasureOptionsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureOptionsPage)

// Construction
public:
	CMeasureOptionsPage();
	~CMeasureOptionsPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE1 };
	BOOL	m_bExtrema;
	BOOL	m_bDiffExtrema;
	BOOL	m_bDiffDatalimits;
	BOOL	m_bHalfrisetime;
	BOOL	m_bHalfrecovery;
	BOOL	m_bDatalimits;
	BOOL	m_bLimitsval;
	WORD	m_wSourcechan;
	WORD	m_wStimulusthresh;
	WORD	m_wStimuluschan;
	BOOL	m_bAllFiles;
	int		m_uiSourceChan;
	float	m_fStimulusoffset;
	UINT	m_uiStimulusThreshold;
	UINT	m_uiStimuluschan;
	// input parameters
	OPTIONS_VIEWDATAMEASURE* m_pMO;
	CdbWaveDoc* 	m_pdbDoc;	
	CAcqDataDoc* 	m_pdatDoc;	
	CLineViewWnd*	m_plineview;


// Overrides
	public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	void ShowLimitsParms(BOOL bShow);
	void ShowChanParm(BOOL bShow);
	void SaveOptions();
	
	// Generated message map functions
	afx_msg void OnAllchannels();
	afx_msg void OnSinglechannel();
	afx_msg void OnVerticaltags();
	afx_msg void OnHorizontaltags();
	afx_msg void OnStimulustag();	
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CMeasureVTtagsPage dialog

class CMeasureVTtagsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureVTtagsPage)

// Construction
public:
	CMeasureVTtagsPage();
	~CMeasureVTtagsPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE2 };
	int		m_index;
	int		m_nbtags;
	int		m_nperiods;
	float	m_period;
	float	m_timesec;
	float	m_timeshift;
	float	m_duration;

	// input parameters
	OPTIONS_VIEWDATAMEASURE* m_pMO;	// init on call
	CdbWaveDoc* 	m_pdbDoc;	
	CAcqDataDoc* 	m_pdatDoc;	
	CLineViewWnd*	m_plineview;
	float	m_samplingrate;
	float	m_verylast;	

	// locals
	CEditCtrl mm_index;
	CEditCtrl mm_fileindex;
	CEditCtrl mm_timesec;
	CEditCtrl mm_duration;
	CEditCtrl mm_period;
	CEditCtrl mm_nperiods;
	CEditCtrl mm_timeshift;

// Overrides
	public:
	virtual void OnCancel();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	BOOL GetVTtagVal(int index);
	void SetspacedTagsOptions();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnRemove();
	afx_msg void OnEnChangeItem();
	afx_msg void OnEnChangeTimesec();
	afx_msg void OnCheck1();
	afx_msg void OnSetDuplicateMode();
	afx_msg void OnEnChangeDuration();
	afx_msg void OnEnChangePeriod();
	afx_msg void OnEnChangeNperiodsedit();
	afx_msg void OnEnChangeTimeshift();
	afx_msg void OnShiftTags();
	afx_msg void OnAddTags();
	afx_msg void OnDeleteSeries();
	afx_msg void OnDeleteAll();
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CMeasureHZtagsPage dialog

class CMeasureHZtagsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureHZtagsPage)

// Construction
public:
	CMeasureHZtagsPage();
	~CMeasureHZtagsPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE3 };
	int		m_datachannel;
	int		m_index;
	float	m_mvlevel;
	int		m_nbcursors;

	// input parameters
	OPTIONS_VIEWDATAMEASURE* m_pMO;
	CdbWaveDoc* 	m_pdbDoc;	
	CAcqDataDoc* 	m_pdatDoc;	
	CLineViewWnd*	m_plineview;

	// locals
	CEditCtrl mm_index;
	CEditCtrl mm_datachannel;
	CEditCtrl mm_mvlevel;

// Overrides
	public:
	virtual void OnCancel();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL GetHZcursorVal(int index);

// Implementation
protected:
	// Generated message map functions
	afx_msg void OnCenter();
	afx_msg void OnAdjust();
	afx_msg void OnRemove();
	afx_msg void OnEnChangeDatachannel();
	afx_msg void OnEnChangeIndex();
	afx_msg void OnEnChangeMvlevel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDeleteAll();
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CMeasureResultsPage dialog

class CMeasureResultsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeasureResultsPage)

// Construction
public:
	CMeasureResultsPage();
	~CMeasureResultsPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE4 };
	CEdit	m_CEditResults;
	CListCtrl m_listResults;

	// input parameters
	CLineViewWnd*			m_plineview;
	CdbWaveDoc* 			m_pdbDoc;	
	CAcqDataDoc* 			m_pdatDoc;	
	OPTIONS_VIEWDATAMEASURE* m_pMO;
	int	m_currentchan;

protected:
	// locals
	BOOL  MeasureParameters();
	void  MeasureFromVTtags(int ichan);
	void  MeasureFromHZcur(int ichan);
	void  MeasureFromRect(int ichan);
	void  MeasureFromStim(int ichan);
	void  OutputTitle();
	LPSTR OutputFileName(LPSTR lpCopy);	

	void  MeasureWithinInterval(int ichan, int line, long l1, long l2);
	void  MeasureBetweenHZ(int ichan, int line, int v1, int v2);
	void  GetMaxMin(int chan, long lFirst, long lLast);
	short	m_max;
	long	m_imax;
	short	m_min;
	long	m_imin;
	short	m_first;
	short	m_last;
	TCHAR	m_szT[64];		// dummy characters buffer
	float	m_mVperBin;
	int		m_nbdatacols;
	int		m_col;
	CString	m_csTitle;

// Overrides
	public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	// Generated message map functions
	afx_msg void OnExport();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
