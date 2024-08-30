
#include "StdAfx.h"
#include "DlgDataComments.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgDataComments::DlgDataComments(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgDataComments::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ACQCHANS, b_acq_channels);
	DDX_Check(pDX, IDC_ACQCOMMENTS, b_acq_comments);
	DDX_Check(pDX, IDC_ACQDATE, b_acq_date);
	DDX_Check(pDX, IDC_ACQTIME, b_acq_time);
	DDX_Check(pDX, IDC_FILESIZE, b_file_size);
	DDX_Check(pDX, IDC_ACQCHSETTING, b_acq_channel_setting);
	DDX_Check(pDX, IDC_CHECK1, b_to_excel);
	DDX_Check(pDX, IDC_DATABASECOLS, b_data_base_cols);
}

BEGIN_MESSAGE_MAP(DlgDataComments, CDialog)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgDataComments message handlers

BOOL DlgDataComments::OnInitDialog()
{
	CDialog::OnInitDialog();

	b_acq_channels = p_options_view_data->b_acq_channel_comment; // copy parms into
	b_acq_channel_setting = p_options_view_data->b_acq_channel_setting; // dlg parms
	b_acq_comments = p_options_view_data->b_acq_comments;
	b_acq_date = p_options_view_data->b_acq_date;
	b_acq_time = p_options_view_data->b_acq_time;
	b_file_size = p_options_view_data->b_file_size;
	b_to_excel = p_options_view_data->b_to_excel;
	b_data_base_cols = p_options_view_data->b_data_base_columns;

	UpdateData(FALSE);

	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgDataComments::OnOK()
{
	UpdateData(TRUE);
	if (p_options_view_data->b_acq_channel_comment != b_acq_channels
		|| p_options_view_data->b_acq_channel_setting != b_acq_channel_setting
		|| p_options_view_data->b_acq_comments != b_acq_comments
		|| p_options_view_data->b_acq_date != b_acq_date
		|| p_options_view_data->b_acq_time != b_acq_time
		|| p_options_view_data->b_file_size != b_file_size
		|| p_options_view_data->b_to_excel != b_to_excel
		|| p_options_view_data->b_data_base_columns != b_data_base_cols)
	{
		p_options_view_data->b_acq_channel_comment = b_acq_channels;
		p_options_view_data->b_acq_channel_setting = b_acq_channel_setting;
		p_options_view_data->b_acq_comments = b_acq_comments;
		p_options_view_data->b_acq_date = b_acq_date;
		p_options_view_data->b_acq_time = b_acq_time;
		p_options_view_data->b_file_size = b_file_size;
		p_options_view_data->b_to_excel = b_to_excel;
		p_options_view_data->b_data_base_columns = b_data_base_cols;
		p_options_view_data->b_changed = TRUE; // save new params in app array
	}
	CDialog::OnOK();
}
