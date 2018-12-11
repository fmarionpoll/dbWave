// CONTROLP.CPP

#include "StdAfx.h"
#include "StretchControls.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CCtrlProp, CObject)

CCtrlProp::CCtrlProp()
{
	m_xSizeHow	= SZEQ_XLEQ;	
	m_ySizeHow	= SZEQ_XLEQ; 
	m_rect		= CRect(0,0,0,0); 
	m_ctrlID	= NULL; 
	m_bVisible	= TRUE;
	m_master	= -1;
	m_hWnd		= nullptr;
	m_slaveorder = 0;
}

CCtrlProp::CCtrlProp(int iID, HWND hC, int xSizeHow, int ySizeHow, CRect* rect)
{
	m_ctrlID	= iID; 
	m_hWnd		= hC; 
	m_xSizeHow	= xSizeHow; 
	m_ySizeHow	= ySizeHow; 
	m_rect		= *rect;
	m_bVisible	= TRUE;
	m_master	= -1;
	m_slaveorder = 0;
}
	
IMPLEMENT_DYNCREATE(CStretchControl, CObject)

//---------------------------------------------------------------------------
// Init Controls properties
//---------------------------------------------------------------------------
CStretchControl::CStretchControl(): m_parent(nullptr), m_DialogSize(), m_DlgMinSize(), m_bHBAR(0), m_bVBAR(0)
{
	m_BOTTOMBARHeight = 0; //::GetSystemMetrics(SM_CYHSCROLL);	// standard height
	m_RIGHTBARWidth = 0; //::GetSystemMetrics(SM_CXHSCROLL);	// std width/scrollbar
	m_slavemax = 0;
}

//---------------------------------------------------------------------------
// Init Controls properties
// newProp
//---------------------------------------------------------------------------
void CStretchControl::AttachParent(CFormView* pF)
{
	m_parent = pF;
	// save pointer to parent
	m_DlgMinSize = pF->GetTotalSize();						// get size of dialog	
	m_DialogSize.cy = m_DlgMinSize.cy - m_BOTTOMBARHeight;	// reduce client area
	m_DialogSize.cx = m_DlgMinSize.cx - m_RIGHTBARWidth;	// by this value
	m_slavemax = 0;
}

void CStretchControl::AttachDialogParent(CDialog* pWnd)
{
	m_parent = pWnd;
	CRect rect;
	pWnd->GetClientRect(rect);
	m_DlgMinSize = CSize(rect.Width(), rect.Height());
	m_DialogSize = m_DlgMinSize;

}

void CStretchControl::ChangeBarSizes(int RIGHTwidth, int BOTTOMheight)
{
	m_BOTTOMBARHeight = RIGHTwidth;		// height/scrollbar
	m_RIGHTBARWidth   = BOTTOMheight;	// width/scrollbar
	m_DialogSize.cy = m_DlgMinSize.cy - m_BOTTOMBARHeight;	// reduce client area
	m_DialogSize.cx = m_DlgMinSize.cx - m_RIGHTBARWidth;	// by this value
}

BOOL CStretchControl::newProp(int iID, int xsizeMode, int ysizeMode)
{
	CWnd* pC = m_parent->GetDlgItem(iID);	
	return newProp(pC, iID, xsizeMode, ysizeMode);
}

BOOL CStretchControl::newProp(CWnd* pWnd, int iID, int xsizeMode, int ysizeMode)
{
	if (m_parent == nullptr)
		return FALSE;

	CRect rect0, rect;
	pWnd->GetWindowRect(&rect);			// get screen window coordinates
	m_parent->ScreenToClient(&rect);	// convert in client coordinates
	rect0 = rect;

	rect.right -= m_DialogSize.cx; 		// distance of right from border
	rect.bottom -= m_DialogSize.cy;		// distance from lower border
	CCtrlProp* ppC = new CCtrlProp(iID, pWnd->m_hWnd, xsizeMode, ysizeMode, &rect);
	ASSERT(ppC != NULL);
	ppC->m_rect0 = rect0;
	ppC->m_rmaster0 = CRect(0, 0, 0, 0);
	ppC->m_slaveorder = 0;

	m_props.Add(ppC);
	return TRUE;
}

