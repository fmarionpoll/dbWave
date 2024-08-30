// RejectRecordDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DlgRejectRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgRejectRecord::DlgRejectRecord(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgRejectRecord::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Check(p_dx, IDC_CHECK1, m_b_consecutive_points);
	DDX_Text(p_dx, IDC_EDIT1, m_n_consecutive_points);
	DDX_Text(p_dx, IDC_EDIT2, m_jitter);
	DDX_Text(p_dx, IDC_EDIT3, m_flag);
}

BEGIN_MESSAGE_MAP(DlgRejectRecord, CDialog)

END_MESSAGE_MAP()
