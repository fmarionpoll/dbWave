#include "StdAfx.h"
#include "ChartData.h"
#include "Ruler.h"
#include "RulerBar.h"
#include <cmath>
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(RulerBar, CWnd)

RulerBar::RulerBar()
{
	h_font_.CreateFont(12, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY,
	                   VARIABLE_PITCH | FF_ROMAN, _T("Arial"));
}

RulerBar::~RulerBar()
= default;

BEGIN_MESSAGE_MAP(RulerBar, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// RulerBar message handlers

void RulerBar::OnPaint()
{
	draw_scale_from_ruler(nullptr);
}

void RulerBar::draw_scale_from_ruler(const Ruler* p_ruler)
{
	CPaintDC dc(this);

	// exit if the length is not properly defined
	if (p_ruler == nullptr)
	{
		if (p_ruler_ != nullptr)
			p_ruler = p_ruler_;
		else if (p_chart_data_wnd_ != nullptr)
		{
			if (rc_client_.Height() > rc_client_.Width())
				p_ruler_ = &p_chart_data_wnd_->y_ruler;
			else
				p_ruler_ = &p_chart_data_wnd_->x_ruler;
			p_ruler = p_ruler_;
		}
		else
			return;
	}

	if (!(p_ruler->m_highest_value > p_ruler->m_lowest_value) && !(p_ruler->m_highest_value < p_ruler->m_lowest_value))
		return;
	

	CPen a_pen;
	a_pen.CreatePen(PS_SOLID, 0, m_pen_color_);
	const auto p_old_pen = dc.SelectObject(&a_pen);
	const auto p_old_font = dc.SelectObject(&h_font_);
	CString str;

	// draw ticks and legends
	int tick_big_height;
	if (!b_horizontal_)
		tick_big_height = rc_client_.Width() / 2 - 2;
	else
		tick_big_height = rc_client_.Height() / 2 - 2;
	const int tick_small_height = tick_big_height / 2;

	// draw solid background
	dc.IntersectClipRect(rc_client_);
	dc.FillSolidRect(rc_client_, GetSysColor(COLOR_3DFACE));

	// draw baseline on the right side
	if (!b_horizontal_)
	{
		dc.MoveTo(rc_client_.right - 1, rc_client_.top);
		dc.LineTo(rc_client_.right - 1, rc_client_.bottom);
	}
	else
	{
		dc.MoveTo(rc_client_.left, rc_client_.top);
		dc.LineTo(rc_client_.right, rc_client_.top);
	}

	// draw scale
	auto d_pos = floor(p_ruler->m_first_major_scale);
	const auto d_len = p_ruler->m_highest_value - p_ruler->m_lowest_value;
	const auto small_scale_inc = p_ruler->m_length_major_scale / 5.;
	dc.SetBkMode(TRANSPARENT);

	while (d_pos <= p_ruler->m_highest_value)
	{
		// display small ticks
		auto d_small_pos = d_pos;
		int tick_pos;
		for (auto i = 0; i < 4; i++)
		{
			d_small_pos += small_scale_inc;
			auto ratio = (p_ruler->m_highest_value - d_small_pos) / d_len;
			if (!b_horizontal_)
			{
				tick_pos = static_cast<int>(rc_client_.Height() * (p_ruler->m_highest_value - d_small_pos) / d_len);
				dc.MoveTo(rc_client_.right, tick_pos);
				dc.LineTo(rc_client_.right - tick_small_height, tick_pos);
			}
			else
			{
				tick_pos = static_cast<int>(rc_client_.Width() * (d_small_pos - p_ruler->m_lowest_value) / d_len);
				dc.MoveTo(tick_pos, rc_client_.top);
				dc.LineTo(tick_pos, rc_client_.top + tick_small_height);
			}
		}

		// display large ticks and text
		if (!b_horizontal_)
			tick_pos = static_cast<int>(rc_client_.Height() * (p_ruler->m_highest_value - d_pos) / d_len);
		else
			tick_pos = static_cast<int>(rc_client_.Width() * (d_pos - p_ruler->m_lowest_value) / d_len);

		if (tick_pos >= 0)
		{
			str.Format(_T("%g"), d_pos);
			auto size = dc.GetTextExtent(str);
			int x, y;
			if (!b_horizontal_)
			{
				// tick
				dc.MoveTo(rc_client_.right, tick_pos);
				dc.LineTo(rc_client_.right - tick_big_height, tick_pos);
				// text
				if (d_pos != 0. && fabs(d_pos) < 1E-10) // prevents "bad" zero 
				{
					d_pos = 0;
					str.Format(_T("%g"), d_pos);
					size = dc.GetTextExtent(str);
				}
				x = rc_client_.right - tick_big_height - size.cx - 2;
				y = tick_pos - (size.cy / 2);
			}
			else
			{
				dc.MoveTo(tick_pos, rc_client_.top);
				dc.LineTo(tick_pos, rc_client_.top + tick_big_height);
				x = tick_pos - (size.cx / 2);
				if (x < 0)
					x = 0;
				if (x + size.cx > rc_client_.right)
					x = rc_client_.right - size.cx;
				y = rc_client_.top + tick_big_height + 1;
			}
			dc.TextOut(x, y, str);
		}
		d_pos += p_ruler->m_length_major_scale;
	}

	// restore objects used in this routine
	dc.SelectObject(p_old_pen);
	dc.SelectObject(p_old_font);
}

BOOL RulerBar::OnEraseBkgnd(CDC* p_dc)
{
	return TRUE;
	//return CWnd::OnEraseBkgnd(p_dc);
}

void RulerBar::OnSize(UINT n_type, int cx, int cy)
{
	CWnd::OnSize(n_type, cx, cy);
	GetClientRect(&rc_client_);
}

void RulerBar::PreSubclassWindow()
{
	if (IsWindow(m_hWnd))
	{
		GetClientRect(&rc_client_);
		if (rc_client_.Width() < rc_client_.Height())
			b_horizontal_ = FALSE;
	}
	CWnd::PreSubclassWindow();
}

BOOL RulerBar::Create(LPCTSTR lpsz_class_name, LPCTSTR lpsz_window_name, const DWORD dw_style, const RECT& rect,
                       CWnd* p_parent_wnd, const UINT n_id, CCreateContext* p_context)
{
	if ((rect.right - rect.left) < (rect.bottom - rect.top))
		b_horizontal_ = FALSE;
	return CWnd::Create(nullptr, _T("RulerBarWnd"), dw_style, rect, p_parent_wnd, n_id);
}

BOOL RulerBar::Create(CWnd* p_parent_wnd, ChartData* p_data_chart_wnd, const BOOL b_as_x_axis, const int d_size, const UINT n_id)
{
	p_chart_data_wnd_ = p_data_chart_wnd;
	CRect rect;
	p_data_chart_wnd->GetWindowRect(&rect);
	auto rect_this = rect;
	if (b_as_x_axis)
	{
		rect_this.top = rect.bottom - 1;
		rect_this.bottom = rect_this.top + d_size;
		b_horizontal_ = TRUE;
	}
	else
	{
		rect_this.right = rect.left - 1;
		rect_this.left = rect_this.right - d_size;
		b_horizontal_ = FALSE;
	}
	return CWnd::Create(_T("RulerBarWnd"), nullptr, WS_CHILD, rect_this, p_parent_wnd, n_id);
}

#define MODE_ZOOM	0
#define MODE_BIAS	1
#define DELTA		10

void RulerBar::OnMouseMove(const UINT n_flags, const CPoint point)
{
	if (!b_horizontal_ && (point.y < DELTA || point.y > rc_client_.Height() - DELTA))
		SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITVERTICAL));
	else if (b_horizontal_ && (point.x < DELTA || point.x > rc_client_.Width() - DELTA))
		SetCursor(AfxGetApp()->LoadCursor(IDC_SPLITHORIZONTAL));

	CWnd::OnMouseMove(n_flags, point);
}

