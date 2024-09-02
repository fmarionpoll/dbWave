#pragma once

// GridCellDateTime.h: interface for the GridCellDateTime class.
//
// Provides the implementation for a datetime picker cell type of the
// grid control.
//
// For use with GridCtrl v2.22+
//
//////////////////////////////////////////////////////////////////////

#include "GridCell.h"
#include "afxdtctl.h"	// for CDateTimeCtrl

class GridCellDateTime : public GridCell
{
	friend class GridCtrl;
	DECLARE_DYNCREATE(GridCellDateTime)

	CTime m_cTime;
	DWORD m_dwStyle;

public:
	GridCellDateTime();
	GridCellDateTime(DWORD dw_style);
	~GridCellDateTime() override;
	CSize GetCellExtent(CDC* p_dc) override;

	// editing cells
public:
	void Init(DWORD dw_style);
	BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar) override;
	CWnd* GetEditWnd() const override;
	void EndEdit() override;

	CTime* GetTime() { return &m_cTime; };
	void SetTime(CTime time);
};

class InPlaceDateTime : public CDateTimeCtrl
{
	// Construction
public:
	InPlaceDateTime(CWnd* p_parent, // parent
	                 CRect& rect, // dimensions & location
	                 DWORD dw_style, // window/combobox style
	                 UINT nID, // control ID
	                 int nRow, int nColumn, // row and column
	                 COLORREF crFore, COLORREF crBack, // Foreground, background colour
	                 CTime* pcTime,
	                 UINT nFirstChar); // first character to pass to control

private:
	CTime* m_pcTime;
	int m_nRow;
	int m_nCol;
	UINT m_nLastChar;
	BOOL m_bExitOnArrows;
	COLORREF m_crForeClr, m_crBackClr;

	// Overrides
protected:
	void PostNcDestroy() override;

	// Implementation
public:
	~InPlaceDateTime() override;
	void EndEdit();

	// Generated message map functions
protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT n_flags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT n_flags);
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()
};
