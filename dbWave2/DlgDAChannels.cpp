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

DlgDAChannels::DlgDAChannels(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD, pParent)
	  , m_bChannel2(FALSE), m_iseldigital(0)
{
	m_samplingRate = 10E3;
}

DlgDAChannels::~DlgDAChannels()
{
}

void DlgDAChannels::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECKCHAN0, outputparms_array[0].bON);
	DDX_Check(pDX, IDC_CHECKCHAN1, outputparms_array[1].bON);
	DDX_Check(pDX, IDC_CHECKDIGITAL, m_bChannel2);

	DDX_Text(pDX, IDC_EDITAMPLITUDE0, outputparms_array[0].dAmplitudeMaxV);
	DDX_Text(pDX, IDC_EDITAMPLITUDELOW0, outputparms_array[0].dAmplitudeMinV);
	DDX_Text(pDX, IDC_EDITAMPLITUDE1, outputparms_array[1].dAmplitudeMaxV);
	DDX_Text(pDX, IDC_EDITAMPLITUDELOW1, outputparms_array[1].dAmplitudeMinV);

	DDX_Text(pDX, IDC_EDITFREQ0, outputparms_array[0].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ1, outputparms_array[1].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ2, outputparms_array[2].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ3, outputparms_array[3].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ4, outputparms_array[4].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ5, outputparms_array[5].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ6, outputparms_array[6].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ7, outputparms_array[7].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ8, outputparms_array[8].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ9, outputparms_array[9].dFrequency);

	DDX_Control(pDX, IDC_COMBOSOURCE0, *combobox_ptr_array[0]);
	DDX_Control(pDX, IDC_COMBOSOURCE1, *combobox_ptr_array[1]);
	DDX_Control(pDX, IDC_COMBOSOURCE2, *combobox_ptr_array[2]);
	DDX_Control(pDX, IDC_COMBOSOURCE3, *combobox_ptr_array[3]);
	DDX_Control(pDX, IDC_COMBOSOURCE4, *combobox_ptr_array[4]);
	DDX_Control(pDX, IDC_COMBOSOURCE5, *combobox_ptr_array[5]);
	DDX_Control(pDX, IDC_COMBOSOURCE6, *combobox_ptr_array[6]);
	DDX_Control(pDX, IDC_COMBOSOURCE7, *combobox_ptr_array[7]);
	DDX_Control(pDX, IDC_COMBOSOURCE8, *combobox_ptr_array[8]);
	DDX_Control(pDX, IDC_COMBOSOURCE9, *combobox_ptr_array[9]);
}

BEGIN_MESSAGE_MAP(DlgDAChannels, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE0, &DlgDAChannels::OnCbnSelchangeCombosource0)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE1, &DlgDAChannels::OnCbnSelchangeCombosource1)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE2, &DlgDAChannels::OnCbnSelchangeCombosource2)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE3, &DlgDAChannels::OnCbnSelchangeCombosource3)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE4, &DlgDAChannels::OnCbnSelchangeCombosource4)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE5, &DlgDAChannels::OnCbnSelchangeCombosource5)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE6, &DlgDAChannels::OnCbnSelchangeCombosource6)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE7, &DlgDAChannels::OnCbnSelchangeCombosource7)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE8, &DlgDAChannels::OnCbnSelchangeCombosource8)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE9, &DlgDAChannels::OnCbnSelchangeCombosource9)

	ON_BN_CLICKED(IDC_CHECKCHAN0, &DlgDAChannels::OnBnClickedCheckchan0)
	ON_BN_CLICKED(IDC_CHECKCHAN1, &DlgDAChannels::OnBnClickedCheckchan1)
	ON_BN_CLICKED(IDC_CHECKDIGITAL, &DlgDAChannels::OnBnClickedCheckchan2)

	ON_BN_CLICKED(IDC_BUTTONSOURCE0, &DlgDAChannels::OnBnClickedButtonsource0)
	ON_BN_CLICKED(IDC_BUTTONSOURCE1, &DlgDAChannels::OnBnClickedButtonsource1)
	ON_BN_CLICKED(IDC_BUTTONSOURCE2, &DlgDAChannels::OnBnClickedButtonsource2)
	ON_BN_CLICKED(IDC_BUTTONSOURCE3, &DlgDAChannels::OnBnClickedButtonsource3)
	ON_BN_CLICKED(IDC_BUTTONSOURCE4, &DlgDAChannels::OnBnClickedButtonsource4)
	ON_BN_CLICKED(IDC_BUTTONSOURCE5, &DlgDAChannels::OnBnClickedButtonsource5)
	ON_BN_CLICKED(IDC_BUTTONSOURCE6, &DlgDAChannels::OnBnClickedButtonsource6)
	ON_BN_CLICKED(IDC_BUTTONSOURCE7, &DlgDAChannels::OnBnClickedButtonsource7)
	ON_BN_CLICKED(IDC_BUTTONSOURCE8, &DlgDAChannels::OnBnClickedButtonsource8)
	ON_BN_CLICKED(IDC_BUTTONSOURCE9, &DlgDAChannels::OnBnClickedButtonsource9)

	ON_BN_CLICKED(IDOK, &DlgDAChannels::OnBnClickedOk)
