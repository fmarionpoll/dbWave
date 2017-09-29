// DeleteRecordOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"
#include "DeleteRecordOptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDeleteRecordOptionsDlg dialog

IMPLEMENT_DYNAMIC(CDeleteRecordOptionsDlg, CDialog)

CDeleteRecordOptionsDlg::CDeleteRecordOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteRecordOptionsDlg::IDD, pParent)
	, m_bKeepChoice(FALSE)
	, m_bDeleteFile(FALSE)
{

}

CDeleteRecordOptionsDlg::~CDeleteRecordOptionsDlg()
{
}

void CDeleteRecordOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK3, m_bKeepChoice);
	DDX_Check(pDX, IDC_CHECK2, m_bDeleteFile);
}


BEGIN_MESSAGE_MAP(CDeleteRecordOptionsDlg, CDialog)
END_MESSAGE_MAP()


// CDeleteRecordOptionsDlg message handlers
