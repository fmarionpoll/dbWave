// DAChannelsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"
#include "DAChannelsDlg.h"
#include "afxdialogex.h"
#include "EditStimArrayDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDAChannelsDlg dialog

IMPLEMENT_DYNAMIC(CDAChannelsDlg, CDialogEx)

CDAChannelsDlg::CDAChannelsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDAChannelsDlg::IDD, pParent)
	, m_bChannel0(FALSE)
	, m_bChannel1(FALSE)
	, m_bChannelDigital(FALSE)
	, m_famplitude0(0)
	, m_ffrequence0(0)
	, m_famplitude1(0)
	, m_ffrequence1(0)
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
	DDX_Check(pDX, IDC_CHECKDIGITAL, m_bChannelDigital);
	DDX_Text(pDX, IDC_EDITAMPLITUDE0, m_famplitude0);
	DDX_Text(pDX, IDC_EDITFREQ0, m_ffrequence0);
	DDX_Text(pDX, IDC_EDITAMPLITUDE1, m_famplitude1);
	DDX_Text(pDX, IDC_EDITFREQ1, m_ffrequence1);
}


BEGIN_MESSAGE_MAP(CDAChannelsDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE0, &CDAChannelsDlg::OnCbnSelchangeCombosource0)
	ON_CBN_SELCHANGE(IDC_COMBOSOURCE1, &CDAChannelsDlg::OnCbnSelchangeCombosource1)
	ON_BN_CLICKED(IDOK, &CDAChannelsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTONSOURCE0, &CDAChannelsDlg::OnBnClickedButtonsource0)
END_MESSAGE_MAP()


// CDAChannelsDlg message handlers


void CDAChannelsDlg::OnCbnSelchangeCombosource0()
{
	int isel = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0))->GetCurSel();
	BOOL	bEnabled = TRUE;
	BOOL	bEnabled2 = FALSE;
	switch (isel) {
	case DA_SINEWAVE:		// sinusoid
	case DA_SQUAREWAVE:		// square
	case DA_TRIANGLEWAVE:	// triangle
		bEnabled = TRUE;
		break;

	case DA_SEQUENCEWAVE:	// sequence
		bEnabled2 = TRUE;
	case DA_MSEQWAVE:		// M-seq
	case DA_NOISEWAVE:		// Noise
	case DA_FILEWAVE:		// data file
	default:
		bEnabled = FALSE;
		break;
	}
	GetDlgItem(IDC_STATIC00)->EnableWindow(bEnabled); 
	GetDlgItem(IDC_EDITAMPLITUDE0)->EnableWindow(bEnabled);
	GetDlgItem(IDC_STATIC01)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDITFREQ0)->EnableWindow(bEnabled);
	GetDlgItem(IDC_BUTTONSOURCE0)->EnableWindow(bEnabled2);
}


void CDAChannelsDlg::OnCbnSelchangeCombosource1()
{
	int isel = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1))->GetCurSel();
	BOOL	bEnabled = TRUE;
	BOOL	bEnabled2 = FALSE;
	switch (isel) {
	case DA_SINEWAVE:		// sinusoid
	case DA_SQUAREWAVE:		// square
	case DA_TRIANGLEWAVE:	// triangle
		bEnabled = TRUE;
		break;

	case DA_SEQUENCEWAVE:	// sequence
		bEnabled2 = TRUE;
	case DA_MSEQWAVE:		// M-seq
	case DA_NOISEWAVE:		// Noise
	case DA_FILEWAVE:		// data file
	default:
		bEnabled = FALSE;
		break;
	}
	GetDlgItem(IDC_STATIC10)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDITAMPLITUDE1)->EnableWindow(bEnabled);
	GetDlgItem(IDC_STATIC11)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDITFREQ1)->EnableWindow(bEnabled);
	GetDlgItem(IDC_BUTTONSOURCE1)->EnableWindow(bEnabled2);
}


void CDAChannelsDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	m_outD.bChan0 = m_bChannel0 ;
	m_outD.bChan1 = m_bChannel1;
	m_outD.bDigitalOut = m_bChannelDigital;

	m_outD.DAparmsChan0.iWaveform = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0))->GetCurSel();
	m_outD.DAparmsChan0.dAmplitudeMaxV = m_famplitude0;
	m_outD.DAparmsChan0.dFrequency = m_ffrequence0;
	
	m_outD.DAparmsChan1.iWaveform = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1))->GetCurSel();
	m_outD.DAparmsChan1.dAmplitudeMaxV = m_famplitude1;
	m_outD.DAparmsChan1.dFrequency = m_ffrequence1;
	
	CDialogEx::OnOK();
}


BOOL CDAChannelsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_bChannel0 = m_outD.bChan0;
	m_bChannel1 = m_outD.bChan1;
	m_bChannelDigital	= m_outD.bDigitalOut;
	
	m_waveformChannel0	= m_outD.DAparmsChan0.iWaveform;
	m_famplitude0		= m_outD.DAparmsChan0.dAmplitudeMaxV;
	m_ffrequence0		= m_outD.DAparmsChan0.dFrequency;
	((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0))->SetCurSel(m_waveformChannel0);
	OnCbnSelchangeCombosource0();

	m_waveformChannel1	= m_outD.DAparmsChan1.iWaveform;
	m_famplitude1		= m_outD.DAparmsChan1.dAmplitudeMaxV;
	m_ffrequence1		= m_outD.DAparmsChan1.dFrequency;
	((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1))->SetCurSel(m_waveformChannel1);
	OnCbnSelchangeCombosource1();

	UpdateData(FALSE);
	return TRUE;  
}


void CDAChannelsDlg::OnBnClickedButtonsource0()
{
	CEditStimArrayDlg dlg;
	dlg.m_stim = m_outD.DAparmsChan0.stimulussequence;
	dlg.m_pstimsaved = &m_stimsaved;
	dlg.m_rate = m_samplingRate;
	if (IDOK == dlg.DoModal())
	{
		m_outD.DAparmsChan0.stimulussequence = dlg.m_stim;
	}
}
