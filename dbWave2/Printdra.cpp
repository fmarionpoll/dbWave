// printdra.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "./Printdra.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPrintDrawAreaDlg::CPrintDrawAreaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
	  , m_bFilterDat(FALSE)
{
	m_HeightDoc = 0;
	m_heightSeparator = 0;
	m_WidthDoc = 0;
	m_spkheight = 0;
	m_spkwidth = 0;
	mdPM = nullptr;
}

void CPrintDrawAreaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HEIGHTDOC, m_HeightDoc);
	DDX_Text(pDX, IDC_SEPARATORHEIGHT, m_heightSeparator);
	DDX_Text(pDX, IDC_WIDTHDOC, m_WidthDoc);
	DDX_Text(pDX, IDC_EDIT1, m_spkheight);
	DDX_Text(pDX, IDC_EDIT2, m_spkwidth);
	DDX_Check(pDX, IDC_CHECKFILTERDATA, m_bFilterDat);
}

BEGIN_MESSAGE_MAP(CPrintDrawAreaDlg, CDialog)

END_MESSAGE_MAP()

BOOL CPrintDrawAreaDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_HeightDoc = mdPM->HeightDoc;
	m_WidthDoc = mdPM->WidthDoc;
	m_spkheight = mdPM->spkheight;
	m_spkwidth = mdPM->spkwidth;
	m_heightSeparator = mdPM->heightSeparator;
	m_bFilterDat = mdPM->bFilterDataSource;
	UpdateData(FALSE);

	return TRUE; // return TRUE  unless you set the focus to a control
}

void CPrintDrawAreaDlg::OnOK()
{
	UpdateData(TRUE);

	mdPM->HeightDoc = m_HeightDoc;
	mdPM->WidthDoc = m_WidthDoc;
	mdPM->heightSeparator = m_heightSeparator;
	mdPM->spkheight = m_spkheight;
	mdPM->spkwidth = m_spkwidth;
	mdPM->bFilterDataSource = m_bFilterDat;
	CDialog::OnOK();
}
