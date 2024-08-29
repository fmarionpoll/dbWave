#pragma once
#include "Editctrl.h"

class DlgChartProps : public CDialog
{
	// Construction
public:
	DlgChartProps(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_SCOPEPROPERTIES };

	int m_xy_ticks;
	int m_xy_tick_line;
	int m_x_cells;
	int m_y_cells;

	ChartWnd* m_p_scope;
	COLORREF m_cr_scope_fill;
	COLORREF m_cr_scope_grid;

	CEditCtrl mm_x_cells;
	CEditCtrl mm_y_cells;
	CEditCtrl mm_xy_ticks;
	CEditCtrl mm_xy_tick_line;

	// Overrides

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void on_en_change_x_cells();
	afx_msg void on_en_change_xy_ticks();
	afx_msg void on_en_change_xy_ticks_line();
	afx_msg void on_en_change_y_cells();
	afx_msg void on_background_color();
	afx_msg void on_grid_color();

	DECLARE_MESSAGE_MAP()
};
