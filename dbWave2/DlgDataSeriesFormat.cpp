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
	ON_BN_CLICKED(IDC_MFCCOLORBUTTON1, &DlgDataSeriesFormat::on_bn_clicked_mfc_color_button1)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void DlgDataSeriesFormat::OnOK()
{
	UpdateData(TRUE);
	set_params(m_list_series.GetCurSel());
	CDialog::OnOK();
}

void  DlgDataSeriesFormat::define_custom_palette()
{
	constexpr int color_table_length = std::size(ChartWnd::color_spike_class);
	const auto p_log_palette = reinterpret_cast<LOGPALETTE*>(new BYTE[sizeof(LOGPALETTE) +
		(color_table_length * sizeof(PALETTEENTRY))]);
	p_log_palette->palVersion = 0x300;
	p_log_palette->palNumEntries = color_table_length;

	for (int i = 0; i < color_table_length; i++)
	{
		const COLORREF current_color = ChartWnd::color_spike_class[i];
		p_log_palette->palPalEntry[i].peRed = GetRValue(current_color);
		p_log_palette->palPalEntry[i].peGreen = GetGValue(current_color);
		p_log_palette->palPalEntry[i].peBlue = GetBValue(current_color);
	}

	m_p_palette_ = new CPalette();
	m_p_palette_->CreatePalette(p_log_palette);
	delete[]p_log_palette;
}

void  DlgDataSeriesFormat::init_colors_button()
{
	constexpr int color_table_length = std::size(ChartWnd::color_spike_class);
	constexpr int i_color = 1;
	init_color_button(i_color);
}

void  DlgDataSeriesFormat::init_color_button(const int i_color)
{
	m_color_button.SetPalette(m_p_palette_);
	m_color_button.SetColumnsNumber(10);
}

void DlgDataSeriesFormat::get_params(const int index)
{
	const CChanlistItem* chan = m_p_chart_data_wnd->get_channel_list_item(index);
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
		i_color = std::size(ChartWnd::color_spike_class);
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

	define_custom_palette();
	init_colors_button();

	get_params(m_list_index);
	UpdateData(FALSE);
	m_list_series.SetCurSel(m_list_index);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgDataSeriesFormat::on_sel_change_list_series()
{
	UpdateData(TRUE); 
	const auto list_index = m_list_series.GetCurSel();
	set_params(m_list_index);
	m_list_index = list_index;
	get_params(list_index);
	UpdateData(FALSE); 
}

void DlgDataSeriesFormat::on_bn_clicked_mfc_color_button1()
{
	const auto list_index = m_list_series.GetCurSel();
	set_params(m_list_index);
}

void DlgDataSeriesFormat::OnDestroy()
{
	CDialog::OnDestroy();
	delete m_p_palette_;
}
