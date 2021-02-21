// EditSpikeClassDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "dbWave.h"
#include "DlgEditSpikeClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEditSpikeClassDlg dialog

IMPLEMENT_DYNAMIC(CEditSpikeClassDlg, CDialog)

CEditSpikeClassDlg::CEditSpikeClassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditSpikeClassDlg::IDD, pParent)
	, m_iClass(0)
{
}

CEditSpikeClassDlg::~CEditSpikeClassDlg()
{
}

void CEditSpikeClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_iClass);
}

BEGIN_MESSAGE_MAP(CEditSpikeClassDlg, CDialog)
END_MESSAGE_MAP()

// CEditSpikeClassDlg message handlers