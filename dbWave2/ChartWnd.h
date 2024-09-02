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

constexpr auto BLACK_COLOR = 0;
constexpr auto SILVER_COLOR = 2;
constexpr auto RED_COLOR = 4;

constexpr auto NB_CURSORS = 6;

#include "RulerBar.h"
#include "options_scope_struct.h"

// CChartWnd window

class ChartWnd : public CWnd
{
public:
	ChartWnd();
	~ChartWnd();

	DECLARE_SERIAL(ChartWnd)
	void Serialize(CArchive& ar) override;

	virtual BOOL Create(LPCTSTR lpsz_window_name, DWORD dw_style, const RECT& rect, CWnd* p_parent_wnd, UINT n_id,
	                    CCreateContext* p_context = nullptr);

	static constexpr int nb_colors {10}; 
	static COLORREF color_spike_class[nb_colors];
	static COLORREF color_spike_class_selected[nb_colors];
	static COLORREF color_spike_class_text[nb_colors];

protected:
	static int cursors_count_;
	static HCURSOR cursors_[NB_CURSORS];
	static int cursors_drag_mode_[NB_CURSORS]; // cursor mode: 0invert rect; 1catch object
	int cursor_index_max_{ NB_CURSORS };
	int cursor_type_{ 0 };
	int old_cursor_type_{ 0 };
	HCURSOR handle_current_cursor_{};
	int current_cursor_mode_{ 0 }; 

	CPen pen_table_[nb_colors];
	BOOL b_left_mouse_button_down_ {false};
	BOOL b_use_dib_{ false };
	CDC plot_dc_{};
	int saved_dc_;
	COLORREF saved_background_color_;
	long window_duration_{ 1 };

	CWordArray array_mark_{};
	CPen black_dotted_pen_{};
	CString cs_empty_;
	options_scope_struct scope_structure_{};

	int plot_mode_ {0};
	int index_color_background_ {SILVER_COLOR};
	int index_color_selected_  {BLACK_COLOR};
	BOOL b_erase_background_  {TRUE}; // erase background (flag)
	BOOL b_vertical_tags_as_long_  {FALSE}; // TRUE if VT tags are defined as long
	long file_position_first_left_pixel_  {0}; // file position of first left pixel
	long file_position_last_right_pixel_  {0}; // file position of last right pixel
	long file_position_equivalent_to_mouse_jitter_{}; // file position range corresponding mouse jitter

	// plotting options - parameters for PLOT_WITHIN_BOUNDS
	int plot_within_mode_{};
	int lower_limit_{};
	int upper_limit_{};
	int color_index_within_limits_{};
	int color_index_outside_limits_{};

	// mouse tracking modes
	int hc_trapped_{}; // cursor index trapped by the mouse
	int track_mode_  {TRACK_OFF};

	int x_wo_  {0};
	int x_we_  {1};
	int x_viewport_origin_  {0};
	int x_viewport_extent_  {1};

	int y_wo_  {0};
	int y_we_  {1};
	int y_vo_  {0};
	int y_ve_  {1};

	int cur_track_{}; // threshold  tracked
	CPoint pt_first_{};
	CPoint pt_curr_{};
	CPoint pt_last_{};
	CRect client_rect_{};
	CRect display_rect_{};

	int cx_mouse_jitter_;
	int cy_mouse_jitter_;
	CRect rect_zoom_from_;
	CRect rect_zoom_to_;
	int i_undo_zoom_  {0}; // 1: rect+ ; -1: rect- ; 0: none stored (not implemented)

	BOOL b_allow_props_  {true};
	HWND h_wnd_reflect_  {nullptr};
	Tag* temp_vertical_tag_  {nullptr};

public:
	virtual void plot_data_to_dc(CDC* p_dc);
	void display_text_bottom_left(CDC* p_dc, const CString& text, const COLORREF& color) const;
	virtual options_scope_struct* get_scope_parameters();
	virtual void set_scope_parameters(options_scope_struct* p_struct);
	virtual int set_mouse_cursor_type(int cursor_type);
	virtual void zoom_data(CRect* prev_rect, CRect* new_rect);

	void erase_background(CDC* p_dc);
	void plot_to_bitmap(CDC* p_dc);

	static COLORREF get_color(const int color_index) { return color_spike_class[color_index]; }
	static void set_color_table_at(const int color_index, const COLORREF color_ref) { color_spike_class[color_index] = color_ref; }
	static int find_color_index(COLORREF color_ref);

	void set_string(const CString& cs) { cs_empty_ = cs; }
	void set_b_use_dib(BOOL b_set_plot); // use DIB or not
	void set_display_area_size(int cx, int cy); // set size of the display area

	CSize get_rect_size() const { return {display_rect_.Width() + 1, display_rect_.Height() + 1}; }
	int get_rect_height() const { return display_rect_.Height() + 1; }
	int get_rect_width() const { return display_rect_.Width() + 1; }
	int get_mouse_cursor_type() const { return cursor_type_; }
	void set_mouse_cursor(int cursor_type);