END_MESSAGE_MAP()

// CDAChannelsDlg message handlers

void DlgDAChannels::OnBnClickedOk()
{
	auto channel = 0;
	outputparms_array[channel].iChan = channel;
	outputparms_array[channel].bDigital = FALSE;
	outputparms_array[channel].iWaveform = combobox_ptr_array[channel]->GetItemData(
		combobox_ptr_array[channel]->GetCurSel());

	channel = 1;
	outputparms_array[channel].iChan = channel;
	outputparms_array[channel].bDigital = FALSE;
	outputparms_array[channel].iWaveform = combobox_ptr_array[channel]->GetItemData(
		combobox_ptr_array[channel]->GetCurSel());

	for (int ichannel = 2; ichannel < 10; ichannel++)
	{
		outputparms_array[ichannel].bDigital = TRUE;
		outputparms_array[ichannel].iChan = ichannel - 2;
		outputparms_array[ichannel].bON = m_bChannel2;
		outputparms_array[ichannel].iWaveform = combobox_ptr_array[ichannel]->GetItemData(
			combobox_ptr_array[ichannel]->GetCurSel());
	}

	CDialogEx::OnOK();
}

CString DlgDAChannels::comboText[] = {
	_T("Stored sequence"), _T("Sinusoid"), _T("Square"), _T("Triangle"), _T("M-sequence"), _T("Noise"), _T("Data File"),
	_T("Constant"), _T("OFF"), _T("ON")
};
DWORD DlgDAChannels::comboVal[] = {
	DA_SEQUENCEWAVE, DA_SINEWAVE, DA_SQUAREWAVE, DA_TRIANGLEWAVE, DA_MSEQWAVE, DA_NOISEWAVE, DA_FILEWAVE, DA_CONSTANT,
	DA_ZERO, DA_ONE
};

void DlgDAChannels::FillCombo(int channel)
{
	auto p_combo = combobox_ptr_array[channel];
	p_combo->ResetContent();
	switch (channel)
	{
	case 0:
	case 1:
		for (auto i = 0; i < 8; i++)
		{
			const auto j = p_combo->AddString(comboText[i]);
			p_combo->SetItemData(j, comboVal[i]);
		}
		break;
	default:
		{
			auto i = 0;
			auto j = p_combo->AddString(comboText[i]);
			p_combo->SetItemData(j, comboVal[i]);
			i = 2;
			j = p_combo->AddString(comboText[i]);
			p_combo->SetItemData(j, comboVal[i]);
			i = 4;
			j = p_combo->AddString(comboText[i]);
			p_combo->SetItemData(j, comboVal[i]);
			i = 8;
			j = p_combo->AddString(comboText[i]);
			p_combo->SetItemData(j, comboVal[i]);
			i = 9;
			j = p_combo->AddString(comboText[i]);
			p_combo->SetItemData(j, comboVal[i]);
		}
		break;
	}
}

