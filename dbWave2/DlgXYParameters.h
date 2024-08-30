#pragma once
#include "Editctrl.h"


class DlgXYParameters : public CDialog
{
	// Construction
public:
	DlgXYParameters(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_XYPARAMETERS };

	int m_x_extent{ 0 };
	int m_x_zero{ 0 };
	int m_y_extent{ 0 };
	int m_y_zero{ 0 };
	BOOL b_display_spikes{ false };
	BOOL m_b_display_bars{ false };
	BOOL m_b_display_source{ false };
	BOOL m_x_param{ true };
	BOOL m_y_param{ true };

	CEditCtrl mm_x_extent;
	CEditCtrl mm_x_zero;
	CEditCtrl mm_y_extent;
	CEditCtrl mm_y_zero;

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	BOOL OnInitDialog() override;
	afx_msg void on_en_change_x_extent();
	afx_msg void on_en_change_x_zero();
	afx_msg void on_en_change_y_extent();
	afx_msg void on_en_change_y_zero();

	DECLARE_MESSAGE_MAP()
};
