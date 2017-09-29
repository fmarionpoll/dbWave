// DAOutputsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"
#include "DAOutputsDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDAOutputsDlg dialog

IMPLEMENT_DYNAMIC(CDAOutputsDlg, CDialogEx)

CDAOutputsDlg::CDAOutputsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDAOutputsDlg::IDD, pParent)
{

}

CDAOutputsDlg::~CDAOutputsDlg()
{
}

void CDAOutputsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDAOutputsDlg, CDialogEx)
END_MESSAGE_MAP()


// CDAOutputsDlg message handlers
