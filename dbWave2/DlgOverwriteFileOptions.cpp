#include "StdAfx.h"
#include "DlgOverwriteFileOptions.h"


IMPLEMENT_DYNAMIC(DlgOverwriteFileOptions, CDialogEx)

DlgOverwriteFileOptions::DlgOverwriteFileOptions(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_OPTIONSOVERWRITEFILE, pParent)
{

}

DlgOverwriteFileOptions::~DlgOverwriteFileOptions()
{
}

void DlgOverwriteFileOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK3, m_bKeepChoice);
	DDX_Check(pDX, IDC_CHECK2, m_bOverwriteFile);
}


BEGIN_MESSAGE_MAP(DlgOverwriteFileOptions, CDialogEx)
END_MESSAGE_MAP()


// DlgOverwriteFileOptions message handlers
