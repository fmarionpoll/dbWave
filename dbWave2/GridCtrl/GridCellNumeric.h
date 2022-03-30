// GridCellNumeric.h: interface for the GridCellNumeric class.
//
// Written by Andrew Truckle [ajtruckle@wsatkins.co.uk]
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "GridCell.h"

class GridCellNumeric : public GridCell
{
	DECLARE_DYNCREATE(GridCellNumeric)

public:
	BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar) override;
	void EndEdit() override;
};
