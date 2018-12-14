// DAChannelsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "dbWave.h"
#include "DAChannelsDlg.h"
#include "afxdialogex.h"
#include "EditStimArrayDlg.h"
#include "EditDAMseqDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDAChannelsDlg dialog

IMPLEMENT_DYNAMIC(CDAChannelsDlg, CDialogEx)

CDAChannelsDlg::CDAChannelsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDAChannelsDlg::IDD, pParent)
	  , m_bChannel2(FALSE), m_iseldigital(0)
{
	m_samplingRate = 10E3;
}

CDAChannelsDlg::~CDAChannelsDlg()
{
}

void CDAChannelsDlg::DoDataExchange(CDataExchange* pDX)
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

BEGIN_MESSAGE_MAP(CDAChannelsDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE0, &CDAChannelsDlg::OnCbnSelchangeCombosource0)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE1, &CDAChannelsDlg::OnCbnSelchangeCombosource1)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE2, &CDAChannelsDlg::OnCbnSelchangeCombosource2)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE3, &CDAChannelsDlg::OnCbnSelchangeCombosource3)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE4, &CDAChannelsDlg::OnCbnSelchangeCombosource4)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE5, &CDAChannelsDlg::OnCbnSelchangeCombosource5)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE6, &CDAChannelsDlg::OnCbnSelchangeCombosource6)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE7, &CDAChannelsDlg::OnCbnSelchangeCombosource7)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE8, &CDAChannelsDlg::OnCbnSelchangeCombosource8)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE9, &CDAChannelsDlg::OnCbnSelchangeCombosource9)

	ON_BN_CLICKED(IDC_CHECKCHAN0, &CDAChannelsDlg::OnBnClickedCheckchan0)
	ON_BN_CLICKED(IDC_CHECKCHAN1, &CDAChannelsDlg::OnBnClickedCheckchan1)
	ON_BN_CLICKED(IDC_CHECKDIGITAL, &CDAChannelsDlg::OnBnClickedCheckchan2)

	ON_BN_CLICKED(IDC_BUTTONSOURCE0, &CDAChannelsDlg::OnBnClickedButtonsource0)
	ON_BN_CLICKED(IDC_BUTTONSOURCE1, &CDAChannelsDlg::OnBnClickedButtonsource1)
	ON_BN_CLICKED(IDC_BUTTONSOURCE2, &CDAChannelsDlg::OnBnClickedButtonsource2)
	ON_BN_CLICKED(IDC_BUTTONSOURCE3, &CDAChannelsDlg::OnBnClickedButtonsource3)
	ON_BN_CLICKED(IDC_BUTTONSOURCE4, &CDAChannelsDlg::OnBnClickedButtonsource4)
	ON_BN_CLICKED(IDC_BUTTONSOURCE5, &CDAChannelsDlg::OnBnClickedButtonsource5)
	ON_BN_CLICKED(IDC_BUTTONSOURCE6, &CDAChannelsDlg::OnBnClickedButtonsource6)
	ON_BN_CLICKED(IDC_BUTTONSOURCE7, &CDAChannelsDlg::OnBnClickedButtonsource7)
	ON_BN_CLICKED(IDC_BUTTONSOURCE8, &CDAChannelsDlg::OnBnClickedButtonsource8)
	ON_BN_CLICKED(IDC_BUTTONSOURCE9, &CDAChannelsDlg::OnBnClickedButtonsource9)

	ON_BN_CLICKED(IDOK, &CDAChannelsDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CDAChannelsDlg message handlers

void CDAChannelsDlg::OnBnClickedOk()
{
	int channel = 0;
	outputparms_array[channel].iChan = channel;
	outputparms_array[channel].bDigital = FALSE;
	outputparms_array[channel].iWaveform = combobox_ptr_array[channel]->GetItemData(combobox_ptr_array[channel]->GetCurSel());

	channel = 1;
	outputparms_array[channel].iChan = channel;
	outputparms_array[channel].bDigital = FALSE;
	outputparms_array[channel].iWaveform = combobox_ptr_array[channel]->GetItemData(combobox_ptr_array[channel]->GetCurSel());

	for (int ichannel = 2; ichannel < 10; ichannel++)
	{
		outputparms_array[ichannel].bDigital = TRUE;
		outputparms_array[ichannel].iChan = ichannel - 2;
		outputparms_array[ichannel].bON = m_bChannel2;
		outputparms_array[ichannel].iWaveform = combobox_ptr_array[ichannel]->GetItemData(combobox_ptr_array[ichannel]->GetCurSel());
	}

	CDialogEx::OnOK();
}

CString CDAChannelsDlg::comboText[] = { _T("Stored sequence"), _T("Sinusoid"), _T("Square"), _T("Triangle"), _T("M-sequence"), _T("Noise"), _T("Data File"), _T("Constant"), _T("OFF"), _T("ON") };
DWORD	CDAChannelsDlg::comboVal[] = { DA_SEQUENCEWAVE, DA_SINEWAVE, DA_SQUAREWAVE, DA_TRIANGLEWAVE, DA_MSEQWAVE, DA_NOISEWAVE, DA_FILEWAVE, DA_CONSTANT, DA_ZERO, DA_ONE };

void CDAChannelsDlg::FillCombo( int channel)
{
	CComboBox* pCombo = combobox_ptr_array[channel];
	pCombo->ResetContent();
	switch (channel)
	{
	case 0:
	case 1:
			for (int i = 0; i < 8; i++)
			{
				int j = pCombo->AddString(comboText[i]);
				pCombo->SetItemData(j, comboVal[i]);
			}
			break;
	default:
		{
			int i = 0;
			int j = pCombo->AddString(comboText[i]);
			pCombo->SetItemData(j, comboVal[i]);
			i = 2;
			j = pCombo->AddString(comboText[i]);
			pCombo->SetItemData(j, comboVal[i]);
			i = 4;
			j = pCombo->AddString(comboText[i]);
			pCombo->SetItemData(j, comboVal[i]);
			i = 8;
			j = pCombo->AddString(comboText[i]);
			pCombo->SetItemData(j, comboVal[i]);
			i = 9;
			j = pCombo->AddString(comboText[i]);
			pCombo->SetItemData(j, comboVal[i]);
		}
		break;
	}
}

void CDAChannelsDlg::SelectComboItemFromOutputParm(int ival)
{
	CComboBox* pCombo = combobox_ptr_array[ival];
	DWORD val = outputparms_array[ival].iWaveform;
	int isel = 0;
	for (isel; isel < pCombo->GetCount(); isel++)
	{
		if (val == pCombo->GetItemData(isel))
			break;
	}
	pCombo->SetCurSel(isel);
}

BOOL CDAChannelsDlg::OnInitDialog()
{
	combobox_ptr_array.SetSize(10);
	combobox_ptr_array[0] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE0);
	combobox_ptr_array[1] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE1);
	combobox_ptr_array[2] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE2);
	combobox_ptr_array[3] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE3);
	combobox_ptr_array[4] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE4);
	combobox_ptr_array[5] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE5);
	combobox_ptr_array[6] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE6);
	combobox_ptr_array[7] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE7);
	combobox_ptr_array[8] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE8);
	combobox_ptr_array[9] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE9);

	CDialogEx::OnInitDialog();

	if (outputparms_array.GetSize() < 10)
		outputparms_array.SetSize(10);

	for (int i = 0; i < 10; i++)
	{
		FillCombo(i);
		SelectComboItemFromOutputParm(i);
		OnCbnSelchangeCombosource(i);
	}
	
	for (int i = 2; i < 10; i++) {
		if (outputparms_array[i].bON) {
			m_bChannel2 = TRUE;
			break;
		}
	}
	UpdateData(FALSE);
	return TRUE;  
}

