// ImportOptions.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "ImportOptions.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CImportOptionsDlg dialog

IMPLEMENT_DYNAMIC(CImportOptionsDlg, CDialogEx)

CImportOptionsDlg::CImportOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImportOptionsDlg::IDD, pParent)
{
}

CImportOptionsDlg::~CImportOptionsDlg()
{
}

void CImportOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK2, m_bNewIDs);
	DDX_Check(pDX, IDC_CHECKDISCARD, m_bAllowDuplicateFiles);
	DDX_Check(pDX, IDC_HEADERFIRSTLINE, m_bHeader);
	DDX_Check(pDX, IDC_READCOLUMNS, m_bReadColumns);
}

BEGIN_MESSAGE_MAP(CImportOptionsDlg, CDialogEx)
END_MESSAGE_MAP()

// CImportOptionsDlg message handlers