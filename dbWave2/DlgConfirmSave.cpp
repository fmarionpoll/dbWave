#include "StdAfx.h"
//#include "dbWave.h"
#include "DlgConfirmSave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CDlgConfirmSave, CDialog)
CDlgConfirmSave::CDlgConfirmSave(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfirmSave::IDD, pParent)
{
	m_timeleft = 5;
}

CDlgConfirmSave::~CDlgConfirmSave()
{
}

BEGIN_MESSAGE_MAP(CDlgConfirmSave, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CDlgConfirmSave::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_timeleft = 5;	// number of seconds during which the routine will wait
	m_cstimeleft.Format(_T("(in %i seconds)"), m_timeleft);
	SetDlgItemText(IDC_STATIC2, m_cstimeleft);
	SetDlgItemText(IDC_FILENAME, m_csfilename);
	SetTimer(1, 1000, nullptr);
	return TRUE;
}

void CDlgConfirmSave::OnTimer(UINT nIDEvent)
{
	// update variables
	m_timeleft--;
	m_cstimeleft.Format(_T("(in %i seconds)"), m_timeleft);
	SetDlgItemText(IDC_STATIC2, m_cstimeleft);
	// pass info to parent and take appropriate action
	CDialog::OnTimer(nIDEvent);
	if (m_timeleft > 0)
		SetTimer(1, 1000, nullptr);	// one more tick
	else
		EndDialog(IDOK);		// end of the game
}