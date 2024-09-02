// GridCtrl.cpp : implementation file
//
// MFC Grid Control v2.27
//
// Written by Chris Maunder <chris@codeproject.com>
// Copyright (c) 1998-2005. All Rights Reserved.
//
// The code contained in this file was based on the original
// WorldCom Grid control written by Joe Willcoxson,
//        mailto:chinajoe@aol.com
//        http://users.aol.com/chinajoe
// (These addresses may be out of date) The code has gone through
// so many modifications that I'm not sure if there is even a single
// original line of code. In any case Joe's code was a great
// framework on which to build.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name and all copyright
// notices remains intact.
//
// An email letting me know how you are using it would be nice as well.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// Expect bugs!
//
// Please use and enjoy, and let me know of any bugs/mods/improvements
// that you have found/implemented and I will fix/incorporate them into
// this file.
//
//  History:
//  --------
//  This control is constantly evolving, sometimes due to new features that I
//  feel are necessary, and sometimes due to existing bugs. Where possible I
//  have credited the changes to those who contributed code corrections or
//  enhancements (names in brackets) or code suggestions (suggested by...)
//
//          1.0 - 1.13   20 Feb 1998 - 6 May 1999
//                          First release version. Progressed from being a basic
//                          grid based on the original WorldCom Grid control
//                          written by Joe Willcoxson (mailto:chinajoe@aol.com,
//                          http://users.aol.com/chinajoe) to something a little
//                          more feature rich. Rewritten so many times I doubt
//                          there is a single line of Joe's code left. Many, many,
//                          MANY people sent in bug reports and fixes. Thank you
//                          all.
//
//          2.0         1 Feb 2000
//                          Rewritten to make the grid more object oriented, in
//                          that the GridCell class now takes care of cell-specific
//                          tasks. This makes the code more robust, but more
//                          importantly it allows the simple insertion of other
//                          types of cells.
//
//          2.10       11 Mar 2000 - Ken Bertelson and Chris Maunder
//                          - Additions for virtual GridCell support of embedded tree
//                            & cell buttons implementation
//                          - Optional WYSIWYG printing
//                          - Awareness of hidden (0 width/height) rows and columns for
//                            key movements,  cut, copy, paste, and autosizing
//                          - GridCell can make title tips display any text rather than
//                            cell text only
//                          - Minor vis bug fixes
//                          - GridCtrl now works with GridCellBase instead of GridCell
//                            This is a taste of things to come.
//
//          2.20       30 Jul 2000 - Chris Maunder
//                          - Font storage optimised (suggested by Martin Richter)
//                          - AutoSizeColumn works on either column header, data or both
//                          - EnsureVisible. The saga continues... (Ken)
//                          - Rewrote exception handling
//                          - Added TrackFocusCell and FrameFocusCell properties, as well as
//                            ExpandLastColumn (suggested by Bruce E. Stemplewski).
//                          - InsertColumn now allows you to insert columns at the end of the
//                            column range (David Weibel)
//                          - Shift-cell-selection more intuitive
//                          - API change: Set/GetGridColor now Set/GetGridLineColor
//                          - API change: Set/GetBkColor now Set/GetGridBkColor
//                          - API change: Set/GetTextColor, Set/GetTextBkColor depricated
//                          - API change: Set/GetFixedTextColor, Set/GetFixedBkColor depricated
//                          - Stupid DDX_GridControl workaround removed.
//                          - Added "virtual mode" via Set/GetVirtualMode
//                          - Added SetCallbackFunc to allow callback functions in virtual mode
//                          - Added Set/GetAutoSizeStyle
//                          - AutoSize() bug fixed
//                          - added GVIS_FIXEDROW, GVIS_FIXEDCOL states
//                          - added Get/SetFixed[Row|Column]Selection
//                          - cell "Get" methods now const'd. Sorry folks...
//                          - GetMouseScrollLines now uses win98/W2K friendly code
//                          - WS_EX_CLIENTEDGE style now implicit
//
//                          [ Only the latest version and major version changes will be shown ]
////
//          2.25       13 Mar 2004 - Chris Maunder
//                                 - Minor changes so it will compile in VS.NET (inc. Whidbey)
//                                 - Fixed minor bug in EnsureVisible - Junlin Xu
//                                 - Changed AfxGetInstanceHandle for AfxGetResourceHandle in RegisterWindowClass
//					               - Various changes thanks to Yogurt
//
//          2.26       13 Dec 2005 - Pierre Couderc
//                                 - Added sort in Virtual mode
//                                 - Change row/column order programatically or via drag and drop
//                                 - Added save/restore layer (for undoing row/column order changes)
//
// TODO:   1) Implement sparse grids (super easy now)
//         2) Fix it so that as you drag select, the speed of selection increases
//            with time.
//         3) Scrolling is still a little dodgy (too much grey area). I know there
//            is a simple fix but it's been a low priority
//
// ISSUES: 1) WindowFromPoint seems to do weird things in W2K. Causing problems for
//            the right-click-on-title tip code.
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GridCtrl.h"
#include <algorithm>

// OLE stuff for clipboard operations
#include <afxadv.h>            // For CSharedFile

#include "../ColorNames.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Spit out some messages as a sanity check for programmers
#ifdef GRIDCONTROL_NO_TITLETIPS
#pragma message(" -- GridCtrl: No titletips for cells with large data")
#endif
#ifdef GRIDCONTROL_NO_DRAGDROP
#pragma message(" -- GridCtrl: No OLE drag and drop")
#endif
#ifdef GRIDCONTROL_NO_CLIPBOARD
#pragma message(" -- GridCtrl: No clipboard support")
#endif
#ifdef GRIDCONTROL_NO_PRINTING
#pragma message(" -- GridCtrl: No printing support")
#endif

IMPLEMENT_DYNCREATE(GridCtrl, CWnd)

// Get the number of lines to scroll with each mouse wheel notch
// Why doesn't windows give us this function???
UINT GetMouseScrollLines()
{
	int nScrollLines = 3; // reasonable default
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	return nScrollLines;
}


GridCtrl::GridCtrl(int nRows, int nCols, int nFixedRows, int nFixedCols)
{
	RegisterWindowClass();

#if !defined(GRIDCONTROL_NO_DRAGDROP) || !defined(GRIDCONTROL_NO_CLIPBOARD)
	const _AFX_THREAD_STATE* pState = AfxGetThreadState();
	if (!pState->m_bNeedTerm && !AfxOleInit())
		AfxMessageBox(_T("OLE initialization failed. Make sure that the OLE libraries are the correct version"));
#endif

	// Store the system colours in case they change. The gridctrl uses
	// these colours, and in OnSysColorChange we can check to see if
	// the gridctrl colours have been changed from the system colours.
	// If they have, then leave them, otherwise change them to reflect
	// the new system colours.
	m_crWindowText = GetSysColor(COLOR_WINDOWTEXT);
	m_crWindowColour = GetSysColor(COLOR_WINDOW);
	m_cr3DFace = GetSysColor(COLOR_3DFACE);
	m_crShadow = GetSysColor(COLOR_3DSHADOW);
	m_crGridLineColour = col_silver;

	m_nRows = 0;
	m_nCols = 0;
	m_nFixedRows = 0;
	m_nFixedCols = 0;
	m_InDestructor = false;

	m_bVirtualMode = FALSE;
	m_pfnCallback = nullptr;

	m_nVScrollMax = 0; // Scroll position
	m_nHScrollMax = 0;
	m_nRowsPerWheelNotch = GetMouseScrollLines(); // Get the number of lines
	// per mouse wheel notch to scroll
	m_nBarState = GVL_NONE;
	m_MouseMode = MOUSE_NOTHING;
	m_nGridLines = GVL_BOTH;
	m_bEditable = TRUE;
	m_bListMode = FALSE;
	m_bSingleRowSelection = FALSE;
	m_bSingleColSelection = FALSE;
	m_bLMouseButtonDown = FALSE;
	m_bRMouseButtonDown = FALSE;
	m_bAllowDraw = TRUE; // allow draw updates
	m_bEnableSelection = TRUE;
	m_bFixedColumnSelection = TRUE;
	m_bFixedRowSelection = TRUE;
	m_bAllowRowResize = TRUE;
	m_bAllowColumnResize = TRUE;
	m_bSortOnClick = FALSE; // Sort on header row click
	m_bHandleTabKey = TRUE;
#ifdef _WIN32_WCE
	m_bDoubleBuffer = FALSE;      // Use double buffering to avoid flicker?
#else
	m_bDoubleBuffer = TRUE; // Use double buffering to avoid flicker?
#endif
	m_bTitleTips = TRUE; // show cell title tips

	m_bWysiwygPrinting = FALSE; // use size-to-width printing

	m_bHiddenColUnhide = TRUE; // 0-width columns can be expanded via mouse
	m_bHiddenRowUnhide = TRUE; // 0-GetRectHeight rows can be expanded via mouse

	m_bAllowColHide = TRUE; // Columns can be contracted to 0-width via mouse
	m_bAllowRowHide = TRUE; // Rows can be contracted to 0-height via mouse

	m_bAscending = TRUE; // sorting stuff
	m_nSortColumn = -1;
	m_pfnCompare = nullptr;
	m_pfnVirtualCompare = nullptr;
	m_nAutoSizeColumnStyle = GVS_BOTH; // Autosize grid using header and data info

	m_nTimerID = 0; // For drag-selection
	m_nTimerInterval = 25; // (in milliseconds)
	m_nResizeCaptureRange = 3; // When resizing columns/row, the cursor has to be
	// within +/-3 pixels of the dividing line for
	// resizing to be possible
	m_pImageList = nullptr; // Images in the grid
	m_bAllowDragAndDrop = FALSE; // for drag and drop - EFW - off by default
	m_bTrackFocusCell = TRUE; // Track Focus cell?
	m_bFrameFocus = TRUE; // Frame the selected cell?
	m_AllowReorderColumn = false;
	m_QuitFocusOnTab = false;
	m_AllowSelectRowInFixedCol = false;
	m_bDragRowMode = TRUE; // allow to drop a line over another one to change row order
	m_pRtcDefault = RUNTIME_CLASS(GridCell);
	m_lParam = 0;

	SetupDefaultCells();
	SetGridBkColor(m_crShadow);

	// Set up the initial grid size
	SetRowCount(nRows);
	SetColumnCount(nCols);
	SetFixedRowCount(nFixedRows);
	SetFixedColumnCount(nFixedCols);

	SetTitleTipTextClr(CLR_DEFAULT); //FNA
	SetTitleTipBackClr(CLR_DEFAULT);

	// set initial selection range (ie. none)
	m_SelectedCellMap.RemoveAll();
	m_PrevSelectedCellMap.RemoveAll();

#if !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)
	// EFW - Added to support shaded/unshaded printout and
	// user-definable margins.
	m_bShadedPrintOut = TRUE;
	SetPrintMarginInfo(2, 2, 4, 4, 1, 1, 1);
#endif
}

GridCtrl::~GridCtrl()
{
	m_InDestructor = true;
	DeleteAllItems();

#ifndef GRIDCONTROL_NO_TITLETIPS
	if (m_bTitleTips && IsWindow(m_TitleTip.GetSafeHwnd()))
		m_TitleTip.DestroyWindow();
#endif

	DestroyWindow();

#if !defined(GRIDCONTROL_NO_DRAGDROP) || !defined(GRIDCONTROL_NO_CLIPBOARD)
	// BUG FIX - EFW
	const COleDataSource* p_source = COleDataSource::GetClipboardOwner();
	if (p_source)
		COleDataSource::FlushClipboard();
#endif
}

// Register the window class if it has not already been registered.
BOOL GridCtrl::RegisterWindowClass()
{
	WNDCLASS wndcls;
	//HINSTANCE hInst = AfxGetInstanceHandle();
	const HINSTANCE hInst = AfxGetResourceHandle();

	if (!(::GetClassInfo(hInst, GRIDCTRL_CLASSNAME, &wndcls)))
	{
		// otherwise we need to register a new class
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = nullptr;
#ifndef _WIN32_WCE_NO_CURSOR
		wndcls.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
#else
		wndcls.hCursor = 0;
#endif
		wndcls.hbrBackground = reinterpret_cast<HBRUSH>((COLOR_3DFACE + 1));
		wndcls.lpszMenuName = nullptr;
		wndcls.lpszClassName = GRIDCTRL_CLASSNAME;

		if (!AfxRegisterClass(&wndcls))
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}

BOOL GridCtrl::Initialize()
{
	// Stop re-entry problems
	static BOOL bInProcedure = FALSE;
	if (bInProcedure)
		return FALSE;
	bInProcedure = TRUE;

#ifndef GRIDCONTROL_NO_TITLETIPS
	m_TitleTip.SetParentWnd(this);
#endif

	// This would be a good place to register the droptarget but
	// unfortunately this causes problems if you are using the
	// grid in a view.
	// Moved from OnSize.
	//#ifndef GRIDCONTROL_NO_DRAGDROP
	//    m_DropTarget.Register(this);
	//#endif

	if (IsWindow(m_hWnd))
		ModifyStyleEx(0, WS_EX_CLIENTEDGE);

	//// Kludge: Make sure the client edge shows
	//// This is so horrible it makes my eyes water.
	//CRect rect;
	//GetWindowRect(rect);
	//CWnd* p_parent = GetParent();
	//if (p_parent != NULL)
	//	p_parent->ScreenToClient(rect);
	//rect.InflateRect(1,1);	MoveWindow(rect);
	//rect.DeflateRect(1,1);  MoveWindow(rect);
	// SetWindowPos(...) needs at least Windows 2000.
	SetWindowPos(nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	bInProcedure = FALSE;
	return TRUE;
}

// creates the control - use like any other window create control
BOOL GridCtrl::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dw_style)
{
	ASSERT(pParentWnd->GetSafeHwnd());

	if (!CWnd::Create(GRIDCTRL_CLASSNAME, nullptr, dw_style, rect, pParentWnd, nID))
		return FALSE;

	//Initialise(); - called in PreSubclassWnd

	// The number of rows and columns will only be non-zero if the constructor
	// was called with non-zero initialising parameters. If this window was created
	// using a dialog template then the number of rows and columns will be 0 (which
	// means that the code below will not be needed - which is lucky 'cause it ain't
	// gonna get called in a dialog-template-type-situation.

	TRY
		{
			m_arRowHeights.SetSize(m_nRows); // initialize row heights
			m_arColWidths.SetSize(m_nCols); // initialize column widths
		}
	CATCH(CMemoryException, e)
		{
			e->ReportError();
			return FALSE;
		}
	END_CATCH

	int i;
	for (i = 0; i < m_nRows; i++)
		m_arRowHeights[i] = m_cellDefault.GetHeight();
	for (i = 0; i < m_nCols; i++)
		m_arColWidths[i] = m_cellDefault.GetWidth();

	return TRUE;
}

void GridCtrl::SetupDefaultCells()
{
	m_cellDefault.SetGrid(this); // Normal editable cell
	m_cellFixedColDef.SetGrid(this); // Cell for fixed columns
	m_cellFixedRowDef.SetGrid(this); // Cell for fixed rows
	m_cellFixedRowColDef.SetGrid(this); // Cell for area overlapped by fixed columns/rows

	m_cellDefault.SetTextClr(m_crWindowText);
	m_cellDefault.SetBackClr(m_crWindowColour);
	m_cellFixedColDef.SetTextClr(m_crWindowText);
	m_cellFixedColDef.SetBackClr(m_cr3DFace);
	m_cellFixedRowDef.SetTextClr(m_crWindowText);
	m_cellFixedRowDef.SetBackClr(m_cr3DFace);
	m_cellFixedRowColDef.SetTextClr(m_crWindowText);
	m_cellFixedRowColDef.SetBackClr(m_cr3DFace);
}

void GridCtrl::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	//HFONT hFont = ::CreateFontIndirect(m_cellDefault.GetFont());
	//OnSetFont((LPARAM)hFont, 0);
	//DeleteObject(hFont);

	Initialize();
}

// Sends a message to the parent in the form of a WM_NOTIFY message with
// a NM_GRIDVIEW structure attached
LRESULT GridCtrl::SendMessageToParent(int nRow, int nCol, int nMessage) const
{
	if (!IsWindow(m_hWnd))
		return 0;

	NM_GRIDVIEW nmgv;
	nmgv.iRow = nRow;
	nmgv.iColumn = nCol;
	nmgv.hdr.hwndFrom = m_hWnd;
	nmgv.hdr.idFrom = GetDlgCtrlID();
	nmgv.hdr.code = nMessage;

	const CWnd* pOwner = GetOwner();
	if (pOwner && IsWindow(pOwner->m_hWnd))
		return pOwner->SendMessage(WM_NOTIFY, nmgv.hdr.idFrom, reinterpret_cast<LPARAM>(&nmgv));
	return 0;
}

// Send a request to the parent to return information on a given cell
LRESULT GridCtrl::SendDisplayRequestToParent(GV_DISPINFO* pDisplayInfo) const
{
	if (!IsWindow(m_hWnd))
		return 0;

	// Fix up the message headers
	pDisplayInfo->hdr.hwndFrom = m_hWnd;
	pDisplayInfo->hdr.idFrom = GetDlgCtrlID();
	pDisplayInfo->hdr.code = GVN_GETDISPINFO;

	// Send the message
	const CWnd* pOwner = GetOwner();
	if (pOwner && IsWindow(pOwner->m_hWnd))
		return pOwner->SendMessage(WM_NOTIFY, pDisplayInfo->hdr.idFrom, reinterpret_cast<LPARAM>(pDisplayInfo));
	return 0;
}

// Send a hint to the parent about caching information
LRESULT GridCtrl::SendCacheHintToParent(const GridCellRange& range) const
{
	if (!IsWindow(m_hWnd))
		return 0;

	GV_CACHEHINT CacheHint;

	// Fix up the message headers
	CacheHint.hdr.hwndFrom = m_hWnd;
	CacheHint.hdr.idFrom = GetDlgCtrlID();
	CacheHint.hdr.code = GVN_ODCACHEHINT;

	CacheHint.range = range;

	// Send the message
	const CWnd* pOwner = GetOwner();
	if (pOwner && IsWindow(pOwner->m_hWnd))
		return pOwner->SendMessage(WM_NOTIFY, CacheHint.hdr.idFrom, reinterpret_cast<LPARAM>(&CacheHint));
	return 0;
}

/*
#define LAYER_SIGNATURE (0x5FD4E64)
int GridCtrl::GetLayer(int** pLayer) // used to save and restore order of columns
{ //  gives back the size of the area (do not forget to delete pLayer)
	int Length = (1 + GetColumnCount()) * sizeof(int);
	//int Length = 2 + GetColumnCount() * 2; 
	int* Layer = new int[Length];	// the caller is supposed to delete it
	Layer[0] = LAYER_SIGNATURE;
	Layer[1] = GetColumnCount();
	memcpy(&Layer[2], &m_arColOrder[0], GetColumnCount() * sizeof(int));
	memcpy(&Layer[2 + GetColumnCount()], &m_arColWidths[0], GetColumnCount() * sizeof(int));
	*pLayer = Layer;
	return Length;
}
void GridCtrl::SetLayer(int* pLayer)
{ // coming from a previous GetLayer (ignored if not same number of column, or the same revision number)
	if (pLayer[0] != LAYER_SIGNATURE) return;
	if (pLayer[1] != GetColumnCount()) return;
	memcpy(&m_arColOrder[0], &pLayer[2], GetColumnCount() * sizeof(int));
	memcpy(&m_arColWidths[0], &pLayer[2 + GetColumnCount()], GetColumnCount() * sizeof(int));
}
*/

BEGIN_MESSAGE_MAP(GridCtrl, CWnd)
	//EFW - Added ON_WM_RBUTTONUP
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_WM_SYSKEYDOWN()
#ifndef _WIN32_WCE_NO_CURSOR
	ON_WM_SETCURSOR()
#endif
#ifndef _WIN32_WCE
	ON_WM_RBUTTONUP()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CAPTURECHANGED()
#endif
#ifndef GRIDCONTROL_NO_CLIPBOARD
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
#endif
#if (_WIN32_WCE >= 210)
	ON_WM_SETTINGCHANGE()
#endif
#if !defined(_WIN32_WCE) && (_MFC_VER >= 0x0421)
	ON_WM_MOUSEWHEEL()
#endif
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE(WM_IME_CHAR, OnImeChar)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_INPLACE_CONTROL, OnEndInPlaceEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GridCtrl message handlers

void GridCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (m_bDoubleBuffer) // Use a memory DC to remove flicker
	{
		// CMemDC from afxcontrolbarutil.h
		CMemDC memDC(dc, this);
		CDC* p_dc = &memDC.GetDC();
		OnDraw(p_dc);
	}
	else // Draw raw - this helps in debugging vis problems.
		OnDraw(&dc);
}

BOOL GridCtrl::OnEraseBkgnd(CDC* /*p_dc*/)
{
	return TRUE; // Don't erase the background.
}

// Custom background erasure. This gets called from within the OnDraw function,
// since we will (most likely) be using a memory DC to stop flicker. If we just
// erase the background normally through OnEraseBkgnd, and didn't fill the memDC's
// selected bitmap with colour, then all sorts of vis problems would occur
void GridCtrl::EraseBkgnd(CDC* p_dc)
{
	CRect VisRect, ClipRect, rect;
	CBrush FixedRowColBack(GetDefaultCell(TRUE, TRUE)->GetBackClr()),
	       FixedRowBack(GetDefaultCell(TRUE, FALSE)->GetBackClr()),
	       FixedColBack(GetDefaultCell(FALSE, TRUE)->GetBackClr()),
	       TextBack(GetDefaultCell(FALSE, FALSE)->GetBackClr());
	CBrush Back(GetGridBkColor());
	//CBrush Back(GetTextBkColor());

	if (p_dc->GetClipBox(ClipRect) == ERROR)
		return;
	GetVisibleNonFixedCellRange(VisRect);

	const int nFixedColumnWidth = GetFixedColumnWidth();
	const int nFixedRowHeight = GetFixedRowHeight();

	// Draw Fixed row/column background
	if (ClipRect.left < nFixedColumnWidth && ClipRect.top < nFixedRowHeight)
		p_dc->FillRect(CRect(ClipRect.left, ClipRect.top,
		                     nFixedColumnWidth, nFixedRowHeight),
		               &FixedRowColBack);

	// Draw Fixed columns background
	if (ClipRect.left < nFixedColumnWidth && ClipRect.top < VisRect.bottom)
		p_dc->FillRect(CRect(ClipRect.left, ClipRect.top,
		                     nFixedColumnWidth, VisRect.bottom),
		               &FixedColBack);

	// Draw Fixed rows background
	if (ClipRect.top < nFixedRowHeight &&
		ClipRect.right > nFixedColumnWidth && ClipRect.left < VisRect.right)
		p_dc->FillRect(CRect(nFixedColumnWidth - 1, ClipRect.top,
		                     VisRect.right, nFixedRowHeight),
		               &FixedRowBack);

	// Draw non-fixed cell background
	if (rect.IntersectRect(VisRect, ClipRect))
	{
		CRect CellRect(__max(nFixedColumnWidth, rect.left),
		               __max(nFixedRowHeight, rect.top),
		               rect.right, rect.bottom);
		p_dc->FillRect(CellRect, &TextBack);
	}

	// Draw right hand side of window outside grid
	if (VisRect.right < ClipRect.right)
		p_dc->FillRect(CRect(VisRect.right, ClipRect.top,
		                     ClipRect.right, ClipRect.bottom),
		               &Back);

	// Draw bottom of window below grid
	if (VisRect.bottom < ClipRect.bottom && ClipRect.left < VisRect.right)
		p_dc->FillRect(CRect(ClipRect.left, VisRect.bottom,
		                     VisRect.right, ClipRect.bottom),
		               &Back);
}

void GridCtrl::OnSize(UINT n_type, int cx, int cy)
{
	static BOOL bAlreadyInsideThisProcedure = FALSE;
	if (bAlreadyInsideThisProcedure)
		return;

	if (!IsWindow(m_hWnd))
		return;

	// This is not the ideal place to register the droptarget
#ifndef GRIDCONTROL_NO_DRAGDROP
	m_DropTarget.Register(this);
#endif

	// Start re-entry blocking
	bAlreadyInsideThisProcedure = TRUE;

	EndEditing(); // destroy any InPlaceEdit's
	CWnd::OnSize(n_type, cx, cy);
	ResetScrollBars();

	// End re-entry blocking
	bAlreadyInsideThisProcedure = FALSE;
}

UINT GridCtrl::OnGetDlgCode()
{
	UINT nCode = DLGC_WANTARROWS | DLGC_WANTCHARS; // DLGC_WANTALLKEYS; //

	if (m_bHandleTabKey && !IsCTRLpressed())
		nCode |= DLGC_WANTTAB;

	return nCode;
}

#ifndef _WIN32_WCE
// If system colours change, then redo colours
void GridCtrl::OnSysColorChange()
{
	CWnd::OnSysColorChange();

	if (GetDefaultCell(FALSE, FALSE)->GetTextClr() == m_crWindowText) // Still using system colours
		GetDefaultCell(FALSE, FALSE)->SetTextClr(GetSysColor(COLOR_WINDOWTEXT)); // set to new system colour
	if (GetDefaultCell(FALSE, FALSE)->GetBackClr() == m_crWindowColour)
		GetDefaultCell(FALSE, FALSE)->SetBackClr(GetSysColor(COLOR_WINDOW));

	if (GetDefaultCell(TRUE, FALSE)->GetTextClr() == m_crWindowText) // Still using system colours
		GetDefaultCell(TRUE, FALSE)->SetTextClr(GetSysColor(COLOR_WINDOWTEXT)); // set to new system colour
	if (GetDefaultCell(TRUE, FALSE)->GetBackClr() == m_crWindowColour)
		GetDefaultCell(TRUE, FALSE)->SetBackClr(GetSysColor(COLOR_WINDOW));

	if (GetDefaultCell(FALSE, TRUE)->GetTextClr() == m_crWindowText) // Still using system colours
		GetDefaultCell(FALSE, TRUE)->SetTextClr(GetSysColor(COLOR_WINDOWTEXT)); // set to new system colour
	if (GetDefaultCell(FALSE, TRUE)->GetBackClr() == m_crWindowColour)
		GetDefaultCell(FALSE, TRUE)->SetBackClr(GetSysColor(COLOR_WINDOW));

	if (GetDefaultCell(TRUE, TRUE)->GetTextClr() == m_crWindowText) // Still using system colours
		GetDefaultCell(TRUE, TRUE)->SetTextClr(GetSysColor(COLOR_WINDOWTEXT)); // set to new system colour
	if (GetDefaultCell(TRUE, TRUE)->GetBackClr() == m_crWindowColour)
		GetDefaultCell(TRUE, TRUE)->SetBackClr(GetSysColor(COLOR_WINDOW));

	if (GetGridBkColor() == m_crShadow)
		SetGridBkColor(GetSysColor(COLOR_3DSHADOW));

	m_crWindowText = GetSysColor(COLOR_WINDOWTEXT);
	m_crWindowColour = GetSysColor(COLOR_WINDOW);
	m_cr3DFace = GetSysColor(COLOR_3DFACE);
	m_crShadow = GetSysColor(COLOR_3DSHADOW);
}
#endif

#ifndef _WIN32_WCE_NO_CURSOR
// If we are drag-selecting cells, or drag and dropping, stop now
void GridCtrl::OnCaptureChanged(CWnd* p_wnd)
{
	if (p_wnd->GetSafeHwnd() == GetSafeHwnd())
		return;

	// kill timer if active
	if (m_nTimerID != 0)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}

#ifndef GRIDCONTROL_NO_DRAGDROP
	// Kill drag and drop if active
	if (m_MouseMode == MOUSE_DRAGGING)
		m_MouseMode = MOUSE_NOTHING;
#endif
}
#endif

#if (_MFC_VER >= 0x0421) || (_WIN32_WCE >= 210)
// If system settings change, then redo colours
void GridCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CWnd::OnSettingChange(uFlags, lpszSection);

	if (GetDefaultCell(FALSE, FALSE)->GetTextClr() == m_crWindowText) // Still using system colours
		GetDefaultCell(FALSE, FALSE)->SetTextClr(GetSysColor(COLOR_WINDOWTEXT)); // set to new system colour
	if (GetDefaultCell(FALSE, FALSE)->GetBackClr() == m_crWindowColour)
		GetDefaultCell(FALSE, FALSE)->SetBackClr(GetSysColor(COLOR_WINDOW));

	if (GetDefaultCell(TRUE, FALSE)->GetTextClr() == m_crWindowText) // Still using system colours
		GetDefaultCell(TRUE, FALSE)->SetTextClr(GetSysColor(COLOR_WINDOWTEXT)); // set to new system colour
	if (GetDefaultCell(TRUE, FALSE)->GetBackClr() == m_crWindowColour)
		GetDefaultCell(TRUE, FALSE)->SetBackClr(GetSysColor(COLOR_WINDOW));

	if (GetDefaultCell(FALSE, TRUE)->GetTextClr() == m_crWindowText) // Still using system colours
		GetDefaultCell(FALSE, TRUE)->SetTextClr(GetSysColor(COLOR_WINDOWTEXT)); // set to new system colour
	if (GetDefaultCell(FALSE, TRUE)->GetBackClr() == m_crWindowColour)
		GetDefaultCell(FALSE, TRUE)->SetBackClr(GetSysColor(COLOR_WINDOW));

	if (GetDefaultCell(TRUE, TRUE)->GetTextClr() == m_crWindowText) // Still using system colours
		GetDefaultCell(TRUE, TRUE)->SetTextClr(GetSysColor(COLOR_WINDOWTEXT)); // set to new system colour
	if (GetDefaultCell(TRUE, TRUE)->GetBackClr() == m_crWindowColour)
		GetDefaultCell(TRUE, TRUE)->SetBackClr(GetSysColor(COLOR_WINDOW));

	if (GetGridBkColor() == m_crShadow)
		SetGridBkColor(GetSysColor(COLOR_3DSHADOW));

	m_crWindowText = GetSysColor(COLOR_WINDOWTEXT);
	m_crWindowColour = GetSysColor(COLOR_WINDOW);
	m_cr3DFace = GetSysColor(COLOR_3DFACE);
	m_crShadow = GetSysColor(COLOR_3DSHADOW);

	m_nRowsPerWheelNotch = GetMouseScrollLines(); // Get the number of lines
}
#endif

