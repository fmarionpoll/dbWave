///////////////////////////////////////////////////////////////////////
// GridCellRange.h: header file
//
// MFC Grid Control - interface for the GridCellRange class.
//
// Written by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2002. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name and all copyright
// notices remains intact.
//
// An email letting me know how you are using it would be nice as well.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with GridCtrl v2.20+
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CELLRANGE_H__F86EF761_725A_11D1_ABBA_00A0243D1382__INCLUDED_)
#define AFX_CELLRANGE_H__F86EF761_725A_11D1_ABBA_00A0243D1382__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// The code contained in this file is based on the original
// WorldCom Grid control written by Joe Willcoxson,
//      mailto:chinajoe@aol.com
//      http://users.aol.com/chinajoe

class GridCellID
{
	// Attributes
public:
	int row, col;

	// Operations
public:
	explicit GridCellID(int nRow = -1, int nCol = -1) : row(nRow), col(nCol)

	{
	}

	int IsValid() const { return (row >= 0 && col >= 0); }
	int operator==(const GridCellID& rhs) const { return (row == rhs.row && col == rhs.col); }
	int operator!=(const GridCellID& rhs) const { return !operator==(rhs); }
};

class GridCellRange
{
public:
	GridCellRange(int nMinRow = -1, int nMinCol = -1, int nMaxRow = -1, int nMaxCol = -1)
	{
		m_nMinRow = -1;
		m_nMinCol = -1;
		m_nMaxRow = -1;
		m_nMaxCol = -1;
		Set(nMinRow, nMinCol, nMaxRow, nMaxCol);
	}

	void Set(int nMinRow = -1, int nMinCol = -1, int nMaxRow = -1, int nMaxCol = -1);

	int IsValid() const;
	int InRange(int row, int col) const;
	int InRange(const GridCellID& cellID) const;
	int Count() { return (m_nMaxRow - m_nMinRow + 1) * (m_nMaxCol - m_nMinCol + 1); }

	GridCellID GetTopLeft() const;
	GridCellRange Intersect(const GridCellRange& rhs) const;

	int GetMinRow() const { return m_nMinRow; }
	void SetMinRow(int minRow) { m_nMinRow = minRow; }

	int GetMinCol() const { return m_nMinCol; }
	void SetMinCol(int minCol) { m_nMinCol = minCol; }

	int GetMaxRow() const { return m_nMaxRow; }
	void SetMaxRow(int maxRow) { m_nMaxRow = maxRow; }

	int GetMaxCol() const { return m_nMaxCol; }
	void SetMaxCol(int maxCol) { m_nMaxCol = maxCol; }

	int GetRowSpan() const { return m_nMaxRow - m_nMinRow + 1; }
	int GetColSpan() const { return m_nMaxCol - m_nMinCol + 1; }

	void operator=(const GridCellRange& rhs);
	int operator==(const GridCellRange& rhs);
	int operator!=(const GridCellRange& rhs);

protected:
	int m_nMinRow;
	int m_nMinCol;
	int m_nMaxRow;
	int m_nMaxCol;
};

inline void GridCellRange::Set(int minRow, int minCol, int maxRow, int maxCol)
{
	m_nMinRow = minRow;
	m_nMinCol = minCol;
	m_nMaxRow = maxRow;
	m_nMaxCol = maxCol;
}

inline void GridCellRange::operator=(const GridCellRange& rhs)
{
	if (this != &rhs) Set(rhs.m_nMinRow, rhs.m_nMinCol, rhs.m_nMaxRow, rhs.m_nMaxCol);
}

inline int GridCellRange::operator==(const GridCellRange& rhs)
{
	return ((m_nMinRow == rhs.m_nMinRow) && (m_nMinCol == rhs.m_nMinCol) &&
		(m_nMaxRow == rhs.m_nMaxRow) && (m_nMaxCol == rhs.m_nMaxCol));
}

inline int GridCellRange::operator!=(const GridCellRange& rhs)
{
	return !operator==(rhs);
}

inline int GridCellRange::IsValid() const
{
	return (m_nMinRow >= 0 && m_nMinCol >= 0 && m_nMaxRow >= 0 && m_nMaxCol >= 0 &&
		m_nMinRow <= m_nMaxRow && m_nMinCol <= m_nMaxCol);
}

inline int GridCellRange::InRange(int row, int col) const
{
	return (row >= m_nMinRow && row <= m_nMaxRow && col >= m_nMinCol && col <= m_nMaxCol);
}

inline int GridCellRange::InRange(const GridCellID& cellID) const
{
	return InRange(cellID.row, cellID.col);
}

inline GridCellID GridCellRange::GetTopLeft() const
{
	return GridCellID(m_nMinRow, m_nMinCol);
}

inline GridCellRange GridCellRange::Intersect(const GridCellRange& rhs) const
{
	return GridCellRange(max(m_nMinRow, rhs.m_nMinRow), max(m_nMinCol, rhs.m_nMinCol),
	                  min(m_nMaxRow, rhs.m_nMaxRow), min(m_nMaxCol, rhs.m_nMaxCol));
}

#endif // !defined(AFX_CELLRANGE_H__F86EF761_725A_11D1_ABBA_00A0243D1382__INCLUDED_)
