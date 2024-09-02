// CONTROLP.CPP

#include "StdAfx.h"
#include "StretchControls.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CCtrlProp, CObject)

CCtrlProp::CCtrlProp()
= default;

CCtrlProp::CCtrlProp(const int i_id, const HWND h_c, const int x_size_how, const int y_size_how, const CRect* rect)
{
	m_ctrl_id = i_id;
	m_h_wnd = h_c;
	m_x_size_how = x_size_how;
	m_y_size_how = y_size_how;
	m_rect = *rect;
}

IMPLEMENT_DYNCREATE(CStretchControl, CObject)

CStretchControl::CStretchControl()
= default;

void CStretchControl::attach_parent(CFormView* p_f)
{
	m_parent_ = p_f;
	// save pointer to parent
	m_dlg_min_size_ = p_f->GetTotalSize(); // get size of dialog
	m_dialog_size_.cy = m_dlg_min_size_.cy - m_bottom_bar_height_; // reduce client area
	m_dialog_size_.cx = m_dlg_min_size_.cx - m_right_bar_width_; // by this value
	m_slave_max_ = 0;
}

void CStretchControl::attach_dialog_parent(CDialog* p_wnd)
{
	m_parent_ = p_wnd;
	CRect rect;
	p_wnd->GetClientRect(rect);
	m_dlg_min_size_ = CSize(rect.Width(), rect.Height());
	m_dialog_size_ = m_dlg_min_size_;
}

void CStretchControl::change_bar_sizes(const int right_width, const int bottom_height)
{
	m_bottom_bar_height_ = right_width; // height/scrollbar
	m_right_bar_width_ = bottom_height; // width/scrollbar
	m_dialog_size_.cy = m_dlg_min_size_.cy - m_bottom_bar_height_; // reduce client area
	m_dialog_size_.cx = m_dlg_min_size_.cx - m_right_bar_width_; // by this value
}

BOOL CStretchControl::new_prop(const int i_id, const int x_size_mode, const int y_size_mode)
{
	CWnd* pC = m_parent_->GetDlgItem(i_id);
	return new_prop(pC, i_id, x_size_mode, y_size_mode);
}

BOOL CStretchControl::new_prop(const CWnd* p_wnd, const int i_id, const int x_size_mode, const int y_size_mode)
{
	if (m_parent_ == nullptr)
		return FALSE;

	CRect rect;
	p_wnd->GetWindowRect(&rect); // get screen window coordinates
	m_parent_->ScreenToClient(&rect); // convert in client coordinates
	const CRect rect0 = rect;

	rect.right -= m_dialog_size_.cx; // distance of right from border
	rect.bottom -= m_dialog_size_.cy; // distance from lower border
	auto* ppC = new CCtrlProp(i_id, p_wnd->m_hWnd, x_size_mode, y_size_mode, &rect);
	ASSERT(ppC != NULL);
	ppC->m_rect0 = rect0;
	ppC->m_r_master0 = CRect(0, 0, 0, 0);
	ppC->m_slave_order = 0;

	ctrl_prop_ptr_array_.Add(ppC);
	return TRUE;
}

// iID stays at a constant offset from the sides of iMASTER
// sizeModes concern iMASTER
// if iMASTER already defined x_sizeMaster and y_sizeMaster are not used

