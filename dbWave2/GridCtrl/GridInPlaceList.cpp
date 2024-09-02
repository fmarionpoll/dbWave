#include "StdAfx.h"

#include "GridCtrl.h"
#include "GridInPlaceList.h"


GridInPlaceList::GridInPlaceList(CWnd* p_parent, CRect& rect, DWORD dw_style, UINT nID,
                                   int nRow, int nColumn,
                                   COLORREF crFore, COLORREF crBack,
                                   CStringArray& Items, CString sInitText,
                                   UINT nFirstChar)
{
	m_crForeClr = crFore;
	m_crBackClr = crBack;
	m_nNumLines = 4;
	m_sInitText = sInitText;
	m_nRow = nRow;
	m_nCol = nColumn;
	m_nLastChar = 0;
	m_bExitOnArrows = FALSE; //(nFirstChar != VK_LBUTTON);	// If mouse click brought us here,

	// Create the combobox
	const DWORD dwComboStyle = WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		CBS_AUTOHSCROLL | dw_style;
	const int nHeight = rect.Height();
	rect.bottom = rect.bottom + m_nNumLines * nHeight + GetSystemMetrics(SM_CYHSCROLL);
	if (!Create(dwComboStyle, rect, p_parent, nID)) 
		return;

	// Add the strings
	for (int i = 0; i < Items.GetSize(); i++)
		AddString(Items[i]);

	SetFont(p_parent->GetFont());
	SetItemHeight(-1, nHeight);

	const int nMaxLength = GetCorrectDropWidth();
	/*
	if (nMaxLength > rect.GetRectWidth())
		rect.right = rect.left + nMaxLength;
	// Resize the edit window and the drop down window
	MoveWindow(rect);
	*/

	SetDroppedWidth(nMaxLength);

	SetHorizontalExtent(0); // no horz scrolling

	// Set the initial text to m_sInitText
	if (IsWindow(m_hWnd) && SelectString(-1, m_sInitText) == CB_ERR)
		SetWindowText(m_sInitText); // No text selected, so restore what was there before

	ShowDropDown();

	// Subclass the combobox edit control if style includes CBS_DROPDOWN
	if ((dw_style & CBS_DROPDOWNLIST) != CBS_DROPDOWNLIST)
	{
		m_edit.SubclassDlgItem(IDC_COMBOEDIT, this);
		SetFocus();
		switch (nFirstChar)
		{
		case VK_LBUTTON:
		case VK_RETURN:
			m_edit.SetSel(static_cast<int>(_tcslen(m_sInitText)), -1);
			return;
		case VK_BACK: 
			m_edit.SetSel(static_cast<int>(_tcslen(m_sInitText)), -1);
			break;
		case VK_DOWN:
		case VK_UP:
		case VK_RIGHT:
		case VK_LEFT:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_HOME:
		case VK_END:
			m_edit.SetSel(0, -1);
			return;
		default: 
			m_edit.SetSel(0, -1);
		}
		SendMessage(WM_CHAR, nFirstChar);
	}
	else
		SetFocus();
}

GridInPlaceList::~GridInPlaceList()
= default;

void GridInPlaceList::EndEdit()
{
	CString str;
	if (IsWindow(m_hWnd))
		GetWindowText(str);

	// Send Notification to parent
	GV_DISPINFO dispinfo;

	dispinfo.hdr.hwndFrom = GetSafeHwnd();
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = GVN_ENDLABELEDIT;

	dispinfo.item.mask = LVIF_TEXT | LVIF_PARAM;
	dispinfo.item.row = m_nRow;
	dispinfo.item.col = m_nCol;
	dispinfo.item.strText = str;
	dispinfo.item.lParam = static_cast<LPARAM>(m_nLastChar);

	const CWnd* pOwner = GetOwner();
	if (IsWindow(pOwner->GetSafeHwnd()))
		pOwner->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dispinfo);

	// Close this window (PostNcDestroy will delete this)
	if (IsWindow(m_hWnd))
		PostMessage(WM_CLOSE, 0, 0);
}

int GridInPlaceList::GetCorrectDropWidth()
{
	constexpr int nMaxWidth = 200; // don't let the box be bigger than this

	// Reset the dropped width
	const int nNumEntries = GetCount();
	int nWidth = 0;
	CString str;

	CClientDC dc(this);
	const int nSave = dc.SaveDC();
	dc.SelectObject(GetFont());

	const int nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	for (int i = 0; i < nNumEntries; i++)
	{
		GetLBText(i, str);
		int nLength = dc.GetTextExtent(str).cx + nScrollWidth;
		nWidth = max(nWidth, nLength);
	}

	// Add margin space to the calculations
	nWidth += dc.GetTextExtent(_T("0")).cx;

	dc.RestoreDC(nSave);

	nWidth = min(nWidth, nMaxWidth);

	return nWidth;
	//SetDroppedWidth(nWidth);
}

/*
// Fix by Ray (raybie@Exabyte.COM)
void GridInPlaceList::OnSelendOK()
{
	int iIndex = GetCurSel();
	if( iIndex != CB_ERR)
	{
		CString strLbText;
		GetLBText( iIndex, strLbText);

		if (!((GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST))
		   m_edit.SetWindowText( strLbText);
	}

	GetParent()->SetFocus();
}
*/

void GridInPlaceList::PostNcDestroy()
{
	CComboBox::PostNcDestroy();

	delete this;
}

BEGIN_MESSAGE_MAP(GridInPlaceList, CComboBox)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropdown)
	ON_WM_GETDLGCODE()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GridInPlaceList message handlers

UINT GridInPlaceList::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}

void GridInPlaceList::OnDropdown()
{
	SetDroppedWidth(GetCorrectDropWidth());
}

void GridInPlaceList::OnKillFocus(CWnd* pNewWnd)
{
	CComboBox::OnKillFocus(pNewWnd);

	if (GetSafeHwnd() == pNewWnd->GetSafeHwnd())
		return;

	// Only end editing on change of focus if we're using the CBS_DROPDOWNLIST style
	if ((GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)
		EndEdit();
}

// If an arrow key (or associated) is pressed, then exit if
//  a) The Ctrl key was down, or
//  b) m_bExitOnArrows == TRUE
void GridInPlaceList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT n_flags)
{
	if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		nChar == VK_DOWN || nChar == VK_UP ||
		nChar == VK_RIGHT || nChar == VK_LEFT) &&
		(m_bExitOnArrows || GetKeyState(VK_CONTROL) < 0))
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus();
		return;
	}

	CComboBox::OnKeyDown(nChar, nRepCnt, n_flags);
}

// Need to keep a lookout for Tabs, Esc and Returns.
void GridInPlaceList::OnKeyUp(UINT nChar, UINT nRepCnt, UINT n_flags)
{
	if (nChar == VK_ESCAPE)
		SetWindowText(m_sInitText); // restore previous text

	if (nChar == VK_TAB || nChar == VK_RETURN || nChar == VK_ESCAPE)
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus(); // This will destroy this window
		return;
	}

	CComboBox::OnKeyUp(nChar, nRepCnt, n_flags);
}

HBRUSH GridInPlaceList::CtlColor(CDC* /*p_dc*/, UINT /*nCtlColor*/)
{
	/*
	static CBrush brush(m_crBackClr);
	p_dc->SetTextColor(m_crForeClr);
	p_dc->SetBkMode(TRANSPARENT);
	return (HBRUSH) brush.GetSafeHandle();
	*/

	// TODO: Return a non-NULL brush if the parent's handler should not be called
	return nullptr;
}

