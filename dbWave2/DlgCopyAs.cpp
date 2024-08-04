
#include "StdAfx.h"
#include "DlgCopyAs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgCopyAs::DlgCopyAs(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgCopyAs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NABCISSA, m_nabscissa);
	DDX_Text(pDX, IDC_NORDINATES, m_nordinates);
}

BEGIN_MESSAGE_MAP(DlgCopyAs, CDialog)

END_MESSAGE_MAP()

void DlgCopyAs::OnOK()
{
	UpdateData(TRUE);
	m_ioption = 0;
	if (static_cast<CButton*>(GetDlgItem(IDC_COPYMIDDLE))->GetCheck())
		m_ioption = 1;
	m_iunit = 0;
	if (static_cast<CButton*>(GetDlgItem(IDC_UNITMILLIVOLTS))->GetCheck())
		m_iunit = 1;
	m_bgraphics = 0;
	if (static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->GetCheck())
		m_bgraphics = 1;
	CDialog::OnOK();
}

BOOL DlgCopyAs::OnInitDialog()
{
	CDialog::OnInitDialog();
	int IDC = IDC_COPYCONTOURS;
	if (m_ioption == 1)
		IDC = IDC_COPYMIDDLE;
	CheckRadioButton(IDC_COPYCONTOURS, IDC_COPYMIDDLE, IDC);
	IDC = IDC_UNITBINS;
	if (m_iunit == 1)
		IDC = IDC_UNITMILLIVOLTS;
	CheckRadioButton(IDC_UNITBINS, IDC_UNITMILLIVOLTS, IDC);
	IDC = IDC_RADIO2;
	if (m_bgraphics)
		IDC = IDC_RADIO1;
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
