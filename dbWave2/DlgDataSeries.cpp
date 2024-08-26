

#include "StdAfx.h"
#include "ChartData.h"
#include "resource.h"
#include "DlgDataSeries.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgDataSeries::DlgDataSeries(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgDataSeries::DoDataExchange(CDataExchange* pDX)
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

BEGIN_MESSAGE_MAP(DlgDataSeries, CDialog)
	ON_LBN_SELCHANGE(IDC_LISTSERIES, OnSelchangeListseries)
	ON_BN_CLICKED(IDC_DELETESERIES, OnClickedDeleteseries)
	ON_BN_CLICKED(IDC_DEFINESERIES, OnClickedDefineseries)
	ON_CBN_SELCHANGE(IDC_TRANSFORM, OnSelchangeTransform)
	ON_LBN_DBLCLK(IDC_LISTSERIES, OnSelchangeListseries)
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
END_MESSAGE_MAP()

BOOL DlgDataSeries::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_listseries.AddString(_T("New"));
	int chanmax = m_pChartDataWnd->get_channel_list_size();
	int i = 0;
	for (i = 0; i < chanmax; i++)
		m_listseries.AddString(m_pChartDataWnd->get_channel_list_item(i)->get_comment());

	// doc channel comments
	chanmax = (m_pdbDoc->get_wave_format())->scan_count;
	const auto pchan_array = m_pdbDoc->get_wave_channels_array();
	for (i = 0; i < chanmax; i++)
		m_ordinates.AddString(pchan_array->get_p_channel(i)->am_csComment);

	// doc transferts allowed
	chanmax = m_pdbDoc->get_transforms_count();
	for (i = 0; i < chanmax; i++)
		m_transform.AddString(m_pdbDoc->get_transform_name(i));

	// select...
	m_listseries.SetCurSel(1);
	OnSelchangeListseries();
	OnSelchangeTransform();
	return TRUE;
}

void DlgDataSeries::OnSelchangeListseries()
{
	auto b_delete_series = FALSE;
	m_listindex = m_listseries.GetCurSel() - 1;
	if (m_listindex >= 0) // if lineview type channel
	{
		// select corresp source chan & transform mode
		CChanlistItem* chan = m_pChartDataWnd->get_channel_list_item(m_listindex);
		m_listseries.GetText(m_listindex + 1, m_name); // chan comment
		auto i = chan->get_source_chan(); // data source chan
		m_ordinates.SetCurSel(i); // and transform mode
		i = chan->get_transform_mode();
		m_transform.SetCurSel(i);
		b_delete_series = TRUE; // Un-enable "define new series button"
	}

	m_deleteseries.EnableWindow(b_delete_series); // update button state
	UpdateData(FALSE); // transfer data to controls
}

void DlgDataSeries::OnClickedDeleteseries()
{
	if (m_pChartDataWnd->remove_channel_list_item(m_listindex))
	{
		m_listseries.DeleteString(m_listindex + 1);
		m_listseries.SetCurSel(m_listindex);
		m_pChartDataWnd->Invalidate();
	}
	OnSelchangeListseries();
}

void DlgDataSeries::OnClickedDefineseries()
{
	UpdateData(TRUE); // load data from controls
	m_listindex = m_listseries.GetCurSel() - 1;
	const auto ns = m_ordinates.GetCurSel(); // get data doc source channel
	const auto mode = m_transform.GetCurSel(); // get transformation mode

	// modify current series
	if (m_listindex >= 0)
	{
		m_pChartDataWnd->set_channel_list_source_channel(m_listindex, ns);
		m_pChartDataWnd->set_channel_list_transform_mode(m_listindex, mode);
	}
	// or create new series
	else
	{
		const auto i = m_pChartDataWnd->add_channel_list_item(ns, mode);
		if (i >= 0) // new channel created? yes
		{
			CChanlistItem* chan = m_pChartDataWnd->get_channel_list_item(i);
			m_name = chan->get_comment();
			m_listseries.AddString(m_name); // controls' variables
			m_listindex = i; // update current index
			m_listseries.SetCurSel(m_listindex + 1); // select new channel
		}
	}

	// cope with the changes: display, adjust curve
	m_pChartDataWnd->get_data_from_doc(); // load data from document
	int max, min;
	CChanlistItem* chan = m_pChartDataWnd->get_channel_list_item(m_listindex);
	chan->get_max_min(&max, &min);
	chan->set_y_zero((max + min) / 2);
	chan->set_y_extent(static_cast<int>((max - min + 1) * 1.2));
	m_pChartDataWnd->Invalidate();

	OnSelchangeListseries();
}

void DlgDataSeries::OnOK()
{
	OnClickedDefineseries(); // take into account last changes
	m_listindex = m_listseries.GetCurSel() - 1;
	if (m_listindex < 0)
		m_listindex = 0;
	m_pChartDataWnd->get_data_from_doc();
	m_pChartDataWnd->Invalidate();
	CDialog::OnOK();
}

void DlgDataSeries::OnCancel()
{
	m_listindex = m_listseries.GetCurSel() - 1;
	if (m_listindex < 0)
		m_listindex = 0;
	CDialog::OnCancel();
}

void DlgDataSeries::OnSelchangeTransform()
{
	const auto i = m_transform.GetCurSel();
	m_ispan = m_pdbDoc->get_transformed_data_span(i);
	const auto b_change = m_pdbDoc->is_wb_span_change_allowed(i);
	const auto nshow = (b_change == 1) ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_EDIT1)->ShowWindow(nshow);
	GetDlgItem(IDC_STATIC1)->ShowWindow(nshow);

	UpdateData(FALSE);
}

void DlgDataSeries::OnEnChangeEdit1()
{
	const auto i = m_transform.GetCurSel();
	//int oldspan = m_pdbDoc->GetTransfDataSpan(i);
	m_ispan = GetDlgItemInt(IDC_EDIT1);
	if (m_ispan < 0)
	{
		m_ispan = 0;
		SetDlgItemInt(IDC_EDIT1, m_ispan);
	}
	m_pdbDoc->set_wb_transform_span(i, m_ispan);
}
