// exportsp.cpp : implementation file
//

#include "StdAfx.h"
#include "DlgExportSpikeInfos.h"

#include "options_view_spikes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgExportSpikeInfos::DlgExportSpikeInfos(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgExportSpikeInfos::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Check(p_dx, IDC_ACQCHSETTING, m_b_acq_ch_settings);
	DDX_Check(p_dx, IDC_ACQCOMMENTS, m_b_acq_comments);
	DDX_Check(p_dx, IDC_ACQDATE, m_b_acq_date);
	DDX_Text(p_dx, IDC_TIMEEND, m_time_end);
	DDX_Text(p_dx, IDC_BINSIZE, m_time_bin);
	DDX_Text(p_dx, IDC_TIMESTART, m_time_start);
	DDX_Check(p_dx, IDC_CHECK4, m_b_relation);
	DDX_Text(p_dx, IDC_SPIKECLASS, m_class_nb);
	DDX_Text(p_dx, IDC_SPIKECLASS2, m_class_nb2);
	DDX_Check(p_dx, IDC_TOTALSPIKES, m_b_total_spikes);
	DDX_Check(p_dx, IDC_SPKFILECOMMENTS, m_b_spk_comments);

	DDX_Radio(p_dx, IDC_RADIO1, m_i_spike_class_options);
	DDX_Text(p_dx, IDC_NBINS, m_n_bins);
	DDX_Text(p_dx, IDC_HISTMAX, m_hist_ampl_v_max);
	DDX_Text(p_dx, IDC_HISTMIN, m_hist_ampl_v_min);
	DDX_Text(p_dx, IDC_HISTNBINS, m_hist_ampl_n_bins);
	DDX_Radio(p_dx, IDC_BPSTH, m_i_export_options);
	DDX_Check(p_dx, IDC_CHECK1, m_b_export_zero);
	DDX_Check(p_dx, IDC_CHECK2, m_b_export_pivot);
	DDX_Check(p_dx, IDC_CHECK3, m_b_export_to_excel);
	DDX_Text(p_dx, IDC_EDIT1, m_i_stimulus_index);
}

BEGIN_MESSAGE_MAP(DlgExportSpikeInfos, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, on_class_filter)
	ON_EN_CHANGE(IDC_NBINS, on_en_change_n_bins)
	ON_EN_CHANGE(IDC_BINSIZE, on_en_change_bin_size)
	ON_BN_CLICKED(IDC_BPSTH, onclick_psth)
	ON_BN_CLICKED(IDC_BISI, onclick_isi)
	ON_BN_CLICKED(IDC_BAUTOCORRELATION, onclick_autocorrelation)
	ON_BN_CLICKED(IDC_BINTERVALS, onclick_others)
	ON_BN_CLICKED(IDC_AMPLIHIST, onclick_ampl_histogram)
	ON_BN_CLICKED(IDC_BEXTREMA, onclick_others)
	ON_BN_CLICKED(IDC_RADIO2, on_class_filter)
	ON_BN_CLICKED(IDC_RADIO3, on_class_filter)
	ON_BN_CLICKED(IDC_SPIKEMAXTOMIN, onclick_others)
	ON_BN_CLICKED(IDC_BSPIKELATENCY, onclick_others)
	ON_BN_CLICKED(IDC_CHECKAVERAGE, onclick_others)
	ON_BN_CLICKED(IDC_RADIO5, on_bn_clicked_radio5)
	ON_BN_CLICKED(IDC_RADIO4, on_bn_clicked_radio4)
	ON_BN_CLICKED(IDC_SPIKEPOINTS, on_bn_clicked_spike_points)
END_MESSAGE_MAP()

