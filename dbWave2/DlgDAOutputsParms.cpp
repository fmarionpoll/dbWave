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

DlgDAOutputParameters::DlgDAOutputParameters(CWnd* p_parent /*=NULL*/)
	: CDialogEx(IDD, p_parent)
{
}

DlgDAOutputParameters::~DlgDAOutputParameters()
{
}

void DlgDAOutputParameters::DoDataExchange(CDataExchange* p_dx)
{
	CDialogEx::DoDataExchange(p_dx);
}

BEGIN_MESSAGE_MAP(DlgDAOutputParameters, CDialogEx)
END_MESSAGE_MAP()

// CDAOutputsDlg message handlers
