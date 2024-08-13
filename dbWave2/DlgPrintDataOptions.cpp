
#include "StdAfx.h"
#include "resource.h"
#include "DlgPrintDataOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgPrintDataOptions::DlgPrintDataOptions(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgPrintDataOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CLIPRECT, m_bClipRect);
	DDX_Check(pDX, IDC_FRAMERECT, m_bFrameRect);
	DDX_Check(pDX, IDC_TIMESCALEBAR, m_bTimeScaleBar);
	DDX_Check(pDX, IDC_VOLTAGESCALEBAR, m_bVoltageScaleBar);
	DDX_Check(pDX, IDC_CHECK1, m_bPrintSelection);
	DDX_Check(pDX, IDC_CHECK2, m_bPrintSpkBars);
}

BEGIN_MESSAGE_MAP(DlgPrintDataOptions, CDialog)

END_MESSAGE_MAP()

CPrintDataCommentsDlg::CPrintDataCommentsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	m_bacqcomment = FALSE;
	m_bacqdatetime = FALSE;
	m_bchanscomment = FALSE;
	m_bchansettings = FALSE;
	m_bdocname = FALSE;
	m_fontsize = 0;
	m_textseparator = 0;
	mdPM = nullptr;
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

void CPrintDataCommentsDlg::OnOK()
{
	UpdateData(TRUE);

	mdPM->b_acq_comment = m_bacqcomment;
	mdPM->b_acq_date_time = m_bacqdatetime;
	mdPM->b_channel_comment = m_bchanscomment;
	mdPM->b_channel_settings = m_bchansettings;
	mdPM->b_doc_name = m_bdocname;
	mdPM->text_separator = m_textseparator;
	mdPM->font_size = m_fontsize;

	CDialog::OnOK();
}

BOOL CPrintDataCommentsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bacqcomment = mdPM->b_acq_comment;
	m_bacqdatetime = mdPM->b_acq_date_time;
	m_bchanscomment = mdPM->b_channel_comment;
	m_bchansettings = mdPM->b_channel_settings;
	m_bdocname = mdPM->b_doc_name;
	m_fontsize = mdPM->font_size;
	m_textseparator = mdPM->text_separator;

	UpdateData(FALSE);
	return TRUE; // return TRUE  unless you set the focus to a control
}

BOOL DlgPrintDataOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bFrameRect = mdPM->b_frame_rect;
	m_bClipRect = mdPM->b_clip_rect;
	m_bTimeScaleBar = mdPM->b_time_scale_bar;
	m_bVoltageScaleBar = mdPM->b_voltage_scale_bar;
	m_bPrintSelection = mdPM->b_print_selection;
	m_bPrintSpkBars = mdPM->b_print_spk_bars;

	UpdateData(FALSE);
	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgPrintDataOptions::OnOK()
{
	UpdateData(TRUE);

	mdPM->b_frame_rect = m_bFrameRect;
	mdPM->b_clip_rect = m_bClipRect;
	mdPM->b_time_scale_bar = m_bTimeScaleBar;
	mdPM->b_voltage_scale_bar = m_bVoltageScaleBar;
	mdPM->b_print_selection = m_bPrintSelection;
	mdPM->b_print_spk_bars = m_bPrintSpkBars;

	CDialog::OnOK();
}
