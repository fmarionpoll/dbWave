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
	, m_bNewIDs(FALSE)
	, m_bAllowDuplicateFiles(FALSE)
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
}


BEGIN_MESSAGE_MAP(CImportOptionsDlg, CDialogEx)
END_MESSAGE_MAP()


// CImportOptionsDlg message handlers
