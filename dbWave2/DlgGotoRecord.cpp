#include "StdAfx.h"
#include "resource.h"
#include "DlgGotoRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgGotoRecord::DlgGotoRecord(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
	m_record_id = 0;
	m_record_pos = 0;
	m_b_goto_record_id = false;
}

void DlgGotoRecord::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_EDIT2, m_record_id);
	DDX_Text(p_dx, IDC_EDIT1, m_record_pos);
}

BEGIN_MESSAGE_MAP(DlgGotoRecord, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, on_clicked_position)
	ON_BN_CLICKED(IDC_RADIO2, on_clicked_id)
END_MESSAGE_MAP()

BOOL DlgGotoRecord::OnInitDialog()
{
	CDialog::OnInitDialog();
	set_options();
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgGotoRecord::set_options() const
{
	static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->SetCheck(!m_b_goto_record_id);
	static_cast<CButton*>(GetDlgItem(IDC_RADIO2))->SetCheck(m_b_goto_record_id);
	int i_select = IDC_EDIT2;
	int i_disable = IDC_EDIT1;
	if (!m_b_goto_record_id)
	{
		i_disable = IDC_EDIT2;
		i_select = IDC_EDIT1;
	}
	auto p_wnd = static_cast<CEdit*>(GetDlgItem(i_select));
	p_wnd->EnableWindow(TRUE);
	p_wnd->SetFocus();
	p_wnd->SetSel(0, -1, FALSE);
	GetDlgItem(i_disable)->EnableWindow(FALSE);
}

void DlgGotoRecord::on_clicked_position()
{
	m_b_goto_record_id = FALSE;
	set_options();
}

void DlgGotoRecord::on_clicked_id()
{
	m_b_goto_record_id = TRUE;
	set_options();
}
