// TODO : measure data and output to note_doc_view

#include "StdAfx.h"
#include "ChartWnd.h"

#include "dbWaveDoc.h"
#include "resource.h"

#include "MeasureOptionsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureOptionsPage, CPropertyPage)


CMeasureOptionsPage::CMeasureOptionsPage() : CPropertyPage(IDD)
{
}

CMeasureOptionsPage::~CMeasureOptionsPage()
= default;

void CMeasureOptionsPage::DoDataExchange(CDataExchange* p_dx)
{
	CPropertyPage::DoDataExchange(p_dx);
	DDX_Check(p_dx, IDC_CHECKEXTREMA, m_b_extrema);
	DDX_Check(p_dx, IDC_CHECKDIFFERENCE, m_b_diff_extrema);
	DDX_Check(p_dx, IDC_CHECK2, m_b_diff_data_limits);
	DDX_Check(p_dx, IDC_CHECKRISETIME, m_b_half_rise_time);
	DDX_Check(p_dx, IDC_CHECKRECOVERYTIME, m_b_half_recovery);
	DDX_Check(p_dx, IDC_CHECKATLIMITS, m_b_data_limits);
	DDX_Check(p_dx, IDC_APPLYTOALLFILES, m_b_all_files);
	DDX_Text(p_dx, IDC_SOURCECHANNEL, m_ui_source_chan);
	DDX_Text(p_dx, IDC_TIMEOFFSET, m_f_stimulus_offset);
	DDX_Text(p_dx, IDC_STIMULUSTHRESHOLD, m_ui_stimulus_threshold);
	DDX_Text(p_dx, IDC_STIMULUSCHANNEL, m_ui_stimulus_chan);
}

BEGIN_MESSAGE_MAP(CMeasureOptionsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_ALLCHANNELS, on_all_channels)
	ON_BN_CLICKED(IDC_SINGLECHANNEL, on_single_channel)
	ON_BN_CLICKED(IDC_VERTICALTAGS, on_vertical_tags)
	ON_BN_CLICKED(IDC_HORIZONTALTAGS, on_horizontal_tags)
	ON_BN_CLICKED(IDC_STIMULUSTAG, on_stimulus_tag)
END_MESSAGE_MAP()

// display groups of controls as visible or disabled

void CMeasureOptionsPage::show_limits_parameters(const BOOL b_show) const
{
	GetDlgItem(IDC_STIMULUSCHANNEL)->EnableWindow(b_show);
	GetDlgItem(IDC_STIMULUSTHRESHOLD)->EnableWindow(b_show);
	GetDlgItem(IDC_TIMEOFFSET)->EnableWindow(b_show);
}

void CMeasureOptionsPage::show_chan_parameters(const BOOL b_show) const
{
	GetDlgItem(IDC_SOURCECHANNEL)->EnableWindow(b_show);
}

void CMeasureOptionsPage::on_all_channels()
{
	show_chan_parameters(FALSE);
}

void CMeasureOptionsPage::on_single_channel()
{
	show_chan_parameters(TRUE);
}

void CMeasureOptionsPage::on_vertical_tags()
{
	m_p_chart_data_wnd->hz_tags.remove_all_tags();
	m_p_chart_data_wnd->vt_tags.copy_tag_list(m_p_dat_doc->get_vt_tags_list());
	m_p_chart_data_wnd->Invalidate();
	show_limits_parameters(FALSE);
}

void CMeasureOptionsPage::on_horizontal_tags()
{
	m_p_chart_data_wnd->vt_tags.remove_all_tags();
	m_p_chart_data_wnd->hz_tags.copy_tag_list(m_p_dat_doc->get_hz_tags_list());
	m_p_chart_data_wnd->Invalidate();
	show_limits_parameters(FALSE);
}

void CMeasureOptionsPage::on_stimulus_tag()
{
	show_limits_parameters(TRUE);
}