// For drag-selection. Scrolls hidden cells into view
// TODO: decrease timer interval over time to speed up selection over time
void GridCtrl::OnTimer(UINT nIDEvent)
{
	ASSERT(nIDEvent == WM_LBUTTONDOWN);
	if (nIDEvent != WM_LBUTTONDOWN)
		return;

	CPoint pt, origPt;

#ifdef _WIN32_WCE
	if (m_MouseMode == MOUSE_NOTHING)
		return;
	origPt = GetMessagePos();
#else
	if (!GetCursorPos(&origPt))
		return;
#endif

	ScreenToClient(&origPt);

	CRect rect;
	GetClientRect(rect);

	const int nFixedRowHeight = GetFixedRowHeight();
	const int nFixedColWidth = GetFixedColumnWidth();

	pt = origPt;
	if (pt.y > rect.bottom)
	{
		//SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
		SendMessage(WM_KEYDOWN, VK_DOWN, 0);

		if (pt.x < rect.left)
			pt.x = rect.left;
		if (pt.x > rect.right)
			pt.x = rect.right;
		pt.y = rect.bottom;
		OnSelecting(GetCellFromPt(pt));
	}
	else if (pt.y < nFixedRowHeight)
	{
		//SendMessage(WM_VSCROLL, SB_LINEUP, 0);
		SendMessage(WM_KEYDOWN, VK_UP, 0);

		if (pt.x < rect.left)
			pt.x = rect.left;
		if (pt.x > rect.right)
			pt.x = rect.right;
		pt.y = nFixedRowHeight + 1;
		OnSelecting(GetCellFromPt(pt));
	}

	pt = origPt;
	if (pt.x > rect.right)
	{
		// SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);
		SendMessage(WM_KEYDOWN, VK_RIGHT, 0);

		if (pt.y < rect.top)
			pt.y = rect.top;
		if (pt.y > rect.bottom)
			pt.y = rect.bottom;
		pt.x = rect.right;
		OnSelecting(GetCellFromPt(pt));
	}
	else if (pt.x < nFixedColWidth)
	{
		//SendMessage(WM_HSCROLL, SB_LINELEFT, 0);
		SendMessage(WM_KEYDOWN, VK_LEFT, 0);

		if (pt.y < rect.top)
			pt.y = rect.top;
		if (pt.y > rect.bottom)
			pt.y = rect.bottom;
		pt.x = nFixedColWidth + 1;
		OnSelecting(GetCellFromPt(pt));
	}
}

// move about with keyboard
void GridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT n_flags)
{
	if (!IsValid(m_idCurrentCell))
	{
		CWnd::OnKeyDown(nChar, nRepCnt, n_flags);
		return;
	}
	GridCellID next = m_idCurrentCell;
	BOOL bChangeLine = FALSE;
	BOOL bHorzScrollAction = FALSE;
	BOOL bVertScrollAction = FALSE;

	if (IsCTRLpressed())
	{
		switch (nChar)
		{
		case 'A':
			OnEditSelectAll();
			break;
		case 'k': // This is ctrl+ on french keyboard, may need to be better processed for other locales
			AutoSizeColumns();
			Invalidate();
			break;
#ifndef GRIDCONTROL_NO_CLIPBOARD
		case 'X':
			OnEditCut();
			break;
		case VK_INSERT:
		case 'C':
			OnEditCopy();
			break;
		case 'V':
			OnEditPaste();
			break;
		default: ;
#endif
		}
	}

#ifndef GRIDCONTROL_NO_CLIPBOARD
	if (IsSHIFTpressed() && (nChar == VK_INSERT))
		OnEditPaste();
#endif

	BOOL bFoundVisible;
	int iOrig;

	if (nChar == VK_DELETE)
	{
		CutSelectedText();
	}
	else if (nChar == VK_DOWN)
	{
		// don't let user go to a hidden row
		bFoundVisible = FALSE;
		iOrig = next.row;
		next.row++;
		while (next.row < GetRowCount())
		{
			if (GetRowHeight(next.row) > 0)
			{
				bFoundVisible = TRUE;
				break;
			}
			next.row++;
		}
		if (!bFoundVisible)
			next.row = iOrig;
	}
	else if (nChar == VK_UP)
	{
		// don't let user go to a hidden row
		bFoundVisible = FALSE;
		iOrig = next.row;
		next.row--;
		while (next.row >= m_nFixedRows)
		{
			if (GetRowHeight(next.row) > 0)
			{
				bFoundVisible = TRUE;
				break;
			}
			next.row--;
		}
		if (!bFoundVisible)
			next.row = iOrig;
	}
	else if (nChar == VK_RIGHT || (nChar == VK_TAB && !IsSHIFTpressed()))
	{
		if ((nChar == VK_TAB) && m_QuitFocusOnTab)
		{
			auto p = static_cast<CDialog*>(GetParent());
			if (p) p->NextDlgCtrl();
			return;
		}
		// don't let user go to a hidden column
		bFoundVisible = FALSE;
		iOrig = next.col;
		next.col++;

		if (nChar == VK_TAB)
		{
			// If we're at the end of a row, go down a row till we find a non-hidden row
			if (next.col == (GetColumnCount()) && next.row < (GetRowCount() - 1))
			{
				next.row++;
				while (next.row < GetRowCount())
				{
					if (GetRowHeight(next.row) > 0)
					{
						bFoundVisible = TRUE;
						break;
					}
					next.row++;
				}

				next.col = m_nFixedCols; // Place focus on first non-fixed column
				bChangeLine = TRUE;
			}
			else
				CWnd::OnKeyDown(nChar, nRepCnt, n_flags);
		}

		// We're on a non-hidden row, so look across for the next non-hidden column
		while (next.col < GetColumnCount())
		{
			if (GetColumnWidth(next.col) > 0)
			{
				bFoundVisible = TRUE;
				break;
			}
			next.col++;
		}

		// If nothing worked then don't bother
		if (!bFoundVisible)
			next.col = iOrig;
	}
	else if (nChar == VK_LEFT || (nChar == VK_TAB && IsSHIFTpressed()))
	{
		// don't let user go to a hidden column
		bFoundVisible = FALSE;
		iOrig = next.col;
		next.col--;

		if (nChar == VK_TAB)
		{
			if (next.col == (GetFixedColumnCount() - 1) && next.row > GetFixedRowCount())
			{
				next.row--;
				while (next.row > GetFixedRowCount())
				{
					if (GetRowHeight(next.row) > 0)
					{
						bFoundVisible = TRUE;
						break;
					}
					next.row--;
				}

				next.col = GetColumnCount() - 1;
				bChangeLine = TRUE;
			}
			else
				CWnd::OnKeyDown(nChar, nRepCnt, n_flags);
		}

		while (next.col >= m_nFixedCols)
		{
			if (GetColumnWidth(next.col) > 0)
			{
				bFoundVisible = TRUE;
				break;
			}
			next.col--;
		}
		if (!bFoundVisible)
			next.col = iOrig;
	}
	else if (nChar == VK_NEXT)
	{
		const GridCellID idOldTopLeft = GetTopleftNonFixedCell();
		SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
		bVertScrollAction = TRUE;
		const GridCellID idNewTopLeft = GetTopleftNonFixedCell();

		const int increment = idNewTopLeft.row - idOldTopLeft.row;
		if (increment)
		{
			next.row += increment;
			if (next.row > (GetRowCount() - 1))
				next.row = GetRowCount() - 1;
		}
		else
			next.row = GetRowCount() - 1;
	}
	else if (nChar == VK_PRIOR)
	{
		const GridCellID idOldTopLeft = GetTopleftNonFixedCell();
		SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
		bVertScrollAction = TRUE;
		const GridCellID idNewTopLeft = GetTopleftNonFixedCell();

		const int increment = idNewTopLeft.row - idOldTopLeft.row;
		if (increment)
		{
			next.row += increment;
			if (next.row < m_nFixedRows)
				next.row = m_nFixedRows;
		}
		else
			next.row = m_nFixedRows;
	}
	else if (nChar == VK_HOME)
	{
		// Home and Ctrl-Home work more like Excel
		//  and don't let user go to a hidden cell
		if (IsCTRLpressed())
		{
			SendMessage(WM_VSCROLL, SB_TOP, 0);
			SendMessage(WM_HSCROLL, SB_LEFT, 0);
			bVertScrollAction = TRUE;
			bHorzScrollAction = TRUE;
			next.row = m_nFixedRows;
			next.col = m_nFixedCols;
		}
		else
		{
			SendMessage(WM_HSCROLL, SB_LEFT, 0);
			bHorzScrollAction = TRUE;
			next.col = m_nFixedCols;
		}
		// adjust column to avoid hidden columns and rows
		while (next.col < GetColumnCount() - 1)
		{
			if (GetColumnWidth(next.col) > 0)
				break;
			next.col++;
		}
		while (next.row < GetRowCount() - 1)
		{
			if (GetRowHeight(next.row) > 0)
				break;
			next.row++;
		}
	}
	else if (nChar == VK_END)
	{
		// End and Ctrl-End work more like Excel
		//  and don't let user go to a hidden cell
		if (IsCTRLpressed())
		{
			SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
			SendMessage(WM_HSCROLL, SB_RIGHT, 0);
			bHorzScrollAction = TRUE;
			bVertScrollAction = TRUE;
			next.row = GetRowCount() - 1;
			next.col = GetColumnCount() - 1;
		}
		else
		{
			SendMessage(WM_HSCROLL, SB_RIGHT, 0);
			bHorzScrollAction = TRUE;
			next.col = GetColumnCount() - 1;
		}
		// adjust column to avoid hidden columns and rows
		while (next.col > m_nFixedCols + 1)
		{
			if (GetColumnWidth(next.col) > 0)
				break;
			next.col--;
		}
		while (next.row > m_nFixedRows + 1)
		{
			if (GetRowHeight(next.row) > 0)
				break;
			next.row--;
		}
	}
	else if (nChar == VK_F2)
	{
		OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, CPoint(-1, -1), VK_LBUTTON);
	}
	else
	{
		CWnd::OnKeyDown(nChar, nRepCnt, n_flags);
		return;
	}

	if (next != m_idCurrentCell)
	{
		// While moving with the Cursorkeys the current ROW/CELL will get selected
		// OR Selection will get expanded when SHIFT is pressed
		// Cut n paste from OnLButtonDown - Franco Bez
		// Added check for NULL mouse mode - Chris Maunder.
		if (m_MouseMode == MOUSE_NOTHING)
		{
			m_PrevSelectedCellMap.RemoveAll();
			m_MouseMode = m_bListMode ? MOUSE_SELECT_ROW : MOUSE_SELECT_CELLS;
			if (!IsSHIFTpressed() || nChar == VK_TAB)
				m_SelectionStartCell = next;

			// Notify parent that selection is changing - Arthur Westerman/Scot Brennecke
			SendMessageToParent(next.row, next.col, GVN_SELCHANGING);
			OnSelecting(next);
			SendMessageToParent(next.row, next.col, GVN_SELCHANGED);

			m_MouseMode = MOUSE_NOTHING;
		}

		SetFocusCell(next);

		if (!IsCellVisible(next))
		{
			switch (nChar)
			{
			case VK_RIGHT:
				SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);
				bHorzScrollAction = TRUE;
				break;

			case VK_LEFT:
				SendMessage(WM_HSCROLL, SB_LINELEFT, 0);
				bHorzScrollAction = TRUE;
				break;

			case VK_DOWN:
				SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
				bVertScrollAction = TRUE;
				break;

			case VK_UP:
				SendMessage(WM_VSCROLL, SB_LINEUP, 0);
				bVertScrollAction = TRUE;
				break;

			case VK_TAB:
				if (IsSHIFTpressed())
				{
					if (bChangeLine)
					{
						SendMessage(WM_VSCROLL, SB_LINEUP, 0);
						bVertScrollAction = TRUE;
						SetScrollPos32(SB_HORZ, m_nHScrollMax);
						break;
					}
					SendMessage(WM_HSCROLL, SB_LINELEFT, 0);
					bHorzScrollAction = TRUE;
				}
				else
				{
					if (bChangeLine)
					{
						SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
						bVertScrollAction = TRUE;
						SetScrollPos32(SB_HORZ, 0);
						break;
					}
					SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);
					bHorzScrollAction = TRUE;
				}
				break;
			default: ;
			}
			EnsureVisible(next); // Make sure cell is visible
			Invalidate();
		}
		EnsureVisible(next); // Make sure cell is visible

		if (bHorzScrollAction)
			SendMessage(WM_HSCROLL, SB_ENDSCROLL, 0);
		if (bVertScrollAction)
			SendMessage(WM_VSCROLL, SB_ENDSCROLL, 0);
	}
}

void GridCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT n_flags)
{
	CWnd::OnKeyUp(nChar, nRepCnt, n_flags);
}

void GridCtrl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT n_flags)
{
#ifdef GRIDCONTROL_USE_TITLETIPS
	m_TitleTip.Hide();  // hide any titletips
#endif

	CWnd::OnSysKeyDown(nChar, nRepCnt, n_flags);
}

// Instant editing of cells when keys are pressed
void GridCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT n_flags)
{
	// EFW - BUG FIX
	if (!IsCTRLpressed() && m_MouseMode == MOUSE_NOTHING && nChar != VK_ESCAPE)
	{
		if (!m_bHandleTabKey || (m_bHandleTabKey && nChar != VK_TAB))
			OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, CPoint(-1, -1), nChar);
	}

	CWnd::OnChar(nChar, nRepCnt, n_flags);
}

// Added by KiteFly
LRESULT GridCtrl::OnImeChar(WPARAM wCharCode, LPARAM)
{
	// EFW - BUG FIX
	if (!IsCTRLpressed() && m_MouseMode == MOUSE_NOTHING && wCharCode != VK_ESCAPE)
		OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, CPoint(-1, -1), wCharCode);
	return 0;
}

// Callback from any CInPlaceEdits that ended. This just calls OnEndEditCell,
// refreshes the edited cell and moves onto next cell if the return character
// from the edit says we should.
void GridCtrl::OnEndInPlaceEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto pgvDispInfo = reinterpret_cast<GV_DISPINFO*>(pNMHDR);
	const GV_ITEM* pgvItem = &pgvDispInfo->item;

	// In case OnEndInPlaceEdit called as window is being destroyed
	if (!IsWindow(GetSafeHwnd()))
		return;

	OnEndEditCell(pgvItem->row, pgvItem->col, pgvItem->strText);
	//InvalidateCellRect(GridCellID(pgvItem->row, pgvItem->col));

	switch (pgvItem->lParam)
	{
	case VK_TAB:
	case VK_DOWN:
	case VK_UP:
	case VK_RIGHT:
	case VK_LEFT:
	case VK_NEXT:
	case VK_PRIOR:
	case VK_HOME:
	case VK_END:
		OnKeyDown(static_cast<UINT>(pgvItem->lParam), 0, 0);
		OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, CPoint(-1, -1), static_cast<UINT>(pgvItem->lParam));
	}

	*pResult = 0;
}

// Handle horz scrollbar notifications
void GridCtrl::OnHScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar* /*pScrollBar*/)
{
	EndEditing();

#ifndef GRIDCONTROL_NO_TITLETIPS
	m_TitleTip.Hide(); // hide any titletips
#endif

	const int scrollPos = GetScrollPos32(SB_HORZ);

	GridCellID idTopLeft = GetTopleftNonFixedCell();

	CRect rect;
	GetClientRect(rect);

	switch (nSBCode)
	{
	case SB_LINERIGHT:
		if (scrollPos < m_nHScrollMax)
		{
			// may have contiguous hidden columns.  Blow by them
			while (idTopLeft.col < (GetColumnCount() - 1)
				&& GetColumnWidth(idTopLeft.col) < 1)
			{
				idTopLeft.col++;
			}
			const int xScroll = GetColumnWidth(idTopLeft.col);
			SetScrollPos32(SB_HORZ, scrollPos + xScroll);
			if (GetScrollPos32(SB_HORZ) == scrollPos)
				break; // didn't work

			rect.left = GetFixedColumnWidth();
			//rect.left = GetFixedColumnWidth() + xScroll;
			//ScrollWindow(-xScroll, 0, rect);
			//rect.left = rect.right - xScroll;
			InvalidateRect(rect);
		}
		break;

	case SB_LINELEFT:
		if (scrollPos > 0 && idTopLeft.col > GetFixedColumnCount())
		{
			int iColToUse = idTopLeft.col - 1;
			// may have contiguous hidden columns.  Blow by them
			while (iColToUse > GetFixedColumnCount()
				&& GetColumnWidth(iColToUse) < 1)
			{
				iColToUse--;
			}

			const int xScroll = GetColumnWidth(iColToUse);
			SetScrollPos32(SB_HORZ, __max(0, scrollPos - xScroll));
			rect.left = GetFixedColumnWidth();
			//ScrollWindow(xScroll, 0, rect);
			//rect.right = rect.left + xScroll;
			InvalidateRect(rect);
		}
		break;

	case SB_PAGERIGHT:
		if (scrollPos < m_nHScrollMax)
		{
			rect.left = GetFixedColumnWidth();
			const int offset = rect.Width();
			const int pos = min(m_nHScrollMax, scrollPos + offset);
			SetScrollPos32(SB_HORZ, pos);
			rect.left = GetFixedColumnWidth();
			InvalidateRect(rect);
		}
		break;

	case SB_PAGELEFT:
		if (scrollPos > 0)
		{
			rect.left = GetFixedColumnWidth();
			const int offset = -rect.Width();
			const int pos = __max(0, scrollPos + offset);
			SetScrollPos32(SB_HORZ, pos);
			rect.left = GetFixedColumnWidth();
			InvalidateRect(rect);
		}
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			SetScrollPos32(SB_HORZ, GetScrollPos32(SB_HORZ, TRUE));
			m_idTopLeftCell.row = -1;
			const GridCellID idNewTopLeft = GetTopleftNonFixedCell();
			if (idNewTopLeft != idTopLeft)
			{
				rect.left = GetFixedColumnWidth();
				InvalidateRect(rect);
			}
		}
		break;

	case SB_LEFT:
		if (scrollPos > 0)
		{
			SetScrollPos32(SB_HORZ, 0);
			Invalidate();
		}
		break;

	case SB_RIGHT:
		if (scrollPos < m_nHScrollMax)
		{
			SetScrollPos32(SB_HORZ, m_nHScrollMax);
			Invalidate();
		}
		break;

	default:
		break;
	}
}

