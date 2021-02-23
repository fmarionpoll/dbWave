// DAOutputsDlg.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "DlgDAOutputsParms.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDAOutputsDlg dialog

IMPLEMENT_DYNAMIC(CDlgDAOutputParameters, CDialogEx)

CDlgDAOutputParameters::CDlgDAOutputParameters(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgDAOutputParameters::IDD, pParent)
{
}

CDlgDAOutputParameters::~CDlgDAOutputParameters()
{
}

void CDlgDAOutputParameters::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgDAOutputParameters, CDialogEx)
END_MESSAGE_MAP()

// CDAOutputsDlg message handlers