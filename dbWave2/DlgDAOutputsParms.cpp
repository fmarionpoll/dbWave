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

IMPLEMENT_DYNAMIC(DlgDAOutputParameters, CDialogEx)

DlgDAOutputParameters::DlgDAOutputParameters(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD, pParent)
{
}

DlgDAOutputParameters::~DlgDAOutputParameters()
{
}

void DlgDAOutputParameters::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgDAOutputParameters, CDialogEx)
END_MESSAGE_MAP()

// CDAOutputsDlg message handlers
