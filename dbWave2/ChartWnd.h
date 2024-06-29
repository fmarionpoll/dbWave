#pragma once
#include "dbWave_constants.h"
#include "Ruler.h"
#include "Taglist.h"

// display parameters: m_range_mode
constexpr auto RANGE_ALL = 0;
constexpr auto RANGE_TIME_INTERVALS = 1;
constexpr auto RANGE_INDEX = 2;

// display parameters: plot_mode_
constexpr auto PLOT_BLACK = 0;
constexpr auto PLOT_ONE_CLASS_ONLY = 1;
constexpr auto PLOT_ONE_CLASS = 2;
constexpr auto PLOT_CLASS_COLORS = 3;
constexpr auto PLOT_SINGLE_SPIKE = 4;
constexpr auto PLOT_ALLGREY = 5;
constexpr auto PLOT_WITHIN_BOUNDS = 6;
constexpr auto PLOT_ONE_COLOR = 7;

constexpr auto NB_COLORS = 17;
constexpr auto BLACK_COLOR = 0;
constexpr auto SILVER_COLOR = 2;
constexpr auto RED_COLOR = 4;

constexpr auto NB_CURSORS = 6;

#include "RulerBar.h"
#include "SCOPESTRUCT.h"

// CChartWnd window

class ChartWnd : public CWnd
{
public:
	ChartWnd();
	~ChartWnd() override;
	DECLARE_SERIAL(ChartWnd)
	void Serialize(CArchive& archive) override;

	virtual BOOL Create(LPCTSTR lpsz_window_name, DWORD dw_style, const RECT& rect, CWnd* p_parent_wnd, UINT n_id,
	                    CCreateContext* p_context = nullptr);
	virtual SCOPESTRUCT* get_scope_parameters();
	virtual void set_scope_parameters(SCOPESTRUCT* p_struct);
	virtual int set_mouse_cursor_type(int cursor_type);
	virtual void zoom_data(CRect* prev_rect, CRect* new_rect);
	virtual void display_vertical_tags(CDC* p_dc);
	virtual void display_horizontal_tags(CDC* p_dc);
	virtual void plot_data_to_dc(CDC* p_dc);

	void erase_background(CDC* p_dc);
	void PlotToBitmap(CDC* p_dc);
	static COLORREF get_color(const int color_index) { return color_table_[color_index]; }
	static void set_color_table_at(const int color_index, const COLORREF color_ref) { color_table_[color_index] = color_ref; }
	static int find_color_index(COLORREF color_ref);

	void set_string(const CString& cs) { cs_empty_ = cs; }

	void set_b_use_dib(BOOL b_set_plot); // use DIB or not
	void set_display_area_size(int cx, int cy); // set size of the display area

	CSize get_rect_size() const { return {m_display_rect_.Width() + 1, m_display_rect_.Height() + 1}; }
	int get_rect_height() const { return m_display_rect_.Height() + 1; }
	int get_rect_width() const { return m_display_rect_.Width() + 1; }
	int get_mouse_cursor_type() const { return cursor_type_; }
	void set_mouse_cursor(int cursor_type);

	void set_yw_ext_org(int extent, int zero);
	void set_xw_ext_org(const int extent, const int zero)
	{
		m_x_we_ = extent;
		m_x_wo_ = zero;
	}

	int get_yw_extent() const { return m_y_we_; }
	int get_yw_org() const { return m_y_wo_; }
	int get_xw_extent() const { return m_x_we_; }
	int get_xw_org() const { return m_x_wo_; }

	auto set_nx_scale_cells(int i_cells, int i_ticks = 0, int i_tick_line = 0) -> void;
	void set_ny_scale_cells(int i_cells, int i_ticks = 0, int i_tick_line = 0);
	int get_nx_scale_cells() const { return scope_structure_.iXCells; }
	int get_ny_scale_cells() const { return scope_structure_.iYCells; }
	void set_x_scale_unit_value(float x);
	void set_y_scale_unit_value(float y);
	void attach_external_x_ruler(RulerBar* p_x_ruler) { x_ruler_bar = p_x_ruler; }
	void attach_external_y_ruler(RulerBar* p_y_ruler) { y_ruler_bar = p_y_ruler; }

	void xor_vertical_tag(int x_point);
	void xor_temp_vertical_tag(int x_point);
	void xor_horizontal_tag(int y_point);

	void reset_xor_tag()
	{
		m_pt_last_.x = -1;
		m_pt_last_.y = -1;
	}

	void reflect_mouse_move_message(const HWND h_window) { m_hwnd_reflect_ = h_window; }
	void set_cursor_max_on_dbl_click(const int imax) { cursor_index_max_ = imax; }
	void plot_to_bitmap(CBitmap* p_bitmap);
	void draw_grid(CDC* p_dc);
	void adjust_display_rect(const CRect* rect);
	BOOL get_b_draw_frame() const { return scope_structure_.bDrawframe; }
	void set_b_draw_frame(const BOOL flag) { scope_structure_.bDrawframe = flag; }
	CRect get_defined_rect() const { return {m_pt_first_.x, m_pt_first_.y, m_pt_last_.x, m_pt_last_.y}; }
	void set_bottom_comment(const BOOL flag, const CString& cs)
	{
		cs_bottom_comment = cs;
		b_bottom_comment = flag;
	}

