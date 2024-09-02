#pragma once

#include <afxwin.h>
#include "GridComboEdit.h"

class GridInPlaceList : public CComboBox
{
	friend class GridComboEdit;

	// Construction
public:
	GridInPlaceList(CWnd* p_parent,	// parent
		CRect& rect,				// dimensions & location
		DWORD dw_style,				// window/combobox style
		UINT nID,					// control ID
		int nRow, int nColumn,		// row and column
		COLORREF crFore,			// Foreground colour
		COLORREF crBack,			// Background colour
		CStringArray& Items,		// Items in list
		CString sInitText,			// initial selection
		UINT nFirstChar);			// first character to pass to control

	GridComboEdit m_edit;			// subclassed edit control
private:
	int			m_nNumLines;
	CString		m_sInitText;
	int			m_nRow;
	int			m_nCol;
	UINT		m_nLastChar;
	BOOL		m_bExitOnArrows;
	COLORREF	m_crForeClr;
	COLORREF	m_crBackClr;

public:
	void PostNcDestroy() override;
	~GridInPlaceList() override;
	void EndEdit();

protected:
	int GetCorrectDropWidth();

	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT n_flags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT n_flags);
	afx_msg void OnDropdown();
	afx_msg UINT OnGetDlgCode();
	afx_msg HBRUSH CtlColor(CDC* p_dc, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};