void DlgDAChannels::SelectComboItemFromOutputParm(int ival)
{
	auto p_combo = combobox_ptr_array[ival];
	const DWORD val = outputparms_array[ival].iWaveform;
	auto isel = 0;
	for (auto i = 0; i < p_combo->GetCount(); i++, isel++)
	{
		if (val == p_combo->GetItemData(isel))
			break;
	}
	p_combo->SetCurSel(isel);
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

	if (outputparms_array.GetSize() < 10)
		outputparms_array.SetSize(10);

	for (int i = 0; i < 10; i++)
	{
		FillCombo(i);
		SelectComboItemFromOutputParm(i);
		OnCbnSelchangeCombosource(i);
	}

	for (int i = 2; i < 10; i++)
	{
		if (outputparms_array[i].bON)
		{
			m_bChannel2 = TRUE;
			break;
		}
	}
	UpdateData(FALSE);
	return TRUE;
}

void DlgDAChannels::OnBnClickedCheckchan0()
{
	UpdateData(TRUE);
	const auto bchan = outputparms_array[0].bON;
	GetDlgItem(IDC_COMBOSOURCE0)->EnableWindow(bchan);
	GetDlgItem(IDC_STATIC00)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITAMPLITUDE0)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITAMPLITUDELOW0)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITFREQ0)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE0)->EnableWindow(bchan);
	if (bchan)
		OnCbnSelchangeCombosource0();
}

void DlgDAChannels::OnBnClickedCheckchan1()
{
	UpdateData(TRUE);
	const auto bchan = outputparms_array[1].bON;
	GetDlgItem(IDC_COMBOSOURCE1)->EnableWindow(bchan);
	GetDlgItem(IDC_STATIC10)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITAMPLITUDE1)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITAMPLITUDELOW1)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITFREQ1)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE1)->EnableWindow(bchan);
	if (bchan)
		OnCbnSelchangeCombosource1();
}

void DlgDAChannels::OnBnClickedCheckchan2()
{
	UpdateData(TRUE);
	const auto bchan = m_bChannel2;
	GetDlgItem(IDC_COMBOSOURCE2)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE3)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE4)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE5)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE6)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE7)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE8)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE9)->EnableWindow(bchan);

	GetDlgItem(IDC_BUTTONSOURCE2)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE3)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE4)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE5)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE6)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE7)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE8)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE9)->EnableWindow(bchan);

	if (bchan)
		OnCbnSelchangeCombosource2();
}

void DlgDAChannels::EditSequence(int iID, int channel)
{
	switch (iID)
	{
	case DA_MSEQWAVE: // M-seq
		{
			DlgEditDAMseq dlg;
			dlg.m_outDParms = outputparms_array.GetAt(channel);
			if (IDOK == dlg.DoModal())
			{
				auto p_parms = &outputparms_array.GetAt(channel);
				*p_parms = dlg.m_outDParms;
				p_parms->sti.ImportIntervalsSeries(&p_parms->stimulussequence);
			}
		}
		break;

	case DA_SEQUENCEWAVE: // sequence
		{
			DlgEditStimArray dlg;
			dlg.intervals_and_levels_array.RemoveAll();
			dlg.intervals_and_levels_array.Add(&outputparms_array.GetAt(channel).stimulussequence);
			dlg.intervals_and_levels_saved = &m_stimsaved;
			dlg.m_sampling_rate = m_samplingRate;
			if (IDOK == dlg.DoModal())
			{
				auto p_parms = &outputparms_array.GetAt(channel);
				p_parms->sti.ImportIntervalsSeries(&p_parms->stimulussequence);
			}
		}
		break;

	case DA_NOISEWAVE: // Noise
	case DA_FILEWAVE: // data file
	default:
		break;
	}
}

