// GridCellNumeric.h: interface for the CGridCellNumeric class.
//
// Written by Andrew Truckle [ajtruckle@wsatkins.co.uk]
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "GridCell.h"

class CGridCellNumeric : public CGridCell
{
	DECLARE_DYNCREATE(CGridCellNumeric)

public:
	BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar) override;
	void EndEdit() override;
};