BOOL DlgExportSpikeInfos::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_b_spk_comments = options_view_spikes->b_spk_comments;
	m_b_acq_ch_settings = options_view_spikes->b_acq_ch_settings;
	m_b_acq_comments = options_view_spikes->b_acq_comments;
	m_b_acq_date = options_view_spikes->b_acq_date;
	m_class_nb = options_view_spikes->class_nb;
	m_class_nb2 = options_view_spikes->class_nb_2;
	m_b_total_spikes = options_view_spikes->b_total_spikes;
	m_b_export_zero = options_view_spikes->b_export_zero;
	m_b_export_pivot = options_view_spikes->b_export_pivot;
	m_b_export_to_excel = options_view_spikes->b_export_to_excel;
	m_time_end = options_view_spikes->time_end;
	m_time_start = options_view_spikes->time_start;
	m_hist_ampl_n_bins = options_view_spikes->hist_ampl_n_bins;
	m_hist_ampl_v_max = options_view_spikes->hist_ampl_v_max;
	m_hist_ampl_v_min = options_view_spikes->hist_ampl_v_min;
	m_i_stimulus_index = options_view_spikes->i_stimulus_index;

	m_i_export_options = options_view_spikes->export_data_type;
	if (m_i_export_options < 0)
		m_i_export_options = EXPORT_PSTH;
	m_i_spike_class_options = options_view_spikes->spike_class_option + 1;
	auto display_hist = SW_HIDE;
	auto display_hist_amplitudes = SW_HIDE;
	switch (m_i_export_options)
	{
	case EXPORT_PSTH:
		m_n_bins = options_view_spikes->n_bins;
		m_time_bin = (m_time_end - m_time_start) / static_cast<float>(m_n_bins);
		display_hist = SW_SHOW;
		break;
	case EXPORT_ISI:
	case EXPORT_AUTOCORR:
		m_n_bins = options_view_spikes->n_bins_isi;
		m_time_bin = options_view_spikes->bin_isi;
		display_hist = SW_SHOW;
		break;
	case EXPORT_HISTAMPL:
		display_hist_amplitudes = SW_SHOW;
		break;
	default:
		break;
	}

	display_hist_parameters(display_hist);
	display_hist_ampl_parameters(display_hist_amplitudes);

	static_cast<CButton*>(GetDlgItem(IDC_RADIO4))->SetCheck(options_view_spikes->b_absolute_time);
	static_cast<CButton*>(GetDlgItem(IDC_RADIO5))->SetCheck(!options_view_spikes->b_absolute_time);
	GetDlgItem(IDC_EDIT1)->ShowWindow(!options_view_spikes->b_absolute_time);

	static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCHANS))->SetCheck(options_view_spikes->b_all_channels);
	static_cast<CButton*>(GetDlgItem(IDC_RADIOCURRCHAN))->SetCheck(!options_view_spikes->b_all_channels);

	UpdateData(FALSE);
	on_class_filter();
	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgExportSpikeInfos::OnOK()
{
	UpdateData(TRUE);

	if (m_time_start >= m_time_end)
	{
		MessageBox(_T("Current time intervals are incorrect!"), _T("Define exports"), MB_ICONSTOP);
		return;
	}

	// intervals OK - proceed
	options_view_spikes->b_changed = TRUE;
	options_view_spikes->b_acq_ch_settings = m_b_acq_ch_settings;
	options_view_spikes->b_acq_comments = m_b_acq_comments;
	options_view_spikes->b_acq_date = m_b_acq_date;
	options_view_spikes->export_data_type = m_i_export_options;
	options_view_spikes->b_export_zero = m_b_export_zero;
	options_view_spikes->b_export_pivot = m_b_export_pivot;
	options_view_spikes->b_export_to_excel = m_b_export_to_excel;
	if (m_i_export_options == EXPORT_PSTH)
	{
		options_view_spikes->n_bins = m_n_bins;
		options_view_spikes->time_bin = m_time_bin;
	}
	else if (m_i_export_options == EXPORT_ISI || m_i_export_options == EXPORT_AUTOCORR)
	{
		options_view_spikes->n_bins_isi = m_n_bins;
		options_view_spikes->bin_isi = m_time_bin;
	}
	//m_pvdS->bartefacts = m_bartefacts;
	options_view_spikes->class_nb = m_class_nb;
	options_view_spikes->class_nb_2 = m_class_nb2;
	options_view_spikes->b_column_header = TRUE;
	options_view_spikes->b_total_spikes = m_b_total_spikes;
	options_view_spikes->b_spk_comments = m_b_spk_comments;
	options_view_spikes->spike_class_option = m_i_spike_class_options - 1;

	options_view_spikes->time_start = m_time_start;
	options_view_spikes->time_end = m_time_end;

	options_view_spikes->hist_ampl_n_bins = m_hist_ampl_n_bins;
	options_view_spikes->i_stimulus_index = m_i_stimulus_index;
	options_view_spikes->hist_ampl_v_max = m_hist_ampl_v_max;
	options_view_spikes->hist_ampl_v_min = m_hist_ampl_v_min;

	options_view_spikes->b_absolute_time =static_cast<CButton*>(GetDlgItem(IDC_RADIO4))->GetCheck();
	options_view_spikes->b_all_channels = static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCHANS))->GetCheck();

	CDialog::OnOK();
}

