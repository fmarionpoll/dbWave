
#include "StdAfx.h"
#include "DlgCopyAs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgCopyAs::DlgCopyAs(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgCopyAs::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_NABCISSA, m_n_abscissa);
	DDX_Text(p_dx, IDC_NORDINATES, m_n_ordinates);
}

BEGIN_MESSAGE_MAP(DlgCopyAs, CDialog)

END_MESSAGE_MAP()

void DlgCopyAs::OnOK()
{
	UpdateData(TRUE);
	m_i_option = 0;
	if (static_cast<CButton*>(GetDlgItem(IDC_COPYMIDDLE))->GetCheck())
		m_i_option = 1;
	m_i_unit = 0;
	if (static_cast<CButton*>(GetDlgItem(IDC_UNITMILLIVOLTS))->GetCheck())
		m_i_unit = 1;
	b_graphics = 0;
	if (static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->GetCheck())
		b_graphics = 1;
	CDialog::OnOK();
}

BOOL DlgCopyAs::OnInitDialog()
{
	CDialog::OnInitDialog();
	int IDC = IDC_COPYCONTOURS;
	if (m_i_option == 1)
		IDC = IDC_COPYMIDDLE;
	CheckRadioButton(IDC_COPYCONTOURS, IDC_COPYMIDDLE, IDC);
	IDC = IDC_UNITBINS;
	if (m_i_unit == 1)
		IDC = IDC_UNITMILLIVOLTS;
	CheckRadioButton(IDC_UNITBINS, IDC_UNITMILLIVOLTS, IDC);
	IDC = IDC_RADIO2;
	if (b_graphics)
		IDC = IDC_RADIO1;
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
