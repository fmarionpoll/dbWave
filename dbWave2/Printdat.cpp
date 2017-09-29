// printdat.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "printdat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrintDataOptionsDlg dialog


CPrintDataOptionsDlg::CPrintDataOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrintDataOptionsDlg::IDD, pParent)
{
	m_bClipRect = FALSE;
	m_bFrameRect = FALSE;
	m_bTimeScaleBar = FALSE;
	m_bVoltageScaleBar = FALSE;
}

void CPrintDataOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CLIPRECT, m_bClipRect);
	DDX_Check(pDX, IDC_FRAMERECT, m_bFrameRect);
	DDX_Check(pDX, IDC_TIMESCALEBAR, m_bTimeScaleBar);
	DDX_Check(pDX, IDC_VOLTAGESCALEBAR, m_bVoltageScaleBar);
	DDX_Check(pDX, IDC_CHECK1, m_bPrintSelection);
	DDX_Check(pDX, IDC_CHECK2, m_bPrintSpkBars);
}

BEGIN_MESSAGE_MAP(CPrintDataOptionsDlg, CDialog)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrintDataOptionsDlg message handlers
/////////////////////////////////////////////////////////////////////////////
// CPrintDataCommentsDlg dialog


CPrintDataCommentsDlg::CPrintDataCommentsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrintDataCommentsDlg::IDD, pParent)
{
	m_bacqcomment = FALSE;
	m_bacqdatetime = FALSE;
	m_bchanscomment = FALSE;
	m_bchansettings = FALSE;
	m_bdocname = FALSE;
	m_fontsize = 0;
	m_textseparator = 0;
}

void CPrintDataCommentsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ACQCOMMENT, m_bacqcomment);
	DDX_Check(pDX, IDC_ACQDATETIME, m_bacqdatetime);
	DDX_Check(pDX, IDC_CHANSCOMMENT, m_bchanscomment);
	DDX_Check(pDX, IDC_CHANSETTINGS, m_bchansettings);
	DDX_Check(pDX, IDC_DOCNAME, m_bdocname);
	DDX_Text(pDX, IDC_FONTSIZE, m_fontsize);
	DDX_Text(pDX, IDC_TEXTSEPARATOR, m_textseparator);
}

BEGIN_MESSAGE_MAP(CPrintDataCommentsDlg, CDialog)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrintDataCommentsDlg message handlers

void CPrintDataCommentsDlg::OnOK()
{
	UpdateData(TRUE);

	mdPM->bAcqComment = m_bacqcomment;
	mdPM->bAcqDateTime = m_bacqdatetime;
	mdPM->bChansComment = m_bchanscomment;
	mdPM->bChanSettings = m_bchansettings;
	mdPM->bDocName = m_bdocname;
	mdPM->textseparator=m_textseparator;
	mdPM->fontsize=m_fontsize;

	CDialog::OnOK();
}

BOOL CPrintDataCommentsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bacqcomment = mdPM->bAcqComment;
	m_bacqdatetime = mdPM->bAcqDateTime;
	m_bchanscomment = mdPM->bChansComment;
	m_bchansettings = mdPM->bChanSettings;
	m_bdocname = mdPM->bDocName;
	m_fontsize=mdPM->fontsize;
	m_textseparator = mdPM->textseparator;
	
	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CPrintDataOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bFrameRect = mdPM->bFrameRect;
	m_bClipRect = mdPM->bClipRect;
	m_bTimeScaleBar = mdPM->bTimeScaleBar;
	m_bVoltageScaleBar = mdPM->bVoltageScaleBar;
	m_bPrintSelection = mdPM->bPrintSelection;
	m_bPrintSpkBars = mdPM->bPrintSpkBars;

	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPrintDataOptionsDlg::OnOK()
{
	UpdateData(TRUE);

	mdPM->bFrameRect = m_bFrameRect;
	mdPM->bClipRect = m_bClipRect;
	mdPM->bTimeScaleBar = m_bTimeScaleBar;
	mdPM->bVoltageScaleBar = m_bVoltageScaleBar;
	mdPM->bPrintSelection = m_bPrintSelection;
	mdPM->bPrintSpkBars = m_bPrintSpkBars;

	CDialog::OnOK();
}
