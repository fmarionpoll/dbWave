// MeasureProperties.cpp : implementation file
//

#include "stdafx.h"
#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"
#include "editctrl.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"

#include "resource.h"
#include "MeasureOptions.h"
#include "MeasureProperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMeasureProperties

IMPLEMENT_DYNAMIC(CMeasureProperties, CPropertySheet)

CMeasureProperties::CMeasureProperties(CWnd* pWndParent, int iselect)
	 : CPropertySheet(IDS_PROPSHT_CAPTION, pWndParent)
{
	// One way to make a different property page the 
	// active one is to call SetActivePage().

	m_plineview = nullptr;
	AddPage(&m_Page3);	// HZ edit
	AddPage(&m_Page2);	// VT tags edit
	AddPage(&m_Page1);	// measure options
	AddPage(&m_Page4);	// measure results
	SetActivePage(iselect);
}

CMeasureProperties::~CMeasureProperties()
{
}


BEGIN_MESSAGE_MAP(CMeasureProperties, CPropertySheet)
	
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMeasureProperties message handlers

BOOL CMeasureProperties::OnInitDialog() 
{
	// vertical tags
	int ntags = m_plineview->GetNVTtags();
	m_Page2.m_plineview=m_plineview;
	m_Page2.m_pMO = m_pMO;
	m_Page2.m_samplingrate = m_samplingrate;
	m_Page2.m_pdatDoc=m_pdatDoc;
	m_Page2.m_pdbDoc=m_pdbDoc;

	// measure options
	m_Page1.m_pMO = m_pMO;
	m_Page1.m_plineview=m_plineview;	
	m_Page1.m_pdatDoc=m_pdatDoc;
	m_Page1.m_pdbDoc=m_pdbDoc;

	// HZ tags
	m_Page3.m_plineview=m_plineview;	
	m_Page3.m_pMO = m_pMO;
	m_Page3.m_pdatDoc=m_pdatDoc;
	m_Page3.m_pdbDoc=m_pdbDoc;

	// measure
	m_Page4.m_plineview= m_plineview;
	m_Page4.m_pdatDoc=m_pdatDoc;
	m_Page4.m_pdbDoc=m_pdbDoc;
	m_Page4.m_pMO=m_pMO;
	m_Page4.m_currentchan=m_currentchan;
	return CPropertySheet::OnInitDialog();
}
