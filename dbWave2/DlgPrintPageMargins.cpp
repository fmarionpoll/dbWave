// printpag.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DlgPrintPageMargins.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DlgPrintPageMargins::DlgPrintPageMargins(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgPrintPageMargins::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_LEFTPAGEMARGIN, m_left_page_margin);
	DDX_Text(p_dx, IDC_LOWERPAGEMARGIN, m_bottom_page_margin);
	DDX_Text(p_dx, IDC_RIGHTPAGEMARGIN, m_right_page_margin);
	DDX_Text(p_dx, IDC_UPPERPAGEMARGIN, m_top_page_margin);
}

BEGIN_MESSAGE_MAP(DlgPrintPageMargins, CDialog)

END_MESSAGE_MAP()

BOOL DlgPrintPageMargins::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_left_page_margin = options_view_data->left_page_margin;
	m_bottom_page_margin = options_view_data->bottom_page_margin;
	m_right_page_margin = options_view_data->right_page_margin;
	m_top_page_margin = options_view_data->top_page_margin;
	UpdateData(FALSE);

	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgPrintPageMargins::OnOK()
{
	UpdateData(TRUE);

	options_view_data->left_page_margin = m_left_page_margin;
	options_view_data->bottom_page_margin = m_bottom_page_margin;
	options_view_data->right_page_margin = m_right_page_margin;
	options_view_data->top_page_margin = m_top_page_margin;

	CDialog::OnOK();
}
