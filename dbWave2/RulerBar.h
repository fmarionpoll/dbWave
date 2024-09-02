#pragma once

class Ruler;
class ChartData;


class RulerBar : public CWnd
{
	DECLARE_DYNAMIC(RulerBar)
public:
	RulerBar();
	~RulerBar() override;
protected:
	void PreSubclassWindow() override;
private:
	COLORREF m_pen_color_ { GetSysColor(COLOR_WINDOWTEXT) };
	CFont h_font_;
	BOOL b_horizontal_ {-1};
	CRect rc_client_;
	BOOL b_captured_ {false};
	BOOL b_bottom_ {true};
	int capture_mode_ {-1};
	CPoint old_pt_;
	CPoint new_pt_;
	Ruler* p_ruler_ { nullptr };
	ChartData* p_chart_data_wnd_ { nullptr };
	
public:
	BOOL Create(LPCTSTR lpsz_class_name, LPCTSTR lpsz_window_name,
	            DWORD dw_style, const RECT& rect, CWnd* p_parent_wnd,
	            UINT n_id, CCreateContext* p_context = nullptr) override;
	BOOL Create(CWnd* p_parent_wnd, ChartData* p_data_chart_wnd, BOOL b_as_x_axis, int d_size, UINT n_id);

	void draw_scale_from_ruler(const Ruler* p_ruler);
	void attach_scope_wnd(ChartData* p_data_chart_wnd, const BOOL b_x_axis)
	{
		p_chart_data_wnd_ = p_data_chart_wnd;
		b_horizontal_ = b_x_axis;
	}

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* p_dc);
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void OnMouseMove(UINT n_flags, CPoint point);
	afx_msg void OnLButtonDown(UINT n_flags, CPoint point);
	afx_msg void OnLButtonUp(UINT n_flags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
