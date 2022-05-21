#include "StdAfx.h"
#include "DlgAbout.h"

#include <afxdao.h>
#include "Fileversion.h"

DlgAbout::DlgAbout() : CDialogEx(IDD)
{
}

void DlgAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgAbout, CDialogEx)
END_MESSAGE_MAP()

#pragma warning(disable : 4995)
BOOL DlgAbout::OnInitDialog()
{
	CDialog::OnInitDialog();

	try
	{
		CDaoWorkspace dao_workspace;
		dao_workspace.Create(_T("VersionWorkspace"), _T("Admin"), _T(""));
		GetDlgItem(IDC_VERSION)->SetWindowText(dao_workspace.GetVersion());
	}
	catch (CDaoException* e)
	{
		e->ReportError();
		e->Delete();
	}

	CFileVersion c_fv;
	const CString cs_app_name = _T("dbwave2.exe");
	c_fv.Open(cs_app_name);
	GetDlgItem(IDC_DBWAVEVERSION)->SetWindowText(c_fv.GetProductVersion());
	GetDlgItem(IDC_STATIC7)->SetWindowText(c_fv.GetLegalCopyright());

	return TRUE;
}