// Handle vert scrollbar notifications
void GridCtrl::OnVScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar* /*pScrollBar*/)
{
	EndEditing();

#ifndef GRIDCONTROL_NO_TITLETIPS
	m_TitleTip.Hide(); // hide any titletips
#endif

	// Get the scroll position ourselves to ensure we get a 32 bit value
	int scrollPos = GetScrollPos32(SB_VERT);

	GridCellID idTopLeft = GetTopleftNonFixedCell();

	CRect rect;
	GetClientRect(rect);

	switch (nSBCode)
	{
	case SB_LINEDOWN:
		if (scrollPos < m_nVScrollMax)
		{
			// may have contiguous hidden rows.  Blow by them
			while (idTopLeft.row < (GetRowCount() - 1)
				&& GetRowHeight(idTopLeft.row) < 1)
			{
				idTopLeft.row++;
			}

			const int yScroll = GetRowHeight(idTopLeft.row);
			SetScrollPos32(SB_VERT, scrollPos + yScroll);
			if (GetScrollPos32(SB_VERT) == scrollPos)
				break; // didn't work

			rect.top = GetFixedRowHeight();
			//rect.top = GetFixedRowHeight() + yScroll;
			//ScrollWindow(0, -yScroll, rect);
			//rect.top = rect.bottom - yScroll;
			InvalidateRect(rect);
		}
		break;

	case SB_LINEUP:
		if (scrollPos > 0 && idTopLeft.row > GetFixedRowCount())
		{
			int iRowToUse = idTopLeft.row - 1;
			// may have contiguous hidden rows.  Blow by them
			while (iRowToUse > GetFixedRowCount()
				&& GetRowHeight(iRowToUse) < 1)
			{
				iRowToUse--;
			}

			const int yScroll = GetRowHeight(iRowToUse);
			SetScrollPos32(SB_VERT, __max(0, scrollPos - yScroll));
			rect.top = GetFixedRowHeight();
			//ScrollWindow(0, yScroll, rect);
			//rect.bottom = rect.top + yScroll;
			InvalidateRect(rect);
		}
		break;

	case SB_PAGEDOWN:
		if (scrollPos < m_nVScrollMax)
		{
			rect.top = GetFixedRowHeight();
			scrollPos = min(m_nVScrollMax, scrollPos + rect.Height());
			SetScrollPos32(SB_VERT, scrollPos);
			rect.top = GetFixedRowHeight();
			InvalidateRect(rect);
		}
		break;

	case SB_PAGEUP:
		if (scrollPos > 0)
		{
			rect.top = GetFixedRowHeight();
			const int offset = -rect.Height();
			const int pos = __max(0, scrollPos + offset);
			SetScrollPos32(SB_VERT, pos);
			rect.top = GetFixedRowHeight();
			InvalidateRect(rect);
		}
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			SetScrollPos32(SB_VERT, GetScrollPos32(SB_VERT, TRUE));
			m_idTopLeftCell.row = -1;
			const GridCellID idNewTopLeft = GetTopleftNonFixedCell();
			if (idNewTopLeft != idTopLeft)
			{
				rect.top = GetFixedRowHeight();
				InvalidateRect(rect);
			}
		}
		break;

	case SB_TOP:
		if (scrollPos > 0)
		{
			SetScrollPos32(SB_VERT, 0);
			Invalidate();
		}
		break;

	case SB_BOTTOM:
		if (scrollPos < m_nVScrollMax)
		{
			SetScrollPos32(SB_VERT, m_nVScrollMax);
			Invalidate();
		}

	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// GridCtrl implementation functions

void GridCtrl::OnDraw(CDC* p_dc)
{
	if (!m_bAllowDraw)
		return;

	CRect clipRect;
	if (p_dc->GetClipBox(&clipRect) == ERROR)
		return;

	EraseBkgnd(p_dc); // OnEraseBkgnd does nothing, so erase bkgnd here.
	// This necessary since we may be using a Memory DC.

#ifdef _DEBUG
	LARGE_INTEGER iStartCount;
	QueryPerformanceCounter(&iStartCount);
#endif

	CRect rect;
	int row, col;
	GridCellBase* pCell;

	const int nFixedRowHeight = GetFixedRowHeight();
	const int nFixedColWidth = GetFixedColumnWidth();

	const GridCellID idTopLeft = GetTopleftNonFixedCell();
	const int minVisibleRow = idTopLeft.row;
	const int minVisibleCol = idTopLeft.col;

	CRect VisRect;
	const GridCellRange VisCellRange = GetVisibleNonFixedCellRange(VisRect);
	const int maxVisibleRow = VisCellRange.GetMaxRow();
	const int maxVisibleCol = VisCellRange.GetMaxCol();

	if (GetVirtualMode())
		SendCacheHintToParent(VisCellRange);

	// draw top-left cells 0..m_nFixedRows-1, 0..m_nFixedCols-1
	rect.bottom = -1;
	for (row = 0; row < m_nFixedRows; row++)
	{
		if (GetRowHeight(row) <= 0) continue;

		rect.top = rect.bottom + 1;
		rect.bottom = rect.top + GetRowHeight(row) - 1;
		rect.right = -1;

		for (col = 0; col < m_nFixedCols; col++)
		{
			if (GetColumnWidth(col) <= 0) continue;

			rect.left = rect.right + 1;
			rect.right = rect.left + GetColumnWidth(col) - 1;

			pCell = GetCell(row, col);
			if (pCell)
			{
				pCell->SetCoords(row, col);
				pCell->Draw(p_dc, row, col, rect, FALSE);
			}
		}
	}

	// draw fixed column cells:  m_nFixedRows..n, 0..m_nFixedCols-1
	rect.bottom = nFixedRowHeight - 1;
	for (row = minVisibleRow; row <= maxVisibleRow; row++)
	{
		if (GetRowHeight(row) <= 0) continue;

		rect.top = rect.bottom + 1;
		rect.bottom = rect.top + GetRowHeight(row) - 1;

		// rect.bottom = bottom pixel of previous row
		if (rect.top > clipRect.bottom)
			break; // Gone past cliprect
		if (rect.bottom < clipRect.top)
			continue; // Reached cliprect yet?

		rect.right = -1;
		for (col = 0; col < m_nFixedCols; col++)
		{
			if (GetColumnWidth(col) <= 0) continue;

			rect.left = rect.right + 1;
			rect.right = rect.left + GetColumnWidth(col) - 1;

			if (rect.left > clipRect.right)
				break; // gone past cliprect
			if (rect.right < clipRect.left)
				continue; // Reached cliprect yet?

			pCell = GetCell(row, col);
			if (pCell)
			{
				pCell->SetCoords(row, col);
				pCell->Draw(p_dc, row, col, rect, FALSE);
			}
		}
	}

	// draw fixed row cells  0..m_nFixedRows, m_nFixedCols..n
	rect.bottom = -1;
	for (row = 0; row < m_nFixedRows; row++)
	{
		if (GetRowHeight(row) <= 0) continue;

		rect.top = rect.bottom + 1;
		rect.bottom = rect.top + GetRowHeight(row) - 1;

		// rect.bottom = bottom pixel of previous row
		if (rect.top > clipRect.bottom)
			break; // Gone past cliprect
		if (rect.bottom < clipRect.top)
			continue; // Reached cliprect yet?

		rect.right = nFixedColWidth - 1;
		for (col = minVisibleCol; col <= maxVisibleCol; col++)
		{
			if (GetColumnWidth(col) <= 0) continue;

			rect.left = rect.right + 1;
			rect.right = rect.left + GetColumnWidth(col) - 1;

			if (rect.left > clipRect.right)
				break; // gone past cliprect
			if (rect.right < clipRect.left)
				continue; // Reached cliprect yet?

			pCell = GetCell(row, col);
			if (pCell)
			{
				pCell->SetCoords(row, col);
				pCell->Draw(p_dc, row, col, rect, FALSE);
			}
		}
	}

	// draw rest of non-fixed cells
	rect.bottom = nFixedRowHeight - 1;
	for (row = minVisibleRow; row <= maxVisibleRow; row++)
	{
		if (GetRowHeight(row) <= 0) continue;

		rect.top = rect.bottom + 1;
		rect.bottom = rect.top + GetRowHeight(row) - 1;

		// rect.bottom = bottom pixel of previous row
		if (rect.top > clipRect.bottom)
			break; // Gone past cliprect
		if (rect.bottom < clipRect.top)
			continue; // Reached cliprect yet?

		rect.right = nFixedColWidth - 1;
		for (col = minVisibleCol; col <= maxVisibleCol; col++)
		{
			if (GetColumnWidth(col) <= 0) continue;

			rect.left = rect.right + 1;
			rect.right = rect.left + GetColumnWidth(col) - 1;

			if (rect.left > clipRect.right)
				break; // gone past cliprect
			if (rect.right < clipRect.left)
				continue; // Reached cliprect yet?

			pCell = GetCell(row, col);
			// ATLTRACE2(_T("Cell %d,%d type: %s\n"), row, col, pCell->GetRuntimeClass()->m_lpszClassName);
			if (pCell)
			{
				pCell->SetCoords(row, col);
				pCell->Draw(p_dc, row, col, rect, FALSE);
			}
		}
	}

	CPen pen;
	pen.CreatePen(PS_SOLID, 0, m_crGridLineColour);
	p_dc->SelectObject(&pen);

	// draw vertical lines (drawn at ends of cells)
	if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
	{
		int x = nFixedColWidth;
		for (col = minVisibleCol; col <= maxVisibleCol; col++)
		{
			if (GetColumnWidth(col) <= 0) continue;

			x += GetColumnWidth(col);
			p_dc->MoveTo(x - 1, nFixedRowHeight);
			p_dc->LineTo(x - 1, VisRect.bottom);
		}
	}

	// draw horizontal lines (drawn at bottom of each cell)
	if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
	{
		int y = nFixedRowHeight;
		for (row = minVisibleRow; row <= maxVisibleRow; row++)
		{
			if (GetRowHeight(row) <= 0) continue;

			y += GetRowHeight(row);
			p_dc->MoveTo(nFixedColWidth, y - 1);
			p_dc->LineTo(VisRect.right, y - 1);
		}
	}

	p_dc->SelectStockObject(NULL_PEN);

	// Let parent know it can discard it's data if it needs to.
	if (GetVirtualMode())
		SendCacheHintToParent(GridCellRange(-1, -1, -1, -1));

#ifdef _DEBUG
	LARGE_INTEGER iEndCount;
	QueryPerformanceCounter(&iEndCount);
	//ATLTRACE2("Draw counter ticks: %d\n", iEndCount.LowPart-iStartCount.LowPart);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
// GridCtrl Cell selection stuff

// Is a given cell designation valid (ie within the bounds of our number
// of columns/rows)?
BOOL GridCtrl::IsValid(int nRow, int nCol) const
{
	return (nRow >= 0 && nRow < m_nRows && nCol >= 0 && nCol < m_nCols);
}

BOOL GridCtrl::IsValid(const GridCellID& cell) const
{
	return IsValid(cell.row, cell.col);
}

// Is a given cell range valid (ie within the bounds of our number
// of columns/rows)?
BOOL GridCtrl::IsValid(const GridCellRange& range) const
{
	return (range.GetMinRow() >= 0 && range.GetMinCol() >= 0 &&
		range.GetMaxRow() >= 0 && range.GetMaxCol() >= 0 &&
		range.GetMaxRow() < m_nRows && range.GetMaxCol() < m_nCols &&
		range.GetMinRow() <= range.GetMaxRow() && range.GetMinCol() <= range.GetMaxCol());
}

// Enables/Disables redraw for certain operations like columns auto-sizing etc,
// but not for user caused things such as selection changes.
void GridCtrl::SetRedraw(BOOL bAllowDraw, BOOL bResetScrollBars /* = FALSE */)
{
	if (bAllowDraw && !m_bAllowDraw)
	{
		m_bAllowDraw = TRUE;
		Refresh();
	}

	m_bAllowDraw = bAllowDraw;
	if (bResetScrollBars)
		ResetScrollBars();
}

// Forces a redraw of a cell immediately (using a direct DC construction,
// or the supplied dc)
BOOL GridCtrl::RedrawCell(const GridCellID& cell, CDC* p_dc /* = NULL */)
{
	return RedrawCell(cell.row, cell.col, p_dc);
}

BOOL GridCtrl::RedrawCell(int nRow, int nCol, CDC* p_dc /* = NULL */)
{
	BOOL bResult = TRUE;
	BOOL bMustReleaseDC = FALSE;

	if (!m_bAllowDraw || !IsCellVisible(nRow, nCol))
		return FALSE;

	CRect rect;
	if (!GetCellRect(nRow, nCol, rect))
		return FALSE;

	if (!p_dc)
	{
		p_dc = GetDC();
		if (p_dc)
			bMustReleaseDC = TRUE;
	}

	if (p_dc)
	{
		// Redraw cells directly
		if (nRow < m_nFixedRows || nCol < m_nFixedCols)
		{
			GridCellBase* pCell = GetCell(nRow, nCol);
			if (pCell)
				bResult = pCell->Draw(p_dc, nRow, nCol, rect, TRUE);
		}
		else
		{
			GridCellBase* pCell = GetCell(nRow, nCol);
			if (pCell)
				bResult = pCell->Draw(p_dc, nRow, nCol, rect, TRUE);

			// Since we have erased the background, we will need to redraw the gridlines
			CPen pen;
			pen.CreatePen(PS_SOLID, 0, m_crGridLineColour);

			auto pOldPen = p_dc->SelectObject(&pen);
			if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
			{
				p_dc->MoveTo(rect.left, rect.bottom);
				p_dc->LineTo(rect.right + 1, rect.bottom);
			}
			if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
			{
				p_dc->MoveTo(rect.right, rect.top);
				p_dc->LineTo(rect.right, rect.bottom + 1);
			}
			p_dc->SelectObject(pOldPen);
		}
	}
	else
		InvalidateRect(rect, TRUE); // Could not get a DC - invalidate it anyway
	// and hope that OnPaint manages to get one

	if (bMustReleaseDC)
		ReleaseDC(p_dc);

	return bResult;
}

// redraw a complete row
BOOL GridCtrl::RedrawRow(int row)
{
	BOOL bResult = TRUE;

	CDC* p_dc = GetDC();
	for (int col = 0; col < GetColumnCount(); col++)
		bResult = RedrawCell(row, col, p_dc) && bResult;
	if (p_dc)
		ReleaseDC(p_dc);

	return bResult;
}

// redraw a complete column
BOOL GridCtrl::RedrawColumn(int col)
{
	BOOL bResult = TRUE;

	CDC* p_dc = GetDC();
	for (int row = 0; row < GetRowCount(); row++)
		bResult = RedrawCell(row, col, p_dc) && bResult;
	if (p_dc)
		ReleaseDC(p_dc);

	return bResult;
}

// Sets the currently selected cell, returning the previous current cell
GridCellID GridCtrl::SetFocusCell(int nRow, int nCol)
{
	return SetFocusCell(GridCellID(nRow, nCol));
}

GridCellID GridCtrl::SetFocusCell(GridCellID cell)
{
	if (cell == m_idCurrentCell)
		return m_idCurrentCell;

	GridCellID idPrev = m_idCurrentCell;

	// EFW - Bug Fix - Force focus to be in a non-fixed cell
	if (cell.row != -1 && cell.row < GetFixedRowCount())
		cell.row = GetFixedRowCount();
	if (cell.col != -1 && cell.col < GetFixedColumnCount())
		cell.col = GetFixedColumnCount();

	m_idCurrentCell = cell;

	if (IsValid(idPrev))
	{
		SetItemState(idPrev.row, idPrev.col,
		             GetItemState(idPrev.row, idPrev.col) & ~GVIS_FOCUSED);
		RedrawCell(idPrev); // comment to reduce flicker

		if (GetTrackFocusCell() && idPrev.col != m_idCurrentCell.col)
			for (int row = 0; row < m_nFixedRows; row++)
				RedrawCell(row, idPrev.col);
		if (GetTrackFocusCell() && idPrev.row != m_idCurrentCell.row)
			for (int col = 0; col < m_nFixedCols; col++)
				RedrawCell(idPrev.row, col);
	}

	if (IsValid(m_idCurrentCell))
	{
		SetItemState(m_idCurrentCell.row, m_idCurrentCell.col,
		             GetItemState(m_idCurrentCell.row, m_idCurrentCell.col) | GVIS_FOCUSED);

		RedrawCell(m_idCurrentCell); // comment to reduce flicker

		if (GetTrackFocusCell() && idPrev.col != m_idCurrentCell.col)
			for (int row = 0; row < m_nFixedRows; row++)
				RedrawCell(row, m_idCurrentCell.col);
		if (GetTrackFocusCell() && idPrev.row != m_idCurrentCell.row)
			for (int col = 0; col < m_nFixedCols; col++)
				RedrawCell(m_idCurrentCell.row, col);

		// EFW - New addition.  If in list mode, make sure the selected
		// row highlight follows the cursor.
		// Removed by C Maunder 27 May
		//if (m_bListMode)
		//{
		//    m_PrevSelectedCellMap.RemoveAll();
		//    m_MouseMode = MOUSE_SELECT_ROW;
		//    OnSelecting(m_idCurrentCell);

		// Leave this off so that you can still drag the highlight around
		// without selecting rows.
		// m_MouseMode = MOUSE_NOTHING;
		//}
	}

	return idPrev;
}

// Sets the range of currently selected cells
void GridCtrl::SetSelectedRange(const GridCellRange& Range,
                                 BOOL bForceRepaint /* = FALSE */, BOOL bSelectCells/*=TRUE*/)
{
	SetSelectedRange(Range.GetMinRow(), Range.GetMinCol(),
	                 Range.GetMaxRow(), Range.GetMaxCol(),
	                 bForceRepaint, bSelectCells);
}

void GridCtrl::SetSelectedRange(int nMinRow, int nMinCol, int nMaxRow, int nMaxCol,
                                 BOOL bForceRepaint /* = FALSE */, BOOL bSelectCells/*=TRUE*/)
{
	if (!m_bEnableSelection)
		return;

	CWaitCursor wait; // Thomas Haase

	CDC* p_dc = nullptr;
	if (bForceRepaint)
		p_dc = GetDC();

	// Only redraw visible cells
	GridCellRange VisCellRange, FixedVisCellRange;
	if (IsWindow(GetSafeHwnd()))

	{
		VisCellRange = GetVisibleNonFixedCellRange();
		FixedVisCellRange = GetVisibleFixedCellRange();
	}

	// EFW - Bug fix - Don't allow selection of fixed rows
	int Left = (m_AllowSelectRowInFixedCol ? 0 : GetFixedColumnCount());

	if (nMinRow >= 0 && nMinRow < GetFixedRowCount())
		nMinRow = GetFixedRowCount();
	if (nMaxRow >= 0 && nMaxRow < GetFixedRowCount())
		nMaxRow = GetFixedRowCount();
	if (nMinCol >= 0 && nMinCol < Left)
		nMinCol = GetFixedColumnCount();
	if (nMaxCol >= 0 && nMaxCol < Left)
		nMaxCol = GetFixedColumnCount();

	// If we are selecting cells, then first clear out the list of currently selected cells, then
	if (bSelectCells)
	{
		POSITION pos;

		// Unselect all previously selected cells
		for (pos = m_SelectedCellMap.GetStartPosition(); pos != nullptr;)
		{
			DWORD key;
			GridCellID cell;
			m_SelectedCellMap.GetNextAssoc(pos, key, cell);

			// Reset the selection flag on the cell
			if (IsValid(cell))
			{
				// This will remove the cell from the m_SelectedCellMap map
				SetItemState(cell.row, cell.col,
				             GetItemState(cell.row, cell.col) & ~GVIS_SELECTED);

				// If this is to be reselected, continue on past the redraw
				if (nMinRow <= cell.row && cell.row <= nMaxRow &&
					nMinCol <= cell.col && cell.col <= nMaxCol)
					continue;

				if ((VisCellRange.IsValid() && VisCellRange.InRange(cell)) || FixedVisCellRange.InRange(cell))
				{
					if (bForceRepaint && p_dc) // Redraw NOW
						RedrawCell(cell.row, cell.col, p_dc);
					else
						InvalidateCellRect(cell); // Redraw at leisure
				}
			}
			else
			{
				m_SelectedCellMap.RemoveKey(key); // if it's not valid, get rid of it!
			}
		}

		// if we are selecting cells, and there are previous selected cells to be retained
		// (eg Ctrl is being held down) then copy them to the newly created list, and mark
		// all these cells as selected
		// Note that if we are list mode, single row selection, then we won't be adding
		// the previous cells. Only the current row of cells will be added (see below)
		if (!GetSingleRowSelection() &&
			nMinRow >= 0 && nMinCol >= 0 && nMaxRow >= 0 && nMaxCol >= 0)
		{
			for (pos = m_PrevSelectedCellMap.GetStartPosition(); pos != nullptr; /* nothing */)
			{
				DWORD key;
				GridCellID cell;
				m_PrevSelectedCellMap.GetNextAssoc(pos, key, cell);

				if (!IsValid(cell))
					continue;

				const int nState = GetItemState(cell.row, cell.col);

				// Set state as Selected. This will add the cell to m_SelectedCellMap
				SetItemState(cell.row, cell.col, nState | GVIS_SELECTED);

				if ((VisCellRange.IsValid() && VisCellRange.InRange(cell)) || FixedVisCellRange.InRange(cell))
				{
					// Redraw (immediately or at leisure)
					if (bForceRepaint && p_dc)
						RedrawCell(cell.row, cell.col, p_dc);
					else
						InvalidateCellRect(cell);
				}
			}
		}
	}

	// Now select/deselect all cells in the cell range specified. If selecting, and the cell
	// has already been marked as selected (above) then ignore it. If we are deselecting and
	// the cell isn't selected, then ignore
	if (nMinRow >= 0 && nMinCol >= 0 && nMaxRow >= 0 && nMaxCol >= 0 &&
		nMaxRow < m_nRows && nMaxCol < m_nCols &&
		nMinRow <= nMaxRow && nMinCol <= nMaxCol)
	{
		for (int row = nMinRow; row <= nMaxRow; row++)
			for (int col = nMinCol; col <= nMaxCol; col++)
			{
				const BOOL bCellSelected = IsCellSelected(row, col);
				if (bSelectCells == bCellSelected)
					continue; // Already selected or deselected - ignore

				// Set the selected state. This will add/remove the cell to m_SelectedCellMap
				if (bSelectCells)
					SetItemState(row, col, GetItemState(row, col) | GVIS_SELECTED);
				else
					SetItemState(row, col, GetItemState(row, col) & ~GVIS_SELECTED);

				if ((VisCellRange.IsValid() && VisCellRange.InRange(row, col)) || FixedVisCellRange.InRange(row, col))
				{
					// Redraw (immediately or at leisure)
					if (bForceRepaint && p_dc)
						RedrawCell(row, col, p_dc);
					else
						InvalidateCellRect(row, col);
				}
			}
	}
	if (p_dc != nullptr)
		ReleaseDC(p_dc);
}

// selects all cells
void GridCtrl::SelectAllCells()
{
	if (!m_bEnableSelection)
		return;

	SetSelectedRange(m_nFixedRows, m_nFixedCols, GetRowCount() - 1, GetColumnCount() - 1);
}

// selects columns
void GridCtrl::SelectColumns(GridCellID currentCell,
                              BOOL bForceRedraw /*=FALSE*/, BOOL bSelectCells /*=TRUE*/)
{
	if (!m_bEnableSelection)
		return;

	//if (currentCell.col == m_idCurrentCell.col) return;
	if (currentCell.col < m_nFixedCols)
		return;
	if (!IsValid(currentCell))
		return;

	if (GetSingleColSelection())
		SetSelectedRange(GetFixedRowCount(), currentCell.col,
		                 GetRowCount() - 1, currentCell.col,
		                 bForceRedraw, bSelectCells);
	else
		SetSelectedRange(GetFixedRowCount(),
		                 min(m_SelectionStartCell.col, currentCell.col),
		                 GetRowCount() - 1,
		                 max(m_SelectionStartCell.col, currentCell.col),
		                 bForceRedraw, bSelectCells);
}

// selects rows
void GridCtrl::SelectRows(GridCellID currentCell,
                           BOOL bForceRedraw /*=FALSE*/, BOOL bSelectCells /*=TRUE*/)
{
	if (!m_bEnableSelection)
		return;

	//if (currentCell.row; == m_idCurrentCell.row) return;
	if (currentCell.row < m_nFixedRows)
		return;
	if (!IsValid(currentCell))
		return;
	int Left = (m_AllowSelectRowInFixedCol ? 0 : GetFixedColumnCount());
	if (GetSingleRowSelection())
		SetSelectedRange(currentCell.row, Left,
		                 currentCell.row, GetColumnCount() - 1,
		                 bForceRedraw, bSelectCells);
	else
		SetSelectedRange(min(m_SelectionStartCell.row, currentCell.row),
		                 Left,
		                 __max(m_SelectionStartCell.row, currentCell.row),
		                 GetColumnCount() - 1,
		                 bForceRedraw, bSelectCells);
}

// selects cells
void GridCtrl::SelectCells(GridCellID currentCell,
                            BOOL bForceRedraw /*=FALSE*/, BOOL bSelectCells /*=TRUE*/)
{
	if (!m_bEnableSelection)
		return;

	int row = currentCell.row;
	int col = currentCell.col;
	if (row < m_nFixedRows || col < m_nFixedCols)
		return;
	if (!IsValid(currentCell))
		return;

	// Prevent unnecessary redraws
	//if (currentCell == m_LeftClickDownCell)  return;
	//else if (currentCell == m_idCurrentCell) return;

	SetSelectedRange(min(m_SelectionStartCell.row, row),
	                 min(m_SelectionStartCell.col, col),
	                 __max(m_SelectionStartCell.row, row),
	                 __max(m_SelectionStartCell.col, col),
	                 bForceRedraw, bSelectCells);
}

// Called when mouse/keyboard selection is a-happening.
void GridCtrl::OnSelecting(const GridCellID& currentCell)
{
	if (!m_bEnableSelection)
		return;

	switch (m_MouseMode)
	{
	case MOUSE_SELECT_ALL:
		SelectAllCells();
		break;
	case MOUSE_SELECT_COL:
		SelectColumns(currentCell, FALSE);
		break;
	case MOUSE_SELECT_ROW:
		SelectRows(currentCell, FALSE);
		break;
	case MOUSE_SELECT_CELLS:
		SelectCells(currentCell, FALSE);
		break;
	}

	// EFW - Bug fix [REMOVED CJM: this will cause infinite loop in list mode]
	// SetFocusCell(max(currentCell.row, m_nFixedRows), max(currentCell.col, m_nFixedCols));
}

void GridCtrl::ValidateAndModifyCellContents(int nRow, int nCol, LPCTSTR strText)
{
	if (!IsCellEditable(nRow, nCol))
		return;

	if (SendMessageToParent(nRow, nCol, GVN_BEGINLABELEDIT) >= 0)
	{
		const CString strCurrentText = GetItemText(nRow, nCol);
		if (strCurrentText != strText)
		{
			SetItemText(nRow, nCol, strText);
			if (ValidateEdit(nRow, nCol, strText) &&
				SendMessageToParent(nRow, nCol, GVN_ENDLABELEDIT) >= 0)
			{
				SetModified(TRUE, nRow, nCol);
				RedrawCell(nRow, nCol);
			}
			else
			{
				SetItemText(nRow, nCol, strCurrentText);
			}
		}
	}
}

void GridCtrl::ClearCells(GridCellRange Selection)
{
	for (int row = Selection.GetMinRow(); row <= Selection.GetMaxRow(); row++)
	{
		for (int col = Selection.GetMinCol(); col <= Selection.GetMaxCol(); col++)
		{
			// don't clear hidden cells
			if (m_arRowHeights[row] > 0 && m_arColWidths[col] > 0)
			{
				ValidateAndModifyCellContents(row, col, _T(""));
			}
		}
	}
	Refresh();
}

#ifndef GRIDCONTROL_NO_CLIPBOARD

////////////////////////////////////////////////////////////////////////////////////////
// Clipboard functions

// Deletes the contents from the selected cells
void GridCtrl::CutSelectedText()
{
	if (!IsEditable())
		return;

	for (POSITION pos = m_SelectedCellMap.GetStartPosition(); pos != nullptr;)
	{
		DWORD key;
		GridCellID cell;
		m_SelectedCellMap.GetNextAssoc(pos, key, cell);
		ValidateAndModifyCellContents(cell.row, cell.col, _T(""));
	}
}

// Copies text from the selected cells to the clipboard
COleDataSource* GridCtrl::CopyTextFromGrid()
{
	USES_CONVERSION;

	const GridCellRange Selection = GetSelectedCellRange();
	if (!IsValid(Selection))
		return nullptr;

	if (GetVirtualMode())
		SendCacheHintToParent(Selection);

	// Write to shared file (REMEMBER: CF_TEXT is ANSI, not UNICODE, so we need to convert)
	CSharedFile sf(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);

	// Get a tab delimited string to copy to cache
	CString str;
	for (int row = Selection.GetMinRow(); row <= Selection.GetMaxRow(); row++)
	{
		// don't copy hidden cells
		if (m_arRowHeights[row] <= 0)
			continue;

		str.Empty();
		for (int col = Selection.GetMinCol(); col <= Selection.GetMaxCol(); col++)
		{
			// don't copy hidden cells
			if (m_arColWidths[col] <= 0)
				continue;

			const GridCellBase* pCell = GetCell(row, col);
			if (pCell && (pCell->GetState() & GVIS_SELECTED))
			{
				str += pCell->GetText();
			}
			if (col != Selection.GetMaxCol())
				str += _T("\t");
		}

		if (row != Selection.GetMaxRow())
			str += _T("\r\n");

		sf.Write(str, sizeof(TCHAR) * str.GetLength());
	}

	TCHAR c = '\0';
	sf.Write(&c, sizeof(TCHAR));

	if (GetVirtualMode())
		SendCacheHintToParent(GridCellRange(-1, -1, -1, -1));

	const DWORD dwLen = static_cast<DWORD>(sf.GetLength());
	HGLOBAL hMem = sf.Detach();
	if (!hMem)
		return nullptr;

	hMem = GlobalReAlloc(hMem, dwLen, GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	if (!hMem)
		return nullptr;

	// Cache data
	const auto p_source = new COleDataSource();
	p_source->CacheGlobalData(CF_UNICODETEXT, hMem); //modified: CF_TEXT

	return p_source;
}

// Pastes text from the clipboard to the selected cells
BOOL GridCtrl::PasteTextToGrid(GridCellID cell, COleDataObject* pDataObject,
                                BOOL bSelectPastedCells /*=TRUE*/)
{
	if (!IsValid(cell) || !IsCellEditable(cell) || !pDataObject->IsDataAvailable(CF_TEXT))
		return FALSE;

	// Get the text from the COleDataObject
	const HGLOBAL hmem = pDataObject->GetGlobalData(CF_TEXT);
	CMemFile sf(static_cast<BYTE*>(GlobalLock(hmem)), (GlobalSize(hmem)));

	// CF_TEXT is ANSI text, so we need to allocate a char* buffer
	// to hold this.
	const auto sz_buffer = new char[GlobalSize(hmem)]; // FIX: Use LPSTR char here
	if (!sz_buffer)
		return FALSE;

	sf.Read(sz_buffer, static_cast<UINT>(GlobalSize(hmem)));
	GlobalUnlock(hmem);

	// Now store in generic TCHAR form so we no longer have to deal with
	// ANSI/UNICODE problems
	CString strText(sz_buffer);
	delete[] sz_buffer;

	// Parse text data and set in cells...
	strText.LockBuffer();
	CString strLine = strText;
	int nLine = 0;

	// Find the end of the first line
	GridCellRange PasteRange(cell.row, cell.col, -1, -1);
	int nIndex;
	do
	{
		int nColumn = 0;
		nIndex = strLine.Find(_T("\n"));

		// Store the remaining chars after the newline
		const CString strNext = (nIndex < 0) ? _T("") : strLine.Mid(nIndex + 1);

		// Remove all chars after the newline
		if (nIndex >= 0)
			strLine = strLine.Left(nIndex);

		int nLineIndex = strLine.FindOneOf(_T("\t,"));
		CString strCellText = (nLineIndex >= 0) ? strLine.Left(nLineIndex) : strLine;

		// skip hidden rows
		int iRowVis = cell.row + nLine;
		while (iRowVis < GetRowCount())
		{
			if (GetRowHeight(iRowVis) > 0)
				break;
			nLine++;
			iRowVis++;
		}

		while (!strLine.IsEmpty())
		{
			// skip hidden columns
			int iColVis = cell.col + nColumn;
			while (iColVis < GetColumnCount())
			{
				if (GetColumnWidth(iColVis) > 0)
					break;
				nColumn++;
				iColVis++;
			}

			GridCellID TargetCell(iRowVis, iColVis);
			if (IsValid(TargetCell))
			{
				strCellText.TrimLeft();
				strCellText.TrimRight();

				ValidateAndModifyCellContents(TargetCell.row, TargetCell.col, strCellText);

				// Make sure cell is not selected to avoid data loss
				SetItemState(TargetCell.row, TargetCell.col,
				             GetItemState(TargetCell.row, TargetCell.col) & ~GVIS_SELECTED);

				if (iRowVis > PasteRange.GetMaxRow()) PasteRange.SetMaxRow(iRowVis);
				if (iColVis > PasteRange.GetMaxCol()) PasteRange.SetMaxCol(iColVis);
			}

			strLine = (nLineIndex >= 0) ? strLine.Mid(nLineIndex + 1) : _T("");
			nLineIndex = strLine.FindOneOf(_T("\t,"));
			strCellText = (nLineIndex >= 0) ? strLine.Left(nLineIndex) : strLine;

			nColumn++;
		}

		strLine = strNext;
		nLine++;
	}
	while (nIndex >= 0);

	strText.UnlockBuffer();

	if (bSelectPastedCells)
		SetSelectedRange(PasteRange, TRUE);
	else
	{
		ResetSelectedRange();
		Refresh();
	}

	return TRUE;
}
#endif

#ifndef GRIDCONTROL_NO_DRAGDROP

// Start drag n drop
void GridCtrl::OnBeginDrag()
{
	COleDataSource* p_source = nullptr;
	if (!m_bAllowDragAndDrop && m_CurCol == -1)
		return;

	if (m_CurCol >= 0) p_source = new COleDataSource();
	if (!p_source && m_bAllowDragAndDrop) p_source = CopyTextFromGrid();
	if (p_source)
	{
		SendMessageToParent(GetSelectedCellRange().GetTopLeft().row,
		                    GetSelectedCellRange().GetTopLeft().col,
		                    GVN_BEGINDRAG);

		m_MouseMode = MOUSE_DRAGGING;
		m_bLMouseButtonDown = FALSE;

		DROPEFFECT dropEffect = p_source->DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE);

		if (dropEffect & DROPEFFECT_MOVE)
			CutSelectedText();

		if (p_source)
			delete p_source; // Did not pass source to clipboard, so must delete
	}
}

// Handle drag over grid
DROPEFFECT GridCtrl::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState,
                                 CPoint point)
{
	// Find which cell we are over and drop-highlight it
	GridCellID cell = GetCellFromPt(point, FALSE);
	bool Valid;
	// Any text data available for us?
	if (m_CurCol == -1)
	{
		if (m_bDragRowMode)
		{
			Valid = cell.col >= GetFixedColumnCount() && cell.row >= GetFixedRowCount();
		}
		else
		{
			if (!m_bAllowDragAndDrop || !IsEditable() || !pDataObject->IsDataAvailable(CF_TEXT))
				return DROPEFFECT_NONE;
			Valid = IsValid(cell) != 0;
		}
	}
	else
	{
		Valid = cell.col >= GetFixedColumnCount() && cell.row < GetFixedRowCount();
	}

	// If not valid, set the previously drop-highlighted cell as no longer drop-highlighted
	if (!Valid)
	{
		OnDragLeave();
		m_LastDragOverCell = GridCellID(-1, -1);
		return DROPEFFECT_NONE;
	}
	if (m_CurCol == -1)
	{
		if (!m_bDragRowMode && !IsCellEditable(cell))
			return DROPEFFECT_NONE;
	}

	// Have we moved over a different cell than last time?
	if (cell != m_LastDragOverCell)
	{
		// Set the previously drop-highlighted cell as no longer drop-highlighted
		if (IsValid(m_LastDragOverCell))
		{
			const UINT nState = GetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col);
			SetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col,
			             nState & ~GVIS_DROPHILITED);
			RedrawCell(m_LastDragOverCell);
		}

		m_LastDragOverCell = cell;

		// Set the new cell as drop-highlighted
		if (IsValid(m_LastDragOverCell))
		{
			const UINT nState = GetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col);
			SetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col,
			             nState | GVIS_DROPHILITED);
			RedrawCell(m_LastDragOverCell);
		}
	}

	// Return an appropraite value of DROPEFFECT so mouse cursor is set properly
	if (dwKeyState & MK_CONTROL)
		return DROPEFFECT_COPY;
	return DROPEFFECT_MOVE;
}

// Something has just been dragged onto the grid
DROPEFFECT GridCtrl::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState,
                                  CPoint point)
{
	// Any text data available for us?
	m_LastDragOverCell = GetCellFromPt(point, m_CurCol >= 0);
	bool Valid;
	if (m_CurCol == -1)
	{
		if (!m_bAllowDragAndDrop || !pDataObject->IsDataAvailable(CF_TEXT))
			return DROPEFFECT_NONE;

		// Find which cell we are over and drop-highlight it
		if (!IsValid(m_LastDragOverCell))
			return DROPEFFECT_NONE;

		if (!IsCellEditable(m_LastDragOverCell))
			return DROPEFFECT_NONE;
		Valid = IsValid(m_LastDragOverCell) != 0;
	}
	else
	{
		Valid = m_LastDragOverCell.row >= 0 && m_LastDragOverCell.row < GetFixedRowCount() && m_LastDragOverCell.col >=
			GetFixedColumnCount();
	}

	if (Valid)
	{
		UINT nState = GetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col);
		SetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col,
		             nState | GVIS_DROPHILITED);
		RedrawCell(m_LastDragOverCell);
	}

	// Return an appropraite value of DROPEFFECT so mouse cursor is set properly
	if (dwKeyState & MK_CONTROL)
		return DROPEFFECT_COPY;
	return DROPEFFECT_MOVE;
}

// Something has just been dragged away from the grid
void GridCtrl::OnDragLeave()
{
	// Set the previously drop-highlighted cell as no longer drop-highlighted
	if (IsValid(m_LastDragOverCell))
	{
		const UINT nState = GetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col);
		SetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col,
		             nState & ~GVIS_DROPHILITED);
		RedrawCell(m_LastDragOverCell);
	}
}

// Something has just been dropped onto the grid
BOOL GridCtrl::OnDrop(COleDataObject* pDataObject, DROPEFFECT /*dropEffect*/,
                       CPoint /* point */)
{
	m_MouseMode = MOUSE_NOTHING;
	if (m_CurCol == -1)
	{
		if (!m_bAllowDragAndDrop || (!IsCellEditable(m_LastDragOverCell) && !m_bDragRowMode))
			return FALSE;
	}

	OnDragLeave();
	if (m_CurCol >= 0)
	{
		if (m_LastDragOverCell.col == m_CurCol || m_LastDragOverCell.row >= GetFixedRowCount())
			return FALSE;
		int New = m_arColOrder[m_CurCol];
		m_arColOrder.erase(m_arColOrder.begin() + m_CurCol);
		m_arColOrder.insert(m_arColOrder.begin() + m_LastDragOverCell.col, New);
		m_CurCol = -1;
		Invalidate();
		return TRUE;
	}
	if (m_bDragRowMode)
	{
		Reorder(m_CurRow, m_LastDragOverCell.row);
		Invalidate();
		return TRUE;
	}
	return PasteTextToGrid(m_LastDragOverCell, pDataObject, FALSE);
}
#endif

