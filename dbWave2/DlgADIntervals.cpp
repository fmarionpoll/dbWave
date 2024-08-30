#include "StdAfx.h"
#include "Editctrl.h"
#include "DlgADIntervals.h"

#include <Olxdadefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgADIntervals::DlgADIntervals(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgADIntervals::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);

	DDX_Text(p_dx, IDC_NBUFFERS, m_buffer_n_items);
	DDX_Text(p_dx, IDC_ADRATECHAN, m_ad_rate_channel);
	DDX_Text(p_dx, IDC_ACQDURATION, m_acquisition_duration);
	DDX_Text(p_dx, IDC_SWEEPDURATION, m_sweep_duration);
	DDX_Text(p_dx, IDC_BUFFERSIZE, m_buffer_w_size);
	DDX_Text(p_dx, IDC_EDIT1, m_under_sample_factor);
	DDV_MinMaxUInt(p_dx, m_under_sample_factor, 1, 20000);
	DDX_Check(p_dx, IDC_CHECK1, m_b_audible_sound);
	DDX_Text(p_dx, IDC_THRESHOLDCHAN, m_threshold_channel);
	DDX_Text(p_dx, IDC_THRESHOLDVAL, m_threshold_value);
}

BEGIN_MESSAGE_MAP(DlgADIntervals, CDialog)
	ON_BN_CLICKED(IDC_ADCHANNELS, on_ad_channels)
	ON_EN_CHANGE(IDC_ADRATECHAN, on_en_change_ad_rate_per_channel)
	ON_EN_CHANGE(IDC_SWEEPDURATION, on_en_change_duration)
	ON_EN_CHANGE(IDC_BUFFERSIZE, on_en_change_buffer_size)
	ON_EN_CHANGE(IDC_NBUFFERS, on_en_change_n_buffers)
	ON_EN_CHANGE(IDC_ACQDURATION, on_en_change_acquisition_duration)
	ON_BN_CLICKED(IDC_TRIGSOFT, on_trigger_threshold_off)
	ON_BN_CLICKED(IDC_TRIGSUPTHRESHOLD, on_trigger_threshold_on)
	ON_BN_CLICKED(IDC_TRIGKEYBOARD, on_trigger_threshold_off)
	ON_BN_CLICKED(IDC_TRIGEXTERNAL, on_trigger_threshold_off)
	ON_BN_CLICKED(IDC_TRIGINFTHRESHOLD, on_trigger_threshold_on)
END_MESSAGE_MAP()

void DlgADIntervals::on_ad_channels()
{
	m_postmessage = IDC_ADCHANNELS;
	OnOK();
}

void DlgADIntervals::OnOK()
{
	UpdateData(TRUE);
	wave_format_.sampling_rate_per_channel = m_ad_rate_channel;
	wave_format_.buffer_size = static_cast<WORD>(m_buffer_w_size * static_cast<UINT>(wave_format_.scan_count));
	wave_format_.buffer_n_items = static_cast<short>(m_buffer_n_items);
	wave_format_.sample_count = static_cast<long>(m_acquisition_duration * static_cast<float>(wave_format_.scan_count) * m_ad_rate_channel);
	wave_format_.b_online_display = static_cast<CButton*>(GetDlgItem(IDC_ONLINEDISPLAY))->GetCheck();
	wave_format_.b_ad_write_to_file = static_cast<CButton*>(GetDlgItem(IDC_WRITETODISK))->GetCheck();
	wave_format_.data_flow = (GetCheckedRadioButton(IDC_CONTINUOUS, IDC_BURST) == IDC_CONTINUOUS) ? 0 : 1;
	wave_format_.duration = m_acquisition_duration;

	wave_format_.trig_chan = static_cast<short>(m_threshold_channel);
	wave_format_.trig_threshold = static_cast<short>(m_threshold_value);
	auto i_id = static_cast<short>(GetCheckedRadioButton(IDC_TRIGSOFT, IDC_TRIGINFTHRESHOLD));
	switch (i_id)
	{
	case IDC_TRIGSOFT: i_id = OLx_TRG_SOFT;
		break;
	case IDC_TRIGKEYBOARD: i_id = OLx_TRG_EXTRA + 1;
		break;
	case IDC_TRIGEXTERNAL: i_id = OLx_TRG_EXTERN;
		break;
	case IDC_TRIGSUPTHRESHOLD: i_id = OLx_TRG_THRESHPOS;
		break;
	case IDC_TRIGINFTHRESHOLD: i_id = OLx_TRG_THRESHNEG;
		break;
	default: i_id = OLx_TRG_SOFT;
		break;
	}
	wave_format_.trig_mode = i_id;

	m_p_wave_format->copy(&wave_format_);
	CDialog::OnOK();
}

