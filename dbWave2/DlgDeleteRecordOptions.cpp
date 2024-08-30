#include "StdAfx.h"
#include "DlgDeleteRecordOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgDeleteRecordOptions, CDialog)

DlgDeleteRecordOptions::DlgDeleteRecordOptions(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

DlgDeleteRecordOptions::~DlgDeleteRecordOptions()
= default;

void DlgDeleteRecordOptions::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Check(p_dx, IDC_CHECK3, m_b_keep_choice);
	DDX_Check(p_dx, IDC_CHECK2, m_b_delete_file);
}

BEGIN_MESSAGE_MAP(DlgDeleteRecordOptions, CDialog)
END_MESSAGE_MAP()

