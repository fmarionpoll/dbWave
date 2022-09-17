// GridCellNumeric.cpp: implementation of the GridCellNumeric class.
//
// Written by Andrew Truckle [ajtruckle@wsatkins.co.uk]
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GridCellNumeric.h"
#include "Gridinplaceedit.h"
#include "GridCtrl.h"

IMPLEMENT_DYNCREATE(GridCellNumeric, GridCell)

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Create a control to do the editing
BOOL GridCellNumeric::Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar)
{
	m_bEditing = TRUE;
	m_pEditWnd = new GridInPlaceEdit(GetGrid(), rect, /*GetStyle() |*/ ES_NUMBER, nID, nRow, nCol,
	                              GetText(), nChar);

	return TRUE;
}

// Cancel the editing.
void GridCellNumeric::EndEdit()
{
	if (m_pEditWnd)
		static_cast<GridInPlaceEdit*>(m_pEditWnd)->EndEdit();
}
