// TODO : measure data and output to notedocview

#include "StdAfx.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "dbWaveDoc.h"
#include "resource.h"
#include "MeasureHZtagsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureHZtagsPage, CPropertyPage)

CMeasureHZtagsPage::CMeasureHZtagsPage() : CPropertyPage(IDD)
{
}

CMeasureHZtagsPage::~CMeasureHZtagsPage()
= default;

void CMeasureHZtagsPage::DoDataExchange(CDataExchange* p_dx)
{
	CPropertyPage::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_DATACHANNEL, m_data_channel);
	DDX_Text(p_dx, IDC_INDEX, m_index);
	DDX_Text(p_dx, IDC_MVLEVEL, m_mv_level);
	DDX_Text(p_dx, IDC_NBCURSORS, m_nb_cursors);
}

BEGIN_MESSAGE_MAP(CMeasureHZtagsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_CENTER, on_center)
	ON_BN_CLICKED(IDC_ADJUST, on_adjust)
	ON_BN_CLICKED(IDC_REMOVE, on_remove)
	ON_EN_CHANGE(IDC_DATACHANNEL, on_en_change_data_channel)
	ON_EN_CHANGE(IDC_INDEX, on_en_change_index)
	ON_EN_CHANGE(IDC_MVLEVEL, on_en_change_mv_level)
	ON_BN_CLICKED(IDC_BUTTON4, on_delete_all)
END_MESSAGE_MAP()

BOOL CMeasureHZtagsPage::get_hz_cursor_value(const int index)
{
	const BOOL flag = (m_nb_cursors > 0 && index < m_nb_cursors && index >= 0);
	GetDlgItem(IDC_ADJUST)->EnableWindow(flag);
	GetDlgItem(IDC_CENTER)->EnableWindow(flag);
	GetDlgItem(IDC_REMOVE)->EnableWindow(flag);

	if (index < 0 || index >= m_nb_cursors)
		return FALSE;
	m_index = index;
	m_data_channel = m_p_chart_data_wnd->hz_tags.get_channel(index);
	int k = m_p_chart_data_wnd->hz_tags.get_value_int(m_index);
	m_mv_level = m_p_chart_data_wnd->get_channel_list_item(m_data_channel)->convert_data_bins_to_volts(k) * 1000.f;

	return TRUE;
}

void CMeasureHZtagsPage::on_center()
{
	int max, min;
	const CChanlistItem* p_chan = m_p_chart_data_wnd->get_channel_list_item(m_data_channel);
	p_chan->get_max_min(&max, &min);
	const auto val = (max + min) / 2;
	m_p_chart_data_wnd->hz_tags.set_value_int(m_index, val);
	m_p_chart_data_wnd->Invalidate();
	m_mv_level = p_chan->convert_data_bins_to_volts(val) * 1000.f;
	UpdateData(FALSE);
}

void CMeasureHZtagsPage::on_remove()
{
	if (m_index >= 0 && m_index < m_nb_cursors)
	{
		m_p_chart_data_wnd->hz_tags.remove_tag(m_index);
		m_nb_cursors--;
	}
	if (m_index > m_nb_cursors - 1)
		m_index = m_nb_cursors;
	get_hz_cursor_value(m_index);
	m_p_chart_data_wnd->Invalidate();
	UpdateData(FALSE);
}

