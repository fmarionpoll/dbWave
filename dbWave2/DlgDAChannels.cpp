// DAChannelsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "dbWave.h"
#include "DlgDAChannels.h"
#include "afxdialogex.h"
#include "DlgEditStimArray.h"
#include "DlgEditDAMseq.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDAChannelsDlg dialog

IMPLEMENT_DYNAMIC(DlgDAChannels, CDialogEx)

DlgDAChannels::DlgDAChannels(CWnd* p_parent /*=NULL*/)
	: CDialogEx(IDD, p_parent)
	  , m_bChannel2(FALSE), m_i_sel_digital(0)
{
	m_sampling_rate = 10E3;
}

DlgDAChannels::~DlgDAChannels()
{
}

void DlgDAChannels::DoDataExchange(CDataExchange* p_dx)
{
	CDialogEx::DoDataExchange(p_dx);

	DDX_Check(p_dx, IDC_CHECKCHAN0, output_params_array[0].b_on);
	DDX_Check(p_dx, IDC_CHECKCHAN1, output_params_array[1].b_on);
	DDX_Check(p_dx, IDC_CHECKDIGITAL, m_bChannel2);

	DDX_Text(p_dx, IDC_EDITAMPLITUDE0, output_params_array[0].d_amplitude_max_v);
	DDX_Text(p_dx, IDC_EDITAMPLITUDELOW0, output_params_array[0].d_amplitude_min_v);
	DDX_Text(p_dx, IDC_EDITAMPLITUDE1, output_params_array[1].d_amplitude_max_v);
	DDX_Text(p_dx, IDC_EDITAMPLITUDELOW1, output_params_array[1].d_amplitude_min_v);

	DDX_Text(p_dx, IDC_EDITFREQ0, output_params_array[0].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ1, output_params_array[1].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ2, output_params_array[2].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ3, output_params_array[3].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ4, output_params_array[4].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ5, output_params_array[5].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ6, output_params_array[6].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ7, output_params_array[7].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ8, output_params_array[8].d_frequency);
	DDX_Text(p_dx, IDC_EDITFREQ9, output_params_array[9].d_frequency);

	DDX_Control(p_dx, IDC_COMBOSOURCE0, *combobox_ptr_array[0]);
	DDX_Control(p_dx, IDC_COMBOSOURCE1, *combobox_ptr_array[1]);
	DDX_Control(p_dx, IDC_COMBOSOURCE2, *combobox_ptr_array[2]);
	DDX_Control(p_dx, IDC_COMBOSOURCE3, *combobox_ptr_array[3]);
	DDX_Control(p_dx, IDC_COMBOSOURCE4, *combobox_ptr_array[4]);
	DDX_Control(p_dx, IDC_COMBOSOURCE5, *combobox_ptr_array[5]);
	DDX_Control(p_dx, IDC_COMBOSOURCE6, *combobox_ptr_array[6]);
	DDX_Control(p_dx, IDC_COMBOSOURCE7, *combobox_ptr_array[7]);
	DDX_Control(p_dx, IDC_COMBOSOURCE8, *combobox_ptr_array[8]);
	DDX_Control(p_dx, IDC_COMBOSOURCE9, *combobox_ptr_array[9]);
}

