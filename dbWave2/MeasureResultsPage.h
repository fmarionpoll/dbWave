#pragma once
#include "dbWaveDoc.h"
#include "Acqdatad.h"
#include "Lineview.h"
#include "Editctrl.h"


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
	CAcqDataDoc* 			m_pdatDoc{};	
	OPTIONS_VIEWDATAMEASURE* m_pMO{};
	int	m_currentchan{};

protected:
	// locals
	BOOL  MeasureParameters();
	void  MeasureFromVTtags(int ichan);
	void  MeasureFromHZcur(int ichan);
	void  MeasureFromRect(int ichan);
	void  MeasureFromStim(int ichan);
	void  OutputTitle();
	//LPSTR OutputFileName(LPSTR lpCopy);	

	void  MeasureWithinInterval(int ichan, int line, long l1, long l2);
	void  MeasureBetweenHZ(int ichan, int line, int v1, int v2);
	void  GetMaxMin(int chan, long l_first, long l_last);
	short	m_max{};
	long	m_imax{};
	short	m_min{};
	long	m_imin{};
	short	m_first{};
	short	m_last{};
	TCHAR	m_szT[64]{};		// dummy characters buffer
	float	m_mVperBin{};
	int		m_nbdatacols{};
	int		m_col{};
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
