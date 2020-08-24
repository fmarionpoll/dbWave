// CONTROLP.CPP

#include "StdAfx.h"
#include "StretchControls.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CCtrlProp, CObject)

CCtrlProp::CCtrlProp()
{
	m_xSizeHow = SZEQ_XLEQ;
	m_ySizeHow = SZEQ_XLEQ;
	m_rect = CRect(0, 0, 0, 0);
	m_ctrlID = NULL;
	m_bVisible = TRUE;
	m_master = -1;
	m_hWnd = nullptr;
	m_slaveorder = 0;
}

CCtrlProp::CCtrlProp(int iID, HWND hC, int xSizeHow, int ySizeHow, CRect* rect)
{
	m_ctrlID = iID;
	m_hWnd = hC;
	m_xSizeHow = xSizeHow;
	m_ySizeHow = ySizeHow;
	m_rect = *rect;
	m_bVisible = TRUE;
	m_master = -1;
	m_slaveorder = 0;
}

IMPLEMENT_DYNCREATE(CStretchControl, CObject)

CStretchControl::CStretchControl() : m_parent(nullptr), m_DialogSize(), m_DlgMinSize(), m_bHBAR(0), m_bVBAR(0)
{
	m_BOTTOMBARHeight = 0; //::GetSystemMetrics(SM_CYHSCROLL);	// standard height
	m_RIGHTBARWidth = 0; //::GetSystemMetrics(SM_CXHSCROLL);	// std width/scrollbar
	m_slavemax = 0;
}

void CStretchControl::AttachParent(CFormView* pF)
{
	m_parent = pF;
	// save pointer to parent
	m_DlgMinSize = pF->GetTotalSize();						// get size of dialog
	m_DialogSize.cy = m_DlgMinSize.cy - m_BOTTOMBARHeight;	// reduce client area
	m_DialogSize.cx = m_DlgMinSize.cx - m_RIGHTBARWidth;	// by this value
	m_slavemax = 0;
}

void CStretchControl::AttachDialogParent(CDialog* p_wnd)
{
	m_parent = p_wnd;
	CRect rect;
	p_wnd->GetClientRect(rect);
	m_DlgMinSize = CSize(rect.Width(), rect.Height());
	m_DialogSize = m_DlgMinSize;
}

void CStretchControl::ChangeBarSizes(int RIGHTwidth, int BOTTOMheight)
{
	m_BOTTOMBARHeight = RIGHTwidth;		// height/scrollbar
	m_RIGHTBARWidth = BOTTOMheight;	// width/scrollbar
	m_DialogSize.cy = m_DlgMinSize.cy - m_BOTTOMBARHeight;	// reduce client area
	m_DialogSize.cx = m_DlgMinSize.cx - m_RIGHTBARWidth;	// by this value
}

BOOL CStretchControl::newProp(int iID, int xsizeMode, int ysizeMode)
{
	CWnd* pC = m_parent->GetDlgItem(iID);
	return newProp(pC, iID, xsizeMode, ysizeMode);
}

BOOL CStretchControl::newProp(CWnd* p_wnd, int iID, int xsizeMode, int ysizeMode)
{
	if (m_parent == nullptr)
		return FALSE;

	CRect rect;
	p_wnd->GetWindowRect(&rect);			// get screen window coordinates
	m_parent->ScreenToClient(&rect);	// convert in client coordinates
	const CRect rect0 = rect;

	rect.right -= m_DialogSize.cx; 		// distance of right from border
	rect.bottom -= m_DialogSize.cy;		// distance from lower border
	auto* ppC = new CCtrlProp(iID, p_wnd->m_hWnd, xsizeMode, ysizeMode, &rect);
	ASSERT(ppC != NULL);
	ppC->m_rect0 = rect0;
	ppC->m_rmaster0 = CRect(0, 0, 0, 0);
	ppC->m_slaveorder = 0;

	ctrlprop_ptr_array.Add(ppC);
	return TRUE;
}

