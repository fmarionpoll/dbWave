
#include "StdAfx.h"
#include "DlgBrowseFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgBrowseFile::DlgBrowseFile(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgBrowseFile::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Check(p_dx, IDC_ALLCHANNELS, m_all_channels);
	DDX_Check(p_dx, IDC_CENTERCURVES, m_center_curves);
	DDX_Check(p_dx, IDC_COMPLETEFILE, m_complete_file);
	DDX_Check(p_dx, IDC_MAXIMIZEGAIN, m_maximize_gain);
	DDX_Check(p_dx, IDC_SPLITCURVES, m_split_curves);
	DDX_Check(p_dx, IDC_MULTILINEDISPLAY, m_multi_row_display);
	DDX_Check(p_dx, IDC_KEEPFOREACHFILE, m_keep_for_each_file);
}

BEGIN_MESSAGE_MAP(DlgBrowseFile, CDialog)

END_MESSAGE_MAP()

BOOL DlgBrowseFile::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (p_options_view_data != nullptr)
	{
		m_all_channels = p_options_view_data->b_all_channels;
		m_center_curves = p_options_view_data->b_center_curves;
		m_complete_file = p_options_view_data->b_complete_record;
		m_split_curves = p_options_view_data->b_split_curves;
		m_maximize_gain = p_options_view_data->b_maximize_gain;
		m_multi_row_display = p_options_view_data->b_multiple_rows;
		m_keep_for_each_file = p_options_view_data->b_keep_for_each_file;
	}
	UpdateData(FALSE);
	return TRUE; 
}

void DlgBrowseFile::OnOK()
{
	UpdateData(TRUE);
	p_options_view_data->b_all_channels = m_all_channels;
	p_options_view_data->b_center_curves = m_center_curves;
	p_options_view_data->b_complete_record = m_complete_file;
	p_options_view_data->b_split_curves = m_split_curves;
	p_options_view_data->b_maximize_gain = m_maximize_gain;
	p_options_view_data->b_multiple_rows = m_multi_row_display;
	p_options_view_data->b_keep_for_each_file = m_keep_for_each_file;
	p_options_view_data->b_changed = TRUE;
	CDialog::OnOK();
}
