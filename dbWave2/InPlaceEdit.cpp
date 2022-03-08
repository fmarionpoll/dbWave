#include <StdAfx.h>
#include "InPlaceEdit.h"

#include "GridCtrl.h"

// The code contained in this file is based on the original
// CInPlaceEdit from
// https://www.codeguru.com/cplusplus/editable-subitems/

CInPlaceEdit::CInPlaceEdit(CWnd* parent, int iItem, int iSubItem, CString sInitText)
	:m_sInitText(sInitText)
{
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_sInitText = sInitText;
	m_parent = parent;
}

CInPlaceEdit::~CInPlaceEdit()
{
}

BEGIN_MESSAGE_MAP(CInPlaceEdit, CEdit)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
END_MESSAGE_MAP()


BOOL CInPlaceEdit::PreTranslateMessage(MSG* pMsg)
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


void CInPlaceEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

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
	dispinfo.item.pszText = m_bESC ? NULL : LPTSTR((LPCTSTR)str);
	dispinfo.item.cchTextMax = str.GetLength();

	m_parent -> SendMessage(WM_NOTIFY, IDC_INPLACE_CONTROL, (LPARAM)&dispinfo);
	DestroyWindow();
}

void CInPlaceEdit::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;
}


void CInPlaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		if (nChar == VK_ESCAPE)
			m_bESC = TRUE;
		GetParent()->SetFocus();
		return;
	}


	CEdit::OnChar(nChar, nRepCnt, nFlags);

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

int CInPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

