#include <StdAfx.h>
#include "InPlaceEdit.h"
#include "GridCtrl.h"


// The code contained in this file is based on the original
// InPlaceEdit from
// https://www.codeguru.com/cplusplus/editable-subitems/

InPlaceEdit::InPlaceEdit(CWnd* parent, int iItem, int iSubItem, CString sInitText)
	:m_sInitText(sInitText)
{
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_sInitText = sInitText;
	m_parent = parent;
}

InPlaceEdit::InPlaceEdit(CWnd* p_parent, CRect& rect, DWORD dwStyle, UINT nID,
	int nRow, int nColumn, CString sInitText,
	UINT nFirstChar)
{
	m_sInitText = sInitText;
	m_nRow = nRow;
	m_nColumn = nColumn;
	m_nLastChar = 0;

	// pja - I find 'exit on arrows' to be counter intuitive. Ctrl-arrow or Tab
	//       should be good enough.
	m_bExitOnArrows = 0;//(nFirstChar != VK_LBUTTON);    // If mouse click brought us here,
													 // then no exit on arrows

	m_Rect = rect;  // For bizarre CE bug.

	const DWORD dwEditStyle = WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL //|ES_MULTILINE
		| dwStyle;
	if (!Create(dwEditStyle, rect, p_parent, nID)) return;

	SetFont(p_parent->GetFont());

	SetWindowText(sInitText);
	SetFocus();

	switch (nFirstChar) {
	case VK_LBUTTON:
	case VK_RETURN:   SetSel(static_cast<int>(_tcslen(m_sInitText)), -1); return;
	case VK_BACK:     SetSel(static_cast<int>(_tcslen(m_sInitText)), -1); break;
	case VK_TAB:
	case VK_DOWN:
	case VK_UP:
	case VK_RIGHT:
	case VK_LEFT:
	case VK_NEXT:
	case VK_PRIOR:
	case VK_HOME:
	case VK_SPACE:
	case VK_END:      SetSel(0, -1); return;
	default:          SetSel(0, -1);
	}

	// Added by KiteFly. When entering DBCS chars into cells the first char was being lost
	// SendMessage changed to PostMessage (John Lagerquist)
	if (nFirstChar < 0x80)
		PostMessage(WM_CHAR, nFirstChar);
	else
		PostMessage(WM_IME_CHAR, nFirstChar);
}

InPlaceEdit::~InPlaceEdit()
{
}

BEGIN_MESSAGE_MAP(InPlaceEdit, CEdit)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
END_MESSAGE_MAP()

BOOL InPlaceEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN
			|| pMsg->wParam == VK_DELETE
			|| pMsg->wParam == VK_ESCAPE
			|| GetKeyState(VK_CONTROL)
			)
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void InPlaceEdit::PostNcDestroy()
{
	CEdit::PostNcDestroy();
	//delete this;
}

void InPlaceEdit::OnKillFocus(CWnd* p_new_wnd)
{
	CEdit::OnKillFocus(p_new_wnd);

	CString str;
	GetWindowText(str);

	// Send Notification to parent of ListView ctrl
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = m_hWnd; // GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = IDC_INPLACE_CONTROL; // GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;

	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	dispinfo.item.pszText = m_bESC ? NULL : const_cast<LPTSTR>(static_cast<LPCTSTR>(str));
	dispinfo.item.cchTextMax = str.GetLength();

	m_parent -> SendMessage(WM_NOTIFY, IDC_INPLACE_CONTROL, reinterpret_cast<LPARAM>(&dispinfo));
	DestroyWindow();
}

void InPlaceEdit::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;
}

void InPlaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT n_flags)
{
	if (nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		if (nChar == VK_ESCAPE)
			m_bESC = TRUE;
		GetParent()->SetFocus();
		return;
	}


	CEdit::OnChar(nChar, nRepCnt, n_flags);

	// Resize edit control if needed

	// Get text extent
	CString str;
	GetWindowText(str);
	CWindowDC dc(this);
	CFont* pFont = GetParent()->GetFont();
	CFont* pFontDC = dc.SelectObject(pFont);
	CSize size = dc.GetTextExtent(str);
	dc.SelectObject(pFontDC);
	size.cx += 5;

	// Get client rect
	CRect rect, parentrect;
	GetClientRect(&rect);
	GetParent()->GetClientRect(&parentrect);
	ClientToScreen(&rect);
	GetParent()->ScreenToClient(&rect);

	// Check whether control needs to be resized
	if (size.cx > rect.Width())
	{
		if (size.cx + rect.left < parentrect.right)
			rect.right = rect.left + size.cx;
		else
			rect.right = parentrect.right;
		MoveWindow(&rect);
	}
}

int InPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	CFont* font = GetParent()->GetFont();
	SetFont(font);

	SetWindowText(m_sInitText);
	SetFocus();
	SetSel(0, -1);
	SetDlgCtrlID(IDC_INPLACE_CONTROL);
	return 0;
}

void InPlaceEdit::EndEdit()
{
	CString str;

	// EFW - BUG FIX - Clicking on a grid scroll bar in a derived class
	// that validates input can cause this to get called multiple times
	// causing assertions because the edit control goes away the first time.
	static BOOL bAlreadyEnding = FALSE;

	if (bAlreadyEnding)
		return;

	bAlreadyEnding = TRUE;
	GetWindowText(str);

	// Send Notification to parent
	GV_DISPINFO dispinfo;

	dispinfo.hdr.hwndFrom = GetSafeHwnd();
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = GVN_ENDLABELEDIT;

	dispinfo.item.mask = LVIF_TEXT | LVIF_PARAM;
	dispinfo.item.row = m_nRow;
	dispinfo.item.col = m_nColumn;
	dispinfo.item.strText = str;
	dispinfo.item.lParam = static_cast<LPARAM>(m_nLastChar);

	CWnd* pOwner = GetOwner();
	if (pOwner)
		pOwner->SendMessage(WM_NOTIFY, GetDlgCtrlID(), reinterpret_cast<LPARAM>(&dispinfo));

	// Close this window (PostNcDestroy will delete this)
	if (IsWindow(GetSafeHwnd()))
		SendMessage(WM_CLOSE, 0, 0);
	bAlreadyEnding = FALSE;
}