// iID stays at a constant offset from the sides of iMASTER
// sizeModes concern iMASTER
// if iMASTER already defined xsizeMaster and ysizeMaster are not used
BOOL CStretchControl::newSlaveProp(int iID, int xsizeSlave, int ysizeSlave, int idMASTER)
{
	if (m_parent == nullptr)
		return FALSE;

	auto p_c = m_parent->GetDlgItem(idMASTER);	// get handle of master
	CRect rectmaster;
	p_c->GetWindowRect(&rectmaster);				// get screen window coordinates
	m_parent->ScreenToClient(&rectmaster);		// convert in client coordinates
	const auto rectmaster0 = rectmaster;

	// find idMASTER
	auto imaster = -1;							// master index
	CCtrlProp* propmaster = nullptr;					// pointer
	if (ctrlprop_ptr_array.GetUpperBound() >= 0)			// do we have masters?
	{
		for (int i = 0; i <= ctrlprop_ptr_array.GetUpperBound(); i++) // loop
		{
			propmaster = ctrlprop_ptr_array.GetAt(i); // get pointer to current master
			if (propmaster->GetID() == idMASTER)		// same ID number??
			{
				imaster = i;					// yes: exit loop
				break;							// omit considering size modes
			}
		}
	}
	// not found: exit with error
	if (imaster < 0)
		return FALSE;

	// create slave
	CRect rect;
	p_c = m_parent->GetDlgItem(iID); 	// get handle of control
	p_c->GetWindowRect(&rect);			// get screen window coordinates
	m_parent->ScreenToClient(&rect);	// convert in client coordinates
	const auto rect0 = rect;
	rect.left = rect.left - rectmaster0.left;
	rect.right = rect.right - rectmaster0.right;
	rect.top = rect.top - rectmaster0.top;
	rect.bottom = rect.bottom - rectmaster0.bottom;

	auto* pb = new CCtrlProp(iID, p_c->m_hWnd, xsizeSlave, ysizeSlave, &rect);
	ASSERT(pb != NULL);
	pb->SetMaster(imaster);
	pb->m_rect0 = rect0;
	pb->m_rmaster0 = rectmaster0;
	pb->m_slaveorder = 1;
	if (propmaster != nullptr)
		pb->m_slaveorder = propmaster->m_slaveorder + 1;

	if (m_slavemax < pb->m_slaveorder)
		m_slavemax = pb->m_slaveorder;

	ctrlprop_ptr_array.Add(pb);
	return TRUE;
}

CStretchControl::~CStretchControl()
{
	if (ctrlprop_ptr_array.GetUpperBound() >= 0)
	{
		for (auto i = ctrlprop_ptr_array.GetUpperBound(); i >= 0; i--)
		{
			delete ctrlprop_ptr_array[i];
			ctrlprop_ptr_array.RemoveAt(i);
		}
		ctrlprop_ptr_array.RemoveAll();
	}
}