void RulerBar::OnLButtonDown(const UINT n_flags, const CPoint point)
{
	if (!b_horizontal_ && (point.y < DELTA || point.y > rc_client_.Height() - DELTA))
	{
		capture_mode_ = MODE_ZOOM;
		SetCapture();
		b_captured_ = TRUE;
		old_pt_ = point;
		b_bottom_ = (point.y < DELTA);
	}
	else if (b_horizontal_ && (point.x < DELTA || point.x > rc_client_.Width() - DELTA))
	{
		capture_mode_ = MODE_ZOOM;
		SetCapture();
		b_captured_ = TRUE;
		old_pt_ = point;
		b_bottom_ = (point.x < DELTA);
	}
	else
		CWnd::OnLButtonDown(n_flags, point);
}

void RulerBar::OnLButtonUp(const UINT n_flags, const CPoint point)
{
	if (b_captured_)
	{
		b_captured_ = FALSE;
		capture_mode_ = 0;
		ReleaseCapture();
		new_pt_ = point;
		const int delta = -(new_pt_.y - old_pt_.y);
		if (p_chart_data_wnd_ != nullptr)
		{
			CRect prev_rect;
			p_chart_data_wnd_->GetClientRect(prev_rect);
			auto new_rect = prev_rect;
			if (!b_horizontal_)
			{
				if (b_bottom_)
					new_rect.bottom -= delta;
				else
					new_rect.top += delta;
				p_chart_data_wnd_->zoom_data(&prev_rect, &new_rect);
				p_chart_data_wnd_->Invalidate();
			}
			else
			{
				if (b_bottom_)
					new_rect.left -= delta;
				else
					new_rect.right += delta;
			}
		}
	}
	else
		CWnd::OnLButtonUp(n_flags, point);
}