#ifndef GRIDCONTROL_NO_CLIPBOARD
void GridCtrl::OnEditCut()
{
	if (!IsEditable())
		return;

	COleDataSource* p_source = CopyTextFromGrid();
	if (!p_source)
		return;

	p_source->SetClipboard();
	CutSelectedText();
}

void GridCtrl::OnEditCopy()
{
	COleDataSource* p_source = CopyTextFromGrid();
	if (!p_source)
		return;

	p_source->SetClipboard();
}

void GridCtrl::OnEditPaste()
{
	if (!IsEditable())
		return;

	GridCellRange cellRange = GetSelectedCellRange();

	// Get the top-left selected cell, or the Focus cell, or the topleft (non-fixed) cell
	GridCellID cell;
	if (cellRange.IsValid())
	{
		cell.row = cellRange.GetMinRow();
		cell.col = cellRange.GetMinCol();
	}
	else
	{
		cell = GetFocusCell();
		if (!IsValid(cell))
			cell = GetTopleftNonFixedCell();
		if (!IsValid(cell))
			return;
	}

	// If a cell is being edited, then call it's edit window paste function.
	if (IsItemEditing(cell.row, cell.col))
	{
		const GridCellBase* pCell = GetCell(cell.row, cell.col);
		ASSERT(pCell);
		if (!pCell) return;

		CWnd* pEditWnd = pCell->GetEditWnd();
		if (pEditWnd && pEditWnd->IsKindOf(RUNTIME_CLASS(CEdit)))
		{
			static_cast<CEdit*>(pEditWnd)->Paste();
			return;
		}
	}

	// Attach a COleDataObject to the clipboard and paste the data to the grid
	COleDataObject obj;
	if (obj.AttachClipboard())
		PasteTextToGrid(cell, &obj);
}
#endif

void GridCtrl::OnEditSelectAll()
{
	SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, GVN_SELCHANGING);
	SelectAllCells();
	SendMessageToParent(m_idCurrentCell.row, m_idCurrentCell.col, GVN_SELCHANGED);
}

#ifndef GRIDCONTROL_NO_CLIPBOARD
void GridCtrl::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetSelectedCount() > 0); // - Thomas Haase
	//GridCellRange Selection = GetSelectedCellRange();
	//pCmdUI->Enable(Selection.Count() && IsValid(Selection));
}

void GridCtrl::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsEditable() && GetSelectedCount() > 0); // - Thomas Haase
	//GridCellRange Selection = GetSelectedCellRange();
	//pCmdUI->Enable(IsEditable() && Selection.Count() && IsValid(Selection));
}

void GridCtrl::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	GridCellID cell = GetFocusCell();

	const BOOL bCanPaste = IsValid(cell) && IsCellEditable(cell) &&
		IsClipboardFormatAvailable(CF_TEXT);

	pCmdUI->Enable(bCanPaste);
}
#endif

void GridCtrl::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bEnableSelection);
}

////////////////////////////////////////////////////////////////////////////////////////
// hittest-like functions

// TRUE if the mouse is over a row resize area
BOOL GridCtrl::MouseOverRowResizeArea(CPoint& point)
{
	if (point.x >= GetFixedColumnWidth())
		return FALSE;

	const GridCellID idCurrentCell = GetCellFromPt(point);
	CPoint start;
	if (!GetCellOrigin(idCurrentCell, &start))
		return FALSE;

	int endy = start.y + GetRowHeight(idCurrentCell.row);

	if ((point.y - start.y < m_nResizeCaptureRange && idCurrentCell.row != 0) ||
		endy - point.y < m_nResizeCaptureRange)
	{
		return TRUE;
	}
	return FALSE;
}

// TRUE if the mouse is over a column resize area. point is in Client coords
BOOL GridCtrl::MouseOverColumnResizeArea(CPoint& point)
{
	if (point.y >= GetFixedRowHeight())
		return FALSE;

	const GridCellID idCurrentCell = GetCellFromPt(point);
	CPoint start;
	if (!GetCellOrigin(idCurrentCell, &start))
		return FALSE;

	const int endx = start.x + GetColumnWidth(idCurrentCell.col);

	if ((point.x - start.x < m_nResizeCaptureRange && idCurrentCell.col != 0) ||
		endx - point.x < m_nResizeCaptureRange)
	{
		return TRUE;
	}
	return FALSE;
}

// Get cell from point.
// point - client coordinates
// bAllowFixedCellCheck - if TRUE then fixed cells are checked
GridCellID GridCtrl::GetCellFromPt(CPoint point, BOOL bAllowFixedCellCheck /*=TRUE*/)
{
	GridCellID cellID; // return value

	const GridCellID idTopLeft = GetTopleftNonFixedCell();
	if (!bAllowFixedCellCheck && !IsValid(idTopLeft))
		return cellID;

	// calculate column index
	const int fixedColWidth = GetFixedColumnWidth();

	if (point.x < 0 || (!bAllowFixedCellCheck && point.x < fixedColWidth)) // not in window
		cellID.col = -1;
	else if (point.x < fixedColWidth) // in fixed col
	{
		int xpos = 0;
		int col = 0;
		while (col < m_nFixedCols)
		{
			xpos += GetColumnWidth(col);
			if (xpos > point.x)
				break;
			col++;
		}

		cellID.col = col;
	}
	else // in non-fixed col
	{
		int xpos = fixedColWidth;
		int col = idTopLeft.col; //m_nFixedCols;
		while (col < GetColumnCount())
		{
			xpos += GetColumnWidth(col);
			if (xpos > point.x)
				break;
			col++;
		}

		if (col >= GetColumnCount())
			cellID.col = -1;
		else
			cellID.col = col;
	}

	// calculate row index
	const int fixedRowHeight = GetFixedRowHeight();
	if (point.y < 0 || (!bAllowFixedCellCheck && point.y < fixedRowHeight)) // not in window
		cellID.row = -1;
	else if (point.y < fixedRowHeight) // in fixed col
	{
		int ypos = 0;
		int row = 0;
		while (row < m_nFixedRows)
		{
			ypos += GetRowHeight(row);
			if (ypos > point.y)
				break;
			row++;
		}
		cellID.row = row;
	}
	else
	{
		int ypos = fixedRowHeight;
		int row = idTopLeft.row; //m_nFixedRows;
		while (row < GetRowCount())
		{
			ypos += GetRowHeight(row);
			if (ypos > point.y)
				break;
			row++;
		}

		if (row >= GetRowCount())
			cellID.row = -1;
		else
			cellID.row = row;
	}
	return cellID;
}

////////////////////////////////////////////////////////////////////////////////
// GridCtrl cellrange functions

// Gets the first non-fixed cell ID
GridCellID GridCtrl::GetTopleftNonFixedCell(BOOL bForceRecalculation /*=FALSE*/)
{
	// Used cached value if possible
	if (m_idTopLeftCell.IsValid() && !bForceRecalculation)
		return m_idTopLeftCell;

	const int nVertScroll = GetScrollPos(SB_VERT);
	int nHorzScroll = GetScrollPos(SB_HORZ);

	m_idTopLeftCell.col = m_nFixedCols;
	int nRight = 0;
	while (nRight < nHorzScroll && m_idTopLeftCell.col < (GetColumnCount() - 1))
		nRight += GetColumnWidth(m_idTopLeftCell.col++);

	m_idTopLeftCell.row = m_nFixedRows;
	int nTop = 0;
	while (nTop < nVertScroll && m_idTopLeftCell.row < (GetRowCount() - 1))
		nTop += GetRowHeight(m_idTopLeftCell.row++);

	//ATLTRACE2("TopLeft cell is row %d, col %d\n",m_idTopLeftCell.row, m_idTopLeftCell.col);
	return m_idTopLeftCell;
}

// This gets even partially visible cells
GridCellRange GridCtrl::GetVisibleNonFixedCellRange(LPRECT pRect /*=NULL*/,
                                                  BOOL bForceRecalculation /*=FALSE*/)
{
	int i;
	CRect rect;
	GetClientRect(rect);

	GridCellID idTopLeft = GetTopleftNonFixedCell(bForceRecalculation);

	// calc bottom
	int bottom = GetFixedRowHeight();
	for (i = idTopLeft.row; i < GetRowCount(); i++)
	{
		bottom += GetRowHeight(i);
		if (bottom >= rect.bottom)
		{
			bottom = rect.bottom;
			break;
		}
	}
	const int maxVisibleRow = min(i, GetRowCount() - 1);

	// calc right
	int right = GetFixedColumnWidth();
	for (i = idTopLeft.col; i < GetColumnCount(); i++)
	{
		right += GetColumnWidth(i);
		if (right >= rect.right)
		{
			right = rect.right;
			break;
		}
	}
	const int maxVisibleCol = min(i, GetColumnCount() - 1);
	if (pRect)
	{
		pRect->left = pRect->top = 0;
		pRect->right = right;
		pRect->bottom = bottom;
	}

	return GridCellRange(idTopLeft.row, idTopLeft.col, maxVisibleRow, maxVisibleCol);
}

GridCellRange GridCtrl::GetVisibleFixedCellRange(LPRECT pRect /*=NULL*/,
                                               BOOL bForceRecalculation /*=FALSE*/)
{
	int i;
	CRect rect;
	GetClientRect(rect);

	GridCellID idTopLeft = GetTopleftNonFixedCell(bForceRecalculation);

	// calc bottom
	int bottom = GetFixedRowHeight();
	for (i = idTopLeft.row; i < GetRowCount(); i++)
	{
		bottom += GetRowHeight(i);
		if (bottom >= rect.bottom)
		{
			bottom = rect.bottom;
			break;
		}
	}
	const int maxVisibleRow = min(i, GetRowCount() - 1);

	// calc right
	int right = 0;
	for (i = 0; i < GetFixedColumnCount(); i++)
	{
		right += GetColumnWidth(i);
		if (right >= rect.right)
		{
			right = rect.right;
			break;
		}
	}
	const int maxVisibleCol = min(i, GetColumnCount() - 1);
	if (pRect)
	{
		pRect->left = pRect->top = 0;
		pRect->right = right;
		pRect->bottom = bottom;
	}

	return GridCellRange(idTopLeft.row, 0, maxVisibleRow, maxVisibleCol);
}

// used by ResetScrollBars() - This gets only fully visible cells
GridCellRange GridCtrl::GetUnobstructedNonFixedCellRange(BOOL bForceRecalculation /*=FALSE*/)
{
	CRect rect;
	GetClientRect(rect);

	const GridCellID idTopLeft = GetTopleftNonFixedCell(bForceRecalculation);

	// calc bottom
	int i;
	int bottom = GetFixedRowHeight();
	for (i = idTopLeft.row; i < GetRowCount(); i++)
	{
		bottom += GetRowHeight(i);
		if (bottom >= rect.bottom)
			break;
	}
	int maxVisibleRow = min(i, GetRowCount() - 1);
	if (maxVisibleRow > 0 && bottom > rect.bottom)
		maxVisibleRow--;

	// calc right
	int right = GetFixedColumnWidth();
	for (i = idTopLeft.col; i < GetColumnCount(); i++)
	{
		right += GetColumnWidth(i);
		if (right >= rect.right)
			break;
	}
	int maxVisibleCol = min(i, GetColumnCount() - 1);
	if (maxVisibleCol > 0 && right > rect.right)
		maxVisibleCol--;

	return GridCellRange(idTopLeft.row, idTopLeft.col, maxVisibleRow, maxVisibleCol);
}

// Returns the minimum bounding range of the current selection
// If no selection, then the returned GridCellRange will be invalid
GridCellRange GridCtrl::GetSelectedCellRange() const
{
	GridCellRange Selection(GetRowCount(), GetColumnCount(), -1, -1);

	for (POSITION pos = m_SelectedCellMap.GetStartPosition(); pos != nullptr;)
	{
		DWORD key;
		GridCellID cell;
		m_SelectedCellMap.GetNextAssoc(pos, key, cell);

		Selection.SetMinRow(min(Selection.GetMinRow(), cell.row));
		Selection.SetMinCol(min(Selection.GetMinCol(), cell.col));
		Selection.SetMaxRow(__max(Selection.GetMaxRow(), cell.row));
		Selection.SetMaxCol(__max(Selection.GetMaxCol(), cell.col));
	}

	return Selection;
}

// Returns ALL the cells in the grid
GridCellRange GridCtrl::GetCellRange() const
{
	return GridCellRange(0, 0, GetRowCount() - 1, GetColumnCount() - 1);
}

// Resets the selected cell range to the empty set.
void GridCtrl::ResetSelectedRange()
{
	m_PrevSelectedCellMap.RemoveAll();
	SetSelectedRange(-1, -1, -1, -1);
	SetFocusCell(-1, -1);
}

// Get/Set scroll position using 32 bit functions
int GridCtrl::GetScrollPos32(int nBar, BOOL bGetTrackPos /* = FALSE */)
{
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);

	if (bGetTrackPos)
	{
		if (GetScrollInfo(nBar, &si, SIF_TRACKPOS))
			return si.nTrackPos;
	}
	else
	{
		if (GetScrollInfo(nBar, &si, SIF_POS))
			return si.nPos;
	}

	return 0;
}

BOOL GridCtrl::SetScrollPos32(int nBar, int nPos, BOOL bRedraw /* = TRUE */)
{
	m_idTopLeftCell.row = -1;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	si.nPos = nPos;
	return SetScrollInfo(nBar, &si, bRedraw);
}

void GridCtrl::EnableScrollBars(int nBar, BOOL bEnable /*=TRUE*/)
{
	if (bEnable)
	{
		if (!IsVisibleHScroll() && (nBar == SB_HORZ || nBar == SB_BOTH))
		{
			m_nBarState |= GVL_HORZ;
			EnableScrollBarCtrl(SB_HORZ, bEnable);
		}

		if (!IsVisibleVScroll() && (nBar == SB_VERT || nBar == SB_BOTH))
		{
			m_nBarState |= GVL_VERT;
			EnableScrollBarCtrl(SB_VERT, bEnable);
		}
	}
	else
	{
		if (IsVisibleHScroll() && (nBar == SB_HORZ || nBar == SB_BOTH))
		{
			m_nBarState &= ~GVL_HORZ;
			EnableScrollBarCtrl(SB_HORZ, bEnable);
		}

		if (IsVisibleVScroll() && (nBar == SB_VERT || nBar == SB_BOTH))
		{
			m_nBarState &= ~GVL_VERT;
			EnableScrollBarCtrl(SB_VERT, bEnable);
		}
	}
}

// If resizing or cell counts/sizes change, call this - it'll fix up the scroll bars
void GridCtrl::ResetScrollBars()
{
	// Force a refresh.
	m_idTopLeftCell.row = -1;

	if (!m_bAllowDraw || !IsWindow(GetSafeHwnd()))
		return;

	CRect rect;

	// This would have caused OnSize event - Brian
	//EnableScrollBars(SB_BOTH, FALSE);

	GetClientRect(rect);

	if (rect.left == rect.right || rect.top == rect.bottom)
		return;

	if (IsVisibleVScroll())
		rect.right += GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXBORDER);

	if (IsVisibleHScroll())
		rect.bottom += GetSystemMetrics(SM_CYHSCROLL) + GetSystemMetrics(SM_CYBORDER);

	rect.left += GetFixedColumnWidth();
	rect.top += GetFixedRowHeight();

	if (rect.left >= rect.right || rect.top >= rect.bottom)
	{
		EnableScrollBarCtrl(SB_BOTH, FALSE);
		return;
	}

	CRect VisibleRect(GetFixedColumnWidth(), GetFixedRowHeight(),
	                  rect.right, rect.bottom);
	const CRect VirtualRect(GetFixedColumnWidth(), GetFixedRowHeight(),
	                        GetVirtualWidth(), GetVirtualHeight());

	// If vertical scroll bar, horizontal space is reduced
	if (VisibleRect.Height() < VirtualRect.Height())
		VisibleRect.right -= GetSystemMetrics(SM_CXVSCROLL);
	// If horz scroll bar, vert space is reduced
	if (VisibleRect.Width() < VirtualRect.Width())
		VisibleRect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

	// Recheck vertical scroll bar
	if (VisibleRect.Height() < VirtualRect.Height())
	{
		EnableScrollBars(SB_VERT, TRUE);
		m_nVScrollMax = VirtualRect.Height() - 1;
	}
	else
	{
		EnableScrollBars(SB_VERT, FALSE);
		m_nVScrollMax = 0;
	}

	if (VisibleRect.Width() < VirtualRect.Width())
	{
		EnableScrollBars(SB_HORZ, TRUE);
		m_nHScrollMax = VirtualRect.Width() - 1;
	}
	else
	{
		EnableScrollBars(SB_HORZ, FALSE);
		m_nHScrollMax = 0;
	}

	ASSERT(m_nVScrollMax < INT_MAX&& m_nHScrollMax < INT_MAX); // This should be fine

	/* Old code - CJM
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE;
	si.nPage = (m_nHScrollMax>0)? VisibleRect.GetRectWidth() : 0;
	SetScrollInfo(SB_HORZ, &si, FALSE);
	si.nPage = (m_nVScrollMax>0)? VisibleRect.GetRectHeight() : 0;
	SetScrollInfo(SB_VERT, &si, FALSE);

	SetScrollRange(SB_VERT, 0, m_nVScrollMax, TRUE);
	SetScrollRange(SB_HORZ, 0, m_nHScrollMax, TRUE);
	*/

	// New code - Paul Runstedler
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE;
	si.nPage = (m_nHScrollMax > 0) ? VisibleRect.Width() : 0;
	si.nMin = 0;
	si.nMax = m_nHScrollMax;
	SetScrollInfo(SB_HORZ, &si, TRUE);

	si.fMask |= SIF_DISABLENOSCROLL;
	si.nPage = (m_nVScrollMax > 0) ? VisibleRect.Height() : 0;
	si.nMin = 0;
	si.nMax = m_nVScrollMax;
	SetScrollInfo(SB_VERT, &si, TRUE);
}

////////////////////////////////////////////////////////////////////////////////////
// Row/Column position functions

// returns the top left point of the cell. Returns FALSE if cell not visible.
BOOL GridCtrl::GetCellOrigin(int nRow, int nCol, LPPOINT p)
{
	int i;

	if (!IsValid(nRow, nCol))
		return FALSE;

	GridCellID idTopLeft;
	if (nCol >= m_nFixedCols || nRow >= m_nFixedRows)
		idTopLeft = GetTopleftNonFixedCell();

	if ((nRow >= m_nFixedRows && nRow < idTopLeft.row) ||
		(nCol >= m_nFixedCols && nCol < idTopLeft.col))
		return FALSE;

	p->x = 0;
	if (nCol < m_nFixedCols) // is a fixed column
		for (i = 0; i < nCol; i++)
			p->x += GetColumnWidth(i);
	else
	{
		// is a scrollable data column
		for (i = 0; i < m_nFixedCols; i++)
			p->x += GetColumnWidth(i);
		for (i = idTopLeft.col; i < nCol; i++)
			p->x += GetColumnWidth(i);
	}

	p->y = 0;
	if (nRow < m_nFixedRows) // is a fixed row
		for (i = 0; i < nRow; i++)
			p->y += GetRowHeight(i);
	else
	{
		// is a scrollable data row
		for (i = 0; i < m_nFixedRows; i++)
			p->y += GetRowHeight(i);
		for (i = idTopLeft.row; i < nRow; i++)
			p->y += GetRowHeight(i);
	}

	return TRUE;
}

BOOL GridCtrl::GetCellOrigin(const GridCellID& cell, LPPOINT p)
{
	return GetCellOrigin(cell.row, cell.col, p);
}

// Returns the bounding box of the cell
BOOL GridCtrl::GetCellRect(const GridCellID& cell, LPRECT pRect)
{
	return GetCellRect(cell.row, cell.col, pRect);
}

BOOL GridCtrl::GetCellRect(int nRow, int nCol, LPRECT pRect)
{
	CPoint CellOrigin;
	if (!GetCellOrigin(nRow, nCol, &CellOrigin))
		return FALSE;

	pRect->left = CellOrigin.x;
	pRect->top = CellOrigin.y;
	pRect->right = CellOrigin.x + GetColumnWidth(nCol) - 1;
	pRect->bottom = CellOrigin.y + GetRowHeight(nRow) - 1;

	//ATLTRACE2("Row %d, col %d: L %d, T %d, W %d, H %d:  %d,%d - %d,%d\n",
	//      nRow,nCol, CellOrigin.x, CellOrigin.y, GetColumnWidth(nCol), GetRowHeight(nRow),
	//      pRect->left, pRect->top, pRect->right, pRect->bottom);

	return TRUE;
}

BOOL GridCtrl::GetTextRect(const GridCellID& cell, LPRECT pRect)
{
	return GetTextRect(cell.row, cell.col, pRect);
}

BOOL GridCtrl::GetTextRect(int nRow, int nCol, LPRECT pRect)
{
	GridCellBase* pCell = GetCell(nRow, nCol);
	if (pCell == nullptr)
		return FALSE;

	if (!GetCellRect(nRow, nCol, pRect))
		return FALSE;

	return pCell->GetTextRect(pRect);
}

// Returns the bounding box of a range of cells
BOOL GridCtrl::GetCellRangeRect(const GridCellRange& cellRange, LPRECT lpRect)
{
	CPoint MinOrigin, MaxOrigin;

	if (!GetCellOrigin(cellRange.GetMinRow(), cellRange.GetMinCol(), &MinOrigin))
		return FALSE;
	if (!GetCellOrigin(cellRange.GetMaxRow(), cellRange.GetMaxCol(), &MaxOrigin))
		return FALSE;

	lpRect->left = MinOrigin.x;
	lpRect->top = MinOrigin.y;
	lpRect->right = MaxOrigin.x + GetColumnWidth(cellRange.GetMaxCol()) - 1;
	lpRect->bottom = MaxOrigin.y + GetRowHeight(cellRange.GetMaxRow()) - 1;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
// Grid attribute functions

LRESULT GridCtrl::OnSetFont(WPARAM hFont, LPARAM /*lParam */)
{
	LRESULT result = Default();

	// Get the logical font
	LOGFONT lf;
	if (!GetObject((HFONT)hFont, sizeof(LOGFONT), &lf))
		return result;

	m_cellDefault.SetFont(&lf);
	m_cellFixedColDef.SetFont(&lf);
	m_cellFixedRowDef.SetFont(&lf);
	m_cellFixedRowColDef.SetFont(&lf);

	Refresh();

	return result;
}

LRESULT GridCtrl::OnGetFont(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//LOGFONT    lf;
	//m_cellDefault.GetFontObject()->GetLogFont(&lf);

	return (LRESULT)m_cellDefault.GetFontObject()->GetSafeHandle();
}

#ifndef _WIN32_WCE_NO_CURSOR
BOOL GridCtrl::OnSetCursor(CWnd* p_wnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		switch (m_MouseMode)
		{
		case MOUSE_OVER_COL_DIVIDE:
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
			break;
		case MOUSE_OVER_ROW_DIVIDE:
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
			break;
#ifndef GRIDCONTROL_NO_DRAGDROP
		case MOUSE_DRAGGING:
			break;
#endif
		default:
			if (!GetVirtualMode())
			{
				CPoint pt(GetMessagePos());
				ScreenToClient(&pt);
				GridCellID cell = GetCellFromPt(pt);
				if (IsValid(cell))
				{
					GridCellBase* pCell = GetCell(cell.row, cell.col);
					if (pCell)
						return pCell->OnSetCursor();
				}
			}

			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		}
		return TRUE;
	}

	return CWnd::OnSetCursor(p_wnd, nHitTest, message);
}
#endif

////////////////////////////////////////////////////////////////////////////////////
// Row/Column count functions

BOOL GridCtrl::SetFixedRowCount(int nFixedRows)
{
	if (m_nFixedRows == nFixedRows)
		return TRUE;

	ASSERT(nFixedRows >= 0);

	ResetSelectedRange();

	// Force recalculation
	m_idTopLeftCell.col = -1;

	if (nFixedRows > GetRowCount())
		if (!SetRowCount(nFixedRows))
			return FALSE;

	if (m_idCurrentCell.row < nFixedRows)
		SetFocusCell(-1, -1);

	if (!GetVirtualMode())
	{
		if (nFixedRows > m_nFixedRows)
		{
			for (int i = m_nFixedRows; i < nFixedRows; i++)
				for (int j = 0; j < GetColumnCount(); j++)
				{
					SetItemState(i, j, GetItemState(i, j) | GVIS_FIXED | GVIS_FIXEDROW);
					SetItemBkColour(i, j, CLR_DEFAULT);
					SetItemFgColour(i, j, CLR_DEFAULT);
				}
		}
		else
		{
			for (int i = nFixedRows; i < m_nFixedRows; i++)
			{
				int j;
				for (j = 0; j < GetFixedColumnCount(); j++)
					SetItemState(i, j, GetItemState(i, j) & ~GVIS_FIXEDROW);

				for (j = GetFixedColumnCount(); j < GetColumnCount(); j++)
				{
					SetItemState(i, j, GetItemState(i, j) & ~(GVIS_FIXED | GVIS_FIXEDROW));
					SetItemBkColour(i, j, CLR_DEFAULT);
					SetItemFgColour(i, j, CLR_DEFAULT);
				}
			}
		}
	}

	m_nFixedRows = nFixedRows;

	Refresh();

	return TRUE;
}

BOOL GridCtrl::SetFixedColumnCount(int nFixedCols)
{
	if (m_nFixedCols == nFixedCols)
		return TRUE;

	ASSERT(nFixedCols >= 0);

	if (nFixedCols > GetColumnCount())
		if (!SetColumnCount(nFixedCols))
			return FALSE;

	if (m_idCurrentCell.col < nFixedCols)
		SetFocusCell(-1, -1);

	ResetSelectedRange();

	// Force recalculation
	m_idTopLeftCell.col = -1;

	if (!GetVirtualMode())
	{
		if (nFixedCols > m_nFixedCols)
		{
			for (int i = 0; i < GetRowCount(); i++)
				for (int j = m_nFixedCols; j < nFixedCols; j++)
				{
					SetItemState(i, j, GetItemState(i, j) | GVIS_FIXED | GVIS_FIXEDCOL);
					SetItemBkColour(i, j, CLR_DEFAULT);
					SetItemFgColour(i, j, CLR_DEFAULT);
				}
		}
		else
		{
			{
				// Scope limit i,j
				for (int i = 0; i < GetFixedRowCount(); i++)
					for (int j = nFixedCols; j < m_nFixedCols; j++)
						SetItemState(i, j, GetItemState(i, j) & ~GVIS_FIXEDCOL);
			}
			{
				// Scope limit i,j
				for (int i = GetFixedRowCount(); i < GetRowCount(); i++)
					for (int j = nFixedCols; j < m_nFixedCols; j++)
					{
						SetItemState(i, j, GetItemState(i, j) & ~(GVIS_FIXED | GVIS_FIXEDCOL));
						SetItemBkColour(i, j, CLR_DEFAULT);
						SetItemFgColour(i, j, CLR_DEFAULT);
					}
			}
		}
	}

	m_nFixedCols = nFixedCols;

	Refresh();

	return TRUE;
}

BOOL GridCtrl::SetRowCount(int nRows)
{
	BOOL bResult = TRUE;

	ASSERT(nRows >= 0);
	if (nRows == GetRowCount())
		return bResult;

	// Force recalculation
	m_idTopLeftCell.col = -1;

	if (nRows < m_nFixedRows)
		m_nFixedRows = nRows;

	if (m_idCurrentCell.row >= nRows)
		SetFocusCell(-1, -1);

	int addedRows = nRows - GetRowCount();

	// If we are about to lose rows, then we need to delete the GridCell objects
	// in each column within each row
	if (addedRows < 0)
	{
		if (!GetVirtualMode())
		{
			for (int row = nRows; row < m_nRows; row++)
			{
				// Delete cells
				for (int col = 0; col < m_nCols; col++)
					DestroyCell(row, col);

				// Delete rows
				const GRID_ROW* pRow = m_RowData[row];
				delete pRow;
			}
		}
		m_nRows = nRows;
	}

	TRY
		{
			m_arRowHeights.SetSize(nRows);

			if (GetVirtualMode())
			{
				m_nRows = nRows;
				if (addedRows > 0)
				{
					const int startRow = nRows - addedRows;
					for (int row = startRow; row < nRows; row++)
						m_arRowHeights[row] = m_cellDefault.GetHeight();
				}
				ResetVirtualOrder();
			}
			else
			{
				// Change the number of rows.
				m_RowData.SetSize(nRows);

				// If we have just added rows, we need to construct new elements for each cell
				// and set the default row height
				if (addedRows > 0)
				{
					// initialize row heights and data
					const int startRow = nRows - addedRows;
					for (int row = startRow; row < nRows; row++)
					{
						m_arRowHeights[row] = m_cellDefault.GetHeight();

						m_RowData[row] = new GRID_ROW;
						m_RowData[row]->SetSize(m_nCols);
						for (int col = 0; col < m_nCols; col++)
						{
							GRID_ROW* pRow = m_RowData[row];
							if (pRow && !GetVirtualMode())
								pRow->SetAt(col, CreateCell(row, col));
						}
						m_nRows++;
					}
				}
			}
		}
	CATCH(CMemoryException, e)
		{
			e->ReportError();
			bResult = FALSE;
		}
	END_CATCH

	SetModified();
	ResetScrollBars();
	Refresh();

	return bResult;
}