BOOL DlgADIntervals::OnInitDialog()
{
	CDialog::OnInitDialog();

	// display/hide chain dialog buttons
	auto show_window = SW_HIDE;
	if (m_b_chain_dialog)
		show_window = SW_SHOW;
	GetDlgItem(IDC_ADCHANNELS)->ShowWindow(show_window);
	GetDlgItem(IDC_ADINTERVALS)->ShowWindow(show_window);

	// subclass edit controls
	VERIFY(mm_ad_rate_channel.SubclassDlgItem(IDC_ADRATECHAN, this));
	VERIFY(mm_sweep_duration.SubclassDlgItem(IDC_SWEEPDURATION, this));
	VERIFY(mm_acquisition_duration.SubclassDlgItem(IDC_ACQDURATION, this));
	VERIFY(mm_buffer_w_size.SubclassDlgItem(IDC_BUFFERSIZE, this));
	VERIFY(mm_buffer_n_items.SubclassDlgItem(IDC_NBUFFERS, this));

	// load data from document
	wave_format_.copy(m_p_wave_format);
	m_ad_rate_channel = wave_format_.sampling_rate_per_channel;
	m_buffer_n_items = wave_format_.buffer_n_items;
	m_buffer_w_size = static_cast<UINT>(m_sweep_duration * m_ad_rate_channel / static_cast<float>(m_buffer_n_items));

	// init parameters manually if there is no driver
	if (0.0f == m_rate_minimum) m_rate_minimum = 0.1f;
	if (0.0f == m_rate_maximum) m_rate_maximum = 50000.f;

	static_cast<CButton*>(GetDlgItem(IDC_ONLINEDISPLAY))->SetCheck(wave_format_.b_online_display);
	static_cast<CButton*>(GetDlgItem(IDC_WRITETODISK))->SetCheck(wave_format_.b_ad_write_to_file);
	auto i_id = (wave_format_.data_flow == 0) ? IDC_CONTINUOUS : IDC_BURST;
	CheckRadioButton(IDC_CONTINUOUS, IDC_BURST, i_id);

	// trigger section
	auto flag = FALSE;
	switch (wave_format_.trig_mode)
	{
	case OLx_TRG_SOFT: i_id = IDC_TRIGSOFT;
		break;
	case OLx_TRG_EXTRA + 1: i_id = IDC_TRIGKEYBOARD;
		break;
	case OLx_TRG_EXTERN: i_id = IDC_TRIGEXTERNAL;
		break;
	case OLx_TRG_THRESHPOS: i_id = IDC_TRIGSUPTHRESHOLD;
		flag = TRUE;
		break;
	case OLx_TRG_THRESHNEG: i_id = IDC_TRIGINFTHRESHOLD;
		flag = TRUE;
		break;
	default: i_id = IDC_TRIGSOFT;
		break;
	}
	CheckRadioButton(IDC_TRIGSOFT, IDC_TRIGINFTHRESHOLD, i_id);
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(flag);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(flag);
	m_threshold_channel = wave_format_.trig_chan;
	if (m_threshold_channel < 0 || m_threshold_channel > 7)
		m_threshold_channel = 0;
	m_threshold_value = wave_format_.trig_threshold;
	if (m_threshold_value < 0 || m_threshold_value > 4095)
		m_threshold_value = 2048;

	UpdateData(FALSE);
	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgADIntervals::on_en_change_ad_rate_per_channel()
{
	if (mm_ad_rate_channel.m_b_entry_done)
	{
		mm_ad_rate_channel.on_en_change(this, m_ad_rate_channel, 1.f, -1.f);

		// check value and modifies dependent parameters
		if (m_ad_rate_channel < m_rate_minimum)
		{
			m_ad_rate_channel = m_rate_minimum;
			MessageBeep(MB_ICONEXCLAMATION);
		}
		if (m_ad_rate_channel > m_rate_maximum)
		{
			m_ad_rate_channel = m_rate_maximum;
			MessageBeep(MB_ICONEXCLAMATION);
		}
		m_sweep_duration = static_cast<float>(m_buffer_w_size) * static_cast<float>(m_buffer_n_items) / m_ad_rate_channel;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::on_en_change_duration()
{
	if (mm_sweep_duration.m_b_entry_done)
	{
		mm_sweep_duration.on_en_change(this, m_sweep_duration, 1.f, -1.f);
		// check value and modifies dependent parameters
		// leave ad_rate_chan constant, modifies m_buffer_W_size & m_buffer_N_items ...?
		// first: try to adjust buffer_size
		const auto l_total_points = static_cast<long>(m_sweep_duration * m_ad_rate_channel);
		long n_buffers = m_buffer_n_items;
		auto l_w_size = l_total_points / n_buffers;
		const auto l_w_size_min = static_cast<long>(m_ad_rate_channel * 0.05f); // minimum buffer size = 50 ms (!?)
		// corresponding buffer size is too much: add one buffer and decrease buffer size
		if (l_w_size > static_cast<long>(m_buffer_w_size_maximum))
		{
			n_buffers++;
			l_w_size = l_total_points / n_buffers;
		}
		// corresponding buffer size is too small: remove one buffer (if possible) and increase buffer size
		else if (l_w_size < l_w_size_min)
		{
			if (n_buffers == 1)
			{
				AfxMessageBox(IDS_ACQDATA_SWEEPDURATIONLOW, NULL, MB_ICONEXCLAMATION | MB_OK);
			}
			else
			{
				n_buffers--;
				l_w_size = l_total_points / n_buffers;
			}
		}
		m_buffer_w_size = static_cast<WORD>(l_w_size);
		m_buffer_n_items = static_cast<int>(n_buffers);
		m_sweep_duration = static_cast<float>(l_w_size) * static_cast<float>(n_buffers) / m_ad_rate_channel;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::on_en_change_buffer_size()
{
	if (mm_buffer_w_size.m_b_entry_done)
	{
		mm_buffer_w_size.on_en_change(this, m_buffer_w_size, 1, -1);

		// check value and update dependent parameter
		const auto ui_w_size_min = static_cast<WORD>(m_ad_rate_channel * 0.05f); // minimum buffer size = 50 ms (!?)
		if (m_buffer_w_size > m_buffer_w_size_maximum)
			m_buffer_w_size = m_buffer_w_size_maximum;
		else if (m_buffer_w_size < ui_w_size_min)
			m_buffer_w_size = ui_w_size_min;
		m_buffer_w_size = (m_buffer_w_size / m_under_sample_factor) * m_under_sample_factor;
		if (m_buffer_w_size == 0)
			m_buffer_w_size = m_under_sample_factor;
		m_sweep_duration = float(m_buffer_w_size * m_buffer_n_items) / m_ad_rate_channel;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::on_en_change_n_buffers()
{
	if (mm_buffer_n_items.m_b_entry_done)
	{
		mm_buffer_n_items.on_en_change(this, m_buffer_n_items, 1, -1);
		// update dependent parameters
		if (m_buffer_n_items < 1)
			m_buffer_n_items = 1;
		m_sweep_duration = float(m_buffer_w_size * m_buffer_n_items) / m_ad_rate_channel;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::on_en_change_acquisition_duration()
{
	if (mm_acquisition_duration.m_b_entry_done)
	{
		mm_acquisition_duration.on_en_change(this, m_acquisition_duration, 1.f, -1.f);
		const auto min_duration = static_cast<float>(m_buffer_w_size) / m_ad_rate_channel;
		if (m_acquisition_duration < min_duration)
			m_acquisition_duration = min_duration;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::on_trigger_threshold_off()
{
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(FALSE);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(FALSE);
}

void DlgADIntervals::on_trigger_threshold_on()
{
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(TRUE);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(TRUE);
}