// --------------------------------------------------------------------------
// OnSize passes:
// nType: SIZE_MAXIMIZED, _MINIMIZED, _RESTORED, _MAXHIDE, _MAXSHOW
// cx = new width of the client area
// cy = new height of the client area
// --------------------------------------------------------------------------
void CStretchControl::ResizeControls(UINT nType, int cx, int cy)
{
	if (ctrlprop_ptr_array.GetSize() <= 0 || cx <= 0 || cy <= 0)
		return;

	cx = (cx < m_DlgMinSize.cx) ? m_DlgMinSize.cx : cx;	// no action if less than min size
	cy = (cy < m_DlgMinSize.cy) ? m_DlgMinSize.cy : cy;	// no action if less than min size
	AreaLeftbyControls(&cx, &cy);
	if (cx == m_DialogSize.cx && cy == m_DialogSize.cy)
		return;

	// prepare looping through all controls
	auto h_dwp = ::BeginDeferWindowPos(ctrlprop_ptr_array.GetSize());
	if (h_dwp == nullptr)
		return;		// exit if none available

	// loop through all windows
	for (auto j = 0; j <= m_slavemax; j++)
	{
		for (auto i = ctrlprop_ptr_array.GetUpperBound(); i >= 0; i--)
		{
			auto pa = ctrlprop_ptr_array.GetAt(i);
			if (pa->m_slaveorder != j)
				continue;
			const auto new_rect = AlignControl(pa, cx, cy);
			pa->SetMasterRect(new_rect);
			h_dwp = ::DeferWindowPos(h_dwp, pa->GetHWnd(), nullptr,
				new_rect.left,
				new_rect.top,
				new_rect.right - new_rect.left,
				new_rect.bottom - new_rect.top,
				SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}

	::EndDeferWindowPos(h_dwp);

	// update dialog size
	m_DialogSize.cx = cx;
	m_DialogSize.cy = cy;
}

// AlignControls()
//	i = index for array containing IDs && resizing flags
//	cx, cy = new coordinates of window
//	hDWP = handle for deferwindowpos

CRect CStretchControl::AlignControl(CCtrlProp* pa, int cx, int cy)
{
	const auto hwnd_child = pa->GetHWnd();					// hwndChild = window to move
	// compute reference rectangle either from dialog or reference rect
	CRect ref_rect;									// reference rectangle
	CRect new_rect;									// control's rect

	::GetWindowRect(hwnd_child, &new_rect);			// get window coordinates of the window before it has moved
	m_parent->ScreenToClient(&new_rect);				// convert into client coordinates
	CSize ref_size = m_DialogSize;

	if (pa->GetMaster() >= 0)						// if slave, get ref from master
	{
		auto pb = ctrlprop_ptr_array.GetAt(pa->GetMaster());
		ref_rect = pb->GetMasterRect();				// load already transformed rect
		cx = ref_rect.Width();						// get width
		cy = ref_rect.Height();						// and height
		ref_size = CSize(cx, cy);
	}
	// set ref rectangle from dialog box
	else
	{
		ref_rect.left = ref_rect.top = 0;				// otherwise fill refRect with
		ref_rect.right = cx;							// dialog coordinates
		ref_rect.bottom = cy;
	}

	// change size according to the flags associated with x and y directions

	// align along the horizontal direction
	int diff;										// volatile variable
	switch (pa->GetxSizeHow())
	{
	case SZEQ_XLEQ:								// x left== - no action
		diff = new_rect.Width();					// size of the control (fixed)
		new_rect.left = ref_rect.left + pa->m_rect.left;
		new_rect.right = new_rect.left + diff;
		break;
	case SZEQ_XREQ:								// x right== && same size
		diff = new_rect.Width();					// size of the control (fixed)
		new_rect.right = ref_rect.right + pa->m_rect.right; // offset from right side
		new_rect.left = new_rect.right - diff;	// -> left
		break;
	case XLEQ_XREQ:								// x left== && x right== (change size)
		new_rect.left = ref_rect.left + pa->m_rect.left;   // constant offset from left side
		new_rect.right = ref_rect.right + pa->m_rect.right;// constant offset from right side
		break;
	case SZEQ_XLPR:								// x left%  && same size: ?? wrong with slaves
		diff = new_rect.Width();					// size of the control (constant)
		new_rect.left = ref_rect.left + MulDiv(pa->m_rect.left, cx, ref_size.cx);
		new_rect.right = new_rect.left + diff;	// -> left
		break;
	case SZEQ_XRPR:								// x right%	&& same size
		diff = new_rect.Width();					// get size
		new_rect.right = ref_rect.right + MulDiv(pa->m_rect.right, cx, ref_size.cx);
		new_rect.left = new_rect.right - diff;	// left
		break;
	case XLPR_XRPR:								// x left% && x right%		size changes
		new_rect.right = cx + MulDiv(pa->m_rect.right, cx, m_DlgMinSize.cx);
		new_rect.left = MulDiv(pa->m_rect.left, cx, m_DlgMinSize.cx);
		break;
	case SZPR_XLEQ:								// x left== && size prop
		diff = MulDiv(new_rect.Width(), cx, ref_size.cx); // get size
		new_rect.left = ref_rect.left + pa->m_rect.left;// left position
		new_rect.right = new_rect.left + diff;	// right position
		break;
	case SZPR_XREQ:
		diff = MulDiv(new_rect.Width(), cx, ref_size.cx); // get size
		new_rect.right = ref_rect.right + pa->m_rect.right;	// clip right to the right side
		new_rect.left = new_rect.right - diff;	// left position
		break;
	case RIGHT_BAR:								// clip to the right, outside the area
		new_rect.left = ref_rect.right;			// left= window size
		new_rect.right = new_rect.left + m_RIGHTBARWidth;
		break;
	default:
		break;
	}

	// align along the vertical direction
	switch (pa->GetySizeHow())
	{
	case SZEQ_YTEQ:		// y top== - no action
		diff = new_rect.Height();
		new_rect.top = ref_rect.top + pa->m_rect.top;
		new_rect.bottom = new_rect.top + diff;
		break;
	case SZEQ_YBEQ:		// y bottom==
		diff = new_rect.Height();
		new_rect.bottom = ref_rect.bottom + pa->m_rect.bottom;
		new_rect.top = new_rect.bottom - diff;
		break;
	case YTEQ_YBEQ:		// ytop== && ybottom==
		new_rect.top = ref_rect.top + pa->m_rect.top;
		new_rect.bottom = ref_rect.bottom + pa->m_rect.bottom;
		break;
	case SZEQ_YTPR:		// y top%
		diff = new_rect.Height();
		new_rect.top = ref_rect.top + MulDiv(pa->m_rect.top, cy, ref_size.cy);
		new_rect.bottom = new_rect.top + diff;
		break;
	case SZEQ_YBPR:		// y bottom%
		diff = new_rect.Height();
		new_rect.bottom = ref_rect.bottom + MulDiv(pa->m_rect.bottom, cy, ref_size.cy);
		new_rect.top = new_rect.bottom - diff;
		break;
	case YTPR_YBPR:		// proportional distance from both borders -- size accordingly
		new_rect.top = MulDiv(pa->m_rect.top, cy, m_DlgMinSize.cy);
		new_rect.bottom = cy + MulDiv(pa->m_rect.bottom, cy, m_DlgMinSize.cy);
		break;
	case SZPR_YTEQ:								// y top== && size prop
		if (pa->GetMaster() >= 0)						// if slave, get ref from master
		{
			diff = MulDiv(pa->m_rect0.Height(), ref_rect.Height(), pa->m_rmaster0.Height());
		}
		else
			diff = MulDiv(new_rect.Height(), cy, ref_size.cy); // get size
		new_rect.top = ref_rect.top + pa->m_rect.top;
		new_rect.bottom = new_rect.top + diff;	// right position
		break;
	case SZPR_YBEQ:
		if (pa->GetMaster() >= 0)						// if slave, get ref from master
		{
			diff = MulDiv(pa->m_rect0.Height(), ref_rect.Height(), pa->m_rmaster0.Height());
		}
		else
			diff = MulDiv(new_rect.Height(), cy, ref_size.cy); // get size
		new_rect.bottom = ref_rect.bottom + pa->m_rect.bottom;	// clip right to the right side
		new_rect.top = new_rect.bottom - diff;	// left position
		break;
	case BOTTOM_BAR:
		new_rect.top = ref_rect.bottom;
		new_rect.bottom = new_rect.top + m_BOTTOMBARHeight;
		break;
	default:
		break;
	}
	return new_rect;
}

void CStretchControl::AreaLeftbyControls(int* cx, int* cy)
{
	if (GetbVBarControls())
		*cx -= m_RIGHTBARWidth;
	if (GetbHBarControls())
		*cy -= m_BOTTOMBARHeight;
}

// --------------------------------------------------------------------------
// DisplayVBarControls - assume m_DialogSize minus bar area
// return previous state
// --------------------------------------------------------------------------

BOOL CStretchControl::DisplayVBarControls(BOOL bVisible)
{
	const auto b_prev_state = m_bVBAR;		// previous state of bar
	m_bVBAR = bVisible;	            // set state
	const auto show_flag = (m_bVBAR ? SW_SHOW : SW_HIDE);
	const auto rect_flag = ((m_bHBAR && m_bVBAR) ? SW_SHOW : SW_HIDE);

	// loop through all controls and set visible state
	for (auto i = 0; i < ctrlprop_ptr_array.GetSize(); i++)
	{
		auto pa = ctrlprop_ptr_array.GetAt(i);
		if (pa->GetxSizeHow() == RIGHT_BAR)
		{
			if (pa->GetySizeHow() != BOTTOM_BAR)
				::ShowWindow(pa->GetHWnd(), show_flag);
			else
				::ShowWindow(pa->GetHWnd(), rect_flag);
		}
	}
	UpdateClientControls();
	return b_prev_state;
}

// --------------------------------------------------------------------------
// DisplayHBarControls -- assume m_DialogSize minus bar area
// return previous state
// --------------------------------------------------------------------------

BOOL CStretchControl::DisplayHBarControls(BOOL bVisible)
{
	const auto b_prev_state = m_bHBAR;		// previous state of bar
	m_bHBAR = bVisible;	            // set state
	const auto show_flag = (m_bHBAR ? SW_SHOW : SW_HIDE);
	const auto rect_flag = ((m_bHBAR && m_bVBAR) ? SW_SHOW : SW_HIDE);
	// loop through all controls and set visible state
	for (auto i = 0; i < ctrlprop_ptr_array.GetSize(); i++)
	{
		auto pa = (CCtrlProp*)ctrlprop_ptr_array.GetAt(i);
		if (pa->GetySizeHow() == BOTTOM_BAR)
		{
			if (pa->GetxSizeHow() != RIGHT_BAR)
				::ShowWindow(pa->GetHWnd(), show_flag);
			else
				::ShowWindow(pa->GetHWnd(), rect_flag);
		}
	}
	UpdateClientControls();
	return b_prev_state;
}

void CStretchControl::UpdateClientControls()
{
	RECT client_rect;
	m_parent->GetClientRect(&client_rect);
	// force resizing
	::SendMessage(m_parent->m_hWnd,  //::PostMessage(m_parent->m_hWnd,
		WM_SIZE,
		0,
		MAKELPARAM(client_rect.right, client_rect.bottom));
}