// FileNew1.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"
#include "FileNew1.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFileNew1 dialog

IMPLEMENT_DYNAMIC(CFileNew1, CDialogEx)

CFileNew1::CFileNew1(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileNew1::IDD, pParent)
{
	m_icursel = 0;
}

CFileNew1::~CFileNew1()
{
}

void CFileNew1::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CFileNew1, CDialogEx)
	ON_BN_CLICKED(IDOK, &CFileNew1::OnBnClickedOk)
END_MESSAGE_MAP()


// CFileNew1 message handlers


BOOL CFileNew1::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_list.AddString(_T("Database"));
	m_list.AddString(_T("Project"));
	//m_list.AddString("Simple text");
	m_list.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CFileNew1::OnBnClickedOk()
{
	m_icursel = m_list.GetCurSel();
	CDialogEx::OnOK();
}
