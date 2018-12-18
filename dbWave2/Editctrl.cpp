// CEditCtrl.cpp : implementation file
//
// CEditCtrl is a control derived from the CEdit control, ie it accept
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
// the parent window should therefore trap EN_CHANGE messages from the subclassed edit
// control and take action oly when m_bEntryDone is TRUE. It is to the responsablility
// of this parent to reset m_bEntryDone to FALSE when input is processed so that a new
// value can be validated.
// 

#include "StdAfx.h"
#include "Editctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditCtrl

CEditCtrl::CEditCtrl()
{
	m_bEntryDone = FALSE;	
	m_nChar=0;	
}

CEditCtrl::~CEditCtrl()
{
}

BEGIN_MESSAGE_MAP(CEditCtrl, CWnd)
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_VSCROLL()	
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditCtrl message handlers
UINT CEditCtrl::OnGetDlgCode()
{
	return CEdit::OnGetDlgCode() | DLGC_WANTALLKEYS;
}

//--------------------------------------------------------------------------
void CEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// VK_SPACE (20), _PRIOR, _NEXT, _END, _HOME, _LEFT, _UP, _RIGHT, _DOWN, _SELECT(28)
	if (nChar > VK_SPACE && nChar < VK_SELECT)
		ProcessKeys(nChar);
	else		
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}
//--------------------------------------------------------------------------
void CEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!ProcessKeys(nChar))		
		CEdit::OnChar(nChar, nRepCnt, nFlags);
}

//--------------------------------------------------------------------------
// ProcessKeys
// envoie un message EN_CHANGE
// quand le control est valid� par CR
// quand l'utilisateur appuie sur arrow & page UP/DOWN
//
// de-select et select le prochain control en r�ponse � TAB, arrow left/right
//--------------------------------------------------------------------------
BOOL CEditCtrl::ProcessKeys(UINT nChar)
{
	switch (nChar)
	{		
	case VK_TAB:					// change selection with TAB
		{
		BOOL bNext = (GetKeyState(VK_SHIFT) & 0x8000);			
		HWND hNext = ::GetNextDlgGroupItem(::GetParent(m_hWnd), m_hWnd, bNext);
		::SetFocus(hNext);			// select next dlg item
		}
		break;

	case VK_RETURN:		// post message to parent
	case VK_UP:			// arrow up
	case VK_DOWN:		// arrow down
	case VK_PRIOR:		// page up
	case VK_NEXT:		// page down
		m_bEntryDone=TRUE;
		m_nChar=nChar;
		GetParent()->PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), EN_CHANGE),(LPARAM) m_hWnd);
		break;

	default:
		return FALSE;
	}
	// change select	
	return TRUE;
}
//--------------------------------------------------------------------------
// OnSetFocus()
// quand le control est en mode "caret" (barre vertic clignotante)
// Postmessage n'arrive pas au control!? (ex OnEnChangeChannel)
// cela marche par contre quand tout le texte est selectionne
//
// OnSetFocus et le SetSel inclus dans OnEnChangeChannel permettent
// d'�viter cela lorsque (1) le control obtient le focus et (2) quand
// le contenu du control est valid� par l'utilisateur
//--------------------------------------------------------------------------
void CEditCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);	
	SetSel(0, -1);	
}

// force validation if m_bEntry=FALSE (entry not processed)
void CEditCtrl::OnKillFocus(CWnd* pNewWnd)
{
	if (!m_bEntryDone)
	{
		m_bEntryDone = TRUE;
		m_nChar = VK_RETURN;
		GetParent()->PostMessage(WM_COMMAND,MAKELONG(GetDlgCtrlID(), EN_CHANGE),(LPARAM) m_hWnd);
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
	m_bEntryDone=TRUE;
	GetParent()->PostMessage(WM_COMMAND,MAKELONG(GetDlgCtrlID(), EN_CHANGE),(LPARAM) m_hWnd);
}