void DlgDAChannels::OnBnClickedButtonsource(int channel)
{
	const auto p_combo = combobox_ptr_array[channel];
	const auto isel = p_combo->GetCurSel();
	EditSequence(p_combo->GetItemData(isel), channel);
}

void DlgDAChannels::OnBnClickedButtonsource0()
{
	OnBnClickedButtonsource(0);
}

void DlgDAChannels::OnBnClickedButtonsource1()
{
	OnBnClickedButtonsource(1);
}

void DlgDAChannels::OnBnClickedButtonsource2()
{
	OnBnClickedButtonsource(2);
}

void DlgDAChannels::OnBnClickedButtonsource3()
{
	OnBnClickedButtonsource(3);
}

void DlgDAChannels::OnBnClickedButtonsource4()
{
	OnBnClickedButtonsource(4);
}

void DlgDAChannels::OnBnClickedButtonsource5()
{
	OnBnClickedButtonsource(5);
}

void DlgDAChannels::OnBnClickedButtonsource6()
{
	OnBnClickedButtonsource(6);
}

void DlgDAChannels::OnBnClickedButtonsource7()
{
	OnBnClickedButtonsource(7);
}

void DlgDAChannels::OnBnClickedButtonsource8()
{
	OnBnClickedButtonsource(8);
}

void DlgDAChannels::OnBnClickedButtonsource9()
{
	OnBnClickedButtonsource(9);
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

void DlgDAChannels::OnCbnSelchangeCombosource(int ival)
{
	const auto idbutton = matrix[ival][0];
	const auto idfreq = matrix[ival][1];
	const auto idamplitudehigh = matrix[ival][2];
	const auto idamplitudelow = matrix[ival][3];

	auto p_combo = combobox_ptr_array[ival];
	p_combo->EnableWindow(TRUE);
	const auto isel = p_combo->GetItemData(p_combo->GetCurSel());
	BOOL b_enable_frequency;
	BOOL b_enable_button;
	auto b_enable_amplitude_high = TRUE;
	auto b_enable_amplitude_low = TRUE;

	switch (isel)
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
	GetDlgItem(idbutton)->EnableWindow(b_enable_button);
	GetDlgItem(idfreq)->EnableWindow(b_enable_frequency);
	if (idamplitudehigh != 0)
	{
		GetDlgItem(idamplitudehigh)->EnableWindow(b_enable_amplitude_high);
		GetDlgItem(idamplitudelow)->EnableWindow(b_enable_amplitude_low);
	}
}

void DlgDAChannels::OnCbnSelchangeCombosource0()
{
	OnCbnSelchangeCombosource(0);
}

void DlgDAChannels::OnCbnSelchangeCombosource1()
{
	OnCbnSelchangeCombosource(1);
}

void DlgDAChannels::OnCbnSelchangeCombosource2()
{
	OnCbnSelchangeCombosource(2);
}

void DlgDAChannels::OnCbnSelchangeCombosource3()
{
	OnCbnSelchangeCombosource(3);
}

void DlgDAChannels::OnCbnSelchangeCombosource4()
{
	OnCbnSelchangeCombosource(4);
}

void DlgDAChannels::OnCbnSelchangeCombosource5()
{
	OnCbnSelchangeCombosource(5);
}

void DlgDAChannels::OnCbnSelchangeCombosource6()
{
	OnCbnSelchangeCombosource(6);
}

void DlgDAChannels::OnCbnSelchangeCombosource7()
{
	OnCbnSelchangeCombosource(7);
}

void DlgDAChannels::OnCbnSelchangeCombosource8()
{
	OnCbnSelchangeCombosource(8);
}

void DlgDAChannels::OnCbnSelchangeCombosource9()
{
	OnCbnSelchangeCombosource(9);
}
