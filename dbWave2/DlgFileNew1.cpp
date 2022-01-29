// FileNew1.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "DlgFileNew1.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFileNew1 dialog

IMPLEMENT_DYNAMIC(CDlgFileNew1, CDialogEx)

CDlgFileNew1::CDlgFileNew1(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD, pParent)
{
	m_icursel = 0;
}

CDlgFileNew1::~CDlgFileNew1()
{
}

void CDlgFileNew1::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}

BEGIN_MESSAGE_MAP(CDlgFileNew1, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgFileNew1::OnBnClickedOk)
END_MESSAGE_MAP()

// CFileNew1 message handlers

BOOL CDlgFileNew1::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_list.AddString(_T("Database"));
	m_list.AddString(_T("Project"));
	//m_list.AddString("Simple text");
	m_list.SetCurSel(0);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgFileNew1::OnBnClickedOk()
{
	m_icursel = m_list.GetCurSel();
	CDialogEx::OnOK();
}
