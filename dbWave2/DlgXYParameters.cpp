#include "StdAfx.h"
#include "resource.h"
#include "DlgXYParameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgXYParameters::DlgXYParameters(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgXYParameters::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_XEXTENT, m_x_extent);
	DDX_Text(p_dx, IDC_XZERO, m_x_zero);
	DDX_Text(p_dx, IDC_YEXTENT, m_y_extent);
	DDX_Text(p_dx, IDC_YZERO, m_y_zero);
	DDX_Check(p_dx, IDC_DISPLAY_SPIKES, b_display_spikes);
	DDX_Check(p_dx, IDC_CHART_BARS, m_b_display_bars);
	DDX_Check(p_dx, IDC_DISPLAYSOURCE, m_b_display_source);
}

BEGIN_MESSAGE_MAP(DlgXYParameters, CDialog)
	ON_EN_CHANGE(IDC_XEXTENT, on_en_change_x_extent)
	ON_EN_CHANGE(IDC_XZERO, on_en_change_x_zero)
	ON_EN_CHANGE(IDC_YEXTENT, on_en_change_y_extent)
	ON_EN_CHANGE(IDC_YZERO, on_en_change_y_zero)
END_MESSAGE_MAP()

BOOL DlgXYParameters::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (!m_x_param) // x parameters are not valid
	{
		GetDlgItem(IDC_XZERO)->EnableWindow(FALSE);
		GetDlgItem(IDC_XEXTENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_YZERO)->SetFocus();
	}
	if (!m_y_param)
	{
		GetDlgItem(IDC_YZERO)->EnableWindow(FALSE);
		GetDlgItem(IDC_YEXTENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_XZERO)->SetFocus();
	}
	// subclass edit controls
	VERIFY(mm_x_extent.SubclassDlgItem(IDC_XEXTENT, this));
	VERIFY(mm_x_zero.SubclassDlgItem(IDC_XZERO, this));
	VERIFY(mm_y_extent.SubclassDlgItem(IDC_YEXTENT, this));
	VERIFY(mm_y_zero.SubclassDlgItem(IDC_YZERO, this));

	return FALSE;
	//return TRUE;  // return TRUE  unless you set the focus to a control
}

void DlgXYParameters::on_en_change_x_extent()
{
	// TODO: Add your control notification handler code here
}

void DlgXYParameters::on_en_change_x_zero()
{
	// TODO: Add your control notification handler code here
}

void DlgXYParameters::on_en_change_y_extent()
{
	// TODO: Add your control notification handler code here
}

void DlgXYParameters::on_en_change_y_zero()
{
	// TODO: Add your control notification handler code here
}
