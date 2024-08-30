#pragma once


class DlgFormatHistogram : public CDialog
{
	// Construction
public:
	DlgFormatHistogram(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_HIISTOGRAMPARAMETERS };

	float m_y_max{ 0.f };
	float m_x_first{ 0.f };
	float m_x_last{ 0.f };
	BOOL m_b_y_max_auto{ false };
	COLORREF m_cr_hist_fill{ 0 };
	COLORREF m_cr_hist_border{ 0 };
	COLORREF m_cr_stimulus_fill{ 0 };
	COLORREF m_cr_stimulus_border{ 0 };
	COLORREF m_cr_chart_area{ 0 };
	// Overrides
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	// Generated message map functions
	afx_msg void on_check_max_auto();
	afx_msg void on_button_hist_fill();
	afx_msg void OnPaint();
	afx_msg void on_hist_border_color();
	afx_msg void on_stimulus_fill_color();
	afx_msg void on_stimulus_border_color();
	afx_msg void on_background_color();

	DECLARE_MESSAGE_MAP()
};
