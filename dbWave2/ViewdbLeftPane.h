#pragma once

// dbLeftPaneView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CdbLeftPaneView DAO record view

class CdbWaveDoc;

class CdbLeftPaneView : public CDaoRecordView
{
protected:
	CdbLeftPaneView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CdbLeftPaneView)

// Form Data
public:
	enum { IDD = IDD_VIEWPANELEFT1 };
	CdbMainTable*	m_pSet;
	CComboBox		m_ctllocation;
	CComboBox		m_ctlconc;
	CComboBox		m_ctlsensillum;
	CComboBox		m_ctlinsectname;
	CComboBox		m_ctlstim;
	CComboBox		m_ctlOperator;
	CComboBox		m_ctlstim2;
	CComboBox		m_ctlconc2;
	CComboBox		m_ctlSensillumID;
	CComboBox		m_ctlinsectID;
	CComboBox		m_ctlstrain;
	CComboBox		m_ctlsex;
	CComboBox		m_ctlrepeat;
	CComboBox		m_ctlrepeat2;
	CComboBox		m_ctlflag;
	CComboBox		m_ctldate;
	CComboBox		m_ctlexpt;

// Attributes
public:
	CdbWaveDoc*		GetDocument();
	BOOL			m_bUpdateFromDoc;

// Operations
public:
// Overrides
		// NOTE - the ClassWizard will add and remove member functions here.
	public:
	virtual CDaoRecordset* OnGetRecordset();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMove(UINT nIDMoveCommand);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);


// Implementation
protected:
	virtual ~CdbLeftPaneView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
protected:
	CStretchControl m_stretch;
	CBitmapButton	myButton;
	BOOL			m_binit;
	
	BOOL	m_bAddMode;
	int		m_dattransform;

	void	PopulateControls();
	void	PopulateComboFromLinkedTable (CComboBox* pcombo, CDaoRecordset& mSet, /*long& max,*/ LPCTSTR csColName);
	void	PopulateComboFromTable (CComboBox* pcombo, int indexcol);
	void	PopulateComboFromTablewithDate (CComboBox* pcombo, int indexcol);

	void	OnMoveUpdateControls(void* piID, CComboBox& cb, BOOL bDate);
	void	UpdateControls();
	void	DlgUpdateCombo(int IDC);
	void	GotoRecordID(long recordID);

	void	OnFilter_Item(int IDC);
	void	OnEnChange_Item(int IDC);
	void	OnEditChange_Item_LinkedList(int IDC);
	DB_ITEMDESC* GetItemDescriptors(int IDC);

	// Generated message map functions
	afx_msg void OnDestroy();

	afx_msg void OnFilter_insectName();
	afx_msg void OnFilter_sensillum();
	afx_msg void OnFilter_location();
	afx_msg void OnFilter_stimulus();
	afx_msg void OnFilter_concentration();
	afx_msg void OnFilter_operator();
	afx_msg void OnFilter_insectID();
	afx_msg void OnFilter_sensillumID();
	afx_msg void OnFilter_stimulus2();
	afx_msg void OnFilter_concentration2();
	afx_msg void OnFilter_flag();
	afx_msg void OnFilter_sex();
	afx_msg void OnFilter_strain();
	afx_msg void OnFilter_repeat();
	afx_msg void OnFilter_repeat2();
	afx_msg void OnFilter_date();
	afx_msg void OnFilter_expt();
	
	afx_msg void OnEnChange_insectName();
	afx_msg void OnEnChange_sensillum();
	afx_msg void OnEnChange_stimulus();
	afx_msg void OnEnChange_concentration();
	afx_msg void OnEnChange_operator();
	afx_msg void OnEnChange_location();
	afx_msg void OnEnChange_insectID();
	afx_msg void OnEnChange_sensillumID();
	afx_msg void OnEnChange_stimulus2();
	afx_msg void OnEnChange_concentration2();
	afx_msg void OnEnChange_strain();
	afx_msg void OnEnChange_sex();
	afx_msg void OnEnChange_repeat();
	afx_msg void OnEnChange_repeat2();
	afx_msg void OnEnChange_flag();
	afx_msg void OnEnChange_date();
	afx_msg void OnEnChange_expt();
	
	afx_msg void OnEditChange_insectname();
	afx_msg void OnEditChange_sensillum();
	afx_msg void OnEditChange_concentration();
	afx_msg void OnEditChange_operator();
	afx_msg void OnEditChange_stimulus();
	afx_msg void OnEditChange_location();
	afx_msg void OnEditChange_insectID();
	afx_msg void OnEditChange_sensillumID();
	afx_msg void OnEditChange_concentration2();
	afx_msg void OnEditChange_stimulus2();
	afx_msg void OnEditChange_sex();
	afx_msg void OnEditChange_strain();
	afx_msg void OnEditChange_repeat();
	afx_msg void OnEditChange_repeat2();
	afx_msg void OnEditChange_flag();
	afx_msg void OnEditChange_date();
	afx_msg void OnEditChange_expt();
	
	afx_msg void OnRecordAdd();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnBnClickedChecksort();
	afx_msg void OnCbnSelchangeSort();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonrepeat3();
};

#ifndef _DEBUG  // debug version in dbWaveView.cpp
inline CdbWaveDoc* CdbLeftPaneView::GetDocument()
   { return (CdbWaveDoc*)m_pDocument; }
#endif