BEGIN_MESSAGE_MAP(DlgDAChannels, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE0, &DlgDAChannels::on_cbn_sel_change_combo_source0)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE1, &DlgDAChannels::on_cbn_sel_change_combo_source1)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE2, &DlgDAChannels::on_cbn_sel_change_combo_source2)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE3, &DlgDAChannels::on_cbn_sel_change_combo_source3)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE4, &DlgDAChannels::on_cbn_sel_change_combo_source4)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE5, &DlgDAChannels::on_cbn_sel_change_combo_source5)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE6, &DlgDAChannels::on_cbn_sel_change_combo_source6)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE7, &DlgDAChannels::on_cbn_sel_change_combo_source7)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE8, &DlgDAChannels::on_cbn_sel_change_combo_source8)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE9, &DlgDAChannels::on_cbn_sel_change_combo_source9)

	ON_BN_CLICKED(IDC_CHECKCHAN0, &DlgDAChannels::on_bn_clicked_check_chan0)
	ON_BN_CLICKED(IDC_CHECKCHAN1, &DlgDAChannels::on_bn_clicked_check_chan1)
	ON_BN_CLICKED(IDC_CHECKDIGITAL, &DlgDAChannels::on_bn_clicked_check_chan2)

	ON_BN_CLICKED(IDC_BUTTONSOURCE0, &DlgDAChannels::on_bn_clicked_button_source0)
	ON_BN_CLICKED(IDC_BUTTONSOURCE1, &DlgDAChannels::on_bn_clicked_button_source1)
	ON_BN_CLICKED(IDC_BUTTONSOURCE2, &DlgDAChannels::on_bn_clicked_button_source2)
	ON_BN_CLICKED(IDC_BUTTONSOURCE3, &DlgDAChannels::on_bn_clicked_button_source3)
	ON_BN_CLICKED(IDC_BUTTONSOURCE4, &DlgDAChannels::on_bn_clicked_button_source4)
	ON_BN_CLICKED(IDC_BUTTONSOURCE5, &DlgDAChannels::on_bn_clicked_button_source5)
	ON_BN_CLICKED(IDC_BUTTONSOURCE6, &DlgDAChannels::on_bn_clicked_button_source6)
	ON_BN_CLICKED(IDC_BUTTONSOURCE7, &DlgDAChannels::on_bn_clicked_button_source7)
	ON_BN_CLICKED(IDC_BUTTONSOURCE8, &DlgDAChannels::on_bn_clicked_button_source8)
	ON_BN_CLICKED(IDC_BUTTONSOURCE9, &DlgDAChannels::on_bn_clicked_button_source9)

	ON_BN_CLICKED(IDOK, &DlgDAChannels::on_bn_clicked_ok)
END_MESSAGE_MAP()

// CDAChannelsDlg message handlers

void DlgDAChannels::on_bn_clicked_ok()
{
	auto channel = 0;
	output_params_array[channel].i_chan = channel;
	output_params_array[channel].b_digital = FALSE;
	output_params_array[channel].i_waveform = combobox_ptr_array[channel]->GetItemData(
		combobox_ptr_array[channel]->GetCurSel());

	channel = 1;
	output_params_array[channel].i_chan = channel;
	output_params_array[channel].b_digital = FALSE;
	output_params_array[channel].i_waveform = combobox_ptr_array[channel]->GetItemData(
		combobox_ptr_array[channel]->GetCurSel());

	for (int i_channel = 2; i_channel < 10; i_channel++)
	{
		output_params_array[i_channel].b_digital = TRUE;
		output_params_array[i_channel].i_chan = i_channel - 2;
		output_params_array[i_channel].b_on = m_bChannel2;
		output_params_array[i_channel].i_waveform = combobox_ptr_array[i_channel]->GetItemData(
			combobox_ptr_array[i_channel]->GetCurSel());
	}

	CDialogEx::OnOK();
}

CString DlgDAChannels::combo_text_[] = {
	_T("Stored sequence"), _T("Sinusoid"), _T("Square"), _T("Triangle"), _T("M-sequence"), _T("Noise"), _T("Data File"),
	_T("Constant"), _T("OFF"), _T("ON")
};
DWORD DlgDAChannels::combo_val_[] = {
	DA_SEQUENCEWAVE, DA_SINEWAVE, DA_SQUAREWAVE, DA_TRIANGLEWAVE, DA_MSEQWAVE, DA_NOISEWAVE, DA_FILEWAVE, DA_CONSTANT,
	DA_ZERO, DA_ONE
};