	void set_yw_ext_org(int extent, int zero);
	void set_xw_ext_org(const int extent, const int zero) {
		x_we_ = extent;
		x_wo_ = zero;
	}

	int get_yw_extent() const { return y_we_; }
	int get_yw_org() const { return y_wo_; }
	int get_xw_extent() const { return x_we_; }
	int get_xw_org() const { return x_wo_; }
	int get_nx_scale_cells() const { return scope_structure_.i_x_cells; }
	int	get_ny_scale_cells() const { return scope_structure_.i_y_cells; }

	auto set_n_x_scale_cells(int i_cells, int i_ticks = 0, int i_tick_line = 0) -> void;
	void set_ny_scale_cells(int i_cells, int i_ticks = 0, int i_tick_line = 0);
	void set_x_scale_unit_value(float x);
	void set_y_scale_unit_value(float y);

	void attach_external_x_ruler(RulerBar* p_x_ruler) { x_ruler_bar = p_x_ruler; }
	void attach_external_y_ruler(RulerBar* p_y_ruler) { y_ruler_bar = p_y_ruler; }

	void reflect_mouse_move_message(const HWND h_window) { h_wnd_reflect_ = h_window; }
	void set_cursor_max_on_dbl_click(const int imax) { cursor_index_max_ = imax; }
	void draw_grid(CDC* p_dc);
	void adjust_display_rect(const CRect* p_rect);
	BOOL get_b_draw_frame() const { return scope_structure_.b_draw_frame; }
	void set_b_draw_frame(const BOOL flag) { scope_structure_.b_draw_frame = flag; }
	CRect get_defined_rect() const { return {pt_first_.x, pt_first_.y, pt_last_.x, pt_last_.y}; }
	void set_bottom_comment(const BOOL flag, const CString& cs)
	{
		cs_bottom_comment = cs;
		b_bottom_comment = flag;
	}

	Ruler x_ruler {};
	Ruler y_ruler {};
	CFont h_font {};
	boolean b_nice_grid {false};
	int abscissa_height {10};
	int ordinates_width {25};
	RulerBar* x_ruler_bar {nullptr};
	RulerBar* y_ruler_bar {nullptr};

	CString cs_bottom_comment {};
	BOOL b_bottom_comment {false};

	TagList vt_tags{};
	virtual void display_vt_tags_int_values(CDC* p_dc);
	void xor_vt_tag(int x_new, int x_old);
	void xor_temp_vt_tag(int x_point);

	TagList hz_tags{};
	virtual void display_hz_tags(CDC* p_dc);
	void xor_hz_tag(int y_new, int y_old);

	// Implementation
protected:
	void PreSubclassWindow() override;
	
	void prepare_dc(CDC* p_dc, const CPrintInfo* p_info = nullptr);
	void capture_cursor();
	void release_cursor();
	void left_button_up_horizontal_tag(UINT n_flags, CPoint point);
	void left_button_up_vertical_tag(UINT n_flags, CPoint point);

	void send_my_message(int code, int code_parameter) const;

	void invert_tracker(CPoint point);

	virtual int hit_curve(CPoint point);
	int hit_horizontal_tag(int y);
	int hit_vertical_tag_pixel(const int x) {
		return vt_tags.hit_vertical_tag_pixel(x, 3); }
	int hit_vertical_tag_long(const long lx) {
		return vt_tags.hit_vertical_tag_long(lx, file_position_equivalent_to_mouse_jitter_); }

	void zoom_in();
	void zoom_out();
	void zoom_pop();

	void draw_grid_evenly_spaced(CDC* p_dc) const;
	void draw_grid_from_ruler(CDC* p_dc, const Ruler* p_ruler) const;
	void draw_grid_nicely_spaced(CDC* p_dc);
	void draw_scale_from_ruler(CDC* p_dc, const Ruler* p_ruler);

	// Generated message map functions
	afx_msg virtual void OnSize(UINT n_type, int cx, int cy);
	afx_msg virtual BOOL OnEraseBkgnd(CDC* p_dc);
	afx_msg virtual void OnPaint();
	afx_msg virtual BOOL OnSetCursor(CWnd* p_wnd, UINT n_hit_test, UINT message);
	afx_msg virtual void OnMouseMove(UINT n_flags, CPoint point);
	afx_msg virtual void OnLButtonDown(UINT n_flags, CPoint point);
	afx_msg virtual void OnLButtonUp(UINT n_flags, CPoint point);
	afx_msg virtual void OnLButtonDblClk(UINT n_flags, CPoint point);
	afx_msg virtual void OnRButtonUp(UINT n_flags, CPoint point);
	afx_msg virtual void OnRButtonDown(UINT n_flags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
