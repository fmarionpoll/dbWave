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
	ON_BN_CLICKED(IDC_CHECKCHAN0, &CDAChannelsDlg::OnBnClickedCheckchan0)
	ON_BN_CLICKED(IDC_CHECKCHAN1, &CDAChannelsDlg::OnBnClickedCheckchan1)
	ON_BN_CLICKED(IDC_BUTTONSOURCE1, &CDAChannelsDlg::OnBnClickedButtonsource1)
END_MESSAGE_MAP()


// CDAChannelsDlg message handlers


void CDAChannelsDlg::OnCbnSelchangeCombosource0()
{
	if (!m_bChannel0) 
	{
		GetDlgItem(IDC_COMBOSOURCE0)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC00)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITAMPLITUDE0)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC01)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITFREQ0)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTONSOURCE0)->EnableWindow(FALSE);
		return;
	}
	else
		GetDlgItem(IDC_COMBOSOURCE0)->EnableWindow(TRUE);

	int isel = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0))->GetCurSel();
	BOOL	bEnable = TRUE;
	BOOL	bEnable2 = TRUE;
	BOOL	bEnable3 = FALSE;
	switch (isel) {
	case DA_SINEWAVE:		// sinusoid
	case DA_SQUAREWAVE:		// square
	case DA_TRIANGLEWAVE:	// triangle
		break;

	case DA_SEQUENCEWAVE:	// sequence
		bEnable2 = TRUE;
		bEnable = FALSE;
		bEnable3 = TRUE;
		break;

	case DA_MSEQWAVE:		// M-seq
	case DA_NOISEWAVE:		// Noise
	case DA_FILEWAVE:		// data file
	default:
		bEnable = FALSE;
		bEnable2 = FALSE;
		bEnable3 = TRUE;
		break;
	}
	GetDlgItem(IDC_STATIC00)->EnableWindow(bEnable2); 
	GetDlgItem(IDC_EDITAMPLITUDE0)->EnableWindow(bEnable2);
	GetDlgItem(IDC_STATIC01)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDITFREQ0)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTONSOURCE0)->EnableWindow(bEnable3);
}


void CDAChannelsDlg::OnCbnSelchangeCombosource1()
{
	if (!m_bChannel1)
	{
		GetDlgItem(IDC_COMBOSOURCE1)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC10)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITAMPLITUDE1)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC11)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITFREQ1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTONSOURCE1)->EnableWindow(FALSE);
		return;
	}
	else
		GetDlgItem(IDC_COMBOSOURCE1)->EnableWindow(TRUE);

	int isel = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1))->GetCurSel();
	BOOL	bEnable = TRUE;
	BOOL	bEnable2 = TRUE;
	BOOL	bEnable3 = FALSE;
	switch (isel) {
	case DA_SINEWAVE:		// sinusoid
	case DA_SQUAREWAVE:		// square
	case DA_TRIANGLEWAVE:	// triangle
		break;

	case DA_SEQUENCEWAVE:	// sequence
		bEnable2 = TRUE;
		bEnable = FALSE;
		bEnable3 = TRUE;
		break; 

	case DA_MSEQWAVE:		// M-seq
	case DA_NOISEWAVE:		// Noise
	case DA_FILEWAVE:		// data file
	default:
		bEnable = FALSE;
		bEnable2 = FALSE;
		bEnable3 = TRUE;
		break;
	}
	GetDlgItem(IDC_STATIC10)->EnableWindow(bEnable2);
	GetDlgItem(IDC_EDITAMPLITUDE1)->EnableWindow(bEnable2);
	GetDlgItem(IDC_STATIC11)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDITFREQ1)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTONSOURCE1)->EnableWindow(bEnable3);
}


void CDAChannelsDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	
	int isize = m_outD.parmsChan.GetSize();
	if (isize < 3)
		m_outD.parmsChan.SetSize(3);
	m_outD.parmsChan.GetAt(0).iChan = 0;
	m_outD.parmsChan.GetAt(0).bON = m_bChannel0;
	m_outD.parmsChan.GetAt(0).iWaveform = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0))->GetCurSel();
	m_outD.parmsChan.GetAt(0).dAmplitudeMaxV = m_famplitude0;
	m_outD.parmsChan.GetAt(0).dFrequency = m_ffrequence0;
	
	m_outD.parmsChan.GetAt(1).iChan = 1;
	m_outD.parmsChan.GetAt(1).bON = m_bChannel1;
	m_outD.parmsChan.GetAt(1).iWaveform = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1))->GetCurSel();
	m_outD.parmsChan.GetAt(1).dAmplitudeMaxV = m_famplitude1;
	m_outD.parmsChan.GetAt(1).dFrequency = m_ffrequence1;
	
	m_outD.parmsChan.GetAt(2).iChan = 2;
	m_outD.parmsChan.GetAt(2).bON = m_bChannelDigital;

	CDialogEx::OnOK();
}


