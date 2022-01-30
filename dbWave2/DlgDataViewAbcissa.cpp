// vdabcissa.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DlgDataViewAbcissa.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgDataViewAbcissa::DlgDataViewAbcissa(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

BOOL DlgDataViewAbcissa::OnInitDialog()
{
	CDialog::OnInitDialog(); // ??
	OnSelchangeAbcissaunits(); // after DDX
	GetDlgItem(IDC_FIRSTABCISSA)->SetFocus();
	return FALSE;
}

void DlgDataViewAbcissa::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_FIRSTABCISSA, m_firstAbcissa);
	DDX_Text(pDX, IDC_FRAMEDURATION, m_frameDuration);
	DDX_Text(pDX, IDC_LASTABCISSA, m_lastAbcissa);
	DDX_CBIndex(pDX, IDC_ABCISSAUNITS, m_abcissaUnitIndex);
	DDX_Text(pDX, IDC_CENTERABCISSA, m_centerAbcissa);
}

BEGIN_MESSAGE_MAP(DlgDataViewAbcissa, CDialog)
	ON_CBN_SELCHANGE(IDC_ABCISSAUNITS, OnSelchangeAbcissaunits)
	ON_EN_KILLFOCUS(IDC_FIRSTABCISSA, OnKillfocusAbcissa)
	ON_EN_KILLFOCUS(IDC_FRAMEDURATION, OnKillfocusDuration)
	ON_EN_KILLFOCUS(IDC_LASTABCISSA, OnKillfocusAbcissa)
	ON_EN_KILLFOCUS(IDC_CENTERABCISSA, OnKillfocusCenter)
END_MESSAGE_MAP()

void DlgDataViewAbcissa::OnSelchangeAbcissaunits()
{
	m_abcissaUnitIndex = static_cast<CComboBox*>(GetDlgItem(IDC_ABCISSAUNITS))->GetCurSel();
	if (m_previousIndex != m_abcissaUnitIndex)
	{
		m_firstAbcissa = m_firstAbcissa * m_abcissaScale;
		m_lastAbcissa = m_lastAbcissa * m_abcissaScale;
		m_veryLastAbcissa = m_veryLastAbcissa * m_abcissaScale;

		switch (m_abcissaUnitIndex)
		{
		case 0: // ms
			m_abcissaScale = 0.001f;
			break;
		case 1: // seconds
			m_abcissaScale = 1.0f;
			break;
		case 2: // minutes
			m_abcissaScale = 60.0f;
			break;
		case 3: // hours
			m_abcissaScale = 3600.0f;
			break;
		default: // seconds
			m_abcissaScale = 1.0f;
			m_abcissaUnitIndex = 1;
			break;
		}
		m_firstAbcissa = m_firstAbcissa / m_abcissaScale;
		m_lastAbcissa = m_lastAbcissa / m_abcissaScale;
		m_veryLastAbcissa = m_veryLastAbcissa / m_abcissaScale;
		m_previousIndex = m_abcissaUnitIndex;
	}
	m_frameDuration = m_lastAbcissa - m_firstAbcissa;
	m_centerAbcissa = m_firstAbcissa + m_frameDuration / static_cast<float>(2.);
	UpdateData(FALSE);
}

void DlgDataViewAbcissa::OnOK()
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

void DlgDataViewAbcissa::OnKillfocusAbcissa()
{
	UpdateData(TRUE);
	CheckLimits();
}

void DlgDataViewAbcissa::OnKillfocusDuration()
{
	UpdateData(TRUE);
	m_lastAbcissa = m_firstAbcissa + m_frameDuration;
	m_centerAbcissa = m_firstAbcissa + m_frameDuration / static_cast<float>(2.);
	CheckLimits();
}

void DlgDataViewAbcissa::OnKillfocusCenter()
{
	float deltaAbcissa = m_centerAbcissa;
	UpdateData(TRUE);
	deltaAbcissa -= m_centerAbcissa;
	m_firstAbcissa -= deltaAbcissa;
	m_lastAbcissa -= deltaAbcissa;
	CheckLimits();
}

void DlgDataViewAbcissa::CheckLimits()
{
	BOOL flag = FALSE;
	if (m_firstAbcissa < 0.)
	{
		m_firstAbcissa = 0.0f;
		flag = TRUE;
	}
	if (m_lastAbcissa < m_firstAbcissa || m_lastAbcissa > m_veryLastAbcissa)
	{
		m_lastAbcissa = m_veryLastAbcissa;
		flag = TRUE;
	}
	if (flag)
		MessageBeep(MB_ICONEXCLAMATION);
	m_frameDuration = m_lastAbcissa - m_firstAbcissa;
	m_centerAbcissa = m_firstAbcissa + m_frameDuration / static_cast<float>(2.);
	UpdateData(FALSE);
}