BOOL GridCtrl::SetColumnCount(int nCols)
{
	BOOL bResult = TRUE;

	ASSERT(nCols >= 0);

	if (nCols == GetColumnCount())
		return bResult;

	// Force recalculation
	m_idTopLeftCell.col = -1;

	if (nCols < m_nFixedCols)
		m_nFixedCols = nCols;

	if (m_idCurrentCell.col >= nCols)
		SetFocusCell(-1, -1);

	int addedCols = nCols - GetColumnCount();

	// If we are about to lose columns, then we need to delete the GridCell objects
	// within each column
	if (addedCols < 0 && !GetVirtualMode())
	{
		for (int row = 0; row < m_nRows; row++)
			for (int col = nCols; col < GetColumnCount(); col++)
				DestroyCell(row, col);
	}

	TRY
		{
			// Change the number of columns.
			m_arColWidths.SetSize(nCols);

			// Change the number of columns in each row.
			if (!GetVirtualMode())
				for (int i = 0; i < m_nRows; i++)
					if (m_RowData[i])
						m_RowData[i]->SetSize(nCols);

			// If we have just added columns, we need to construct new elements for each cell
			// and set the default column width
			if (addedCols > 0)
			{
				int row, col;

				// initialized column widths
				const int startCol = nCols - addedCols;
				for (col = startCol; col < nCols; col++)
					m_arColWidths[col] = m_cellFixedColDef.GetWidth();

				// initialise column data
				if (!GetVirtualMode())
				{
					for (row = 0; row < m_nRows; row++)
						for (col = startCol; col < nCols; col++)
						{
							GRID_ROW* pRow = m_RowData[row];
							if (pRow)
								pRow->SetAt(col, CreateCell(row, col));
						}
				}
			}
			// else    // check for selected cell ranges
			//    ResetSelectedRange();
		}
	CATCH(CMemoryException, e)
		{
			e->ReportError();
			bResult = FALSE;
		}
	END_CATCH

	m_arColOrder.resize(nCols); // Reset Column Order
	for (int i = 0; i < nCols; i++)
	{
		m_arColOrder[i] = i;
	}

	m_nCols = nCols;

	SetModified();
	ResetScrollBars();
	Refresh();

	return bResult;
}

// Insert a column at a given position, or add to end of columns (if nColumn = -1)
int GridCtrl::InsertColumn(LPCTSTR strHeading,
                            UINT nFormat /* = DT_CENTER|DT_VCENTER|DT_SINGLELINE */,
                            int nColumn /* = -1 */)
{
	ASSERT(!m_AllowReorderColumn); // function not implemented in case of m_AllowReorderColumn option
	if (nColumn >= 0 && nColumn < m_nFixedCols)
	{
		// TODO: Fix it so column insertion works for in the fixed column area
		ASSERT(FALSE);
		return -1;
	}

	// If the insertion is for a specific column, check it's within range.
	if (nColumn >= 0 && nColumn > GetColumnCount())
		return -1;

	// Force recalculation
	m_idTopLeftCell.col = -1;

	ResetSelectedRange();

	// Gotta be able to at least _see_ some of the column.
	if (m_nRows < 1)
		SetRowCount(1);

	// Allow the user to insert after the last of the columns, but process it as a
	// "-1" column, meaning it gets flaged as being the last column, and not a regular
	// "insert" routine.
	if (nColumn == GetColumnCount())
		nColumn = -1;

	TRY
		{
			if (nColumn < 0)
			{
				nColumn = m_nCols;
				m_arColWidths.Add(0);
				if (!GetVirtualMode())
				{
					for (int row = 0; row < m_nRows; row++)
					{
						GRID_ROW* pRow = m_RowData[row];
						if (!pRow)
							return -1;
						pRow->Add(CreateCell(row, nColumn));
					}
				}
			}
			else
			{
				m_arColWidths.InsertAt(nColumn, static_cast<UINT>(0));
				if (!GetVirtualMode())
				{
					for (int row = 0; row < m_nRows; row++)
					{
						GRID_ROW* pRow = m_RowData[row];
						if (!pRow)
							return -1;
						pRow->InsertAt(nColumn, CreateCell(row, nColumn));
					}
				}
			}
		}
	CATCH(CMemoryException, e)
		{
			e->ReportError();
			return -1;
		}
	END_CATCH

	m_nCols++;

	// Initialise column data
	SetItemText(0, nColumn, strHeading);
	for (int row = 0; row < m_nRows; row++)
		SetItemFormat(row, nColumn, nFormat);

	// initialized column width
	m_arColWidths[nColumn] = GetTextExtent(0, nColumn, strHeading).cx;

	if (m_idCurrentCell.col != -1 && nColumn < m_idCurrentCell.col)
		m_idCurrentCell.col++;

	ResetScrollBars();

	SetModified();

	return nColumn;
}

// Insert a row at a given position, or add to end of rows (if nRow = -1)
int GridCtrl::InsertRow(LPCTSTR strHeading, int nRow /* = -1 */)
{
	if (nRow >= 0 && nRow < m_nFixedRows)
	{
		// TODO: Fix it so column insertion works for in the fixed row area
		ASSERT(FALSE);
		return -1;
	}

	// If the insertion is for a specific row, check it's within range.
	if (nRow >= 0 && nRow >= GetRowCount())
		return -1;

	// Force recalculation
	m_idTopLeftCell.col = -1;

	ResetSelectedRange();

	// Gotta be able to at least _see_ some of the row.
	if (m_nCols < 1)
		SetColumnCount(1);

	TRY
		{
			// Adding a row to the bottom
			if (nRow < 0)
			{
				nRow = m_nRows;
				m_arRowHeights.Add(0);
				if (!GetVirtualMode())
					m_RowData.Add(new GRID_ROW);
				else
					m_arRowOrder.push_back(m_nRows);
			}
			else
			{
				m_arRowHeights.InsertAt(nRow, static_cast<UINT>(0));
				if (!GetVirtualMode())
					m_RowData.InsertAt(nRow, new GRID_ROW);
				else
				{
					ResetVirtualOrder();
				}
			}

			if (!GetVirtualMode())
				m_RowData[nRow]->SetSize(m_nCols);
		}
	CATCH(CMemoryException, e)
		{
			e->ReportError();
			return -1;
		}
	END_CATCH

	m_nRows++;

	// Initialise cell data
	if (!GetVirtualMode())
	{
		for (int col = 0; col < m_nCols; col++)
		{
			GRID_ROW* pRow = m_RowData[nRow];
			if (!pRow)
				return -1;
			pRow->SetAt(col, CreateCell(nRow, col));
		}
	}

	// Set row title
	SetItemText(nRow, 0, strHeading);

	// initialized row height
	if (strHeading && strHeading[0])
		m_arRowHeights[nRow] = GetTextExtent(nRow, 0, strHeading).cy;
	else
		m_arRowHeights[nRow] = m_cellFixedRowDef.GetHeight();

	if (m_idCurrentCell.row != -1 && nRow < m_idCurrentCell.row)
		m_idCurrentCell.row++;

	ResetScrollBars();

	SetModified();

	return nRow;
}

///////////////////////////////////////////////////////////////////////////////
// Cell creation stuff

BOOL GridCtrl::SetCellType(int nRow, int nCol, CRuntimeClass* pRuntimeClass)
{
	if (GetVirtualMode())
		return FALSE;

	ASSERT(IsValid(nRow, nCol));
	if (!IsValid(nRow, nCol))
		return FALSE;

	if (!pRuntimeClass->IsDerivedFrom(RUNTIME_CLASS(GridCellBase)))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	auto pNewCell = static_cast<GridCellBase*>(pRuntimeClass->CreateObject());

	GridCellBase* pCurrCell = GetCell(nRow, nCol);
	if (pCurrCell)
		*pNewCell = *pCurrCell;

	SetCell(nRow, nCol, pNewCell);
	delete pCurrCell;

	return TRUE;
}

BOOL GridCtrl::SetDefaultCellType(CRuntimeClass* pRuntimeClass)
{
	if (pRuntimeClass && !pRuntimeClass->IsDerivedFrom(RUNTIME_CLASS(GridCellBase)))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	m_pRtcDefault = pRuntimeClass;
	return TRUE;
}

// Creates a new grid cell and performs any necessary initialisation
/*virtual*/
GridCellBase* GridCtrl::CreateCell(int nRow, int nCol)
{
	ASSERT(!GetVirtualMode());

	if (!m_pRtcDefault || !m_pRtcDefault->IsDerivedFrom(RUNTIME_CLASS(GridCellBase)))
	{
		ASSERT(FALSE);
		return nullptr;
	}
	const auto pCell = static_cast<GridCellBase*>(m_pRtcDefault->CreateObject());
	if (!pCell)
		return nullptr;

	pCell->SetGrid(this);
	pCell->SetCoords(nRow, nCol);

	if (nCol < m_nFixedCols)
		pCell->SetState(pCell->GetState() | GVIS_FIXED | GVIS_FIXEDCOL);
	if (nRow < m_nFixedRows)
		pCell->SetState(pCell->GetState() | GVIS_FIXED | GVIS_FIXEDROW);

	pCell->SetFormat(pCell->GetDefaultCell()->GetFormat());

	return pCell;
}

// Performs any cell cleanup necessary to maintain grid integrity
/*virtual*/
void GridCtrl::DestroyCell(int nRow, int nCol)
{
	// Should NEVER get here in virtual mode.
	ASSERT(!GetVirtualMode());

	// Set the cells state to 0. If the cell is selected, this
	// will remove the cell from the selected list.
	SetItemState(nRow, nCol, 0);

	delete GetCell(nRow, nCol);
}

BOOL GridCtrl::DeleteColumn(int nColumn)
{
	if (nColumn < 0 || nColumn >= GetColumnCount())
		return FALSE;

	ResetSelectedRange();

	if (!GetVirtualMode())
	{
		for (int row = 0; row < GetRowCount(); row++)
		{
			GRID_ROW* pRow = m_RowData[row];
			if (!pRow)
				return FALSE;

			DestroyCell(row, nColumn);

			pRow->RemoveAt(nColumn);
		}
	}
	m_arColWidths.RemoveAt(nColumn);
	m_nCols--;
	if (nColumn < m_nFixedCols)
		m_nFixedCols--;

	if (nColumn == m_idCurrentCell.col)
		m_idCurrentCell.row = m_idCurrentCell.col = -1;
	else if (nColumn < m_idCurrentCell.col)
		m_idCurrentCell.col--;

	ResetScrollBars();

	SetModified();

	return TRUE;
}

void GridCtrl::AddSubVirtualRow(int Num, int Nb)
{
	ASSERT(Nb == -1 || Nb == 1); // only these vlaues are implemented now
	if (!GetVirtualMode()) return;
	for (int ind = 0; ind < m_nRows; ind++)
		if (m_arRowOrder[ind] > Num) m_arRowOrder[ind] += Nb;
	if (Nb > 0)
		m_arRowOrder.insert(m_arRowOrder.begin() + Num, Num);
	else
		m_arRowOrder.erase(m_arRowOrder.begin() + Num);
}

BOOL GridCtrl::DeleteRow(int nRow)
{
	if (nRow < 0 || nRow >= GetRowCount())
		return FALSE;

	ResetSelectedRange();

	if (!GetVirtualMode())
	{
		const GRID_ROW* pRow = m_RowData[nRow];
		if (!pRow)
			return FALSE;

		for (int col = 0; col < GetColumnCount(); col++)
			DestroyCell(nRow, col);

		delete pRow;
		m_RowData.RemoveAt(nRow);
	}
	else
		AddSubVirtualRow(nRow, -1);

	m_arRowHeights.RemoveAt(nRow);

	m_nRows--;
	if (nRow < m_nFixedRows)
		m_nFixedRows--;

	if (nRow == m_idCurrentCell.row)
		m_idCurrentCell.row = m_idCurrentCell.col = -1;
	else if (nRow < m_idCurrentCell.row)
		m_idCurrentCell.row--;

	ResetScrollBars();

	SetModified();

	return TRUE;
}

// Handy function that removes all non-fixed rows
BOOL GridCtrl::DeleteNonFixedRows()
{
	ResetSelectedRange();
	const int nFixed = GetFixedRowCount();
	const int nCount = GetRowCount();
	if (GetVirtualMode())
	{
		if (nCount != nFixed)
		{
			SetRowCount(nFixed);
			m_arRowOrder.resize(nFixed);
			m_arRowHeights.SetSize(nFixed);
			m_idCurrentCell.row = m_idCurrentCell.col = -1;
			ResetScrollBars();
			SetModified();
		}
	}
	else
	{
		// Delete all data rows
		for (int nRow = nCount; nRow >= nFixed; nRow--)
			DeleteRow(nRow);
	}
	return TRUE;
}

// Removes all rows, columns and data from the grid.
BOOL GridCtrl::DeleteAllItems()
{
	ResetSelectedRange();

	m_arColWidths.RemoveAll();
	m_arRowHeights.RemoveAll();

	// Delete all cells in the grid
	if (!GetVirtualMode())
	{
		for (int row = 0; row < m_nRows; row++)
		{
			for (int col = 0; col < m_nCols; col++)
				DestroyCell(row, col);

			const GRID_ROW* pRow = m_RowData[row];
			delete pRow;
		}

		// Remove all rows
		m_RowData.RemoveAll();
	}
	else
		m_arRowOrder.clear();

	m_idCurrentCell.row = m_idCurrentCell.col = -1;
	m_nRows = m_nFixedRows = m_nCols = m_nFixedCols = 0;

	ResetScrollBars();

	SetModified();

	return TRUE;
}

void GridCtrl::AutoFill()
{
	if (!IsWindow(m_hWnd))
		return;

	CRect rect;
	GetClientRect(rect);

	SetColumnCount(rect.Width() / m_cellDefault.GetWidth() + 1);
	SetRowCount(rect.Height() / m_cellDefault.GetHeight() + 1);
	SetFixedRowCount(1);
	SetFixedColumnCount(1);
	ExpandToFit();
}

/////////////////////////////////////////////////////////////////////////////
// GridCtrl data functions

// Set CListCtrl::GetNextItem for details
GridCellID GridCtrl::GetNextItem(GridCellID& cell, int n_flags) const
{
	if ((n_flags & GVNI_ALL) == GVNI_ALL)
	{
		// GVNI_ALL Search whole Grid beginning from cell
		//          First row (cell.row) -- ONLY Columns to the right of cell
		//          following rows       -- ALL  Columns
		int row = cell.row, col = cell.col + 1;
		if (row <= 0)
			row = GetFixedRowCount();
		for (; row < GetRowCount(); row++)
		{
			if (col <= 0)
				col = GetFixedColumnCount();
			for (; col < GetColumnCount(); col++)
			{
				const int nState = GetItemState(row, col);
				if ((n_flags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) ||
					(n_flags & GVNI_FOCUSED && nState & GVIS_FOCUSED) ||
					(n_flags & GVNI_SELECTED && nState & GVIS_SELECTED) ||
					(n_flags & GVNI_READONLY && nState & GVIS_READONLY) ||
					(n_flags & GVNI_FIXED && nState & GVIS_FIXED) ||
					(n_flags & GVNI_MODIFIED && nState & GVIS_MODIFIED))
					return GridCellID(row, col);
			}
			// go to First Column
			col = GetFixedColumnCount();
		}
	}
	else if ((n_flags & GVNI_BELOW) == GVNI_BELOW &&
		(n_flags & GVNI_TORIGHT) == GVNI_TORIGHT)
	{
		// GVNI_AREA Search Grid beginning from cell to Lower-Right of Grid
		//           Only rows starting with  cell.row and below
		//           All rows   -- ONLY Columns to the right of cell
		int row = cell.row;
		if (row <= 0)
			row = GetFixedRowCount();
		for (; row < GetRowCount(); row++)
		{
			int col = cell.col + 1;
			if (col <= 0)
				col = GetFixedColumnCount();
			for (; col < GetColumnCount(); col++)
			{
				const int nState = GetItemState(row, col);
				if ((n_flags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) ||
					(n_flags & GVNI_FOCUSED && nState & GVIS_FOCUSED) ||
					(n_flags & GVNI_SELECTED && nState & GVIS_SELECTED) ||
					(n_flags & GVNI_READONLY && nState & GVIS_READONLY) ||
					(n_flags & GVNI_FIXED && nState & GVIS_FIXED) ||
					(n_flags & GVNI_MODIFIED && nState & GVIS_MODIFIED))
					return GridCellID(row, col);
			}
		}
	}
	else if ((n_flags & GVNI_ABOVE) == GVNI_ABOVE)
	{
		for (int row = cell.row - 1; row >= GetFixedRowCount(); row--)
		{
			const int nState = GetItemState(row, cell.col);
			if ((n_flags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) ||
				(n_flags & GVNI_FOCUSED && nState & GVIS_FOCUSED) ||
				(n_flags & GVNI_SELECTED && nState & GVIS_SELECTED) ||
				(n_flags & GVNI_READONLY && nState & GVIS_READONLY) ||
				(n_flags & GVNI_FIXED && nState & GVIS_FIXED) ||
				(n_flags & GVNI_MODIFIED && nState & GVIS_MODIFIED))
				return GridCellID(row, cell.col);
		}
	}
	else if ((n_flags & GVNI_BELOW) == GVNI_BELOW)
	{
		for (int row = cell.row + 1; row < GetRowCount(); row++)
		{
			const int nState = GetItemState(row, cell.col);
			if ((n_flags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) ||
				(n_flags & GVNI_FOCUSED && nState & GVIS_FOCUSED) ||
				(n_flags & GVNI_SELECTED && nState & GVIS_SELECTED) ||
				(n_flags & GVNI_READONLY && nState & GVIS_READONLY) ||
				(n_flags & GVNI_FIXED && nState & GVIS_FIXED) ||
				(n_flags & GVNI_MODIFIED && nState & GVIS_MODIFIED))
				return GridCellID(row, cell.col);
		}
	}
	else if ((n_flags & GVNI_TOLEFT) == GVNI_TOLEFT)
	{
		for (int col = cell.col - 1; col >= GetFixedColumnCount(); col--)
		{
			const int nState = GetItemState(cell.row, col);
			if ((n_flags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) ||
				(n_flags & GVNI_FOCUSED && nState & GVIS_FOCUSED) ||
				(n_flags & GVNI_SELECTED && nState & GVIS_SELECTED) ||
				(n_flags & GVNI_READONLY && nState & GVIS_READONLY) ||
				(n_flags & GVNI_FIXED && nState & GVIS_FIXED) ||
				(n_flags & GVNI_MODIFIED && nState & GVIS_MODIFIED))
				return GridCellID(cell.row, col);
		}
	}
	else if ((n_flags & GVNI_TORIGHT) == GVNI_TORIGHT)
	{
		for (int col = cell.col + 1; col < GetColumnCount(); col++)
		{
			const int nState = GetItemState(cell.row, col);
			if ((n_flags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) ||
				(n_flags & GVNI_FOCUSED && nState & GVIS_FOCUSED) ||
				(n_flags & GVNI_SELECTED && nState & GVIS_SELECTED) ||
				(n_flags & GVNI_READONLY && nState & GVIS_READONLY) ||
				(n_flags & GVNI_FIXED && nState & GVIS_FIXED) ||
				(n_flags & GVNI_MODIFIED && nState & GVIS_MODIFIED))
				return GridCellID(cell.row, col);
		}
	}

	return GridCellID(-1, -1);
}

// Sorts on a given column using the cell text
BOOL GridCtrl::SortTextItems(int nCol, BOOL bAscending, LPARAM data /* = 0 */)
{
	return SortItems(pfnCellTextCompare, nCol, bAscending, data);
}

void GridCtrl::SetCompareFunction(PFNLVCOMPARE pfnCompare)
{
	m_pfnCompare = pfnCompare;
}

// Sorts on a given column using the cell text and using the specified comparison
// function
BOOL GridCtrl::SortItems(int nCol, BOOL bAscending, LPARAM data /* = 0 */)
{
	SetSortColumn(nCol);
	SetSortAscending(bAscending);
	ResetSelectedRange();
	SetFocusCell(-1, -1);

	if (m_pfnCompare == nullptr)
		return SortItems(pfnCellTextCompare, nCol, bAscending, data);
	return SortItems(m_pfnCompare, nCol, bAscending, data);
}

// Sorts on a given column using the supplied compare function (see CListCtrl::SortItems)
BOOL GridCtrl::SortItems(PFNLVCOMPARE pfnCompare, int nCol, BOOL bAscending,
                          LPARAM data /* = 0 */)
{
	SetSortColumn(nCol);
	SetSortAscending(bAscending);
	ResetSelectedRange();
	SetFocusCell(-1, -1);
	return SortItems(pfnCompare, nCol, bAscending, data, GetFixedRowCount(), -1);
}

int CALLBACK GridCtrl::pfnCellTextCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	UNUSED_ALWAYS(lParamSort);

	const auto pCell1 = reinterpret_cast<GridCellBase*>(lParam1);
	const auto pCell2 = reinterpret_cast<GridCellBase*>(lParam2);
	if (!pCell1 || !pCell2) return 0;

	return _tcscmp(pCell1->GetText(), pCell2->GetText());
}

int CALLBACK GridCtrl::pfnCellNumericCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	UNUSED_ALWAYS(lParamSort);

	const auto pCell1 = reinterpret_cast<GridCellBase*>(lParam1);
	const auto pCell2 = reinterpret_cast<GridCellBase*>(lParam2);
	if (!pCell1 || !pCell2) return 0;

	const int nValue1 = _ttol(pCell1->GetText());
	const int nValue2 = _ttol(pCell2->GetText());

	if (nValue1 < nValue2)
		return -1;
	if (nValue1 == nValue2)
		return 0;
	return 1;
}

GridCtrl* GridCtrl::m_This;
// private recursive sort implementation
bool GridCtrl::NotVirtualCompare(int c1, int c2)
{
	/*return ! GridCtrl::m_This->m_pfnVirtualCompare(c1, c2);*/
	return m_This->m_pfnVirtualCompare(c2, c1);
}

