#include "StdAfx.h"
#include "ComboEdit.h"
#include "GridInPlaceList.h"



ComboEdit::ComboEdit()
= default;

ComboEdit::~ComboEdit()
= default;

// win95 accelerator key problem workaround - Matt Weagle.
BOOL ComboEdit::PreTranslateMessage(MSG* pMsg)
{
	// Make sure that the keystrokes continue to the appropriate handlers
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		TranslateMessage(pMsg);
		::DispatchMessage(pMsg);
		return TRUE;
	}

	// Catch the Alt key so we don't choke if focus is going to an owner drawn button
	if (pMsg->message == WM_SYSCHAR)
		return TRUE;

	return CEdit::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(ComboEdit, CEdit)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()



void ComboEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	auto pOwner = static_cast<GridInPlaceList*>(GetOwner()); // This MUST be a GridInPlaceList
	if (pOwner)
		pOwner->EndEdit();
}

void ComboEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		nChar == VK_DOWN || nChar == VK_UP ||
		nChar == VK_RIGHT || nChar == VK_LEFT) &&
		(GetKeyState(VK_CONTROL) < 0 && GetDlgCtrlID() == IDC_COMBOEDIT))
	{
		CWnd* pOwner = GetOwner();
		if (pOwner)
			pOwner->SendMessage(WM_KEYDOWN, nChar, nRepCnt + (static_cast<DWORD>(nFlags) << 16));
		return;
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void ComboEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE)
	{
		CWnd* pOwner = GetOwner();
		if (pOwner)
			pOwner->SendMessage(WM_KEYUP, nChar, nRepCnt + (static_cast<DWORD>(nFlags) << 16));
		return;
	}

	if (nChar == VK_TAB || nChar == VK_RETURN || nChar == VK_ESCAPE)
	{
		CWnd* pOwner = GetOwner();
		if (pOwner)
			pOwner->SendMessage(WM_KEYUP, nChar, nRepCnt + (static_cast<DWORD>(nFlags) << 16));
		return;
	}

	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}
