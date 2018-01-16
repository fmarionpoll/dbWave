// DAChannelsDlg.cpp : implementation file
//

#include "stdafx.h"
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
	, m_bChannel2(FALSE)
{

}

CDAChannelsDlg::~CDAChannelsDlg()
{
}

void CDAChannelsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECKCHAN0, m_outD.parmsChan[0].bON);
	DDX_Check(pDX, IDC_CHECKCHAN1, m_outD.parmsChan[1].bON);
	DDX_Check(pDX, IDC_CHECKDIGITAL, m_bChannel2);

	DDX_Text(pDX, IDC_EDITAMPLITUDE0, m_outD.parmsChan[0].dAmplitudeMaxV);
	DDX_Text(pDX, IDC_EDITAMPLITUDELOW0, m_outD.parmsChan[0].dAmplitudeMinV);
	DDX_Text(pDX, IDC_EDITAMPLITUDE1, m_outD.parmsChan[1].dAmplitudeMaxV);
	DDX_Text(pDX, IDC_EDITAMPLITUDELOW1, m_outD.parmsChan[1].dAmplitudeMinV);

	DDX_Text(pDX, IDC_EDITFREQ0, m_outD.parmsChan[0].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ1, m_outD.parmsChan[1].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ2, m_outD.parmsChan[2].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ3, m_outD.parmsChan[3].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ4, m_outD.parmsChan[4].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ5, m_outD.parmsChan[5].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ6, m_outD.parmsChan[6].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ7, m_outD.parmsChan[7].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ8, m_outD.parmsChan[8].dFrequency);
	DDX_Text(pDX, IDC_EDITFREQ9, m_outD.parmsChan[9].dFrequency);

	DDX_CBIndex(pDX, IDC_COMBOSOURCE0, m_outD.parmsChan[0].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE1, m_outD.parmsChan[1].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE2, m_outD.parmsChan[2].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE3, m_outD.parmsChan[3].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE4, m_outD.parmsChan[4].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE5, m_outD.parmsChan[5].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE6, m_outD.parmsChan[6].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE7, m_outD.parmsChan[7].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE8, m_outD.parmsChan[8].iWaveform);
	DDX_CBIndex(pDX, IDC_COMBOSOURCE9, m_outD.parmsChan[9].iWaveform);

	DDX_Control(pDX, IDC_COMBOSOURCE0, *m_pcombos[0]);
	DDX_Control(pDX, IDC_COMBOSOURCE1, *m_pcombos[1]);
	DDX_Control(pDX, IDC_COMBOSOURCE2, *m_pcombos[2]);
	DDX_Control(pDX, IDC_COMBOSOURCE3, *m_pcombos[3]);
	DDX_Control(pDX, IDC_COMBOSOURCE4, *m_pcombos[4]);
	DDX_Control(pDX, IDC_COMBOSOURCE5, *m_pcombos[5]);
	DDX_Control(pDX, IDC_COMBOSOURCE6, *m_pcombos[6]);
	DDX_Control(pDX, IDC_COMBOSOURCE7, *m_pcombos[7]);
	DDX_Control(pDX, IDC_COMBOSOURCE8, *m_pcombos[8]);
	DDX_Control(pDX, IDC_COMBOSOURCE9, *m_pcombos[9]);
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

