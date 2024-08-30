#pragma once
#include "options_view_data.h"


class DlgPrintMargins : public CDialog
{
	// Construction
public:
	DlgPrintMargins(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_PRINTMARGINS };

	options_view_data* options_view_data{ nullptr };
	int m_view_type{ 0 };

	// Implementation
protected:
	void sketch_printer_page();
	void get_page_size();

	CRect m_page_rect_;
	CRect m_bars_[8];

	CRect m_rect_;
	BOOL m_b_captured_{ false };
	int m_capture_mode_{ 0 };
	int m_i_captured_bar_{ 0 };

	static void draw_bar(const CRect* bar, CDC* pdc);
	int is_mouse_over_any_bar(const CPoint* point);

	void DoDataExchange(CDataExchange* p_dx) override;
	// Generated message map functions
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point);
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point);
	afx_msg void OnMouseMove(UINT n_flags, CPoint point);

	afx_msg void on_comments_options();
	afx_msg void on_draw_area();
	afx_msg void on_draw_options();
	afx_msg void on_page_margins();
	afx_msg void on_printer_setup();
	

	DECLARE_MESSAGE_MAP()
};
