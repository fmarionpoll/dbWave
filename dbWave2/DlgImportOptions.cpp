#include "StdAfx.h"
#include "DlgImportOptions.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// DlgImportOptions dialog

IMPLEMENT_DYNAMIC(DlgImportOptions, CDialogEx)

DlgImportOptions::DlgImportOptions(CWnd* p_parent /*=NULL*/)
	: CDialogEx(IDD, p_parent)
{
}

DlgImportOptions::~DlgImportOptions()
{
}

void DlgImportOptions::DoDataExchange(CDataExchange* p_dx)
{
	CDialogEx::DoDataExchange(p_dx);
	DDX_Check(p_dx, IDC_CHECK2, m_b_new_i_ds);
	DDX_Check(p_dx, IDC_CHECKDISCARD, m_b_allow_duplicate_files);
	DDX_Check(p_dx, IDC_HEADERFIRSTLINE, m_b_header);
	DDX_Check(p_dx, IDC_READCOLUMNS, m_b_read_columns);
}

BEGIN_MESSAGE_MAP(DlgImportOptions, CDialogEx)
END_MESSAGE_MAP()

// DlgImportOptions message handlers
