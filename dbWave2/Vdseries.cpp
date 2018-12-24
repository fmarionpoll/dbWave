// vdseries.cpp : implementation file
//

#include "StdAfx.h"
//#include "Cscale.h"
//#include "scopescr.h"
#include "Lineview.h"
#include "resource.h"
#include "Vdseries.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDataSeriesDlg::CDataSeriesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataSeriesDlg::IDD, pParent), m_ispan(0), m_lineview(nullptr), m_pdbDoc(nullptr), m_listindex(0)
{
	m_name = "";
}

void CDataSeriesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DELETESERIES, m_deleteseries);
	DDX_Control(pDX, IDC_DEFINESERIES, m_defineseries);
	DDX_Control(pDX, IDC_TRANSFORM, m_transform);
	DDX_Control(pDX, IDC_ORDINATES, m_ordinates);
	DDX_Control(pDX, IDC_LISTSERIES, m_listseries);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_EDIT1, m_ispan);
}

BEGIN_MESSAGE_MAP(CDataSeriesDlg, CDialog)
	ON_LBN_SELCHANGE(IDC_LISTSERIES, OnSelchangeListseries)
	ON_BN_CLICKED(IDC_DELETESERIES, OnClickedDeleteseries)
	ON_BN_CLICKED(IDC_DEFINESERIES, OnClickedDefineseries)
	ON_CBN_SELCHANGE(IDC_TRANSFORM, OnSelchangeTransform)
	ON_LBN_DBLCLK(IDC_LISTSERIES, OnSelchangeListseries)
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
END_MESSAGE_MAP()

BOOL CDataSeriesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_listseries.AddString(_T("New"));	
	int chanmax = m_lineview->GetChanlistSize();
	int i=0;
	for (i = 0; i<chanmax; i++)
		m_listseries.AddString(m_lineview->GetChanlistComment(i));

	// doc channel comments
	chanmax = (m_pdbDoc->GetpWaveFormat())->scan_count;
	const auto pchan_array = m_pdbDoc->GetpWavechanArray();
	for (i = 0; i<chanmax; i++)
		m_ordinates.AddString(pchan_array->get_p_channel(i)->am_csComment);

	// doc transferts allowed
	chanmax = m_pdbDoc->GetTransfDataNTypes();
	for (i=0; i<chanmax; i++)
		m_transform.AddString(m_pdbDoc->GetTransfDataName(i));

	// select...
	m_listseries.SetCurSel(1);
	OnSelchangeListseries();
	OnSelchangeTransform();
	return TRUE;
} 

void CDataSeriesDlg::OnSelchangeListseries()
{
	auto b_delete_series=FALSE;
	m_listindex = m_listseries.GetCurSel()-1;
	if (m_listindex >= 0)		// if lineview type channel
	{							// select corresp source chan & transform mode
		m_listseries.GetText(m_listindex+1, m_name);			// chan comment
		auto i = m_lineview->GetChanlistSourceChan(m_listindex);	// data source chan
		m_ordinates.SetCurSel(i);								// and transform mode 
		i = m_lineview->GetChanlistTransformMode(m_listindex);
		m_transform.SetCurSel(i);
		b_delete_series=TRUE;										// Un-enable "define new series button"		
	}	

	m_deleteseries.EnableWindow(b_delete_series);	// update button state
	UpdateData(FALSE);                              // transfer data to controls
}

void CDataSeriesDlg::OnClickedDeleteseries()
{	
	if (m_lineview->RemoveChanlistItem(m_listindex))
	{
		m_listseries.DeleteString(m_listindex+1);
		m_listseries.SetCurSel(m_listindex);
		m_lineview->Invalidate();
	}
	OnSelchangeListseries();
}

void CDataSeriesDlg::OnClickedDefineseries()
{
	UpdateData(TRUE);					// load data from controls
	m_listindex = m_listseries.GetCurSel()-1;
	const auto ns = m_ordinates.GetCurSel();	// get data doc source channel
	const auto mode = m_transform.GetCurSel();	// get transformation mode
		
	// modify current series
	if (m_listindex >= 0)
	{
		m_lineview->SetChanlistSourceChan(m_listindex, ns);
		m_lineview->SetChanlistTransformMode(m_listindex, mode);
	}
	// or create new series
	else
	{
		const auto i = m_lineview->AddChanlistItem(ns, mode);
		if (i>= 0)							// new channel created? yes
		{			
			m_name = m_lineview->GetChanlistComment(i);
			m_listseries.AddString(m_name);		// controls' variables
			m_listindex = i;   					// update current index
			m_listseries.SetCurSel(m_listindex+1);	// select new channel
		}
	}

	// cope with the changes: display, adjust curve	
	m_lineview->GetDataFromDoc();	// load data from document
	int max, min;					// center the new curve
	m_lineview->GetChanlistMaxMin(m_listindex, &max, &min);		
	m_lineview->SetChanlistYzero(m_listindex, (max+min)/2);	
	m_lineview->SetChanlistYextent(m_listindex, static_cast<int>((max - min + 1) * 1.2));
	m_lineview->Invalidate();

	OnSelchangeListseries();	
}

void CDataSeriesDlg::OnOK()
{	
	OnClickedDefineseries(); 	// take into account last changes
	m_listindex = m_listseries.GetCurSel()-1;	
	if (m_listindex < 0)
		m_listindex = 0;
	m_lineview->GetDataFromDoc();
	m_lineview->Invalidate();
	CDialog::OnOK();
}

void CDataSeriesDlg::OnCancel()
{
	m_listindex = m_listseries.GetCurSel()-1;
	if (m_listindex < 0)
		m_listindex = 0;	
	CDialog::OnCancel();
}

void CDataSeriesDlg::OnSelchangeTransform()
{
	const auto i = m_transform.GetCurSel();
	m_ispan = m_pdbDoc->GetTransfDataSpan(i);
	const auto b_change = m_pdbDoc->IsWBSpanChangeAllowed(i);
	const auto nshow = (b_change == 1) ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_EDIT1)->ShowWindow(nshow);
	GetDlgItem(IDC_STATIC1)->ShowWindow(nshow);
	
	UpdateData(FALSE);
}

void CDataSeriesDlg::OnEnChangeEdit1() 
{
	const auto i = m_transform.GetCurSel();
	//int oldspan = m_pdbDoc->GetTransfDataSpan(i);
	m_ispan = GetDlgItemInt( IDC_EDIT1);
	if (m_ispan < 0)
	{
		m_ispan = 0;
		SetDlgItemInt( IDC_EDIT1, m_ispan);
	}
	m_pdbDoc->SetWBTransformSpan(i, m_ispan);
}
