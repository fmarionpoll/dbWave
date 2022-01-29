// xyparame.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "resource.h"
//#include "Editctrl.h"
#include "Xyparame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

XYParametersDlg::XYParametersDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	m_xextent = 0;
	m_xzero = 0;
	m_yextent = 0;
	m_yzero = 0;
	m_bDisplayspikes = FALSE;
	m_bDisplaybars = FALSE;
	m_bDisplaysource = FALSE;
	m_xparam = TRUE;
	m_yparam = TRUE;
}

void XYParametersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_XEXTENT, m_xextent);
	DDX_Text(pDX, IDC_XZERO, m_xzero);
	DDX_Text(pDX, IDC_YEXTENT, m_yextent);
	DDX_Text(pDX, IDC_YZERO, m_yzero);
	DDX_Check(pDX, IDC_DISPLAYSPIKES, m_bDisplayspikes);
	DDX_Check(pDX, IDC_DISPLAYBARS, m_bDisplaybars);
	DDX_Check(pDX, IDC_DISPLAYSOURCE, m_bDisplaysource);
}

BEGIN_MESSAGE_MAP(XYParametersDlg, CDialog)
	ON_EN_CHANGE(IDC_XEXTENT, OnEnChangeXextent)
	ON_EN_CHANGE(IDC_XZERO, OnEnChangeXzero)
	ON_EN_CHANGE(IDC_YEXTENT, OnEnChangeYextent)
	ON_EN_CHANGE(IDC_YZERO, OnEnChangeYzero)
END_MESSAGE_MAP()

BOOL XYParametersDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (!m_xparam) // x parameters are not valid
	{
		GetDlgItem(IDC_XZERO)->EnableWindow(FALSE);
		GetDlgItem(IDC_XEXTENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_YZERO)->SetFocus();
	}
	if (!m_yparam)
	{
		GetDlgItem(IDC_YZERO)->EnableWindow(FALSE);
		GetDlgItem(IDC_YEXTENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_XZERO)->SetFocus();
	}
	// subclass edit controls
	VERIFY(mm_xextent.SubclassDlgItem(IDC_XEXTENT, this));
	VERIFY(mm_xzero.SubclassDlgItem(IDC_XZERO, this));
	VERIFY(mm_yextent.SubclassDlgItem(IDC_YEXTENT, this));
	VERIFY(mm_yzero.SubclassDlgItem(IDC_YZERO, this));

	return FALSE;
	//return TRUE;  // return TRUE  unless you set the focus to a control
}

void XYParametersDlg::OnEnChangeXextent()
{
	// TODO: Add your control notification handler code here
}

void XYParametersDlg::OnEnChangeXzero()
{
	// TODO: Add your control notification handler code here
}

void XYParametersDlg::OnEnChangeYextent()
{
	// TODO: Add your control notification handler code here
}

void XYParametersDlg::OnEnChangeYzero()
{
	// TODO: Add your control notification handler code here
}
