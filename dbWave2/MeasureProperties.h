#pragma once

// MeasureProperties.h : header file
//
// This class defines custom modal property sheet 
// CMeasureProperties.


/////////////////////////////////////////////////////////////////////////////
// CMeasureProperties

class CMeasureProperties : public CPropertySheet
{
	DECLARE_DYNAMIC(CMeasureProperties)

// Construction
public:
	CMeasureProperties(CWnd* pParentWnd = NULL, int iselect=0);

// Attributes
public:
	CMeasureOptionsPage m_Page1;
	CMeasureVTtagsPage	m_Page2;
	CMeasureHZtagsPage	m_Page3;
	CMeasureResultsPage m_Page4;

	// CMeasureResults
	CLineViewWnd*	m_plineview;
	CdbWaveDoc* 	m_pdbDoc;	
	CAcqDataDoc* 	m_pdatDoc;	

	OPTIONS_VIEWDATAMEASURE* m_pMO;
	int	m_currentchan;
	float m_samplingrate;

// Operations
public:

// Overrides
	public:
	virtual BOOL OnInitDialog();

// Implementation
public:
	virtual ~CMeasureProperties();

// Generated message map functions
protected:
	
	DECLARE_MESSAGE_MAP()
};

