#include "StdAfx.h"
#include "DlgImportOptions.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// DlgImportOptions dialog

IMPLEMENT_DYNAMIC(DlgImportOptions, CDialogEx)

DlgImportOptions::DlgImportOptions(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD, pParent)
{
}

DlgImportOptions::~DlgImportOptions()
{
}

void DlgImportOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK2, m_bNewIDs);
	DDX_Check(pDX, IDC_CHECKDISCARD, m_bAllowDuplicateFiles);
	DDX_Check(pDX, IDC_HEADERFIRSTLINE, m_bHeader);
	DDX_Check(pDX, IDC_READCOLUMNS, m_bReadColumns);
}

BEGIN_MESSAGE_MAP(DlgImportOptions, CDialogEx)
END_MESSAGE_MAP()

// DlgImportOptions message handlers
