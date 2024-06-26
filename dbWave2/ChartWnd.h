#pragma once
#include "dbWave_constants.h"
#include "Ruler.h"
#include "Taglist.h"

// display parameters: m_range_mode
constexpr auto RANGE_ALL = 0;
constexpr auto RANGE_TIMEINTERVALS = 1;
constexpr auto RANGE_INDEX = 2;

// display parameters: m_plotmode
constexpr auto PLOT_BLACK = 0;
constexpr auto PLOT_ONECLASSONLY = 1;
constexpr auto PLOT_ONECLASS = 2;
constexpr auto PLOT_CLASSCOLORS = 3;
constexpr auto PLOT_SINGLESPIKE = 4;
constexpr auto PLOT_ALLGREY = 5;
constexpr auto PLOT_WITHINBOUNDS = 6;
constexpr auto PLOT_ONECOLOR = 7;

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

	virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dw_style, const RECT& rect, CWnd* pParentWnd, UINT nID,
	                    CCreateContext* pContext = nullptr);
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
	int find_color_index(COLORREF color_ref);

	void set_string(const CString& cs) { m_csEmpty = cs; }

	void set_b_use_dib(BOOL b_set_plot); // use DIB or not
	void set_display_area_size(int cx, int cy); // set size of the display area

	CSize get_rect_size() const { return {m_displayRect.Width() + 1, m_displayRect.Height() + 1}; }
	int get_rect_height() const { return m_displayRect.Height() + 1; }
	int get_rect_width() const { return m_displayRect.Width() + 1; }
	int get_mouse_cursor_type() const { return cursor_type_; }
	void set_mouse_cursor(int cursor_type);

	void set_yw_ext_org(int extent, int zero);
	void set_xw_ext_org(const int extent, const int zero)
	{
		m_xWE = extent;
		m_xWO = zero;
	}

	int get_yw_extent() const { return m_yWE; }
	int get_yw_org() const { return m_yWO; }
	int get_xw_extent() const { return m_xWE; }
	int get_xw_org() const { return m_xWO; }

	auto set_nx_scale_cells(int i_cells, int i_ticks = 0, int i_tick_line = 0) -> void;
	void set_ny_scale_cells(int i_cells, int i_ticks = 0, int i_tick_line = 0);
	int get_nx_scale_cells() const { return scope_structure_.iXCells; }
	int get_ny_scale_cells() const { return scope_structure_.iYCells; }
	void set_x_scale_unit_value(float x);
	void set_y_scale_unit_value(float y);
	void attach_external_x_ruler(RulerBar* p_x_ruler) { m_pXRulerBar = p_x_ruler; }
	void attach_external_y_ruler(RulerBar* p_y_ruler) { m_pYRulerBar = p_y_ruler; }

	void xor_vertical_tag(int x_point);
	void xor_temp_vertical_tag(int x_point);
	void xor_horizontal_tag(int y_point);

	void reset_xor_tag()
	{
		m_ptLast.x = -1;
		m_ptLast.y = -1;
	}

	void ReflectMouseMoveMessg(HWND hwnd) { m_hwndReflect = hwnd; }
	void set_cursor_max_on_dbl_click(const int imax) { cursor_index_max_ = imax; }
	void plot_to_bitmap(CBitmap* p_bitmap);
	void draw_grid(CDC* p_dc);
	void adjust_display_rect(CRect* rect);
	BOOL get_b_draw_frame() const { return scope_structure_.bDrawframe; }
	void set_b_draw_frame(BOOL flag) { scope_structure_.bDrawframe = flag; }
	CRect get_defined_rect() const { return {m_ptFirst.x, m_ptFirst.y, m_ptLast.x, m_ptLast.y}; }
	void set_bottom_comment(const BOOL flag, const CString& cs)
	{
		m_csBottomComment = cs;
		m_bBottomComment = flag;
	}

	Ruler m_xRuler{};
	Ruler m_yRuler{};
	CFont m_hFont{};
	BOOL m_bNiceGrid = false;
	int abscissa_height = 10;
	int ordinates_width = 25;
	RulerBar* m_pXRulerBar = nullptr;
	RulerBar* m_pYRulerBar = nullptr;

	TagList m_HZtags{}; // List of horizontal tag lines
	TagList m_VTtags{}; // List of vertical tag lines
	CString m_csBottomComment{};
	BOOL m_bBottomComment = false;

	// Implementation
