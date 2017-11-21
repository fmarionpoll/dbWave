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
	, m_bChannel0(FALSE)
	, m_bChannel1(FALSE)
	, m_bChannel2(FALSE)
	, m_famplitude0(0)
	, m_ffrequence0(0)
	, m_famplitude1(0)
	, m_ffrequence1(0)
	, m_ffrequence2(0)
{

}

CDAChannelsDlg::~CDAChannelsDlg()
{
}

void CDAChannelsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECKCHAN0, m_bChannel0);
	DDX_Check(pDX, IDC_CHECKCHAN1, m_bChannel1);
	DDX_Check(pDX, IDC_CHECKDIGITAL, m_bChannel2);
	DDX_Text(pDX, IDC_EDITAMPLITUDE0, m_famplitude0);
	DDX_Text(pDX, IDC_EDITAMPLITUDELOW0, m_famplitudelow0);
	DDX_Text(pDX, IDC_EDITFREQ0, m_ffrequence0);
	DDX_Text(pDX, IDC_EDITAMPLITUDE1, m_famplitude1);
	DDX_Text(pDX, IDC_EDITAMPLITUDELOW1, m_famplitudelow1);
	DDX_Text(pDX, IDC_EDITFREQ1, m_ffrequence1);
	DDX_Text(pDX, IDC_EDITFREQ2, m_ffrequence2);
}

BEGIN_MESSAGE_MAP(CDAChannelsDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE0, &CDAChannelsDlg::OnCbnSelchangeCombosource0)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE1, &CDAChannelsDlg::OnCbnSelchangeCombosource1)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE2, &CDAChannelsDlg::OnCbnSelchangeCombosource2)
	ON_BN_CLICKED(IDOK, &CDAChannelsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECKCHAN0, &CDAChannelsDlg::OnBnClickedCheckchan0)
	ON_BN_CLICKED(IDC_CHECKCHAN1, &CDAChannelsDlg::OnBnClickedCheckchan1)
	ON_BN_CLICKED(IDC_CHECKDIGITAL, &CDAChannelsDlg::OnBnClickedCheckchan2)
	ON_BN_CLICKED(IDC_BUTTONSOURCE0, &CDAChannelsDlg::OnBnClickedButtonsource0)
	ON_BN_CLICKED(IDC_BUTTONSOURCE1, &CDAChannelsDlg::OnBnClickedButtonsource1)
	ON_BN_CLICKED(IDC_BUTTONSOURCE2, &CDAChannelsDlg::OnBnClickedButtonsource2)
	ON_CBN_SELCHANGE(IDC_COMBOCHANDIGITAL, &CDAChannelsDlg::OnCbnSelchangeCombochandigital)
END_MESSAGE_MAP()

// CDAChannelsDlg message handlers

void CDAChannelsDlg::OnCbnSelchangeCombosource0()
{
	CComboBox* pCombo = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0));
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

void CDAChannelsDlg::OnCbnSelchangeCombosource2()
{
	CComboBox* pCombo = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE2));
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
	GetDlgItem(IDC_BUTTONSOURCE2)->EnableWindow(bEnable2);
	GetDlgItem(IDC_STATIC12)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDITFREQ2)->EnableWindow(bEnable);
}

void CDAChannelsDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	
	int isize = m_outD.parmsChan.GetSize();
	if (isize < 10)
		m_outD.parmsChan.SetSize(10);

	int channel = 0;
	CComboBox* pCombo = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0));
	OUTPUTPARMS* pParms = &m_outD.parmsChan.GetAt(channel);
	pParms->iChan			= channel;
	pParms->bON				= m_bChannel0;
	pParms->bDigital		= FALSE;
	pParms->iWaveform		= pCombo->GetItemData(pCombo->GetCurSel());
	pParms->dAmplitudeMaxV	= m_famplitude0;
	pParms->dAmplitudeMinV	= m_famplitudelow0;
	pParms->dFrequency		= m_ffrequence0;
	
	channel = 1;
	pParms = &m_outD.parmsChan.GetAt(channel);
	pCombo = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1));
	pParms->iChan			= channel;
	pParms->bON				= m_bChannel1;
	pParms->bDigital		= FALSE;
	pParms->iWaveform		= pCombo->GetItemData(pCombo->GetCurSel());
	pParms->dAmplitudeMaxV	= m_famplitude1;
	pParms->dAmplitudeMinV	= m_famplitudelow1;
	pParms->dFrequency		= m_ffrequence1;
	
	CComboBox* pCombo2 = ((CComboBox*)GetDlgItem(IDC_COMBOCHANDIGITAL));
	channel = pCombo2->GetCurSel() + 2;
	pParms = &m_outD.parmsChan.GetAt(channel);
	pCombo = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE2));
	pParms->bDigital		= TRUE;
	pParms->iChan			= channel-2;
	pParms->bON				= m_bChannel2;
	pParms->dFrequency		= m_ffrequence2;
	pParms->iWaveform		= pCombo->GetItemData(pCombo->GetCurSel());

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

	int isize = m_outD.parmsChan.GetSize();
	CComboBox* pCombo = NULL;

	pCombo = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE0);
	FillCombo(pCombo, 0);
	if (isize > 0) 
	{
		m_bChannel0 = m_outD.parmsChan.GetAt(0).bON;
		m_waveformChannel0 = m_outD.parmsChan.GetAt(0).iWaveform;
		m_famplitude0 = m_outD.parmsChan.GetAt(0).dAmplitudeMaxV;
		m_famplitudelow0 = m_outD.parmsChan.GetAt(0).dAmplitudeMinV;
		m_ffrequence0 = m_outD.parmsChan.GetAt(0).dFrequency;
	}
	SelectComboItem(pCombo, m_waveformChannel0);
	OnCbnSelchangeCombosource0();
	
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE1);
	FillCombo(pCombo, 1);
	if (isize > 1) 
	{
		m_bChannel1 = m_outD.parmsChan.GetAt(1).bON;
		m_waveformChannel1 = m_outD.parmsChan.GetAt(1).iWaveform;
		m_famplitude1 = m_outD.parmsChan.GetAt(1).dAmplitudeMaxV;
		m_famplitudelow1 = m_outD.parmsChan.GetAt(1).dAmplitudeMinV;
		m_ffrequence1 = m_outD.parmsChan.GetAt(1).dFrequency;
	}
	SelectComboItem(pCombo, m_waveformChannel1);
	OnCbnSelchangeCombosource1();

	pCombo = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE2);
	FillCombo(pCombo, 2);
	if (isize > 2) 
	{
		m_bChannel2 = m_outD.parmsChan.GetAt(2).bON;
		m_waveformChannel2 = m_outD.parmsChan.GetAt(2).iWaveform;
		m_ffrequence2 = m_outD.parmsChan.GetAt(2).dFrequency;
	}
	SelectComboItem(pCombo, m_waveformChannel2);
	OnCbnSelchangeCombosource2();
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBOCHANDIGITAL);
	for (int i = 0; i < 8; i++)
	{
		CString cs;
		cs.Format(_T("channel %i"), i);
		pCombo->AddString(cs);
	}
	m_iseldigital = 0;
	pCombo->SetCurSel(m_iseldigital);

	UpdateData(FALSE);
	return TRUE;  
}

void CDAChannelsDlg::OnBnClickedCheckchan0()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_COMBOSOURCE0)->EnableWindow(m_bChannel0);
	GetDlgItem(IDC_STATIC00)->EnableWindow(m_bChannel0);
	GetDlgItem(IDC_EDITAMPLITUDE0)->EnableWindow(m_bChannel0);
	GetDlgItem(IDC_EDITAMPLITUDELOW0)->EnableWindow(m_bChannel0);
	GetDlgItem(IDC_STATIC01)->EnableWindow(m_bChannel0);
	GetDlgItem(IDC_EDITFREQ0)->EnableWindow(m_bChannel0);
	GetDlgItem(IDC_BUTTONSOURCE0)->EnableWindow(m_bChannel0);
	if (m_bChannel0) 
		OnCbnSelchangeCombosource0();
}

void CDAChannelsDlg::OnBnClickedCheckchan1()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_COMBOSOURCE1)->EnableWindow(m_bChannel1);
	GetDlgItem(IDC_STATIC10)->EnableWindow(m_bChannel1);
	GetDlgItem(IDC_EDITAMPLITUDE1)->EnableWindow(m_bChannel1);
	GetDlgItem(IDC_EDITAMPLITUDELOW1)->EnableWindow(m_bChannel1);
	GetDlgItem(IDC_STATIC11)->EnableWindow(m_bChannel1);
	GetDlgItem(IDC_EDITFREQ1)->EnableWindow(m_bChannel1);
	GetDlgItem(IDC_BUTTONSOURCE1)->EnableWindow(m_bChannel1);
	if (m_bChannel1)
		OnCbnSelchangeCombosource1();
}

void CDAChannelsDlg::OnBnClickedCheckchan2()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_COMBOSOURCE2)->EnableWindow(m_bChannel2);
	GetDlgItem(IDC_BUTTONSOURCE2)->EnableWindow(m_bChannel2);
	GetDlgItem(IDC_COMBOCHANDIGITAL)->EnableWindow(m_bChannel2);
	if (m_bChannel2)
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

void CDAChannelsDlg::OnBnClickedButtonsource2()
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBOSOURCE2);
	int isel = pCombo->GetCurSel();
	CComboBox* pCombo1 = (CComboBox*)GetDlgItem(IDC_COMBOCHANDIGITAL);
	int channel = 2+ pCombo1->GetCurSel();
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

void CDAChannelsDlg::OnCbnSelchangeCombochandigital()
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBOCHANDIGITAL);
	int isel = pCombo->GetCurSel();
	if (isel != m_iseldigital)
		GetDigitalParmsFromDlg(m_iseldigital);
	m_iseldigital = isel;
	SetDigitalParmsToDlg(m_iseldigital);
	OnCbnSelchangeCombosource2();
}

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
}