void CDAChannelsDlg::OnCbnSelchangeCombosource0()
{
	CComboBox* pCombo = m_pcombos[0];
	pCombo->EnableWindow(TRUE);
	DWORD isel = pCombo->GetItemData(pCombo->GetCurSel());
	BOOL	bEnable = TRUE;
	BOOL	bEnable2 = TRUE;
	BOOL	bEnable2b = TRUE;
	BOOL	bEnable3 = FALSE;
	switch (isel) {
	case DA_SINEWAVE:		// sinusoid
	case DA_SQUAREWAVE:		// square
	case DA_TRIANGLEWAVE:	// triangle
		break;

	case DA_SEQUENCEWAVE:	// sequence
	case DA_MSEQWAVE:		// M-seq
		bEnable = FALSE;
		bEnable3 = TRUE;
		break;

	case DA_CONSTANT:
		bEnable2b = FALSE;
		break;

	case DA_NOISEWAVE:		// Noise
	case DA_FILEWAVE:		// data file
	default:
		bEnable = FALSE;
		bEnable2 = FALSE;
		bEnable2b = FALSE;
		bEnable3 = TRUE;
		break;
	}
	GetDlgItem(IDC_STATIC00)->EnableWindow(bEnable2); 
	GetDlgItem(IDC_EDITAMPLITUDE0)->EnableWindow(bEnable2);
	GetDlgItem(IDC_EDITAMPLITUDELOW0)->EnableWindow(bEnable2b);
	GetDlgItem(IDC_STATIC01)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDITFREQ0)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTONSOURCE0)->EnableWindow(bEnable3);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource1()
{
	CComboBox* pCombo = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1));
	pCombo->EnableWindow(TRUE);
	DWORD isel = pCombo->GetItemData(pCombo->GetCurSel());
	BOOL	bEnable = TRUE;
	BOOL	bEnable2 = TRUE;
	BOOL	bEnable2b = TRUE;
	BOOL	bEnable3 = FALSE;
	switch (isel) {
	case DA_SINEWAVE:		// sinusoid
	case DA_SQUAREWAVE:		// square
	case DA_TRIANGLEWAVE:	// triangle
		break;

	case DA_SEQUENCEWAVE:	// sequence
	case DA_MSEQWAVE:		// M-seq
		bEnable = FALSE;
		bEnable3 = TRUE;
		break; 

	case DA_CONSTANT:
		bEnable2b = FALSE;
		break;

	case DA_NOISEWAVE:		// Noise
	case DA_FILEWAVE:		// data file
	default:
		bEnable = FALSE;
		bEnable2 = FALSE;
		bEnable2b = FALSE;
		bEnable3 = TRUE;
		break;
	}
	GetDlgItem(IDC_STATIC10)->EnableWindow(bEnable2);
	GetDlgItem(IDC_EDITAMPLITUDE1)->EnableWindow(bEnable2);
	GetDlgItem(IDC_EDITAMPLITUDELOW1)->EnableWindow(bEnable2b);
	GetDlgItem(IDC_STATIC11)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDITFREQ1)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTONSOURCE1)->EnableWindow(bEnable3);
}

void CDAChannelsDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	int isize = m_outD.parmsChan.GetSize();
	if (isize < 10)
		m_outD.parmsChan.SetSize(10);

	int channel = 0;
	OUTPUTPARMS* pParms = &m_outD.parmsChan.GetAt(channel);
	pParms->iChan = channel;
	pParms->bDigital = FALSE;
	pParms->iWaveform = m_pcombos[channel]->GetItemData(m_pcombos[channel]->GetCurSel());

	channel = 1;
	pParms = &m_outD.parmsChan.GetAt(channel);
	pParms->iChan = channel;
	pParms->bDigital = FALSE;
	pParms->iWaveform = m_pcombos[channel]->GetItemData(m_pcombos[channel]->GetCurSel());

	for (channel = 2; channel < 10; channel++)
	{
		pParms->bDigital = TRUE;
		pParms->iChan = channel - 2;
		pParms->bON = m_bChannel2;
	}

	CDialogEx::OnOK();
}

CString CDAChannelsDlg::comboText[] = { _T("Stored sequence"), _T("Sinusoid"), _T("Square"), _T("Triangle"), _T("M - sequence"), _T("Noise"), _T("Data File"), _T("Constant"), _T("OFF"), _T("ON") };
DWORD	CDAChannelsDlg::comboVal[] = { DA_SEQUENCEWAVE, DA_SINEWAVE, DA_SQUAREWAVE, DA_TRIANGLEWAVE, DA_MSEQWAVE, DA_NOISEWAVE, DA_FILEWAVE, DA_CONSTANT, DA_ZERO, DA_ONE };

void CDAChannelsDlg::FillCombo(CComboBox* pCombo, int channel)
{
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
	case 2:
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
	default:
		break;
	}
}

