#include "stdafx.h"
#include "DlgPrintDataComments.h"

DlgPrintDataComments::DlgPrintDataComments(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
	m_b_acq_comment = FALSE;
	m_b_acq_date_time = FALSE;
	m_b_channels_comment = FALSE;
	m_b_channel_settings = FALSE;
	m_b_doc_name = FALSE;
	m_font_size = 0;
	m_text_separator = 0;
	options_view_data = nullptr;
}

void DlgPrintDataComments::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ACQCOMMENT, m_b_acq_comment);
	DDX_Check(pDX, IDC_ACQDATETIME, m_b_acq_date_time);
	DDX_Check(pDX, IDC_CHANSCOMMENT, m_b_channels_comment);
	DDX_Check(pDX, IDC_CHANSETTINGS, m_b_channel_settings);
	DDX_Check(pDX, IDC_DOCNAME, m_b_doc_name);
	DDX_Text(pDX, IDC_FONTSIZE, m_font_size);
	DDX_Text(pDX, IDC_TEXTSEPARATOR, m_text_separator);
}

BEGIN_MESSAGE_MAP(DlgPrintDataComments, CDialog)

END_MESSAGE_MAP()

void DlgPrintDataComments::OnOK()
{
	UpdateData(TRUE);

	options_view_data->b_acq_comment = m_b_acq_comment;
	options_view_data->b_acq_date_time = m_b_acq_date_time;
	options_view_data->b_channel_comment = m_b_channels_comment;
	options_view_data->b_channel_settings = m_b_channel_settings;
	options_view_data->b_doc_name = m_b_doc_name;
	options_view_data->text_separator = m_text_separator;
	options_view_data->font_size = m_font_size;

	CDialog::OnOK();
}

BOOL DlgPrintDataComments::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_b_acq_comment = options_view_data->b_acq_comment;
	m_b_acq_date_time = options_view_data->b_acq_date_time;
	m_b_channels_comment = options_view_data->b_channel_comment;
	m_b_channel_settings = options_view_data->b_channel_settings;
	m_b_doc_name = options_view_data->b_doc_name;
	m_font_size = options_view_data->font_size;
	m_text_separator = options_view_data->text_separator;

	UpdateData(FALSE);
	return TRUE; // return TRUE  unless you set the focus to a control
}
