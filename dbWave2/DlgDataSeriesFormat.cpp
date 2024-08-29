#include "StdAfx.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "DlgDataSeriesFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgDataSeriesFormat::DlgDataSeriesFormat(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgDataSeriesFormat::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_LISTSERIES, m_list_series);
	DDX_Text(p_dx, IDC_MAXMV, m_max_mv);
	DDX_Text(p_dx, IDC_MINMV, m_min_mv);
	DDX_Control(p_dx, IDC_MFCCOLORBUTTON1, m_color_button);
}

BEGIN_MESSAGE_MAP(DlgDataSeriesFormat, CDialog)
	ON_LBN_DBLCLK(IDC_LISTSERIES, on_sel_change_list_series)
	ON_LBN_SELCHANGE(IDC_LISTSERIES, on_sel_change_list_series)
END_MESSAGE_MAP()

void DlgDataSeriesFormat::OnOK()
{
	UpdateData(TRUE);
	set_params(m_list_series.GetCurSel());
	CDialog::OnOK();
}

void DlgDataSeriesFormat::get_params(int index)
{
	CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(index);
	m_y_zero = chan->get_y_zero();
	m_y_extent = chan->get_y_extent();
	const auto color = chan->get_color_index();
	m_color_button.SetColor(color);
	m_mv_per_bin = chan->get_volts_per_bin() * 1000.0f;
	m_bin_zero = 0;
	m_max_mv = (m_y_extent / 2.f + m_y_zero - m_bin_zero) * m_mv_per_bin;
	m_min_mv = (-m_y_extent / 2.f + m_y_zero - m_bin_zero) * m_mv_per_bin;
}

void DlgDataSeriesFormat::set_params(const int index)
{
	CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(index);
	m_y_zero = static_cast<int>((m_max_mv + m_min_mv) / (m_mv_per_bin * 2.0f)) + m_bin_zero;
	m_y_extent = static_cast<int>((m_max_mv - m_min_mv) / m_mv_per_bin);
	chan->set_y_zero(m_y_zero);
	chan->set_y_extent(m_y_extent);
	const auto c_color = m_color_button.GetColor();
	auto i_color = ChartData::find_color_index(c_color);
	if (i_color < 0)
	{
		i_color = NB_COLORS - 1;
		ChartData::set_color_table_at(i_color, c_color);
	}
	chan->set_color(i_color);
}

void DlgDataSeriesFormat::OnCancel()
{
	// TODO: Add extra cleanup here
	CDialog::OnCancel();
}

BOOL DlgDataSeriesFormat::OnInitDialog()
{
	CDialog::OnInitDialog();

	// load channel description CComboBox
	const auto chan_max = m_p_chart_data_wnd->get_channel_list_size();
	for (auto i = 0; i < chan_max; i++)
		m_list_series.AddString(m_p_chart_data_wnd->get_channel_list_item(i)->get_comment());

	// select...
	get_params(m_list_index);
	UpdateData(FALSE);
	m_list_series.SetCurSel(m_list_index);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgDataSeriesFormat::on_sel_change_list_series()
{
	UpdateData(TRUE); // transfer data to controls
	const auto list_index = m_list_series.GetCurSel();
	set_params(m_list_index);
	m_list_index = list_index;
	get_params(list_index);
	UpdateData(FALSE); // transfer data to controls
}
