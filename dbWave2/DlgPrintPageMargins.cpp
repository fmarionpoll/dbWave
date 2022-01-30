// printpag.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DlgPrintPageMargins.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DlgPrintPageMargins::DlgPrintPageMargins(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgPrintPageMargins::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LEFTPAGEMARGIN, m_leftPageMargin);
	DDX_Text(pDX, IDC_LOWERPAGEMARGIN, m_bottomPageMargin);
	DDX_Text(pDX, IDC_RIGHTPAGEMARGIN, m_rightPageMargin);
	DDX_Text(pDX, IDC_UPPERPAGEMARGIN, m_topPageMargin);
}

BEGIN_MESSAGE_MAP(DlgPrintPageMargins, CDialog)

END_MESSAGE_MAP()

BOOL DlgPrintPageMargins::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_leftPageMargin = mdPM->leftPageMargin;
	m_bottomPageMargin = mdPM->bottomPageMargin;
	m_rightPageMargin = mdPM->rightPageMargin;
	m_topPageMargin = mdPM->topPageMargin;
	UpdateData(FALSE);

	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgPrintPageMargins::OnOK()
{
	UpdateData(TRUE);

	mdPM->leftPageMargin = m_leftPageMargin;
	mdPM->bottomPageMargin = m_bottomPageMargin;
	mdPM->rightPageMargin = m_rightPageMargin;
	mdPM->topPageMargin = m_topPageMargin;

	CDialog::OnOK();
}