void DlgDAChannels::fill_combo(int channel)
{
	auto p_combo = combobox_ptr_array[channel];
	p_combo->ResetContent();
	switch (channel)
	{
	case 0:
	case 1:
		for (auto i = 0; i < 8; i++)
		{
			const auto j = p_combo->AddString(combo_text_[i]);
			p_combo->SetItemData(j, combo_val_[i]);
		}
		break;
	default:
		{
			auto i = 0;
			auto j = p_combo->AddString(combo_text_[i]);
			p_combo->SetItemData(j, combo_val_[i]);
			i = 2;
			j = p_combo->AddString(combo_text_[i]);
			p_combo->SetItemData(j, combo_val_[i]);
			i = 4;
			j = p_combo->AddString(combo_text_[i]);
			p_combo->SetItemData(j, combo_val_[i]);
			i = 8;
			j = p_combo->AddString(combo_text_[i]);
			p_combo->SetItemData(j, combo_val_[i]);
			i = 9;
			j = p_combo->AddString(combo_text_[i]);
			p_combo->SetItemData(j, combo_val_[i]);
		}
		break;
	}
}

void DlgDAChannels::select_combo_item_from_output_parameters_array(const int i_val)
{
	const auto p_combo = combobox_ptr_array[i_val];
	const DWORD val = output_params_array[i_val].i_waveform;
	auto i_sel = 0;
	for (auto i = 0; i < p_combo->GetCount(); i++, i_sel++)
	{
		if (val == p_combo->GetItemData(i_sel))
			break;
	}
	p_combo->SetCurSel(i_sel);
}

BOOL DlgDAChannels::OnInitDialog()
{
	combobox_ptr_array.SetSize(10);
	combobox_ptr_array[0] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE0));
	combobox_ptr_array[1] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE1));
	combobox_ptr_array[2] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE2));
	combobox_ptr_array[3] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE3));
	combobox_ptr_array[4] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE4));
	combobox_ptr_array[5] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE5));
	combobox_ptr_array[6] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE6));
	combobox_ptr_array[7] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE7));
	combobox_ptr_array[8] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE8));
	combobox_ptr_array[9] = static_cast<CComboBox*>(GetDlgItem(IDC_COMBOSOURCE9));

	CDialogEx::OnInitDialog();

	if (output_params_array.GetSize() < 10)
		output_params_array.SetSize(10);

	for (int i = 0; i < 10; i++)
	{
		fill_combo(i);
		select_combo_item_from_output_parameters_array(i);
		on_cbn_sel_change_combo_source(i);
	}

	for (int i = 2; i < 10; i++)
	{
		if (output_params_array[i].b_on)
		{
			m_bChannel2 = TRUE;
			break;
		}
	}
	UpdateData(FALSE);
	return TRUE;
}

void DlgDAChannels::on_bn_clicked_check_chan0()
{
	UpdateData(TRUE);
	const auto b_chan = output_params_array[0].b_on;
	GetDlgItem(IDC_COMBOSOURCE0)->EnableWindow(b_chan);
	GetDlgItem(IDC_STATIC00)->EnableWindow(b_chan);
	GetDlgItem(IDC_EDITAMPLITUDE0)->EnableWindow(b_chan);
	GetDlgItem(IDC_EDITAMPLITUDELOW0)->EnableWindow(b_chan);
	GetDlgItem(IDC_EDITFREQ0)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE0)->EnableWindow(b_chan);
	if (b_chan)
		on_cbn_sel_change_combo_source0();
}

void DlgDAChannels::on_bn_clicked_check_chan1()
{
	UpdateData(TRUE);
	const auto b_chan = output_params_array[1].b_on;
	GetDlgItem(IDC_COMBOSOURCE1)->EnableWindow(b_chan);
	GetDlgItem(IDC_STATIC10)->EnableWindow(b_chan);
	GetDlgItem(IDC_EDITAMPLITUDE1)->EnableWindow(b_chan);
	GetDlgItem(IDC_EDITAMPLITUDELOW1)->EnableWindow(b_chan);
	GetDlgItem(IDC_EDITFREQ1)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE1)->EnableWindow(b_chan);
	if (b_chan)
		on_cbn_sel_change_combo_source1();
}