protected:
	static int cursors_count_; // objects counter
	static HCURSOR cursors_[NB_CURSORS]; // array with cursor handles
	static int cursors_drag_mode_[NB_CURSORS]; // cursor mode: 0=invert rect; 1=catch object
	int cursor_index_max_ = NB_CURSORS;
	int cursor_type_ = 0; // current cursor
	int old_cursor_type_ = 0;
	HCURSOR current_cursor_{}; // handle to current cursor
	int current_cursor_mode_ = 0; // current cursor drag mode
	static COLORREF color_table_[NB_COLORS]; // array with colorref

	CPen pen_table_[NB_COLORS]; // table with CPen objects (same colors as color table
	BOOL m_bLmouseDown = false;
	BOOL b_use_dib_ = false;
	CDC m_PlotDC{};
	CWordArray m_arrayMark{};
	CPen m_blackDottedPen{};
	CString m_csEmpty;
	SCOPESTRUCT scope_structure_{};

	int m_plotmode = 0;
	int m_colorbackgr = SILVER_COLOR;
	int m_colorselected = BLACK_COLOR;
	BOOL m_erasebkgnd = TRUE; // erase background (flag)
	BOOL m_bVTtagsLONG = FALSE; // flag: TRUE if VTtags are defined as long
	long m_liFirst = 0; // file position of first left pixel
	long m_liLast = 0; // file position of last right pixel
	long m_liJitter{}; // file position range corresponding mouse jitter

	// plotting options - parameters for PLOT_WITHINBOUNDS
	int m_plotwithin_mode{};
	int m_lowerlimit{};
	int m_upperlimit{};
	int m_color_withinlimits{};
	int m_color_outsidelimits{};

	// mouse tracking modes
	int m_HCtrapped{}; // cursor index trapped by the mouse
	int m_trackMode = TRACK_OFF;

	int m_xWO = 0; // x origin, extent / window & view
	int m_xWE = 1;
	int m_xVO = 0;
	int m_xVE = 1;
	int m_yWO = 0; // y origin, extent / window & view
	int m_yWE = 1;
	int m_yVO = 0;
	int m_yVE = 1;

	int m_curTrack{}; // threshold  tracked
	CPoint m_ptFirst{};
	CPoint m_ptCurr{};
	CPoint m_ptLast{};
	CRect m_clientRect{};
	CRect m_displayRect{};

	int m_cxjitter; // mouse horizontal hitter
	int m_cyjitter; // mouse vertical jitter
	CRect m_ZoomFrom; // temp rect
	CRect m_ZoomTo; // temp rect
	int m_iUndoZoom = 0; // 1: rect+ stored; -1: rect- stored; 0: none stored (not implemented)

	BOOL m_bAllowProps = true;
	HWND m_hwndReflect = nullptr;
	Tag* m_tempVTtag = nullptr;

protected:
	void PreSubclassWindow() override;
	virtual int hit_curve(CPoint point);

	void prepare_dc(CDC* p_dc, const CPrintInfo* p_info = nullptr);
	void capture_cursor();
	static void release_cursor();
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
	void draw_grid_from_ruler(CDC* p_dc, const Ruler* pRuler) const;
	void draw_grid_nicely_spaced(CDC* p_dc);
	void draw_scale_from_ruler(CDC* p_dc, Ruler* pRuler);

	// Generated message map functions
protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
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