void CDAChannelsDlg::SelectComboItem(CComboBox* pCombo, DWORD val)
{
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
	CDialogEx::OnInitDialog();

	if (m_outD.parmsChan.GetSize() < 10)
		m_outD.parmsChan.SetSize(10);

	m_pcombos.SetSize(10);
	m_pcombos[0] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE0);
	m_pcombos[1] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE1);
	m_pcombos[2] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE2);
	m_pcombos[3] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE3);
	m_pcombos[4] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE4);
	m_pcombos[5] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE5);
	m_pcombos[6] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE6);
	m_pcombos[7] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE7);
	m_pcombos[8] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE8);
	m_pcombos[9] = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE9);

	for (int i= 0; i < 10; i++) 
		FillCombo(m_pcombos[i], i);
	
	for (int i = 2; i < 10; i++) {
		if (m_outD.parmsChan.GetAt(i).bON) {
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
	BOOL bchan = m_outD.parmsChan[0].bON;
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
	BOOL bchan = m_outD.parmsChan[1].bON;
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
	GetDlgItem(IDC_BUTTONSOURCE2)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE3)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE3)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE4)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE4)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE5)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE5)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE6)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE6)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE7)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE7)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE8)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE8)->EnableWindow(bchan);
	GetDlgItem(IDC_COMBOSOURCE9)->EnableWindow(bchan);
	GetDlgItem(IDC_BUTTONSOURCE9)->EnableWindow(bchan);
	if (bchan)
		OnCbnSelchangeCombosource2();
}

void CDAChannelsDlg::OnBnClickedButtonsource0()
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE0);
	int isel = pCombo->GetCurSel();
	int channel = 0;
	EditSequence(pCombo->GetItemData(isel), channel);
}

void CDAChannelsDlg::OnBnClickedButtonsource1()
{
	CComboBox* pCombo = (CComboBox*) GetDlgItem(IDC_COMBOSOURCE1);
	int isel = pCombo->GetCurSel();
	int channel = 1;
	EditSequence(pCombo->GetItemData(isel), channel);
}

void CDAChannelsDlg::EditSequence(int iID, int channel)
{
	switch (iID) {
		case DA_MSEQWAVE:		// M-seq
		{
			CEditDAMseqDlg dlg;
			dlg.m_outDParms = m_outD.parmsChan.GetAt(channel);
			if (IDOK == dlg.DoModal())
			{
				OUTPUTPARMS* pParms = &m_outD.parmsChan.GetAt(channel);
				*pParms = dlg.m_outDParms;
				pParms->sti.ImportIntervalsSeries(&pParms->stimulussequence);
			}
		}
		break;

		case DA_SEQUENCEWAVE:	// sequence
		{
			CEditStimArrayDlg dlg;
			dlg.m_pIntervalArrays.RemoveAll();
			dlg.m_pIntervalArrays.Add(&m_outD.parmsChan.GetAt(channel).stimulussequence);
			dlg.m_pstimsaved = &m_stimsaved;
			dlg.m_rate = m_samplingRate;
			if (IDOK == dlg.DoModal())
			{
				OUTPUTPARMS* pParms = &m_outD.parmsChan.GetAt(channel);
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

//void CDAChannelsDlg::OnCbnSelchangeCombochandigital()
//{
	// TODO
	//CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBOCHANDIGITAL);
	//int isel = pCombo->GetCurSel();
	//if (isel != m_iseldigital)
	//	GetDigitalParmsFromDlg(m_iseldigital);
	//m_iseldigital = isel;
	//SetDigitalParmsToDlg(m_iseldigital);
	//OnCbnSelchangeCombosource2();
//}

void CDAChannelsDlg::SetDigitalParmsToDlg(int digitalchannel)
{
	int channel = m_iseldigital + 2;
	OUTPUTPARMS* pParms = &m_outD.parmsChan.GetAt(channel);
	m_ffrequence2 = pParms->dFrequency;
	CComboBox* pCombo2 = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE2));
	int val = pParms->iWaveform;

	SelectComboItem(pCombo2, val);
	UpdateData(FALSE);
}

void CDAChannelsDlg::GetDigitalParmsFromDlg(int digitalchannel)
{
	int channel = m_iseldigital + 2;
	OUTPUTPARMS* pParms = &m_outD.parmsChan.GetAt(channel);
	UpdateData(TRUE);

	pParms->bDigital = TRUE;
	pParms->iChan = channel-2;
	pParms->bON = m_bChannel2;
	pParms->dFrequency = m_ffrequence2;
	CComboBox* pCombo2 = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE2));
	pParms->iWaveform = pCombo2->GetItemData(pCombo2->GetCurSel());
	if (DA_ZERO == pParms->iWaveform)
		pParms->bON = FALSE;
}

