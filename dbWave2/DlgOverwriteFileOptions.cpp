#include "StdAfx.h"
#include "DlgOverwriteFileOptions.h"


IMPLEMENT_DYNAMIC(DlgOverwriteFileOptions, CDialog)

DlgOverwriteFileOptions::DlgOverwriteFileOptions(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_OPTIONSOVERWRITEFILE, pParent)
{

}

//DlgOverwriteFileOptions::~DlgOverwriteFileOptions()
//= default;

void DlgOverwriteFileOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK3, m_bKeepChoice);
	DDX_Check(pDX, IDC_CHECK2, m_bOverwriteFile);
}


BEGIN_MESSAGE_MAP(DlgOverwriteFileOptions, CDialog)

END_MESSAGE_MAP()


