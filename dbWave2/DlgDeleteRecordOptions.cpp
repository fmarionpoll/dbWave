#include "StdAfx.h"
#include "DlgDeleteRecordOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgDeleteRecordOptions, CDialog)

DlgDeleteRecordOptions::DlgDeleteRecordOptions(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

DlgDeleteRecordOptions::~DlgDeleteRecordOptions()
= default;

void DlgDeleteRecordOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK3, m_bKeepChoice);
	DDX_Check(pDX, IDC_CHECK2, m_bDeleteFile);
}

BEGIN_MESSAGE_MAP(DlgDeleteRecordOptions, CDialog)
END_MESSAGE_MAP()

