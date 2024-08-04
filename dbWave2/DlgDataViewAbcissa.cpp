// vdabscissa.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DlgDataViewAbscissa.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgDataViewAbscissa::DlgDataViewAbscissa(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

BOOL DlgDataViewAbscissa::OnInitDialog()
{
	CDialog::OnInitDialog(); // ??
	OnSelchangeAbscissaunits(); // after DDX
	GetDlgItem(IDC_FIRSTABCISSA)->SetFocus();
	return FALSE;
}

void DlgDataViewAbscissa::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_FIRSTABCISSA, m_firstAbscissa);
	DDX_Text(pDX, IDC_FRAMEDURATION, m_frameDuration);
	DDX_Text(pDX, IDC_LASTABCISSA, m_lastAbscissa);
	DDX_CBIndex(pDX, IDC_ABCISSAUNITS, m_abscissaUnitIndex);
	DDX_Text(pDX, IDC_CENTERABCISSA, m_centerAbscissa);
}

BEGIN_MESSAGE_MAP(DlgDataViewAbscissa, CDialog)
	ON_CBN_SELCHANGE(IDC_ABCISSAUNITS, OnSelchangeAbscissaunits)
	ON_EN_KILLFOCUS(IDC_FIRSTABCISSA, OnKillfocusAbscissa)
	ON_EN_KILLFOCUS(IDC_FRAMEDURATION, OnKillfocusDuration)
	ON_EN_KILLFOCUS(IDC_LASTABCISSA, OnKillfocusAbscissa)
	ON_EN_KILLFOCUS(IDC_CENTERABCISSA, OnKillfocusCenter)
END_MESSAGE_MAP()

void DlgDataViewAbscissa::OnSelchangeAbscissaunits()
{
	m_abscissaUnitIndex = static_cast<CComboBox*>(GetDlgItem(IDC_ABCISSAUNITS))->GetCurSel();
	if (m_previousIndex != m_abscissaUnitIndex)
	{
		m_firstAbscissa = m_firstAbscissa * m_abscissaScale;
		m_lastAbscissa = m_lastAbscissa * m_abscissaScale;
		m_veryLastAbscissa = m_veryLastAbscissa * m_abscissaScale;

		switch (m_abscissaUnitIndex)
		{
		case 0: // ms
			m_abscissaScale = 0.001f;
			break;
		case 1: // seconds
			m_abscissaScale = 1.0f;
			break;
		case 2: // minutes
			m_abscissaScale = 60.0f;
			break;
		case 3: // hours
			m_abscissaScale = 3600.0f;
			break;
		default: // seconds
			m_abscissaScale = 1.0f;
			m_abscissaUnitIndex = 1;
			break;
		}
		m_firstAbscissa = m_firstAbscissa / m_abscissaScale;
		m_lastAbscissa = m_lastAbscissa / m_abscissaScale;
		m_veryLastAbscissa = m_veryLastAbscissa / m_abscissaScale;
		m_previousIndex = m_abscissaUnitIndex;
	}
	m_frameDuration = m_lastAbscissa - m_firstAbscissa;
	m_centerAbscissa = m_firstAbscissa + m_frameDuration / static_cast<float>(2.);
	UpdateData(FALSE);
}

void DlgDataViewAbscissa::OnOK()
{
	// trap CR to validate current field
	switch (GetFocus()->GetDlgCtrlID())
	{
	case 1:
		CDialog::OnOK();
		break;
	case IDC_FRAMEDURATION:
	case IDC_FIRSTABCISSA:
	case IDC_LASTABCISSA:
	case IDC_CENTERABCISSA:
		NextDlgCtrl();
		break;
	default:
		CDialog::OnOK();
		break;
	}
}

void DlgDataViewAbscissa::OnKillfocusAbscissa()
{
	UpdateData(TRUE);
	CheckLimits();
}

void DlgDataViewAbscissa::OnKillfocusDuration()
{
	UpdateData(TRUE);
	m_lastAbscissa = m_firstAbscissa + m_frameDuration;
	m_centerAbscissa = m_firstAbscissa + m_frameDuration / static_cast<float>(2.);
	CheckLimits();
}

void DlgDataViewAbscissa::OnKillfocusCenter()
{
	float deltaAbscissa = m_centerAbscissa;
	UpdateData(TRUE);
	deltaAbscissa -= m_centerAbscissa;
	m_firstAbscissa -= deltaAbscissa;
	m_lastAbscissa -= deltaAbscissa;
	CheckLimits();
}

void DlgDataViewAbscissa::CheckLimits()
{
	BOOL flag = FALSE;
	if (m_firstAbscissa < 0.)
	{
		m_firstAbscissa = 0.0f;
		flag = TRUE;
	}
	if (m_lastAbscissa < m_firstAbscissa || m_lastAbscissa > m_veryLastAbscissa)
	{
		m_lastAbscissa = m_veryLastAbscissa;
		flag = TRUE;
	}
	if (flag)
		MessageBeep(MB_ICONEXCLAMATION);
	m_frameDuration = m_lastAbscissa - m_firstAbscissa;
	m_centerAbscissa = m_firstAbscissa + m_frameDuration / static_cast<float>(2.);
	UpdateData(FALSE);
}
