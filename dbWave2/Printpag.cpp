// printpag.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Printpag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrintPageMarginsDlg dialog

CPrintPageMarginsDlg::CPrintPageMarginsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrintPageMarginsDlg::IDD, pParent)
{
	m_leftPageMargin = 0;
	m_bottomPageMargin = 0;
	m_rightPageMargin = 0;
	m_topPageMargin = 0;
	mdPM = nullptr;
}

void CPrintPageMarginsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LEFTPAGEMARGIN, m_leftPageMargin);
	DDX_Text(pDX, IDC_LOWERPAGEMARGIN, m_bottomPageMargin);
	DDX_Text(pDX, IDC_RIGHTPAGEMARGIN, m_rightPageMargin);
	DDX_Text(pDX, IDC_UPPERPAGEMARGIN, m_topPageMargin);
}

BEGIN_MESSAGE_MAP(CPrintPageMarginsDlg, CDialog)

END_MESSAGE_MAP()

BOOL CPrintPageMarginsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_leftPageMargin = mdPM->leftPageMargin;
	m_bottomPageMargin = mdPM->bottomPageMargin;
	m_rightPageMargin = mdPM->rightPageMargin;
	m_topPageMargin = mdPM->topPageMargin;
	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPrintPageMarginsDlg::OnOK()
{
	UpdateData(TRUE);

	mdPM->leftPageMargin = m_leftPageMargin;
	mdPM->bottomPageMargin = m_bottomPageMargin;
	mdPM->rightPageMargin = m_rightPageMargin;
	mdPM->topPageMargin = m_topPageMargin;

	CDialog::OnOK();
}