BOOL CDAChannelsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int isize = m_outD.parmsChan.GetSize();
	if (isize > 0) {
		m_bChannel0 = m_outD.parmsChan.GetAt(0).bON;
		m_waveformChannel0 = m_outD.parmsChan.GetAt(0).iWaveform;
		m_famplitude0 = m_outD.parmsChan.GetAt(0).dAmplitudeMaxV;
		m_ffrequence0 = m_outD.parmsChan.GetAt(0).dFrequency;
	}
	((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0))->SetCurSel(m_waveformChannel0);
	OnCbnSelchangeCombosource0();

	if (isize > 1) {
		m_bChannel1 = m_outD.parmsChan.GetAt(1).bON;
		m_waveformChannel1 = m_outD.parmsChan.GetAt(1).iWaveform;
		m_famplitude1 = m_outD.parmsChan.GetAt(1).dAmplitudeMaxV;
		m_ffrequence1 = m_outD.parmsChan.GetAt(1).dFrequency;
	}
	((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1))->SetCurSel(m_waveformChannel1);
	OnCbnSelchangeCombosource1();

	if (isize > 2) {
		m_bChannelDigital = m_outD.parmsChan.GetAt(2).bON;
	}

	UpdateData(FALSE);
	return TRUE;  
}


void CDAChannelsDlg::OnBnClickedCheckchan0()
{
	UpdateData(TRUE);
	OnCbnSelchangeCombosource0();
}


void CDAChannelsDlg::OnBnClickedCheckchan1()
{
	UpdateData(TRUE);
	OnCbnSelchangeCombosource1();
}


void CDAChannelsDlg::OnBnClickedButtonsource0()
{
	int isel = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE0))->GetCurSel();
	switch (isel) {
		case DA_MSEQWAVE:		// M-seq
			{
				CEditDAMseqDlg dlg;
				dlg.m_outDParms = m_outD.parmsChan.GetAt(0);
				if (IDOK == dlg.DoModal()) {
					m_outD.parmsChan.GetAt(0) = dlg.m_outDParms;
					}
			}
			break;

		case DA_SEQUENCEWAVE:	// sequence
			{
				CEditStimArrayDlg dlg;
				dlg.m_stim = m_outD.parmsChan.GetAt(0).stimulussequence;
				dlg.m_pstimsaved = &m_stimsaved;
				dlg.m_rate = m_samplingRate;
				if (IDOK == dlg.DoModal())
				{
					m_outD.parmsChan.GetAt(0).stimulussequence = dlg.m_stim;
				}
			}
			break;

		case DA_NOISEWAVE:		// Noise
		case DA_FILEWAVE:		// data file
		default:
			break;
	}
}


void CDAChannelsDlg::OnBnClickedButtonsource1()
{
	int isel = ((CComboBox*)GetDlgItem(IDC_COMBOSOURCE1))->GetCurSel();
	switch (isel) {
		case DA_MSEQWAVE:		// M-seq
		{
			CEditDAMseqDlg dlg;
			dlg.m_outDParms = m_outD.parmsChan.GetAt(1);
			if (IDOK == dlg.DoModal()) {
				m_outD.parmsChan.GetAt(1) = dlg.m_outDParms;
			}
		}
			break;

		case DA_SEQUENCEWAVE:	// sequence
		{
			CEditStimArrayDlg dlg;
			dlg.m_stim = m_outD.parmsChan.GetAt(1).stimulussequence;
			dlg.m_pstimsaved = &m_stimsaved;
			dlg.m_rate = m_samplingRate;
			if (IDOK == dlg.DoModal())
			{
				m_outD.parmsChan.GetAt(1).stimulussequence = dlg.m_stim;
			}
		}
		break;

		case DA_NOISEWAVE:		// Noise
		case DA_FILEWAVE:		// data file
		default:
			break;
	}
}