// save options
void CMeasureOptionsPage::save_options()
{
	m_p_options_measure->b_extrema = m_b_extrema;
	m_p_options_measure->b_diff_extrema = m_b_diff_extrema;
	m_p_options_measure->b_diff_data_limits = m_b_diff_data_limits;
	m_p_options_measure->b_half_rise_time = m_b_half_rise_time;
	m_p_options_measure->b_half_recovery = m_b_half_recovery;
	m_p_options_measure->b_data_limits = m_b_data_limits;
	m_p_options_measure->w_source_channel = static_cast<WORD>(m_ui_source_chan);
	m_p_options_measure->w_stimulus_threshold = static_cast<WORD>(m_ui_stimulus_threshold);
	m_p_options_measure->w_stimulus_channel = static_cast<WORD>(m_ui_stimulus_chan);
	m_p_options_measure->f_stimulus_offset = m_f_stimulus_offset;
	m_p_options_measure->b_all_files = m_b_all_files;
	m_p_options_measure->b_time = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();

	auto i_id = GetCheckedRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG);
	switch (i_id)
	{
	case IDC_VERTICALTAGS:
		i_id = 0;
		m_p_chart_data_wnd->hz_tags.remove_all_tags();
		m_p_chart_data_wnd->vt_tags.copy_tag_list(m_p_dat_doc->get_vt_tags_list());
		break;
	case IDC_HORIZONTALTAGS:
		i_id = 1;
		m_p_chart_data_wnd->vt_tags.remove_all_tags();
		m_p_chart_data_wnd->hz_tags.copy_tag_list(m_p_dat_doc->get_hz_tags_list());
		break;
	case IDC_RECTANGLETAG: i_id = 2;
		break;
	case IDC_STIMULUSTAG: i_id = 3;
		break;
	default: i_id = 2;
		break;
	}
	m_p_options_measure->w_option = i_id;
	m_p_options_measure->b_all_channels = static_cast<CButton*>(GetDlgItem(IDC_ALLCHANNELS))->GetCheck();
	m_p_options_measure->b_changed = TRUE;
}

BOOL CMeasureOptionsPage::OnKillActive()
{
	UpdateData(TRUE);
	save_options();
	return CPropertyPage::OnKillActive();
}

void CMeasureOptionsPage::OnOK()
{
	UpdateData(TRUE);
	save_options();
	CPropertyPage::OnOK();
}

BOOL CMeasureOptionsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_b_extrema = m_p_options_measure->b_extrema;
	m_b_diff_extrema = m_p_options_measure->b_diff_extrema;
	m_b_diff_data_limits = m_p_options_measure->b_diff_data_limits;
	m_b_half_rise_time = m_p_options_measure->b_half_rise_time;
	m_b_half_recovery = m_p_options_measure->b_half_recovery;
	m_b_data_limits = m_p_options_measure->b_data_limits;
	m_ui_source_chan = m_p_options_measure->w_source_channel;
	m_ui_stimulus_threshold = m_p_options_measure->w_stimulus_threshold;
	m_ui_stimulus_chan = m_p_options_measure->w_stimulus_channel;
	m_f_stimulus_offset = m_p_options_measure->f_stimulus_offset;
	m_b_all_files = m_p_options_measure->b_all_files;
	int i_id;
	auto flag = FALSE;
	switch (m_p_options_measure->w_option)
	{
	case 0: i_id = IDC_VERTICALTAGS;
		break;
	case 1: i_id = IDC_HORIZONTALTAGS;
		break;
	case 2: i_id = IDC_RECTANGLETAG;
		break;
	case 3: i_id = IDC_STIMULUSTAG;
		flag = TRUE;
		break;
	default: i_id = IDC_VERTICALTAGS;
		break;
	}
	show_limits_parameters(flag);

	auto i_id1 = IDC_ALLCHANNELS;
	flag = FALSE;
	if (!m_p_options_measure->b_all_channels)
	{
		i_id1 = IDC_SINGLECHANNEL;
		flag = TRUE;
	}
	show_chan_parameters(flag);

	CheckRadioButton(IDC_ALLCHANNELS, IDC_SINGLECHANNEL, i_id1);
	CheckRadioButton(IDC_VERTICALTAGS, IDC_STIMULUSTAG, i_id);

	GetDlgItem(IDC_CHECKRISETIME)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECKRECOVERYTIME)->EnableWindow(FALSE);
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_p_options_measure->b_time);

	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
