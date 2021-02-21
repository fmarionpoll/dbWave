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

IMPLEMENT_DYNAMIC(CDAOutputParametersDlg, CDialogEx)

CDAOutputParametersDlg::CDAOutputParametersDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDAOutputParametersDlg::IDD, pParent)
{
}

CDAOutputParametersDlg::~CDAOutputParametersDlg()
{
}

void CDAOutputParametersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDAOutputParametersDlg, CDialogEx)
END_MESSAGE_MAP()

// CDAOutputsDlg message handlers