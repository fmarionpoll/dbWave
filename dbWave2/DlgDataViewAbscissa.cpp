// vdabscissa.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DlgDataViewAbscissa.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgDataViewAbscissa::DlgDataViewAbscissa(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

BOOL DlgDataViewAbscissa::OnInitDialog()
{
	CDialog::OnInitDialog(); // ??
	on_sel_change_abscissa_units(); // after DDX
	GetDlgItem(IDC_FIRSTABCISSA)->SetFocus();
	return FALSE;
}

void DlgDataViewAbscissa::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);

	DDX_Text(p_dx, IDC_FIRSTABCISSA, m_first_abscissa);
	DDX_Text(p_dx, IDC_FRAMEDURATION, m_frame_duration);
	DDX_Text(p_dx, IDC_LASTABCISSA, m_last_abscissa);
	DDX_CBIndex(p_dx, IDC_ABCISSAUNITS, m_abscissa_unit_index);
	DDX_Text(p_dx, IDC_CENTERABCISSA, m_center_abscissa);
}

BEGIN_MESSAGE_MAP(DlgDataViewAbscissa, CDialog)
	ON_CBN_SELCHANGE(IDC_ABCISSAUNITS, on_sel_change_abscissa_units)
	ON_EN_KILLFOCUS(IDC_FIRSTABCISSA, on_kill_focus_abscissa)
	ON_EN_KILLFOCUS(IDC_FRAMEDURATION, on_kill_focus_duration)
	ON_EN_KILLFOCUS(IDC_LASTABCISSA, on_kill_focus_abscissa)
	ON_EN_KILLFOCUS(IDC_CENTERABCISSA, on_kill_focus_center)
END_MESSAGE_MAP()

void DlgDataViewAbscissa::on_sel_change_abscissa_units()
{
	m_abscissa_unit_index = static_cast<CComboBox*>(GetDlgItem(IDC_ABCISSAUNITS))->GetCurSel();
	if (m_previous_index != m_abscissa_unit_index)
	{
		m_first_abscissa = m_first_abscissa * m_abscissa_scale;
		m_last_abscissa = m_last_abscissa * m_abscissa_scale;
		m_very_last_abscissa = m_very_last_abscissa * m_abscissa_scale;

		switch (m_abscissa_unit_index)
		{
		case 0: // ms
			m_abscissa_scale = 0.001f;
			break;
		case 1: // seconds
			m_abscissa_scale = 1.0f;
			break;
		case 2: // minutes
			m_abscissa_scale = 60.0f;
			break;
		case 3: // hours
			m_abscissa_scale = 3600.0f;
			break;
		default: // seconds
			m_abscissa_scale = 1.0f;
			m_abscissa_unit_index = 1;
			break;
		}
		m_first_abscissa = m_first_abscissa / m_abscissa_scale;
		m_last_abscissa = m_last_abscissa / m_abscissa_scale;
		m_very_last_abscissa = m_very_last_abscissa / m_abscissa_scale;
		m_previous_index = m_abscissa_unit_index;
	}
	m_frame_duration = m_last_abscissa - m_first_abscissa;
	m_center_abscissa = m_first_abscissa + m_frame_duration / static_cast<float>(2.);
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

void DlgDataViewAbscissa::on_kill_focus_abscissa()
{
	UpdateData(TRUE);
	check_limits();
}

void DlgDataViewAbscissa::on_kill_focus_duration()
{
	UpdateData(TRUE);
	m_last_abscissa = m_first_abscissa + m_frame_duration;
	m_center_abscissa = m_first_abscissa + m_frame_duration / static_cast<float>(2.);
	check_limits();
}

void DlgDataViewAbscissa::on_kill_focus_center()
{
	float deltaAbscissa = m_center_abscissa;
	UpdateData(TRUE);
	deltaAbscissa -= m_center_abscissa;
	m_first_abscissa -= deltaAbscissa;
	m_last_abscissa -= deltaAbscissa;
	check_limits();
}

void DlgDataViewAbscissa::check_limits()
{
	BOOL flag = FALSE;
	if (m_first_abscissa < 0.)
	{
		m_first_abscissa = 0.0f;
		flag = TRUE;
	}
	if (m_last_abscissa < m_first_abscissa || m_last_abscissa > m_very_last_abscissa)
	{
		m_last_abscissa = m_very_last_abscissa;
		flag = TRUE;
	}
	if (flag)
		MessageBeep(MB_ICONEXCLAMATION);
	m_frame_duration = m_last_abscissa - m_first_abscissa;
	m_center_abscissa = m_first_abscissa + m_frame_duration / static_cast<float>(2.);
	UpdateData(FALSE);
}