void CDAChannelsDlg::OnBnClickedButtonsource(int ID, int channel) 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(ID);
	int isel = pCombo->GetCurSel();
	EditSequence(pCombo->GetItemData(isel), channel);
}

void CDAChannelsDlg::OnBnClickedButtonsource2()
{
	OnBnClickedButtonsource(IDC_COMBOSOURCE2, 0);
}

void CDAChannelsDlg::OnBnClickedButtonsource3()
{
	OnBnClickedButtonsource(IDC_COMBOSOURCE3, 1);
}

void CDAChannelsDlg::OnBnClickedButtonsource4()
{
	OnBnClickedButtonsource(IDC_COMBOSOURCE4, 2);
}

void CDAChannelsDlg::OnBnClickedButtonsource5()
{
	OnBnClickedButtonsource(IDC_COMBOSOURCE5, 3);
}

void CDAChannelsDlg::OnBnClickedButtonsource6()
{
	OnBnClickedButtonsource(IDC_COMBOSOURCE6, 4);
}

void CDAChannelsDlg::OnBnClickedButtonsource7()
{
	OnBnClickedButtonsource(IDC_COMBOSOURCE7, 5);
}

void CDAChannelsDlg::OnBnClickedButtonsource8()
{
	OnBnClickedButtonsource(IDC_COMBOSOURCE8, 6);
}

void CDAChannelsDlg::OnBnClickedButtonsource9()
{
	OnBnClickedButtonsource(IDC_COMBOSOURCE9, 7);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource(int IDCOMBO, int IDBUTTON, int IDFREQ)
{
	CComboBox* pCombo = ((CComboBox*)GetDlgItem(IDCOMBO));
	pCombo->EnableWindow(TRUE);
	DWORD isel = pCombo->GetItemData(pCombo->GetCurSel());
	BOOL	bEnable = TRUE;
	BOOL	bEnable2 = TRUE;
	switch (isel) {
	case DA_SEQUENCEWAVE:	// sequence
	case DA_MSEQWAVE:		// M-seq
		bEnable = FALSE;
		break;
	case DA_SQUAREWAVE:		// square
		bEnable = TRUE;
		bEnable2 = FALSE;
		break;
	case DA_CONSTANT:
	case DA_NOISEWAVE:		// Noise
	case DA_FILEWAVE:		// data file
	case DA_SINEWAVE:		// sinusoid
	case DA_TRIANGLEWAVE:	// triangle
	default:
		bEnable = FALSE;
		bEnable2 = FALSE;
		break;
	}
	GetDlgItem(IDBUTTON)->EnableWindow(bEnable2);
	GetDlgItem(IDFREQ)->EnableWindow(bEnable);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource2()
{
	OnCbnSelchangeCombosource(IDC_COMBOSOURCE2, IDC_BUTTONSOURCE2, IDC_EDITFREQ2);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource3()
{
	OnCbnSelchangeCombosource(IDC_COMBOSOURCE3, IDC_BUTTONSOURCE3, IDC_EDITFREQ3);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource4()
{
	OnCbnSelchangeCombosource(IDC_COMBOSOURCE4, IDC_BUTTONSOURCE4, IDC_EDITFREQ4);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource5()
{
	OnCbnSelchangeCombosource(IDC_COMBOSOURCE5, IDC_BUTTONSOURCE5, IDC_EDITFREQ5);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource6()
{
	OnCbnSelchangeCombosource(IDC_COMBOSOURCE6, IDC_BUTTONSOURCE6, IDC_EDITFREQ6);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource7()
{
	OnCbnSelchangeCombosource(IDC_COMBOSOURCE7, IDC_BUTTONSOURCE7, IDC_EDITFREQ7);
}

void CDAChannelsDlg::OnCbnSelchangeCombosource8()
{
	OnCbnSelchangeCombosource(IDC_COMBOSOURCE8, IDC_BUTTONSOURCE8, IDC_EDITFREQ8); 
}

void CDAChannelsDlg::OnCbnSelchangeCombosource9()
{
	OnCbnSelchangeCombosource(IDC_COMBOSOURCE9, IDC_BUTTONSOURCE9, IDC_EDITFREQ9);
}

