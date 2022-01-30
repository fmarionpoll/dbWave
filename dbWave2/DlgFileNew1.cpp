
#include "StdAfx.h"
#include "DlgFileNew1.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFileNew1 dialog

IMPLEMENT_DYNAMIC(DlgFileNew1, CDialogEx)

DlgFileNew1::DlgFileNew1(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD, pParent)
{
}

DlgFileNew1::~DlgFileNew1()
= default;

void DlgFileNew1::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}

BEGIN_MESSAGE_MAP(DlgFileNew1, CDialogEx)
	ON_BN_CLICKED(IDOK, &DlgFileNew1::OnBnClickedOk)
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

void DlgFileNew1::OnBnClickedOk()
{
	m_icursel = m_list.GetCurSel();
	CDialogEx::OnOK();
}