void CMeasureHZtagsPage::on_en_change_data_channel()
{
	if (mm_data_channel.m_b_entry_done)
	{
		mm_data_channel.on_en_change(this, m_data_channel, 1, -1);

		// update dependent parameters
		if (m_data_channel < 0)
			m_data_channel = 0;
		if (m_data_channel >= m_p_chart_data_wnd->get_channel_list_size())
			m_data_channel = m_p_chart_data_wnd->get_channel_list_size() - 1;
		if (m_nb_cursors > 0 && m_index >= 0 && m_index < m_nb_cursors)
		{
			m_p_chart_data_wnd->hz_tags.set_channel(m_index, m_data_channel);
			m_p_chart_data_wnd->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void CMeasureHZtagsPage::on_en_change_index()
{
	if (mm_index.m_b_entry_done)
	{
		mm_index.on_en_change(this, m_index, 1, -1);
		// update dependent parameters
		if (m_index >= m_nb_cursors)
			m_index = m_nb_cursors - 1;
		if (m_index < 0)
			m_index = 0;
		get_hz_cursor_value(m_index);
		UpdateData(FALSE);
	}
}

void CMeasureHZtagsPage::on_en_change_mv_level()
{
	if (mm_mv_level.m_b_entry_done)
	{
		mm_mv_level.on_en_change(this, m_mv_level, 1.f, -1.f);
		UpdateData(FALSE);
		if (m_nb_cursors > 0 && m_index >= 0 && m_index < m_nb_cursors)
		{
			const auto val = m_p_chart_data_wnd->get_channel_list_item(m_data_channel)->convert_volts_to_data_bins(
				m_mv_level / 1000.0f);
			m_p_chart_data_wnd->hz_tags.set_value_int(m_index, val);
			m_p_chart_data_wnd->Invalidate();
		}
	}
}

void CMeasureHZtagsPage::on_adjust()
{
	int max, min;
	m_p_chart_data_wnd->get_channel_list_item(m_data_channel)->get_max_min(&max, &min);
	// get nb cursors / m_data_channel
	auto n_cursors = 0;
	for (auto i = m_nb_cursors - 1; i >= 0; i--)
		if (m_p_chart_data_wnd->hz_tags.get_channel(i) == m_data_channel)
			n_cursors++;

	// then split cursors across m_data_channel span
	if (n_cursors == 1)
	{
		on_center();
		return;
	}

	const auto dv = (max - min) / (n_cursors - 1);
	auto val = min;
	for (auto i = 0; i < m_nb_cursors; i++)
	{
		if (m_p_chart_data_wnd->hz_tags.get_channel(i) == m_data_channel)
		{
			m_p_chart_data_wnd->hz_tags.set_value_int(i, val);
			val += dv;
		}
	}
	m_p_chart_data_wnd->Invalidate();
	val = m_p_chart_data_wnd->hz_tags.get_value_int(m_index);
	m_mv_level = m_p_chart_data_wnd->get_channel_list_item(m_data_channel)->convert_data_bins_to_volts(val) * 1000.f;
	UpdateData(FALSE);
}

void CMeasureHZtagsPage::OnOK()
{
	auto p_tags_list = m_p_dat_doc->get_hz_tags_list();
	p_tags_list->copy_tag_list(&m_p_chart_data_wnd->hz_tags);
	m_p_options_measure->b_changed = TRUE;
	if (m_p_options_measure->w_option != MEASURE_HORIZONTAL)
	{
		m_p_chart_data_wnd->hz_tags.remove_all_tags();
		if (m_p_options_measure->w_option == MEASURE_VERTICAL)
			m_p_chart_data_wnd->vt_tags.copy_tag_list(m_p_dat_doc->get_vt_tags_list());
	}
	CPropertyPage::OnOK();
}

void CMeasureHZtagsPage::OnCancel()
{
	// restore initial state of HZ_cursors
	if (m_p_options_measure->w_option != MEASURE_HORIZONTAL)
	{
		m_p_chart_data_wnd->hz_tags.remove_all_tags();
		if (m_p_options_measure->w_option == MEASURE_VERTICAL)
			m_p_chart_data_wnd->vt_tags.copy_tag_list(m_p_dat_doc->get_vt_tags_list());
	}
	else
		m_p_chart_data_wnd->hz_tags.copy_tag_list(m_p_dat_doc->get_hz_tags_list());
	m_p_chart_data_wnd->Invalidate();
	CPropertyPage::OnCancel();
}

BOOL CMeasureHZtagsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_p_chart_data_wnd->hz_tags.copy_tag_list(m_p_dat_doc->get_hz_tags_list());
	m_p_chart_data_wnd->vt_tags.remove_all_tags();
	m_p_chart_data_wnd->Invalidate();
	m_nb_cursors = m_p_chart_data_wnd->hz_tags.get_tag_list_size();
	get_hz_cursor_value(0);

	// sub-classed edits
	VERIFY(mm_index.SubclassDlgItem(IDC_INDEX, this));
	VERIFY(mm_data_channel.SubclassDlgItem(IDC_DATACHANNEL, this));
	VERIFY(mm_mv_level.SubclassDlgItem(IDC_MVLEVEL, this));
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMeasureHZtagsPage::on_delete_all()
{
	m_p_chart_data_wnd->hz_tags.remove_all_tags();
	m_p_chart_data_wnd->Invalidate();
	m_nb_cursors = 0;
	get_hz_cursor_value(0);
	UpdateData(FALSE);
}
