// RejectRecordDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DlgRejectRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CRejectRecordDlg dialog

CRejectRecordDlg::CRejectRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRejectRecordDlg::IDD, pParent)
{
	m_bconsecutivepoints = FALSE;
	m_Nconsecutivepoints = 0;
	m_jitter = 0;
	m_flag = 1;
}

void CRejectRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_bconsecutivepoints);
	DDX_Text(pDX, IDC_EDIT1, m_Nconsecutivepoints);
	DDX_Text(pDX, IDC_EDIT2, m_jitter);
	DDX_Text(pDX, IDC_EDIT3, m_flag);
}

BEGIN_MESSAGE_MAP(CRejectRecordDlg, CDialog)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRejectRecordDlg message handlers