	Ruler x_ruler{};
	Ruler y_ruler{};
	CFont h_font{};
	BOOL b_nice_grid = false;
	int abscissa_height = 10;
	int ordinates_width = 25;
	RulerBar* x_ruler_bar = nullptr;
	RulerBar* y_ruler_bar = nullptr;

	TagList horizontal_tags{}; 
	TagList vertical_tags{}; 
	CString cs_bottom_comment{};
	BOOL b_bottom_comment = false;

	// Implementation
protected:
	static int cursors_count_; 
	static HCURSOR cursors_[NB_CURSORS]; 
	static int cursors_drag_mode_[NB_CURSORS]; // cursor mode: 0=invert rect; 1=catch object
	int cursor_index_max_ = NB_CURSORS;
	int cursor_type_ = 0; 
	int old_cursor_type_ = 0;
	HCURSOR handle_current_cursor_{}; 
	int current_cursor_mode_ = 0; // current cursor drag mode
	static COLORREF color_table_[NB_COLORS]; // array with color ref

	CPen pen_table_[NB_COLORS]; // table with CPen objects (same colors as color table
	BOOL b_left_mouse_button_down_ = false;
	BOOL b_use_dib_ = false;
	CDC plot_dc_{};
	CWordArray array_mark_{};
	CPen black_dotted_pen_{};
	CString cs_empty_;
	SCOPESTRUCT scope_structure_{};

	int plot_mode_ = 0;
	int index_color_background_ = SILVER_COLOR;
	int index_color_selected_ = BLACK_COLOR;
	BOOL b_erase_background_ = TRUE; // erase background (flag)
	BOOL b_vertical_tags_as_long_ = FALSE; // flag: TRUE if VT tags are defined as long
	long file_position_first_left_pixel_ = 0; // file position of first left pixel
	long file_position_last_right_pixel_ = 0; // file position of last right pixel
	long file_position_equivalent_to_mouse_jitter_{}; // file position range corresponding mouse jitter

	// plotting options - parameters for PLOT_WITHIN_BOUNDS
	int plot_within_mode_{};
	int lower_limit_{};
	int upper_limit_{};
	int color_index_within_limits_{};
	int color_index_outside_limits_{};

	// mouse tracking modes
	int hc_trapped_{}; // cursor index trapped by the mouse
	int track_mode_ = TRACK_OFF;

	int m_x_wo_ = 0; // x origin, extent / window & view
	int m_x_we_ = 1;
	int m_x_vo_ = 0;
	int m_x_ve_ = 1;
	int m_y_wo_ = 0; // y origin, extent / window & view
	int m_y_we_ = 1;
	int m_y_vo_ = 0;
	int m_y_ve_ = 1;

	int m_cur_track_{}; // threshold  tracked
	CPoint m_pt_first_{};
	CPoint m_pt_curr_{};
	CPoint m_pt_last_{};
	CRect m_client_rect_{};
	CRect m_display_rect_{};

	int cx_jitter_; // mouse horizontal hitter
	int cy_jitter_; // mouse vertical jitter
	CRect rect_zoom_from_; // temp rect
	CRect rect_zoom_to_; // temp rect
	int i_undo_zoom_ = 0; // 1: rect+ stored; -1: rect- stored; 0: none stored (not implemented)

	BOOL m_b_allow_props_ = true;
	HWND m_hwnd_reflect_ = nullptr;
	Tag* m_temp_vertical_tag_ = nullptr;

protected:
	void PreSubclassWindow() override;
	virtual int hit_curve(CPoint point);

	void prepare_dc(CDC* p_dc, const CPrintInfo* p_info = nullptr);
	void capture_cursor();
	void release_cursor();
	void left_button_up_horizontal_tag(UINT nFlags, CPoint point);
	void send_my_message(int code, int code_parameter) const;
	void post_my_message(int code, int code_parameter) const;
	void invert_tracker(CPoint point); // invert rectangle when mouse-left is down
	int hit_horizontal_tag(int y); // test if point is on an horizontal tag line
	int hit_vertical_tag_pixel(int x); // test if point is on a vertical tag line
	int hit_vertical_tag_long(long lx); // test if point is on a vertical tag line
	void zoom_in();
	void zoom_out();
	void zoom_pop();
	void draw_grid_evenly_spaced(CDC* p_dc) const;
	void draw_grid_from_ruler(CDC* p_dc, const Ruler* p_ruler) const;
	void draw_grid_nicely_spaced(CDC* p_dc);
	void draw_scale_from_ruler(CDC* p_dc, const Ruler* p_ruler);

	// Generated message map functions
protected:
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* p_dc);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* p_wnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