BOOL CStretchControl::new_slave_prop(const int i_id, const int x_size_slave, const int y_size_slave, const int id_master)
{
	if (m_parent_ == nullptr)
		return FALSE;

	auto p_c = m_parent_->GetDlgItem(id_master); // get handle of master
	CRect rect_master;
	p_c->GetWindowRect(&rect_master); // get screen window coordinates
	m_parent_->ScreenToClient(&rect_master); // convert in client coordinates
	const auto rect_master_0 = rect_master;

	// find idMASTER
	auto i_master = -1; // master index
	const CCtrlProp* prop_master = nullptr; // pointer
	if (ctrl_prop_ptr_array_.GetUpperBound() >= 0) // do we have masters?
	{
		for (int i = 0; i <= ctrl_prop_ptr_array_.GetUpperBound(); i++) // loop
		{
			prop_master = ctrl_prop_ptr_array_.GetAt(i); // get pointer to current master
			if (prop_master->get_id() == id_master) // same ID number??
			{
				i_master = i; // yes: exit loop
				break; // omit considering size modes
			}
		}
	}
	// not found: exit with error
	if (i_master < 0)
		return FALSE;

	// create slave
	CRect rect;
	p_c = m_parent_->GetDlgItem(i_id); // get handle of control
	p_c->GetWindowRect(&rect); // get screen window coordinates
	m_parent_->ScreenToClient(&rect); // convert in client coordinates
	const auto rect0 = rect;
	rect.left = rect.left - rect_master_0.left;
	rect.right = rect.right - rect_master_0.right;
	rect.top = rect.top - rect_master_0.top;
	rect.bottom = rect.bottom - rect_master_0.bottom;

	auto* pb = new CCtrlProp(i_id, p_c->m_hWnd, x_size_slave, y_size_slave, &rect);
	ASSERT(pb != NULL);
	pb->set_master(i_master);
	pb->m_rect0 = rect0;
	pb->m_r_master0 = rect_master_0;
	pb->m_slave_order = 1;
	if (prop_master != nullptr)
		pb->m_slave_order = prop_master->m_slave_order + 1;

	if (m_slave_max_ < pb->m_slave_order)
		m_slave_max_ = pb->m_slave_order;

	ctrl_prop_ptr_array_.Add(pb);
	return TRUE;
}

CStretchControl::~CStretchControl()
{
	if (ctrl_prop_ptr_array_.GetUpperBound() >= 0)
	{
		for (auto i = ctrl_prop_ptr_array_.GetUpperBound(); i >= 0; i--)
		{
			delete ctrl_prop_ptr_array_[i];
			ctrl_prop_ptr_array_.RemoveAt(i);
		}
		ctrl_prop_ptr_array_.RemoveAll();
	}
}

// --------------------------------------------------------------------------
// OnSize passes:
// n_type: SIZE_MAXIMIZED, _MINIMIZED, _RESTORED, _MAX_HIDE, _MAX_SHOW
// cx = new width of the client area
// cy = new height of the client area
// --------------------------------------------------------------------------

