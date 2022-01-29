// RejectRecordDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DlgRejectRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CDlgRejectRecord::CDlgRejectRecord(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	m_bconsecutivepoints = FALSE;
	m_Nconsecutivepoints = 0;
	m_jitter = 0;
	m_flag = 1;
}

void CDlgRejectRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_bconsecutivepoints);
	DDX_Text(pDX, IDC_EDIT1, m_Nconsecutivepoints);
	DDX_Text(pDX, IDC_EDIT2, m_jitter);
	DDX_Text(pDX, IDC_EDIT3, m_flag);
}

BEGIN_MESSAGE_MAP(CDlgRejectRecord, CDialog)

END_MESSAGE_MAP()
