

#include "StdAfx.h"
#include "ChartData.h"
#include "resource.h"
#include "DlgDataSeries.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgDataSeries::DlgDataSeries(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgDataSeries::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DELETESERIES, m_delete_series);
	DDX_Control(pDX, IDC_DEFINESERIES, m_define_series);
	DDX_Control(pDX, IDC_TRANSFORM, m_transform);
	DDX_Control(pDX, IDC_ORDINATES, m_ordinates);
	DDX_Control(pDX, IDC_LISTSERIES, m_list_series);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_EDIT1, m_i_span);
}

BEGIN_MESSAGE_MAP(DlgDataSeries, CDialog)
	ON_LBN_SELCHANGE(IDC_LISTSERIES, on_sel_change_list_series)
	ON_BN_CLICKED(IDC_DELETESERIES, on_clicked_delete_series)
	ON_BN_CLICKED(IDC_DEFINESERIES, on_clicked_define_series)
	ON_CBN_SELCHANGE(IDC_TRANSFORM, on_sel_change_transform)
	ON_LBN_DBLCLK(IDC_LISTSERIES, on_sel_change_list_series)
	ON_EN_CHANGE(IDC_EDIT1, on_en_change_edit1)
END_MESSAGE_MAP()

BOOL DlgDataSeries::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_series.AddString(_T("New"));
	int chan_max = m_p_chart_data_wnd->get_channel_list_size();
	int i = 0;
	for (i = 0; i < chan_max; i++)
		m_list_series.AddString(m_p_chart_data_wnd->get_channel_list_item(i)->get_comment());

	// doc channel comments
	chan_max = (m_pdb_doc->get_wave_format())->scan_count;
	const auto p_chan_array = m_pdb_doc->get_wave_channels_array();
	for (i = 0; i < chan_max; i++)
		m_ordinates.AddString(p_chan_array->get_p_channel(i)->am_csComment);

	// doc transfers allowed
	chan_max = AcqDataDoc::get_transforms_count();
	for (i = 0; i < chan_max; i++)
		m_transform.AddString(AcqDataDoc::get_transform_name(i));

	// select...
	m_list_series.SetCurSel(1);
	on_sel_change_list_series();
	on_sel_change_transform();
	return TRUE;
}

void DlgDataSeries::on_sel_change_list_series()
{
	auto b_delete_series = FALSE;
	m_list_index = m_list_series.GetCurSel() - 1;
	if (m_list_index >= 0) // if lineview type channel
	{
		// select corresponding source chan & transform mode
		const CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(m_list_index);
		m_list_series.GetText(m_list_index + 1, m_name); // chan comment
		auto i = chan->get_source_chan(); // data source chan
		m_ordinates.SetCurSel(i); // and transform mode
		i = chan->get_transform_mode();
		m_transform.SetCurSel(i);
		b_delete_series = TRUE; // Un-enable "define new series button"
	}

	m_delete_series.EnableWindow(b_delete_series); // update button state
	UpdateData(FALSE); // transfer data to controls
}

void DlgDataSeries::on_clicked_delete_series()
{
	if (m_p_chart_data_wnd->remove_channel_list_item(m_list_index))
	{
		m_list_series.DeleteString(m_list_index + 1);
		m_list_series.SetCurSel(m_list_index);
		m_p_chart_data_wnd->Invalidate();
	}
	on_sel_change_list_series();
}

void DlgDataSeries::on_clicked_define_series()
{
	UpdateData(TRUE); // load data from controls
	m_list_index = m_list_series.GetCurSel() - 1;
	const auto ns = m_ordinates.GetCurSel(); // get data doc source channel
	const auto mode = m_transform.GetCurSel(); // get transformation mode

	// modify current series
	if (m_list_index >= 0)
	{
		m_p_chart_data_wnd->set_channel_list_source_channel(m_list_index, ns);
		m_p_chart_data_wnd->set_channel_list_transform_mode(m_list_index, mode);
	}
	// or create new series
	else
	{
		const auto i = m_p_chart_data_wnd->add_channel_list_item(ns, mode);
		if (i >= 0) // new channel created? yes
		{
			CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(i);
			m_name = chan->get_comment();
			m_list_series.AddString(m_name); 
			m_list_index = i;
			m_list_series.SetCurSel(m_list_index + 1); 
		}
	}

	// cope with the changes: display, adjust curve
	m_p_chart_data_wnd->get_data_from_doc(); // load data from document
	int max, min;
	CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(m_list_index);
	chan->get_max_min(&max, &min);
	chan->set_y_zero((max + min) / 2);
	chan->set_y_extent(static_cast<int>((max - min + 1) * 1.2));
	m_p_chart_data_wnd->Invalidate();

	on_sel_change_list_series();
}

void DlgDataSeries::OnOK()
{
	on_clicked_define_series(); // take into account last changes
	m_list_index = m_list_series.GetCurSel() - 1;
	if (m_list_index < 0)
		m_list_index = 0;
	m_p_chart_data_wnd->get_data_from_doc();
	m_p_chart_data_wnd->Invalidate();
	CDialog::OnOK();
}

void DlgDataSeries::OnCancel()
{
	m_list_index = m_list_series.GetCurSel() - 1;
	if (m_list_index < 0)
		m_list_index = 0;
	CDialog::OnCancel();
}

void DlgDataSeries::on_sel_change_transform()
{
	const auto i = m_transform.GetCurSel();
	m_i_span = AcqDataDoc::get_transformed_data_span(i);
	const auto b_change = AcqDataDoc::is_wb_span_change_allowed(i);
	const auto nshow = (b_change == 1) ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_EDIT1)->ShowWindow(nshow);
	GetDlgItem(IDC_STATIC1)->ShowWindow(nshow);

	UpdateData(FALSE);
}

void DlgDataSeries::on_en_change_edit1()
{
	const auto i = m_transform.GetCurSel();
	m_i_span = GetDlgItemInt(IDC_EDIT1);
	if (m_i_span < 0)
	{
		m_i_span = 0;
		SetDlgItemInt(IDC_EDIT1, m_i_span);
	}
	AcqDataDoc::set_wb_transform_span(i, m_i_span);
}
