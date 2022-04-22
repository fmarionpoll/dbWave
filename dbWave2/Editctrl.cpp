// CEditCtrl.cpp : implementation file
//
// CEditCtrl is a control derived from the CEdit control, ie it accepts
// characters from the keyboard and store them as text within the window.
//
// CEditCtrl is used to subclass edit controls and check the incoming keys
// when the user press CR, the field is considered as validated and
// a message is sent to the parent to signal that.
//
// in addition CEditCtrl traps up/down arrow keys and signals it to the parent
// so that the programmer can implement an automatic increment/decrement at the
// parent level
//
// lastly but not least, if the programmer wants that the edit control to be displayed
// with a short vertical scrollbar, the original edit control only needs to be declared
// (within the rc file or from within App studio's menu) as multiline and with a vertical
// scrollbar. CEditCtrl will trap scroll messages and forward them to the parent so
// that the value of the edit control can be changed. Vertical scroll messages are sent to
// the parent as VK_UP or VK_DOWN keys
//
// communication with parent window is done via messages posted and public variables.
// 	(::GetParent(m_hWnd), WM_COMMAND, ::GetDlgCtrlID(m_hWnd), MAKELONG(m_hWnd, EN_CHANGE));
// posted message is EN_CHANGE
// the state of the control is governed by 3 parameters.
// 	BOOL	m_bEntryDone;  TRUE only if CR, or up/down arrows = input is valid
//	WORD	m_nChar;		number of characters within windows text buffer
//
// the parent window should therefore trap EN_CHANGE messages from the sub-classed edit
// control and take action oly when m_bEntryDone is TRUE. It is to the responsibility
// of this parent to reset m_bEntryDone to FALSE when input is processed so that a new
// value can be validated.
//

#include "StdAfx.h"
#include "Editctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CEditCtrl::CEditCtrl()
= default;

CEditCtrl::~CEditCtrl()
= default;

BEGIN_MESSAGE_MAP(CEditCtrl, CWnd)
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


UINT CEditCtrl::OnGetDlgCode()
{
	return CEdit::OnGetDlgCode() | DLGC_WANTALLKEYS;
}

void CEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// VK_SPACE (20), _PRIOR, _NEXT, _END, _HOME, _LEFT, _UP, _RIGHT, _DOWN, _SELECT(28)
	if (nChar > VK_SPACE && nChar < VK_SELECT)
		ProcessKeys(nChar);
	else
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!ProcessKeys(nChar))
		CEdit::OnChar(nChar, nRepCnt, nFlags);
}

//--------------------------------------------------------------------------
// ProcessKeys
// envoie un message EN_CHANGE
// quand le control est validé par CR
// quand l'utilisateur appuie sur arrow & page UP/DOWN
//
// de-select et select le prochain control en réponse à TAB, arrow left/right
//--------------------------------------------------------------------------

BOOL CEditCtrl::ProcessKeys(UINT nChar)
{
	switch (nChar)
	{
	case VK_TAB: 
		{
			const auto b_next = (GetKeyState(VK_SHIFT) & 0x8000);
			const auto h_next = ::GetNextDlgGroupItem(::GetParent(m_hWnd), m_hWnd, b_next);
			::SetFocus(h_next); 
		}
		break;

	case VK_RETURN: 
	case VK_UP: 
	case VK_DOWN: 
	case VK_PRIOR:
	case VK_NEXT: 
		m_bEntryDone = TRUE;
		m_nChar = nChar;
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), EN_CHANGE), reinterpret_cast<LPARAM>(m_hWnd));
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

void CEditCtrl::OnEnChange(CWnd* parent_wnd, float& parameter, float delta_up, float delta_down)
{
	
	;
	switch (m_nChar)
	{
	case VK_RETURN:
		//parent_wnd->UpdateData(TRUE);
		{
			CString cs;
			GetWindowText(cs);
			parameter = static_cast<float>(_ttof(cs));
		}
		break;
	case VK_UP:
	case VK_PRIOR:
		parameter += delta_up;
		break;
	case VK_DOWN:
	case VK_NEXT:
		parameter += delta_down;
		break;
	default:;
	}
	m_bEntryDone = FALSE;
	m_nChar = 0;
	SetSel(0, -1);
}

void CEditCtrl::OnEnChange(CWnd* parent_wnd, int& parameter, int delta_up, int delta_down)
{
	switch (m_nChar)
	{
	case VK_RETURN:
		//parent_wnd->UpdateData(TRUE);
		{
			CString cs;
			GetWindowText(cs);
			parameter = static_cast<int>(_ttoi(cs));
		}
		break;
	case VK_UP:
	case VK_PRIOR:
		parameter += delta_up;
		break;
	case VK_DOWN:
	case VK_NEXT:
		parameter += delta_down;
		break;
	default:;
	}
	m_bEntryDone = FALSE;
	m_nChar = 0;
	SetSel(0, -1);
}

void CEditCtrl::OnEnChange(CWnd* parent_wnd, UINT& parameter, UINT delta_up, UINT delta_down)
{
	switch (m_nChar)
	{
	case VK_RETURN:
		//parent_wnd->UpdateData(TRUE);
		{
			CString cs;
			GetWindowText(cs);
			parameter = static_cast<int>(_ttoi(cs));
		}
		break;
	case VK_UP:
	case VK_PRIOR:
		parameter += delta_up;
		break;
	case VK_DOWN:
	case VK_NEXT:
		parameter += delta_down;
		break;
	default:;
	}
	m_bEntryDone = FALSE;
	m_nChar = 0;
	SetSel(0, -1);
}

//--------------------------------------------------------------------------
// OnSetFocus()
// quand le control est en mode "caret" (barre vertic clignotante)
// Postmessage n'arrive pas au control!? (ex OnEnChangeChannel)
// cela marche par contre quand tout le texte est selectionne
//
// OnSetFocus et le SetSel inclus dans OnEnChangeChannel permettent
// d'éviter cela lorsque (1) le control obtient le focus et (2) quand
// le contenu du control est validé par l'utilisateur
//--------------------------------------------------------------------------
void CEditCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	SetSel(0, -1);
}

void CEditCtrl::OnKillFocus(CWnd* pNewWnd)
{
	if (!m_bEntryDone)
	{
		m_bEntryDone = TRUE;
		m_nChar = VK_RETURN;
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), EN_CHANGE), reinterpret_cast<LPARAM>(m_hWnd));
	}
	CWnd::OnKillFocus(pNewWnd);
}

void CEditCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (nSBCode == SB_LINEDOWN)
		m_nChar = VK_DOWN;
	else if (nSBCode == SB_LINEUP)
		m_nChar = VK_UP;
	else
		return; // nothing special
	m_bEntryDone = TRUE;
	GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), EN_CHANGE), reinterpret_cast<LPARAM>(m_hWnd));
}