void DlgDAChannels::on_bn_clicked_check_chan2()
{
	UpdateData(TRUE);
	const auto b_chan = m_bChannel2;
	GetDlgItem(IDC_COMBOSOURCE2)->EnableWindow(b_chan);
	GetDlgItem(IDC_COMBOSOURCE3)->EnableWindow(b_chan);
	GetDlgItem(IDC_COMBOSOURCE4)->EnableWindow(b_chan);
	GetDlgItem(IDC_COMBOSOURCE5)->EnableWindow(b_chan);
	GetDlgItem(IDC_COMBOSOURCE6)->EnableWindow(b_chan);
	GetDlgItem(IDC_COMBOSOURCE7)->EnableWindow(b_chan);
	GetDlgItem(IDC_COMBOSOURCE8)->EnableWindow(b_chan);
	GetDlgItem(IDC_COMBOSOURCE9)->EnableWindow(b_chan);

	GetDlgItem(IDC_BUTTONSOURCE2)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE3)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE4)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE5)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE6)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE7)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE8)->EnableWindow(b_chan);
	GetDlgItem(IDC_BUTTONSOURCE9)->EnableWindow(b_chan);

	if (b_chan)
		on_cbn_sel_change_combo_source2();
}

void DlgDAChannels::edit_sequence(const int i_id, const int channel)
{
	switch (i_id)
	{
	case DA_MSEQWAVE: // M-seq
		{
			DlgEditDAMseq dlg;
			dlg.m_out_d_parameters = output_params_array.GetAt(channel);
			if (IDOK == dlg.DoModal())
			{
				const auto p_params = &output_params_array.GetAt(channel);
				*p_params = dlg.m_out_d_parameters;
				p_params->sti.import_intervals_series(&p_params->stimulus_sequence);
			}
		}
		break;

	case DA_SEQUENCEWAVE: // sequence
		{
			DlgEditStimArray dlg;
			dlg.intervals = output_params_array.GetAt(channel).stimulus_sequence;
			dlg.intervals_saved = m_stimulus_saved;
			dlg.m_sampling_rate = m_sampling_rate;
			if (IDOK == dlg.DoModal())
			{
				const auto p_params = &output_params_array.GetAt(channel);
				p_params->sti.import_intervals_series(&p_params->stimulus_sequence);
			}
		}
		break;

	case DA_NOISEWAVE: // Noise
	case DA_FILEWAVE: // data file
	default:
		break;
	}
}

void DlgDAChannels::on_bn_clicked_button_source(const int channel)
{
	const auto p_combo = combobox_ptr_array[channel];
	const auto i_sel = p_combo->GetCurSel();
	edit_sequence(p_combo->GetItemData(i_sel), channel);
}

void DlgDAChannels::on_bn_clicked_button_source0()
{
	on_bn_clicked_button_source(0);
}

void DlgDAChannels::on_bn_clicked_button_source1()
{
	on_bn_clicked_button_source(1);
}

void DlgDAChannels::on_bn_clicked_button_source2()
{
	on_bn_clicked_button_source(2);
}

void DlgDAChannels::on_bn_clicked_button_source3()
{
	on_bn_clicked_button_source(3);
}

void DlgDAChannels::on_bn_clicked_button_source4()
{
	on_bn_clicked_button_source(4);
}

void DlgDAChannels::on_bn_clicked_button_source5()
{
	on_bn_clicked_button_source(5);
}

void DlgDAChannels::on_bn_clicked_button_source6()
{
	on_bn_clicked_button_source(6);
}

void DlgDAChannels::on_bn_clicked_button_source7()
{
	on_bn_clicked_button_source(7);
}

void DlgDAChannels::on_bn_clicked_button_source8()
{
	on_bn_clicked_button_source(8);
}

void DlgDAChannels::on_bn_clicked_button_source9()
{
	on_bn_clicked_button_source(9);
}

