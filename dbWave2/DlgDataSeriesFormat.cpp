#include "StdAfx.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "DlgDataSeriesFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgDataSeriesFormat::DlgDataSeriesFormat(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgDataSeriesFormat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTSERIES, m_listseries);
	DDX_Text(pDX, IDC_MAXMV, m_maxmv);
	DDX_Text(pDX, IDC_MINMV, m_minmv);
	DDX_Control(pDX, IDC_MFCCOLORBUTTON1, m_colorbutton);
}

BEGIN_MESSAGE_MAP(DlgDataSeriesFormat, CDialog)
	ON_LBN_DBLCLK(IDC_LISTSERIES, OnSelchangeListseries)
	ON_LBN_SELCHANGE(IDC_LISTSERIES, OnSelchangeListseries)
END_MESSAGE_MAP()

void DlgDataSeriesFormat::OnOK()
{
	UpdateData(TRUE);
	SetParams(m_listseries.GetCurSel());
	CDialog::OnOK();
}

void DlgDataSeriesFormat::GetParams(int index)
{
	CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(index);
	m_yzero = chan->GetYzero();
	m_yextent = chan->GetYextent();
	const auto color = chan->GetColorIndex();
	m_colorbutton.SetColor(color);
	m_mVperbin = chan->GetVoltsperDataBin() * 1000.0f;
	m_binzero = 0; // m_dbDoc->m_pDataFile->GetpWaveFormat()->binzero;
	m_maxmv = (m_yextent / 2.f + m_yzero - m_binzero) * m_mVperbin;
	m_minmv = (-m_yextent / 2.f + m_yzero - m_binzero) * m_mVperbin;
}

void DlgDataSeriesFormat::SetParams(const int index)
{
	CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(index);
	m_yzero = static_cast<int>((m_maxmv + m_minmv) / (m_mVperbin * 2.0f)) + m_binzero;
	m_yextent = static_cast<int>((m_maxmv - m_minmv) / m_mVperbin);
	chan->SetYzero(m_yzero);
	chan->SetYextent(m_yextent);
	const auto ccolor = m_colorbutton.GetColor();
	auto icolor = m_pChartDataWnd->FindColorIndex(ccolor);
	if (icolor < 0)
	{
		icolor = NB_COLORS - 1;
		m_pChartDataWnd->SetColorTableAt(icolor, ccolor);
	}
	chan->SetColor(icolor);
}

void DlgDataSeriesFormat::OnCancel()
{
	// TODO: Add extra cleanup here
	CDialog::OnCancel();
}

BOOL DlgDataSeriesFormat::OnInitDialog()
{
	CDialog::OnInitDialog();

	/*

	for (int i=0; i<NB_COLORS; i++)
		m_colors.AddColorItem( m_plineview->GetColorIndex(i));
	*/

	// load channel description CComboBox
	const auto chanmax = m_pChartDataWnd->GetChanlistSize();
	for (auto i = 0; i < chanmax; i++)
		m_listseries.AddString(m_pChartDataWnd->GetChanlistItem(i)->GetComment());

	// select...
	GetParams(m_listindex);
	UpdateData(FALSE);
	m_listseries.SetCurSel(m_listindex);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgDataSeriesFormat::OnSelchangeListseries()
{
	UpdateData(TRUE); // transfer data to controls
	const auto listindex = m_listseries.GetCurSel();
	SetParams(m_listindex);
	m_listindex = listindex;
	GetParams(listindex);
	UpdateData(FALSE); // transfer data to controls
}
