// datacomm.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "Datacomm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataCommentsDlg dialog

CDataCommentsDlg::CDataCommentsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataCommentsDlg::IDD, pParent), m_pvO(nullptr)
{
	m_bacqchans = FALSE;
	m_bacqcomments = FALSE;
	m_bacqdate = FALSE;
	m_bacqtime = FALSE;
	m_bfilesize = FALSE;
	m_bacqchsetting = FALSE;
	m_btoExcel = FALSE;
	m_bdatabasecols = false;
}

void CDataCommentsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ACQCHANS, m_bacqchans);
	DDX_Check(pDX, IDC_ACQCOMMENTS, m_bacqcomments);
	DDX_Check(pDX, IDC_ACQDATE, m_bacqdate);
	DDX_Check(pDX, IDC_ACQTIME, m_bacqtime);
	DDX_Check(pDX, IDC_FILESIZE, m_bfilesize);
	DDX_Check(pDX, IDC_ACQCHSETTING, m_bacqchsetting);
	DDX_Check(pDX, IDC_CHECK1, m_btoExcel);
	DDX_Check(pDX, IDC_DATABASECOLS, m_bdatabasecols);
}

BEGIN_MESSAGE_MAP(CDataCommentsDlg, CDialog)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataCommentsDlg message handlers

BOOL CDataCommentsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bacqchans = m_pvO->bacqchcomment;		// copy parms into
	m_bacqchsetting = m_pvO->bacqchsetting;	// dlg parms
	m_bacqcomments = m_pvO->bacqcomments;
	m_bacqdate = m_pvO->bacqdate;
	m_bacqtime = m_pvO->bacqtime;
	m_bfilesize = m_pvO->bfilesize;
	m_btoExcel = m_pvO->btoExcel;
	m_bdatabasecols = m_pvO->bdatabasecols;

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDataCommentsDlg::OnOK()
{
	UpdateData(TRUE);
	if (m_pvO->bacqchcomment != m_bacqchans
		|| m_pvO->bacqchsetting != m_bacqchsetting
		|| m_pvO->bacqcomments != m_bacqcomments
		|| m_pvO->bacqdate != m_bacqdate
		|| m_pvO->bacqtime != m_bacqtime
		|| m_pvO->bfilesize != m_bfilesize
		|| m_pvO->btoExcel != m_btoExcel
		|| m_pvO->bdatabasecols != m_bdatabasecols)
	{
		m_pvO->bacqchcomment = m_bacqchans;
		m_pvO->bacqchsetting = m_bacqchsetting;
		m_pvO->bacqcomments = m_bacqcomments;
		m_pvO->bacqdate = m_bacqdate;
		m_pvO->bacqtime = m_bacqtime;
		m_pvO->bfilesize = m_bfilesize;
		m_pvO->btoExcel = m_btoExcel;
		m_pvO->bdatabasecols = m_bdatabasecols;
		m_pvO->b_changed = TRUE;	// save new params in app array
	}
	CDialog::OnOK();
}