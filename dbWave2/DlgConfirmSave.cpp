#include "StdAfx.h"
//#include "dbWave.h"
#include "DlgConfirmSave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgConfirmSave, CDialog)

DlgConfirmSave::DlgConfirmSave(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
	m_time_left_ = 5;
}

DlgConfirmSave::~DlgConfirmSave()
{
}

BEGIN_MESSAGE_MAP(DlgConfirmSave, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL DlgConfirmSave::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_time_left_ = 5; // number of seconds during which the routine will wait
	m_cs_time_left.Format(_T("(in %i seconds)"), m_time_left_);
	SetDlgItemText(IDC_STATIC2, m_cs_time_left);
	SetDlgItemText(IDC_FILENAME, m_cs_file_name);
	SetTimer(1, 1000, nullptr);
	return TRUE;
}

void DlgConfirmSave::OnTimer(UINT n_id_event)
{
	// update variables
	m_time_left_--;
	m_cs_time_left.Format(_T("(in %i seconds)"), m_time_left_);
	SetDlgItemText(IDC_STATIC2, m_cs_time_left);
	// pass info to parent and take appropriate action
	CDialog::OnTimer(n_id_event);
	if (m_time_left_ > 0)
		SetTimer(1, 1000, nullptr); // one more tick
	else
		EndDialog(IDOK); // end of the game
}