// iID stays at a constant offset from the sides of iMASTER
// sizeModes concern iMASTER
// if iMASTER already defined xsizeMaster and ysizeMaster are not used
BOOL CStretchControl::newSlaveProp(int iID, int xsizeSlave, int ysizeSlave, int idMASTER)
{
	if (m_parent == nullptr)
		return FALSE;

	CWnd* pC = m_parent->GetDlgItem(idMASTER);	// get handle of master
	CRect rectmaster, rectmaster0;
	pC->GetWindowRect(&rectmaster);				// get screen window coordinates
	m_parent->ScreenToClient(&rectmaster);		// convert in client coordinates
	rectmaster0 = rectmaster;
	
	// find idMASTER 
	int imaster = -1;							// master index
	CCtrlProp* propmaster= nullptr;					// pointer
	if (m_props.GetUpperBound() >= 0)			// do we have masters?
	{		
		for (int i = 0; i<= m_props.GetUpperBound(); i++) // loop
		{
			propmaster = (CCtrlProp*) m_props.GetAt(i); // get pointer to current master
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
	CRect rect, rect0;
	pC = m_parent->GetDlgItem(iID); 	// get handle of control	
	pC->GetWindowRect(&rect);			// get screen window coordinates
	m_parent->ScreenToClient(&rect);	// convert in client coordinates
	rect0 = rect;
	rect.left 	= rect.left		- rectmaster0.left;
	rect.right 	= rect.right 	- rectmaster0.right;
	rect.top 	= rect.top 		- rectmaster0.top;
	rect.bottom = rect.bottom 	- rectmaster0.bottom;

	CCtrlProp* pb = new CCtrlProp(iID, pC->m_hWnd, xsizeSlave, ysizeSlave, &rect);
	ASSERT(pb != NULL);
	pb->SetMaster(imaster);	
	pb->m_rect0 = rect0;
	pb->m_rmaster0 = rectmaster0;
	pb->m_slaveorder = 1;
	if(propmaster != nullptr)
		pb->m_slaveorder = propmaster->m_slaveorder + 1;

	if (m_slavemax < pb->m_slaveorder)
		m_slavemax = pb->m_slaveorder;

	m_props.Add(pb);
	return TRUE;
}

//---------------------------------------------------------------------------
// Remove Controls properties from mem (props are created with a new)
// RemoveProps
//---------------------------------------------------------------------------
CStretchControl::~CStretchControl()
{
	int i;
	if (m_props.GetUpperBound() >= 0)
	{
		for (i=m_props.GetUpperBound(); i>= 0; i--)
		{
			delete (CCtrlProp*) m_props[i];
			m_props.RemoveAt(i);
		}	
		m_props.RemoveAll();
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
	if (m_props.GetSize() <= 0 || cx <= 0 || cy <= 0)
		return;

	cx = (cx < m_DlgMinSize.cx) ? m_DlgMinSize.cx: cx;	// no action if less than min size
	cy = (cy < m_DlgMinSize.cy) ? m_DlgMinSize.cy: cy;	// no action if less than min size
	AreaLeftbyControls(&cx, &cy);
	if (cx == m_DialogSize.cx && cy == m_DialogSize.cy)
		return;

	// prepare looping through all controls	
	HDWP hDWP = ::BeginDeferWindowPos(m_props.GetSize());
	if (hDWP == nullptr)		
		return;		// exit if none available

	// loop through all windows	
	CCtrlProp* pa;		// pointer to properties
	int i;				// counter

	// first loop through master windows
	for (int j=0; j<= m_slavemax; j++)
	{
		for (i=m_props.GetUpperBound(); i>=0; i--)
		{
			pa = (CCtrlProp*) m_props.GetAt(i);
			if (pa->m_slaveorder != j)
				continue;
			CRect newRect = AlignControl(pa, cx, cy);
			pa->SetMasterRect(newRect);
			hDWP = ::DeferWindowPos(hDWP, pa->GetHWnd(), nullptr, 
				newRect.left, 
				newRect.top, 
				newRect.right - newRect.left, 
				newRect.bottom - newRect.top,
				SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}

	::EndDeferWindowPos(hDWP);

	// update dialog size
	m_DialogSize.cx = cx;
	m_DialogSize.cy = cy;
}

// --------------------------------------------------------------------------
// AlignControls()
//	i = index for array containing IDs && resizing flags
//	cx, cy = new coordinates of window
//	hDWP = handle for deferwindowpos
// --------------------------------------------------------------------------

CRect CStretchControl::AlignControl(CCtrlProp* pa, int cx, int cy)
{
	HWND hwndChild = pa->GetHWnd();					// hwndChild = window to move
	// compute reference rectangle either from dialog or reference rect
	CRect refRect;									// reference rectangle
	CRect newRect;									// control's rect

	::GetWindowRect(hwndChild, &newRect);			// get window coordinates of the window before it has moved
	m_parent->ScreenToClient(&newRect);				// convert into client coordinates
	CSize refSize = m_DialogSize;

	if (pa->GetMaster() >= 0)						// if slave, get ref from master
	{
		CCtrlProp* pb = (CCtrlProp*) m_props.GetAt(pa->GetMaster());
		refRect = pb->GetMasterRect();				// load already transformed rect
		cx = refRect.Width();						// get width
		cy = refRect.Height();						// and height
		refSize = CSize (cx, cy);
	}
	// set ref rectangle from dialog box
	else
	{
		refRect.left = refRect.top = 0;				// otherwise fill refRect with
		refRect.right = cx;							// dialog coordinates
		refRect.bottom = cy;
	}	

	// change size according to the flags associated with x and y directions
	
	// align along the horizontal direction	
	int diff;										// volatile variable
	switch (pa->GetxSizeHow())
	{
	case SZEQ_XLEQ:								// x left== - no action    	
		diff = newRect.Width();					// size of the control (fixed)
		newRect.left = refRect.left + pa->m_rect.left;
		newRect.right = newRect.left + diff;
		break;
	case SZEQ_XREQ:								// x right== && same size
		diff = newRect.Width();					// size of the control (fixed)
		newRect.right = refRect.right + pa->m_rect.right; // offset from right side
		newRect.left  = newRect.right - diff;	// -> left
		break;
	case XLEQ_XREQ:								// x left== && x right== (change size)
		newRect.left = refRect.left + pa->m_rect.left;   // constant offset from left side
		newRect.right = refRect.right + pa->m_rect.right;// constant offset from right side
		break;
	case SZEQ_XLPR:								// x left%  && same size: ?? wrong with slaves
		diff = newRect.Width();					// size of the control (constant)
		newRect.left = refRect.left + MulDiv(pa->m_rect.left, cx, refSize.cx);
		newRect.right = newRect.left + diff;	// -> left
		break;
	case SZEQ_XRPR:								// x right%	&& same size
		diff = newRect.Width();					// get size
		newRect.right = refRect.right + MulDiv(pa->m_rect.right, cx, refSize.cx);
		newRect.left = newRect.right - diff;	// left
		break;
	case XLPR_XRPR:								// x left% && x right%		size changes
		newRect.right = cx+ MulDiv(pa->m_rect.right, cx, m_DlgMinSize.cx);
		newRect.left = MulDiv(pa->m_rect.left, cx, m_DlgMinSize.cx);
		break;
	case SZPR_XLEQ:								// x left== && size prop
		diff = MulDiv(newRect.Width(), cx, refSize.cx); // get size
		newRect.left = refRect.left + pa->m_rect.left;// left position
		newRect.right = newRect.left + diff;	// right position
		break;
	case SZPR_XREQ:
		diff = MulDiv(newRect.Width(), cx, refSize.cx); // get size
		newRect.right = refRect.right + pa->m_rect.right;	// clip right to the right side
		newRect.left = newRect.right - diff;	// left position			
		break;
	case RIGHT_BAR:								// clip to the right, outside the area
		newRect.left = refRect.right;			// left= window size
		newRect.right = newRect.left+m_RIGHTBARWidth;
		break;
	default:
		break;
	}

	// align along the vertical direction		
	switch (pa->GetySizeHow())
	{   
	case SZEQ_YTEQ:		// y top== - no action
		diff = newRect.Height();
		newRect.top = refRect.top + pa->m_rect.top;
		newRect.bottom = newRect.top + diff;
		break;
	case SZEQ_YBEQ:		// y bottom==
		diff = newRect.Height();
		newRect.bottom = refRect.bottom + pa->m_rect.bottom;
		newRect.top  = newRect.bottom - diff;
		break;
	case YTEQ_YBEQ:		// ytop== && ybottom==
		newRect.top = refRect.top + pa->m_rect.top;
		newRect.bottom = refRect.bottom + pa->m_rect.bottom;
		break;
	case SZEQ_YTPR:		// y top%
		diff = newRect.Height();
		newRect.top = refRect.top + MulDiv(pa->m_rect.top, cy, refSize.cy);
		newRect.bottom= newRect.top + diff;			
		break;			
	case SZEQ_YBPR:		// y bottom%
		diff = newRect.Height();
		newRect.bottom = refRect.bottom + MulDiv(pa->m_rect.bottom, cy, refSize.cy);
		newRect.top = newRect.bottom - diff;
		break;
	case YTPR_YBPR:		// proportional distance from both borders -- size accordingly		
		newRect.top = MulDiv(pa->m_rect.top, cy, m_DlgMinSize.cy);
		newRect.bottom = cy + MulDiv(pa->m_rect.bottom, cy, m_DlgMinSize.cy);
		break;
	case SZPR_YTEQ:								// y top== && size prop
		if (pa->GetMaster() >= 0)						// if slave, get ref from master
		{
			diff = MulDiv(pa->m_rect0.Height(), refRect.Height(), pa->m_rmaster0.Height());
		}
		else
			diff = MulDiv(newRect.Height(), cy, refSize.cy); // get size
		newRect.top = refRect.top + pa->m_rect.top;
		newRect.bottom = newRect.top + diff;	// right position
		break;
	case SZPR_YBEQ:
		if (pa->GetMaster() >= 0)						// if slave, get ref from master
		{
			diff = MulDiv(pa->m_rect0.Height(), refRect.Height(), pa->m_rmaster0.Height());
		}
		else
			diff = MulDiv(newRect.Height(), cy, refSize.cy); // get size
		newRect.bottom = refRect.bottom + pa->m_rect.bottom;	// clip right to the right side
		newRect.top = newRect.bottom - diff;	// left position
		break;
	case BOTTOM_BAR:
		newRect.top = refRect.bottom;
		newRect.bottom = newRect.top + m_BOTTOMBARHeight;
		break;
	default:
		break;		
	}	
	return newRect;
}

void CStretchControl::AreaLeftbyControls(int* cx, int *cy)
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
	BOOL bPrevState = m_bVBAR;		// previous state of bar
	m_bVBAR = bVisible;	            // set state
	int showFlag = (m_bVBAR ? SW_SHOW: SW_HIDE);
	int rectFlag = ((m_bHBAR && m_bVBAR) ? SW_SHOW: SW_HIDE);
	CCtrlProp* pa;
	// loop through all controls and set visible state
	for (int i=0; i<m_props.GetSize(); i++)
		{
		pa = (CCtrlProp*) m_props.GetAt(i);
		if (pa->GetxSizeHow() == RIGHT_BAR)
			{
			if (pa->GetySizeHow() != BOTTOM_BAR)
				::ShowWindow(pa->GetHWnd(), showFlag);
			else
				::ShowWindow(pa->GetHWnd(), rectFlag);
			}
		}	
	UpdateClientControls();
	return bPrevState;
}

// --------------------------------------------------------------------------
// DisplayHBarControls -- assume m_DialogSize minus bar area
// return previous state
// --------------------------------------------------------------------------

BOOL CStretchControl::DisplayHBarControls(BOOL bVisible)
{	
	BOOL bPrevState = m_bHBAR;		// previous state of bar
	m_bHBAR = bVisible;	            // set state
	int showFlag = (m_bHBAR ? SW_SHOW: SW_HIDE);
	int rectFlag = ((m_bHBAR && m_bVBAR) ? SW_SHOW: SW_HIDE);
	CCtrlProp* pa;
	// loop through all controls and set visible state
	for (int i=0; i<m_props.GetSize(); i++)
		{
		pa = (CCtrlProp*) m_props.GetAt(i);
		if (pa->GetySizeHow() == BOTTOM_BAR)
			{
			if (pa->GetxSizeHow() != RIGHT_BAR)
				::ShowWindow(pa->GetHWnd(), showFlag);
			else
				::ShowWindow(pa->GetHWnd(), rectFlag);
			}
		}	
	UpdateClientControls();
	return bPrevState;
}

void CStretchControl::UpdateClientControls()
{
	RECT clientRect;
	m_parent->GetClientRect(&clientRect);
	// force resizing	
	::SendMessage(m_parent->m_hWnd,  //::PostMessage(m_parent->m_hWnd, 
			WM_SIZE, 
			0, 
			MAKELPARAM(clientRect.right, clientRect.bottom));
}
