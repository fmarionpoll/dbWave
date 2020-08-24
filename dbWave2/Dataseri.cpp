// dataseri.cpp : implementation file
//

#include "StdAfx.h"
//#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Dataseri.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataSeriesFormatDlg dialog

CDataSeriesFormatDlg::CDataSeriesFormatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataSeriesFormatDlg::IDD, pParent), m_plineview(nullptr), m_pdbDoc(nullptr), m_listindex(0), m_yzero(0),
	m_yextent(0), m_mVperbin(0)
{
	m_maxmv = 0.0f;
	m_minmv = 0.0f;
	m_binzero = 2048;
}

void CDataSeriesFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTSERIES, m_listseries);
	DDX_Text(pDX, IDC_MAXMV, m_maxmv);
	DDX_Text(pDX, IDC_MINMV, m_minmv);
	DDX_Control(pDX, IDC_MFCCOLORBUTTON1, m_colorbutton);
}

BEGIN_MESSAGE_MAP(CDataSeriesFormatDlg, CDialog)
	ON_LBN_DBLCLK(IDC_LISTSERIES, OnSelchangeListseries)
	ON_LBN_SELCHANGE(IDC_LISTSERIES, OnSelchangeListseries)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataSeriesFormatDlg message handlers

void CDataSeriesFormatDlg::OnOK()
{
	UpdateData(TRUE);                              // transfer data to controls
	SetParams(m_listseries.GetCurSel());
	CDialog::OnOK();
}

void CDataSeriesFormatDlg::GetParams(int index)
{
	m_yzero = m_plineview->GetChanlistYzero(index);
	m_yextent = m_plineview->GetChanlistYextent(index);
	const auto color = m_plineview->GetChanlistColor(index);
	m_colorbutton.SetColor(color);
	m_mVperbin = m_plineview->GetChanlistVoltsperDataBin(index) * 1000.0f;
	m_binzero = 0; // m_dbDoc->m_pDataFile->GetpWaveFormat()->binzero;
	m_maxmv = (m_yextent / 2.f + m_yzero - m_binzero) * m_mVperbin;
	m_minmv = (-m_yextent / 2.f + m_yzero - m_binzero) * m_mVperbin;
}

void CDataSeriesFormatDlg::SetParams(const int index)
{
	m_yzero = static_cast<int>((m_maxmv + m_minmv) / (m_mVperbin * 2.0f)) + m_binzero;
	m_yextent = static_cast<int>((m_maxmv - m_minmv) / m_mVperbin);
	m_plineview->SetChanlistYzero(index, m_yzero);
	m_plineview->SetChanlistYextent(index, m_yextent);
	const auto ccolor = m_colorbutton.GetColor();
	auto icolor = m_plineview->FindColor(ccolor);
	if (icolor < 0)
	{
		icolor = NB_COLORS - 1;
		m_plineview->SetColor(icolor, ccolor);
	}
	m_plineview->SetChanlistColor(index, icolor);
}
void CDataSeriesFormatDlg::OnCancel()
{
	// TODO: Add extra cleanup here
	CDialog::OnCancel();
}

BOOL CDataSeriesFormatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	/*

	for (int i=0; i<NB_COLORS; i++)
		m_colors.AddColorItem( m_plineview->GetColor(i));
	*/

	// load channel description CComboBox
	const auto chanmax = m_plineview->GetChanlistSize();
	for (auto i = 0; i < chanmax; i++)
		m_listseries.AddString(m_plineview->GetChanlistComment(i));

	// select...
	GetParams(m_listindex);
	UpdateData(FALSE);
	m_listseries.SetCurSel(m_listindex);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDataSeriesFormatDlg::OnSelchangeListseries()
{
	UpdateData(TRUE);                              // transfer data to controls
	const auto listindex = m_listseries.GetCurSel();
	SetParams(m_listindex);
	m_listindex = listindex;
	GetParams(listindex);
	UpdateData(FALSE);                              // transfer data to controls
}