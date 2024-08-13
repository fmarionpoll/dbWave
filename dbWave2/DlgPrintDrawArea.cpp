// printdra.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "./DlgPrintDrawArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgPrintDrawArea::DlgPrintDrawArea(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{

}

void DlgPrintDrawArea::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HEIGHTDOC, m_HeightDoc);
	DDX_Text(pDX, IDC_SEPARATORHEIGHT, m_heightSeparator);
	DDX_Text(pDX, IDC_WIDTHDOC, m_WidthDoc);
	DDX_Text(pDX, IDC_EDIT1, m_spkheight);
	DDX_Text(pDX, IDC_EDIT2, m_spkwidth);
	DDX_Check(pDX, IDC_CHECKFILTERDATA, m_bFilterDat);
}

BEGIN_MESSAGE_MAP(DlgPrintDrawArea, CDialog)

END_MESSAGE_MAP()

BOOL DlgPrintDrawArea::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_HeightDoc = mdPM->height_doc;
	m_WidthDoc = mdPM->width_doc;
	m_spkheight = mdPM->spike_height;
	m_spkwidth = mdPM->spike_width;
	m_heightSeparator = mdPM->height_separator;
	m_bFilterDat = mdPM->b_filter_data_source;
	UpdateData(FALSE);

	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgPrintDrawArea::OnOK()
{
	UpdateData(TRUE);

	mdPM->height_doc = m_HeightDoc;
	mdPM->width_doc = m_WidthDoc;
	mdPM->height_separator = m_heightSeparator;
	mdPM->spike_height = m_spkheight;
	mdPM->spike_width = m_spkwidth;
	mdPM->b_filter_data_source = m_bFilterDat;
	CDialog::OnOK();
}
