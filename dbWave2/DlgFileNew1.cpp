
#include "StdAfx.h"
#include "DlgFileNew1.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFileNew1 dialog

IMPLEMENT_DYNAMIC(DlgFileNew1, CDialogEx)

DlgFileNew1::DlgFileNew1(CWnd* p_parent /*=NULL*/)
	: CDialogEx(IDD, p_parent)
{
}

DlgFileNew1::~DlgFileNew1()
= default;

void DlgFileNew1::DoDataExchange(CDataExchange* p_dx)
{
	CDialogEx::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_LIST, m_list);
}

BEGIN_MESSAGE_MAP(DlgFileNew1, CDialogEx)
	ON_BN_CLICKED(IDOK, &DlgFileNew1::on_bn_clicked_ok)
END_MESSAGE_MAP()

// CFileNew1 message handlers

BOOL DlgFileNew1::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_list.AddString(_T("Database"));
	m_list.AddString(_T("Project"));
	//m_list.AddString("Simple text");
	m_list.SetCurSel(0);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgFileNew1::on_bn_clicked_ok()
{
	m_i_cur_sel = m_list.GetCurSel();
	CDialogEx::OnOK();
}
