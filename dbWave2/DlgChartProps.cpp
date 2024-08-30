// ScopeScreenPropsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Editctrl.h"
#include "resource.h"
#include "ChartWnd.h"
//#include "Editctrl.h"
#include "DlgChartProps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgChartProps::DlgChartProps(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent), m_p_scope(nullptr), m_cr_scope_fill(0)
{
	m_xy_ticks = 0;
	m_xy_tick_line = 0;
	m_x_cells = 0;
	m_y_cells = 0;
	m_cr_scope_grid = 0;
}

void DlgChartProps::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_EDIT4, m_xy_ticks);
	DDX_Text(p_dx, IDC_EDIT5, m_xy_tick_line);
	DDX_Text(p_dx, IDC_EDIT3, m_x_cells);
	DDX_Text(p_dx, IDC_EDIT6, m_y_cells);
}

BEGIN_MESSAGE_MAP(DlgChartProps, CDialog)
	ON_EN_CHANGE(IDC_EDIT3, on_en_change_x_cells)
	ON_EN_CHANGE(IDC_EDIT4, on_en_change_xy_ticks)
	ON_EN_CHANGE(IDC_EDIT5, on_en_change_xy_ticks_line)
	ON_EN_CHANGE(IDC_EDIT6, on_en_change_y_cells)
	ON_BN_CLICKED(IDC_BUTTON1, on_background_color)
	ON_BN_CLICKED(IDC_BUTTON2, on_grid_color)
END_MESSAGE_MAP()

BOOL DlgChartProps::OnInitDialog()
{
	CDialog::OnInitDialog();
	options_scope_struct* pStruct = m_p_scope->get_scope_parameters();
	m_x_cells = pStruct->i_x_cells;
	m_y_cells = pStruct->i_y_cells;
	m_xy_ticks = pStruct->i_x_ticks;
	m_xy_tick_line = pStruct->i_x_tick_line;

	VERIFY(mm_x_cells.SubclassDlgItem(IDC_EDIT3, this));
	VERIFY(mm_y_cells.SubclassDlgItem(IDC_EDIT6, this));
	VERIFY(mm_xy_ticks.SubclassDlgItem(IDC_EDIT4, this));
	VERIFY(mm_xy_tick_line.SubclassDlgItem(IDC_EDIT5, this));

	mm_x_cells.ShowScrollBar(SB_VERT);
	mm_y_cells.ShowScrollBar(SB_VERT);
	mm_xy_ticks.ShowScrollBar(SB_VERT);
	mm_xy_tick_line.ShowScrollBar(SB_VERT);

	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgChartProps::on_en_change_x_cells()
{
	if (mm_x_cells.m_b_entry_done)
	{
		auto x_cells = m_x_cells;
		mm_x_cells.on_en_change(this, m_x_cells, 1, -1);

		// check boundaries
		if (m_x_cells < 1) m_x_cells = 1;

		if (m_x_cells != x_cells)
		{
			options_scope_struct* pStruct = m_p_scope->get_scope_parameters();
			pStruct->x_scale_unit_value = pStruct->x_scale_unit_value * x_cells / m_x_cells;
			m_p_scope->set_n_x_scale_cells(m_x_cells, m_xy_ticks, m_xy_tick_line);
			m_p_scope->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void DlgChartProps::on_en_change_y_cells()
{
	if (mm_y_cells.m_b_entry_done)
	{
		auto y_cells = m_y_cells;
		mm_y_cells.on_en_change(this, m_y_cells, 1, -1);

		if (m_y_cells < 1) m_y_cells = 1;
		if (m_y_cells != y_cells)
		{
			options_scope_struct* pStruct = m_p_scope->get_scope_parameters();
			pStruct->y_scale_unit_value = pStruct->y_scale_unit_value * y_cells / m_y_cells;
			m_p_scope->set_ny_scale_cells(m_y_cells, m_xy_ticks, m_xy_tick_line);
			m_p_scope->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void DlgChartProps::on_en_change_xy_ticks()
{
	if (mm_xy_ticks.m_b_entry_done)
	{
		const auto xy_ticks = m_xy_ticks;
		mm_xy_ticks.on_en_change(this, m_xy_ticks, 1, -1);

		// check boundaries
		if (m_xy_ticks < 0) m_xy_ticks = 0;
		if (m_xy_ticks != xy_ticks)
		{
			if (m_x_cells > 0)
				m_p_scope->set_n_x_scale_cells(m_x_cells, m_xy_ticks, m_xy_tick_line);
			if (m_y_cells > 0)
				m_p_scope->set_ny_scale_cells(m_y_cells, m_xy_ticks, m_xy_tick_line);
			m_p_scope->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void DlgChartProps::on_en_change_xy_ticks_line()
{
	if (mm_xy_tick_line.m_b_entry_done)
	{
		const auto xy_tick_line = m_xy_tick_line;
		mm_xy_tick_line.on_en_change(this, m_xy_tick_line, 1, -1);

		if (m_xy_tick_line < 0) m_xy_tick_line = 0;
		if (m_xy_tick_line != xy_tick_line)
		{
			if (m_x_cells > 0)
				m_p_scope->set_n_x_scale_cells(m_x_cells, m_xy_ticks, m_xy_tick_line);
			if (m_y_cells > 0)
				m_p_scope->set_ny_scale_cells(m_y_cells, m_xy_ticks, m_xy_tick_line);
			m_p_scope->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void DlgChartProps::on_background_color() 
{
	CColorDialog dlg(m_cr_scope_fill, CC_RGBINIT, nullptr);
	if (IDOK != dlg.DoModal())
		return;
	(m_p_scope->get_scope_parameters())->cr_scope_fill = dlg.GetColor();
	m_p_scope->Invalidate();
}

void DlgChartProps::on_grid_color()
{
	CColorDialog dlg(m_cr_scope_grid, CC_RGBINIT, nullptr);
	if (IDOK != dlg.DoModal())
		return;
	(m_p_scope->get_scope_parameters())->cr_scope_grid = dlg.GetColor();
	m_p_scope->Invalidate();
}
