#pragma once

// GridCellDateTime.h: interface for the CGridCellDateTime class.
//
// Provides the implementation for a datetime picker cell type of the
// grid control.
//
// For use with CGridCtrl v2.22+
//
//////////////////////////////////////////////////////////////////////


#include "GridCell.h"
#include "afxdtctl.h"	// for CDateTimeCtrl

class CGridCellDateTime : public CGridCell  
{
  friend class CGridCtrl;
  DECLARE_DYNCREATE(CGridCellDateTime)

  CTime m_cTime;
  DWORD m_dwStyle;

public:
	CGridCellDateTime();
	CGridCellDateTime(DWORD dw_style);
	virtual ~CGridCellDateTime();
	virtual CSize GetCellExtent(CDC* p_dc);

  // editing cells
public:
	void Init(DWORD dw_style);
	virtual BOOL  Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
	virtual CWnd* GetEditWnd() const;
	virtual void  EndEdit();


	CTime* GetTime() {return &m_cTime;};
	void   SetTime(CTime time);
};

class CInPlaceDateTime : public CDateTimeCtrl
{
// Construction
public:
	CInPlaceDateTime(CWnd* pParent,         // parent
				   CRect& rect,           // dimensions & location
				   DWORD dw_style,         // window/combobox style
				   UINT nID,              // control ID
				   int nRow, int nColumn, // row and column
				   COLORREF crFore, COLORREF crBack,  // Foreground, background colour
				   CTime* pcTime,
				   UINT nFirstChar);      // first character to pass to control

// Overrides
	protected:
	virtual void PostNcDestroy();

// Implementation
public:
	virtual ~CInPlaceDateTime();
	void EndEdit();

// Generated message map functions
protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()

private:
	CTime*   m_pcTime;
	int		 m_nRow;
	int		 m_nCol;
	UINT     m_nLastChar; 
	BOOL	 m_bExitOnArrows; 
	COLORREF m_crForeClr, m_crBackClr;
};
