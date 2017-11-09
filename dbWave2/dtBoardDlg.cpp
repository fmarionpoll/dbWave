#include "stdafx.h"
#include "dbWave.h"
#include "dtBoardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDataTranslationBoardDlg, CDialog)

CDataTranslationBoardDlg::CDataTranslationBoardDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataTranslationBoardDlg::IDD, pParent)
{
}


CDataTranslationBoardDlg::~CDataTranslationBoardDlg()
{
}

void CDataTranslationBoardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}

BEGIN_MESSAGE_MAP(CDataTranslationBoardDlg, CDialog)

END_MESSAGE_MAP()

BOOL CDataTranslationBoardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return FALSE;
}