static int matrix[10][4] = {
	{IDC_BUTTONSOURCE0, IDC_EDITFREQ0, IDC_EDITAMPLITUDE0, IDC_EDITAMPLITUDELOW0},
	{IDC_BUTTONSOURCE1, IDC_EDITFREQ1, IDC_EDITAMPLITUDE1, IDC_EDITAMPLITUDELOW1},
	{IDC_BUTTONSOURCE2, IDC_EDITFREQ2, 0, 0},
	{IDC_BUTTONSOURCE3, IDC_EDITFREQ3, 0, 0},
	{IDC_BUTTONSOURCE4, IDC_EDITFREQ4, 0, 0},
	{IDC_BUTTONSOURCE5, IDC_EDITFREQ5, 0, 0},
	{IDC_BUTTONSOURCE6, IDC_EDITFREQ6, 0, 0},
	{IDC_BUTTONSOURCE7, IDC_EDITFREQ7, 0, 0},
	{IDC_BUTTONSOURCE8, IDC_EDITFREQ8, 0, 0},
	{IDC_BUTTONSOURCE9, IDC_EDITFREQ9, 0, 0}
};

void DlgDAChannels::on_cbn_sel_change_combo_source(const int i_val)
{
	const auto id_button = matrix[i_val][0];
	const auto id_freq = matrix[i_val][1];
	const auto id_amplitude_high = matrix[i_val][2];
	const auto id_amplitude_low = matrix[i_val][3];

	const auto p_combo = combobox_ptr_array[i_val];
	p_combo->EnableWindow(TRUE);
	const auto i_sel = p_combo->GetItemData(p_combo->GetCurSel());
	BOOL b_enable_frequency;
	BOOL b_enable_button;
	auto b_enable_amplitude_high = TRUE;
	auto b_enable_amplitude_low = TRUE;

	switch (i_sel)
	{
	case DA_SINEWAVE: // sinusoid
	case DA_SQUAREWAVE: // square
	case DA_TRIANGLEWAVE: // triangle
		b_enable_frequency = TRUE;
		b_enable_button = FALSE;
		break;

	case DA_SEQUENCEWAVE: // sequence
	case DA_MSEQWAVE: // M-seq
		b_enable_frequency = FALSE;
		b_enable_button = TRUE;
		break;

	case DA_CONSTANT:
		b_enable_amplitude_low = FALSE;
		b_enable_button = FALSE;
		b_enable_frequency = FALSE;
		b_enable_amplitude_high = TRUE;
		break;

	case DA_NOISEWAVE: // Noise
	case DA_FILEWAVE: // data file
	default:
		b_enable_frequency = FALSE;
		b_enable_button = FALSE;
		b_enable_amplitude_high = FALSE;
		b_enable_amplitude_low = FALSE;
		break;
	}
	GetDlgItem(id_button)->EnableWindow(b_enable_button);
	GetDlgItem(id_freq)->EnableWindow(b_enable_frequency);
	if (id_amplitude_high != 0)
	{
		GetDlgItem(id_amplitude_high)->EnableWindow(b_enable_amplitude_high);
		GetDlgItem(id_amplitude_low)->EnableWindow(b_enable_amplitude_low);
	}
}

void DlgDAChannels::on_cbn_sel_change_combo_source0()
{
	on_cbn_sel_change_combo_source(0);
}

void DlgDAChannels::on_cbn_sel_change_combo_source1()
{
	on_cbn_sel_change_combo_source(1);
}

void DlgDAChannels::on_cbn_sel_change_combo_source2()
{
	on_cbn_sel_change_combo_source(2);
}

void DlgDAChannels::on_cbn_sel_change_combo_source3()
{
	on_cbn_sel_change_combo_source(3);
}

void DlgDAChannels::on_cbn_sel_change_combo_source4()
{
	on_cbn_sel_change_combo_source(4);
}

void DlgDAChannels::on_cbn_sel_change_combo_source5()
{
	on_cbn_sel_change_combo_source(5);
}

void DlgDAChannels::on_cbn_sel_change_combo_source6()
{
	on_cbn_sel_change_combo_source(6);
}

void DlgDAChannels::on_cbn_sel_change_combo_source7()
{
	on_cbn_sel_change_combo_source(7);
}

void DlgDAChannels::on_cbn_sel_change_combo_source8()
{
	on_cbn_sel_change_combo_source(8);
}

void DlgDAChannels::on_cbn_sel_change_combo_source9()
{
	on_cbn_sel_change_combo_source(9);
}