void CDAChannelsDlg::OnBnClickedCheckchan0()
{
	UpdateData(TRUE);
	BOOL bchan = outputparms_array[0].bON;
	GetDlgItem(IDC_COMBOSOURCE0)->EnableWindow(bchan);
	GetDlgItem(IDC_STATIC00)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITAMPLITUDE0)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITAMPLITUDELOW0)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITFREQ0)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE0)->EnableWindow(bchan);
	if (bchan) 
		OnCbnSelchangeCombosource0();
}

void CDAChannelsDlg::OnBnClickedCheckchan1()
{
	UpdateData(TRUE);
	BOOL bchan = outputparms_array[1].bON;
	GetDlgItem(IDC_COMBOSOURCE1)->EnableWindow(bchan);
	GetDlgItem(IDC_STATIC10)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITAMPLITUDE1)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITAMPLITUDELOW1)->EnableWindow(bchan);
	GetDlgItem(IDC_EDITFREQ1)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE1)->EnableWindow(bchan);
	if (bchan)
		OnCbnSelchangeCombosource1();
}

void CDAChannelsDlg::OnBnClickedCheckchan2()
{
	UpdateData(TRUE);
	BOOL bchan = m_bChannel2;
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

void CDAChannelsDlg::EditSequence(int iID, int channel)
{
	switch (iID) {
		case DA_MSEQWAVE:		// M-seq
		{
			CEditDAMseqDlg dlg;
			dlg.m_outDParms = outputparms_array.GetAt(channel);
			if (IDOK == dlg.DoModal())
			{
				OUTPUTPARMS* pParms = &outputparms_array.GetAt(channel);
				*pParms = dlg.m_outDParms;
				pParms->sti.ImportIntervalsSeries(&pParms->stimulussequence);
			}
		}
		break;

		case DA_SEQUENCEWAVE:	// sequence
		{
			CEditStimArrayDlg dlg;
			dlg.intervalsandlevels_ptr_array.RemoveAll();
			dlg.intervalsandlevels_ptr_array.Add(&outputparms_array.GetAt(channel).stimulussequence);
			dlg.m_pstimsaved = &m_stimsaved;
			dlg.m_rate = m_samplingRate;
			if (IDOK == dlg.DoModal())
			{
				OUTPUTPARMS* pParms = &outputparms_array.GetAt(channel);
				pParms->sti.ImportIntervalsSeries(&pParms->stimulussequence);
			}
		}
		break;

		case DA_NOISEWAVE:		// Noise
		case DA_FILEWAVE:		// data file
		default:
			break;
	}
}

void CDAChannelsDlg::OnBnClickedButtonsource(int channel)
{
	CComboBox* pCombo = combobox_ptr_array[channel];
	int isel = pCombo->GetCurSel();
	EditSequence(pCombo->GetItemData(isel), channel);
}

void CDAChannelsDlg::OnBnClickedButtonsource0()
{
	OnBnClickedButtonsource(0);
}

void CDAChannelsDlg::OnBnClickedButtonsource1()
{
	OnBnClickedButtonsource(1);
}

void CDAChannelsDlg::OnBnClickedButtonsource2()
{
	OnBnClickedButtonsource(2);
}

void CDAChannelsDlg::OnBnClickedButtonsource3()
{
	OnBnClickedButtonsource(3);
}

void CDAChannelsDlg::OnBnClickedButtonsource4()
{
	OnBnClickedButtonsource(4);
}

void CDAChannelsDlg::OnBnClickedButtonsource5()
{
	OnBnClickedButtonsource(5);
}

void CDAChannelsDlg::OnBnClickedButtonsource6()
{
	OnBnClickedButtonsource(6);
}

void CDAChannelsDlg::OnBnClickedButtonsource7()
{
	OnBnClickedButtonsource(7);
}

void CDAChannelsDlg::OnBnClickedButtonsource8()
{
	OnBnClickedButtonsource(8);
}

void CDAChannelsDlg::OnBnClickedButtonsource9()
{
	OnBnClickedButtonsource(9);
}

static int matrix[10][4] = {
	{ IDC_BUTTONSOURCE0, IDC_EDITFREQ0, IDC_EDITAMPLITUDE0, IDC_EDITAMPLITUDELOW0},
	{ IDC_BUTTONSOURCE1, IDC_EDITFREQ1, IDC_EDITAMPLITUDE1, IDC_EDITAMPLITUDELOW1},
	{ IDC_BUTTONSOURCE2, IDC_EDITFREQ2,  0, 0},
	{ IDC_BUTTONSOURCE3, IDC_EDITFREQ3, 0, 0},
	{ IDC_BUTTONSOURCE4, IDC_EDITFREQ4, 0, 0},
	{ IDC_BUTTONSOURCE5, IDC_EDITFREQ5, 0, 0},
	{ IDC_BUTTONSOURCE6, IDC_EDITFREQ6, 0, 0},
	{ IDC_BUTTONSOURCE7, IDC_EDITFREQ7, 0, 0},
	{ IDC_BUTTONSOURCE8, IDC_EDITFREQ8, 0, 0},
	{ IDC_BUTTONSOURCE9, IDC_EDITFREQ9, 0, 0}
};

void CDAChannelsDlg::OnCbnSelchangeCombosource(int ival)
{
	int IDBUTTON = matrix[ival][0]; 
	int IDFREQ = matrix[ival][1];
	int IDAMPLITUDEHIGH = matrix[ival][2]; 
	int IDAMPLITUDELOW = matrix[ival][3];;

	CComboBox* pCombo = combobox_ptr_array[ival];
	pCombo->EnableWindow(TRUE);
	DWORD isel = pCombo->GetItemData(pCombo->GetCurSel());
	BOOL	bEnableFrequency = TRUE;
	BOOL	bEnableButton = TRUE;
	BOOL	bEnableAmplitudeHigh = TRUE;
	BOOL	bEnableAmplitudeLow = TRUE;

	switch (isel) {
	case DA_SINEWAVE:		// sinusoid
	case DA_SQUAREWAVE:		// square
	case DA_TRIANGLEWAVE:	// triangle
		bEnableFrequency	= TRUE;
		bEnableButton		= FALSE;
		break;

	case DA_SEQUENCEWAVE:	// sequence
	case DA_MSEQWAVE:		// M-seq
		bEnableFrequency	= FALSE;
		bEnableButton		= TRUE;
		break;

	case DA_CONSTANT:
		bEnableAmplitudeLow	= FALSE;
		bEnableButton		= FALSE;
		bEnableFrequency	= FALSE;
		bEnableAmplitudeHigh= TRUE;
		break;

	case DA_NOISEWAVE:		// Noise
	case DA_FILEWAVE:		// data file
	default:
		bEnableFrequency	= FALSE;
		bEnableButton		= FALSE;
		bEnableAmplitudeHigh= FALSE;
		bEnableAmplitudeLow = FALSE;
		break;
	}
	GetDlgItem(IDBUTTON)->EnableWindow(bEnableButton);
	GetDlgItem(IDFREQ)->EnableWindow(bEnableFrequency);
	if (IDAMPLITUDEHIGH != 0) {
		GetDlgItem(IDAMPLITUDEHIGH)->EnableWindow(bEnableAmplitudeHigh);
		GetDlgItem(IDAMPLITUDELOW)->EnableWindow(bEnableAmplitudeLow);
	}
}

void CDAChannelsDlg::OnCbnSelchangeCombosource0()
{
	OnCbnSelchangeCombosource(0);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource1()
{
	OnCbnSelchangeCombosource(1);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource2()
{
	OnCbnSelchangeCombosource(2);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource3()
{
	OnCbnSelchangeCombosource(3);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource4()
{
	OnCbnSelchangeCombosource(4);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource5()
{
	OnCbnSelchangeCombosource(5);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource6()
{
	OnCbnSelchangeCombosource(6);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource7()
{
	OnCbnSelchangeCombosource(7);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource8()
{
	OnCbnSelchangeCombosource(8); 
}

void CDAChannelsDlg::OnCbnSelchangeCombosource9()
{
	OnCbnSelchangeCombosource(9);
}