void DlgExportSpikeInfos::on_class_filter()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(0 == m_i_spike_class_options);
	GetDlgItem(IDC_SPIKECLASS2)->EnableWindow(0 == m_i_spike_class_options);
}

void DlgExportSpikeInfos::on_en_change_n_bins()
{
	UpdateData(TRUE);
	if (m_n_bins > 0 && m_i_export_options == EXPORT_PSTH && m_b_relation)
	{
		m_time_bin = (m_time_end - m_time_start) / m_n_bins;
		UpdateData(FALSE);
	}
}

void DlgExportSpikeInfos::on_en_change_bin_size()
{
	UpdateData(TRUE);
	if (m_time_bin > 0 && m_i_export_options == EXPORT_PSTH && m_b_relation)
	{
		m_n_bins = static_cast<int>((m_time_end - m_time_start) / m_time_bin);
		UpdateData(FALSE);
	}
}

void DlgExportSpikeInfos::onclick_psth()
{
	if (m_i_export_options != EXPORT_PSTH)
	{
		m_i_export_options = EXPORT_PSTH;
		m_n_bins = options_view_spikes->n_bins;
		m_time_bin = options_view_spikes->time_bin;
		UpdateData(FALSE);
	}
	display_hist_ampl_parameters(SW_HIDE);
	display_hist_parameters(SW_SHOW);
}

void DlgExportSpikeInfos::onclick_isi()
{
	if (m_i_export_options != EXPORT_ISI)
	{
		m_i_export_options = EXPORT_ISI;
		m_n_bins = options_view_spikes->n_bins_isi;
		m_time_bin = options_view_spikes->bin_isi;
		UpdateData(FALSE);
	}
	display_hist_ampl_parameters(SW_HIDE);
	display_hist_parameters(SW_SHOW);
}

void DlgExportSpikeInfos::onclick_autocorrelation()
{
	if (m_i_export_options != EXPORT_AUTOCORR)
	{
		m_i_export_options = EXPORT_AUTOCORR;
		m_n_bins = options_view_spikes->n_bins_isi;
		m_time_bin = options_view_spikes->bin_isi;
		UpdateData(FALSE);
	}
	display_hist_ampl_parameters(SW_HIDE);
	display_hist_parameters(SW_SHOW);
}

void DlgExportSpikeInfos::onclick_others()
{
	display_hist_ampl_parameters(SW_HIDE);
	display_hist_parameters(SW_HIDE);
}

void DlgExportSpikeInfos::onclick_ampl_histogram()
{
	display_hist_ampl_parameters(SW_SHOW);
	display_hist_parameters(SW_HIDE);
}

void DlgExportSpikeInfos::display_hist_parameters(int b_display)
{
	if (m_b_hist != b_display)
	{
		m_b_hist = b_display;
		GetDlgItem(IDC_NBINS)->ShowWindow(b_display);
		GetDlgItem(IDC_BINSIZE)->ShowWindow(b_display);
		GetDlgItem(IDC_STATIC10)->ShowWindow(b_display);
		GetDlgItem(IDC_STATIC11)->ShowWindow(b_display);
		GetDlgItem(IDC_CHECK4)->ShowWindow(b_display);
	}
}

void DlgExportSpikeInfos::display_hist_ampl_parameters(int b_display)
{
	if (m_b_hist_ampl != b_display)
	{
		m_b_hist_ampl = b_display;
		GetDlgItem(IDC_HISTNBINS_STATIC)->ShowWindow(b_display);
		GetDlgItem(IDC_HISTMIN_STATIC)->ShowWindow(b_display);
		GetDlgItem(IDC_HISTMAX_STATIC)->ShowWindow(b_display);
		GetDlgItem(IDC_HISTNBINS)->ShowWindow(b_display);
		GetDlgItem(IDC_HISTMIN)->ShowWindow(b_display);
		GetDlgItem(IDC_HISTMAX)->ShowWindow(b_display);
	}
}

void DlgExportSpikeInfos::on_bn_clicked_radio5()
{
	options_view_spikes->b_absolute_time = FALSE;
	GetDlgItem(IDC_EDIT1)->ShowWindow(TRUE);
}

void DlgExportSpikeInfos::on_bn_clicked_radio4()
{
	options_view_spikes->b_absolute_time = TRUE;
	GetDlgItem(IDC_EDIT1)->ShowWindow(FALSE);
}

void DlgExportSpikeInfos::on_bn_clicked_spike_points()
{
	m_i_export_options = EXPORT_SPIKEPOINTS;
	display_hist_ampl_parameters(SW_HIDE);
	display_hist_parameters(SW_HIDE);
}
