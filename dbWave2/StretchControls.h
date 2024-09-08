#if !defined (_CONTROLP_H_)
#define _CONTROLP_H_

constexpr auto SZEQ_XLEQ = 1; // equal size -- constant distance with x left;
constexpr auto SZEQ_XREQ = 2; // equal size -- constant dist with x right;
constexpr auto XLEQ_XREQ = 3; // equal dist from both sides -- size accordingly;
constexpr auto SZEQ_XLPR = 4; // equal size -- proportional distance from left border;
constexpr auto SZEQ_XRPR = 5; // equal size -- proportional distance from right border;
constexpr auto XLPR_XRPR = 6; // proportional distance from both borders -- size accordingly;
constexpr auto SZPR_XLEQ = 7;
constexpr auto SZPR_XREQ = 8;
constexpr auto RIGHT_BAR = 9;

constexpr auto SZEQ_YTEQ = 1; // equal size -- constant distance with y top;
constexpr auto SZEQ_YBEQ = 2; // equal size -- constant dist with y bottom;
constexpr auto YTEQ_YBEQ = 3; // equal dist from both sides -- size accordingly;
constexpr auto SZEQ_YTPR = 4; // equal size -- proportional distance from top border;
constexpr auto SZEQ_YBPR = 5; // equal size -- proportional distance from bottom border;
constexpr auto YTPR_YBPR = 6; // proportional distance from both borders -- size accordingly;
constexpr auto SZPR_YTEQ = 7;
constexpr auto SZPR_YBEQ = 8;
constexpr auto BOTTOM_BAR = 9;

/*
// Définitions supplémentaires
// afin de raccorder les objets de AppStudio entre eux.
// pour la coordonnée y

#define SZEQ_YDS		10	    // equal size -- constant distance with IDC_DISPLAYSOURCE
#define SZPR_YDS		11		// proportional size -- constant distance with IDC_DISPLAYSOURCE
#define SZEQ_YDM		12		// equal size -- constant distance with IDC_DISPLAYMODIF
#define SZPR_YDM		13		// proportional size -- constant distance with IDC_DISPLAYMODIF
#define SZEQ_YDH		14		// equal size -- constant distance with IDC_DISPLAYHISTOG
#define SZPR_YDH		15		// proportional size -- constant distance with IDC_DISPLAYHISTOG
*/


class CCtrlProp : public CObject
{
protected:
public:
	CCtrlProp();
	CCtrlProp(int i_id, HWND h_c, int x_size_how, int y_size_how, const CRect* rect);
	DECLARE_DYNCREATE(CCtrlProp)

	// Attributes
public:
	int m_ctrl_id {0}; // id of the control
	int m_x_size_how { SZEQ_XLEQ }; // flag: how window x position is affected by wm_size
	int m_y_size_how { SZEQ_XLEQ }; // flag: how window y position is affected by wm_size
	HWND m_h_wnd {nullptr}; // handle of window
	BOOL m_b_visible {TRUE}; // state of the window
	int m_slave_order {0}; // 0: no master, 1: 1 master; 2: 2 cascaded masters, etc.
	int m_master {-1}; // ID master from which it is slave; else -1
	CRect m_rect { CRect(0, 0, 0, 0) }; // distances from initial dlg borders
	CRect m_r_master; // master rect dialog client coords
	CRect m_rect0;
	CRect m_r_master0;

	// Operations
public:
	// Helper functions
public:
	int get_id() const { return m_ctrl_id; }
	HWND get_h_wnd() const { return m_h_wnd; }
	int get_x_size_how() const { return m_x_size_how; }
	int get_y_size_how() const { return m_y_size_how; }
	BOOL is_visible() const { return m_b_visible; }
	int get_master() const { return m_master; }
	CRect get_master_rect() const { return m_r_master; }

	void set_id(const int id) { m_ctrl_id = id; }
	void set_h_wnd(const HWND h_c) { m_h_wnd = h_c; }
	void set_x_size_how(const int x_size_how) { m_x_size_how = x_size_how; }
	void set_y_size_how(const int y_size_how) { m_y_size_how = y_size_how; }
	void set_rect_limits(const CRect* rect) { m_rect = *rect; }
	void set_visible(const BOOL b_visible) { m_b_visible = b_visible; }
	void set_master(int index) { m_master = index; }
	void set_master_rect(const CRect& r_master) { m_r_master = r_master; }
};


class CStretchControl : public CObject
{
public:
	CStretchControl();
	DECLARE_DYNCREATE(CStretchControl)
	~CStretchControl() override;

	// Attributes
protected:
	CWnd* m_parent_ {nullptr}; // handle to window parent
	CArray<CCtrlProp*, CCtrlProp*> ctrl_prop_ptr_array_; // CCtrlProps props relative to resizable controls
	SIZE m_dialog_size_ {}; // current dialog size
	SIZE m_dlg_min_size_ {}; // minimal dialog size
	int m_bottom_bar_height_ {0}; // height of system scrollbars
	int m_right_bar_width_ {0}; // width of system scrollbars
	BOOL b_horizontal_bar_ {false}; // true: horizontal bar and associated controls
	BOOL b_vertical_bar_ {false}; // true: vertical bar and associated controls
	int m_slave_max_ {0};

public:
	void attach_parent(CFormView* p_f);
	void attach_dialog_parent(CDialog* p_wnd);
	BOOL new_prop(int i_id, int x_size_mode, int y_size_mode);
	BOOL new_prop(const CWnd* p_wnd, int i_id, int x_size_mode, int y_size_mode);

	BOOL new_slave_prop(int i_id, int x_size_slave, int y_size_slave, int iMASTER);
	BOOL GetbVBarControls() const { return b_vertical_bar_; }
	BOOL GetbHBarControls() const { return b_horizontal_bar_; }
	BOOL display_v_bar_controls(BOOL b_visible);
	BOOL display_h_bar_controls(BOOL b_visible);
	void change_bar_sizes(int right_width, int bottom_height);
	void update_client_controls() const;
	void resize_controls(UINT n_type, int cx, int cy);

	// Attributes
	CSize GetDlgMinSize() const { return m_dlg_min_size_; }
	CSize GetDialogSize() const { return m_dialog_size_; }
	int GetBOTTOMBARHeight() const { return m_bottom_bar_height_; }
	int GetRIGHTBARWidth() const { return m_right_bar_width_; }

	// Implementation
protected:
	CRect align_control(const CCtrlProp* pa, int cx, int cy);
	void area_left_by_controls(int* cx, int* cy) const;
};

#endif // _CONTROLP_H_