BOOL GridCtrl::SortItems(PFNLVCOMPARE pfnCompare, int nCol, BOOL bAscending, LPARAM data,
                          int low, int high)
{
	if (nCol >= GetColumnCount())
		return FALSE;

	if (high == -1)
		high = GetRowCount() - 1;

	int lo = low;
	int hi = high;

	if (hi <= lo)
		return FALSE;

	if (GetVirtualMode())
	{
		ASSERT(m_pfnVirtualCompare);
		m_CurCol = m_arColOrder[nCol];
		m_This = this;
		std::stable_sort(m_arRowOrder.begin() + m_nFixedRows, m_arRowOrder.end(),
		                 bAscending ? m_pfnVirtualCompare : NotVirtualCompare);
		return TRUE;
	}

	//LPARAM midItem = GetItemData((lo + hi)/2, nCol);
	const LPARAM pMidCell = reinterpret_cast<LPARAM>(GetCell((lo + hi) / 2, nCol));

	// loop through the list until indices cross
	while (lo <= hi)
	{
		// Find the first element that is greater than or equal to the partition
		// element starting from the left Index.
		if (bAscending)
			while (lo < high && pfnCompare(reinterpret_cast<LPARAM>(GetCell(lo, nCol)), pMidCell, data) < 0)
				++lo;
		else
			while (lo < high && pfnCompare(reinterpret_cast<LPARAM>(GetCell(lo, nCol)), pMidCell, data) > 0)
				++lo;

		// Find an element that is smaller than or equal to  the partition
		// element starting from the right Index.
		if (bAscending)
			while (hi > low && pfnCompare(reinterpret_cast<LPARAM>(GetCell(hi, nCol)), pMidCell, data) > 0)
				--hi;
		else
			while (hi > low && pfnCompare(reinterpret_cast<LPARAM>(GetCell(hi, nCol)), pMidCell, data) < 0)
				--hi;

		// If the indexes have not crossed, swap if the items are not equal
		if (lo <= hi)
		{
			// swap only if the items are not equal
			if (pfnCompare(reinterpret_cast<LPARAM>(GetCell(lo, nCol)), reinterpret_cast<LPARAM>(GetCell(hi, nCol)), data) != 0)
			{
				for (int col = 0; col < GetColumnCount(); col++)
				{
					GridCellBase* pCell = GetCell(lo, col);
					SetCell(lo, col, GetCell(hi, col));
					SetCell(hi, col, pCell);
				}
				const UINT nRowHeight = m_arRowHeights[lo];
				m_arRowHeights[lo] = m_arRowHeights[hi];
				m_arRowHeights[hi] = nRowHeight;
			}

			++lo;
			--hi;
		}
	}

	// If the right index has not reached the left side of array
	// must now sort the left partition.
	if (low < hi)
		SortItems(pfnCompare, nCol, bAscending, data, low, hi);

	// If the left index has not reached the right side of array
	// must now sort the right partition.
	if (lo < high)
		SortItems(pfnCompare, nCol, bAscending, data, lo, high);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// GridCtrl data functions

BOOL GridCtrl::SetItem(const GV_ITEM* pItem)
{
	if (!pItem || GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(pItem->row, pItem->col);
	if (!pCell)
		return FALSE;

	SetModified(TRUE, pItem->row, pItem->col);

	if (pItem->mask & GVIF_TEXT)
		pCell->SetText(pItem->strText);
	if (pItem->mask & GVIF_PARAM)
		pCell->SetData(pItem->lParam);
	if (pItem->mask & GVIF_IMAGE)
		pCell->SetImage(pItem->iImage);
	if (pItem->mask & GVIF_STATE)
		SetItemState(pItem->row, pItem->col, pItem->nState);
	if (pItem->mask & GVIF_FORMAT)
		pCell->SetFormat(pItem->nFormat);
	if (pItem->mask & GVIF_BKCLR)
		pCell->SetBackClr(pItem->crBkClr);
	if (pItem->mask & GVIF_FGCLR)
		pCell->SetTextClr(pItem->crFgClr);
	if (pItem->mask & GVIF_FONT)
		pCell->SetFont(&(pItem->lfFont));
	if (pItem->mask & GVIF_MARGIN)
		pCell->SetMargin(pItem->nMargin);

	return TRUE;
}

BOOL GridCtrl::GetItem(GV_ITEM* pItem)
{
	if (!pItem)
		return FALSE;
	const GridCellBase* pCell = GetCell(pItem->row, pItem->col);
	if (!pCell)
		return FALSE;

	if (pItem->mask & GVIF_TEXT)
		pItem->strText = GetItemText(pItem->row, pItem->col);
	if (pItem->mask & GVIF_PARAM)
		pItem->lParam = pCell->GetData();
	if (pItem->mask & GVIF_IMAGE)
		pItem->iImage = pCell->GetImage();
	if (pItem->mask & GVIF_STATE)
		pItem->nState = pCell->GetState();
	if (pItem->mask & GVIF_FORMAT)
		pItem->nFormat = pCell->GetFormat();
	if (pItem->mask & GVIF_BKCLR)
		pItem->crBkClr = pCell->GetBackClr();
	if (pItem->mask & GVIF_FGCLR)
		pItem->crFgClr = pCell->GetTextClr();
	if (pItem->mask & GVIF_FONT)
		memcpy(&(pItem->lfFont), pCell->GetFont(), sizeof(LOGFONT));
	if (pItem->mask & GVIF_MARGIN)
		pItem->nMargin = pCell->GetMargin();

	return TRUE;
}

BOOL GridCtrl::SetItemText(int nRow, int nCol, LPCTSTR str)
{
	if (GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(nRow, nCol);
	if (!pCell)
		return FALSE;

	pCell->SetText(str);

	SetModified(TRUE, nRow, nCol);
	return TRUE;
}

#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 210)
// EFW - 06/13/99 - Added to support printf-style formatting codes
BOOL GridCtrl::SetItemTextFmt(int nRow, int nCol, LPCTSTR szFmt, ...)
{
	if (GetVirtualMode())
		return FALSE;

	CString strText;

	va_list argptr;

	GridCellBase* pCell = GetCell(nRow, nCol);
	if (!pCell)
		return FALSE;

	// Format the message text
	va_start(argptr, szFmt);
	strText.FormatV(szFmt, argptr);
	va_end(argptr);

	pCell->SetText(strText);

	SetModified(TRUE, nRow, nCol);
	return TRUE;
}

// EFW - 06/13/99 - Added to support string resource ID.  Supports
// a variable argument list too.
BOOL GridCtrl::SetItemTextFmtID(int nRow, int nCol, UINT nID, ...)
{
	if (GetVirtualMode())
		return FALSE;

	CString strFmt, strText;
	va_list argptr;

	GridCellBase* pCell = GetCell(nRow, nCol);
	if (!pCell)
		return FALSE;

	// Format the message text
	va_start(argptr, nID);
	VERIFY(strFmt.LoadString(nID));
	strText.FormatV(strFmt, argptr);
	va_end(argptr);

	pCell->SetText(strText);

	SetModified(TRUE, nRow, nCol);
	return TRUE;
}
#endif

BOOL GridCtrl::SetItemData(int nRow, int nCol, LPARAM lParam)
{
	if (GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(nRow, nCol);
	if (!pCell)
		return FALSE;

	pCell->SetData(lParam);
	SetModified(TRUE, nRow, nCol);
	return TRUE;
}

LPARAM GridCtrl::GetItemData(int nRow, int nCol) const
{
	GridCellBase* pCell = GetCell(nRow, nCol);
	if (!pCell)
		return 0;

	return pCell->GetData();
}

BOOL GridCtrl::SetItemImage(int nRow, int nCol, int iImage)
{
	if (GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(nRow, nCol);
	if (!pCell)
		return FALSE;

	pCell->SetImage(iImage);
	SetModified(TRUE, nRow, nCol);
	return TRUE;
}

int GridCtrl::GetItemImage(int nRow, int nCol) const
{
	GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return -1;

	return pCell->GetImage();
}

BOOL GridCtrl::SetItemState(int nRow, int nCol, UINT state)
{
	const BOOL bSelected = IsCellSelected(nRow, nCol);

	// If the cell is being unselected, remove it from the selected list
	if (bSelected && !(state & GVIS_SELECTED))
	{
		GridCellID cell;
		DWORD key = MAKELONG(nRow, nCol);

		if (m_SelectedCellMap.Lookup(key, cell))
			m_SelectedCellMap.RemoveKey(key);
	}

	// If cell is being selected, add it to the list of selected cells
	else if (!bSelected && (state & GVIS_SELECTED))
	{
		GridCellID cell(nRow, nCol);
		m_SelectedCellMap.SetAt(MAKELONG(nRow, nCol), cell);
	}

	if (GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return FALSE;

	// Set the cell's state
	pCell->SetState(state);

	return TRUE;
}

UINT GridCtrl::GetItemState(int nRow, int nCol) const
{
	const GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return 0;

	return pCell->GetState();
}

BOOL GridCtrl::SetItemFormat(int nRow, int nCol, UINT nFormat)
{
	if (GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return FALSE;

	pCell->SetFormat(nFormat);
	return TRUE;
}

UINT GridCtrl::GetItemFormat(int nRow, int nCol) const
{
	const GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return 0;

	return pCell->GetFormat();
}

BOOL GridCtrl::SetItemBkColour(int nRow, int nCol, COLORREF cr /* = CLR_DEFAULT */)
{
	if (GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return FALSE;

	pCell->SetBackClr(cr);
	return TRUE;
}

COLORREF GridCtrl::GetItemBkColour(int nRow, int nCol) const
{
	const GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return 0;

	return pCell->GetBackClr();
}

BOOL GridCtrl::SetItemFgColour(int nRow, int nCol, COLORREF cr /* = CLR_DEFAULT */)
{
	if (GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return FALSE;

	pCell->SetTextClr(cr);
	return TRUE;
}

COLORREF GridCtrl::GetItemFgColour(int nRow, int nCol) const
{
	const GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return 0;

	return pCell->GetTextClr();
}

BOOL GridCtrl::SetItemFont(int nRow, int nCol, const LOGFONT* plf)
{
	if (GetVirtualMode())
		return FALSE;

	GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return FALSE;

	pCell->SetFont(plf);

	return TRUE;
}

const LOGFONT* GridCtrl::GetItemFont(int nRow, int nCol)
{
	const GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return GetDefaultCell(nRow < GetFixedRowCount(), nCol < GetFixedColumnCount())->GetFont();

	return pCell->GetFont();
}

BOOL GridCtrl::IsItemEditing(int nRow, int nCol)
{
	const GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return FALSE;

	return pCell->IsEditing();
}

////////////////////////////////////////////////////////////////////////////////////
// Row/Column size functions

long GridCtrl::GetVirtualWidth() const
{
	long lVirtualWidth = 0;
	const int iColCount = GetColumnCount();
	for (int i = 0; i < iColCount; i++)
		lVirtualWidth += m_arColWidths[i];

	return lVirtualWidth;
}

long GridCtrl::GetVirtualHeight() const
{
	long lVirtualHeight = 0;
	const int iRowCount = GetRowCount();
	for (int i = 0; i < iRowCount; i++)
		lVirtualHeight += m_arRowHeights[i];

	return lVirtualHeight;
}

int GridCtrl::GetRowHeight(int nRow) const
{
	ASSERT(nRow >= 0 && nRow < m_nRows);
	if (nRow < 0 || nRow >= m_nRows)
		return -1;

	return m_arRowHeights[nRow];
}

int GridCtrl::GetColumnWidth(int nCol) const
{
	ASSERT(nCol >= 0 && nCol < m_nCols);
	if (nCol < 0 || nCol >= m_nCols)
		return -1;

	return m_arColWidths[m_arColOrder[nCol]];
}

BOOL GridCtrl::SetRowHeight(int nRow, int height)
{
	ASSERT(nRow >= 0 && nRow < m_nRows&& height >= 0);
	if (nRow < 0 || nRow >= m_nRows || height < 0)
		return FALSE;

	m_arRowHeights[nRow] = height;
	ResetScrollBars();

	return TRUE;
}

BOOL GridCtrl::SetColumnWidth(int nCol, int width)
{
	ASSERT(nCol >= 0 && nCol < m_nCols&& width >= 0);
	if (nCol < 0 || nCol >= m_nCols || width < 0)
		return FALSE;

	m_arColWidths[m_arColOrder[nCol]] = width;
	ResetScrollBars();

	return TRUE;
}

int GridCtrl::GetFixedRowHeight() const
{
	int nHeight = 0;
	for (int i = 0; i < m_nFixedRows; i++)
		nHeight += GetRowHeight(i);

	return nHeight;
}

int GridCtrl::GetFixedColumnWidth() const
{
	int nWidth = 0;
	for (int i = 0; i < m_nFixedCols; i++)
		nWidth += GetColumnWidth(i);

	return nWidth;
}

BOOL GridCtrl::AutoSizeColumn(int nCol, UINT nAutoSizeStyle /*=GVS_DEFAULT*/,
                               BOOL bResetScroll /*=TRUE*/)
{
	ASSERT(nCol >= 0 && nCol < m_nCols);
	if (nCol < 0 || nCol >= m_nCols)
		return FALSE;

	//  Skip hidden columns when autosizing
	if (GetColumnWidth(nCol) <= 0)
		return FALSE;

	CSize size;
	CDC* p_dc = GetDC();
	if (!p_dc)
		return FALSE;

	int nWidth = 0;

	ASSERT(GVS_DEFAULT <= nAutoSizeStyle && nAutoSizeStyle <= GVS_BOTH);
	if (nAutoSizeStyle == GVS_DEFAULT)
		nAutoSizeStyle = GetAutoSizeStyle();

	const int nStartRow = (nAutoSizeStyle & GVS_HEADER) ? 0 : GetFixedRowCount();
	const int nEndRow = (nAutoSizeStyle & GVS_DATA) ? GetRowCount() - 1 : GetFixedRowCount() - 1;

	if (GetVirtualMode())
		SendCacheHintToParent(GridCellRange(nStartRow, nCol, nEndRow, nCol));

	for (int nRow = nStartRow; nRow <= nEndRow; nRow++)
	{
		GridCellBase* pCell = GetCell(nRow, nCol);
		if (pCell)
			size = pCell->GetCellExtent(p_dc);
		if (size.cx > nWidth)
			nWidth = size.cx;
	}

	if (GetVirtualMode())
		SendCacheHintToParent(GridCellRange(-1, -1, -1, -1));

	m_arColWidths[nCol] = nWidth;

	ReleaseDC(p_dc);
	if (bResetScroll)
		ResetScrollBars();

	return TRUE;
}

BOOL GridCtrl::AutoSizeRow(int nRow, BOOL bResetScroll /*=TRUE*/)
{
	ASSERT(nRow >= 0 && nRow < m_nRows);
	if (nRow < 0 || nRow >= m_nRows)
		return FALSE;

	//  Skip hidden rows when autosizing
	if (GetRowHeight(nRow) <= 0)
		return FALSE;

	CSize size;
	CDC* p_dc = GetDC();
	if (!p_dc)
		return FALSE;

	int nHeight = 0;
	const int nNumColumns = GetColumnCount();

	if (GetVirtualMode())
		SendCacheHintToParent(GridCellRange(nRow, 0, nRow, nNumColumns));

	for (int nCol = 0; nCol < nNumColumns; nCol++)
	{
		GridCellBase* pCell = GetCell(nRow, nCol);
		if (pCell)
			size = pCell->GetCellExtent(p_dc);
		if (size.cy > nHeight)
			nHeight = size.cy;
	}
	m_arRowHeights[nRow] = nHeight;

	if (GetVirtualMode())
		SendCacheHintToParent(GridCellRange(-1, -1, -1, -1));

	ReleaseDC(p_dc);
	if (bResetScroll)
		ResetScrollBars();

	return TRUE;
}

void GridCtrl::AutoSizeColumns(UINT nAutoSizeStyle /*=GVS_DEFAULT*/)
{
	const int nNumColumns = GetColumnCount();
	for (int nCol = 0; nCol < nNumColumns; nCol++)
	{
		//  Skip hidden columns when autosizing
		if (GetColumnWidth(nCol) > 0)
			AutoSizeColumn(nCol, nAutoSizeStyle, FALSE);
	}
	ResetScrollBars();
}

void GridCtrl::AutoSizeRows()
{
	const int nNumRows = GetRowCount();
	for (int nRow = 0; nRow < nNumRows; nRow++)
	{
		//  Skip hidden rows when autosizing
		if (GetRowHeight(nRow) > 0)
			AutoSizeRow(nRow, FALSE);
	}
	ResetScrollBars();
}

// sizes all rows and columns
// faster than calling both AutoSizeColumns() and AutoSizeRows()
void GridCtrl::AutoSize(UINT nAutoSizeStyle /*=GVS_DEFAULT*/)
{
	CDC* p_dc = GetDC();
	if (!p_dc)
		return;

	const int nNumColumns = GetColumnCount();

	int nRow;

	ASSERT(GVS_DEFAULT <= nAutoSizeStyle && nAutoSizeStyle <= GVS_BOTH);
	if (nAutoSizeStyle == GVS_DEFAULT)
		nAutoSizeStyle = GetAutoSizeStyle();

	const int nStartRow = (nAutoSizeStyle & GVS_HEADER) ? 0 : GetFixedRowCount();
	const int nEndRow = (nAutoSizeStyle & GVS_DATA) ? GetRowCount() - 1 : GetFixedRowCount() - 1;

	if (GetVirtualMode())
		SendCacheHintToParent(GridCellRange(nStartRow, 0, nEndRow, nNumColumns));

	// Row initialisation - only work on rows whose height is > 0
	for (nRow = nStartRow; nRow <= nEndRow; nRow++)
	{
		if (GetRowHeight(nRow) > 0)
			m_arRowHeights[nRow] = 1;
	}

	CSize size;
	for (int nCol = 0; nCol < nNumColumns; nCol++)
	{
		//  Don't size hidden columns or rows
		if (GetColumnWidth(nCol) > 0)
		{
			// Skip columns that are hidden, but now initialize
			m_arColWidths[nCol] = 0;
			for (nRow = nStartRow; nRow <= nEndRow; nRow++)
			{
				if (GetRowHeight(nRow) > 0)
				{
					GridCellBase* pCell = GetCell(nRow, nCol);
					if (pCell)
						size = pCell->GetCellExtent(p_dc);
					if (size.cx > static_cast<int>(m_arColWidths[nCol]))
						m_arColWidths[nCol] = size.cx;
					if (size.cy > static_cast<int>(m_arRowHeights[nRow]))
						m_arRowHeights[nRow] = size.cy;
				}
			}
		}
	}

	if (GetVirtualMode())
		SendCacheHintToParent(GridCellRange(-1, -1, -1, -1));

	ReleaseDC(p_dc);

	ResetScrollBars();
	Refresh();
}

// Expands the columns to fit the screen space. If bExpandFixed is FALSE then fixed
// columns will not be affected
void GridCtrl::ExpandColumnsToFit(BOOL bExpandFixed /*=TRUE*/)
{
	if (bExpandFixed)
	{
		if (GetColumnCount() <= 0) return;
	}
	else
	{
		if (GetColumnCount() <= GetFixedColumnCount()) return;
	}

	EnableScrollBars(SB_HORZ, FALSE);

	int col;
	CRect rect;
	GetClientRect(rect);

	const int nFirstColumn = (bExpandFixed) ? 0 : GetFixedColumnCount();

	int nNumColumnsAffected = 0;
	for (col = nFirstColumn; col < GetColumnCount(); col++)
	{
		if (m_arColWidths[col] > 0)
			nNumColumnsAffected++;
	}

	if (nNumColumnsAffected <= 0)
		return;

	const long virtualWidth = GetVirtualWidth();
	const int nDifference = rect.Width() - static_cast<int>(virtualWidth);
	const int nColumnAdjustment = nDifference / nNumColumnsAffected;

	for (col = nFirstColumn; col < GetColumnCount(); col++)
	{
		if (m_arColWidths[col] > 0)
			m_arColWidths[col] += nColumnAdjustment;
	}

	if (nDifference > 0)
	{
		const int leftOver = nDifference % nNumColumnsAffected;
		for (int nCount = 0, col = nFirstColumn;
		     (col < GetColumnCount()) && (nCount < leftOver); col++, nCount++)
		{
			if (m_arColWidths[col] > 0)
				m_arColWidths[col] += 1;
		}
	}
	else
	{
		const int leftOver = (-nDifference) % nNumColumnsAffected;
		for (int nCount = 0, col = nFirstColumn;
		     (col < GetColumnCount()) && (nCount < leftOver); col++, nCount++)
		{
			if (m_arColWidths[col] > 0)
				m_arColWidths[col] -= 1;
		}
	}

	Refresh();

	ResetScrollBars();
}

void GridCtrl::ExpandLastColumn()
{
	if (GetColumnCount() <= 0)
		return;

	// Search for last non-hidden column
	int nLastColumn = GetColumnCount() - 1;
	while (m_arColWidths[nLastColumn] <= 0)
		nLastColumn--;

	if (nLastColumn <= 0)
		return;

	EnableScrollBars(SB_HORZ, FALSE);

	CRect rect;
	GetClientRect(rect);

	const long virtualWidth = GetVirtualWidth();
	const int nDifference = rect.Width() - static_cast<int>(virtualWidth);

	if (nDifference > 0)
	{
		//if (GetVirtualHeight() > rect.GetRectHeight())
		//    nDifference -= GetSystemMetrics(SM_CXVSCROLL);

		m_arColWidths[nLastColumn] += nDifference;
		Refresh();
	}

	ResetScrollBars();
}

// Expands the rows to fit the screen space. If bExpandFixed is FALSE then fixed
// rows will not be affected
void GridCtrl::ExpandRowsToFit(BOOL bExpandFixed /*=TRUE*/)
{
	if (bExpandFixed)
	{
		if (GetRowCount() <= 0) return;
	}
	else
	{
		if (GetRowCount() <= GetFixedRowCount()) return;
	}

	EnableScrollBars(SB_VERT, FALSE);

	int row;
	CRect rect;
	GetClientRect(rect);

	const int nFirstRow = (bExpandFixed) ? 0 : GetFixedRowCount();

	int nNumRowsAffected = 0;
	for (row = nFirstRow; row < GetRowCount(); row++)
	{
		if (m_arRowHeights[row] > 0)
			nNumRowsAffected++;
	}

	if (nNumRowsAffected <= 0)
		return;

	const long virtualHeight = GetVirtualHeight();
	const int nDifference = rect.Height() - static_cast<int>(virtualHeight);
	const int nRowAdjustment = nDifference / nNumRowsAffected;

	for (row = nFirstRow; row < GetRowCount(); row++)
	{
		if (m_arRowHeights[row] > 0)
			m_arRowHeights[row] += nRowAdjustment;
	}

	if (nDifference > 0)
	{
		const int leftOver = nDifference % nNumRowsAffected;
		for (int nCount = 0, row = nFirstRow;
		     (row < GetRowCount()) && (nCount < leftOver); row++, nCount++)
		{
			if (m_arRowHeights[row] > 0)
				m_arRowHeights[row] += 1;
		}
	}
	else
	{
		const int leftOver = (-nDifference) % nNumRowsAffected;
		for (int nCount = 0, row = nFirstRow;
		     (row < GetRowCount()) && (nCount < leftOver); row++, nCount++)
		{
			if (m_arRowHeights[row] > 0)
				m_arRowHeights[row] -= 1;
		}
	}

	Refresh();

	ResetScrollBars();
}

// Expands the cells to fit the screen space. If bExpandFixed is FALSE then fixed
// cells  will not be affected
void GridCtrl::ExpandToFit(BOOL bExpandFixed /*=TRUE*/)
{
	ExpandColumnsToFit(bExpandFixed); // This will remove any existing horz scrollbar
	ExpandRowsToFit(bExpandFixed); // This will remove any existing vert scrollbar
	ExpandColumnsToFit(bExpandFixed); // Just in case the first adjustment was with a vert
	// scrollbar in place
	Refresh();
}

/////////////////////////////////////////////////////////////////////////////////////
// Attributes

void GridCtrl::SetVirtualMode(BOOL bVirtual)
{
	DeleteAllItems();
	m_bVirtualMode = bVirtual;

	// Force some defaults here.
	if (m_bVirtualMode)
	{
		SetEditable(FALSE);
		SetAutoSizeStyle(GVS_HEADER);
		SetFixedColumnSelection(FALSE);
		SetFixedRowSelection(FALSE);
		ResetVirtualOrder();
	}
}

void GridCtrl::SetGridLines(int nWhichLines /*=GVL_BOTH*/)
{
	m_nGridLines = nWhichLines;
	Refresh();
}

void GridCtrl::SetListMode(BOOL bEnableListMode /*=TRUE*/)
{
	ResetSelectedRange();
	SetSortColumn(-1);
	m_bListMode = bEnableListMode;
	SetFixedRowSelection(FALSE);
	Refresh();
}

void GridCtrl::SetSortColumn(int nCol)
{
	if (m_nSortColumn >= 0)
		InvalidateCellRect(0, m_nSortColumn);
	m_nSortColumn = nCol;
	if (nCol >= 0)
		InvalidateCellRect(0, nCol);
}

BOOL GridCtrl::IsCellFixed(int nRow, int nCol)
{
	return (nRow < GetFixedRowCount() || nCol < GetFixedColumnCount());
}

void GridCtrl::SetModified(BOOL bModified /*=TRUE*/, int nRow /*=-1*/, int nCol /*=-1*/)
{
	// Cannot guarantee sorting anymore...
	if (nCol < 0 || nCol == GetSortColumn())
		SetSortColumn(-1);

	if (nRow >= 0 && nCol >= 0)
	{
		if (bModified)
		{
			SetItemState(nRow, nCol, GetItemState(nRow, nCol) | GVIS_MODIFIED);
			m_bModified = TRUE;
		}
		else
			SetItemState(nRow, nCol, GetItemState(nRow, nCol) & ~GVIS_MODIFIED);
	}
	else
		m_bModified = bModified;

	if (!m_bModified)
	{
		for (int row = 0; row < GetRowCount(); row++)
			for (int col = 0; col < GetColumnCount(); col++)
				SetItemState(row, col, GetItemState(row, col) & ~GVIS_MODIFIED);
	}
}

BOOL GridCtrl::GetModified(int nRow /*=-1*/, int nCol /*=-1*/)
{
	if (nRow >= 0 && nCol >= 0)
		return ((GetItemState(nRow, nCol) & GVIS_MODIFIED) == GVIS_MODIFIED);
	return m_bModified;
}

/////////////////////////////////////////////////////////////////////////////////////
// GridCtrl cell visibility tests and invalidation/redraw functions

void GridCtrl::Refresh()
{
	if (GetSafeHwnd() && m_bAllowDraw)
		Invalidate();
}

// EnsureVisible supplied by Roelf Werkman
void GridCtrl::EnsureVisible(int nRow, int nCol)
{
	if (!m_bAllowDraw)
		return;

	CRect rectWindow;
	/*
	// set the scroll to the approximate position of row (Nigel Page-Jones)
	int nPos = (int)((float)nRow / GetRowCount() * 1000);
	float fPos = (float)nPos / 1000;
	SCROLLINFO scrollInfo;
	GetScrollInfo(SB_VERT, &scrollInfo);
	scrollInfo.nPos = (int)(scrollInfo.nMax * fPos);
	SetScrollInfo(SB_VERT, &scrollInfo, FALSE);

	GetClientRect(rectWindow);

	// redraw cells    if necessary (Nigel Page-Jones)
	GridCellID idTopLeft = GetTopleftNonFixedCell(FALSE);
	GridCellID idNewTopLeft = GetTopleftNonFixedCell(TRUE);
	if (idNewTopLeft != idTopLeft)
	{
		rectWindow.top = GetFixedRowHeight();
		InvalidateRect(rectWindow);
	}
	*/

	// We are going to send some scroll messages, which will steal the focus
	// from it's rightful owner. Squirrel it away ourselves so we can give
	// it back. (Damir)
	CWnd* pFocusWnd = GetFocus();

	GridCellRange VisibleCells = GetVisibleNonFixedCellRange();

	int right = nCol - VisibleCells.GetMaxCol();
	int left = VisibleCells.GetMinCol() - nCol;
	int down = nRow - VisibleCells.GetMaxRow();
	int up = VisibleCells.GetMinRow() - nRow;

	int iColumnStart = VisibleCells.GetMaxCol() + 1;
	while (right > 0)
	{
		if (GetColumnWidth(iColumnStart) > 0)
			SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);

		right--;
		iColumnStart++;
	}

	iColumnStart = VisibleCells.GetMinCol() - 1;
	while (left > 0)
	{
		if (GetColumnWidth(iColumnStart) > 0)
			SendMessage(WM_HSCROLL, SB_LINELEFT, 0);
		left--;
		iColumnStart--;
	}

	int iRowStart = VisibleCells.GetMaxRow() + 1;
	while (down > 0)
	{
		if (GetRowHeight(iRowStart) > 0)
			SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
		down--;
		iRowStart++;
	}

	iRowStart = VisibleCells.GetMinRow() - 1;
	while (up > 0)
	{
		if (GetRowHeight(iRowStart) > 0)
			SendMessage(WM_VSCROLL, SB_LINEUP, 0);
		up--;
		iRowStart--;
	}

	// Move one more if we only see a snall bit of the cell
	CRect rectCell;
	if (!GetCellRect(nRow, nCol, rectCell))
	{
		if (pFocusWnd && IsWindow(pFocusWnd->GetSafeHwnd()))
			pFocusWnd->SetFocus();
		return;
	}

	GetClientRect(rectWindow);

	// The previous fix was fixed properly by Martin Richter
	while (rectCell.right > rectWindow.right
		&& rectCell.left > GetFixedColumnWidth()
		&& IsVisibleHScroll() // Junlin Xu: added to prevent infinite loop
	)
	{
		SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);
		if (!GetCellRect(nRow, nCol, rectCell))
		{
			pFocusWnd->SetFocus();
			return;
		}
	}

	while (rectCell.bottom > rectWindow.bottom
		&& rectCell.top > GetFixedRowHeight()
		&& IsVisibleVScroll() // Junlin Xu: added to prevent infinite loop
	)
	{
		SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
		if (!GetCellRect(nRow, nCol, rectCell))
		{
			pFocusWnd->SetFocus();
			return;
		}
	}

	// restore focus to whoever owned it
	pFocusWnd->SetFocus();
}

BOOL GridCtrl::IsCellEditable(GridCellID& cell) const
{
	return IsCellEditable(cell.row, cell.col);
}

BOOL GridCtrl::IsCellEditable(int nRow, int nCol) const
{
	return IsEditable() && ((GetItemState(nRow, nCol) & GVIS_READONLY) != GVIS_READONLY);
}

BOOL GridCtrl::IsCellSelected(GridCellID& cell) const
{
	return IsCellSelected(cell.row, cell.col);
}

BOOL GridCtrl::IsCellSelected(int nRow, int nCol) const
{
	if (GetVirtualMode())
	{
		if (!IsSelectable())
			return FALSE;

		GridCellID cell;
		DWORD key = MAKELONG(nRow, nCol);

		return (m_SelectedCellMap.Lookup(key, cell));
	}
	return IsSelectable() && ((GetItemState(nRow, nCol) & GVIS_SELECTED) == GVIS_SELECTED);
}

BOOL GridCtrl::IsCellVisible(GridCellID cell)
{
	return IsCellVisible(cell.row, cell.col);
}

BOOL GridCtrl::IsCellVisible(int nRow, int nCol)
{
	if (!IsWindow(m_hWnd))
		return FALSE;

	int x, y;

	GridCellID TopLeft;
	if (nCol >= GetFixedColumnCount() || nRow >= GetFixedRowCount())
	{
		TopLeft = GetTopleftNonFixedCell();
		if (nCol >= GetFixedColumnCount() && nCol < TopLeft.col)
			return FALSE;
		if (nRow >= GetFixedRowCount() && nRow < TopLeft.row)
			return FALSE;
	}

	CRect rect;
	GetClientRect(rect);
	if (nCol < GetFixedColumnCount())
	{
		x = 0;
		for (int i = 0; i <= nCol; i++)
		{
			if (x >= rect.right)
				return FALSE;
			x += GetColumnWidth(i);
		}
	}
	else
	{
		x = GetFixedColumnWidth();
		for (int i = TopLeft.col; i <= nCol; i++)
		{
			if (x >= rect.right)
				return FALSE;
			x += GetColumnWidth(i);
		}
	}

	if (nRow < GetFixedRowCount())
	{
		y = 0;
		for (int i = 0; i <= nRow; i++)
		{
			if (y >= rect.bottom)
				return FALSE;
			y += GetRowHeight(i);
		}
	}
	else
	{
		if (nRow < TopLeft.row)
			return FALSE;
		y = GetFixedRowHeight();
		for (int i = TopLeft.row; i <= nRow; i++)
		{
			if (y >= rect.bottom)
				return FALSE;
			y += GetRowHeight(i);
		}
	}

	return TRUE;
}

BOOL GridCtrl::InvalidateCellRect(const GridCellID& cell)
{
	return InvalidateCellRect(cell.row, cell.col);
}

BOOL GridCtrl::InvalidateCellRect(const int row, const int col)
{
	if (!IsWindow(GetSafeHwnd()) || !m_bAllowDraw)
		return FALSE;

	if (!IsValid(row, col))
		return FALSE;

	if (!IsCellVisible(row, col))
		return FALSE;

	CRect rect;
	if (!GetCellRect(row, col, rect))
		return FALSE;
	rect.right++;
	rect.bottom++;
	InvalidateRect(rect, TRUE);

	return TRUE;
}

BOOL GridCtrl::InvalidateCellRect(const GridCellRange& cellRange)
{
	ASSERT(IsValid(cellRange));
	if (!IsWindow(GetSafeHwnd()) || !m_bAllowDraw)
		return FALSE;

	const GridCellRange visibleCellRange = GetVisibleNonFixedCellRange().Intersect(cellRange);

	CRect rect;
	if (!GetCellRangeRect(visibleCellRange, rect))
		return FALSE;

	rect.right++;
	rect.bottom++;
	InvalidateRect(rect, TRUE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// GridCtrl Mouse stuff

// Handles mouse wheel notifications
// Note - if this doesn't work for win95 then use OnRegisteredMouseWheel instead
#if !defined(_WIN32_WCE) && (_MFC_VER >= 0x0421)
BOOL GridCtrl::OnMouseWheel(UINT n_flags, short zDelta, CPoint pt)
{
	// A m_nRowsPerWheelNotch value less than 0 indicates that the mouse
	// wheel scrolls whole pages, not just lines.
	if (m_nRowsPerWheelNotch == -1)
	{
		const int nPagesScrolled = zDelta / 120;

		if (nPagesScrolled > 0)
		{
			for (int i = 0; i < nPagesScrolled; i++)
			{
				PostMessage(WM_VSCROLL, SB_PAGEUP, 0);
			}
		}
		else
		{
			for (int i = 0; i > nPagesScrolled; i--)
			{
				PostMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
			}
		}
	}
	else
	{
		int nRowsScrolled = m_nRowsPerWheelNotch * zDelta / 120;

		if (nRowsScrolled > 0)
		{
			for (int i = 0; i < nRowsScrolled; i++)
			{
				PostMessage(WM_VSCROLL, SB_LINEUP, 0);
			}
		}
		else
		{
			for (int i = 0; i > nRowsScrolled; i--)
			{
				PostMessage(WM_VSCROLL, SB_LINEDOWN, 0);
			}
		}
	}

	return CWnd::OnMouseWheel(n_flags, zDelta, pt);
}
#endif // !defined(_WIN32_WCE) && (_MFC_VER >= 0x0421)

void GridCtrl::OnMouseMove(UINT /*n_flags*/, CPoint point)
{
	CRect rect;
	GetClientRect(rect);

#ifndef GRIDCONTROL_NO_DRAGDROP
	// If outside client area, return (unless we are drag n dropping)
	if (m_MouseMode != MOUSE_DRAGGING && !rect.PtInRect(point))
		return;
#endif

	// Sometimes a MOUSEMOVE message can come after the left buttons
	// has been let go, but before the BUTTONUP message hs been processed.
	// We'll keep track of mouse buttons manually to avoid this.
	// All bMouseButtonDown's have been replaced with the member m_bLMouseButtonDown
	// BOOL bMouseButtonDown = ((n_flags & MK_LBUTTON) == MK_LBUTTON);

	// If the left mouse button is up, then test to see if row/column sizing is imminent
	if (!m_bLMouseButtonDown ||
		(m_bLMouseButtonDown && m_MouseMode == MOUSE_NOTHING))
	{
		if (m_bAllowColumnResize && MouseOverColumnResizeArea(point))
		{
			if (m_MouseMode != MOUSE_OVER_COL_DIVIDE)
			{
#ifndef _WIN32_WCE_NO_CURSOR
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
#endif
				m_MouseMode = MOUSE_OVER_COL_DIVIDE;
			}
		}
		else if (m_bAllowRowResize && MouseOverRowResizeArea(point))
		{
			if (m_MouseMode != MOUSE_OVER_ROW_DIVIDE)
			{
#ifndef _WIN32_WCE_NO_CURSOR
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
#endif
				m_MouseMode = MOUSE_OVER_ROW_DIVIDE;
			}
		}
		else if (m_MouseMode != MOUSE_NOTHING)
		{
#ifndef _WIN32_WCE_NO_CURSOR
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
#endif
			m_MouseMode = MOUSE_NOTHING;
		}

		if (m_MouseMode == MOUSE_NOTHING)
		{
			GridCellBase* pCell = nullptr;
			GridCellID idCurrentCell;
			if (!GetVirtualMode() || m_bTitleTips)
			{
				// Let the cell know that a big fat cursor is currently hovering
				// over it.
				idCurrentCell = GetCellFromPt(point);
				pCell = GetCell(idCurrentCell.row, idCurrentCell.col);
				if (pCell)
					pCell->OnMouseOver();
			}

#ifndef GRIDCONTROL_NO_TITLETIPS
			// Titletips anyone? anyone?
			if (m_bTitleTips)
			{
				CRect TextRect;
				if (pCell)
				{
					CRect CellRect;
					const LPCTSTR szTipText = pCell->GetTipText();
					if (!m_bRMouseButtonDown
						&& szTipText && szTipText[0]
						&& !pCell->IsEditing()
						&& GetCellRect(idCurrentCell.row, idCurrentCell.col, &TextRect)
						&& pCell->GetTipTextRect(&TextRect)
						&& GetCellRect(idCurrentCell.row, idCurrentCell.col, CellRect))
					{
						//	ATLTRACE2("Showing TitleTip\n");
						m_TitleTip.Show(TextRect, pCell->GetTipText(), 0, CellRect,
						                pCell->GetFont(), GetTitleTipTextClr(), GetTitleTipBackClr());
					}
				}
			}
#endif
		}

		m_LastMousePoint = point;
		return;
	}

	if (!IsValid(m_LeftClickDownCell))
	{
		m_LastMousePoint = point;
		return;
	}

	// If the left mouse button is down, then process appropriately
	if (m_bLMouseButtonDown)
	{
		switch (m_MouseMode)
		{
		case MOUSE_SELECT_ALL:
			break;

		case MOUSE_SELECT_COL:
		case MOUSE_SELECT_ROW:
		case MOUSE_SELECT_CELLS:
			{
				const GridCellID idCurrentCell = GetCellFromPt(point);
				if (!IsValid(idCurrentCell))
					return;

				if (idCurrentCell != GetFocusCell())
				{
					OnSelecting(idCurrentCell);

					// EFW - BUG FIX - Keep the appropriate cell row and/or
					// column focused.  A fix in SetFocusCell() will place
					// the cursor in a non-fixed cell as needed.
					if ((idCurrentCell.row >= m_nFixedRows &&
							idCurrentCell.col >= m_nFixedCols) ||
						m_MouseMode == MOUSE_SELECT_COL ||
						m_MouseMode == MOUSE_SELECT_ROW)
					{
						SetFocusCell(idCurrentCell);
					}
				}
				break;
			}

		case MOUSE_SIZING_COL:
			{
				CDC* p_dc = GetDC();
				if (!p_dc)
					break;

				const CRect oldInvertedRect(m_LastMousePoint.x, rect.top,
				                            m_LastMousePoint.x + 2, rect.bottom);
				p_dc->InvertRect(&oldInvertedRect);
				const CRect newInvertedRect(point.x, rect.top,
				                            point.x + 2, rect.bottom);
				p_dc->InvertRect(&newInvertedRect);
				ReleaseDC(p_dc);
			}
			break;

		case MOUSE_SIZING_ROW:
			{
				CDC* p_dc = GetDC();
				if (!p_dc)
					break;

				const CRect oldInvertedRect(rect.left, m_LastMousePoint.y,
				                            rect.right, m_LastMousePoint.y + 2);
				p_dc->InvertRect(&oldInvertedRect);
				const CRect newInvertedRect(rect.left, point.y,
				                            rect.right, point.y + 2);
				p_dc->InvertRect(&newInvertedRect);
				ReleaseDC(p_dc);
			}
			break;

#ifndef GRIDCONTROL_NO_DRAGDROP
		case MOUSE_PREPARE_EDIT:
		case MOUSE_PREPARE_DRAG:
			m_MouseMode = MOUSE_PREPARE_DRAG;
			OnBeginDrag();
			break;
		default: ;
#endif
		}
	}

	m_LastMousePoint = point;
}

// Returns the point inside the cell that was clicked (coords relative to cell top left)
CPoint GridCtrl::GetPointClicked(int nRow, int nCol, const CPoint& point)
{
	CPoint PointCellOrigin;
	if (!GetCellOrigin(nRow, nCol, &PointCellOrigin))
		return CPoint(0, 0);

	CPoint PointClickedCellRelative(point);
	PointClickedCellRelative -= PointCellOrigin;
	return PointClickedCellRelative;
}

void GridCtrl::OnLButtonDblClk(UINT n_flags, CPoint point)
{
	//ATLTRACE2("GridCtrl::OnLButtonDblClk\n");

	GridCellID cell = GetCellFromPt(point);
	if (!IsValid(cell))
	{
		//ASSERT(FALSE);
		return;
	}

#ifdef _WIN32_WCE
	if (MouseOverColumnResizeArea(point))
#else
	if (m_MouseMode == MOUSE_OVER_COL_DIVIDE)
#endif
	{
		CPoint start;
		if (!GetCellOrigin(0, cell.col, &start))
			return;

		if (point.x - start.x < m_nResizeCaptureRange) // Clicked right of border
			cell.col--;

		//  ignore columns that are hidden and look left towards first visible column
		BOOL bFoundVisible = FALSE;
		while (cell.col >= 0)
		{
			if (GetColumnWidth(cell.col) > 0)
			{
				bFoundVisible = TRUE;
				break;
			}
			cell.col--;
		}
		if (!bFoundVisible)
			return;

		AutoSizeColumn(cell.col, GetAutoSizeStyle());
		Invalidate();
	}
#ifdef _WIN32_WCE
	else if (MouseOverRowResizeArea(point))
#else
	else if (m_MouseMode == MOUSE_OVER_ROW_DIVIDE)
#endif
	{
		CPoint start;
		if (!GetCellOrigin(0, cell.col, &start))
			return;

		if (point.y - start.y < m_nResizeCaptureRange) // Clicked below border
			cell.row--;

		//  ignore rows that are hidden and look up towards first visible row
		BOOL bFoundVisible = FALSE;
		while (cell.row >= 0)
		{
			if (GetRowHeight(cell.row) > 0)
			{
				bFoundVisible = TRUE;
				break;
			}
			cell.row--;
		}
		if (!bFoundVisible)
			return;

		AutoSizeRow(cell.row);
		Invalidate();
	}
	else if (m_MouseMode == MOUSE_NOTHING)
	{
		const CPoint pointClickedRel = GetPointClicked(cell.row, cell.col, point);

		GridCellBase* pCell = nullptr;
		if (IsValid(cell))
			pCell = GetCell(cell.row, cell.col);

		// Clicked in the text area? Only then will cell selection work
		BOOL bInTextArea = FALSE;
		if (pCell)
		{
			CRect rectCell;
			if (GetCellRect(cell.row, cell.col, rectCell) && pCell->GetTextRect(rectCell))
				bInTextArea = rectCell.PtInRect(point);
		}

		if (cell.row >= m_nFixedRows && IsValid(m_LeftClickDownCell) &&
			cell.col >= m_nFixedCols && bInTextArea)
		{
			OnEditCell(cell.row, cell.col, pointClickedRel, VK_LBUTTON);
		}
		else if (m_bListMode)
		{
			if (!IsValid(cell))
				return;
			if (cell.row >= m_nFixedRows && cell.col >= m_nFixedCols && bInTextArea)
				OnEditCell(cell.row, cell.col, pointClickedRel, VK_LBUTTON);
		}

		if (IsValid(cell))
		{
			GridCellBase* pCell = GetCell(cell.row, cell.col);
			if (pCell)
				pCell->OnDblClick(pointClickedRel);
			SendMessageToParent(cell.row, cell.col, NM_DBLCLK);
		}
	}

	CWnd::OnLButtonDblClk(n_flags, point);
}

void GridCtrl::OnLButtonDown(UINT n_flags, CPoint point)
{
#ifdef GRIDCONTROL_USE_TITLETIPS
	// EFW - Bug Fix
	m_TitleTip.Hide();  // hide any titletips
#endif

	// ATLTRACE2("GridCtrl::OnLButtonDown\n");
	// CWnd::OnLButtonDown(n_flags, point);

	SetFocus();
	m_CurCol = -1;
	m_bLMouseButtonDown = TRUE;
	m_LeftClickDownPoint = point;
	m_LeftClickDownCell = GetCellFromPt(point);
	if (!IsValid(m_LeftClickDownCell))
		return;
	m_CurRow = m_LeftClickDownCell.row;

	// If the SHIFT key is not down, then the start of the selection area should be the
	// cell just clicked. Otherwise, keep the previous selection-start-cell so the user
	// can add to their previous cell selections in an intuitive way. If no selection-
	// start-cell has been specified, then set it's value here and now.
	if ((n_flags & MK_SHIFT) != MK_SHIFT)
		m_SelectionStartCell = m_LeftClickDownCell;
	else
	{
		if (!IsValid(m_SelectionStartCell))
			m_SelectionStartCell = m_idCurrentCell;
	}

	EndEditing();

	// tell the cell about it
	GridCellBase* pCell = GetCell(m_LeftClickDownCell.row, m_LeftClickDownCell.col);
	if (pCell)
		pCell->OnClickDown(GetPointClicked(m_LeftClickDownCell.row, m_LeftClickDownCell.col, point));

	// Clicked in the text area? Only then will cell selection work
	BOOL bInTextArea = FALSE;
	if (pCell)
	{
		CRect rectCell;
		if (GetCellRect(m_LeftClickDownCell.row, m_LeftClickDownCell.col, rectCell) &&
			pCell->GetTextRect(rectCell))
		{
			bInTextArea = rectCell.PtInRect(point);
		}
	}

	// If the user clicks on the current cell, then prepare to edit it.
	// (If the user moves the mouse, then dragging occurs)
	if (m_LeftClickDownCell == m_idCurrentCell &&
		!(n_flags & MK_CONTROL) && bInTextArea &&
		IsCellEditable(m_LeftClickDownCell))
	{
		m_MouseMode = MOUSE_PREPARE_EDIT;
		return;
	}
	// If the user clicks on a selected cell, then prepare to drag it.
	// (If the user moves the mouse, then dragging occurs)
	if (IsCellSelected(m_LeftClickDownCell))
	{
		SetFocusCell(m_LeftClickDownCell.row, m_LeftClickDownCell.col);

		// If CTRL is pressed then unselect the cell or row (depending on the list mode)
		if (n_flags & MK_CONTROL)
		{
			SetFocusCell(m_LeftClickDownCell);
			if (GetListMode())
				SelectRows(m_LeftClickDownCell, TRUE, FALSE);
			else
				SelectCells(m_LeftClickDownCell, TRUE, FALSE);
			return;
		}
#ifndef GRIDCONTROL_NO_DRAGDROP
		if (m_bAllowDragAndDrop)
			m_MouseMode = MOUSE_PREPARE_DRAG;
#endif
	}
	else if (m_MouseMode != MOUSE_OVER_COL_DIVIDE &&
		m_MouseMode != MOUSE_OVER_ROW_DIVIDE)
	{
		if (m_LeftClickDownCell.row >= GetFixedRowCount() &&
			m_LeftClickDownCell.col >= GetFixedColumnCount())
		{
			SetFocusCell(m_LeftClickDownCell.row, m_LeftClickDownCell.col);
		}
		else
			SetFocusCell(-1, -1);

		//Don't set focus on any cell if the user clicked on a fixed cell - David Pritchard
		//if (GetRowCount() > GetFixedRowCount() &&
		//    GetColumnCount() > GetFixedColumnCount())
		//    SetFocusCell(max(m_LeftClickDownCell.row, m_nFixedRows),
		//                 max(m_LeftClickDownCell.col, m_nFixedCols));
	}

	SetCapture();

	if (m_MouseMode == MOUSE_NOTHING)
	{
		if (m_bAllowColumnResize && MouseOverColumnResizeArea(point))
		{
			if (m_MouseMode != MOUSE_OVER_COL_DIVIDE)
			{
#ifndef _WIN32_WCE_NO_CURSOR
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
#endif
				m_MouseMode = MOUSE_OVER_COL_DIVIDE;
			}
		}
		else if (m_bAllowRowResize && MouseOverRowResizeArea(point))
		{
			if (m_MouseMode != MOUSE_OVER_ROW_DIVIDE)
			{
#ifndef _WIN32_WCE_NO_CURSOR
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
#endif
				m_MouseMode = MOUSE_OVER_ROW_DIVIDE;
			}
		}
		// else if (m_MouseMode != MOUSE_NOTHING)
		//{
		//    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		//    m_MouseMode = MOUSE_NOTHING;
		//}
	}

	if (m_MouseMode == MOUSE_OVER_COL_DIVIDE) // sizing column
	{
		m_MouseMode = MOUSE_SIZING_COL;

		// Kludge for if we are over the last column...
		if (GetColumnWidth(GetColumnCount() - 1) < m_nResizeCaptureRange)
		{
			CRect VisRect;
			GetVisibleNonFixedCellRange(VisRect);
			if (abs(point.x - VisRect.right) < m_nResizeCaptureRange)
				m_LeftClickDownCell.col = GetColumnCount() - 1;
		}

		CPoint start;
		if (!GetCellOrigin(0, m_LeftClickDownCell.col, &start))
			return;

		if (!m_bHiddenColUnhide)
		{
			//  ignore columns that are hidden and look left towards first visible column
			BOOL bLookForVisible = TRUE;
			BOOL bIsCellRightBorder = point.x - start.x >= m_nResizeCaptureRange;

			if (bIsCellRightBorder
				&& m_LeftClickDownCell.col + 1 >= GetColumnCount())
			{
				// clicked on last column's right border

				// if last column is visible, don't do anything
				if (m_LeftClickDownCell.col >= 0)
					bLookForVisible = FALSE;
			}

			if (bLookForVisible)
			{
				// clicked on column divider other than last right border
				BOOL bFoundVisible = FALSE;
				int iOffset = 1;

				if (bIsCellRightBorder)
					iOffset = 0;

				while (m_LeftClickDownCell.col - iOffset >= 0)
				{
					if (GetColumnWidth(m_LeftClickDownCell.col - iOffset) > 0)
					{
						bFoundVisible = TRUE;
						break;
					}
					m_LeftClickDownCell.col--;
				}
				if (!bFoundVisible)
					return;
			}
		}

		CRect rect;
		GetClientRect(rect);
		const CRect invertedRect(point.x, rect.top, point.x + 2, rect.bottom);

		CDC* p_dc = GetDC();
		if (p_dc)
		{
			p_dc->InvertRect(&invertedRect);
			ReleaseDC(p_dc);
		}

		// If we clicked to the right of the colimn divide, then reset the click-down cell
		// as the cell to the left of the column divide - UNLESS we clicked on the last column
		// and the last column is teensy (kludge fix)
		if (point.x - start.x < m_nResizeCaptureRange)
		{
			if (m_LeftClickDownCell.col < GetColumnCount() - 1 ||
				GetColumnWidth(GetColumnCount() - 1) >= m_nResizeCaptureRange)
			{
				if (!GetCellOrigin(0, --m_LeftClickDownCell.col, &start))
					return;
			}
		}

		// Allow a cell resize width no greater than that which can be viewed within
		// the grid itself
		const int nMaxCellWidth = rect.Width() - GetFixedColumnWidth();
		rect.left = start.x + 1;
		rect.right = rect.left + nMaxCellWidth;

		ClientToScreen(rect);
#ifndef _WIN32_WCE_NO_CURSOR
		ClipCursor(rect);
#endif
	}
	else if (m_MouseMode == MOUSE_OVER_ROW_DIVIDE) // sizing row
	{
		m_MouseMode = MOUSE_SIZING_ROW;

		// Kludge for if we are over the last column...
		if (GetRowHeight(GetRowCount() - 1) < m_nResizeCaptureRange)
		{
			CRect VisRect;
			GetVisibleNonFixedCellRange(VisRect);
			if (abs(point.y - VisRect.bottom) < m_nResizeCaptureRange)
				m_LeftClickDownCell.row = GetRowCount() - 1;
		}

		CPoint start;
		if (!GetCellOrigin(m_LeftClickDownCell, &start))
			return;

		if (!m_bHiddenRowUnhide)
		{
			//  ignore rows that are hidden and look up towards first visible row
			BOOL bLookForVisible = TRUE;
			const BOOL bIsCellBottomBorder = point.y - start.y >= m_nResizeCaptureRange;

			if (bIsCellBottomBorder
				&& m_LeftClickDownCell.row + 1 >= GetRowCount())
			{
				// clicked on last row's bottom border

				// if last row is visible, don't do anything
				if (m_LeftClickDownCell.row >= 0)
					bLookForVisible = FALSE;
			}

			if (bLookForVisible)
			{
				// clicked on row divider other than last bottom border
				BOOL bFoundVisible = FALSE;
				int iOffset = 1;

				if (bIsCellBottomBorder)
					iOffset = 0;

				while (m_LeftClickDownCell.row - iOffset >= 0)
				{
					if (GetRowHeight(m_LeftClickDownCell.row - iOffset) > 0)
					{
						bFoundVisible = TRUE;
						break;
					}
					m_LeftClickDownCell.row--;
				}
				if (!bFoundVisible)
					return;
			}
		}

		CRect rect;
		GetClientRect(rect);
		const CRect invertedRect(rect.left, point.y, rect.right, point.y + 2);

		CDC* p_dc = GetDC();
		if (p_dc)
		{
			p_dc->InvertRect(&invertedRect);
			ReleaseDC(p_dc);
		}

		// If we clicked below the row divide, then reset the click-down cell
		// as the cell above the row divide - UNLESS we clicked on the last row
		// and the last row is teensy (kludge fix)
		if (point.y - start.y < m_nResizeCaptureRange) // clicked below border
		{
			if (m_LeftClickDownCell.row < GetRowCount() - 1 ||
				GetRowHeight(GetRowCount() - 1) >= m_nResizeCaptureRange)
			{
				if (!GetCellOrigin(--m_LeftClickDownCell.row, 0, &start))
					return;
			}
		}

		const int nMaxCellHeight = rect.Height() - GetFixedRowHeight();
		rect.top = start.y + 1;
		rect.bottom = rect.top + nMaxCellHeight;

		ClientToScreen(rect);

#ifndef _WIN32_WCE_NO_CURSOR
		ClipCursor(rect);
#endif
	}
	else
#ifndef GRIDCONTROL_NO_DRAGDROP
		if (m_MouseMode != MOUSE_PREPARE_DRAG) // not sizing or editing -- selecting
#endif
		{
			SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, GVN_SELCHANGING);

			// If Ctrl pressed, save the current cell selection. This will get added
			// to the new cell selection at the end of the cell selection process
			m_PrevSelectedCellMap.RemoveAll();
			if (n_flags & MK_CONTROL)
			{
				for (POSITION pos = m_SelectedCellMap.GetStartPosition(); pos != nullptr;)
				{
					DWORD key;
					GridCellID cell;
					m_SelectedCellMap.GetNextAssoc(pos, key, cell);
					m_PrevSelectedCellMap.SetAt(key, cell);
				}
			}

			if (m_LeftClickDownCell.row < GetFixedRowCount())
			{
				OnFixedRowClick(m_LeftClickDownCell);
				if (m_AllowReorderColumn && m_LeftClickDownCell.col >= GetFixedColumnCount())
				{
					ResetSelectedRange();
					// TODO : This is not the better solution, as we do not see why clicking in column header should reset selection
					//but the state of selection is instable after drag (at least until someone debugs it), so better clear it allways.
					m_MouseMode = MOUSE_PREPARE_DRAG;
					m_CurCol = m_LeftClickDownCell.col;
				}
			}
			else if (m_LeftClickDownCell.col < GetFixedColumnCount())
				OnFixedColumnClick(m_LeftClickDownCell);
			else
			{
				m_MouseMode = m_bListMode ? MOUSE_SELECT_ROW : MOUSE_SELECT_CELLS;
				OnSelecting(m_LeftClickDownCell);

				m_nTimerID = SetTimer(WM_LBUTTONDOWN, m_nTimerInterval, nullptr);
			}
		}
	m_LastMousePoint = point;
}

void GridCtrl::OnLButtonUp(UINT n_flags, CPoint point)
{
	// ATLTRACE2("GridCtrl::OnLButtonUp\n");

	CWnd::OnLButtonUp(n_flags, point);

	m_bLMouseButtonDown = FALSE;

#ifndef _WIN32_WCE_NO_CURSOR
	ClipCursor(nullptr);
#endif

	if (GetCapture()->GetSafeHwnd() == GetSafeHwnd())
	{
		ReleaseCapture();
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}

	CPoint pointClickedRel;
	pointClickedRel = GetPointClicked(m_idCurrentCell.row, m_idCurrentCell.col, point);

	// m_MouseMode == MOUSE_PREPARE_EDIT only if user clicked down on current cell
	// and then didn't move mouse before clicking up (releasing button)
	if (m_MouseMode == MOUSE_PREPARE_EDIT)
	{
		OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, pointClickedRel, VK_LBUTTON);
	}
#ifndef GRIDCONTROL_NO_DRAGDROP
	// m_MouseMode == MOUSE_PREPARE_DRAG only if user clicked down on a selected cell
	// and then didn't move mouse before clicking up (releasing button)
	else if (m_MouseMode == MOUSE_PREPARE_DRAG)
	{
		GridCellBase* pCell = GetCell(m_idCurrentCell.row, m_idCurrentCell.col);
		if (pCell)
			pCell->OnClick(GetPointClicked(m_idCurrentCell.row, m_idCurrentCell.col, point));
		SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, NM_CLICK);
		SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, GVN_SELCHANGING);
		ResetSelectedRange();
		SendMessageToParent(m_idCurrentCell.row, m_idCurrentCell.col, GVN_SELCHANGED);
	}
#endif
	else if (m_MouseMode == MOUSE_SIZING_COL)
	{
		CRect rect;
		GetClientRect(rect);
		const CRect invertedRect(m_LastMousePoint.x, rect.top, m_LastMousePoint.x + 2, rect.bottom);

		CDC* p_dc = GetDC();
		if (p_dc)
		{
			p_dc->InvertRect(&invertedRect);
			ReleaseDC(p_dc);
		}

		if (m_LeftClickDownPoint != point && (point.x != 0 || point.y != 0)) // 0 pt fix by email1@bierling.net
		{
			CPoint start;
			if (!GetCellOrigin(m_LeftClickDownCell, &start))
				return;

			int nColumnWidth = __max(point.x - start.x, m_bAllowColHide ? 0 : 1);

			SetColumnWidth(m_LeftClickDownCell.col, nColumnWidth);
			ResetScrollBars();
			Invalidate();
		}
	}
	else if (m_MouseMode == MOUSE_SIZING_ROW)
	{
		CRect rect;
		GetClientRect(rect);
		const CRect invertedRect(rect.left, m_LastMousePoint.y, rect.right, m_LastMousePoint.y + 2);

		CDC* p_dc = GetDC();
		if (p_dc)
		{
			p_dc->InvertRect(&invertedRect);
			ReleaseDC(p_dc);
		}

		if (m_LeftClickDownPoint != point && (point.x != 0 || point.y != 0)) // 0 pt fix by email1@bierling.net
		{
			CPoint start;
			if (!GetCellOrigin(m_LeftClickDownCell, &start))
				return;

			const int nRowHeight = __max(point.y - start.y, m_bAllowRowHide ? 0 : 1);

			SetRowHeight(m_LeftClickDownCell.row, nRowHeight);
			ResetScrollBars();
			Invalidate();
		}
	}
	else
	{
		SendMessageToParent(m_idCurrentCell.row, m_idCurrentCell.col, GVN_SELCHANGED);

		GridCellBase* pCell = GetCell(m_idCurrentCell.row, m_idCurrentCell.col);
		if (pCell)
			pCell->OnClick(GetPointClicked(m_idCurrentCell.row, m_idCurrentCell.col, point));
		SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, NM_CLICK);
	}

	m_MouseMode = MOUSE_NOTHING;

#ifndef _WIN32_WCE_NO_CURSOR
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
#endif

	if (!IsValid(m_LeftClickDownCell))
		return;

	CWnd* pOwner = GetOwner();
	if (pOwner && IsWindow(pOwner->m_hWnd))
		pOwner->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_CLICKED),
		                    reinterpret_cast<LPARAM>(GetSafeHwnd()));
}

#ifndef _WIN32_WCE
void GridCtrl::OnRButtonDown(UINT n_flags, CPoint point)
{
	CWnd::OnRButtonDown(n_flags, point);
	m_bRMouseButtonDown = TRUE;

#ifdef GRIDCONTROL_USE_TITLETIPS
	//ATLTRACE2("Hiding TitleTip\n");
	m_TitleTip.Hide();  // hide any titletips
#endif
}

// EFW - Added to forward right click to parent so that a context
// menu can be shown without deriving a new grid class.
void GridCtrl::OnRButtonUp(UINT n_flags, CPoint point)
{
	CWnd::OnRButtonUp(n_flags, point);

	m_bRMouseButtonDown = FALSE;

	const GridCellID FocusCell = GetCellFromPt(point);

	EndEditing(); // Auto-destroy any InPlaceEdit's

	// If not a valid cell, pass -1 for row and column
	if (!IsValid(FocusCell))
		SendMessageToParent(-1, -1, NM_RCLICK);
	else
	{
		SetFocusCell(-1, -1);
		SetFocusCell(__max(FocusCell.row, m_nFixedRows),
		             __max(FocusCell.col, m_nFixedCols));

		// tell the cell about it
		GridCellBase* pCell = GetCell(FocusCell.row, FocusCell.col);
		if (pCell)
			pCell->OnRClick(GetPointClicked(FocusCell.row, FocusCell.col, point));

		SendMessageToParent(FocusCell.row, FocusCell.col, NM_RCLICK);
	}
}
#endif

#if !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)
/////////////////////////////////////////////////////////////////////////////
// GridCtrl printing

// EFW - New print margin support functions
void GridCtrl::SetPrintMarginInfo(int nHeaderHeight, int nFooterHeight,
                                   int nLeftMargin, int nRightMargin, int nTopMargin,
                                   int nBottomMargin, int nGap)
{
	// If any parameter is -1, keep the existing setting
	if (nHeaderHeight > -1)
		m_nHeaderHeight = nHeaderHeight;
	if (nFooterHeight > -1)
		m_nFooterHeight = nFooterHeight;
	if (nLeftMargin > -1)
		m_nLeftMargin = nLeftMargin;
	if (nRightMargin > -1)
		m_nRightMargin = nRightMargin;
	if (nTopMargin > -1)
		m_nTopMargin = nTopMargin;
	if (nBottomMargin > -1)
		m_nBottomMargin = nBottomMargin;
	if (nGap > -1)
		m_nGap = nGap;
}

void GridCtrl::GetPrintMarginInfo(int& nHeaderHeight, int& nFooterHeight,
                                   int& nLeftMargin, int& nRightMargin, int& nTopMargin,
                                   int& nBottomMargin, int& nGap)
{
	nHeaderHeight = m_nHeaderHeight;
	nFooterHeight = m_nFooterHeight;
	nLeftMargin = m_nLeftMargin;
	nRightMargin = m_nRightMargin;
	nTopMargin = m_nTopMargin;
	nBottomMargin = m_nBottomMargin;
	nGap = m_nGap;
}

void GridCtrl::Print(CPrintDialog* pPrntDialog /*=NULL*/)
{
	CDC dc;

	if (pPrntDialog == nullptr)
	{
		CPrintDialog printDlg(FALSE);
		if (printDlg.DoModal() != IDOK) // Get printer settings from user
			return;

		dc.Attach(printDlg.GetPrinterDC()); // attach a printer DC
	}
	else
		dc.Attach(pPrntDialog->GetPrinterDC()); // attach a printer DC

	dc.m_bPrinting = TRUE;

	CString strTitle;
	BOOL flag = strTitle.LoadString(AFX_IDS_APP_TITLE);

	if (!flag || strTitle.IsEmpty())
	{
		CWnd* pParentWnd = GetParent();
		while (pParentWnd)
		{
			pParentWnd->GetWindowText(strTitle);
			if (strTitle.GetLength()) // can happen if it is a CView, CChildFrm has the title
				break;
			pParentWnd = pParentWnd->GetParent();
		}
	}

	DOCINFO di; // Initialise print doc details
	memset(&di, 0, sizeof(DOCINFO));
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = strTitle;

	BOOL bPrintingOK = dc.StartDoc(&di); // Begin a new print job

	CPrintInfo Info;
	Info.m_rectDraw.SetRect(0, 0, dc.GetDeviceCaps(HORZRES), dc.GetDeviceCaps(VERTRES));

	OnBeginPrinting(&dc, &Info); // Initialise printing
	for (UINT page = Info.GetMinPage(); page <= Info.GetMaxPage() && bPrintingOK; page++)
	{
		dc.StartPage(); // begin new page
		Info.m_nCurPage = page;
		OnPrint(&dc, &Info); // Print page
		bPrintingOK = (dc.EndPage() > 0); // end page
	}
	OnEndPrinting(&dc, &Info); // Clean up after printing

	if (bPrintingOK)
		dc.EndDoc(); // end a print job
	else
		dc.AbortDoc(); // abort job.

	dc.Detach(); // detach the printer DC
}

/////////////////////////////////////////////////////////////////////////////
// GridCtrl printing overridables - for Doc/View print/print preview framework

// EFW - Various changes in the next few functions to support the
// new print margins and a few other adjustments.
void GridCtrl::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	// OnBeginPrinting() is called after the user has committed to
	// printing by OK'ing the Print dialog, and after the framework
	// has created a CDC object for the printer or the preview view.

	// This is the right opportunity to set up the page range.
	// Given the CDC object, we can determine how many rows will
	// fit on a page, so we can in turn determine how many printed
	// pages represent the entire document.

	ASSERT(p_dc && pInfo);
	if (!p_dc || !pInfo) return;

	// Get a DC for the current window (will be a screen DC for print previewing)
	CDC* pCurrentDC = GetDC(); // will have dimensions of the client area
	if (!pCurrentDC) return;

	const CSize PaperPixelsPerInch(p_dc->GetDeviceCaps(LOGPIXELSX), p_dc->GetDeviceCaps(LOGPIXELSY));
	const CSize ScreenPixelsPerInch(pCurrentDC->GetDeviceCaps(LOGPIXELSX), pCurrentDC->GetDeviceCaps(LOGPIXELSY));

	// Create the printer font
	const int nFontSize = -10;
	const CString strFontName = _T("Arial");
	m_PrinterFont.CreateFont(nFontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
	                         OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
	                         DEFAULT_PITCH | FF_DONTCARE, strFontName);

	CFont* pOldFont = p_dc->SelectObject(&m_PrinterFont);

	// Get the average character width (in GridCtrl units) and hence the margins
	m_CharSize = p_dc->GetTextExtent(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSATUVWXYZ"), 52);
	m_CharSize.cx /= 52;
	int nMargins = (m_nLeftMargin + m_nRightMargin) * m_CharSize.cx;

	// Get the page sizes (physical and logical)
	m_PaperSize = CSize(p_dc->GetDeviceCaps(HORZRES), p_dc->GetDeviceCaps(VERTRES));

	if (m_bWysiwygPrinting)
	{
		m_LogicalPageSize.cx = ScreenPixelsPerInch.cx * m_PaperSize.cx / PaperPixelsPerInch.cx * 3 / 4;
		m_LogicalPageSize.cy = ScreenPixelsPerInch.cy * m_PaperSize.cy / PaperPixelsPerInch.cy * 3 / 4;
	}
	else
	{
		m_PaperSize = CSize(p_dc->GetDeviceCaps(HORZRES), p_dc->GetDeviceCaps(VERTRES));

		m_LogicalPageSize.cx = GetVirtualWidth() + nMargins;
#ifdef _WIN32_WCE
		m_LogicalPageSize.cy = (m_LogicalPageSize.cx * m_PaperSize.cy) / m_PaperSize.cx;
#else
		m_LogicalPageSize.cy = MulDiv(m_LogicalPageSize.cx, m_PaperSize.cy, m_PaperSize.cx);
#endif
	}

	m_nPageHeight = m_LogicalPageSize.cy - GetFixedRowHeight()
		- (m_nHeaderHeight + m_nFooterHeight + 2 * m_nGap) * m_CharSize.cy;

	// Get the number of pages. Assumes no row is bigger than the page size.
	int nTotalRowHeight = 0;
	m_nNumPages = 1;
	for (int row = GetFixedRowCount(); row < GetRowCount(); row++)
	{
		nTotalRowHeight += GetRowHeight(row);
		if (nTotalRowHeight > m_nPageHeight)
		{
			m_nNumPages++;
			nTotalRowHeight = GetRowHeight(row);
		}
	}

	// now, figure out how many additional pages must print out if rows ARE bigger
	//  than page size
	int iColumnOffset = 0;
	int i1;
	for (i1 = 0; i1 < GetFixedColumnCount(); i1++)
	{
		iColumnOffset += GetColumnWidth(i1);
	}
	m_nPageWidth = m_LogicalPageSize.cx - iColumnOffset
		- nMargins;
	m_nPageMultiplier = 1;

	if (m_bWysiwygPrinting)
	{
		int iTotalRowWidth = 0;
		for (i1 = GetFixedColumnCount(); i1 < GetColumnCount(); i1++)
		{
			iTotalRowWidth += GetColumnWidth(i1);
			if (iTotalRowWidth > m_nPageWidth)
			{
				m_nPageMultiplier++;
				iTotalRowWidth = GetColumnWidth(i1);
			}
		}
		m_nNumPages *= m_nPageMultiplier;
	}

	// Set up the print info
	pInfo->SetMaxPage(m_nNumPages);
	pInfo->m_nCurPage = 1; // start printing at page# 1

	ReleaseDC(pCurrentDC);
	p_dc->SelectObject(pOldFont);
}

void GridCtrl::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	if (!p_dc || !pInfo)
		return;

	//CRect rcPage(pInfo->m_rectDraw);
	CFont* pOldFont = p_dc->SelectObject(&m_PrinterFont);

	// Set the page map mode to use GridCtrl units, and setup margin
	p_dc->SetMapMode(MM_ANISOTROPIC);
	p_dc->SetWindowExt(m_LogicalPageSize);
	p_dc->SetViewportExt(m_PaperSize);
	p_dc->SetWindowOrg(-m_nLeftMargin * m_CharSize.cx, 0);

	// Header
	pInfo->m_rectDraw.top = 0;
	pInfo->m_rectDraw.left = 0;
	pInfo->m_rectDraw.right = m_LogicalPageSize.cx - (m_nLeftMargin + m_nRightMargin) * m_CharSize.cx;
	pInfo->m_rectDraw.bottom = m_nHeaderHeight * m_CharSize.cy;
	PrintHeader(p_dc, pInfo);
	p_dc->OffsetWindowOrg(0, -m_nHeaderHeight * m_CharSize.cy);

	// Gap between header and column headings
	p_dc->OffsetWindowOrg(0, -m_nGap * m_CharSize.cy);

	p_dc->OffsetWindowOrg(0, -GetFixedRowHeight());

	// We need to find out which row to start printing for this page.
	int nTotalRowHeight = 0;
	UINT nNumPages = 1;
	m_nCurrPrintRow = GetFixedRowCount();

	// Not only the row, but we need to figure out column, too

	// Can print 4 pages, where page 1 and 2 represent the same rows but
	// with different WIDE columns.
	//
	// .......
	// .1 .2 .  If representing page 3  -->    iPageIfIgnoredWideCols = 2
	// .......                                 iWideColPageOffset = 0
	// .3 .4 .  If representing page 2  -->    iPageIfIgnoredWideCols = 1
	// .......                                 iWideColPageOffset = 1

	int iPageIfIgnoredWideCols = pInfo->m_nCurPage / m_nPageMultiplier;
	int iWideColPageOffset = pInfo->m_nCurPage - (iPageIfIgnoredWideCols * m_nPageMultiplier);
	if (iWideColPageOffset > 0)
		iPageIfIgnoredWideCols++;

	if (iWideColPageOffset == 0)
		iWideColPageOffset = m_nPageMultiplier;
	iWideColPageOffset--;

	// calculate current print row based on iPageIfIgnoredWideCols
	while (m_nCurrPrintRow < GetRowCount()
		&& static_cast<int>(nNumPages) < iPageIfIgnoredWideCols)
	{
		nTotalRowHeight += GetRowHeight(m_nCurrPrintRow);
		if (nTotalRowHeight > m_nPageHeight)
		{
			nNumPages++;
			if (static_cast<int>(nNumPages) == iPageIfIgnoredWideCols) break;
			nTotalRowHeight = GetRowHeight(m_nCurrPrintRow);
		}
		m_nCurrPrintRow++;
	}

	m_nPrintColumn = GetFixedColumnCount();
	int iTotalRowWidth = 0;
	int i1, i2;

	// now, calculate which print column to start displaying
	for (i1 = 0; i1 < iWideColPageOffset; i1++)
	{
		for (i2 = m_nPrintColumn; i2 < GetColumnCount(); i2++)
		{
			iTotalRowWidth += GetColumnWidth(i2);
			if (iTotalRowWidth > m_nPageWidth)
			{
				m_nPrintColumn = i2;
				iTotalRowWidth = 0;
				break;
			}
		}
	}

	PrintRowButtons(p_dc, pInfo); // print row buttons on each page
	int iColumnOffset = 0;
	for (i1 = 0; i1 < GetFixedColumnCount(); i1++)
	{
		iColumnOffset += GetColumnWidth(i1);
	}

	// Print the column headings
	pInfo->m_rectDraw.bottom = GetFixedRowHeight();

	if (m_nPrintColumn == GetFixedColumnCount())
	{
		// have the column headings fcn draw the upper left fixed cells
		//  for the very first columns, only
		p_dc->OffsetWindowOrg(0, +GetFixedRowHeight());

		m_nPageWidth += iColumnOffset;
		m_nPrintColumn = 0;
		PrintColumnHeadings(p_dc, pInfo);
		m_nPageWidth -= iColumnOffset;
		m_nPrintColumn = GetFixedColumnCount();

		p_dc->OffsetWindowOrg(-iColumnOffset, -GetFixedRowHeight());
	}
	else
	{
		// changed all of this here to match above almost exactly same
		p_dc->OffsetWindowOrg(0, +GetFixedRowHeight());

		m_nPageWidth += iColumnOffset;

		// print from column 0 ... last column that fits on the current page
		PrintColumnHeadings(p_dc, pInfo);

		m_nPageWidth -= iColumnOffset;

		p_dc->OffsetWindowOrg(-iColumnOffset, -GetFixedRowHeight());
	}

	if (m_nCurrPrintRow >= GetRowCount()) return;

	// Draw as many rows as will fit on the printed page.
	// Clip the printed page so that there is no partially shown
	// row at the bottom of the page (the same row which will be fully
	// shown at the top of the next page).

	BOOL bFirstPrintedRow = TRUE;
	CRect rect;
	rect.bottom = -1;
	while (m_nCurrPrintRow < GetRowCount())
	{
		rect.top = rect.bottom + 1;
		rect.bottom = rect.top + GetRowHeight(m_nCurrPrintRow) - 1;

		if (rect.bottom > m_nPageHeight) break; // Gone past end of page

		rect.right = -1;

		// modified to allow printing of wide grids on multiple pages
		for (int col = m_nPrintColumn; col < GetColumnCount(); col++)
		{
			rect.left = rect.right + 1;
			rect.right = rect.left
				+ GetColumnWidth(col)
				- 1;

			if (rect.right > m_nPageWidth)
				break;

			GridCellBase* pCell = GetCell(m_nCurrPrintRow, col);
			if (pCell)
				pCell->PrintCell(p_dc, m_nCurrPrintRow, col, rect);

			if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
			{
				int Overlap = (col == 0) ? 0 : 1;
				p_dc->MoveTo(rect.left - Overlap, rect.bottom);
				p_dc->LineTo(rect.right, rect.bottom);
				if (m_nCurrPrintRow == 0)
				{
					p_dc->MoveTo(rect.left - Overlap, rect.top);
					p_dc->LineTo(rect.right, rect.top);
				}
			}
			if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
			{
				int Overlap = (bFirstPrintedRow) ? 0 : 1;
				p_dc->MoveTo(rect.right, rect.top - Overlap);
				p_dc->LineTo(rect.right, rect.bottom);
				if (col == 0)
				{
					p_dc->MoveTo(rect.left, rect.top - Overlap);
					p_dc->LineTo(rect.left, rect.bottom);
				}
			}
		}
		m_nCurrPrintRow++;
		bFirstPrintedRow = FALSE;
	}

	// Footer
	pInfo->m_rectDraw.bottom = m_nFooterHeight * m_CharSize.cy;
	p_dc->SetWindowOrg(-m_nLeftMargin * m_CharSize.cx,
	                   -m_LogicalPageSize.cy + m_nFooterHeight * m_CharSize.cy);
	PrintFooter(p_dc, pInfo);

	// SetWindowOrg back for next page
	p_dc->SetWindowOrg(0, 0);

	p_dc->SelectObject(pOldFont);
}

// added by M.Fletcher 12/17/00
void GridCtrl::PrintFixedRowCells(int nStartColumn, int nStopColumn, int& row, CRect& rect,
                                   CDC* p_dc, BOOL& bFirst)
{
	// print all cells from nStartColumn to nStopColumn on row
	for (int col = nStartColumn; col < nStopColumn; col++)
	{
		rect.left = rect.right + 1;
		rect.right = rect.left + GetColumnWidth(col) - 1;

		if (rect.right > m_nPageWidth)
			break;

		GridCellBase* pCell = GetCell(row, col);
		if (pCell)
			pCell->PrintCell(p_dc, row, col, rect);

		if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
		{
			const int Overlap = (col == 0) ? 0 : 1;

			p_dc->MoveTo(rect.left - Overlap, rect.bottom);
			p_dc->LineTo(rect.right, rect.bottom);

			if (row == 0)
			{
				p_dc->MoveTo(rect.left - Overlap, rect.top);
				p_dc->LineTo(rect.right, rect.top);
			}
		}

		if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
		{
			const int Overlap = (row == 0) ? 0 : 1;

			p_dc->MoveTo(rect.right, rect.top - Overlap);
			p_dc->LineTo(rect.right, rect.bottom);

			if (bFirst)
			{
				p_dc->MoveTo(rect.left - 1, rect.top - Overlap);
				p_dc->LineTo(rect.left - 1, rect.bottom);
				bFirst = FALSE;
			}
		}
	} // end of column cells loop
} // end of GridCtrl::PrintFixedRowCells

void GridCtrl::PrintColumnHeadings(CDC* p_dc, CPrintInfo* /*pInfo*/)
{
	CFont* pOldFont = p_dc->SelectObject(&m_PrinterFont);

	CRect rect;
	rect.bottom = -1;

	BOOL bFirst = TRUE;

	// modified to allow column hdr printing of multi-page wide grids
	for (int row = 0; row < GetFixedRowCount(); row++)
	{
		rect.top = rect.bottom + 1;
		rect.bottom = rect.top + GetRowHeight(row) - 1;

		rect.right = -1;

		// if printColumn > fixedcolumncount we are on page 2 or more
		// lets printout those fixed cell headings again the 1 or more that would be missed
		// added by M.Fletcher 12/17/00
		if (m_nPrintColumn >= GetFixedColumnCount())
		{
			const BOOL bOriginal = bFirst;
			// lets print the missing fixed cells on left first out to last fixed column
			PrintFixedRowCells(0, GetFixedColumnCount(), row, rect, p_dc, bFirst);
			bFirst = bOriginal;
		}

		// now back to normal business print cells in heading after all fixed columns
		PrintFixedRowCells(m_nPrintColumn, GetColumnCount(), row, rect, p_dc, bFirst);
	} // end of Row Loop

	p_dc->SelectObject(pOldFont);
} // end of GridCtrl::PrintColumnHeadings

/*****************************************************************************
Prints line of row buttons on each page of the printout.  Assumes that
the window origin is setup before calling

*****************************************************************************/
void GridCtrl::PrintRowButtons(CDC* p_dc, CPrintInfo* /*pInfo*/)
{
	CFont* pOldFont = p_dc->SelectObject(&m_PrinterFont);

	CRect rect;
	rect.right = -1;

	BOOL bFirst = TRUE;
	for (int iCol = 0; iCol < GetFixedColumnCount(); iCol++)
	{
		rect.left = rect.right + 1;
		rect.right = rect.left
			+ GetColumnWidth(iCol)
			- 1;

		rect.bottom = -1;
		for (int iRow = m_nCurrPrintRow; iRow < GetRowCount(); iRow++)
		{
			rect.top = rect.bottom + 1;
			rect.bottom = rect.top + GetRowHeight(iRow) - 1;

			if (rect.bottom > m_nPageHeight)
				break;

			GridCellBase* pCell = GetCell(iRow, iCol);
			if (pCell)
				pCell->PrintCell(p_dc, iRow, iCol, rect);

			if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
			{
				const int Overlap = (iCol == 0) ? 0 : 1;
				p_dc->MoveTo(rect.left - Overlap, rect.bottom);
				p_dc->LineTo(rect.right, rect.bottom);
				if (bFirst)
				{
					p_dc->MoveTo(rect.left - Overlap, rect.top - 1);
					p_dc->LineTo(rect.right, rect.top - 1);
					bFirst = FALSE;
				}
			}
			if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
			{
				const int Overlap = (iRow == 0) ? 0 : 1;
				p_dc->MoveTo(rect.right, rect.top - Overlap);
				p_dc->LineTo(rect.right, rect.bottom);
				if (iCol == 0)
				{
					p_dc->MoveTo(rect.left, rect.top - Overlap);
					p_dc->LineTo(rect.left, rect.bottom);
				}
			}
		}
	}
	p_dc->SelectObject(pOldFont);
}

void GridCtrl::PrintHeader(CDC* p_dc, CPrintInfo* pInfo)
{
	// print App title on top right margin
	CString strRight;
	BOOL flag = strRight.LoadString(AFX_IDS_APP_TITLE);

	// print parent window title in the centre (Gert Rijs)
	CString strCenter;
	const CWnd* pParentWnd = GetParent();
	while (pParentWnd)
	{
		pParentWnd->GetWindowText(strCenter);
		if (strCenter.GetLength()) // can happen if it is a CView, CChildFrm has the title
			break;
		pParentWnd = pParentWnd->GetParent();
	}

	CFont BoldFont;
	LOGFONT lf;

	//create bold font for header and footer
	VERIFY(m_PrinterFont.GetLogFont(&lf));
	lf.lfWeight = FW_BOLD;
	VERIFY(BoldFont.CreateFontIndirect(&lf));

	CFont* pNormalFont = p_dc->SelectObject(&BoldFont);
	const int nPrevBkMode = p_dc->SetBkMode(TRANSPARENT);

	CRect rc(pInfo->m_rectDraw);
	if (!strCenter.IsEmpty())
		p_dc->DrawText(strCenter, &rc, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);
	if (!strRight.IsEmpty())
		p_dc->DrawText(strRight, &rc, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

	p_dc->SetBkMode(nPrevBkMode);
	p_dc->SelectObject(pNormalFont);
	BoldFont.DeleteObject();

	// draw ruled-line across top
	p_dc->SelectStockObject(BLACK_PEN);
	p_dc->MoveTo(rc.left, rc.bottom);
	p_dc->LineTo(rc.right, rc.bottom);
}

//print footer with a line and date, and page number
void GridCtrl::PrintFooter(CDC* p_dc, CPrintInfo* pInfo)
{
	// page numbering on left
	CString strLeft;
	strLeft.Format(_T("Page %d of %d"), pInfo->m_nCurPage, pInfo->GetMaxPage());

	// date and time on the right
	CString strRight;
	COleDateTime t = COleDateTime::GetCurrentTime();
	strRight = t.Format(_T("%c"));

	CRect rc(pInfo->m_rectDraw);

	// draw ruled line on bottom
	p_dc->SelectStockObject(BLACK_PEN);
	p_dc->MoveTo(rc.left, rc.top);
	p_dc->LineTo(rc.right, rc.top);

	CFont BoldFont;
	LOGFONT lf;

	//create bold font for header and footer
	m_PrinterFont.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	BoldFont.CreateFontIndirect(&lf);

	CFont* pNormalFont = p_dc->SelectObject(&BoldFont);
	int nPrevBkMode = p_dc->SetBkMode(TRANSPARENT);

	// EFW - Bug fix - Force text color to black.  It doesn't always
	// get set to a printable color when it gets here.
	p_dc->SetTextColor(col_black);

	if (!strLeft.IsEmpty())
		p_dc->DrawText(strLeft, &rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);
	if (!strRight.IsEmpty())
		p_dc->DrawText(strRight, &rc, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

	p_dc->SetBkMode(nPrevBkMode);
	p_dc->SelectObject(pNormalFont);
	BoldFont.DeleteObject();
}

void GridCtrl::OnEndPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	m_PrinterFont.DeleteObject();
}

#endif  // !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)

#ifndef _WIN32_WCE
/////////////////////////////////////////////////////////////////////////////
// GridCtrl persistance

BOOL GridCtrl::Save(LPCTSTR filename, TCHAR chSeparator/*=_T(',')*/)
{
	CStdioFile File;
	CFileException ex;
	const CString strSeparator(chSeparator);

	if (!File.Open(filename, CFile::modeWrite | CFile::modeCreate | CFile::typeText, &ex))
	{
		ex.ReportError();
		return FALSE;
	}

	TRY
		{
			const int nNumColumns = GetColumnCount();
			for (int i = 0; i < GetRowCount(); i++)
			{
				for (int j = 0; j < nNumColumns; j++)
				{
					File.WriteString(GetItemText(i, j));
					File.WriteString((j == (nNumColumns - 1)) ? _T("\n") : strSeparator);
				}
			}

			File.Close();
		}

	CATCH(CFileException, e)
		{
			AfxMessageBox(_T("Unable to save grid list"));
			return FALSE;
		}
	END_CATCH

	return TRUE;
}

BOOL GridCtrl::Load(LPCTSTR filename, TCHAR chSeparator/*=_T(',')*/)
{
	if (GetVirtualMode())
		return FALSE;

	CStdioFile File;
	CFileException ex;

	if (!File.Open(filename, CFile::modeRead | CFile::typeText))
	{
		ex.ReportError();
		return FALSE;
	}

	DeleteAllItems();

	TRY
		{
			TCHAR *token;
			TCHAR *end;
			TCHAR buffer[1024];
			// Read Header off file
			File.ReadString(buffer, 1024);

			// Get first token
			for (token = buffer, end = buffer;
			     *end && (*end != chSeparator) && (*end != _T('\n'));
			     end++);

			if ((*end == _T('\0')) && (token == end))
				token = nullptr;

			*end = _T('\0');

			while (token)
			{
				InsertColumn(token);

				// Get next token
				for (token = ++end; *end && (*end != chSeparator) && (*end != _T('\n'));
				     end++);

				if ((*end == _T('\0')) && (token == end))
					token = nullptr;

				*end = _T('\0');
			}

			// Read in rest of data
			int nItem = 1;
			while (File.ReadString(buffer, 1024))
			{
				// Get first token
				for (token = buffer, end = buffer;
				     *end && (*end != chSeparator) && (*end != _T('\n')); end++);

				if ((*end == _T('\0')) && (token == end))
					token = nullptr;

				*end = _T('\0');

				int nSubItem = 0;
				while (token)
				{
					if (!nSubItem)
						InsertRow(token);
					else
						SetItemText(nItem, nSubItem, token);

					// Get next token
					for (token = ++end; *end && (*end != chSeparator) && (*end != _T('\n'));
					     end++);

					if ((*end == _T('\0')) && (token == end))
						token = nullptr;

					*end = _T('\0');

					nSubItem++;
				}
				nItem++;
			}

			AutoSizeColumns(GetAutoSizeStyle());
			File.Close();
		}

	CATCH(CFileException, e)
		{
			AfxMessageBox(_T("Unable to load grid data"));
			return FALSE;
		}
	END_CATCH

	return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// GridCtrl overrideables

#ifndef GRIDCONTROL_NO_DRAGDROP
// This is no longer needed since I've changed to OLE drag and drop - but it's
// still cool code. :)
CImageList* GridCtrl::CreateDragImage(CPoint* pHotSpot)
{
	CDC* p_dc = GetDC();
	if (!p_dc)
		return nullptr;

	CRect rect;
	const GridCellID cell = GetFocusCell();
	if (!GetCellRect(cell.row, cell.col, rect))
		return nullptr;

	// Translate coordinate system
	rect.BottomRight() = CPoint(rect.Width(), rect.Height());
	rect.TopLeft() = CPoint(0, 0);
	*pHotSpot = rect.BottomRight();

	// Create a new imagelist (the caller of this function has responsibility
	// for deleting this list)
	const auto pList = new CImageList;
	if (!pList || !pList->Create(rect.Width(), rect.Height(), ILC_MASK, 1, 1))
	{
		delete pList;
		return nullptr;
	}

	// Create mem DC and bitmap
	CDC MemDC;
	CBitmap bm;
	MemDC.CreateCompatibleDC(p_dc);
	bm.CreateCompatibleBitmap(p_dc, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = MemDC.SelectObject(&bm);
	MemDC.SetWindowOrg(0, 0);

	// Draw cell onto bitmap in memDC
	GridCellBase* pCell = GetCell(cell.row, cell.col);
	if (pCell)
		pCell->Draw(&MemDC, cell.row, cell.col, rect, FALSE);

	// Clean up
	MemDC.SelectObject(pOldBitmap);
	ReleaseDC(p_dc);

	// Add the bitmap we just drew to the image list.
	pList->Add(&bm, GetDefaultCell(FALSE, FALSE)->GetBackClr());
	bm.DeleteObject();

	return pList;
}
#endif

void GridCtrl::OnFixedRowClick(GridCellID& cell)
{
	if (!IsValid(cell))
		return;

	if (GetHeaderSort())
	{
		CWaitCursor waiter;
		if (cell.col == GetSortColumn())
			SortItems(cell.col, !GetSortAscending());
		else
			SortItems(cell.col, TRUE);
		Invalidate();
	}

	// Did the user click on a fixed column cell (so the cell was within the overlap of
	// fixed row and column cells) - (fix by David Pritchard)
	if (GetFixedColumnSelection())
	{
		if (cell.col < GetFixedColumnCount())
		{
			m_MouseMode = MOUSE_SELECT_ALL;
			OnSelecting(cell);
		}
		else
		{
			m_MouseMode = MOUSE_SELECT_COL;
			OnSelecting(cell);
		}
	}
}

void GridCtrl::OnFixedColumnClick(GridCellID& cell)
{
	if (!IsValid(cell))
		return;

	//    if (m_bListMode && (GetItemState(cell.row, m_nFixedCols) & GVNI_SELECTED))
	//    {
	//        OnEditCell(cell.row, cell.col, VK_LBUTTON);
	//        return;
	//    }

	// Did the user click on a fixed row cell (so the cell was within the overlap of
	// fixed row and column cells) - (fix by David Pritchard)
	if (GetFixedRowSelection())
	{
		if (cell.row < GetFixedRowCount())
		{
			m_MouseMode = MOUSE_SELECT_ALL;
			OnSelecting(cell);
		}
		else
		{
			m_MouseMode = MOUSE_SELECT_ROW;
			OnSelecting(cell);
		}
	}
}

// Gets the extent of the text pointed to by str (no CDC needed)
// By default this uses the selected font (which is a bigger font)
CSize GridCtrl::GetTextExtent(int nRow, int nCol, LPCTSTR str)
{
	GridCellBase* pCell = GetCell(nRow, nCol);
	if (!pCell)
		return CSize(0, 0);
	return pCell->GetTextExtent(str);
}

// virtual
void GridCtrl::OnEditCell(int nRow, int nCol, CPoint point, UINT nChar)
{
#ifndef GRIDCONTROL_NO_TITLETIPS
	m_TitleTip.Hide(); // hide any titletips
#endif

	// Can we do it?
	GridCellID cell(nRow, nCol);
	if (!IsValid(cell) || !IsCellEditable(nRow, nCol))
		return;

	// Can we see what we are doing?
	EnsureVisible(nRow, nCol);
	if (!IsCellVisible(nRow, nCol))
		return;

	// Where, exactly, are we gonna do it??
	CRect rect;
	if (!GetCellRect(cell, rect))
		return;

	// Check we can edit...
	if (SendMessageToParent(nRow, nCol, GVN_BEGINLABELEDIT) >= 0)
	{
		// Let's do it...
		GridCellBase* pCell = GetCell(nRow, nCol);
		if (pCell)
			pCell->Edit(nRow, nCol, rect, point, IDC_INPLACE_CONTROL, nChar);
	}
}

// virtual
void GridCtrl::EndEditing()
{
	const GridCellID cell = GetFocusCell();
	if (!IsValid(cell)) return;
	GridCellBase* pCell = GetCell(cell.row, cell.col);
	if (pCell)
		pCell->EndEdit();
}

// virtual
void GridCtrl::OnEndEditCell(int nRow, int nCol, CString str)
{
	const CString strCurrentText = GetItemText(nRow, nCol);
	if (strCurrentText != str)
	{
		SetItemText(nRow, nCol, str);
		if (ValidateEdit(nRow, nCol, str) &&
			SendMessageToParent(nRow, nCol, GVN_ENDLABELEDIT) >= 0)
		{
			SetModified(TRUE, nRow, nCol);
			RedrawCell(nRow, nCol);
		}
		else
		{
			SetItemText(nRow, nCol, strCurrentText);
		}
	}

	GridCellBase* pCell = GetCell(nRow, nCol);
	if (pCell)
		pCell->OnEndEdit();
}

// If this returns FALSE then the editing isn't allowed
// virtual
BOOL GridCtrl::ValidateEdit(int nRow, int nCol, LPCTSTR str)
{
	GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return TRUE;

	return pCell->ValidateEdit(str);
}

// virtual
CString GridCtrl::GetItemText(int nRow, int nCol) const
{
	if (nRow < 0 || nRow >= m_nRows || nCol < 0 || nCol >= m_nCols)
		return _T("");

	const GridCellBase* pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return _T("");

	return pCell->GetText();
}

void GridCtrl::ResetVirtualOrder()
{
	m_arRowOrder.resize(m_nRows);
	for (int i = 0; i < m_nRows; i++)
	{
		m_arRowOrder[i] = i;
	}
}

void GridCtrl::Reorder(int From, int To)
{
	// Set line From just after Line To
	ASSERT(From >= GetFixedRowCount() && To >= GetFixedRowCount() - 1 && From < m_nRows&& To < m_nRows);
	const int Value = m_arRowOrder[From];
	m_arRowOrder.erase(m_arRowOrder.begin() + From);
	const int Offset = (From >= To ? 1 : 0);
	m_arRowOrder.insert(m_arRowOrder.begin() + To + Offset, Value);
}
