#include "StdAfx.h"
#include "DlgOverwriteFileOptions.h"


IMPLEMENT_DYNAMIC(DlgOverwriteFileOptions, CDialog)

DlgOverwriteFileOptions::DlgOverwriteFileOptions(CWnd* p_parent /*=nullptr*/)
	: CDialog(IDD_OPTIONSOVERWRITEFILE, p_parent)
{

}

//DlgOverwriteFileOptions::~DlgOverwriteFileOptions()
//= default;

void DlgOverwriteFileOptions::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Check(p_dx, IDC_CHECK3, m_b_keep_choice);
	DDX_Check(p_dx, IDC_CHECK2, m_b_overwrite_file);
}


BEGIN_MESSAGE_MAP(DlgOverwriteFileOptions, CDialog)

END_MESSAGE_MAP()