void CStretchControl::resize_controls(UINT n_type, int cx, int cy)
{
	if (ctrl_prop_ptr_array_.GetSize() <= 0 || cx <= 0 || cy <= 0)
		return;

	cx = (cx < m_dlg_min_size_.cx) ? m_dlg_min_size_.cx : cx; // no action if less than min size
	cy = (cy < m_dlg_min_size_.cy) ? m_dlg_min_size_.cy : cy; // no action if less than min size
	area_left_by_controls(&cx, &cy);
	if (cx == m_dialog_size_.cx && cy == m_dialog_size_.cy)
		return;

	// prepare looping through all controls
	auto h_dwp = BeginDeferWindowPos(ctrl_prop_ptr_array_.GetSize());
	if (h_dwp == nullptr)
		return; // exit if none available

	// loop through all windows
	for (auto j = 0; j <= m_slave_max_; j++)
	{
		for (auto i = ctrl_prop_ptr_array_.GetUpperBound(); i >= 0; i--)
		{
			auto pa = ctrl_prop_ptr_array_.GetAt(i);
			if (pa->m_slave_order != j)
				continue;
			const auto new_rect = align_control(pa, cx, cy);
			pa->set_master_rect(new_rect);
			h_dwp = DeferWindowPos(h_dwp, pa->get_h_wnd(), nullptr,
			                       new_rect.left,
			                       new_rect.top,
			                       new_rect.right - new_rect.left,
			                       new_rect.bottom - new_rect.top,
			                       SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}

	EndDeferWindowPos(h_dwp);

	// update dialog size
	m_dialog_size_.cx = cx;
	m_dialog_size_.cy = cy;
}

// AlignControls()
//	i = index for array containing IDs && resizing flags
//	cx, cy = new coordinates of window
//	hDWP = handle for defer_window_pos

CRect CStretchControl::align_control(const CCtrlProp* pa, int cx, int cy)
{
	const auto h_wnd_child = pa->get_h_wnd(); // h_wnd_Child = window to move
	// compute reference rectangle either from dialog or reference rect
	CRect ref_rect; // reference rectangle
	CRect new_rect; // control's rect

	GetWindowRect(h_wnd_child, &new_rect); // get window coordinates of the window before it has moved
	m_parent_->ScreenToClient(&new_rect); // convert into client coordinates
	CSize ref_size = m_dialog_size_;

	if (pa->get_master() >= 0) // if slave, get ref from master
	{
		const auto pb = ctrl_prop_ptr_array_.GetAt(pa->get_master());
		ref_rect = pb->get_master_rect(); // load already transformed rect
		cx = ref_rect.Width(); // get width
		cy = ref_rect.Height(); // and height
		ref_size = CSize(cx, cy);
	}
	// set ref rectangle from dialog box
	else
	{
		ref_rect.left = ref_rect.top = 0; // otherwise fill refRect with
		ref_rect.right = cx; // dialog coordinates
		ref_rect.bottom = cy;
	}

	// change size according to the flags associated with x and y directions

	// align along the horizontal direction
	int diff; // volatile variable
	switch (pa->get_x_size_how())
	{
	case SZEQ_XLEQ: // x left== - no action
		diff = new_rect.Width(); // size of the control (fixed)
		new_rect.left = ref_rect.left + pa->m_rect.left;
		new_rect.right = new_rect.left + diff;
		break;
	case SZEQ_XREQ: // x right== && same size
		diff = new_rect.Width(); // size of the control (fixed)
		new_rect.right = ref_rect.right + pa->m_rect.right; // offset from right side
		new_rect.left = new_rect.right - diff; // -> left
		break;
	case XLEQ_XREQ: // x left== && x right== (change size)
		new_rect.left = ref_rect.left + pa->m_rect.left; // constant offset from left side
		new_rect.right = ref_rect.right + pa->m_rect.right; // constant offset from right side
		break;
	case SZEQ_XLPR: // x left%  && same size: ?? wrong with slaves
		diff = new_rect.Width(); // size of the control (constant)
		new_rect.left = ref_rect.left + MulDiv(pa->m_rect.left, cx, ref_size.cx);
		new_rect.right = new_rect.left + diff; // -> left
		break;
	case SZEQ_XRPR: // x right%	&& same size
		diff = new_rect.Width(); // get size
		new_rect.right = ref_rect.right + MulDiv(pa->m_rect.right, cx, ref_size.cx);
		new_rect.left = new_rect.right - diff; // left
		break;
	case XLPR_XRPR: // x left% && x right%		size changes
		new_rect.right = cx + MulDiv(pa->m_rect.right, cx, m_dlg_min_size_.cx);
		new_rect.left = MulDiv(pa->m_rect.left, cx, m_dlg_min_size_.cx);
		break;
	case SZPR_XLEQ: // x left== && size prop
		diff = MulDiv(new_rect.Width(), cx, ref_size.cx); // get size
		new_rect.left = ref_rect.left + pa->m_rect.left; // left position
		new_rect.right = new_rect.left + diff; // right position
		break;
	case SZPR_XREQ:
		diff = MulDiv(new_rect.Width(), cx, ref_size.cx); // get size
		new_rect.right = ref_rect.right + pa->m_rect.right; // clip right to the right side
		new_rect.left = new_rect.right - diff; // left position
		break;
	case RIGHT_BAR: // clip to the right, outside the area
		new_rect.left = ref_rect.right; // left= window size
		new_rect.right = new_rect.left + m_right_bar_width_;
		break;
	default:
		break;
	}

	// align along the vertical direction
	switch (pa->get_y_size_how())
	{
	case SZEQ_YTEQ: // y top== - no action
		diff = new_rect.Height();
		new_rect.top = ref_rect.top + pa->m_rect.top;
		new_rect.bottom = new_rect.top + diff;
		break;
	case SZEQ_YBEQ: // y bottom==
		diff = new_rect.Height();
		new_rect.bottom = ref_rect.bottom + pa->m_rect.bottom;
		new_rect.top = new_rect.bottom - diff;
		break;
	case YTEQ_YBEQ: // y_top== && y_bottom==
		new_rect.top = ref_rect.top + pa->m_rect.top;
		new_rect.bottom = ref_rect.bottom + pa->m_rect.bottom;
		break;
	case SZEQ_YTPR: // y top%
		diff = new_rect.Height();
		new_rect.top = ref_rect.top + MulDiv(pa->m_rect.top, cy, ref_size.cy);
		new_rect.bottom = new_rect.top + diff;
		break;
	case SZEQ_YBPR: // y bottom%
		diff = new_rect.Height();
		new_rect.bottom = ref_rect.bottom + MulDiv(pa->m_rect.bottom, cy, ref_size.cy);
		new_rect.top = new_rect.bottom - diff;
		break;
	case YTPR_YBPR: // proportional distance from both borders -- size accordingly
		new_rect.top = MulDiv(pa->m_rect.top, cy, m_dlg_min_size_.cy);
		new_rect.bottom = cy + MulDiv(pa->m_rect.bottom, cy, m_dlg_min_size_.cy);
		break;
	case SZPR_YTEQ: // y top== && size prop
		if (pa->get_master() >= 0) // if slave, get ref from master
		{
			diff = MulDiv(pa->m_rect0.Height(), ref_rect.Height(), pa->m_r_master0.Height());
		}
		else
			diff = MulDiv(new_rect.Height(), cy, ref_size.cy); // get size
		new_rect.top = ref_rect.top + pa->m_rect.top;
		new_rect.bottom = new_rect.top + diff; // right position
		break;
	case SZPR_YBEQ:
		if (pa->get_master() >= 0) // if slave, get ref from master
		{
			diff = MulDiv(pa->m_rect0.Height(), ref_rect.Height(), pa->m_r_master0.Height());
		}
		else
			diff = MulDiv(new_rect.Height(), cy, ref_size.cy); // get size
		new_rect.bottom = ref_rect.bottom + pa->m_rect.bottom; // clip right to the right side
		new_rect.top = new_rect.bottom - diff; // left position
		break;
	case BOTTOM_BAR:
		new_rect.top = ref_rect.bottom;
		new_rect.bottom = new_rect.top + m_bottom_bar_height_;
		break;
	default:
		break;
	}
	return new_rect;
}

void CStretchControl::area_left_by_controls(int* cx, int* cy) const
{
	if (GetbVBarControls())
		*cx -= m_right_bar_width_;
	if (GetbHBarControls())
		*cy -= m_bottom_bar_height_;
}

// --------------------------------------------------------------------------
// DisplayVBarControls - assume m_DialogSize minus bar area
// return previous state
// --------------------------------------------------------------------------

BOOL CStretchControl::display_v_bar_controls(const BOOL b_visible)
{
	const auto b_prev_state = b_vertical_bar_; // previous state of bar
	b_vertical_bar_ = b_visible; // set state
	const auto show_flag = (b_vertical_bar_ ? SW_SHOW : SW_HIDE);
	const auto rect_flag = ((b_horizontal_bar_ && b_vertical_bar_) ? SW_SHOW : SW_HIDE);

	// loop through all controls and set visible state
	for (auto i = 0; i < ctrl_prop_ptr_array_.GetSize(); i++)
	{
		auto pa = ctrl_prop_ptr_array_.GetAt(i);
		if (pa->get_x_size_how() == RIGHT_BAR)
		{
			if (pa->get_y_size_how() != BOTTOM_BAR)
				ShowWindow(pa->get_h_wnd(), show_flag);
			else
				ShowWindow(pa->get_h_wnd(), rect_flag);
		}
	}
	update_client_controls();
	return b_prev_state;
}

// --------------------------------------------------------------------------
// DisplayHBarControls -- assume m_DialogSize minus bar area
// return previous state
// --------------------------------------------------------------------------

BOOL CStretchControl::display_h_bar_controls(const BOOL b_visible)
{
	const auto b_prev_state = b_horizontal_bar_; // previous state of bar
	b_horizontal_bar_ = b_visible; // set state
	const auto show_flag = (b_horizontal_bar_ ? SW_SHOW : SW_HIDE);
	const auto rect_flag = ((b_horizontal_bar_ && b_vertical_bar_) ? SW_SHOW : SW_HIDE);
	// loop through all controls and set visible state
	for (auto i = 0; i < ctrl_prop_ptr_array_.GetSize(); i++)
	{
		auto pa = ctrl_prop_ptr_array_.GetAt(i);
		if (pa->get_y_size_how() == BOTTOM_BAR)
		{
			if (pa->get_x_size_how() != RIGHT_BAR)
				ShowWindow(pa->get_h_wnd(), show_flag);
			else
				ShowWindow(pa->get_h_wnd(), rect_flag);
		}
	}
	update_client_controls();
	return b_prev_state;
}

void CStretchControl::update_client_controls() const
{
	RECT client_rect;
	m_parent_->GetClientRect(&client_rect);
	// force resizing
	::SendMessage(m_parent_->m_hWnd, //::PostMessage(m_parent->m_hWnd,
	              WM_SIZE,
	              0,
	              MAKELPARAM(client_rect.right, client_rect.bottom));
}
